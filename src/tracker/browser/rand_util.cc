#include "tracker/browser/rand_util.h"
#include <ctime>
#include <iostream>
#include <random>
#include <string>

namespace client {
namespace rand_util {

namespace {

void RandBytes(void* output, size_t output_length) {
  char* output_ptr = static_cast<char*>(output);
  std::mt19937 prng((unsigned)time(nullptr));
  std::uniform_int_distribution<> distribution(0, 0xf);
  for (size_t i = 0; i < output_length; i++)
    output_ptr[i] = distribution(prng);
}

std::string RandomDataToGUIDString(const uint64_t bytes[2]) {
  const size_t kGUIDLength = 36U;
  char szGUID[kGUIDLength + 1] = { 0 };
  sprintf_s(szGUID, "%08x-%04x-%04x-%04x-%012llx",
    static_cast<unsigned int>(bytes[0] >> 32),
    static_cast<unsigned int>((bytes[0] >> 16) & 0x0000ffff),
    static_cast<unsigned int>(bytes[0] & 0x0000ffff),
    static_cast<unsigned int>(bytes[1] >> 48),
    bytes[1] & 0x0000ffff'ffffffffULL);
  return szGUID;
}

}  // namespace

// Generates a v4 UUID.
std::string GenerateGUID() {
  uint64_t sixteen_bytes[2];
  RandBytes(&sixteen_bytes, sizeof(sixteen_bytes));

  // Set the GUID to version 4 as described in RFC 4122, section 4.4.
  // The format of GUID version 4 must be xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx,
  // where y is one of [8, 9, A, B].
  // Clear the version bits and set the version to 4:
  sixteen_bytes[0] &= 0xffffffff'ffff0fffULL;
  sixteen_bytes[0] |= 0x00000000'00004000ULL;

  // Set the two most significant bits (bits 6 and 7) of the
  // clock_seq_hi_and_reserved to zero and one, respectively:
  sixteen_bytes[1] &= 0x3fffffff'ffffffffULL;
  sixteen_bytes[1] |= 0x80000000'00000000ULL;

  return RandomDataToGUIDString(sixteen_bytes);
}

}  // namespace rand_util
}  // namespace client
