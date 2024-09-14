#ifndef OTPPASS_HH
#define OTPPASS_HH

#include <cstdint>
#include <string>
#include <vector>

class Otppass {
  public:
    std::string exec(std::string &file);

  private:
    std::vector<unsigned char> extract_secret(const std::string &otpauth_url);
#if defined(__HAIKU__)
    uint64_t htobe64(uint64_t counter);
#endif
    uint32_t generate_totp(const std::vector<unsigned char> &secret,
                           uint64_t counter);
};

#endif
