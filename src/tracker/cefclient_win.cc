// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include <fstream>

#include <windows.h>

#include "include/base/cef_scoped_ptr.h"
#include "include/cef_command_line.h"
#include "include/cef_sandbox_win.h"
#include "tracker/browser/client_urls.h"
#include "tracker/browser/main_context_impl.h"
#include "tracker/browser/main_message_loop_multithreaded_win.h"
#include "tracker/browser/root_window_manager.h"
#include "tracker/browser/test_runner.h"
#include "shared/browser/client_app_browser.h"
#include "shared/browser/main_message_loop_external_pump.h"
#include "shared/browser/main_message_loop_std.h"
#include "shared/common/client_app_other.h"
#include "shared/common/client_switches.h"
#include "shared/renderer/client_app_renderer.h"

// When generating projects with CMake the CEF_USE_SANDBOX value will be defined
// automatically if using the required compiler version. Pass -DUSE_SANDBOX=OFF
// to the CMake command-line to disable use of the sandbox.
// Uncomment this line to manually enable sandbox support.
// #define CEF_USE_SANDBOX 1

#if defined(CEF_USE_SANDBOX)
// The cef_sandbox.lib static library is currently built with VS2015. It may not
// link successfully with other VS versions.
#pragma comment(lib, "cef_sandbox.lib")
#endif

namespace client {
namespace {

// Filename, whose presence indicates a first app run.
const char kFirstRunFilename[] = "first_run";

int RunMain(HINSTANCE hInstance, int nCmdShow) {
  // Enable High-DPI support on Windows 7 or newer.
  CefEnableHighDPISupport();

  CefMainArgs main_args(hInstance);

  void* sandbox_info = NULL;

#if defined(CEF_USE_SANDBOX)
  // Manage the life span of the sandbox information object. This is necessary
  // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
  CefScopedSandboxInfo scoped_sandbox;
  sandbox_info = scoped_sandbox.sandbox_info();
#endif

  // Parse command-line arguments.
  CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
  command_line->InitFromString(::GetCommandLineW());

  // Create a ClientApp of the correct type.
  CefRefPtr<CefApp> app;
  ClientApp::ProcessType process_type = ClientApp::GetProcessType(command_line);
  if (process_type == ClientApp::BrowserProcess) {
    // Allow only one browser instance.
    ::CreateMutex(nullptr, TRUE, L"Local\\86c30d51-9b1a-4633-a268-d707d074aa30");
    if (ERROR_ALREADY_EXISTS == ::GetLastError())
      return 1;

    app = new ClientAppBrowser();
  }
  else if (process_type == ClientApp::RendererProcess)
    app = new ClientAppRenderer();
  else if (process_type == ClientApp::OtherProcess)
    app = new ClientAppOther();

#if defined(NDEBUG)
  // Discard the command line in release builds.
  command_line->Reset();
#endif

  // Execute the secondary process, if any.
  int exit_code = CefExecuteProcess(main_args, app, sandbox_info);
  if (exit_code >= 0)
    return exit_code;

  // Create the main context object.
  scoped_ptr<MainContextImpl> context(new MainContextImpl(command_line, true));

  CefSettings settings;

#if !defined(CEF_USE_SANDBOX)
  settings.no_sandbox = true;
#endif

  // Populate the settings based on command line arguments.
  context->PopulateSettings(&settings);

  // Create the main message loop object.
  scoped_ptr<MainMessageLoop> message_loop;
  if (settings.multi_threaded_message_loop)
    message_loop.reset(new MainMessageLoopMultithreadedWin);
  else if (settings.external_message_pump)
    message_loop = MainMessageLoopExternalPump::Create();
  else
    message_loop.reset(new MainMessageLoopStd);

  // Initialize CEF.
  context->Initialize(main_args, settings, app, sandbox_info);

  // Send analytics event on first app run.
  std::string first_run_file =
    context->GetAppWorkingDirectory() + kFirstRunFilename;
  if (std::fstream(first_run_file))
    if (0 == std::remove(first_run_file.c_str()))
      context->GetAnalytics()->TrackEvent("desktop", "first_run");

  // Register scheme handlers.
  test_runner::RegisterSchemeHandlers();

  RootWindowConfig splash_screen_config;
  splash_screen_config.url = urls::kSplashScreen;
  splash_screen_config.splash_screen = true;
  splash_screen_config.initially_hidden = true;

  // Create a splash screen.
  context->GetRootWindowManager()->CreateRootWindow(splash_screen_config);

  RootWindowConfig window_config;
  window_config.with_osr = settings.windowless_rendering_enabled ? true : false;
  window_config.initially_hidden = true;

  // Create the first window.
  context->GetRootWindowManager()->CreateRootWindow(window_config);

  // Run the message loop. This will block until Quit() is called by the
  // RootWindowManager after all windows have been destroyed.
  int result = message_loop->Run();

  // Shut down CEF.
  context->Shutdown();

  // Release objects in reverse order of creation.
  message_loop.reset();
  context.reset();

  return result;
}

}  // namespace
}  // namespace client

// Program entry point function.
int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPTSTR lpCmdLine,
                      int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);
  return client::RunMain(hInstance, nCmdShow);
}
