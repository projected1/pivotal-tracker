#include "tracker/browser/analytics.h"
#include "tracker/browser/app_info.h"
#include "tracker/browser/google_analytics.h"

namespace client {

// static
Analytics* Analytics::Create(const std::string& client_id) {
  return new GoogleAnalytics(GA_ACCOUNT_ID, client_id);
}

Analytics::Analytics() {}

Analytics::~Analytics() {}

}  // namespace client
