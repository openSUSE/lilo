#!/bin/bash
# $Id: make_zimage_ps3.sh 1024 2007-09-14 08:12:50Z olh $
set -e
set -x

obj_dir=/lib/lilo

vmlinux=
initrd=
tmp=
outputfile=
base_address=
start_address=
system_reset_overlay=
system_reset_kernel=
overlay_dest="256"
overlay_size="256"
no_addnote=false
until [ "$#" = "0" ] ; do
	case "$1" in
		--help|-h|--version)
		echo "create a 'zImage' for Playstation 3"
		echo "Usage: ${0##*/} --vmlinux <ELF binary> --initrd <ramdisk.image.gz> --output <zImage> [--tmp <tempdir>] [--no-addnote]"
		echo "additional options: [--objdir <dir>]"
		exit 1
		;;
		--no-addnote)
		no_addnote=true
		shift
		;;
		--vmlinux)
		shift
		if [ "$#" = "0" -o "$1" = ""  ] ; then
			echo "option --vmlinux requires a filename"
			exit 1
		fi
		vmlinux=$1
		shift
		;;
		--initrd)
		shift
		if [ "$#" = "0"  -o "$1" = "" ] ; then
			echo "option --initrd requires a filename"
			exit 1
		fi
		initrd=$1
		shift
		;;
		--output)
		shift
		if [ "$#" = "0" -o "$1" = "" ] ; then
			echo "option --output requires a filename"
			exit 1
		fi
		output=$1
		shift
		;;
		--objdir)
		shift
		if [ "$#" = "0" -o "$1" = ""  ] ; then
			echo "option --objdir requires a diretory"
			exit 1
		fi
		obj_dir=$1
		shift
		;;
		--tmp)
		shift
		if [ "$#" = "0" -o "$1" = ""  ] ; then
			echo "option --tmp requires a diretory"
			exit 1
		fi
		tmp=$1
		shift
		;;
		*)
		echo "ERROR: unknown option $1"
		exec $0 --help
		exit 1
	esac
done
if [ -z "$vmlinux" ] ; then
	echo "ERROR: no input file"
	exec $0 --help
	exit 1
fi
if [ -z "$output" ] ; then
	echo "ERROR: no output file"
	exec $0 --help
	exit 1
fi
if [ -z "$tmp" ] ; then
	tmp=`mktemp -d ${TMPDIR:-/tmp}/mkzimage_chrp.$$.XXXXXX`
else
	tmp=`mktemp -d $tmp/mkzimage_chrp.$$.XXXXXX`
fi
# convert to RAW format
objcopy -O binary --set-section-flags=.bss=contents,alloc,load,data $vmlinux $tmp/vmlinux.bin
#
cp $obj_dir/common/empty.o $tmp/empty.o
# add sections to empty.o
objcopy $tmp/empty.o \
	--add-section=.kernel:dtb=$obj_dir/ps3/ps3.dtb \
	--set-section-flags=.kernel:dtb=contents,alloc,load,readonly,data
#
objcopy $tmp/empty.o \
	--add-section=.kernel:vmlinux.bin=$tmp/vmlinux.bin \
	--set-section-flags=.kernel:vmlinux.bin=contents,alloc,load,readonly,data
#
if [ ! -z "$initrd" ] ; then
objcopy $tmp/empty.o \
	--add-section=.kernel:initrd=$initrd \
	--set-section-flags=.kernel:initrd=contents,alloc,load,readonly,data
fi
# need to extract ps3-head, so linker is able to find the entry point
ar p $obj_dir/ps3/ps3.a ps3-head.o > $tmp/ps3-head.o
#
rm -f $tmp/output
# link this baby together
ld \
	-m elf32ppc \
	-T $obj_dir/ps3/zImage.ps3.lds \
	-o $tmp/zImage.ps3 \
	$tmp/ps3-head.o \
	$tmp/empty.o \
	$obj_dir/ps3/ps3.a \

# convert to RAW format
objcopy -O binary --set-section-flags=.bss=contents,alloc,load,data $tmp/zImage.ps3 $tmp/zImage.ps3.bin
ls -l $tmp/zImage.ps3
ls -l $tmp/zImage.ps3.bin
# get base address of executable code
base_address=$(nm $tmp/zImage.ps3 | grep ' _start$' | cut '-d ' -f1)
# get entry address of executable code
start_address=$(objdump -f $tmp/zImage.ps3 |grep '^start address '| cut '-d ' -f3)
# get system_reset_overlay
system_reset_overlay=$(nm $tmp/zImage.ps3 | grep ' __system_reset_overlay$' | cut '-d ' -f1)
# convert to decimal
system_reset_overlay=$(printf "%d" 0x$system_reset_overlay)
# get system_reset_kernel
system_reset_kernel=$(nm $tmp/zImage.ps3 | grep ' __system_reset_kernel$' | cut '-d ' -f1)
# convert to decimal
system_reset_kernel=$(printf "%d" 0x$system_reset_kernel)
# preserve overlay range (copy to another location)
dd if=$tmp/zImage.ps3.bin of=$tmp/zImage.ps3.bin conv=notrunc \
	skip=$overlay_dest seek=$system_reset_kernel  \
	count=$overlay_size bs=1
# copy overlayed range to another location
dd if=$tmp/zImage.ps3.bin of=$tmp/zImage.ps3.bin conv=notrunc \
	skip=$system_reset_overlay seek=$overlay_dest  \
	count=$overlay_size bs=1
# gzip the result
gzip --force -9 --stdout $tmp/zImage.ps3.bin > $tmp/otheros.bld
# delete old output file and replace with the new one
rm -f "$output"
cp "$tmp/otheros.bld" "$output"
#rm -rf $tmp
