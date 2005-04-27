## Configuration section

VERSION = 1.2.2
# Debug mode (verbose)
DEBUG = 0
PREFIX = /usr/local

#---------------------------------------------------------------------------
# PPC64 Bridge Mode:
#    * Choose "n" if you are on a 32b machine.
#    * Choose "y" so yaboot can boot on 64b PPC machines in 32b mode.
#---------------------------------------------------------------------------
CONFIG_PPC64BRIDGE = n
# CONFIG_PPC64BRIDGE = y

# Enable text colors
CONFIG_COLOR_TEXT = y
# Enable colormap setup
CONFIG_SET_COLORMAP = y
# Enable splash screen
CONFIG_SPLASH_SCREEN = y

# We use fixed addresses to avoid overlap when relocating
# and other trouble with initrd

# Load the bootstrap at 2Mb
TEXTADDR	= 0x200000
# Malloc block at 3Mb -> 4Mb
MALLOCADDR	= 0x300000
MALLOCSIZE	= 0x100000
# Load kernel at 20Mb and ramdisk just after
KERNELADDR	= 0x01400000

# Set this to the prefix of your cross-compiler, if you have one.
# Else leave it empty.
#
CROSS = 

# The flags for the target compiler.
#
CFLAGS = -O0 -g -nostdinc -Wall -isystem `gcc -print-file-name=include`
CFLAGS += -DVERSION=\"${VERSION}\"	#"
CFLAGS += -DTEXTADDR=$(TEXTADDR) -DDEBUG=$(DEBUG)
CFLAGS += -DMALLOCADDR=$(MALLOCADDR) -DMALLOCSIZE=$(MALLOCSIZE)
CFLAGS += -DKERNELADDR=$(KERNELADDR)
CFLAGS += -I ./include

ifeq ($(CONFIG_PPC64BRIDGE),y)
CFLAGS += -Wa,-mppc64bridge -DCONFIG_PPC64BRIDGE
endif

ifeq ($(CONFIG_COLOR_TEXT),y)
CFLAGS += -DCONFIG_COLOR_TEXT
endif

ifeq ($(CONFIG_SET_COLORMAP),y)
CFLAGS += -DCONFIG_SET_COLORMAP
endif

ifeq ($(CONFIG_SPLASH_SCREEN),y)
CFLAGS += -DCONFIG_SPLASH_SCREEN
endif

# Link flags
#
LFLAGS = -Ttext $(TEXTADDR) -Bstatic 

# Libraries
#
LLIBS = lib/libext2fs.a
#LLIBS = -l ext2fs

# For compiling build-tools that run on the host.
#
HOSTCC = gcc
HOSTCFLAGS = -I/usr/include $(CFLAGS) $(CFLAGS)

## End of configuration section

OBJS = crt0.o yaboot.o cache.o prom.o file.o partition.o fs.o cfg.o \
	setjmp.o cmdline.o fs_of.o fs_ext2.o fs_iso.o iso_util.o \
	lib/nosys.o lib/string.o lib/strtol.o \
	lib/vsprintf.o lib/ctype.o lib/malloc.o

ifeq ($(CONFIG_SPLASH_SCREEN),y)
OBJS += gui/effects.o gui/colormap.o gui/video.o gui/pcx.o
endif

CC = $(CROSS)gcc
LD = $(CROSS)ld
AS = $(CROSS)as
OBJCOPY = $(CROSS)objcopy

all: yaboot

lgcc = `$(CC) -print-libgcc-file-name`

yaboot: $(OBJS) addnote
	$(LD) $(LFLAGS) $(OBJS) $(LLIBS) $(lgcc) -o $@
	strip $@
ifeq ($(CONFIG_PPC64BRIDGE),y)
	./util/addnote $@
endif

yaboot.b: yaboot elfextract
	./util/elfextract yaboot yaboot.b

clean:
	rm -f yaboot util/addnote utils/elfextract $(OBJS)
	find . -name '*~' | xargs rm -f
	find . -name '#*' | xargs rm -f
	find . -name .AppleDouble | xargs rm -rf

addnote: util/addnote.c
	$(HOSTCC) $(HOSTCFLAGS) -o util/addnote util/addnote.c

elfextract: util/elfextract.c
	$(HOSTCC) $(HOSTCFLAGS) -o util/elfextract util/elfextract.c
	
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -D__ASSEMBLY__  -c -o $@ $<

install:
	install -o root -g root -m 0755 -d $(PREFIX)/lib/yaboot
	install -o root -g root -m 0644 yaboot $(PREFIX)/lib/yaboot
