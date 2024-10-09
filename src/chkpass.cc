#include "common.hh"
#include "chkpass.hh"
#include "../main.hh"
#include "showpass.hh"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Scrollbar.H>

#include <string>
#include <thread>
#include <algorithm>

Chkpass chk;
std::vector<std::string> foundRes;
std::vector<std::string> duppath;
std::vector<std::string> duppasswd;

struct InputData {
  Fl_Radio_Button *all;
  Fl_Radio_Button *len;
  Fl_Radio_Button *chr;
  Fl_Radio_Button *dup;
  Fl_Window *dialog;
};

void Chkpass::lenPass(const std::string &path, const std::string &pass,
    const std::string &lang) {
  if (pass.length() > 0 && pass.length() < minimumlen) {
    std::string res = "【E】";
    res += (lang.compare(0, 2, "en") == 0 ?
        "The password \"" + path + "\" is too short, minimum length should be " +
          std::to_string(minimumlen) + " characters, recommended is " +
          std::to_string(recommendlen) + " characters.":
        "パスワード「" + path + "」は短すぎます。最短パスワードの長さは" +
          std::to_string(minimumlen) + "文字ですが、勧めが" +
          std::to_string(recommendlen) + "文字です。");

    foundRes.push_back(res);
    weaklencount++;
    vulncount++;
  } else if (pass.length() >= minimumlen && pass.length() < recommendlen) {
    std::string res = "【W】";
    res += (lang.compare(0, 2, "en") == 0 ?
        "The password \"" + path + "\" is long enough, but for optimal security, " +
          std::to_string(recommendlen) + " characters is recommended.\n" :
        "パスワード「" + path + "」の長さは十分ですが、最強のセキュリティには" +
          std::to_string(recommendlen) + "文字が勧めします。\n");

    foundRes.push_back(res);
  }
}

void Chkpass::charPass(const std::string &path, const std::string &pass,
    const std::string &lang) {
  bool isUpper = false;
  bool isLower = false;
  bool isDigit = false;
  bool isSpecial = false;

  for (char ch : pass) {
    if (std::isupper(static_cast<unsigned char>(ch))) isUpper = true;
    if (std::islower(static_cast<unsigned char>(ch))) isLower = true;
    if (std::isdigit(static_cast<unsigned char>(ch))) isDigit = true;
    if (std::find(spchar.begin(), spchar.end(), ch) != spchar.end()) isSpecial = true;
  }

  if (!isUpper || !isLower || !isDigit || !isSpecial) {
    std::string res = "【E】";
    res += (lang.compare(0, 2, "en") == 0 ?
        "The password \"" + path + "\" is too weak! A strong password contains " + 
          "at least 1 uppercase, 1 lowercase, 1 digit, and 1 special character." :
        "パスワード「" + path + "」は弱すぎます！強いパスワードは最大1大文字、" + 
          "1小文字、1数字、及び1記号の文字が含みます。");

    foundRes.push_back(res);
    weakcharcount++;
    vulncount++;
  }
}

void Chkpass::dupPass(const std::string &path, const std::string &pass,
    const std::string &lang) {
  for (std::size_t k = 0; k < duppasswd.size(); k++) {
    if (pass.compare(duppasswd[k]) == 0) {
      std::string res = "【E】";
      res += (lang.compare(0, 2, "en") == 0 ?
          "The password \"" + path + "\" is the same as \"" + duppath[k] + "\". " +
            "For security, please keep passwords unique!":
          "パスワード「" + path + "」は「" + duppath[k] + "」と一致しています。" +
            "セキュリティの為、各パスワードはユニークにする様にして下さい！");

      foundRes.push_back(res);
      duppasscount++;
      vulncount++;
    }
  }

  duppath.push_back(path);
  duppasswd.push_back(pass);
}

bool Chkpass::exec() {
  std::string lang = Common::getlang();

  // パスワードをスキャンして
  Showpass show;

  for (const auto &dispath : dispaths) {
    std::string fullpath = Common::getbasedir(true) + dispath + ".gpg";
    std::string pass = show.exec(fullpath.c_str(), true);
    if (pass.empty()) continue;
    if (pass.rfind("otpauth://totp/", 0) == 0) continue;

    if (isAll) {
      lenPass(dispath, pass, lang);
      charPass(dispath, pass, lang);
      dupPass(dispath, pass, lang);
    } else if (isLen) {
      lenPass(dispath, pass, lang);
    } else if (isChar) {
      charPass(dispath, pass, lang);
    } else if (isDup) {
      dupPass(dispath, pass, lang);
    }
  }

  return true;
}

void Chkpass::showRes() {
  std::string lang = Common::getlang();

  std::string res;
  if (lang.compare(0, 2, "en") == 0) {
    res = "Weak passwords:\n";
  } else {
    res = "不安定なパスワード：\n";
  }

  for (const auto &path : foundRes) {
    res += path + "\n";
  }

  if (lang.compare(0, 2, "en") == 0) {
    res += "Short password count: " + std::to_string(weaklencount) + "\n";
    res += "Weak password count: " + std::to_string(weakcharcount) + "\n";
    res += "Duplicate password count: " + std::to_string(duppasscount) + "\n";
    res += "Total: " + std::to_string(duppath.size()) + "\n";
    res += "It's advised to change any of the";
    res += "weak passwords as soon as possible!";
  } else {
    res += "短いパスワード数: " + std::to_string(weaklencount) + "\n";
    res += "弱いパスワード数: " + std::to_string(weakcharcount) + "\n";
    res += "同じパスワード数: " + std::to_string(duppasscount) + "\n";
    res += "合計: " + std::to_string(duppath.size()) + "\n";
    res += "不安定なパスワードは出来るだけ早く変更する事をお勧めします！";
  }

  Fl_Window *win = new Fl_Window(500, 440, lang.compare(0, 2, "en") == 0 ?
      "Results" : "結果");
  Fl_Text_Display *display = new Fl_Text_Display(10, 10, 480, 380);
  Fl_Text_Buffer *textbuf = new Fl_Text_Buffer();

  textbuf->text(res.c_str());
  display->buffer(textbuf);
  display->scrollbar_width(15);
  win->resizable(display);

  Fl_Button *okBtn = new Fl_Button(210, 400, 80, 30, "OK");
  okBtn->callback([](Fl_Widget *widget, void *win) {
    (void)widget;
    reinterpret_cast<Fl_Window*>(win)->hide();
  }, win);

  win->add(okBtn);

  win->end();
  win->show();
}

void Chkpass::chk_cb(Fl_Widget *, void *data) {
  InputData *inputs = (InputData *)data;
  std::string lang = Common::getlang();

  if (inputs) {
    isAll = inputs->all->value();
    isLen = inputs->len->value();
    isChar = inputs->chr->value();
    isDup = inputs->dup->value();
  }

  Fl_Window *dialog = new Fl_Window(400, 50,
      (lang.compare(0, 2, "en") == 0 ?
       "Checking for weak password" : "不安的なパスワードの確認中"));

  Fl_Box *box = new Fl_Box(10, 10, 380, 20,
      (lang.compare(0, 2, "en") == 0 ?
       "Checking, please wait for a while..." :
       "確認中。暫くお待ち下さい・・・"));

  dialog->add(box);

  dialog->end();
  dialog->set_modal();
  dialog->show();

  std::thread checker([dialog]() {
    chk.exec();

    Fl::lock();
    dialog->hide();
    chk.showRes();
    Fl::unlock();
  });

  checker.detach();
}

void Chkpass::dialog_cb(Fl_Widget *w, void *) {
  (void)w;

  std::string lang = Common::getlang();

  Fl_Window *dialog = new Fl_Window(390, 145,
      (lang.compare(0, 2, "en") == 0 ?
       "Check for weak password" : "不安的なパスワードの確認"));

  Fl_Radio_Button *all = new Fl_Radio_Button(10, 10, 180, 30,
      (lang.compare(0, 2, "en") == 0) ? "Everything" : "全部");
  Fl_Radio_Button *len = new Fl_Radio_Button(10, 50, 180, 30,
      (lang.compare(0, 2, "en") == 0) ? "Length" : "長さ");
  Fl_Radio_Button *chr = new Fl_Radio_Button(200, 10, 180, 30,
      (lang.compare(0, 2, "en") == 0) ? "Strength" : "強さ");
  Fl_Radio_Button *dup = new Fl_Radio_Button(200, 50, 180, 30,
      (lang.compare(0, 2, "en") == 0) ? "Duplicate" : "服数度");

  dialog->add(all);
  dialog->add(len);
  dialog->add(chr);
  dialog->add(dup);

  InputData *inputs = new InputData();
  inputs->all = all;
  inputs->len = len;
  inputs->chr = chr;
  inputs->dup = dup;
  inputs->dialog = dialog;

  Fl_Button *okbtn = new Fl_Button(105, 100, 80, 30, "OK");
  Fl_Button *cancelbtn = new Fl_Button(205, 100, 80, 30,
      (lang.compare(0, 2, "en") == 0 ? "Cancel" : "キャンセル"));

  okbtn->callback(static_ok_cb, inputs);
  cancelbtn->callback(static_cancel_cb, dialog);

  dialog->add(okbtn);
  dialog->add(cancelbtn);

  dialog->end();
  dialog->set_modal();
  dialog->show();
}

void Chkpass::static_ok_cb(Fl_Widget *w, void *data) {
  (void)w;
  InputData *inputs = (InputData *)data;

  chk.chk_cb(nullptr, inputs);
}

void Chkpass::static_cancel_cb(Fl_Widget *w, void *data) {
  ((Chkpass *)data)->cancel_cb(w, data);
}
