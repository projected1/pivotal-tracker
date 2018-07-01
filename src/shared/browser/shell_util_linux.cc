#include <cstdlib>
#include <sstream>

#include "shared/browser/shell_util.h"
#include "include/base/cef_logging.h"

namespace client {
namespace shell_util {

namespace {

const char kOpenAction[] = "xdg-open";

}

///
// "system()" returns after the command has been completed,
// locking the parent process until it does.
// There are several solutions that can help keeping the
// parent process unlocked:
// 1. Appending "&" to the command, will cause the command
//    to execute in the background and will return control
//    immediately.
// 2. Executing the command in a new, detached thread. It
//    is essential to "detach" so that the wrapping thread
//    object does not get destroyed when it goes out of scope,
//    destroying the thread and the running child process.
//    e.g.
//    std::thread t([] { std::system("do something"); });
//    t.detach();
///
void URLOpenInDefaultBrowser(const std::string& url) {
  if (!std::system(nullptr))
    return;

  std::stringstream ss;
  ss << kOpenAction << ' ' << url << " &";
  std::system(ss.str().c_str());
}

void FileOpenInDefaultFileManager(const std::string& file) {
  NOTIMPLEMENTED() << "Open file in default file manager";
  return;
}

}  // namespace shell_util
}  // namespace client
