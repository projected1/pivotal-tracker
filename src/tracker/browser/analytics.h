#ifndef CEF_TESTS_CEFCLIENT_BROWSER_ANALYTICS_H_
#define CEF_TESTS_CEFCLIENT_BROWSER_ANALYTICS_H_
#pragma once

#include <string>

#include "include/base/cef_macros.h"

namespace client {

class Analytics {
 public:
  static Analytics* Create(const std::string& client_id);

  virtual ~Analytics();

  virtual void TrackEvent(const std::string& category,
                          const std::string& action,
                          const std::string& label = "",
                          const std::string& value = "") = 0;

 protected:
  Analytics();

 private:
  DISALLOW_COPY_AND_ASSIGN(Analytics);
};

}  // namespace client

#endif  // CEF_TESTS_CEFCLIENT_BROWSER_ANALYTICS_H_
