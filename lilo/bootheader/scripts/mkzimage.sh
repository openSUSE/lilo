#!/bin/bash
# $Id$
set -e
# set -x

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

until [ "$#" = "0" ] ; do
	case "$1" in
		--help|-h|--version)
		echo "create a 'zImage' for ppc/ppc64"
		echo "Usage: ${0##*/} --vmlinux <ELF binary> [--initrd <ramdisk.image.gz>] --output <zImage> [--cmdline <kernelcmdline>] [--board <subarch>] [--tmp <tempdir>]"
		echo "additional options: [--objdir <dir>]"
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
if [ "$board_type" = "guess" ] ; then
	test -f /proc/cpuinfo || mount -v -n -t proc proc /proc
	while read line; do
		case "$line" in
		  *MacRISC*)	board_type="pmac" ;;
		  *CHRP*)  	board_type="chrp";;
		  *PReP*)  	board_type="prep";;
		  *iSeries*)    board_type="iseries";;
		  pmac-generation*)
			set -- $line
			board_type=$3
			;;
		esac
	done < /proc/cpuinfo
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
