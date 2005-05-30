# $Id$
SRCROOT  = ..
include ../MCONFIG
include ../MRULES

PROGS       := addnote hack-coff

all:	$(PROGS)

addnote: addnote.c
	$(HOST_CC)  $(HOST_CFLAGS) $(HOST_LDFLAGS) -o $@ $<

hack-coff: hack-coff.c rs6000.h
	$(HOST_CC)  $(HOST_CFLAGS) $(HOST_LDFLAGS) -o $@ $<

install:
	@set -ex; \
	$(INSTALL_DIR) $(DESTDIR)$(INSTALLDIR)/utils ; \
	for i in $(PROGS) ; do \
	$(INSTALL_EXEC) $$i $(DESTDIR)$(INSTALLDIR)/utils/ ; \
	done

clean:
	$(RM) *.o .*.d $(PROGS)

spotless: clean
	$(RM) *~ *.i *.s

ifneq ($(wildcard .*.d),)
include $(wildcard .*.d)
endif

