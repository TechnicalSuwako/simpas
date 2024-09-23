#include "src/addpass.hh"
#include "src/delpass.hh"
#include "src/editpass.hh"
#include "src/genpass.hh"
#include "src/initpass.hh"
#include "src/showpass.hh"
#include "src/common.hh"

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
#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Copy_Surface.H>

#include <dirent.h>
#include <sys/stat.h>

#include <iostream> // REMOVE
#include <string>
#include <vector>

Fl_Select_Browser *browser = nullptr;
Fl_Text_Display *textview = nullptr;
Fl_Text_Buffer *textbuf = nullptr;
Fl_Input *searchfield = nullptr;

Addpass a;
Delpass d;
Editpass e;
Genpass g;
Initpass i;
Showpass s;

const char *sofname = "simpas";
const char *version = "1.0.0";
const char *basedof = "sp-1.4.0";

std::vector<std::string> fullpaths;
std::vector<std::string> dispaths;
std::vector<std::string> filterpaths;

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
  d.btn->deactivate();
  e.btn->deactivate();
  updatelist();
}

void copy_cb(Fl_Widget *, void *) {
  const char *text = textbuf->text();
  if (text && *text) {
    Fl::copy(text, strlen(text), 1, Fl::clipboard_plain_text);
  }
}

void browser_cb(Fl_Widget *w, void *) {
  (void)w;
  int idx = browser->value();
  if (idx == 0) return;

  std::string path = filterpaths[idx - 1];
  std::string cont = s.exec(path.c_str());

  if (!cont.empty()) {
    textbuf->text(cont.c_str());
    Editpass::setFile(path);
    d.btn->activate();
    e.btn->activate();
  }
}

void scandir(const std::string &dpath, const std::string &rpath,
             std::vector<std::string> &fpaths) {
  DIR *dir = opendir(dpath.c_str());
  if (!dir) return;

  struct dirent *entry;
  while ((entry = readdir(dir)) != nullptr) {
    std::string name = entry->d_name;
    if (name == "." || name == "..") continue;

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

void delete_cb(Fl_Widget *, void *) {
  int idx = browser->value();
  if (idx == 0) return;

  std::string path = filterpaths[idx - 1];
  int res = d.exec(path, false);
  if (res == 0) {
    d.btn->deactivate();
    e.btn->deactivate();
    std::vector<std::string> fpaths;
    std::string rdir = Common::getbasedir(false);

    fullpaths.clear();
    dispaths.clear();
    scandir(rdir, rdir, fpaths);
    updatelist();
  }
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
  std::string windowtit = std::string(sofname) + " " + version;
  Fl_Window *window = new Fl_Window(790, 740, windowtit.c_str());

  set_dark_theme();

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
  d.btn->callback(delete_cb);

  e.btn = new Fl_Button(400, 560, 150, 30,
    (lang.compare(0, 2, "en") == 0 ? "Edit password" : "パスワードの編集"));
  e.btn->deactivate();
  e.btn->callback(e.dialog_cb);

  g.btn = new Fl_Button(10, 640, 150, 30,
      (lang.compare(0, 2, "en") == 0 ? "Generate password" : "パスワードの作成"));
  g.btn->callback(g.dialog_cb);

  i.btn = new Fl_Button(10, 680, 150, 30,
      (lang.compare(0, 2, "en") == 0 ?
       "Initialize password" :
       "パスワードの初期設定"));

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
