#ifndef CEF_TESTS_SHARED_BROWSER_SHELL_UTIL_H_
#define CEF_TESTS_SHARED_BROWSER_SHELL_UTIL_H_

#include <string>

namespace client {
namespace shell_util {

void URLOpenInDefaultBrowser(const std::string& url);
void FileOpenInDefaultFileManager(const std::string& file);

}  // namespace shell_util
}  // namespace client

#endif  // CEF_TESTS_SHARED_BROWSER_SHELL_UTIL_H_
