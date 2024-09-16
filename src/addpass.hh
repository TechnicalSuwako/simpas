#ifndef ADDPASS_HH
#define ADDPASS_HH

#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>

#include "dialog.hh"

#include <string>

class Addpass : public Dialog {
  public:
    Fl_Button *btn = nullptr;
    std::string file;
    std::string inputpass1;
    std::string inputpass2;

    void add_cb(Fl_Widget *, void *);
    static void dialog_cb(Fl_Widget *w, void *);
    bool exec(const std::string &file,
        const std::string &pass, const std::string &verify_pass);

  private:
    static void static_ok_cb(Fl_Widget *w, void *data);
    static void static_cancel_cb(Fl_Widget *w, void *data);
};

#endif
