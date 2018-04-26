# memfree - linux free(1) like tool for OpenBSD
# See LICENSE file for copyright and license details.

# memfree version
VERSION = 0.1

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/man

# flags
CPPFLAGS = -D_XOPEN_SOURCE=700 -D_BSD_SOURCE -DVERSION=\"${VERSION}\"
CFLAGS   = -std=c99 -pedantic -Wall ${CPPFLAGS}

# compiler and linker
CC = cc

SRC = memfree.c 
OBJ = ${SRC:.c=.o}

all: options memfree 

options:
	@echo memfree build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

memfree: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ}

clean:
	@echo cleaning
	@rm -f memfree memfree.core ${OBJ}

dist: clean
	@echo creating dist tarball
	@mkdir -p memfree-${VERSION}
	@cp -R LICENSE TODO BUGS Makefile README memfree.c \
		memfree.1 ${SRC} memfree-${VERSION}
	@tar -cf memfree-${VERSION}.tar memfree-${VERSION}
	@gzip memfree-${VERSION}.tar
	@rm -rf memfree-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f memfree ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/memfree
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g" < memfree.1 > ${DESTDIR}${MANPREFIX}/man1/memfree.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/memfree.1

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/memfree
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/memfree.1

.PHONY: all options clean dist install uninstall
