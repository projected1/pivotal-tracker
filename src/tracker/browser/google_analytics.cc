#include "tracker/browser/app_info.h"
#include "tracker/browser/google_analytics.h"
#include "tracker/browser/main_context.h"
#include "tracker/browser/url_request_manager.h"

namespace client {

namespace {

const char kEndpoint[] = "http://www.google-analytics.com/collect?v=1";

const char kHitType[] = "&t=event";   // We always use "event"
const char kDataSource[] = "&ds=app"; // We always use "app"

const char kClientId[] = "&cid=";     // UUID v4
const char kAccountId[] = "&tid=";    // GA account ID

const char kAppId[] = "&aid=";        // e.g. com.example.app
const char kAppName[] = "&an=";       // Required for app related hits
const char kAppVersion[] = "&av=";    // e.g. 1.0.0.acf32a

const char kEventAction[] = "&ea=";    // Event action
const char kEventCategory[] = "&ec=";  // Event category

const char kEventLabel[] = "&el=";     // Event label (optional)
const char kEventValue[] = "&ev=";     // Event value (optional)

const char kCacheBuster[] = "&z=";     // Random number

}  // namespace

GoogleAnalytics::GoogleAnalytics(const std::string& account_id,
                                 const std::string& client_id) {
  srand(time(NULL));

  // Init tracking URL component.
  base_url_ << kEndpoint  << kDataSource << kHitType <<
    kClientId << client_id <<
    kAccountId << account_id <<
    kAppId << APP_ID <<
    kAppName << APP_NAME <<
    kAppVersion << APP_VERSION;
}

GoogleAnalytics::~GoogleAnalytics() {}

void GoogleAnalytics::TrackEvent(const std::string& category,
                                 const std::string& action,
                                 const std::string& label,
                                 const std::string& value) {
  std::stringstream tracking_url;
  tracking_url << base_url_.str() <<
    kEventCategory << category <<
    kEventAction << action <<
    kCacheBuster << rand();

  if (!label.empty())
    tracking_url << kEventLabel << label;

  if (!value.empty())
    tracking_url << kEventValue << value;

  MainContext::Get()->GetURLRequestManager()->IssueRequest(tracking_url.str());
}

}  // namespace client
