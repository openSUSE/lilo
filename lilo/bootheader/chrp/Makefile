SRCROOT  = ..
include ../MCONFIG
include ../MRULES

MAKEDEPS     = -Wp,-MD,.$(subst /,-,$*).d
CFLAGS       = $(MAKEDEPS) $(OPTFLAGS) $(REQFLAGS)
PROGS       := empty main prom zlib \
		crt0 div64 string

OBJS        := $(patsubst %,%.o,$(PROGS))

all:	$(OBJS)

.SECONDARY: $(OBJS)

clean:
	$(RM) *.o .*.d

spotless: clean
	$(RM) *~ *.i *.s

ifneq ($(wildcard .*.d),)
include $(wildcard .*.d)
endif
