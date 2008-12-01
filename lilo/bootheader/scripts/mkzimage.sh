#!/bin/bash
# $Id$
set -e
#set -x

export LANG=C
export LC_ALL=C
obj_dir=/lib/lilo

vmlinux=
initrd=
tmp=
outputfile=
cmdline=
#
options=
#
board_type=guess
kernel_type=guess
zimage_sh=guess
proc_mounted=no

until [ "$#" = "0" ] ; do
	case "$1" in
		--help|-h|--version)
		echo "create a 'zImage' for ppc/ppc64, bootable from Openfirmware prompt"
		echo "Usage: ${0##*/} --vmlinux <ELF binary> --output <zImage>"
		echo "additional options:"
		echo "                    [--objdir <dir>]"
		echo "                    [--initrd <ramdisk.image.gz>]"
		echo "                    [--cmdline <kernelcmdline>]"
		echo "                    [--tmp <tempdir>]"
		echo "                    [--board <subarch>]"
		echo "option --board requires a ppc/ppc64 subarch. It can be one of:"
		echo "chrp|rs6k iseries pmaccoff pmac prep"
		exit 1
		;;
		--board)
		shift
		if [ "$#" = "0" -o "$1" = ""  ] ; then
			exec $0 --help
			exit 1
		fi
		board_type=$1
		shift
		;;
		--cmdline)
		shift
		if [ "$#" = "0" -o "$1" = ""  ] ; then
			exec $0 --help
			exit 1
		fi
		cmdline=$1
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
		;;
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
#
case "$(file -Lb $vmlinux)" in
	gzip\ compressed\ data*)
		if [ -z "$tmp" ] ; then
			tmp=`mktemp -d ${TMPDIR:-/tmp}/mkzimage.$$.XXXXXX`
		else
			tmp=`mktemp -d $tmp/mkzimage.$$.XXXXXX`
		fi
		gzip -c -d -v $vmlinux > $tmp/vmlinux.mkzimage
		vmlinux="$tmp/vmlinux.mkzimage"
	;;
	*)
	;;
esac
case "$(file -Lb $vmlinux)" in
	ELF\ 64-bit*)
		kernel_type=64bit
		;;
	ELF\ 32-bit*)
		kernel_type=32bit
		;;
	*)
		file -b $vmlinux
		echo wrong filetype
		exit 1 
		;;
esac
if [ ! -f /proc/cpuinfo ] ; then
	if mount -v -n -t proc proc /proc ; then
		proc_mounted=yes
	fi
fi
if [ "$board_type" = "guess" ] ; then
	while read line; do
		case "$line" in
		  *MacRISC*)	board_type="pmac" ;;
		  *EFIKA5K2*)	board_type="chrp" ;;
		  *CHRP*)  	board_type="chrp";;
		  *PReP*)  	board_type="prep";;
		  *iSeries*)    board_type="iseries";;
		  *PS3*)	board_type="ps3";;
		  pmac-generation*)
			set -- $line
			board_type=$3
			;;
		esac
	done < /proc/cpuinfo
fi
if [ "$board_type" = "guess" ] ; then
	line=`cat < /proc/device-tree/model`
	case "$line" in
		Momentum,Maple-D)	board_type="chrp" ;;
		Momentum,Maple-L)	board_type="chrp" ;;
		*) echo "board type '$line' not recognized, specify one with --board" ;;
	esac
fi
case "$kernel_type" in
	64bit)
	case "$board_type" in
			chrp|pseries|rs6k)
			zimage_sh=make_zimage_chrp.sh
			;;
			iseries)
			zimage_sh=make_zimage_iseries.sh
			;;
			pmac|NewWorld)
			zimage_sh=make_zimage_pmac_newworld.sh
			;;
			ps3)
			zimage_sh=make_zimage_ps3.sh
			;;
			*)
			echo "ERROR: boardtype \"$board_type\" not supported as 64bit"
			exec $0 --help
			;;
	esac
	;;
	32bit)
	case "$board_type" in
			chrp|rs6k)
			zimage_sh=make_zimage_chrp.sh
			;;
			pmaccoff|OldWorld)
			zimage_sh=make_zimage_pmac_oldworld_coff.sh
			;;
			prep)
			zimage_sh=make_zimage_prep.sh
			;;
			pmac|NewWorld)
			zimage_sh=make_zimage_pmac_newworld.sh
			;;
			ps3)
			zimage_sh=make_zimage_ps3.sh
			;;
			*)
			echo "ERROR: boardtype \"$board_type\" not supported as 32bit"
			exec $0 --help
			;;
	esac
	;;
esac
#
echo board_type $board_type kernel_type $kernel_type zimage_sh $zimage_sh

#
#
#
if [ ! -z "$initrd" ] ; then
options="--initrd $initrd"
fi
if [ ! -z "$tmp" ] ; then
options="$options --tmp $tmp"
fi
bash $obj_dir/scripts/$zimage_sh $options --vmlinux "$vmlinux" --output "$output" --objdir "$obj_dir"
if [ ! -z "$cmdline" ] ; then
$obj_dir/utils/mkzimage_cmdline -a 1 -c -s "$cmdline" "$output"
fi
if [ "$proc_mounted" = "yes" ] ; then
	umount -v -n /proc
fi
