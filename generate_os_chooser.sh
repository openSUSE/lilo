#!/bin/bash
#
# find a OF bootpath on Apple PowerMacs Newworld machines
# olh@suse.de (2000)
#
# When booting via BootX then all symlinks are gone ...
# The MacOS removes them and BenH didn't find (yet) a way to
# bring them back.
# If it looks like a hack then you don't need fielmann.de
#
# Changes
#
# 2000-05-30  do the boot stuff
# 2000-04-26  get rid of strings
# 2000-04-13  get rid of awk, no success
# 2000-03-28  fix scsi on new macs when boot via bootx
# 2000-02-21  giving up and guess it all from aliases ;-)
# 2000-02-20  finish scsi host, works only with one host for now ...
# 2000-02-01  find ide hosts
# 2000-01-30  first try with scsi hosts
#

# This script expects the extistance of hfsutils and the files System, Finder, yaboot and yaboot.conf 
# on your machine.

# This boot mechanism is only for NewWorld machines (iMac and later) with "ROM on disk"

if [ -f /proc/device-tree/openprom/model ] ; then
	if  ( echo `cat /proc/device-tree/openprom/model`|grep -q OpenFirmware ) ; then 
		echo  # running on a newworld machine 
	else
		echo you need a NewWorld machine for this. 
		exit 1
	fi
fi



function display_usage () {
echo
echo This script generates the file os-chooser and 
echo it do small modifications to your yaboot.conf
echo call it with 3 arguments, separated by a space:
echo
echo $0  linuxboot_partition  macos_partition  hfs_folder
echo
echo linuxboot_partition is the partition where yaboot and os-chooser is
echo macos_partition is the partition of your MacOS installation
echo hfs_folder is the name of the faked system folder, usualy  suseboot
echo
echo this script requires the hfsutils to access the special HFS features.
echo
}


function get_OF_pathname () {

FILENAME="$1"
# cut /dev/
FILEDEVICE=$(echo "$FILENAME"|grep "^/dev/"|cut -d "/" -f 3|sed 's/ .*$//')

# cut  a-z and 0-9
FILEPARTITION=$(echo $FILEDEVICE | sed 's/[a-z]*//')
FILE_HOST_DEVICE=$(echo $FILEDEVICE | sed 's/[0-9].*//')
#echo $FILEDEVICE $FILE_HOST_DEVICE

# sd* scsi disks , hd* ide disks , sr* cdroms
case "$FILE_HOST_DEVICE" in
	sd*)
#		echo $FILENAME is on SCSI drive $FILEDEVICE
		PROC_DEVICETREE_BROKEN="0"
		PROC_DEVICETREE_SYMLINKS=$(find /proc/device-tree/ -maxdepth 1 -type l)
		if [ ! "$PROC_DEVICETREE_SYMLINKS" ] ; then
			PROC_DEVICETREE_PCIENTRY=$(find /proc/device-tree/ -name pci -maxdepth 1|wc -l)
			if [ "$PROC_DEVICETREE_PCIENTRY" -gt 1 ] ; then
				echo "There is no symlink in /proc/device-tree/ and"
				echo "there is more than one pci directory in /proc/device-tree/."
				echo "Booting via BootX removes them and I can't figure out SCSI disks "
				echo "I will try /proc/device-tree/chosen/bootpath"
				PROC_DEVICETREE_BROKEN="$(cat /proc/device-tree/chosen/bootpath|grep -v ata|sed 's/.:.*$//')"
				if [ ! "$PROC_DEVICETREE_BROKEN" ] ; then
					echo "There is 'ata' in /proc/device-tree/chosen/bootpath"
					echo "Seems not to be a SCSI boot drive"
					echo -n "ERROR ... "
				fi
			fi
		fi

	if [ "$PROC_DEVICETREE_BROKEN" = "0" ] ; then 
		FILE_HOST_SUBDEVICE=${FILE_HOST_DEVICE##sd}
		FILE_HOST_SUBDEVICE_NUMBER=$[$(echo $FILE_HOST_SUBDEVICE | tr a-z 0-9)+1]
#		echo looking for $FILE_HOST_SUBDEVICE_NUMBER. scsi disk

# find the attached scsi disks = "Direct-Access" and stop at sda=1 sdb=2 or whatever
# count in 3 lines steps 
		xcount=0
		for i in $( seq 1 $[ $( grep -v ^Attach /proc/scsi/scsi | wc -l ) /3] ) ; do
			x=$(grep -v ^Attach /proc/scsi/scsi |head -n $[$i*3]|tail -n 3)
			xtype=$(echo $x|sed 's/^.*Type://'|awk '$1 { print $1}')
			xid=$(echo $x|sed 's/^.*Id: //'|awk '$1 ~ /[0-9]*/ { print $1 }'|sed 's/^0//')
			xhost=$(echo $x|sed 's/^.*Host: //'|awk '$1 { print $1 }'|sed 's/^[a-z]*//')
#			echo x $x 
#			echo xtype $xtype xid $xid xhost $xhost i $i
#			echo xcount $xcount
			if [ "$xtype" = "Direct-Access" ] ; then 
				xcount=$[$xcount+1]				
				if [ "$FILE_HOST_SUBDEVICE_NUMBER" = "$xcount" ] ; then
					break 2
				fi
			fi
		done
#		echo $xcount xcount $xhost xhost
		SCSI_HOSTS=$(/sbin/lspci -n|grep "\(Class 0100:\|Class 0000:\)"|wc -l)
# echo $SCSI_HOSTS SCSI_HOSTS
		for i in $(seq 1 $SCSI_HOSTS);do
			SCSI_HOST_TMP=$(/sbin/lspci -n|grep "\(Class 0100:\|Class 0000:\)"|cut -d " " -f 4) 
			SCSI_HOST_VENDOR_[$i]=$(echo $SCSI_HOST_TMP|cut -d ":" -f 1)
			SCSI_HOST_DEVICE_["$i"]=$(echo $SCSI_HOST_TMP|cut -d ":" -f 2)
#			echo ${SCSI_HOST_DEVICE_[$i]} ${SCSI_HOST_VENDOR_[$i]} $SCSI_HOST_TMP
		done

		xcount=1
		for i in `find /proc/device-tree -name vendor-id` ; do
			i=`dirname $i`
#			echo i $i
			for x in $( seq 1 $SCSI_HOSTS) ; do
			VENDOR_ID_TMP=$(hexdump -n 4 -s 2 $i/vendor-id|head -n 1|awk '$2 { print $2 }')
			DEVICE_ID_TMP=$(hexdump -n 4 -s 2 $i/device-id|head -n 1|awk '$2 { print $2 }')
#			echo DEVICE_ID_TMP $DEVICE_ID_TMP VENDOR_ID_TMP $VENDOR_ID_TMP
			if [ "$VENDOR_ID_TMP" = "${SCSI_HOST_VENDOR_[$x]}" -a "$DEVICE_ID_TMP" = "${SCSI_HOST_DEVICE_[$x]}" ] ; then
			OF_DEVICE_[$xcount]=$(echo $i|sed 's/^\/.*device-tree//')
#				echo i $i   blah --------------------------
#				echo OF_DEVICE ${OF_DEVICE_[$xcount]}
				xcount=$[$xcount+1]
				break 2
			fi
			done
		done
#		echo $xcount
	fi
#echo $FILENAME
#echo $FILEDEVICE
#echo $FILEPARTITION
#echo $FILE_HOST_DEVICE
#echo $FILE_HOST_SUBDEVICE
#echo $FILE_HOST_SUBDEVICE_NUMBER

# echo possible blubb is:
#for i in $(seq 1 $SCSI_HOSTS) ; do
#	echo "${OF_DEVICE_[$i]}"/@"$xid":"$FILEPARTITION","$FILENAME"
#done
echo "${OF_DEVICE_[1]}"/@"$xid":"$FILEPARTITION",

	;;
	hda)
#		echo trying hda
		PATH_IS_CDROM=$(grep "^drive name:" /proc/sys/dev/cdrom/info|grep hda)
#		echo $PATH_IS_CDROM
		if [ -z "$PATH_IS_CDROM" ] ; then
#			echo blubb hda
			HDA_PATH=$(cat /proc/device-tree/aliases/ultra0)
			echo "ultra0":"$FILEPARTITION",
		else
#			echo blah hda
			HDA_PATH=$(cat /proc/device-tree/aliases/cd)
			echo "cd":"$FILEPARTITION",
		fi
	;;
	hdb)
#		echo trying hdb
		PATH_IS_CDROM=$(grep "^drive name:" /proc/sys/dev/cdrom/info|grep hdb)
#		echo $PATH_IS_CDROM
		if [ -z "$PATH_IS_CDROM" ] ; then
#			echo blubb hda
			HDA_PATH=$(cat /proc/device-tree/aliases/ultra1)
			echo "ultra1":"$FILEPARTITION",
		else
#			echo blah hda
			HDA_PATH=$(cat /proc/device-tree/aliases/cd)
			echo "cd":"$FILEPARTITION",
		fi
	;;
	scd*|sr)
		echo SCSI CDROM $FILE_HOST_DEVICE
		echo not yet implemented
		exit 1
	;;
esac

# echo waiting for the patch.
}


# here we go ...

# argument must be present and a partition
if [ -z "$1" -o  ! -b "$1" ] ; then display_usage  ; exit 1 ; fi
if [ -z "$2" -o  ! -b "$2" ] ; then 
	echo 
	echo please specify the partition of your MacOS installation
	display_usage  
	exit 1 
fi
if [ -z "$3" ] ; then 
echo 
echo no folder specified, asume  suseboot
SUSEBOOT_FOLDER="suseboot"
else 
SUSEBOOT_FOLDER="$3"
fi

LINUXBOOT_PARTITION="$1"
MACOS_PARTITION="$2"

LINUXBOOT_PATH=$(get_OF_pathname $LINUXBOOT_PARTITION)
echo LinuxBoot-Partition OpenFirmware path = $LINUXBOOT_PATH
MACOS_PATH=$(get_OF_pathname $MACOS_PARTITION)
echo MacOS-Partition OpenFirmware path = $MACOS_PATH

(echo "<CHRP-BOOT>"
echo "<COMPATIBLE>"
echo "iMac,1 PowerMac1,1 PowerBook1,1 PowerMac2,1 PowerMac3,1 PowerBook2,1 PowerBook3,1"
echo "</COMPATIBLE>"
echo "<DESCRIPTION>"
echo "Linux/PPC Yaboot bootloader"
echo "</DESCRIPTION>"
echo "<BOOT-SCRIPT>"
echo "\" get-key-map\" \" keyboard\" open-dev \$call-method"
echo "dup 20 dump"
echo "5 + c@ 08 = if"
echo "\" Booting MacOS ...\" cr \" boot $MACOS_PATH\\\\:tbxi\" eval"
echo "else"
echo "\" Booting Yaboot ...\" cr \" boot $LINUXBOOT_PATH\\\\yaboot\" eval"
echo "then"
echo "</BOOT-SCRIPT>"
echo "</CHRP-BOOT>") > /tmp/os-chooser-temp.file

echo placed os-chooser temporary in /tmp/os-chooser-temp.file

echo ----
echo Now I start the real work and look for a folder \"$SUSEBOOT_FOLDER\" on the partition $LINUXBOOT_PARTITION 
echo I assume you have the package hfsutils installed on your system: rpm -q hfsutils
rpm -q hfsutils
echo "It is needed to set the file attributes for MacOS to make this partition  bootable"
echo
echo LinuxBoot Partition $LINUXBOOT_PARTITION
echo MacOS Partition $MACOS_PARTITION
echo Folder that contains the bootfiles $SUSEBOOT_FOLDER
echo
echo "Hit Control C to abort, I will continue in 10 seconds ... and do my job."

sleep 13 

# now we do the hack stuff ...
IS_MOUNTED=""
mount | grep "$LINUXBOOT_PARTITION">/dev/null && IS_MOUNTED=$(mount|grep "$LINUXBOOT_PARTITION"|cut -d " " -f 3); echo Your LinuxBoot-Partition $LINUXBOOT_PARTITION is mounted on $IS_MOUNTED 

if [ "$IS_MOUNTED" = "" ] ; then
	mkdir -p /tmp/os-chooser_temp_mount
	echo "trying to mount your LinuxBoot-Partition  $LINUXBOOT_PARTITION as hfs (!!!)"
	mount -t hfs $LINUXBOOT_PARTITION /tmp/os-chooser_temp_mount
	cp -va /tmp/os-chooser-temp.file /tmp/os-chooser_temp_mount/"$SUSEBOOT_FOLDER"/os-chooser
	perl -p -e 's#(^|\x0d)default[^\x0d]*#default = linux#' < /tmp/os-chooser_temp_mount/"$SUSEBOOT_FOLDER"/yaboot.conf >  /tmp/yaboot.conf.temp
	(cat /tmp/yaboot.conf.temp 
	echo -n "     root = "&&df /|tail -n 1|cut -f 1 -d " ") >/tmp/os-chooser_temp_mount/"$SUSEBOOT_FOLDER"/yaboot.conf
	
	umount /tmp/os-chooser_temp_mount
else
	cp -va /tmp/os-chooser-temp.file $IS_MOUNTED/"$SUSEBOOT_FOLDER"/os-chooser
        perl -p -e 's#(^|\x0d)default[^\x0d]*#default = linux#' < $IS_MOUNTED/"$SUSEBOOT_FOLDER"/yaboot.conf > /tmp/yaboot.conf.temp
        (cat /tmp/yaboot.conf.temp
        echo -n "     root = "&&df /|tail -n 1|cut -f 1 -d " ") >$IS_MOUNTED/"$SUSEBOOT_FOLDER"/yaboot.conf
fi

echo trying to hmount the LinuxBoot Partition and set proper Type/Creators 
hmount $LINUXBOOT_PARTITION
hattrib -b "$SUSEBOOT_FOLDER"
hcd "$SUSEBOOT_FOLDER"
hattrib -t tbxi -c chrp os-chooser
hattrib -t FNDR -c MACS Finder
hattrib -t zsys -c MACS System
hattrib -t TEXT -c "R*ch" yaboot.conf
hattrib -t BINA -c UNIX yaboot
humount

