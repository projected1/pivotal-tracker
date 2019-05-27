// Copyright (c) 2015 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "tracker/browser/main_context_impl.h"

#include "include/cef_parser.h"
#include "include/cef_web_plugin.h"
#include "shared/common/client_switches.h"
#include "tracker/browser/client_urls.h"
#include "tracker/browser/rand_util.h"

namespace client {

namespace {

// Send a heartbeat analytics event.
void TrackHeartbeat() {
  CEF_REQUIRE_UI_THREAD();
  MainContext::Get()->GetAnalytics()->TrackEvent("desktop", "heartbeat");
}

// Returns the ARGB value for |color|.
cef_color_t ParseColor(const std::string& color) {
  std::string colorToLower;
  colorToLower.resize(color.size());
  std::transform(color.begin(), color.end(), colorToLower.begin(), ::tolower);

  if (colorToLower == "black")
    return CefColorSetARGB(255, 0, 0, 0);
  else if (colorToLower == "blue")
    return CefColorSetARGB(255, 0, 0, 255);
  else if (colorToLower == "green")
    return CefColorSetARGB(255, 0, 255, 0);
  else if (colorToLower == "red")
    return CefColorSetARGB(255, 255, 0, 0);
  else if (colorToLower == "white")
    return CefColorSetARGB(255, 255, 255, 255);

  // Use the default color.
  return 0;
}

}  // namespace

MainContextImpl::MainContextImpl(CefRefPtr<CefCommandLine> command_line,
                                 bool terminate_when_all_windows_closed)
    : command_line_(command_line),
      terminate_when_all_windows_closed_(terminate_when_all_windows_closed),
      initialized_(false),
      shutdown_(false),
      background_color_(0),
      browser_background_color_(0),
      use_views_(false) {
  DCHECK(command_line_.get());

  // Init the client settings and subscribe for settings changes.
  client_settings_.reset(new ClientSettings(this));
  settings_serializer_.Deserialize(*client_settings_.get());

  // Ensure we have a valid client app ID.
  std::string client_id = client_settings_->GetClientId();
  if (client_id.empty()) {
    client_id = rand_util::GenerateGUID();
    client_settings_->SetClientId(client_id);
  }

  // Init the application analytics object.
  analytics_.reset(Analytics::Create(client_id));

  // Init the heartbeat analytics event thread.
  heartbeat_thread_.reset(new std::thread([] {
    using namespace std::chrono_literals;
    while (1) {
      CefPostTask(TID_UI, base::Bind(&TrackHeartbeat));
      std::this_thread::sleep_for(1min);
    }
  }));

  // Set the main URL.
  if (command_line_->HasSwitch(switches::kUrl))
    main_url_ = command_line_->GetSwitchValue(switches::kUrl);
  if (main_url_.empty()) {
    std::string project_id = client_settings_->GetProjectId();
    main_url_ = project_id.empty() ? urls::kTrackerSignin :
      std::string(urls::kTrackerSigninAndReturnToProject) + project_id;
  }

  // Whether windowless (off-screen) rendering will be used.
  use_windowless_rendering_ =
      command_line_->HasSwitch(switches::kOffScreenRenderingEnabled);

  // Whether transparent painting is used with windowless rendering.
  const bool use_transparent_painting =
      use_windowless_rendering_ &&
      command_line_->HasSwitch(switches::kTransparentPaintingEnabled);

#if defined(OS_WIN) || defined(OS_LINUX)
  // Whether the Views framework will be used.
  use_views_ = command_line_->HasSwitch(switches::kUseViews);

  if (use_windowless_rendering_ && use_views_) {
    LOG(ERROR)
        << "Windowless rendering is not supported by the Views framework.";
    use_views_ = false;
  }

  if (use_views_ && command_line->HasSwitch(switches::kHideFrame) &&
      !command_line_->HasSwitch(switches::kUrl)) {
    // Use the draggable regions test as the default URL for frameless windows.
    main_url_ = "http://tests/draggable";
  }
#endif  // defined(OS_WIN) || defined(OS_LINUX)

  if (command_line_->HasSwitch(switches::kBackgroundColor)) {
    // Parse the background color value.
    background_color_ =
        ParseColor(command_line_->GetSwitchValue(switches::kBackgroundColor));
  }

  if (background_color_ == 0 && !use_views_) {
    // Set an explicit background color.
    background_color_ = CefColorSetARGB(255, 255, 255, 255);
  }

  // |browser_background_color_| should remain 0 to enable transparent painting.
  if (!use_transparent_painting) {
    browser_background_color_ = background_color_;
  }

  const std::string& cdm_path =
      command_line_->GetSwitchValue(switches::kWidevineCdmPath);
  if (!cdm_path.empty()) {
    // Register the Widevine CDM at the specified path. See comments in
    // cef_web_plugin.h for details. It's safe to call this method before
    // CefInitialize(), and calling it before CefInitialize() is required on
    // Linux.
    CefRegisterWidevineCdm(cdm_path, NULL);
  }
}

MainContextImpl::~MainContextImpl() {
  // The context must either not have been initialized, or it must have also
  // been shut down.
  DCHECK(!initialized_ || shutdown_);
}

std::string MainContextImpl::GetConsoleLogPath() {
  return GetAppWorkingDirectory() + "console.log";
}

std::string MainContextImpl::GetMainURL() {
  return main_url_;
}

cef_color_t MainContextImpl::GetBackgroundColor() {
  return background_color_;
}

bool MainContextImpl::UseViews() {
  return use_views_;
}

bool MainContextImpl::UseWindowlessRendering() {
  return use_windowless_rendering_;
}

void MainContextImpl::PopulateSettings(CefSettings* settings) {
#if defined(OS_WIN)
  settings->multi_threaded_message_loop =
      command_line_->HasSwitch(switches::kMultiThreadedMessageLoop);
#endif

  if (!settings->multi_threaded_message_loop) {
    settings->external_message_pump =
        command_line_->HasSwitch(switches::kExternalMessagePump);
  }

  CefString(&settings->cache_path) = GetAppWorkingDirectory() + "app.cache";

  if (use_windowless_rendering_)
    settings->windowless_rendering_enabled = true;

  if (browser_background_color_ != 0)
    settings->background_color = browser_background_color_;

  // Disable processing of CEF/Chromium command-line.
  settings->command_line_args_disabled = 1;

#if defined(NDEBUG)
  settings->log_severity = LOGSEVERITY_ERROR;
#endif
}

void MainContextImpl::PopulateBrowserSettings(CefBrowserSettings* settings) {
  if (command_line_->HasSwitch(switches::kOffScreenFrameRate)) {
    settings->windowless_frame_rate =
        atoi(command_line_->GetSwitchValue(switches::kOffScreenFrameRate)
                 .ToString()
                 .c_str());
  }

  if (browser_background_color_ != 0)
    settings->background_color = browser_background_color_;
}

void MainContextImpl::PopulateOsrSettings(OsrRenderer::Settings* settings) {
  settings->show_update_rect =
      command_line_->HasSwitch(switches::kShowUpdateRect);

  if (browser_background_color_ != 0)
    settings->background_color = browser_background_color_;
}

RootWindowManager* MainContextImpl::GetRootWindowManager() {
  DCHECK(InValidState());
  return root_window_manager_.get();
}

ClientSettings* MainContextImpl::GetClientSettings() {
  return client_settings_.get();
}

Analytics* MainContextImpl::GetAnalytics() {
  DCHECK(InValidState());
  return analytics_.get();
}

URLRequestManager* MainContextImpl::GetURLRequestManager() {
  DCHECK(InValidState());
  return url_request_manager_.get();
}

void MainContextImpl::OnSettingsChanged() {
  settings_serializer_.Serialize(*client_settings_.get());
}

bool MainContextImpl::Initialize(const CefMainArgs& args,
                                 const CefSettings& settings,
                                 CefRefPtr<CefApp> application,
                                 void* windows_sandbox_info) {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(!initialized_);
  DCHECK(!shutdown_);

  if (!CefInitialize(args, settings, application, windows_sandbox_info))
    return false;

  // Need to create the RootWindowManager after calling CefInitialize because
  // TempWindowX11 uses cef_get_xdisplay().
  root_window_manager_.reset(
      new RootWindowManager(terminate_when_all_windows_closed_));

  // Init the URL request manager.
  url_request_manager_.reset(new URLRequestManager());

  initialized_ = true;

  return true;
}

void MainContextImpl::Shutdown() {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(initialized_);
  DCHECK(!shutdown_);

  heartbeat_thread_.reset();

  url_request_manager_.reset();

  root_window_manager_.reset();

  CefShutdown();

  shutdown_ = true;
}

}  // namespace client
