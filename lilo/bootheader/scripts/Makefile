# $Id$
SRCROOT  = ..
include ../MCONFIG
include ../MRULES
SCRIPTS= \
	make_zimage_chrp64.sh \
	make_zimage_pmac_newworld.sh \
	make_zimage_prep.sh \
	make_zimage_iseries.sh \
	make_zimage_pmac_oldworld_coff.sh


all:

install:
	@set -ex; \
	$(INSTALL_DIR) $(DESTDIR)$(INSTALLDIR)/scripts ; \
	$(INSTALL_DIR) $(DESTDIR)/bin ; \
	for i in $(SCRIPTS) ; do \
	$(INSTALL_EXEC) $$i $(DESTDIR)$(INSTALLDIR)/scripts/ ; \
	done ; \
	$(INSTALL_EXEC) mkzimage.sh $(DESTDIR)/bin/mkzimage

clean:
	

spotless: clean
	$(RM) *~ *.i *.s

