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
gzip -9 "$tmp/vmlinux.bin"


if [ -z "$initrd" ] ; then
objcopy -O elf32-powerpc \
	--add-section=.image="$tmp/vmlinux.bin.gz" \
	--set-section-flags=.image=contents,alloc,load,readonly,data \
	"$obj_dir/prep/arch_ppc_boot_simple_dummy.o" \
	"$tmp/arch_ppc_boot_simple_image.o"
else
objcopy -O elf32-powerpc \
	--add-section=.ramdisk="$initrd" \
	--set-section-flags=.ramdisk=contents,alloc,load,readonly,data \
	--add-section=.image="$tmp/vmlinux.bin.gz" \
	--set-section-flags=.image=contents,alloc,load,readonly,data \
	"$obj_dir/prep/arch_ppc_boot_simple_dummy.o" \
	"$tmp/arch_ppc_boot_simple_image.o"
fi

ld \
	-T "$obj_dir/prep/arch_ppc_boot_ld.script" \
	-Ttext 0x00800000 \
	-Bstatic \
	-o "$tmp/arch_ppc_boot_prep_zImage.bin" \
	"$obj_dir/prep/arch_ppc_boot_simple_head.o"  \
	"$obj_dir/prep/arch_ppc_boot_simple_relocate.o"  \
	"$obj_dir/prep/arch_ppc_boot_simple_prepmap.o"  \
	"$obj_dir/prep/arch_ppc_boot_simple_misc.o"  \
	"$obj_dir/prep/arch_ppc_boot_simple_misc-prep.o"  \
	"$obj_dir/prep/arch_ppc_boot_simple_mpc10x_memory.o"  \
	"$tmp/arch_ppc_boot_simple_image.o"  \
	"$obj_dir/prep/arch_ppc_boot_common_lib.a"  \
	"$obj_dir/prep/arch_ppc_boot_lib_lib.a" \
	"$obj_dir/prep/arch_ppc_boot_of1275_lib.a"

objcopy \
	-O elf32-powerpc \
	-R .comment \
	-R .stab \
	-R .stabstr \
	"$tmp/arch_ppc_boot_prep_zImage.bin" \
	"$tmp/arch_ppc_boot_prep_zImage"

"$obj_dir/prep/mkprep" \
	-pbp \
	"$tmp/arch_ppc_boot_prep_zImage" \
	"$tmp/output"


#
rm -f "$output"
cp "$tmp/output" "$output"

