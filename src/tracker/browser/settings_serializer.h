#ifndef CEF_TESTS_CEFCLIENT_BROWSER_SETTINGS_SERIALIZER_H_
#define CEF_TESTS_CEFCLIENT_BROWSER_SETTINGS_SERIALIZER_H_
#pragma once

#include "tracker/browser/client_settings.h"

namespace client {

class SettingsSerializer {
 public:
  SettingsSerializer();
  ~SettingsSerializer();

  void Serialize(const ClientSettings& settings);
  void Deserialize(ClientSettings& settings);

 private:
  std::string settings_file_;

  DISALLOW_COPY_AND_ASSIGN(SettingsSerializer);
};

}  // namespace client

#endif  // CEF_TESTS_CEFCLIENT_BROWSER_SETTINGS_SERIALIZER_H_
