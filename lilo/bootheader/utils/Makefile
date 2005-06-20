# $Id$
SRCROOT  = ..
include ../MCONFIG
include ../MRULES

PROGS       := addnote hack-coff iseries-addRamDisk iseries-addSystemMap mkzimage_cmdline mknote

all:	$(PROGS)

addnote: addnote.c
	$(HOST_CC)  $(HOST_CFLAGS) $(HOST_LDFLAGS) -o $@ $<

hack-coff: hack-coff.c rs6000.h
	$(HOST_CC)  $(HOST_CFLAGS) $(HOST_LDFLAGS) -o $@ $<
iseries-addRamDisk: iseries-addRamDisk.c
	$(HOST_CC)  $(HOST_CFLAGS) $(HOST_LDFLAGS) -o $@ $<
iseries-addSystemMap: iseries-addSystemMap.c
	$(HOST_CC)  $(HOST_CFLAGS) $(HOST_LDFLAGS) -o $@ $<
mknote: mknote.c
	$(HOST_CC)  $(HOST_CFLAGS) $(HOST_LDFLAGS) -o $@ $<
mkzimage_cmdline: mkzimage_cmdline.c
	$(HOST_CC)  $(HOST_CFLAGS) $(HOST_LDFLAGS) -o $@ $<

install:
	@set -ex; \
	$(INSTALL_DIR) $(DESTDIR)$(INSTALLDIR)/utils ; \
	$(INSTALL_DIR) $(DESTDIR)/bin ; \
	for i in $(PROGS) ; do \
	$(INSTALL_EXEC) $$i $(DESTDIR)$(INSTALLDIR)/utils/ ; \
	done
	ln -sv $(INSTALLDIR)/utils/mkzimage_cmdline $(DESTDIR)/bin/mkzimage_cmdline

clean:
	$(RM) *.o .*.d $(PROGS)

spotless: clean
	$(RM) *~ *.i *.s

ifneq ($(wildcard .*.d),)
include $(wildcard .*.d)
endif

