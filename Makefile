UNAME_S != uname -s
UNAME_M != uname -m
OS = ${UNAME_S}
ARCH = ${UNAME_M}

.if ${UNAME_S} == "OpenBSD"
OS = openbsd
.elif ${UNAME_S} == "NetBSD"
OS = netbsd
.elif ${UNAME_S} == "FreeBSD"
OS = freebsd
.elif ${UNAME_S} == "Dragonfly"
OS = dragonfly
.elif ${UNAME_S} == "Linux"
OS = linux
.endif

.if ${UNAME_M} == "x86_64"
ARCH = amd64
.endif

NAME != cat main.cc | grep "const char \*sofname" | awk '{print $$5}' |\
	sed "s/\"//g" | sed "s/;//"
VERSION != cat main.cc | grep "const char \*version" | awk '{print $$5}' |\
	sed "s/\"//g" | sed "s/;//"

PREFIX = /usr/local
.if ${OS} == "linux"
PREFIX = /usr
.endif

CC = c++
FILES = main.cc src/*.cc

.if ${OS} == "openbsd"
DEPS = pkg_add fltk gpgme gpgme-qt gnupg pinentry
.elif ${OS} == "netbsd"
DEPS = pkgin install fltk gpgme gpgmepp gnupg pinentry pinentry-fltk
.elif ${OS} == "freebsd"
DEPS = pkg install fltk gpgme gpgme-cpp gnupg pinentry pinentry-fltk
.elif ${OS} == "linux"
DEPS = xbps-install fltk fltk-devel gpgme gpgmepp gpgmepp-devel gnupg pinentry bmake
.endif

CFLAGS = -Wall -Wextra -Wno-non-c-typedef-for-linkage -Wno-unused-parameter\
				 -Wno-cast-function-type\
				 -I/usr/include -L/usr/lib
.if ${OS} == "freebsd" || ${OS} == "openbsd" || ${OS} == "netbsd" || ${OS} == "dragonfly
CFLAGS += -I/usr/local/include -L/usr/local/lib
.endif
.if ${OS} == "netbsd"
CFLAGS += -I/usr/X11R7/include -L/usr/X11R7/lib -I/usr/pkg/include -L/usr/pkg/lib
.elif ${OS} == "openbsd"
CFLAGS += -I/usr/X11R6/include -L/usr/X11R6/lib
.endif

LDFLAGS = -lfltk_images -lfltk -lX11 -lassuan -lgpgmepp -lgpgme -lcrypto -lgpg-error

.if ${OS} == "openbsd"
LDFLAGS += -lc++abi -lpthread -lm -lc\
			 -lXcursor -lXfixes -lXext -lXft -lfontconfig -lXinerama -lXdmcp -lXau\
			 -lz -lxcb -lXrender -lexpat -lfreetype -lc++ -lintl -liconv -lpng -ljpeg
.elif ${OS} == "freebsd"
LDFLAGS += -lcxxrt -lm -lXrender -lXcursor -lXfixes -lXext -lXft -lfontconfig\
			 -lXinerama -lthr -lz -lxcb -lfreetype -lexpat -lXau -lXdmcp\
			 -lbz2 -lbrotlidec -lbrotlicommon -lc++ -lgcc -lc -lgpgme -lassuan\
			 -lintl -lpng16
.elif ${OS} == "netbsd"
LDFLAGS += -lstdc++ -lpthread -lm -lc -lXft -lxcb -lfontconfig -lfreetype\
			 -lXau -lXdmcp -lXcursor -lXrandr -lXext -lXrender -lXfixes -lXinerama -lX11\
			 -lexpat -lz -lbz2 -lgcc -lassuan -lintl
.elif ${OS} == "linux"
LDFLAGS += -lstdc++ -lgcc -lc -lXft -lXext -lXrender -lfontconfig -lXinerama\
			 -lxcb -lfreetype -lpng16 -lz -lexpat -lXau -lXdmcp -lbz2\
			 -lbrotlidec -lbrotlicommon -lassuan
.endif

all:
	${CC} -O3 ${CFLAGS} -o ${NAME}\
		${FILES} -static ${LDFLAGS}
	strip ${NAME}

depend:
	${DEPS}

debug:
	${CC} -g ${CFLAGS} -o ${NAME} ${FILES} ${LDFLAGS}

clean:
	rm -rf ${NAME}

dist:
	mkdir -p ${NAME}-${VERSION} release/src release/desktop
	cp -R LICENSE.txt Makefile README.md CHANGELOG.md logo.png\
		main.cc src icons ${NAME}.desktop ${NAME}-${VERSION}
	tar zcfv release/src/${NAME}-${VERSION}.tar.gz ${NAME}-${VERSION}
	cp ${NAME}.desktop release/desktop
	rm -rf ${NAME}-${VERSION}

release:
	mkdir -p release/bin/${VERSION}/${OS}/${ARCH}
	${CC} -O3 ${CFLAGS} -o release/bin/${VERSION}/${OS}/${ARCH}/${NAME} ${FILES}\
		-static ${LDFLAGS}
	strip release/bin/${VERSION}/${OS}/${ARCH}/${NAME}

publish:
	rsync -rtvzP release/bin/${VERSION} 192.168.0.143:/zroot/repo/bin/${NAME}
	rsync -rtvzP release/src/* 192.168.0.143:/zroot/repo/src/${NAME}
	rsync -rtvzP release/desktop/* 192.168.0.143:/zroot/repo/desktop
	rsync -rtvzP release/icons/* 192.168.0.143:/zroot/repo/icons

install:
	mkdir -p ${DESTDIR}${PREFIX}/bin ${DESTDIR}${PREFIX}/share/applications
	cp -f ${NAME} ${DESTDIR}${PREFIX}/bin
	cp -f ${NAME}.desktop ${DESTDIR}${PREFIX}/share/applications
.if ${OS} == "linux"
	sed -i 's/\/local//' ${DESTDIR}${PREFIX}/share/applications/${NAME}.desktop
.endif
	cp -rf icons/076 ${DESTDIR}${PREFIX}/share/icons
	chmod 755 ${DESTDIR}${PREFIX}/bin/${NAME}

uninstall:
	rm -f ${DESTDIR}${PREFIX}/share/applications/${NAME}.desktop
	rm -f ${DESTDIR}${PREFIX}/bin/${NAME}

.PHONY: all debug clean dist release publish install uninstall
