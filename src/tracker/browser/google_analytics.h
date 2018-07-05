#ifndef CEF_TESTS_CEFCLIENT_BROWSER_GOOGLE_ANALYTICS_H_
#define CEF_TESTS_CEFCLIENT_BROWSER_GOOGLE_ANALYTICS_H_
#pragma once

#include <sstream>

#include "tracker/browser/analytics.h"

namespace client {

class GoogleAnalytics : public Analytics {
public:
  GoogleAnalytics(const std::string& account_id, const std::string& client_id);
  ~GoogleAnalytics();

  void TrackEvent(const std::string& category,
                  const std::string& action,
                  const std::string& label = "",
                  const std::string& value = "");

private:
  std::stringstream base_url_;

  DISALLOW_COPY_AND_ASSIGN(GoogleAnalytics);
};

}  // namespace client

#endif  // CEF_TESTS_CEFCLIENT_BROWSER_GOOGLE_ANALYTICS_H_
