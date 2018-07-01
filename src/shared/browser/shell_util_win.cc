#include <sstream>

#include <windows.h>
#include <shellapi.h>
#pragma comment(lib, "shell32.lib")

#include "shared/browser/shell_util.h"

namespace client {
namespace shell_util {

namespace {

const char kOpenAction[] = "open";
const char kExplorerProcName[] = "explorer.exe";

}

void URLOpenInDefaultBrowser(const std::string& url) {
  ::ShellExecuteA(
    nullptr, // Parent window handle
    kOpenAction,
    url.c_str(),
    nullptr, // Parameters to be passed to the application
    nullptr, // The default working directory for the action
    SW_SHOWNORMAL);
}

void FileOpenInDefaultFileManager(const std::string& file) {
  std::stringstream params;
  params << "/select," << file;
  ::ShellExecuteA(
    nullptr,
    kOpenAction,
    kExplorerProcName,
    params.str().c_str(),
    nullptr,
    SW_SHOWNORMAL);
}

}  // namespace shell_util
}  // namespace client
