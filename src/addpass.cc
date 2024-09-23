#include "common.hh"
#include "addpass.hh"
#include "../main.hh"

#include <gpgme++/context.h>
#include <gpgme++/data.h>
#include <gpgme++/encryptionresult.h>

#undef None
#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>

#include <fstream>

#include <unistd.h>

Addpass add;

struct InputData {
  Fl_Input *txtin;
  Fl_Secret_Input *pass1;
  Fl_Secret_Input *pass2;
  Fl_Window *dialog;
};

bool Addpass::exec(const std::string &file, const std::string &pass, bool isEdit) {
  std::string lang = Common::getlang();

  std::string basedir = Common::getbasedir(true);
  std::string ext = ".gpg";

  std::string gpgoutfile = (isEdit ? file : basedir + file + ext);

  if (access(gpgoutfile.c_str(), F_OK) != -1) {
    std::string err = (lang.compare(0, 2, "en") == 0 ?
        "Password already exist." :
        "パスワードが既に存在しています。");
    fl_alert("%s", err.c_str());
    return false;
  }

  try {
    // GPGMEライブラリを設置
    std::setlocale(LC_ALL, "");
    GpgME::initializeLibrary();
    GpgME::setDefaultLocale(LC_CTYPE, std::setlocale(LC_CTYPE, NULL));

    GpgME::Error err;
    if (err) {
      std::string ero = (lang.compare(0, 2, "en") == 0 ?
          "Failed to generate GPGME" :
          "GPGエラーの設置に失敗");
      fl_alert("%s: %s", ero.c_str(), gpg_strerror(err.code()));
      return false;
    }

    // GPGMEを創作
    std::unique_ptr<GpgME::Context> ctx =
      GpgME::Context::create(GpgME::Protocol::OpenPGP);

    // GPGMEは非対話的モードに設定
    err = ctx->setPinentryMode(GpgME::Context::PinentryMode::PinentryLoopback);
    if (err) {
      std::string ero = (lang.compare(0, 2, "en") == 0 ?
          "Failed to set pinentry mode" :
          "pinentryモードを設定に失敗");
      fl_alert("%s: %s", ero.c_str(), gpg_strerror(err.code()));
      return false;
    }

    // パスワードからデータオブジェクトを創作
    GpgME::Data in(pass.c_str(), strlen(pass.c_str()), false);
    GpgME::Data out;

    // 鍵を受け取る
    std::string keypath = basedir + ".gpg-id";
    std::ifstream keyfile(keypath, std::ios::binary);
    if (!keyfile.is_open()) {
      std::string ero = (lang.compare(0, 2, "en") == 0 ?
          "Failed to open .gpg-id file" :
          ".gpg-idファイルを開くに失敗");
      fl_alert("%s", ero.c_str());
      return false;
    }

    std::string keyid;
    std::string line;
    while (std::getline(keyfile, line)) {
      line.erase(line.find_last_not_of(" \n\r\t") + 1);
      if (!line.empty()) keyid = line;
    }

    keyfile.close();

    if (keyid.empty()) {
      std::string ero = (lang.compare(0, 2, "en") == 0 ?
          "The .gpg-id file is empty or invalid" :
          ".gpg-idファイルは空か無効です");
      fl_alert("%s", ero.c_str());
      return false;
    }

    GpgME::Key key = ctx->key(keyid.c_str(), err);
    if (key.isNull()) {
      std::string ero = (lang.compare(0, 2, "en") == 0 ?
          "Failed to get key" :
          "鍵を受取に失敗");
      fl_alert("%s", ero.c_str());
      return false;
    }

    // 暗号化
    std::vector<GpgME::Key> keys = {key};

    GpgME::EncryptionResult res =
      ctx->encrypt(keys, in, out, GpgME::Context::AlwaysTrust);
    if (res.error()) {
      std::string ero = (lang.compare(0, 2, "en") == 0 ?
          "Failed to encrypt" :
          "暗号化に失敗");
      fl_alert("%s: %s", ero.c_str(), res.error().asString());
      return false;
    }

    // ディレクトリを創作
    std::string dirpath = (isEdit ? file : basedir + file);
    auto lastsla = dirpath.find_last_of('/');
    if (lastsla != std::string::npos) {
      dirpath = dirpath.substr(0, lastsla);

      try {
        Common common;
        common.mkdir_r(dirpath, 0755);
      } catch (const std::runtime_error &e) {
         std::string ero = (lang.compare(0, 2, "en") == 0 ?
             "Failed to create directory" :
             "ディレクトリを創作に失敗");
         fl_alert("%s: %s", ero.c_str(), e.what());
         return false;
      }
    }

    // 暗号化したファイルを開く
    std::ofstream gpgpath(gpgoutfile, std::ios::binary);
    if (!gpgpath.is_open()) {
      std::string ero = (lang.compare(0, 2, "en") == 0 ?
          "Failed to write file '" + gpgoutfile + "'." :
          "「" + gpgoutfile + "」ファイルを書き込めません。");
      fl_alert("%s", ero.c_str());
      return false;
    }

    // データが保存したかどうか確認
    ssize_t encrypted_data_size = out.seek(0, SEEK_END);
    if (encrypted_data_size <= 0) {
      std::string ero = (lang.compare(0, 2, "en") == 0 ?
          "Failed to store the data" :
          "データを保存に失敗");
      fl_alert("%s", ero.c_str());
      return false;
    }

    // 復号化したパスワードを表示する
    out.seek(0, SEEK_SET);

    char buffer[512];
    ssize_t read_bytes;
    while ((read_bytes = out.read(buffer, sizeof(buffer))) > 0) {
      gpgpath.write(buffer, read_bytes);
    }

    gpgpath.close();

    if (gpgpath.fail()) {
      std::string ero = (lang.compare(0, 2, "en") == 0 ?
          "Failed to write encrypted data to file" :
          "暗号化データを書き込めません。");
      fl_alert("%s", ero.c_str());
      return false;
    }
  } catch (const std::exception &e) {
    std::string err = (lang.compare(0, 2, "en") == 0 ?
        "Error" :
        "エラー");
    fl_alert("%s: %s", err.c_str(), e.what());
    return false;
  }

  if (isEdit) return true;

  std::string msg = (lang.compare(0, 2, "en") == 0 ?
      "The password got saved." :
      "パスワードを保存出来ました");
  fl_alert("%s", msg.c_str());

  return true;
}

void Addpass::add_cb(Fl_Widget *, void *data) {
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

    if (exec(file, inputpass1, false)) {
      inputs->dialog->hide();
    }
  } else {
    std::string err =
      (lang.compare(0, 2, "en") == 0 ?
       "Please fill in all the fields." :
       "全てのフィールドをご入力下さい。");
    fl_alert("%s", err.c_str());
  }
}

void Addpass::dialog_cb(Fl_Widget *w, void *) {
  (void)w;
  std::string lang = Common::getlang();
  Fl_Window *dialog = new Fl_Window(400, 200,
      (lang.compare(0, 2, "en") == 0 ? "Add password" : "パスワードの追加"));

  Fl_Input *txtin = new Fl_Input(150, 20, 180, 30,
      (lang.compare(0, 2, "en") == 0 ? "Path:" : "パス:"));
  dialog->add(txtin);

  Fl_Secret_Input *pass1 = new Fl_Secret_Input(150, 60, 180, 30,
      (lang.compare(0, 2, "en") == 0 ? "Password:" : "パスワード:"));
  Fl_Secret_Input *pass2 = new Fl_Secret_Input(150, 100, 180, 30,
      (lang.compare(0, 2, "en") == 0 ? "Password (confirm):" : "パスワード (確認):"));
  dialog->add(pass1);
  dialog->add(pass2);

  InputData *inputs = new InputData();
  inputs->txtin = txtin;
  inputs->pass1 = pass1;
  inputs->pass2 = pass2;
  inputs->dialog = dialog;

  Fl_Button *okbtn = new Fl_Button(60, 150, 80, 30, "OK");
  Fl_Button *cancelbtn = new Fl_Button(160, 150, 80, 30,
      (lang.compare(0, 2, "en") == 0 ? "Cancel" : "キャンセル"));

  okbtn->callback(static_ok_cb, inputs);
  cancelbtn->callback(static_cancel_cb, dialog);

  dialog->add(okbtn);
  dialog->add(cancelbtn);

  dialog->end();
  dialog->set_modal();
  dialog->show();
}

void Addpass::static_ok_cb(Fl_Widget *w, void *data) {
  (void)w;
  InputData *inputs = (InputData *)data;

  add.add_cb(nullptr, inputs);
  std::vector<std::string> fpaths;
  std::string rdir = Common::getbasedir(false);
  std::string curpath = rdir + "/" + inputs->txtin->value() + ".gpg";
  clearpaths(false, curpath);
  scandir(rdir, rdir, fpaths);
  updatelist();
  browse(curpath, true);
}

void Addpass::static_cancel_cb(Fl_Widget *w, void *data) {
  ((Addpass *)data)->cancel_cb(w, data);
}
