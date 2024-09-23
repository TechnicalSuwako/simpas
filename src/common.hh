#ifndef COMMON_HH
#define COMMON_HH

#include <vector>
#include <string>

#include <sys/stat.h>

class Common {
  public:
    static std::string getbasedir(bool trailing);
    static std::string getlang();
    int mkdir_r(const std::string &path, mode_t mode);
    void tmpcopy(const std::string &inpath, const std::string &outpath);
    static std::vector<std::string> explode(const std::string &str, char delimiter);
};

#endif
