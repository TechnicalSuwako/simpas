#include "common.hh"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fstream>
#include <iostream>
#include <errno.h>

std::string Common::getbasedir(bool trailing) {
  std::string homedir = std::getenv("HOME") ? std::getenv("HOME") : "";
  if (homedir.empty()) return "";

#if defined(__HAIKU__)
  std::string basedir = "/config/settings/sp";
  std::string slash = "/";
#elif defined(_WIN32)
  std::string basedir = "\\AppData\\Local\\076\\sp";
  std::string slash = "\\";
#else
  std::string basedir = "/.local/share/sp";
  std::string slash = "/";
#endif

  return trailing ? (homedir + basedir + slash) : (homedir + basedir);
}

std::string Common::getlang() {
  const char *env = std::getenv("SP_LANG");
  std::string lang;

  if (env) lang = env;
  else lang = "ja";

  return lang;
}

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

void Common::tmpcopy(const std::string &inpath, const std::string &outpath) {
  std::ifstream src(inpath, std::ios::binary);
  std::ofstream dst(outpath, std::ios::binary);
}

std::vector<std::string> Common::explode(const std::string &str, char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  size_t start = 0, end = 0;

  while ((end = str.find(delimiter, start)) != std::string::npos) {
    tokens.push_back(str.substr(start, end - start));
    start = end + 1;
  }
  tokens.push_back(str.substr(start));

  return tokens;
}
