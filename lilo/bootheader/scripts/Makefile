SRCROOT  = ..
include ../MCONFIG
include ../MRULES

all:

install:
	@set -ex; \
	$(INSTALL_DIR) $(DESTDIR)$(INSTALLDIR)/chrp ; \
	$(INSTALL_EXEC) make_zimage_chrp64.sh $(DESTDIR)$(INSTALLDIR)/chrp/

clean:
	

spotless: clean
	$(RM) *~ *.i *.s

