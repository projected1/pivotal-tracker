#ifndef CEF_TESTS_CEFCLIENT_BROWSER_URL_REQUEST_MANAGER_H_
#define CEF_TESTS_CEFCLIENT_BROWSER_URL_REQUEST_MANAGER_H_
#pragma once

#include <map>

#include "include/cef_urlrequest.h"
#include "include/internal/cef_ptr.h"

namespace client {

class URLRequestManager {
 public:

  class Delegate {
   public:
    virtual void OnRequestComplete(CefURLRequest::ErrorCode error_code,
                                   const std::string& download_data) = 0;
  };

  URLRequestManager();
  ~URLRequestManager();

  // Issue a GET request and return immediately. Cache is ignored.
  void IssueRequest(const std::string& request_url, Delegate* delegate = NULL);

 private:
  // Cancel the currently pending URL URLRequestManager, if any.
  void CancelPendingRequests();

  void OnRequestComplete(int request_id,
                         Delegate* delegate,
                         CefURLRequest::ErrorCode error_code,
                         const std::string& download_data);

  typedef std::map<int, CefRefPtr<CefURLRequest>> CefURLRequestMap;
  CefURLRequestMap url_requests_;

  DISALLOW_COPY_AND_ASSIGN(URLRequestManager);
};

}  // namespace client

#endif  // CEF_TESTS_CEFCLIENT_BROWSER_URL_REQUEST_MANAGER_H_
