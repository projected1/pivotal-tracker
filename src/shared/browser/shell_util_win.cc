#include <windows.h>
#include <shellapi.h>
#pragma comment(lib, "shell32.lib")

#include "shared/browser/shell_util.h"

namespace client {
namespace shell_util {

namespace {

const char kOpenAction[] = "open";

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

}  // namespace shell_util
}  // namespace client
