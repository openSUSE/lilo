# $Id$
SRCROOT  = ..
include ../MCONFIG
include ../MRULES

MAKEDEPS     = -Wp,-MD,.$(subst /,-,$*).d
CFLAGS       = $(MAKEDEPS) $(OPTFLAGS) $(REQFLAGS)

CHRP_SRC    := main prom
CHRP_OBJS   := $(patsubst %,%.o,$(CHRP_SRC))
CHRP        := chrp.a

CHRP_CRT0   := crt0.o

all:	$(CHRP) $(CHRP_CRT0)

$(CHRP): $(CHRP_OBJS)
	$(AR) cru $(CHRP).~ $(CHRP_OBJS)
	$(RANLIB) $(CHRP).~
	mv -f $(CHRP).~ $(CHRP)

install:
	@set -ex; \
	$(INSTALL_DIR) $(DESTDIR)$(INSTALLDIR)/chrp ; \
	for i in $(CHRP) $(CHRP_CRT0) ; do \
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
