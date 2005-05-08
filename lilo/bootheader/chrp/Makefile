# $Id$
SRCROOT  = ..
include ../MCONFIG
include ../MRULES

MAKEDEPS     = -Wp,-MD,.$(subst /,-,$*).d
CFLAGS       = $(MAKEDEPS) $(OPTFLAGS) $(REQFLAGS)
PROGS       := empty main prom zlib \
		crt0 string

OBJS        := $(patsubst %,%.o,$(PROGS))

all:	$(OBJS)

.SECONDARY: $(OBJS)

install:
	@set -ex; \
	$(INSTALL_DIR) $(DESTDIR)$(INSTALLDIR)/chrp ; \
	for i in $(OBJS); do \
	$(INSTALL_DATA) $$i $(DESTDIR)$(INSTALLDIR)/chrp/$$i; \
	done ; \
	$(INSTALL_DATA) ld.script.chrp64 $(DESTDIR)$(INSTALLDIR)/chrp/

clean:
	$(RM) *.o .*.d

spotless: clean
	$(RM) *~ *.i *.s

ifneq ($(wildcard .*.d),)
include $(wildcard .*.d)
endif
