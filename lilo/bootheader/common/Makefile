# $Id$
SRCROOT  = ..
include ../MCONFIG
include ../MRULES

MAKEDEPS     = -Wp,-MD,.$(subst /,-,$*).d
CFLAGS       = $(MAKEDEPS) $(OPTFLAGS) $(REQFLAGS)
PROGS       := infblock infcodes inffast inflate inftrees infutil
LIB          = zlib.a

OBJS        := $(patsubst %,%.o,$(PROGS))

all:	$(OBJS) $(LIB)

$(LIB): $(OBJS)
	$(AR) cru $(LIB).~ $(OBJS)
	$(RANLIB) $(LIB).~
	mv -f $(LIB).~ $(LIB)

#.SECONDARY: $(OBJS)

install:
	@set -ex; \
	$(INSTALL_DIR) $(DESTDIR)$(INSTALLDIR)/common ; \
	for i in $(OBJS); do \
	$(INSTALL_DATA) $$i $(DESTDIR)$(INSTALLDIR)/common/$$i; \
	done ; \

clean:
	$(RM) *.o .*.d *.a

spotless: clean
	$(RM) *~ *.i *.s

ifneq ($(wildcard .*.d),)
include $(wildcard .*.d)
endif
