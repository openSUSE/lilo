#!/bin/bash
# set -ex
#
# find a OF bootpath on Apple PowerMacintosh Newworld machines
# Copyright (C) 2000, 2004 Olaf Hering olh@suse.de
#
# When booting via BootX then all symlinks are gone ...
# The MacOS removes them and BenH didn't find (yet) a way to
# bring them back.
# If it looks like a hack then you don't need fielmann.de
#
# Changes
#
# 2004-01-30  sysfs was born
# 2000-08-09  use hd as alias for hda instead of ultra0
# 2000-08-04  run only on pmac new
# 2000-07-19  remove sr* and scd*, not supported yet.
# 2000-07-11  add mesh for Lombard PowerBook
# 2000-07-10  finished the scsi path names for aic and symb
# 2000-07-09  clean up /proc/scsi/scsi parsing
# 2000-07-08  clean up some variable names
# 2000-04-26  get rid of strings
# 2000-04-13  get rid of awk, no success
# 2000-03-28  fix scsi on new macs when boot via bootx
# 2000-02-21  giving up and guess it all from aliases ;-)
# 2000-02-20  finish scsi host, works only with one host for now ...
# 2000-02-01  find ide hosts
# 2000-01-30  first try with scsi hosts
#

myversion=2004-01-30
file=/

if [ "$#" -gt 0 ] ; then
	until  [ "$#" = 0 ] ; do
		case "$1" in
			--version|-v) echo $myversion ; exit 0
			;;
			--help|-h)
			echo "show OpenFirmware device path for a file or a device node"
			echo "usage: ${0##*/} [/]|[/boot/vmlinux]"
			exit 0
			;;
			*)
			file=$1
			break
			;;
		esac
	done
fi

# check if we run on a NewWorld PowerMacintosh
if [ -f /proc/device-tree/openprom/model ] ; then
        while read openfirmware ofversion; do
                case "$openfirmware" in
                iMac,1|OpenFirmware)      MACHINE="pmac_new" ;;
                Open)      MACHINE="pmac_old" ;;
                esac
        done < <(cat /proc/device-tree/openprom/model;echo)
fi

test "$MACHINE" = "pmac_old" && {
echo 1>&2 "ERROR: This machine is an Oldworld, no need for firmware pathnames"
exit 1
}

mystat="`type -p stat`"
if [ -z "$mystat" -o ! -x "$mystat" ] ; then
echo 1>&2 "ERROR: GNU stat required"
exit 1
fi

if [ -b $file ] ; then
file_majorminor=`/bin/ls -l "$file" | awk '{ print $5""$6 }'`
file_major="${file_majorminor%%,*}"
file_minor="${file_majorminor##*,}"
file=/
else
file_majorminor=`"$mystat" --format="%d" "$file"`
file_major="$[file_majorminor/256]"
file_minor="$[file_majorminor - $file_major * 256]"
fi
file_majorminor=$file_major:$file_minor

file_sysfs_path=
for i in `find /sys/block -name dev`
do
: looking at $i
if [ "$(< $i)" = "$file_majorminor" ] ; then file_sysfs_path=$i ; break ; fi
done

if [ -z "$file_sysfs_path" ] ; then
echo 1>&2 "ERROR: can not file major:minor $file_majorminor for $file"
exit 1
fi

: found $file_sysfs_path

file_sysfs_dir="${file_sysfs_path%/dev}"
: $file_sysfs_dir
if [ ! -L "$file_sysfs_dir/device" ] ; then
	# maybe a partition
	file_partition="${file_sysfs_dir##*[a-z]}"
	: file_partition $file_partition
	file_sysfs_dir="${file_sysfs_dir%/*}"
	: $file_sysfs_dir
	if [ ! -L "$file_sysfs_dir/device" ] ; then
		echo 1>&2 "ERROR: driver for sysfs path $file_sysfs_dir has no full sysfs support"
		exit 1
	fi
fi
cd "$file_sysfs_dir/device"
file_full_sysfs_path="`/bin/pwd`"
file_storage_type=
cd "$file_full_sysfs_path"
case "$file_full_sysfs_path" in
	*/ide[0-9]*/[0-9]*)
	file_storage_type=ide
	of_disk_ide_channel="${file_full_sysfs_path##*.}"
	cd ../..
	case "$of_disk_ide_channel" in
		*/*)
		of_disk_ide_channel="${of_disk_ide_channel%%/*}"
		cd ../..
		;;
	esac
	: of_disk_ide_channel $of_disk_ide_channel
	;;
	*/host[0-9]*/[0-9]*:[0-9]*:[0-9]*:[0-9]*)
	file_storage_type=scsi
	of_disk_scsi_lun="${file_full_sysfs_path##*:}"
	of_disk_scsi_id="${file_full_sysfs_path%:*}"
	of_disk_scsi_id="${of_disk_scsi_id##*:}"
	cd ../..
	;;
esac

if [ -f devspec ] ; then
	read file_of_hw_devtype < devspec
	: file_of_hw_devtype $file_of_hw_devtype
	if [ -f /proc/device-tree${file_of_hw_devtype}/device_type ] ; then
		case "$(< /proc/device-tree${file_of_hw_devtype}/device_type)" in
			k2-sata-root)
			: found k2-sata-root, guessing channel
			counter=0
			for i in host[0-9]*
			do
			: working on virtual scsi host $i
				case "$file_full_sysfs_path" in
				*/$i/*)
					: found $i $counter
					break
					;;
				*) ;;
				esac
				: $((counter++))
			done
			file_storage_type=sata
			of_device_path=`grep -l block /proc/device-tree${file_of_hw_devtype}/*@$counter/*/device_type`
			of_device_path=${of_device_path%/device_type}
			of_device_path=${of_device_path##/proc/device-tree}
		esac
	fi
	case "$file_storage_type" in
	ide)
	file_of_hw_path="$(< devspec)/disk@$of_disk_ide_channel"
	;;
	scsi)
	file_of_hw_path=$(printf  "%s/sd@%x,%x"  "$(< devspec)" $of_disk_scsi_id $of_disk_scsi_lun)
	;;
	sata)
	file_of_hw_path=$of_device_path
	;;
	esac
else
	: file_full_sysfs_path $file_full_sysfs_path
	# find the path via the device-tree
	dev_vendor="$(< vendor)"
	dev_device="$(< device)"
	dev_subsystem_vendor="$(< subsystem_vendor)"
	dev_subsystem_device="$(< subsystem_device)"
	for i in `find /proc/device-tree -name vendor-id`
	do
		: looking at $i
		dev_of_pci_id=
		while read a vendor_id
		do
			dev_of_pci_id="0x$vendor_id"
			break
		done  < <(od --read-bytes=8 --width=8 -t x4 $i)
		: vendor-id $dev_of_pci_id
		dev_of_pci_id=$(($dev_of_pci_id))
		dev_vendor=$(($dev_vendor))
		if [ "$dev_of_pci_id" != "$dev_vendor" ] ; then continue ; fi
		if [ ! -f "${i%/*}/device-id" ] ; then continue ; fi
		while read a device_id
		do
			dev_of_pci_id="0x$device_id"
			break
		done < <(od --read-bytes=8 --width=8 -t x4 "${i%/*}/device-id")
		: device-id $dev_of_pci_id
		dev_of_pci_id=$(($dev_of_pci_id))
		dev_device=$(($dev_device))
		if [ "$dev_of_pci_id" != "$dev_device" ] ; then continue ; fi
		if [ -f "${i%/*}/subsystem-vendor-id" ] ; then
			while read a sub_vendor_id
			do
				dev_of_pci_id="0x$sub_vendor_id"
				break
			done < <(od --read-bytes=8 --width=8 -t x4 "${i%/*}/subsystem-vendor-id")
			: sub-vendor-id $dev_of_pci_id
			dev_of_pci_id=$(($dev_of_pci_id))
			dev_subsystem_vendor=$(($dev_subsystem_vendor))
			if [ "$dev_of_pci_id" != "$dev_subsystem_vendor" ] ; then continue ; fi
			while read a sub_device_id
			do
				dev_of_pci_id="0x$sub_device_id"
				break
			done < <(od --read-bytes=8 --width=8 -t x4 "${i%/*}/subsystem-id")
			: sub-device-id $dev_of_pci_id
			dev_of_pci_id=$(($dev_of_pci_id))
			dev_subsystem_device=$(($dev_subsystem_device))
			if [ "$dev_of_pci_id" != "$dev_subsystem_device" ] ; then continue ; fi
		fi
		: found $i
		if [ -z "$of_device_list" ] ; then
			of_device_list="${i%/*}"
		else
			of_device_list="$of_device_list ${i%/*}"
		fi
	done
	: of_device_list $of_device_list
	case "$of_device_list" in
		*\ *)
		: more than one controler found, fun
		for i in $of_device_list
		do
		: working on $i
			while read a addr
			do
			addr="0x$addr"
			break
			done < <(od -t x8 -j4 -N8 < $i/assigned-addresses)
		: addr $addr , pure guess ...
		grep -q ^$addr resource || continue
		: found $i
		of_device_list=$i
		break
		done
		;;
		*)
		;;
	esac

	case "$(< $of_device_list/device_type)" in
		k2-sata-root)
		: found k2-sata-root, guessing channel
		counter=0
		for i in host[0-9]*
		do
		: working on virtual scsi host $i
			case "$file_full_sysfs_path" in
			*/$i/*)
				: found $i $counter
				break
				;;
			*) ;;
			esac
			: $((counter++))
		done
		file_storage_type=sata
		of_device_path=`grep -l block $of_device_list/*@$counter/*/device_type`
		;;
		*)
		of_device_path=`grep -l block $of_device_list/*/device_type`

		;;
	esac

	of_device_path=${of_device_path%/device_type}
	case "$file_storage_type" in
		ide)
		file_of_hw_path="${of_device_path##/proc/device-tree}@$of_disk_ide_channel"
		;;
		scsi)
		file_of_hw_path=$(printf  "%s/sd@%x,%x"  "${of_device_path##/proc/device-tree}" $of_disk_scsi_id $of_disk_scsi_lun)

		;;
		sata)
		file_of_hw_path="${of_device_path##/proc/device-tree}"
		;;
	esac

# no "devspec" available
fi

#
# done
#
file_of_path="$file_of_hw_path"
if [ ! -z "$file_partition" ] ; then
file_of_path="$file_of_hw_path:$file_partition"
fi
if [ "$file" != "/" ] ; then
file_of_path="$file_of_path,$file"
fi
echo $file_of_path
