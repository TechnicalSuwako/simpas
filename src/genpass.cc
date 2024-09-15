#include "common.hh"
#include "genpass.hh"

#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>

#include <fstream>
#include <cstdlib>
#include <vector>

Genpass gen;

std::string Genpass::exec(int count, bool issecure) {
  std::string lang = Common::getlang();

  const std::string charset_risky =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  const std::string charset_secure =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!\"#$%&'()=~-^\\|_@`[{]};:+*<>,./?";
  const std::string &charset = issecure ? charset_secure : charset_risky;

  std::ifstream fp("/dev/random", std::ios_base::binary);
  if (!fp.is_open()) {
    std::string err = (lang.compare(0, 2, "en") == 0 ?
        "Could not open /dev/random" :
        "/dev/randomを開けられませんでした");
    fl_alert("%s", err.c_str());
    return "";
  }

  std::vector<char> password(count + 1);
  for (int i = 0; i < count; ++i) {
    unsigned char key;
    fp.read(reinterpret_cast<char *>(&key), sizeof(key));
    if (!fp) {
      std::string err = (lang.compare(0, 2, "en") == 0 ?
          "Could not read /dev/random" :
          "/dev/randomから読み込みに失敗");
      fl_alert("%s", err.c_str());
      fp.close();
      std::exit(EXIT_FAILURE);
    }

    password[i] = charset[key % charset.size()];
  }

  password[count] = '\0';
  fp.close();

  return std::string(password.data());
}

void Genpass::generate_cb(Fl_Widget *, void *) {
  int count = std::stoi(gen.counter->value());
  bool issecure = gen.securechk->value() > 0;

  std::string password = gen.exec(count, issecure);
  gen.res->value(password.c_str());
}

void Genpass::dialog_cb(Fl_Widget *w, void *) {
  (void)w;
  std::string lang = Common::getlang();
  Fl_Window *dialog = new Fl_Window(450, 250,
      (lang.compare(0, 2, "en") == 0 ? "Generate password" : "パスワードの作成"));

  gen.counter = new Fl_Input(120, 20, 100, 30,
      (lang.compare(0, 2, "en") == 0 ? "Length:" : "長さ:"));
  gen.counter->type(FL_INT_INPUT);
  gen.counter->value("64");
  dialog->add(gen.counter);

  gen.securechk = new Fl_Check_Button(120, 70, 150, 30,
      (lang.compare(0, 2, "en") == 0 ? "Secure?" : "安全化？"));
  gen.securechk->value(1);
  dialog->add(gen.securechk);

  gen.genbtn = new Fl_Button(120, 110, 150, 30,
      (lang.compare(0, 2, "en") == 0 ? "Generate" : "作成"));
  gen.genbtn->callback(generate_cb);
  dialog->add(gen.genbtn);

  gen.res = new Fl_Output(120, 150, 300, 30,
      (lang.compare(0, 2, "en") == 0 ? "Password:" : "パスワード:"));
  gen.res->value("");
  dialog->add(gen.res);

  Fl_Button *okbtn = new Fl_Button(60, 200, 80, 30, "OK");
  Fl_Button *cancelbtn = new Fl_Button(160, 200, 80, 30,
      (lang.compare(0, 2, "en") == 0 ? "Cancel" : "キャンセル"));

  okbtn->callback(static_ok_cb, dialog);
  cancelbtn->callback(static_cancel_cb, dialog);

  dialog->add(okbtn);
  dialog->add(cancelbtn);

  dialog->end();
  dialog->set_modal();
  dialog->show();
}

void Genpass::static_ok_cb(Fl_Widget *w, void *data) {
  ((Genpass *)data)->ok_cb(w, data);
}

void Genpass::static_cancel_cb(Fl_Widget *w, void *data) {
  ((Genpass *)data)->cancel_cb(w, data);
}
