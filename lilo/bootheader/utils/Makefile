SRCROOT  = ..
include ../MCONFIG
include ../MRULES

PROGS       := addnote

all:	$(PROGS)

addnote: addnote.c
	$(HOST_CC)  $(HOST_CFLAGS) $(HOST_LDFLAGS) -o $@ $<

install:
	@set -ex; \
	$(INSTALL_DIR) $(DESTDIR)$(INSTALLDIR)/chrp/chrp64 ; \
	$(INSTALL_EXEC) addnote $(DESTDIR)$(INSTALLDIR)/chrp/ ; \
	ln -fsv ../addnote $(DESTDIR)$(INSTALLDIR)/chrp/chrp64/addnote

clean:
	$(RM) *.o .*.d $(PROGS)

spotless: clean
	$(RM) *~ *.i *.s

ifneq ($(wildcard .*.d),)
include $(wildcard .*.d)
endif

