#!/bin/bash
# vim: syntax=sh
#
# helper script to generate an initrd in the running system
#
# execute it like that:
# sh addRamdisk.sh tmppath vmlinuz initrd.gz outputfile
#
# Example:
# sh addRamdisk.sh /tmp/ppc_lilo /boot/vmlinuz /boot/initrd /boot/myzimage
#
# boot the file '/boot/myzimage' via yaboot.
#
# you need a biarch binutils.rpm from SLES 8 or newer
#
set -ex
echo running $0 with $# args
XXX="${0%/*}/make_zimage_chrp64.sh"
echo "better use $XXX instead of $0"
test "$#" = 4 || exit 1
echo $*
#
my_TMP=${1}/d
my_KERNEL=$2
my_INITRD=$3
my_OUTPUT=$4
mkdir -p "$my_TMP"
exec "$XXX" --tmp "$my_TMP" --vmlinux "$my_KERNEL" --initrd "$my_INITRD" --output "$my_OUTPUT"
