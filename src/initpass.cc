#include "common.hh"
#include "initpass.hh"

#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>

#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include <sys/stat.h>

Initpass init;

void Initpass::exec(const std::string &gpgid) {
  std::string homedir = getenv("HOME");

#if defined(__HAIKU__)
  std::string basedir = "/config/settings/sp/";
#else
  std::string basedir = "/.local/share/sp/";
#endif
  std::string dirpath = std::string(homedir) + basedir;

  Common common;
  if (common.mkdir_r(dirpath, 075) != 0 && errno != EEXIST) {
    std::cout << dirpath << std::endl;
    fl_alert("ディレクトリを作成に失敗。");
    return;
  }

  std::string gpgidpath = dirpath + "/.gpg-id";

  struct stat statbuf;
  if (stat(gpgidpath.c_str(), &statbuf) == 0) {
    fl_alert(".gpg-idファイルは既に存在します。");
    return;
  }

  std::ofstream gpgidfile(gpgidpath);
  if (!gpgidfile.is_open()) {
    fl_alert(".gpg-idファイルを書き込めません。");
    return;
  }

  gpgidfile << gpgid + '\n';

  gpgidfile.close();

  fl_alert("初期設定に完了しました。");
}

void Initpass::init_cb(Fl_Widget *w, void *user_data) {
  init.exec(init.gpgid->value());
  // init.btn->deactivate(); // TODO: segfault
  static_cancel_cb(w, user_data);
}

void Initpass::dialog_cb(Fl_Widget *w, void *) {
  (void)w;
  Fl_Window *dialog = new Fl_Window(450, 120, "パスワードの初期設定");

  init.gpgid = new Fl_Input(90, 20, 300, 30, "gpg秘密鍵:");
  dialog->add(init.gpgid);

  Fl_Button *startbtn = new Fl_Button(185, 70, 80, 30, "開始");

  startbtn->callback(init_cb, dialog);

  dialog->add(startbtn);

  dialog->end();
  dialog->set_modal();
  dialog->show();
}

void Initpass::static_cancel_cb(Fl_Widget *w, void *data) {
  ((Initpass *)data)->cancel_cb(w, data);
}
