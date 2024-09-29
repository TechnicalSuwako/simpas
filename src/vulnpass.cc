#include "common.hh"
#include "vulnpass.hh"
#include "showpass.hh"
#include "../main.hh"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Scrollbar.H>

#include <vector>
#include <string>
#include <thread>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

Vulnpass vuln;
int vulncount = 0;
std::vector<std::string> vulnpaths;

bool Vulnpass::exec() {
  std::string lang = Common::getlang();

  // pwndサーバに接続
  int sock;
  struct sockaddr_in srv;
  struct addrinfo hints, *addr;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  int status = getaddrinfo("076.moe", nullptr, &hints, &addr);
  if (status != 0) {
    fl_alert("getaddrinfo: %s", gai_strerror(status));
    return false;
  }

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    fl_alert(lang.compare(0, 2, "en") == 0 ?
        "Failed to create socket" : "ソケットを作成に失敗");
    return false;
  }

  srv.sin_addr = ((struct sockaddr_in *)(addr->ai_addr))->sin_addr;
  srv.sin_family = AF_INET;
  srv.sin_port = htons(9951);

  freeaddrinfo(addr);

  if (connect(sock, (struct sockaddr *)&srv, sizeof(srv)) < 0) {
    fl_alert(lang.compare(0, 2, "en") == 0 ?
        "Failed to connect" : "接続に失敗");
    close(sock);
    return false;
  }

  // パスワードをスキャンして
  Showpass show;

  for (const auto &dispath : dispaths) {
    std::string fullpath = Common::getbasedir(true) + dispath + ".gpg";
    std::string pass = show.exec(fullpath.c_str(), true);
    if (pass.empty()) continue;

    if (send(sock, pass.c_str(), pass.length(), 0) < 0) {
      fl_alert(lang.compare(0, 2, "en") == 0 ?
          "Failed to send" : "送信に失敗");
      close(sock);
      return false;
    }

    char res[256] = {0};
    int reslen = recv(sock, res, sizeof(res) - 1, 0);
    if (reslen < 0) {
      fl_alert(lang.compare(0, 2, "en") == 0 ?
          "Failed to retrieve" : "受取に失敗");
      close(sock);
      return false;
    }

    res[reslen] = '\0';
    std::string response(res);

    if (response.compare(0, 1, "0") != 0) {
      vulnpaths.push_back(dispath);
      vulncount++;
    }

    close(sock);

    // 再接続
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
      fl_alert(lang.compare(0, 2, "en") == 0 ?
          "Failed to create socket" : "ソケットを作成に失敗");
      return false;
    }

    if (connect(sock, (struct sockaddr *)&srv, sizeof(srv)) < 0) {
      fl_alert(lang.compare(0, 2, "en") == 0 ?
          "Failed to reconnect" : "再接続に失敗");
      close(sock);
      return false;
    }
  }

  close(sock);

  return true;
}

void Vulnpass::showRes() {
  std::string lang = Common::getlang();

  std::string res;
  if (lang.compare(0, 2, "en") == 0) {
    res = "Breached passwords:\n";
  } else {
    res = "漏洩したパスワード：\n";
  }

  for (const auto &path : vulnpaths) {
    res += path + "\n";
  }

  if (lang.compare(0, 2, "en") == 0) {
    res += "Total: " + std::to_string(vulnpaths.size()) + "\n";
    res += "It's advised to change any of the";
    res += "breached passwords as soon as possible!";
  } else {
    res += "合計: " + std::to_string(vulnpaths.size()) + "\n";
    res += "漏洩したパスワードは出来るだけ早く変更する事をお勧めします！";
  }

  Fl_Window *win = new Fl_Window(500, 440, lang.compare(0, 2, "en") == 0 ?
      "Results" : "結果");
  Fl_Text_Display *display = new Fl_Text_Display(10, 10, 480, 380);
  Fl_Text_Buffer *textbuf = new Fl_Text_Buffer();

  textbuf->text(res.c_str());
  display->buffer(textbuf);
  display->scrollbar_width(15);
  win->resizable(display);

  Fl_Button *okBtn = new Fl_Button(210, 400, 80, 30, "OK");
  okBtn->callback([](Fl_Widget *widget, void *win) {
    reinterpret_cast<Fl_Window*>(win)->hide();
  }, win);

  win->add(okBtn);

  win->end();
  win->show();
}

void Vulnpass::dialog_cb(Fl_Widget *w, void *) {
  (void)w;

  std::string lang = Common::getlang();

  Fl_Window *dialog = new Fl_Window(400, 50,
      (lang.compare(0, 2, "en") == 0 ?
       "Check for branched password" : "漏洩されたパスワードの確認"));

  Fl_Box *box = new Fl_Box(10, 10, 380, 20,
      (lang.compare(0, 2, "en") == 0 ?
       "Checking, please wait for a while..." :
       "確認中。暫くお待ち下さい・・・"));

  dialog->add(box);

  dialog->end();
  dialog->set_modal();
  dialog->show();

  std::thread checker([dialog]() {
    vuln.exec();

    Fl::lock();
    dialog->hide();
    vuln.showRes();
    Fl::unlock();
  });

  checker.detach();
}
