#ifndef COMMON_HH
#define COMMON_HH

#include <string>

#include <sys/stat.h>

class Common {
  public:
    static std::string getlang();
    int mkdir_r(const std::string &path, mode_t mode);
};

#endif
