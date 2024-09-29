#include "common.hh"
#include "showpass.hh"
#include "otppass.hh"

#include <gpgme++/decryptionresult.h>

#undef None
#include <FL/fl_ask.H>

std::string Showpass::exec(const char *file, bool stfu) {
  std::string lang = Common::getlang();

  try {
    std::setlocale(LC_ALL, "");
    GpgME::initializeLibrary();

    gpg_error_t err = gpg_err_init();
    if (err) {
      if (!stfu) {
        std::string ero = (lang.compare(0, 2, "en") == 0 ?
            "Failed to generate GPGME" :
            "GPGエラーの設置に失敗");
        fl_alert("%s: %s", ero.c_str(), gpg_strerror(err));
      }
      return "";
    }

    std::unique_ptr<GpgME::Context> ctx =
      GpgME::Context::create(GpgME::Protocol::OpenPGP);
    if (!ctx) {
      if (!stfu) {
        std::string ero = (lang.compare(0, 2, "en") == 0 ?
            "Failed to create GPGME context" :
            "GPGMEコンテキストの作成に失敗");
        fl_alert("%s", ero.c_str());
      }
      return "";
    }

    if (ctx->protocol() != GpgME::Protocol::OpenPGP) {
      if (!stfu) {
        std::string ero = (lang.compare(0, 2, "en") == 0 ?
          "Failed to set OpenPGP protocol" :
          "OpenPGPプロトコールの設置に失敗");
        fl_alert("%s", ero.c_str());
      }
      return "";
    }

    std::ifstream gpgfile(file, std::ios::binary);
    if (!gpgfile.is_open()) {
      if (!stfu) {
        std::string ero = (lang.compare(0, 2, "en") == 0 ?
            "Unable to open the specified file" :
            "指定されたファイルを開けません");
        fl_alert("%s", ero.c_str());
      }
      return "";
    }

    GpgME::Data in(file);
    GpgME::Data out;

    GpgME::DecryptionResult res = ctx->decrypt(in, out);
    if (res.error()) {
      if (!stfu) {
        std::string ero = (lang.compare(0, 2, "en") == 0 ?
            "Failed to decrypt" :
            "復号化に失敗");
        fl_alert("%s: %s", ero.c_str(), res.error().asString());
      }
      return "";
    }

    std::string dec;
    char buf[512];
    ssize_t read_bytes;

    out.seek(0, SEEK_SET);

    while ((read_bytes = out.read(buf, sizeof(buf))) > 0) {
      dec.append(buf, read_bytes);
    }

    if (dec.rfind("otpauth://", 0) == 0 && !stfu) {
      Otppass o;
      return o.exec(dec);
    }

    return dec;
  } catch (const std::exception &e) {
    if (!stfu) {
      std::string err = (lang.compare(0, 2, "en") == 0 ?
          "Error" :
          "エラー");
      fl_alert("%s: %s", err.c_str(), e.what());
    }
    return "";
  }
}
