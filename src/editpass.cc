#include "common.hh"
#include "editpass.hh"
#include "delpass.hh"
#include "addpass.hh"

#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>

#include <unistd.h>

Editpass edit;

struct InputData {
  Fl_Input *txtin;
  Fl_Secret_Input *pass1;
  Fl_Secret_Input *pass2;
};

void Editpass::setFile(std::string &f) {
  edit.file = f;
}

std::string Editpass::getFile() {
  return file;
}

bool Editpass::exec(const std::string &file, const std::string &pass) {
  std::string lang = Common::getlang();

  Common c;
  c.tmpcopy(file, "/tmp/simpas-tmp.gpg");

  Delpass d;
  bool isdel = d.exec(file, true);
  if (!isdel) {
    std::string err =
      (lang.compare(0, 2, "en") == 0 ? "Failed to edit." : "編集に失敗。");
    fl_alert("%s", err.c_str());
    return false;
  }

  Addpass a;
  bool isadd = a.exec(file, pass, true);
  if (!isadd) {
    std::string err =
      (lang.compare(0, 2, "en") == 0 ? "Failed to edit." : "編集に失敗。");
    fl_alert("%s", err.c_str());
    c.tmpcopy("/tmp/simpas-tmp.gpg", file);
    unlink("/tmp/simpas-tmp.gpg");
    return false;
  }

  std::string msg =
    (lang.compare(0, 2, "en") == 0 ? "Edit success." : "編集に成功。");
  fl_alert("%s", msg.c_str());
  unlink("/tmp/simpas-tmp.gpg");

  return true;
}

void Editpass::edit_cb(Fl_Widget *, void *data) {
  InputData *inputs = (InputData *)data;
  std::string lang = Common::getlang();

  if (inputs) {
    file = inputs->txtin->value();
    if (file.empty()) {
      std::string err =
        (lang.compare(0, 2, "en") == 0 ?
         "Please fill in the path." :
         "パスをご入力下さい。");
      fl_alert("%s", err.c_str());
      return;
    }
    inputpass1 = inputs->pass1->value();
    if (inputpass1.empty()) {
      std::string err =
        (lang.compare(0, 2, "en") == 0 ?
         "Please fill in the password." :
         "パスワードをご入力下さい。");
      fl_alert("%s", err.c_str());
      return;
    }
    inputpass2 = inputs->pass2->value();
    if (inputpass2.empty()) {
      std::string err =
        (lang.compare(0, 2, "en") == 0 ?
         "Please fill in the password (confirm)." :
         "パスワード (確認)をご入力下さい。");
      fl_alert("%s", err.c_str());
      return;
    }

    if (inputpass1 != inputpass2) {
      std::string err =
        (lang.compare(0, 2, "en") == 0 ?
         "Password does not match." :
         "パスワードが一致していません。");
      fl_alert("%s", err.c_str());
      return;
    }

    exec(file, inputpass1);
  } else {
    std::string err =
      (lang.compare(0, 2, "en") == 0 ?
       "Please fill in all the fields." :
       "全てのフィールドをご入力下さい。");
    fl_alert("%s", err.c_str());
  }
}

void Editpass::dialog_cb(Fl_Widget *w, void *data) {
  (void)w;
  (void)data;
  std::string lang = Common::getlang();
  Fl_Window *dialog = new Fl_Window(400, 160,
      (lang.compare(0, 2, "en") == 0 ? "Edit password" : "パスワードの編集"));

  Fl_Input *txtin = new Fl_Input(150, 20, 180, 30,
      (lang.compare(0, 2, "en") == 0 ? "Path:" : "パス:"));
  Fl_Secret_Input *pass1 = new Fl_Secret_Input(150, 20, 180, 30,
      (lang.compare(0, 2, "en") == 0 ? "Password:" : "パスワード:"));
  Fl_Secret_Input *pass2 = new Fl_Secret_Input(150, 60, 180, 30,
      (lang.compare(0, 2, "en") == 0 ? "Password (confirm):" : "パスワード (確認):"));
  dialog->add(pass1);
  dialog->add(pass2);
  txtin->hide();
  txtin->value(edit.getFile().c_str());

  InputData *inputs = new InputData();
  inputs->txtin = txtin;
  inputs->pass1 = pass1;
  inputs->pass2 = pass2;

  Fl_Button *okbtn = new Fl_Button(60, 110, 80, 30, "OK");
  Fl_Button *cancelbtn = new Fl_Button(160, 110, 80, 30,
      (lang.compare(0, 2, "en") == 0 ? "Cancel" : "キャンセル"));

  okbtn->callback(static_ok_cb, inputs);
  cancelbtn->callback(static_cancel_cb, dialog);

  dialog->add(okbtn);
  dialog->add(cancelbtn);

  dialog->end();
  dialog->set_modal();
  dialog->show();
}

void Editpass::static_ok_cb(Fl_Widget *w, void *data) {
  (void)w;
  InputData *inputs = (InputData *)data;

  edit.edit_cb(nullptr, inputs);
}

void Editpass::static_cancel_cb(Fl_Widget *w, void *data) {
  ((Editpass *)data)->cancel_cb(w, data);
}
