# $Id$
SRCROOT  = ..
include ../MCONFIG
include ../MRULES

MAKEDEPS     = -Wp,-MD,.$(subst /,-,$*).d
CFLAGS       = $(MAKEDEPS) $(OPTFLAGS) $(REQFLAGS)
ZLIB_SRC    := infblock infcodes inffast inflate inftrees infutil
ZLIB         = zlib.a

ZLIB_OBJS        := $(patsubst %,%.o,$(ZLIB_SRC))

all: $(ZLIB)

$(ZLIB): $(ZLIB_OBJS)
	$(AR) cru $(ZLIB).~ $(OBJS)
	$(RANLIB) $(ZLIB).~
	mv -f $(ZLIB).~ $(ZLIB)

#.SECONDARY: $(OBJS)

install:
	@set -ex; \
	$(INSTALL_DIR) $(DESTDIR)$(INSTALLDIR)/common ; \
	for i in $(ZLIB); do \
	$(INSTALL_DATA) $$i $(DESTDIR)$(INSTALLDIR)/common/$$i; \
	done ; \

clean:
	$(RM) *.o .*.d *.a

spotless: clean
	$(RM) *~ *.i *.s

ifneq ($(wildcard .*.d),)
include $(wildcard .*.d)
endif
