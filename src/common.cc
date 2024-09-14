#include "common.hh"

#include <cstdio>
#include <cstring>

#include <errno.h>

int Common::mkdir_r(const std::string &path, mode_t mode) {
  char tmp[256];
  size_t len;

  std::snprintf(tmp, sizeof(tmp), "%s", path.c_str());

  len = std::strlen(tmp);
  if (tmp[len - 1] == '/') {
    tmp[len - 1] = 0;
  }

  for (char *p = tmp + 1; *p; p++) {
    if (*p == '/') {
      *p = 0;
      if (mkdir(tmp, mode) != 0 && errno != EEXIST) return -1;
      *p = '/';
    }
  }

  if (mkdir(tmp, mode) != 0 && errno != EEXIST) {
    return -1;
  }

  return 0;
}
