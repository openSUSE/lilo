SRCROOT  = ..
include ../MCONFIG
include ../MRULES

all:

install:
	@set -ex; \
	$(INSTALL_DIR) $(DESTDIR)$(INSTALLDIR)/chrp/chrp64 ; \
	$(INSTALL_EXEC) make_zimage_chrp64.sh $(DESTDIR)$(INSTALLDIR)/chrp/ ; \
	ln -fsv ../make_zimage_chrp64.sh $(DESTDIR)$(INSTALLDIR)/chrp/chrp64/make_zimage_chrp64.sh

clean:
	

spotless: clean
	$(RM) *~ *.i *.s

