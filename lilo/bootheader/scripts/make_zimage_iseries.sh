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
		echo "create a 'zImage' for new iSeries"
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
	tmp=`mktemp -d ${TMPDIR:-/tmp}/mkzimage_iseries.$$.XXXXXX`
else
	tmp=`mktemp -d $tmp/mkzimage_iseries.$$.XXXXXX`
fi
#

nm "$vmlinux" | \
	grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | \
	sort > "$tmp/System.map"
if [ -z "$initrd" ] ; then
$obj_dir/iseries/iseries-addSystemMap \
	"$tmp/System.map" \
	"$vmlinux" \
	"$tmp/output"
else
$obj_dir/iseries/iseries-addRamDisk \
	"$initrd" \
	"$tmp/System.map" \
	"$vmlinux" \
	"$tmp/output"
fi
#
rm -f "$output"
cp "$tmp/output" "$output"

