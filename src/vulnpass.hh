#ifndef VULNPASS_HH
#define VULNPASS_HH

#include <FL/Fl_Button.H>

#include "dialog.hh"

class Vulnpass : public Dialog {
  public:
    Fl_Button *btn = nullptr;

    static void dialog_cb(Fl_Widget *w, void *);
    void vuln_cb(Fl_Widget *, void *);
    bool exec();
    void showRes();
};

#endif
