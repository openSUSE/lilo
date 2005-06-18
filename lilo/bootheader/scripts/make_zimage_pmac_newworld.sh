#!/bin/bash
# $Id$
set -e
# set -x

obj_dir=/lib/lilo

bootfile_sizelimit=$((4*1024*1024-15*1024))
vmlinux=
initrd=
tmp=
outputfile=
until [ "$#" = "0" ] ; do
	case "$1" in
		--help|-h|--version)
		echo "create a 'zImage' for new PowerMacs"
		echo "Usage: ${0##*/} --vmlinux <ELF binary> --initrd <ramdisk.image.gz> --output <zImage> [--tmp <tempdir>]"
		echo "additional options: [--objdir <dir>]"
		exit 1
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
	tmp=`mktemp -d ${TMPDIR:-/tmp}/mkzimage_pmac_newworld.$$.XXXXXX`
else
	tmp=`mktemp -d $tmp/mkzimage_pmac_newworld.$$.XXXXXX`
fi
#
strip -s -o $tmp/vmlinux $vmlinux
gzip -c9 $tmp/vmlinux > $tmp/vmlinux.gz
#
#
strings $tmp/vmlinux | grep -E 'Linux version .* .gcc' > $tmp/uts_string.txt
cp $obj_dir/common/empty.o $tmp/empty.o
objcopy $tmp/empty.o \
	--add-section=.uts_string=$tmp/uts_string.txt \
	--set-section-flags=.uts_string=contents,alloc,load,readonly,data
#
objcopy $tmp/empty.o \
	--add-section=.vmlinuz=$tmp/vmlinux.gz \
	--set-section-flags=.vmlinuz=contents,alloc,load,readonly,data
#
if [ ! -z "$initrd" ] ; then
objcopy $tmp/empty.o \
	--add-section=.initrd=$initrd \
	--set-section-flags=.initrd=contents,alloc,load,readonly,data
fi
#
rm -f $tmp/output
#
ld \
	-m elf32ppc \
	-Ttext 0x01000000 \
	-e _start \
	-T $obj_dir/chrp/ld.script \
	-o $tmp/output \
	$obj_dir/chrp/crt0.o \
	$tmp/empty.o \
	$obj_dir/chrp/chrp.a \
	$obj_dir/common/common.a \
	$obj_dir/chrp/prom.a \
	$obj_dir/common/zlib.a
#
$obj_dir/utils/mknote > $tmp/note
objcopy \
	$tmp/output \
	$tmp/output \
	--add-section=.note=$tmp/note \
	-R .comment
	
rm -f "$output"
cp "$tmp/output" "$output"
bootfile_finalsize=`wc -c < "$output"`
if test $bootfile_finalsize -gt $bootfile_sizelimit ; then
	echo "output file $output is $(($bootfile_finalsize - $bootfile_sizelimit)) bytes too large"
	echo "booting from openfirmware prompt will not work"
fi
rm -rf $tmp
