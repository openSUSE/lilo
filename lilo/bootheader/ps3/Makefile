# $Id: Makefile 956 2007-04-25 09:45:23Z olh $
SRCROOT  = ..
include ../MCONFIG
include ../MRULES

MAKEDEPS     = -Wp,-MD,.$(subst /,-,$*).d
CFLAGS       = $(MAKEDEPS) $(OPTFLAGS) $(REQFLAGS)

PS3_SRC    := ps3 \
	ps3-head \
	ps3-hvcall \
	devtree \
	flatdevtree \
	flatdevtree_misc \
	simple_alloc \
	main \
	util \
	string \
	stdio \
	gunzip_util \
	elf_util \
	div64 \
	crt0 
		
PS3_OBJS   := $(patsubst %,%.o,$(PS3_SRC))
PS3        := ps3.a
PS3_DTB = ps3.dtb
$(PS3_DTB): ps3.dts
	dtc -O dtb -o ps3.dtb -b 0 -V 16 ps3.dts

all:	$(PS3) $(PS3_DTB)

$(PS3): $(PS3_OBJS)
	$(AR) cru $(PS3).~ $(PS3_OBJS)
	$(RANLIB) $(PS3).~
	mv -f $(PS3).~ $(PS3)

install:
	@set -ex; \
	$(INSTALL_DIR) $(DESTDIR)$(INSTALLDIR)/ps3 ; \
	for i in $(PS3) ; do \
	$(INSTALL_DATA) $$i $(DESTDIR)$(INSTALLDIR)/ps3/$$i; \
	done ; \
	$(INSTALL_DATA) zImage.ps3.lds $(DESTDIR)$(INSTALLDIR)/ps3/
	$(INSTALL_DATA) ps3.dtb $(DESTDIR)$(INSTALLDIR)/ps3/

clean:
	$(RM) *.o .*.d *.a *.dtb

spotless: clean
	$(RM) *~ *.i *.s

ifneq ($(wildcard .*.d),)
include $(wildcard .*.d)
endif
