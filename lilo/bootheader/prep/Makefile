# $Id$
SRCROOT  = ..
include ../MCONFIG
include ../MRULES

MAKEDEPS     = -Wp,-MD,.$(subst /,-,$*).d
CFLAGS       = $(MAKEDEPS) $(OPTFLAGS) $(REQFLAGS)

PREP_SRC    := head kbd main ns16550 relocate util vreset
PREP_OBJS   := $(patsubst %,%.o,$(PREP_SRC))
PREP        := prep.a

PREP_CRT0   := crt0.o

all:	$(PREP) $(PREP_CRT0)

$(PREP): $(PREP_OBJS)
	$(AR) cru $(PREP).~ $(PREP_OBJS)
	$(RANLIB) $(PREP).~
	mv -f $(PREP).~ $(PREP)

install:
	@set -ex; \
	$(INSTALL_DIR) $(DESTDIR)$(INSTALLDIR)/prep ; \
	for i in $(PREP) $(PREP_CRT0) ; do \
	$(INSTALL_DATA) $$i $(DESTDIR)$(INSTALLDIR)/prep/$$i; \
	done ; \

clean:
	$(RM) *.o .*.d *.a

spotless: clean
	$(RM) *~ *.i *.s

ifneq ($(wildcard .*.d),)
include $(wildcard .*.d)
endif
