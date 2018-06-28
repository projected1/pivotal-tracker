#include "tracker/browser/client_settings.h"

namespace client {

ClientSettings::ClientSettings(Delegate* delegate)
  : delegate_(delegate) {}

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

}  // namespace client
