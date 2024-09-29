# simpas
シンプルなパスワードマネージャー Simple Password Manager\
GUI版のsp

## インストールする方法 | Installation
### BSD
```sh
doas make depends
make
doas make install
```

### Void Linux
```sh
sudo bmake depends
bmake
sudo bmake install
```

### Alpine Linux
```sh
sudo apk add fltk fltk-dev gpgme gpgmepp gnupg pinentry bmake
bmake
sudo bmake install
```
