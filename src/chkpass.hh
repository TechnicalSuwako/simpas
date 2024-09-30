#ifndef CHKPASS_HH
#define CHKPASS_HH

#include <FL/Fl_Button.H>
#include <FL/Fl_Radio_Button.H>

#include "dialog.hh"

#include <string>

class Chkpass : public Dialog {
  public:
    Fl_Button *btn = nullptr;
    Fl_Radio_Button *allChk = nullptr;
    Fl_Radio_Button *lenChk = nullptr;
    Fl_Radio_Button *charChk = nullptr;
    Fl_Radio_Button *dupChk = nullptr;
    bool isAll, isLen, isChar, isDup = false;

    static void dialog_cb(Fl_Widget *w, void *);
    void showRes();
    void chk_cb(Fl_Widget *, void *);
    bool exec();

  private:
    int vulncount = 0;
    int weaklencount = 0;
    int weakcharcount = 0;
    int duppasscount = 0;

    std::size_t minimumlen = 12;
    std::size_t recommendlen = 64;

    std::string spchar = "!@#$%^&*()-_=+[]{}|;:'\",.<>?/\\`~";

    void lenPass(const std::string &path, const std::string &pass,
        const std::string &lang);
    void charPass(const std::string &path, const std::string &pass,
        const std::string &lang);
    void dupPass(const std::string &path, const std::string &pass,
        const std::string &lang);
    static void static_ok_cb(Fl_Widget *w, void *data);
    static void static_cancel_cb(Fl_Widget *w, void *data);
};
#endif
