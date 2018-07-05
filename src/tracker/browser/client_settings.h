#ifndef CEF_TESTS_CEFCLIENT_BROWSER_CLIENT_SETTINGS_H_
#define CEF_TESTS_CEFCLIENT_BROWSER_CLIENT_SETTINGS_H_
#pragma once

#include <string>

#include "include/cef_base.h"

namespace client {

class ClientSettings : public CefBaseRefCounted {
 public:
  class Delegate {
   public:
    virtual void OnSettingsChanged() = 0;
  };

  ClientSettings(Delegate* delegate = nullptr);
  ~ClientSettings();

  const std::string& GetProjectId() const;
  void SetProjectId(const std::string& project_id);

  bool GetMaximized() const;
  void SetMaximized(bool maximized);

  const std::string& GetClientId() const;
  void SetClientId(const std::string& client_id);

 private:
  friend class SettingsSerializer;

  Delegate* delegate_;

  std::string project_id_;
  bool maximized_;
  std::string client_id_;

  IMPLEMENT_REFCOUNTING(ClientSettings);
  DISALLOW_COPY_AND_ASSIGN(ClientSettings);
};

}  // namespace client

#endif  // CEF_TESTS_CEFCLIENT_BROWSER_CLIENT_SETTINGS_H_
