#!/bin/bash
# $Id$
set -e
# set -x

obj_dir=/lib/lilo

tmp=
outputfile=
yabootconf=
do_addnote=false
until [ "$#" = "0" ] ; do
	case "$1" in
		--help|-h|--version)
		echo "Relink yaboot and include a yaboot.conf into the boot binary."
		echo "This is useful on CHRP when booting from 0x41 PReP partition and"
		echo "more than one Linux installation on the same drive."
		echo "Usage: ${0##*/} --configfile <yaboot.conf> --output <zImage> [--tmp <tempdir>] [--addnote]"
		exit 1
		;;
		--addnote)
		do_addnote=true
		shift
		;;
		--configfile)
		shift
		if [ "$#" = "0"  -o "$1" = "" ] ; then
			echo "option --configfile requires a filename"
			exit 1
		fi
		yabootconf=$1
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
if [ -z "$yabootconf" ] ; then
	echo "ERROR: no config file"
	exec $0 --help
	exit 1
fi
if [ -z "$tmp" ] ; then
	tmp=`mktemp -d ${TMPDIR:-/tmp}/mkzimage_chrp.$$.XXXXXX`
else
	tmp=`mktemp -d $tmp/mkzimage_chrp.$$.XXXXXX`
fi
#
cp $obj_dir/common/empty.o $tmp/empty.o
#
objcopy $tmp/empty.o \
	--add-section=.yaboot.conf=$yabootconf \
	--set-section-flags=.yaboot.conf=contents,alloc,load,readonly,data
#
rm -f $tmp/output
#
ld \
	-m elf32ppc \
	-T $obj_dir/chrp/yaboot.ld.script \
	-o $tmp/output \
	$obj_dir/chrp/yaboot.crt0.o \
	$tmp/empty.o \
	$obj_dir/chrp/yaboot.a
#
if [ "$do_addnote" = "true" ] ; then
echo add note section for RS6K
$obj_dir/utils/addnote $tmp/output
fi
#
rm -f "$output"
cp "$tmp/output" "$output"
rm -rf $tmp

