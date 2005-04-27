## Configuration section

VERSION = 1.0
# Debug mode (verbose)
DEBUG = 0 

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

#yaboot.b: yaboot
#	./util/elfextract yaboot yaboot.b

clean:
	rm -f yaboot util/addnote $(OBJS)
	rm -f *.gcse *.c~ *.h~ *.S~ Makefile~
	rm -f include/*.gcse include/*.c~ include/*.h~ include/*.S~
	rm -f include/asm/*.gcse include/asm/*.c~ include/asm/*.h~ include/asm/*.S~
	rm -f include/et/*.gcse include/et/*.c~ include/et/*.h~ include/et/*.S~
	rm -f include/linux/*.gcse include/linux/*.c~ include/linux/*.h~ include/linux/*.S~
	rm -f include/ext2fs/*.gcse include/ext2fs/*.c~ include/ext2fs/*.h~ include/ext2fs/*.S~
	rm -f lib/*.gcse lib/*.c~ lib/*.h~ lib/*.S~
	rm -rf .AppleDouble lib/.AppleDouble include/.AppleDouble
	rm -rf include/asm/.AppleDouble include/et/.AppleDouble
	rm -rf include/linux/.AppleDouble include/ext2fs/.AppleDouble

addnote: util/addnote.c
	$(HOSTCC) $(HOSTCFLAGS) -o util/addnote util/addnote.c

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -D__ASSEMBLY__  -c -o $@ $<
