#include <memory>
#include <type_traits>

#include <CoreFoundation/CFBundle.h>
#include <ApplicationServices/ApplicationServices.h>

#include "shared/browser/shell_util.h"

namespace client {
namespace shell_util {

void URLOpenInDefaultBrowser(const std::string& url) {
  using cfurl_type = std::remove_pointer<CFURLRef>::type;
  using cfrelease_type = decltype(&::CFRelease);

  const UInt8* url_bytes =
    reinterpret_cast<const UInt8*>(url.c_str());

  std::unique_ptr<cfurl_type, cfrelease_type> cf_url(
    ::CFURLCreateWithBytes(
      kCFAllocatorDefault,
      url_bytes,
      url.length(),
      kCFStringEncodingUTF8,
      nullptr), // baseURL - the URL to which URLBytes is relative
    ::CFRelease);

  if (cf_url)
    ::LSOpenCFURLRef(
      cf_url.get(),
      nullptr); // CFURL that identifies the launched application [out]
}

}  // namespace shell_util
}  // namespace client
