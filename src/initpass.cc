#include "common.hh"
#include "initpass.hh"

#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>

#include <cerrno>
#include <fstream>
#include <iostream>
#include <string>

#include <sys/stat.h>

Initpass init;

void Initpass::exec(const std::string &gpgid) {
  std::string lang = Common::getlang();
  std::string basedir = Common::getbasedir(true);

  Common common;
  if (common.mkdir_r(basedir, 0755) != 0 && errno != EEXIST) {
    std::cout << basedir << std::endl;
    std::string err = (lang.compare(0, 2, "en") == 0 ?
        "Failed to create directory." :
        "ディレクトリを作成に失敗。");
    fl_alert("%s", err.c_str());
    return;
  }

  std::string gpgidpath = basedir + "/.gpg-id";

  struct stat statbuf;
  if (stat(gpgidpath.c_str(), &statbuf) == 0) {
    std::string err = (lang.compare(0, 2, "en") == 0 ?
        ".gpg-id file already exists." :
        ".gpg-idファイルは既に存在します。");
    fl_alert("%s", err.c_str());
    return;
  }

  std::ofstream gpgidfile(gpgidpath);
  if (!gpgidfile.is_open()) {
    std::string err = (lang.compare(0, 2, "en") == 0 ?
        "Failed to write .gpg-id file." :
        ".gpg-idファイルを書き込めません。");
    fl_alert("%s", err.c_str());
    return;
  }

  gpgidfile << gpgid + '\n';

  gpgidfile.close();

  std::string msg = (lang.compare(0, 2, "en") == 0 ?
      "Initialization completed." :
      "初期設定に完了しました。");
  fl_alert("%s", msg.c_str());
}

void Initpass::init_cb(Fl_Widget *w, void *user_data) {
  init.exec(init.gpgid->value());
  // init.btn->deactivate(); // TODO: segfault
  static_cancel_cb(w, user_data);
}

void Initpass::dialog_cb(Fl_Widget *w, void *) {
  (void)w;
  std::string lang = Common::getlang();
  Fl_Window *dialog = new Fl_Window(450, 120,
      (lang.compare(0, 2, "en") == 0 ? "Initialize password" : "パスワードの初期設定"));

  init.gpgid = new Fl_Input(90, 20, 300, 30,
      (lang.compare(0, 2, "en") == 0 ? "GPG secret key:" : "gpg秘密鍵:"));
  dialog->add(init.gpgid);

  Fl_Button *startbtn = new Fl_Button(185, 70, 80, 30,
      (lang.compare(0, 2, "en") == 0 ? "Start" : "開始"));

  startbtn->callback(init_cb, dialog);

  dialog->add(startbtn);

  dialog->end();
  dialog->set_modal();
  dialog->show();
}

void Initpass::static_cancel_cb(Fl_Widget *w, void *data) {
  ((Initpass *)data)->cancel_cb(w, data);
}
