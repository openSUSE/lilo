#!/bin/bash
# vim: syntax=sh
#
# helper script to generate an initrd in the running system
#
# execute it like that:
# sh addRamdisk.sh tmppath vmlinuz initrd.gz outputfile
#
# Example:
# sh addRamdisk.sh /tmp/ppc_lilo /boot/vmlinuz /boot/initrd /boot/myzimage
#
# boot the file '/boot/myzimage' via yaboot.
#
# you need a biarch binutils.rpm from SLES 8 or newer
#
set -ex
echo running $0 with $# args
test "$#" = 4 || exit 1
echo $*
#
my_TMP=${1}/d
my_KERNEL=$2
my_INITRD=$3
my_OUTPUT=$4
my_BOOTDIR=/lib/lilo/chrp/chrp64
#
mkdir -p  $my_TMP
chmod 700 $my_TMP
rm -f     $my_TMP/*
#
cp -av $my_BOOTDIR/zImage.o	$my_TMP
cp -av $my_BOOTDIR/addnote	$my_TMP
#
case "$(file -Lb $my_KERNEL)" in
	ELF\ 64-bit*)
		gzip -cv9 $my_KERNEL > $my_TMP/vmlinux.gz
		;;
	ELF\ 32-bit*)
		objcopy -j .kernel:vmlinux -O binary $my_KERNEL $my_TMP/vmlinux.gz
		;;
	*)
		file -b $my_KERNEL
		echo boo ; exit 1 
		;;
esac
#
chmod +w  $my_TMP/zImage.o
chmod +rx $my_TMP/addnote
#
gzip -cd $my_TMP/vmlinux.gz > $my_TMP/vmlinux
#
strings $my_TMP/vmlinux | grep -E 'Linux version .* .gcc version' > $my_TMP/uts_string.txt
objcopy $my_TMP/zImage.o \
	--add-section=.kernel:uts_string=$my_TMP/uts_string.txt \
	--set-section-flags=.kernel:uts_string=contents,alloc,load,readonly,data
#
objcopy $my_TMP/zImage.o \
	--add-section=.kernel:vmlinux=$my_TMP/vmlinux.gz \
	--set-section-flags=.kernel:vmlinux=contents,alloc,load,readonly,data
#
objcopy $my_TMP/zImage.o \
	--add-section=.kernel:initrd=$my_INITRD \
	--set-section-flags=.kernel:initrd=contents,alloc,load,readonly,data
#
rm -f $my_OUTPUT
#
ld -Ttext 0x00400000 -e _start \
	-T $my_BOOTDIR/zImage.lds \
	-o $my_OUTPUT \
	$my_BOOTDIR/crt0.o \
	$my_BOOTDIR/string.o \
	$my_BOOTDIR/prom.o \
	$my_TMP/zImage.o \
	$my_BOOTDIR/zlib.o \
	--defsym _vmlinux_memsize=0x`nm -n $my_TMP/vmlinux | sed '$s/^........\([^ ]*\).*/\1/p;d'` \
	--defsym _vmlinux_filesize=0x`ls -l $my_TMP/vmlinux | awk '{ printf "%x\n", $5 }'`
#
$my_TMP/addnote $my_OUTPUT
