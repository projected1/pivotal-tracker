#include <ctime>
#include <string>

#include "include/base/cef_bind.h"
#include "include/base/cef_callback.h"
#include "include/base/cef_lock.h"
#include "include/base/cef_logging.h"
#include "include/cef_urlrequest.h"
#include "include/wrapper/cef_helpers.h"
#include "tracker/browser/test_runner.h"
#include "tracker/browser/url_request_manager.h"

namespace client {

namespace {

// Implementation of CefURLRequestClient that stores response information.
// Only accessed on the UI thread.
class RequestClientImpl : public CefURLRequestClient {
 public:
  // Callback to be executed on request completion.
  typedef base::Callback<void(CefURLRequest::ErrorCode /*error_code*/,
                              const std::string& /*download_data*/)>
    Callback;

  explicit RequestClientImpl(const Callback& callback) : callback_(callback) {
    CEF_REQUIRE_UI_THREAD();
    DCHECK(!callback_.is_null());
  }

  void Detach() {
    CEF_REQUIRE_UI_THREAD();
    if (!callback_.is_null())
      callback_.Reset();
  }

  void OnRequestComplete(CefRefPtr<CefURLRequest> request) OVERRIDE {
    CEF_REQUIRE_UI_THREAD();
    if (!callback_.is_null()) {
      callback_.Run(request->GetRequestError(), download_data_);
      callback_.Reset();
    }
  }

  void OnUploadProgress(CefRefPtr<CefURLRequest> request,
                        int64 current,
                        int64 total) OVERRIDE {}

  void OnDownloadProgress(CefRefPtr<CefURLRequest> request,
                          int64 current,
                          int64 total) OVERRIDE {}

  void OnDownloadData(CefRefPtr<CefURLRequest> request,
                      const void* data,
                      size_t data_length) OVERRIDE {
    CEF_REQUIRE_UI_THREAD();
    download_data_ += std::string(static_cast<const char*>(data), data_length);
  }

  bool GetAuthCredentials(bool isProxy,
                          const CefString& host,
                          int port,
                          const CefString& realm,
                          const CefString& scheme,
                          CefRefPtr<CefAuthCallback> callback) OVERRIDE {
    return false;
  }

 private:
  Callback callback_;
  std::string download_data_;

  IMPLEMENT_REFCOUNTING(RequestClientImpl);
  DISALLOW_COPY_AND_ASSIGN(RequestClientImpl);
};

}  // namespace

URLRequestManager::URLRequestManager() {
  CEF_REQUIRE_UI_THREAD();
  srand(time(NULL));
}

URLRequestManager::~URLRequestManager() {
  CancelPendingRequests();
}

void URLRequestManager::IssueRequest(const std::string& request_url,
                                     Delegate* delegate) {
  CEF_REQUIRE_UI_THREAD();

  // Create a CefRequest for the specified URL.
  CefRefPtr<CefRequest> request = CefRequest::Create();
  request->SetURL(request_url);
  request->SetMethod("GET");
  request->SetFlags(UR_FLAG_SKIP_CACHE);

  // Generate a request ID so we can identify the request in the callback.
  int request_id;
  do {
    request_id = rand();
  } while (url_requests_.find(request_id) != url_requests_.end());

  // Callback to be executed on request completion.
  const RequestClientImpl::Callback& request_callback =
    base::Bind(&URLRequestManager::OnRequestComplete,
               base::Unretained(this),
               request_id,
               delegate);

  // Create and start a new URL request.
  CefRefPtr<CefURLRequest> url_request = CefURLRequest::Create(
    request, new RequestClientImpl(request_callback), NULL);
  url_requests_[request_id] = url_request;
}

// Cancel the currently pending URL request, if any.
void URLRequestManager::CancelPendingRequests() {
  CEF_REQUIRE_UI_THREAD();

  CefURLRequestMap::iterator it = url_requests_.begin();
  for (; it != url_requests_.end(); ++it) {
    // Don't execute the callback when we explicitly cancel the URL request.
    static_cast<RequestClientImpl*>((it->second)->GetClient().get())->Detach();
    (it->second)->Cancel();
  }
  url_requests_.clear();
}

void URLRequestManager::OnRequestComplete(int request_id,
                                          Delegate* delegate,
                                          CefURLRequest::ErrorCode error_code,
                                          const std::string& download_data) {
  CEF_REQUIRE_UI_THREAD();

  CefURLRequestMap::iterator it = url_requests_.find(request_id);
  if (it != url_requests_.end())
    url_requests_.erase(it);

  if (delegate)
    delegate->OnRequestComplete(error_code, download_data);
}

}  // namespace client
