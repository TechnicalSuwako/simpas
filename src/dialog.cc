#include "dialog.hh"

#include <FL/Fl_Window.H>

void Dialog::ok_cb(Fl_Widget *w, void *dialog) {
  (void)w;
  ((Fl_Window *)dialog)->hide();
}

void Dialog::cancel_cb(Fl_Widget *w, void *dialog) {
  (void)w;
  ((Fl_Window *)dialog)->hide();
}
