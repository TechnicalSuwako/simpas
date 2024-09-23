#include "common.hh"
#include "delpass.hh"
#include "../main.hh"

#include <FL/fl_ask.H>
#include <FL/Fl_Input.H>

#include <unistd.h>

Delpass del;

struct InputData {
  Fl_Input *txtin;
  Fl_Window *dialog;
};

void Delpass::setFile(std::string &f) {
  del.file = f;
}

std::string Delpass::getFile() {
  return file;
}

bool Delpass::exec(const std::string &file, bool force) {
  std::string lang = Common::getlang();

  std::string basedir = Common::getbasedir(true);
  std::string ext = ".gpg";

  // ファイルが既に存在するかどうか確認
  if (access(file.c_str(), F_OK) != 0) {
    std::string err = (lang.compare(0, 2, "en") == 0 ?
        "Password does not exist" :
        "パスワードが存在しません");
    fl_alert("%s", err.c_str());
    return false;
  }

  if (unlink(file.c_str()) == -1) {
    std::string err = (lang.compare(0, 2, "en") == 0 ?
        "Password cannot be deleted" :
        "パスワードを削除出来ませんですた");
    fl_alert("%s", err.c_str());
    return false;
  }

  // 空のディレクトリの場合
  std::vector<std::string> tokens = Common::explode(file, '/');
  std::string passpath = basedir + tokens[0];

  for (size_t i = 1; i < tokens.size(); ++i) {
    if (i == tokens.size() - 1) continue;
    passpath += "/" + tokens[i];
  }

  for (int i = tokens.size() - 1; i >= 0; --i) {
    // ~/.local/share/sp を削除したら危険
    if (passpath.compare(0, basedir.size(), basedir) == 0) {
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

  if (force) return true;

  std::string msg = (lang.compare(0, 2, "en") == 0 ?
      "The password got deleted" :
      "パスワードを削除しました");
  fl_alert("%s", msg.c_str());
  return true;
}

void Delpass::delete_cb(Fl_Widget *, void *data) {
  InputData *inputs = (InputData *)data;

  if (inputs) {
    file = inputs->txtin->value();
  }

  exec(file, false);
  std::vector<std::string> fpaths;
  std::string rdir = Common::getbasedir(false);

  std::string mockpath = "";
  clearpaths(true, mockpath);
  scandir(rdir, rdir, fpaths);
  updatelist();
}

void Delpass::dialog_cb(Fl_Widget *w, void *data) {
  (void)w;
  (void)data;
  std::string lang = Common::getlang();

  Fl_Input *txtin = new Fl_Input(150, 20, 180, 30,
      (lang.compare(0, 2, "en") == 0 ? "Path:" : "パス:"));
  txtin->hide();
  txtin->value(del.getFile().c_str());

  InputData *inputs = new InputData();
  inputs->txtin = txtin;

  std::string asking =
    (lang.compare(0, 2, "en") == 0 ?
    "Are you sure you want to delete the password '" + del.getFile() + "'?" :
    "パスワード「" + del.getFile() + "」を本当に削除する事が宜しいでしょうか？");
  int confirm = fl_choice("%s",
    (lang.compare(0, 2, "en") == 0 ? "Cancel" : "キャンセル"),
    (lang.compare(0, 2, "en") == 0 ? "Delete" : "削除"),
    nullptr, asking.c_str());

  if (confirm == 0) {
    std::string err = (lang.compare(0, 2, "en") == 0 ?
        "Not deleted" :
        "削除しませんでした");
    fl_alert("%s", err.c_str());
  } else {
    static_ok_cb(w, inputs);
  }
}

void Delpass::static_ok_cb(Fl_Widget *w, void *data) {
  (void)w;
  InputData *inputs = (InputData *)data;

  del.delete_cb(nullptr, inputs);
  std::vector<std::string> fpaths;
  std::string rdir = Common::getbasedir(false);
  std::string curpath = "";
  clearpaths(true, curpath);
  scandir(rdir, rdir, fpaths);
  updatelist();
}

void Delpass::static_cancel_cb(Fl_Widget *w, void *data) {
  ((Delpass *)data)->cancel_cb(w, data);
}
