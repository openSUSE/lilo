#!/bin/bash
set -e
# set -x

obj_dir=.
obj_dir=/lib/lilo/pmac/newworld

vmlinux=
initrd=
tmp=
outputfile=
until [ "$#" = "0" ] ; do
	case "$1" in
		--help|-h|--version)
		echo "create a 'zImage' for new PowerMacs"
		echo "Usage: ${0##*/} --vmlinux <ELF binary> --initrd <ramdisk.image.gz> --output <zImage> [--tmp <tempdir>]"
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
	tmp=`mktemp -d /tmp/mkzimage_pmac_newworld.$$.XXXXXX`
else
	tmp=`mktemp -d $tmp/mkzimage_pmac_newworld.$$.XXXXXX`
fi
#

objcopy  -O binary $vmlinux "$tmp/vmlinux.bin"
gzip -v9 "$tmp/vmlinux.bin"
objcopy \
	-R .comment \
	--add-section=.image="$tmp/vmlinux.bin.gz" \
	--set-section-flags=.image=contents,alloc,load,readonly,data \
	"$obj_dir/arch_ppc_boot_openfirmware_dummy.o" \
	"$tmp/arch_ppc_boot_openfirmware_image.o"

if [ -z "$initrd" ] ; then
OBJCOPY_RAMDISK_OBJECT="$tmp/arch_ppc_boot_openfirmware_image.o"
OBJCOPY_RAMDISK=
else
OBJCOPY_RAMDISK_OBJECT="$tmp/arch_ppc_boot_openfirmware_image_initrd.o"
OBJCOPY_RAMDISK=" -R .ramdisk "
objcopy \
	"$tmp/arch_ppc_boot_openfirmware_image.o" \
	$OBJCOPY_RAMDISK_OBJECT \
	--add-section=.ramdisk="$initrd" \
	--set-section-flags=.ramdisk=contents,alloc,load,readonly,data
fi

ld \
	-T "$obj_dir/arch_ppc_boot_ld.script" \
	-e _start \
	-Ttext 0x01000000 \
	-o "$tmp/output" \
	"$obj_dir/arch_ppc_boot_openfirmware_crt0.o" \
	"$obj_dir/arch_ppc_boot_openfirmware_start.o" \
	"$obj_dir/arch_ppc_boot_openfirmware_misc.o" \
	"$obj_dir/arch_ppc_boot_openfirmware_common.o" \
	"$obj_dir/arch_ppc_boot_openfirmware_newworldmain.o" \
	"$obj_dir/lib_lib.a" \
	"$obj_dir/arch_ppc_boot_lib_lib.a" \
	"$obj_dir/arch_ppc_boot_of1275_lib.a" \
	"$obj_dir/arch_ppc_boot_common_lib.a" \
	$OBJCOPY_RAMDISK_OBJECT

objcopy \
	"$tmp/output" \
	"$tmp/output" \
	--add-section=.note="$obj_dir/arch_ppc_boot_openfirmware_note" \
	$OBJCOPY_RAMDISK \
	-R .comment 
#
rm -f "$output" || :
cp "$tmp/output" "$output"

