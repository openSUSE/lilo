#!/bin/bash
#
# find a OF bootpath on Apple PowerMacintosh Newworld machines
# olh@suse.de (2000)
#
# When booting via BootX then all symlinks are gone ...
# The MacOS removes them and BenH didn't find (yet) a way to
# bring them back.
# If it looks like a hack then you don't need fielmann.de
#
# Changes
#
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
echo ERROR: This machine is an Oldworld, no need for firmware pathnames
exit 1
}
unset LILO_ROOT_DRIVE
if [ "$1" == "--lilo-rootdrive" ] ; then
shift
LILO_ROOT_DRIVE=$1
shift
fi
# argument must be a file
FILENAME="$1"
if [ ! "$1" ]; then FILENAME="/" ; fi
#echo try to figure out the OpenFirmware path to $FILENAME

if [ -b $FILENAME ] ; then
# cut /dev/
FILEDEVICENAME=$FILENAME
FILENAME=""
else
if [ ! -z "$LILO_ROOT_DRIVE" ] ; then
 FILEDEVICENAME=$(echo $LILO_ROOT_DRIVE |grep "^/dev/"|cut -d "/" -f 3|sed 's/ .*$//')
 else
 FILEDEVICENAME=$(df "$FILENAME"|grep "^/dev/"|cut -d "/" -f 3|sed 's/ .*$//')
 fi 
fi
#echo FILEDEVICENAME $FILEDEVICENAME
FILE_DEVICE=`echo "$FILEDEVICENAME"|sed 's/[0-9]*$/ &/'|cut -d " " -f 1`
DEVICE_NODENAME=`echo "$FILEDEVICENAME"|sed 's/[0-9]*$/ &/'|cut -d "/" -f 3`
FILE_PARTITION=`echo "$FILEDEVICENAME"|sed 's/[0-9]*$/ &/'|cut -d " " -f 2`
#echo FILE_DEVICE $FILE_DEVICE DEVICE_NODENAME $DEVICE_NODENAME FILE_PARTITION $FILE_PARTITION


# sd* scsi disks , hd* ide disks , sr* cdroms
case "$DEVICE_NODENAME" in
	sd*)
#		echo $FILENAME is on SCSI drive $FILEDEVICENAME
		# did the admin start with bootx? bad on newworld, but maybe we can handle it
		# if there are more then 1 pci folder we are lost
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

	if [ "$PROC_DEVICETREE_BROKEN" != "0" ] ; then 
		echo "ERROR: Don't use BootX on that machine"
	else
		DEVICE_NODENAME=${FILE_DEVICE##*sd}
		FILE_HOST_SUBDEVICE_NUMBER=$[$(echo $DEVICE_NODENAME | tr a-z 0-9)+1]
#		echo looking for $FILE_HOST_SUBDEVICE_NUMBER. scsi disk $DEVICE_NODENAME  =  $FILE_DEVICE

# first we have to figure out the SCSI ID, have to do that anyway
# find the attached scsi disks = "Direct-Access" and stop at sda=1 sdb=2 or whatever
# count in 3 lines steps 
		xcount=0
		for i in $( seq 1 $[ $( grep -v ^Attach /proc/scsi/scsi | wc -l ) /3] ) ; do
# put every scsi device into one single line
			x=$(grep -v ^Attach /proc/scsi/scsi |head -n $[$i*3]|tail -n 3)
#			echo x $x
# cut the type field, expect "Direct-Access"
			xtype=$(echo ${x##*Type: }|cut -d " " -f 1)
#			echo xtype $xtype
			xid=$(echo ${x##*Id: }|cut -d " " -f 1)
			xid=$(echo ${xid#*0})
#			echo xid $xid
			xhost=$(echo ${x##*Host: scsi}|cut -d " " -f 1)
#			echo xhost $xhost 
#			echo result run $i:  xtype $xtype xid $xid xhost $xhost i $i
			if [ "$xtype" = "Direct-Access" ] ; then 
				xcount=$[$xcount+1]				
				if [ "$FILE_HOST_SUBDEVICE_NUMBER" = "$xcount" ] ; then
					DEVICE_HOST=$xhost
					DEVICE_ID=$xid
					break 2
				fi
			fi
		done
#
# now we have the data for /@$xid:$partition,$filename
# lets do the rest
#
# 
		SCSI_DRIVER=$(ls -1 /proc/scsi/*/$DEVICE_HOST |cut -d "/" -f 4) 
		SCSI_HOSTNUMBER=$(ls -1 /proc/scsi/$SCSI_DRIVER/*|grep -n $DEVICE_HOST$|cut -d ":" -f 1)

		case "$SCSI_DRIVER" in
			aic7xxx)
				HOST_LIST=$(for i in `find /proc/device-tree/ -name compatible`;do
				grep -l "\(^ADPT\|^pci900[45]\|^pciclass,01000\)" $i
				done)
				DEVICE_PATH=$(echo ${HOST_LIST%/*}|cut -d " " -f $SCSI_HOSTNUMBER)
				if [ "$FILENAME" = "" ] ; then
				echo ${DEVICE_PATH##*device-tree}/@$DEVICE_ID:$FILE_PARTITION
				else
				echo ${DEVICE_PATH##*device-tree}/@$DEVICE_ID:$FILE_PARTITION,$FILENAME
				fi
			;;
			sym53c8xx)
                                HOST_LIST=$(for i in `find /proc/device-tree/ -name compatible`;do
                                grep -l "\(^Symbios\|^pci1000\|^pciclass,01000\)" $i
                                done)
                                DEVICE_PATH=$(echo ${HOST_LIST%/*}|cut -d " " -f $SCSI_HOSTNUMBER)
				if [ "$FILENAME" = "" ] ; then
				echo ${DEVICE_PATH##*device-tree}/@$DEVICE_ID:$FILE_PARTITION
				else
				echo ${DEVICE_PATH##*device-tree}/@$DEVICE_ID:$FILE_PARTITION,$FILENAME
				fi
			;;
			mesh)
				HOST_LIST=$(for i in `find /proc/device-tree/ -name compatible`;do
                                grep -l "mesh" $i
                                done)
                                DEVICE_PATH=$(echo ${HOST_LIST%/*}|cut -d " " -f $SCSI_HOSTNUMBER)
				if [ "$FILENAME" = "" ] ; then
				echo ${DEVICE_PATH##*device-tree}/@$DEVICE_ID:$FILE_PARTITION
				else
				echo ${DEVICE_PATH##*device-tree}/@$DEVICE_ID:$FILE_PARTITION,$FILENAME
				fi
			;;
			*)
				echo ERROR: driver "$SCSI_DRIVER" not yet supported
			;;
		esac
	fi
	;;
	hda*)
		PATH_IS_CDROM=$(grep "^drive name:" /proc/sys/dev/cdrom/info|grep hda)
		if [ -z "$PATH_IS_CDROM" ] ; then
			HDA_PATH=$(cat /proc/device-tree/aliases/hd)
			echo -n "hd":"$FILE_PARTITION"
		else
			HDA_PATH=$(cat /proc/device-tree/aliases/cd)
			echo -n "cd":"$FILE_PARTITION"
		fi
		if [ "$FILENAME" = "" ] ; then
			echo
		else
			echo ,"$FILENAME"
		fi 
	;;
	hdb*)
		PATH_IS_CDROM=$(grep "^drive name:" /proc/sys/dev/cdrom/info|grep hdb)
		if [ -z "$PATH_IS_CDROM" ] ; then
			HDA_PATH=$(cat /proc/device-tree/aliases/ultra1)
			echo -n "ultra1":"$FILE_PARTITION"
		else
			HDA_PATH=$(cat /proc/device-tree/aliases/cd)
			echo -n "cd":"$FILE_PARTITION"
		fi
		if [ "$FILENAME" = "" ] ; then
			echo
		else
			echo ,"$FILENAME"
		fi 
	;;
	hde*)
		echo "ERROR: hde not configured, please edit $0"
		exit 1
#	choose the boot partition on the /dev/hdeX volume as active boot partition
#	in the Startup Disk control panel in MacOS
#	look in /proc/device-tree/options/boot-device for the correct string
#	the string __could__ look like that:
#	/pci@f2000000/pci-bridge@d/mac-io@7/ata-4@1f000/@0:9,\\:tbxi
#							  ^ cut here
#	put "/pci@f2000000/pci-bridge@d/mac-io@7/ata-4@1f000/@0" in the HDE_PATH variable

			HDE_PATH="blah"
			echo -n "$HDE_PATH":"$FILE_PARTITION"
			if [ "$FILENAME" = "" ] ; then
				echo
			else
				echo ,"$FILENAME"
			fi 
	;;
	hd*)
                PATH_IS_CDROM=$(grep "^drive name:" /proc/sys/dev/cdrom/info|grep ${DEVICE_NODENAME% *})
                if [ -z "$PATH_IS_CDROM" ] ; then
                        echo ERROR: device ${DEVICE_NODENAME% *} not yet supported
                else
                        HDA_PATH=$(cat /proc/device-tree/aliases/cd)
                        echo -n "cd":"$FILE_PARTITION"
			if [ "$FILENAME" = "" ] ; then
				echo
			else
				echo ,"$FILENAME"
			fi 
                fi
	;;
	*)
		echo ERROR: device ${DEVICE_NODENAME% *} not yet supported
	;;
esac
