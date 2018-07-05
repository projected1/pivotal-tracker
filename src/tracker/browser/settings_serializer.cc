#include <fstream>

#include "tracker/browser/main_context.h"
#include "tracker/browser/settings_serializer.h"

namespace client {

namespace {

const char kSettingsFilename[] = "settings.dat";

template <typename T>
void SerializePrimitive(std::ofstream& ofile, const T& t) {
  ofile.write(reinterpret_cast<const char*>(&t), sizeof(t));
}

template <typename T>
void DeserializePrimitive(std::ifstream& ifile, T& t) {
  T tmp;
  ifile.read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
  t = ifile.fail() ? 0 : tmp;
}

void SerializeString(std::ofstream& ofile, const std::string& str) {
  auto size = str.size();
  ofile.write(reinterpret_cast<char*>(&size), sizeof(size));
  if (!ofile.fail() && size > 0)
    ofile.write(str.data(), size);
}

void DeserializeString(std::ifstream& ifile, std::string& str) {
  size_t size { 0 };
  ifile.read(reinterpret_cast<char*>(&size), sizeof(size));
  if (!ifile.fail() && size > 0) {
    str.resize(size);
    ifile.read(reinterpret_cast<char*>(&str.front()), size);
  }
}

}  // namespace

SettingsSerializer::SettingsSerializer() {
  settings_file_ =
    MainContext::Get()->GetAppWorkingDirectory() + kSettingsFilename;
}

SettingsSerializer::~SettingsSerializer() {
}

///
// Serializes |settings| into a file.
// Serialization order matters:
//  * Do not change the order of serialized elements
//  * Do not remove previously serialized elements
///
void SettingsSerializer::Serialize(const ClientSettings& settings) {
  auto ofstream_deleter = [](std::ofstream * ofile) { ofile->close(); };
  std::unique_ptr<std::ofstream, decltype(ofstream_deleter)> ofile(
    new std::ofstream(
      settings_file_, std::ios::binary | std::ofstream::trunc),
    ofstream_deleter);
  if (!ofile->is_open())
    return;

  SerializeString(*ofile, settings.project_id_);
  SerializePrimitive(*ofile, settings.maximized_);
  SerializeString(*ofile, settings.client_id_);
}

///
// Deserializes |settings| from a file.
// Deserialization order matters:
//  * Do not change the order of deserialized elements
//  * Do not remove previously deserialized elements
///
void SettingsSerializer::Deserialize(ClientSettings& settings) {
  auto ifstream_deleter = [](std::ifstream * ifile) { ifile->close(); };
  std::unique_ptr<std::ifstream, decltype(ifstream_deleter)> ifile(
    new std::ifstream(settings_file_, std::ios::binary),
    ifstream_deleter);
  if (!ifile->is_open())
    return;

  DeserializeString(*ifile, settings.project_id_);
  DeserializePrimitive(*ifile, settings.maximized_);
  DeserializeString(*ifile, settings.client_id_);
}

}  // namespace client
