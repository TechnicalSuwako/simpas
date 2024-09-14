# sp-gui
GUIで使ってspをコントロール出来る

## インストールする方法 | Installation
### BSD
```sh
doas make depends
make
doas make install
```

### Void Linux
```sh
sudo make depends
make
sudo bmake install
```

### Alpine Linux
```sh
sudo apk add fltk fltk-dev
bmake
sudo bmake install
```
