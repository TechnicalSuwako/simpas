#ifndef COMMON_HH
#define COMMON_HH

#include <string>

#include <sys/stat.h>

class Common {
  public:
    static std::string getbasedir(bool trailing);
    static std::string getlang();
    int mkdir_r(const std::string &path, mode_t mode);
    void tmpcopy(const std::string &inpath, const std::string &outpath);
};

#endif
