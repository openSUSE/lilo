# $Id$
SRCROOT  = ..
include ../MCONFIG
include ../MRULES
MANPAGES= mkzimage.8  mkzimage_cmdline.8
MANDIR=/usr/share/man/man8

all:

install:
	@set -ex; \
	$(INSTALL_DIR) $(DESTDIR)$(MANDIR) ; \
	for i in $(MANPAGES) ; do \
	$(INSTALL_DATA) $$i $(DESTDIR)$(MANDIR) ; \
	done ; \

clean:

spotless: clean
	$(RM) *~ *.i *.s

