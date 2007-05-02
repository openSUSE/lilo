#!/bin/bash
set -e
# set -x

obj_dir=/lib/lilo

vmlinux=
initrd=
tmp=
outputfile=
until [ "$#" = "0" ] ; do
	case "$1" in
		--help|-h|--version)
		echo "create a 'zImage' for new PReP"
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
	tmp=`mktemp -d ${TMPDIR:-/tmp}/mkzimage_prep.$$.XXXXXX`
else
	tmp=`mktemp -d $tmp/mkzimage_prep.$$.XXXXXX`
fi
#

objcopy  -O binary $vmlinux "$tmp/vmlinux.bin"
strings $tmp/vmlinux.bin | grep -E 'Linux version .* .gcc' > $tmp/uts_string.txt
echo -ne "\000" >> $tmp/uts_string.txt
gzip -9 "$tmp/vmlinux.bin"

cp $obj_dir/common/empty.o $tmp/empty.o
objcopy $tmp/empty.o \
	--add-section=.uts_string=$tmp/uts_string.txt \
	--set-section-flags=.uts_string=contents,alloc,load,readonly,data
#
objcopy $tmp/empty.o \
	--add-section=.kernel:vmlinux.strip=$tmp/vmlinux.bin.gz \
	--set-section-flags=.kernel:vmlinux.strip=contents,alloc,load,readonly,data
#
if [ ! -z "$initrd" ] ; then
objcopy $tmp/empty.o \
	--add-section=.kernel:initrd=$initrd \
	--set-section-flags=.kernel:initrd=contents,alloc,load,readonly,data
fi

ld \
	-m elf32ppc \
	-Ttext 0x00800000 \
	-Bstatic \
	-e _start \
	-T $obj_dir/chrp/ld.script \
	-o $tmp/output.tmp \
	$obj_dir/prep/crt0.o \
	$tmp/empty.o \
	$obj_dir/prep/prep.a \
	$obj_dir/common/common.a \
	$obj_dir/chrp/prom.a \
	$obj_dir/common/zlib.a

objcopy \
	-O elf32-powerpc \
	-R .comment \
	-R .stab \
	-R .stabstr \
	"$tmp/output.tmp" \
	"$tmp/output.strip"

"$obj_dir/utils/mkprep" \
	-pbp \
	"$tmp/output.strip" \
	"$tmp/output"


#
rm -f "$output"
cp "$tmp/output" "$output"
rm -rf $tmp

