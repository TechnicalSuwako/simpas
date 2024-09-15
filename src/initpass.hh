#ifndef INITPASS_HH
#define INITPASS_HH

#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>

#include <string>

#include "dialog.hh"

class Initpass : public Dialog {
  public:
    Fl_Button *btn = nullptr;
    Fl_Input *gpgid = nullptr;

    void exec(const std::string &gpgid);
};

#endif
