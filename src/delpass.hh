#ifndef DELPASS_HH
#define DELPASS_HH

#include <FL/Fl_Button.H>
#include <string>
#include <vector>

#include "dialog.hh"

class Delpass : public Dialog {
  public:
    Fl_Button *btn = nullptr;

    bool exec(const std::string &file, bool force);

  private:
    int cnt(const std::string &str, char delimiter);
    std::vector<std::string> explode(const std::string &str, char delimiter);
};

#endif
