# $Id$
SRCROOT  = ..
include ../MCONFIG
include ../MRULES

MAKEDEPS     = -Wp,-MD,.$(subst /,-,$*).d
CFLAGS       = $(MAKEDEPS) $(OPTFLAGS) $(REQFLAGS)

ZLIB_SRC    := infblock infcodes inffast inflate inftrees infutil
ZLIB_OBJS   := $(patsubst %,%.o,$(ZLIB_SRC))
ZLIB         = zlib.a

COMMON_SRC  := abort div64 exit gunzip string stdio
COMMON_OBJS := $(patsubst %,%.o,$(COMMON_SRC))
COMMON       = common.a

EMPTY_OBJ   := empty.o

all: $(COMMON) $(ZLIB) $(EMPTY_OBJ)

$(ZLIB): $(ZLIB_OBJS)
	$(AR) cru $(ZLIB).~ $(ZLIB_OBJS)
	$(RANLIB) $(ZLIB).~
	mv -f $(ZLIB).~ $(ZLIB)

$(COMMON): $(COMMON_OBJS)
	$(AR) cru $(COMMON).~ $(COMMON_OBJS)
	$(RANLIB) $(COMMON).~
	mv -f $(COMMON).~ $(COMMON)

#.SECONDARY: $(OBJS)

install:
	@set -ex; \
	$(INSTALL_DIR) $(DESTDIR)$(INSTALLDIR)/common ; \
	for i in $(ZLIB) $(COMMON) $(EMPTY_OBJ) ; do \
	$(INSTALL_DATA) $$i $(DESTDIR)$(INSTALLDIR)/common/$$i; \
	done ; \

clean:
	$(RM) *.o .*.d *.a

spotless: clean
	$(RM) *~ *.i *.s

ifneq ($(wildcard .*.d),)
include $(wildcard .*.d)
endif
