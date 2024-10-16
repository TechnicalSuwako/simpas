#include "src/addpass.hh"
#include "src/delpass.hh"
#include "src/editpass.hh"
#include "src/genpass.hh"
#include "src/initpass.hh"
#include "src/showpass.hh"
#include "src/vulnpass.hh"
#include "src/chkpass.hh"
#include "src/common.hh"
#include "main.hh"

#undef Status
#undef None
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Copy_Surface.H>
#include <FL/Fl_PNG_Image.H>

#include <dirent.h>
#include <sys/stat.h>

#include <string>
#include <vector>

Fl_Select_Browser *browser = nullptr;
Fl_Text_Display *textview = nullptr;
Fl_Text_Buffer *textbuf = nullptr;
Fl_Input *searchfield = nullptr;
Fl_Check_Button *hidechk = nullptr;
Fl_Button *showbtn = nullptr;

Addpass a;
Chkpass c;
Delpass d;
Editpass e;
Genpass g;
Initpass i;
Showpass s;
Vulnpass v;

const char *sofname = "simpas";
const char *intname = "SimPas";
const char *version = "1.1.0";
const char *basedof = "sp-1.5.0";

std::vector<std::string> fullpaths;
std::vector<std::string> dispaths;
std::vector<std::string> filterpaths;
int browseId;
bool isPassHidden = true;
std::string realpass = "";

void browse(std::string &path, bool isNew) {
  std::string cont = s.exec(path.c_str(), false);
  realpass = cont;

  if (isNew) browseId = browser->size();
  browser->value(browseId);

  if (!cont.empty()) {
    if (isPassHidden) {
      std::string lang = Common::getlang();
      if (lang.compare(0, 2, "en") == 0) {
        textbuf->text("(Hidden, please click the \"show\" button to reveal)");
      } else {
        textbuf->text("（非表示、「表示」ボタンをクリックして確認して下さい）");
      }
      showbtn->activate();
      showbtn->label((Common::getlang().compare(0, 2, "en") == 0) ?
          "Show password" : "パスワードの表示");
    } else {
      textbuf->text(cont.c_str());
      showbtn->label((Common::getlang().compare(0, 2, "en") == 0) ?
          "Hide password" : "パスワードを隠す");
      showbtn->activate();}
    Editpass::setFile(path);
    Delpass::setFile(path);
    d.btn->activate();
    e.btn->activate();
  }
}

void hide_cb(Fl_Widget *w, void *) {
  isPassHidden = ((Fl_Check_Button *)w)->value();
  int idx = browser->value();

  if (isPassHidden && idx > 0) {
    showbtn->activate();
    showbtn->label((Common::getlang().compare(0, 2, "en") == 0) ?
        "Show password" : "パスワードの表示");
  } else {
    showbtn->deactivate();
    showbtn->label((Common::getlang().compare(0, 2, "en") == 0) ?
        "Hide password" : "パスワードを隠す");
  }

  showbtn->label((Common::getlang().compare(0, 2, "en") == 0) ?
      "Show password" : "パスワードの表示");

  if (idx > 0) {
    std::string path = filterpaths[idx - 1];
    browse(path, false);
  }
}

void clearpaths(bool isReset, std::string &path) {
  fullpaths.clear();
  dispaths.clear();
  if (isReset) {
    std::string mockpath = "";
    Editpass::setFile(mockpath);
    Delpass::setFile(mockpath);
    d.btn->deactivate();
    e.btn->deactivate();
  }
}

void updatelist() {
  browser->clear();
  filterpaths.clear();
  std::string searchtxt = searchfield->value();
  for (size_t i = 0; i < dispaths.size(); ++i) {
    if (dispaths[i].find(searchtxt) != std::string::npos) {
      browser->add(dispaths[i].c_str());
      filterpaths.push_back(fullpaths[i]);
    }
  }
  textbuf->text("");
}

void search_cb(Fl_Widget *, void *) {
  std::string mockpath = "";
  Editpass::setFile(mockpath);
  Delpass::setFile(mockpath);
  d.btn->deactivate();
  e.btn->deactivate();
  updatelist();
}

void copy_cb(Fl_Widget *, void *) {
  if (!realpass.empty()) {
    Fl::copy(realpass.c_str(), realpass.length(), 1, Fl::clipboard_plain_text);
  } else {
    Fl::copy("", 0, 1, Fl::clipboard_plain_text);
  }
}

void browser_cb(Fl_Widget *w, void *) {
  (void)w;
  int idx = browser->value();
  if (idx == 0) return;
  browseId = idx;

  std::string path = filterpaths[idx - 1];
  browse(path, false);
}

void show_cb(Fl_Widget *, void *) {
  int idx = browser->value();
  if (idx == 0) return;
  std::string path = filterpaths[idx - 1];

  if (isPassHidden) {
    std::string cont = s.exec(path.c_str(), false);
    realpass = cont;
    textbuf->text(cont.c_str());

    std::string lang = Common::getlang();
    if (lang.compare(0, 2, "en") == 0) {
      showbtn->label("Hide password");
    } else {
      showbtn->label("パスワードを隠す");
    }
    isPassHidden = false;
  } else {
    std::string lang = Common::getlang();
    if (lang.compare(0, 2, "en") == 0) {
      textbuf->text("(Hidden, please click the \"show\" button to reveal)");
      showbtn->label("Show password");
    } else {
      textbuf->text("（非表示、「表示」ボタンをクリックして確認して下さい）");
      showbtn->label("パスワードの表示");
    }
    isPassHidden = true;
  }
}

void scandir(const std::string &dpath, const std::string &rpath,
             std::vector<std::string> &fpaths) {
  DIR *dir = opendir(dpath.c_str());
  if (!dir) return;

  struct dirent *entry;
  while ((entry = readdir(dir)) != nullptr) {
    std::string name = entry->d_name;
    if (name == "." || name == ".." || name == ".gpg-id") continue;

    std::string fpath = std::string(dpath) + "/" + name;
    struct stat s;
    if (stat(fpath.c_str(), &s) != 0) {
      closedir(dir);
      return;
    }

    if (S_ISDIR(s.st_mode)) {
      scandir(fpath, rpath, fpaths);
    } else if (name.find(".gpg") != std::string::npos) {
      std::string rel = fpath.substr(rpath.size() + 1);
      fpaths.push_back(rel);
      fullpaths.push_back(fpath);

      std::string disname = rel.substr(0, rel.rfind(".gpg"));
      dispaths.push_back(disname);
    }
  }

  closedir(dir);
}

void init_cb(Fl_Widget *w, void *data) {
  i.exec(i.gpgid->value());
  i.btn->deactivate();
  ((Initpass *)data)->cancel_cb(w, data);
}

void init_dialog_cb(Fl_Widget *w, void *) {
  (void)w;
  std::string lang = Common::getlang();
  Fl_Window *dialog = new Fl_Window(450, 120,
      (lang.compare(0, 2, "en") == 0 ?
       "Initialize password" : "パスワードの初期設定"));

  i.gpgid = new Fl_Input(90, 20, 300, 30,
      (lang.compare(0, 2, "en") == 0 ? "GPG secret key:" : "gpg秘密鍵:"));
  dialog->add(i.gpgid);

  Fl_Button *startbtn = new Fl_Button(185, 70, 80, 30,
      (lang.compare(0, 2, "en") == 0 ? "Start" : "開始"));

  startbtn->callback(init_cb, dialog);

  dialog->add(startbtn);

  dialog->end();
  dialog->set_modal();
  dialog->show();
}

void set_dark_theme() {
  Fl::background(35, 32, 35);
  Fl::background2(68, 59, 68);
  Fl::foreground(252, 252, 252);
}

int main(int argc, char **argv) {
  std::string lang = Common::getlang();
  std::string windowtit = std::string(intname) + " " + version;
  Fl_Window *window = new Fl_Window(790, 740, windowtit.c_str());

  set_dark_theme();

#if defined(__linux)
  const char *iconPath = "/usr/share/icons/076/512x512/simpas.png";
#else
  const char *iconPath = "/usr/local/share/icons/076/512x512/simpas.png";
#endif

  Fl_PNG_Image *icon = new Fl_PNG_Image(iconPath);

  window->icon(icon);

  searchfield = new Fl_Input(
      (lang.compare(0, 2, "en") == 0 ? 70 : 50), 10,
      (lang.compare(0, 2, "en") == 0 ? 710 : 730), 30,
      (lang.compare(0, 2, "en") == 0 ? "Search:" : "検索:"));
  searchfield->callback(search_cb);

  browser = new Fl_Select_Browser(10, 50, 380, 500);
  textview = new Fl_Text_Display(400, 50, 380, 500);
  textbuf = new Fl_Text_Buffer();
  textview->buffer(textbuf);

  browser->callback(browser_cb);

  Fl_Button *copybtn = new Fl_Button(400, 600, 150, 30,
      (lang.compare(0, 2, "en") == 0 ? "Copy password" : "パスワードのコピー"));
  copybtn->callback(copy_cb);

  a.btn = new Fl_Button(10, 560, 150, 30,
      (lang.compare(0, 2, "en") == 0 ? "Add password" : "パスワードの追加"));
  a.btn->callback(a.dialog_cb);

  d.btn = new Fl_Button(10, 600, 150, 30,
      (lang.compare(0, 2, "en") == 0 ? "Delete password" : "パスワードの削除"));
  d.btn->deactivate();
  d.btn->callback(d.dialog_cb);

  e.btn = new Fl_Button(400, 560, 150, 30,
    (lang.compare(0, 2, "en") == 0 ? "Edit password" : "パスワードの編集"));
  e.btn->deactivate();
  e.btn->callback(e.dialog_cb);

  hidechk = new Fl_Check_Button(560, 560, 150, 30,
    (lang.compare(0, 2, "en") == 0 ? "Hide password" : "パスワードを隠す"));
  hidechk->set();
  hidechk->callback(hide_cb);

  showbtn = new Fl_Button(560, 600, 150, 30,
    (lang.compare(0, 2, "en") == 0 ? "Show password" : "パスワードの表示"));
  showbtn->deactivate();
  showbtn->callback(show_cb);

  g.btn = new Fl_Button(10, 640, 150, 30,
      (lang.compare(0, 2, "en") == 0 ? "Generate password" : "パスワードの作成"));
  g.btn->callback(g.dialog_cb);

  i.btn = new Fl_Button(10, 680, 150, 30,
      (lang.compare(0, 2, "en") == 0 ?
       "Initialize password" :
       "パスワードの初期設定"));

  v.btn = new Fl_Button(170, 560, 200, 30,
      (lang.compare(0, 2, "en") == 0 ? "Check for breach" : "漏洩されたかの確認"));
  v.btn->callback(v.dialog_cb);

  c.btn = new Fl_Button(170, 600, 200, 30,
      (lang.compare(0, 2, "en") == 0 ?
       "Check for unsafe passwords" : "不安定的なパスワードの確認"));
  c.btn->callback(c.dialog_cb);

  std::string gpgidpath = Common::getbasedir(true) + ".gpg-id";

  struct stat buf;
  if (stat(gpgidpath.c_str(), &buf) == 0) {
    i.btn->deactivate();
  }
  i.btn->callback(init_dialog_cb);

  std::string bothver = windowtit + " (" + std::string(basedof) + ")";
  Fl_Box *versionlabel = new Fl_Box(FL_NO_BOX, 620, 700, 160, 30, bothver.c_str());
  versionlabel->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);

  std::vector<std::string> fpaths;
  std::string rdir = Common::getbasedir(false);
  scandir(rdir, rdir, fpaths);
  updatelist();

  window->end();
  window->show(argc, argv);

  return Fl::run();
}
