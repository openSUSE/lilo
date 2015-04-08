#!/bin/bash
# $Id$
set -e
# set -x

obj_dir=/lib/lilo

vmlinux=
initrd=
tmp=
outputfile=
no_addnote=false
addnote_bin=
until [ "$#" = "0" ] ; do
	case "$1" in
		--help|-h|--version)
		echo "create a 'zImage' for new pSeries"
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
#
strip -o $tmp/vmlinux $vmlinux
gzip -c9 $tmp/vmlinux > $tmp/vmlinux.gz
#
#
strings $tmp/vmlinux | grep -E 'Linux version .* .gcc' > $tmp/uts_string.txt || echo "grep for string 'Linux version' in vmlinux failed, continuing anyway"
echo -ne "\000" >> $tmp/uts_string.txt
cp $obj_dir/common/empty.o $tmp/empty.o
objcopy $tmp/empty.o \
	--add-section=.uts_string=$tmp/uts_string.txt \
	--set-section-flags=.uts_string=contents,alloc,load,readonly,data
#
objcopy $tmp/empty.o \
	--add-section=.kernel:vmlinux.strip=$tmp/vmlinux.gz \
	--set-section-flags=.kernel:vmlinux.strip=contents,alloc,load,readonly,data

md5sum $tmp/vmlinux.gz | awk '{print $1"\000"}' > $tmp/vmlinuz_md5.txt
echo -n "vmlinuz md5sum: "
strings $tmp/vmlinuz_md5.txt
objcopy $tmp/empty.o \
	--add-section=.vmlinuz_md5=$tmp/vmlinuz_md5.txt \
	--set-section-flags=.vmlinuz_md5=contents,alloc,load,readonly,data
#
if [ ! -z "$initrd" ] ; then
objcopy $tmp/empty.o \
	--add-section=.kernel:initrd=$initrd \
	--set-section-flags=.kernel:initrd=contents,alloc,load,readonly,data
md5sum $initrd | awk '{print $1"\000"}' > $tmp/initrd_md5.txt
echo -n "initrd md5sum: "
strings $tmp/initrd_md5.txt
objcopy $tmp/empty.o \
	--add-section=.initrd_md5=$tmp/initrd_md5.txt \
	--set-section-flags=.initrd_md5=contents,alloc,load,readonly,data
fi
#
rm -f $tmp/output
#
link_addr=`printf '0x%08x\n' $(( ( 64 * 1024 * 1024 )  ))`
ld \
	-m elf32ppc \
	-Ttext $link_addr \
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
if [ "$no_addnote" = "false" ] ; then
	case "$HOSTTYPE" in
		ppc*|powerpc*)
			addnote_bin=$obj_dir/utils/addnote
			;;
		*)
			addnote_bin="`type -p chrp-addnote || :`"
			if test -z "$addnote_bin" ; then
				addnote_bin="`type -p addnote || :`"
			fi
			;;
	esac
	if test -z "$addnote_bin" ; then
		echo "Could not find addnote or chrp-addnote binary"
	else
		echo add note section for RS6K
		$addnote_bin $tmp/output
	fi
fi
#
rm -f "$output"
cp "$tmp/output" "$output"
rm -rf $tmp

