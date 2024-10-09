#ifndef SHOWPASS_HH
#define SHOWPASS_HH

#include <gpgme++/context.h>
#include <gpgme++/data.h>

#include <fstream>

class Showpass {
  public:
    std::string otpSav = "";

    static void otpupdate_cb(void *o);
    std::string exec(const char *file, bool stfu);

  private:
    void clean_up(GpgME::Context &ctx, GpgME::Data &in, GpgME::Data &out,
              std::ifstream &gpgfile);
};

#endif
