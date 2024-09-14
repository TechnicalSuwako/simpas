#include "showpass.hh"

#include <clocale>
#include <gpgme++/context.h>
#include <gpgme++/data.h>
#include <gpgme++/decryptionresult.h>
#include <gpgme++/error.h>
#include <gpgme++/global.h>
#include <gpgme++/gpgmefw.h>
#include <gpgme++/key.h>
#include <memory>

#define None X11_None
#include <FL/fl_ask.H>
#undef None

#include <iostream>
#include <fstream>
#include <string>
#include <exception>

std::string Showpass::exec(const char *file) {
  try {
    std::setlocale(LC_ALL, "");
    GpgME::initializeLibrary();

    gpg_error_t err = gpg_err_init();
    if (err) {
      fl_alert("GPGエラーの設置に失敗: %s", gpg_strerror(err));
      return "";
    }

    std::unique_ptr<GpgME::Context> ctx =
      GpgME::Context::create(GpgME::Protocol::OpenPGP);

    std::ifstream gpgfile(file, std::ios::binary);
    std::cout << file << std::endl;
    if (!gpgfile.is_open()) {
      fl_alert("指定されたファイルを開けません");
      return "";
    }

    GpgME::Data in(file);
    GpgME::Data out;

    GpgME::DecryptionResult res = ctx->decrypt(in, out);
    if (res.error()) {
      fl_alert("復号化に失敗: %s", res.error().asString());
      return "";
    }

    std::string dec;
    char buf[512];
    ssize_t read_bytes;

    out.seek(0, SEEK_SET);

    while ((read_bytes = out.read(buf, sizeof(buf))) > 0) {
      dec.append(buf, read_bytes);
    }

    return dec;
  } catch (const std::exception &e) {
    fl_alert("%s%s", "エラー: ", e.what());
    return "";
  }
}
