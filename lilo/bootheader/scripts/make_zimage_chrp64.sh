#!/bin/bash
# $Id$
set -e
# set -x

export LANG=C
export LC_ALL=C
obj_dir=.
obj_dir=/lib/lilo/chrp

vmlinux=
initrd=
tmp=
outputfile=
no_addnote=false
until [ "$#" = "0" ] ; do
	case "$1" in
		--help|-h|--version)
		echo "create a 'zImage' for new pSeries"
		echo "Usage: ${0##*/} --vmlinux <ELF binary> --initrd <ramdisk.image.gz> --output <zImage> [--tmp <tempdir>] [--no-addnote]"
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
	tmp=`mktemp -d ${TMPDIR:-/tmp}/mkzimage_chrp64.$$.XXXXXX`
else
	tmp=`mktemp -d $tmp/mkzimage_chrp64.$$.XXXXXX`
fi
#
case "$(file -Lb $vmlinux)" in
	ELF\ 64-bit*)
		cp -p $vmlinux $tmp/vmlinux
		strip -s $tmp/vmlinux
		gzip -c9 $tmp/vmlinux > $tmp/vmlinux.gz
		;;
	ELF\ 32-bit*)
		objcopy -j .kernel:vmlinux -O binary $vmlinux $tmp/vmlinux.gz
		gzip -dfc9 $tmp/vmlinux.gz > $tmp/vmlinux
		;;
	*)
		file -b $vmlinux
		echo wrong filetype
		exit 1 
		;;
esac
#
#
strings $tmp/vmlinux | grep -E 'Linux version .* .gcc version' > $tmp/uts_string.txt
cp $obj_dir/empty.o $tmp/empty.o
objcopy $tmp/empty.o \
	--add-section=.kernel:uts_string=$tmp/uts_string.txt \
	--set-section-flags=.kernel:uts_string=contents,alloc,load,readonly,data
#
objcopy $tmp/empty.o \
	--add-section=.kernel:vmlinux=$tmp/vmlinux.gz \
	--set-section-flags=.kernel:vmlinux=contents,alloc,load,readonly,data
#
if [ ! -z "$initrd" ] ; then
objcopy $tmp/empty.o \
	--add-section=.kernel:initrd=$initrd \
	--set-section-flags=.kernel:initrd=contents,alloc,load,readonly,data
fi
#
rm -f $tmp/output
#
ld -Ttext 0x00400000 -e _start \
	-T $obj_dir/ld.script.chrp64 \
	-o $tmp/output \
	$obj_dir/crt0.o \
	$obj_dir/string.o \
	$obj_dir/prom.o \
	$obj_dir/main.o \
	$obj_dir/div64.o \
	$tmp/empty.o \
	$obj_dir/../common/zlib.a
#
if [ "$no_addnote" = "false" ] ; then
echo add note section for RS6K
$obj_dir/addnote $tmp/output
fi
#
rm -f "$output"
cp "$tmp/output" "$output"
rm -rf $tmp

