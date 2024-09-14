#ifndef BASE32_HH
#define BASE32_HH

#include <vector>
#include <string>

class Base32 {
  public:
    static std::vector<unsigned char> decode(const std::string &encoded);

  private:
    int char_to_val(char c);
};

#endif
