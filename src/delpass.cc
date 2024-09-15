#include "common.hh"
#include "delpass.hh"

#include <FL/fl_ask.H>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <vector>

#include <unistd.h>
#include <dirent.h>

int Delpass::cnt(const std::string &str, char delimiter) {
  return std::count(str.begin(), str.end(), delimiter);
}

std::vector<std::string> Delpass::explode(const std::string &str, char delimiter) {
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

int Delpass::exec(const std::string &file, bool force) {
  std::string lang = Common::getlang();

  std::string homedir = getenv("HOME") ? getenv("HOME") : "";
  if (homedir.empty()) {
    std::string err = (lang.compare(0, 2, "en") == 0 ?
        "Failed to get home directory" :
        "ホームディレクトリを受取に失敗");
    fl_alert("%s", err.c_str());
    return -1;
  }

#if defined(__HAIKU__)
  std::string basedir = "/config/settings/sp/";
#else
  std::string basedir = "/.local/share/sp/";
#endif
  std::string ext = ".gpg";

  // ファイルが既に存在するかどうか確認
  if (access(file.c_str(), F_OK) != 0) {
    std::string err = (lang.compare(0, 2, "en") == 0 ?
        "Password does not exist" :
        "パスワードが存在しません");
    fl_alert("%s", err.c_str());
    return -1;
  }

  // 削除を確認する
  if (!force) { // パスワードの変更の場合、確認は不要
    std::string asking =
      (lang.compare(0, 2, "en") == 0 ?
      "パスワード「" + file + "」を本当に削除する事が宜しいでしょうか？" :
      "Are you sure you want to delete the password '" + file + "'?");
    int confirm = fl_choice("%s",
      (lang.compare(0, 2, "en") == 0 ? "Cancel" : "キャンセル"),
      (lang.compare(0, 2, "en") == 0 ? "Delete" : "削除"),
      nullptr, asking.c_str());

    if (confirm == 0) {
      std::string err = (lang.compare(0, 2, "en") == 0 ?
          "Not deleted" :
          "削除しませんでした");
      fl_alert("%s", err.c_str());
      return -1;
    }
  }

  if (unlink(file.c_str()) == -1) {
    std::string err = (lang.compare(0, 2, "en") == 0 ?
        "Password cannot be deleted" :
        "パスワードを削除出来ませんですた");
    fl_alert("%s", err.c_str());
    return -1;
  }

  // 空のディレクトリの場合
  std::vector<std::string> tokens = explode(file, '/');
  std::string basepath = homedir + basedir;
  std::string passpath = basepath + tokens[0];

  for (size_t i = 1; i < tokens.size(); ++i) {
    if (i == tokens.size() - 1) continue;
    passpath += "/" + tokens[i];
  }

  for (int i = tokens.size() - 1; i >= 0; --i) {
    // ~/.local/share/sp を削除したら危険
    if (passpath.compare(0, basedir.size(), basepath) == 0) {
      break;
    }

    // ディレクトリが空じゃない場合、削除を止める
    if (rmdir(passpath.c_str()) == -1) {
      break;
    }

    size_t last_slash = passpath.find_last_of('/');
    if (last_slash != std::string::npos) {
      passpath.erase(last_slash);
    }
  }

  if (force) return 0;

  std::string msg = (lang.compare(0, 2, "en") == 0 ?
      "The password got deleted" :
      "パスワードを削除しました");
  fl_alert("%s", msg.c_str());
  return 0;
}
