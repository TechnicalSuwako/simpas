#ifndef MAIN_HH
#define MAIN_HH

#include <vector>
#include <string>

extern std::vector<std::string> fullpaths;
extern std::vector<std::string> dispaths;
extern int browseId;
extern bool isPassHidden;
extern std::string realpass;

class Fl_Text_Display;
class Fl_Text_Buffer;

extern Fl_Text_Display *textview;
extern Fl_Text_Buffer *textbuf;

void browse(std::string &path, bool isNew);
void clearpaths(bool isReset, std::string &path);
void updatelist();
void scandir(const std::string& dir, const std::string& root, std::vector<std::string>& paths);

#endif
