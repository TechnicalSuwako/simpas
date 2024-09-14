#ifndef INITPASS_HH
#define INITPASS_HH

#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>

#include <string>

#include "dialog.hh"

class Initpass : public Dialog {
  public:
    Fl_Button *btn = nullptr;

    void exec(const std::string &gpgid);
    static void dialog_cb(Fl_Widget *w, void *);
    static void init_cb(Fl_Widget *, void *user_data);

  private:
    Fl_Input *gpgid = nullptr;

    static void static_cancel_cb(Fl_Widget *w, void *data);
};

#endif
