#include "tracker/browser/client_settings.h"

namespace client {

ClientSettings::ClientSettings(Delegate* delegate)
  : delegate_(delegate), maximized_(false) {}

ClientSettings::~ClientSettings() {}

const std::string& ClientSettings::GetProjectId() const {
  return project_id_;
}

void ClientSettings::SetProjectId(const std::string& project_id) {
  if (project_id_ == project_id)
    return;

  project_id_ = project_id;
  if (delegate_)
    delegate_->OnSettingsChanged();
}

bool ClientSettings::GetMaximized() const {
  return maximized_;
}

void ClientSettings::SetMaximized(bool maximized) {
  if (maximized_ == maximized)
    return;

  maximized_ = maximized;
  if (delegate_)
    delegate_->OnSettingsChanged();
}

const std::string& ClientSettings::GetClientId() const {
  return client_id_;
}

void ClientSettings::SetClientId(const std::string& client_id) {
  if (client_id_ == client_id)
    return;

  client_id_ = client_id;
  if (delegate_)
    delegate_->OnSettingsChanged();
}

}  // namespace client
