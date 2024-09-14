#ifndef DIALOG_HH
#define DIALOG_HH

#include <FL/Fl_Widget.H>

class Dialog {
  public:
    static void ok_cb(Fl_Widget *w, void *dialog);
    static void cancel_cb(Fl_Widget *w, void *dialog);
};

#endif
