#ifndef DELPASS_HH
#define DELPASS_HH

#include <FL/Fl_Button.H>
#include <string>

#include "dialog.hh"

class Delpass : public Dialog {
  public:
    Fl_Button *btn = nullptr;
    std::string file;

    static void dialog_cb(Fl_Widget *w, void *);
    void delete_cb(Fl_Widget *, void *data);
    static void setFile(std::string &f);
    std::string getFile();
    bool exec(const std::string &file, bool force);

  private:
    static void static_ok_cb(Fl_Widget *w, void *data);
    static void static_cancel_cb(Fl_Widget *w, void *data);
};

#endif
