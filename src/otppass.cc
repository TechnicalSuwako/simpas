#include "base32.hh"
#include "otppass.hh"

#include <cstdio>
#include <fstream>
#include <gpgme++/context.h>
#include <gpgme++/data.h>
#include <gpgme++/decryptionresult.h>
#include <gpgme++/error.h>
#include <gpgme++/global.h>
#include <gpgme++/gpgmefw.h>
#include <gpgme++/key.h>

#include <iostream>
#include <openssl/hmac.h>
#include <openssl/sha.h>

#include <FL/fl_ask.H>

#include <ostream>
#include <stdexcept>
#include <vector>
#include <cstdint>
#include <exception>

#if defined(__APPLE)
#include <libkern/OSByteOrder.h>
#define htobe64(x) OSSwapHostToBigInt64(x)
#endif

std::vector<unsigned char> Otppass::extract_secret(const std::string &otpauth_url) {
  auto secret_start = otpauth_url.find("secret=");
  if (secret_start == std::string::npos) {
    fl_alert("OTPAuth URLの中に、シークレットを見つけられませんでした");
    throw std::runtime_error(
      "OTPAuth URLの中に、シークレットを見つけられませんでした"
    );
  }
  secret_start += 7;

  auto secret_end = otpauth_url.find('&', secret_start);
  if (secret_end == std::string::npos) {
    secret_end = otpauth_url.length();
  }

  if (secret_end < secret_start) {
    fl_alert("不正なシークレットの距離");
    throw std::runtime_error("不正なシークレットの距離");
  }

  std::string secret_encoded =
    otpauth_url.substr(secret_start, secret_end - secret_start);

  std::vector<unsigned char> secret_decoded;
  secret_decoded = Base32::decode(secret_encoded.c_str());

  if (secret_decoded.empty()) {
    fl_alert("BASE32の復号化に失敗");
    throw std::runtime_error("BASE32の復号化に失敗");
  }

  return secret_decoded;
}

#if defined(__HAIKU__)
uint64_t Otppass::htobe64(uint64_t counter) {
  uint64_t res = 0;
  uint8_t *dest = reinterpret_cast<uint8_t*>(&res);
  const uint8_t *src = reinterpret_cast<const uint8_t*>(&counter);

  for (int i = 0; i < 8; ++i) {
    dest[i] = src[7 - i];
  }

  return res;
}
#endif

uint32_t Otppass::generate_totp(const std::vector<unsigned char> &secret,
                                uint64_t counter) {
  counter = htobe64(counter);

  unsigned char hash[SHA_DIGEST_LENGTH];
  HMAC(
    EVP_sha1(),
    secret.data(),
    secret.size(),
    reinterpret_cast<unsigned char *>(&counter),
    sizeof(counter),
    hash,
    NULL
  );

  int offset = hash[SHA_DIGEST_LENGTH - 1] & 0x0F;
  uint32_t truncated_hash =
    (hash[offset] & 0x7F)     << 24 |
    (hash[offset + 1] & 0xFF) << 16 |
    (hash[offset + 2] & 0xFF) << 8 |
    (hash[offset + 3] & 0xFF);

  return truncated_hash % 1000000;
}

std::string Otppass::exec(std::string &secret) {
  try {
    std::vector<unsigned char> secret_decoded = extract_secret(secret);

    time_t current_time = time(nullptr);
    uint64_t counter = current_time / 30;
    uint32_t otp =
      generate_totp(secret_decoded, counter);

    char otpres[7];
    std::snprintf(otpres, sizeof(otpres), "%06u", otp);

    return std::string(otpres);
  } catch (const std::exception &e) {
    fl_alert("%s%s", "エラー: ", e.what());
    return "";
  }
}
