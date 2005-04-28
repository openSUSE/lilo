# $Id$
SRCROOT  = ..
include ../MCONFIG
include ../MRULES

all:

install:
	@set -ex; \
	$(INSTALL_DIR) $(DESTDIR)$(INSTALLDIR)/chrp/chrp64 ; \
	$(INSTALL_DIR) $(DESTDIR)/bin ; \
	$(INSTALL_EXEC) make_zimage_chrp64.sh $(DESTDIR)$(INSTALLDIR)/chrp/ ; \
	ln -fsv ../make_zimage_chrp64.sh $(DESTDIR)$(INSTALLDIR)/chrp/chrp64/make_zimage_chrp64.sh ; \
	$(INSTALL_EXEC) mk_zimage.sh $(DESTDIR)/bin/mk_zimage

clean:
	

spotless: clean
	$(RM) *~ *.i *.s

