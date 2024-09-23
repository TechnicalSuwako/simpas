#ifndef EDITPASS_HH
#define EDITPASS_HH

#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>

#include "dialog.hh"

#include <string>

class Editpass : public Dialog {
  public:
    Fl_Button *btn = nullptr;
    std::string file;
    std::string inputpass1;
    std::string inputpass2;

    static void dialog_cb(Fl_Widget *w, void *);
    void edit_cb(Fl_Widget *, void *data);
    static void setFile(std::string &f);
    std::string getFile();
    bool exec(const std::string &file, const std::string &pass);

  private:
    static void static_ok_cb(Fl_Widget *w, void *data);
    static void static_cancel_cb(Fl_Widget *w, void *data);
};

#endif
