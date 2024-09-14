#include "base32.hh"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <string>

int Base32::char_to_val(char c) {
  std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
  auto it = std::find(alphabet.begin(), alphabet.end(), c);
  return (it != alphabet.end()) ? std::distance(alphabet.begin(), it) : -1;
}

std::vector<unsigned char> Base32::decode(const std::string &encoded) {
  size_t encoded_len = encoded.length();
  size_t padding = 0;
  for (int i = encoded_len - 1; i >= 0 && encoded[i] == '='; --i) {
    ++padding;
  }

  size_t out_len = (encoded_len - padding) * 5 / 8;
  if (out_len == 0) return {};

  std::vector<unsigned char> decoded(out_len);

  int buffer = 0, bits_left = 0, count = 0;
  Base32 b32;
  for (size_t i = 0; i < encoded_len - padding; ++i) {
    int val = b32.char_to_val(encoded[i]);
    if (val < 0) {
      throw std::runtime_error("Base32エンコードした文字の中に不正な文字があります。");
    }

    buffer <<= 5;
    buffer |= val;
    bits_left += 5;

    if (bits_left >= 8) {
      decoded[count++] = static_cast<unsigned char>(buffer >> (bits_left - 8));
      bits_left -= 8;
    }
  }

  decoded.resize(count);
  return decoded;
}
