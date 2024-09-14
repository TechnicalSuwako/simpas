#ifndef GENPASS_HH
#define GENPASS_HH

#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Output.H>

#include <iostream>

#include "dialog.hh"

class Genpass : public Dialog {
  public:
    Fl_Button *btn = nullptr;

    std::string genpass(int count, bool issecure);
    static void dialog_cb(Fl_Widget *w, void *);
    static void generate_cb(Fl_Widget *, void *user_data);

  private:
    Fl_Input *counter = nullptr;
    Fl_Check_Button *securechk = nullptr;
    Fl_Button *genbtn = nullptr;
    Fl_Output *res = nullptr;

    static void static_ok_cb(Fl_Widget *w, void *data);
    static void static_cancel_cb(Fl_Widget *w, void *data);
};

#endif
