# $Id$
SRCROOT  = ..
include ../MCONFIG
include ../MRULES

MAKEDEPS     = -Wp,-MD,.$(subst /,-,$*).d
CFLAGS       = $(MAKEDEPS) $(OPTFLAGS) $(REQFLAGS)

PROM_SRC    := main prom
PROM_OBJS   := $(patsubst %,%.o,$(PROM_SRC))
PROM        := prom.a

PROM_CRT0   := crt0.o

all:	$(PROM) $(PROM_CRT0)

$(PROM): $(PROM_OBJS)
	$(AR) cru $(PROM).~ $(PROM_OBJS)
	$(RANLIB) $(PROM).~
	mv -f $(PROM).~ $(PROM)

install:
	@set -ex; \
	$(INSTALL_DIR) $(DESTDIR)$(INSTALLDIR)/chrp ; \
	for i in $(PROM) $(PROM_CRT0) ; do \
	$(INSTALL_DATA) $$i $(DESTDIR)$(INSTALLDIR)/chrp/$$i; \
	done ; \
	$(INSTALL_DATA) ld.script.chrp64 $(DESTDIR)$(INSTALLDIR)/chrp/
	$(INSTALL_DATA) ld.script $(DESTDIR)$(INSTALLDIR)/chrp/

clean:
	$(RM) *.o .*.d *.a

spotless: clean
	$(RM) *~ *.i *.s

ifneq ($(wildcard .*.d),)
include $(wildcard .*.d)
endif
