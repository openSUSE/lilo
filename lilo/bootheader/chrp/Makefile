# $Id$
SRCROOT  = ..
include ../MCONFIG
include ../MRULES

MAKEDEPS     = -Wp,-MD,.$(subst /,-,$*).d
CFLAGS       = $(MAKEDEPS) $(OPTFLAGS) $(REQFLAGS)

CHRP_SRC    := main of1275_find_type_devices
CHRP_OBJS   := $(patsubst %,%.o,$(CHRP_SRC))
CHRP        := chrp.a

PROM_SRC    := call_prom prominit \
		child \
		claim \
		close \
		enter \
		exit \
		finddevice \
		getprop \
		instance_to_package \
		instance_to_path \
		map \
		open \
		package_to_path \
		parent \
		peer \
		read \
		release \
		seek \
		setprop \
		write 

PROM_OBJS   := $(patsubst %,of1275_%.o,$(PROM_SRC))
PROM        := prom.a

CHRP_CRT0   := crt0.o

all:	$(CHRP) $(CHRP_CRT0) $(PROM)

$(CHRP): $(CHRP_OBJS)
	$(AR) cru $(CHRP).~ $(CHRP_OBJS)
	$(RANLIB) $(CHRP).~
	mv -f $(CHRP).~ $(CHRP)

$(PROM): $(PROM_OBJS)
	$(AR) cru $(PROM).~ $(PROM_OBJS)
	$(RANLIB) $(PROM).~
	mv -f $(PROM).~ $(PROM)

install:
	@set -ex; \
	$(INSTALL_DIR) $(DESTDIR)$(INSTALLDIR)/chrp ; \
	for i in $(CHRP) $(PROM) $(CHRP_CRT0) ; do \
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
