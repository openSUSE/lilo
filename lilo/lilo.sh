#!/bin/bash
#
# a simple lilo to store the boot loader and the kernel images 
# in bash2 ... Think different [tm]
#
# olh@suse.de
#

#CONFIG_LILO_CONF=~olh/ppc/lilo/lilo.conf
CONFIG_LILO_CONF=/etc/lilo.conf
SHOW_OF_PATH_SH=/bin/show_of_path.sh
DEFAULT_BOOTFOLDER=linuxboot





function running_on_chrp () {

echo running on chrp

#only the device is given and dd to the raw device is a bad idea

if [ "$OPTION_PARTITION" = "" ] ; then
  echo guess the chrp boot device 
  PART=`fdisk -l $OPTION_DEVICE | fgrep "PPC PReP"`
  if [ -z "$PART" ] ; then
     echo ERROR: config error, boot = $OPTION_BOOT is not 41 PReP  
     echo '*********************************************************'
     echo '* You must create a PPC PReP Boot partition (type 0x41) *'
     echo '* for the CHRP bootloader to be installed.              *'
     echo '*********************************************************'
  exit -1
  fi
  if [ `echo "$PART" | wc -l` != 1 ] ; then
     echo "ERROR: config error, guessing of boot partition failed"
     echo "**************************************************************"
     echo "* There are more than 1 PPC PReP Boot partitions (type 0x41) *"
     echo "* on this system. Specify the partition NUMBER in boot= $OPTION_BOOT *"
     echo "**************************************************************"
   exit -1
  fi
  P=`echo "$PART" | awk '{print $1}'`

  echo Installing /boot/second onto $P
dd if=/boot/second of=$P
  if [ "$OPTION_ACTIVATE" = "yes" ] ; then
/sbin/activate $(echo "$P"|sed 's/[0-9]*$/ &/')
  fi
else
#we have the device, but better safe than sorry
  echo install on $OPTION_DEVICE
  PART=`fdisk -l $OPTION_DEVICE | grep $OPTION_BOOT | fgrep "PPC PReP"`
  if [ -z "$PART" ] ; then
     echo "ERROR: config error, boot = $OPTION_BOOT is not 41 PReP"
     echo "*********************************************************"
     echo "* You must create a PPC PReP Boot partition (type 0x41) *"
     echo "* for the CHRP bootloader to be installed.              *"
     echo "*********************************************************"
  exit -1
  fi
  echo Installing /boot/second onto $OPTION_BOOT
dd if=/boot/second of=$OPTION_BOOT
  if [ "$OPTION_ACTIVATE" = "yes" ] ; then
/sbin/activate $(echo "$OPTION_BOOT"|sed 's/[0-9]*$/ &/')
  fi
fi

echo check the image files
for i in `seq 1 $CONFIG_IMAGE_COUNT` ; do
if test -f ${CONFIG_IMAGE_FILE[$i]} ; then
	if df ${CONFIG_IMAGE_FILE[$i]}|grep -sq ^$OPTION_DEVICE ; then
	echo -n ${CONFIG_IMAGE_LABEL[$i]}
	test "${CONFIG_IMAGE_LABEL[$i]}" = "$OPTION_DEFAULT" && echo " *" || echo
	else
	echo ERROR: image ${CONFIG_IMAGE_FILE[$i]} is not on bootdevice $OPTION_DEVICE
	fi
else
	echo ERROR: image ${CONFIG_IMAGE_FILE[$i]} in ${CONFIG_IMAGE_LABEL[$i]} is missing
fi
done
}


function running_on_prep () {

echo running on prep

}


function running_on_pmac_old () {

echo running on pmac_old


for i in `seq 1 $CONFIG_IMAGE_COUNT` ; do
test "${CONFIG_IMAGE_LABEL[$i]}" = "$OPTION_DEFAULT" || continue
DEVICENAME=${CONFIG_IMAGE_ROOT[$i]}
done
echo $DEVICENAME > /tmp/ppc_lilo/myrootdevice

# gnereate a generic ramdisk
gzip -vcd /boot/initrd.pmacold.gz > /tmp/ppc_lilo/initrd
mount -o rw,loop /tmp/ppc_lilo/initrd /tmp/ppc_lilo/ramdisk
cp -av /tmp/ppc_lilo/myrootdevice /tmp/ppc_lilo/ramdisk/myrootdevice
umount /tmp/ppc_lilo/ramdisk
gzip -9v /tmp/ppc_lilo/initrd

# umount the boot = partition, or exit if that fails
mount | grep -q "$OPTION_BOOT"
if [ "$?" = "0" ] ; then 
echo "unmount $OPTION_BOOT" ; umount $OPTION_BOOT || exit 1
fi
humount $OPTION_BOOT 2>/dev/null
humount $OPTION_BOOT 2>/dev/null

hmount $OPTION_BOOT  || exit 1
if [ "$OPTION_BOOTFOLDER" != "" ] ; then
HFS_BOOTFOLDER="$OPTION_BOOTFOLDER"
else
HFS_BOOTFOLDER="$DEFAULT_BOOTFOLDER"
fi
hmkdir $HFS_BOOTFOLDER 2>/dev/null
hattrib -b $HFS_BOOTFOLDER
hcd $HFS_BOOTFOLDER
hcopy /boot/Finder.bin :Finder
hcopy /boot/System.bin :System
hattrib -t FNDR -c MACS Finder
hattrib -t zsys -c MACS System

for i in `seq 1 $CONFIG_IMAGE_COUNT` ; do
test "${CONFIG_IMAGE_LABEL[$i]}" = "$OPTION_DEFAULT" || continue
hcopy ${CONFIG_IMAGE_FILE[$i]} :vmlinux
test -z "${CONFIG_IMAGE_INITRD[$i]}" || hcopy ${CONFIG_IMAGE_INITRD[$i]} :ramdisk.image.gz && hcopy /tmp/ppc_lilo/initrd.gz :ramdisk.image.gz
done
hpwd
hls -ltr
humount

}


function running_on_pmac_new () {

echo running on pmac_new

# build the temp yaboot.conf

# build the pathnames, copy the files to bootfolder if / is not bootable
for i in `seq 1 $CONFIG_IMAGE_COUNT` ; do
unset FILE_PATH
# check if the file is a real file
test -f ${CONFIG_IMAGE_FILE[$i]} && FILE_PATH=$($SHOW_OF_PATH_SH ${CONFIG_IMAGE_FILE[$i]}|grep -v /pci[0-9])
if [ "$FILE_PATH" = "" -o "${CONFIG_IMAGE_COPY[$i]}" = "true" ] ; then
        CONFIG_IMAGE_PATH[$i]="copy"
else
	CONFIG_IMAGE_PATH[$i]=$FILE_PATH
fi
unset FILE_PATH
if [ ! -z "${CONFIG_IMAGE_INITRD[$i]}" ] ; then 
	FILE_PATH=$($SHOW_OF_PATH_SH ${CONFIG_IMAGE_INITRD[$i]}|grep -v /pci[0-9])
	if [ "$FILE_PATH" = "" -o "${CONFIG_IMAGE_COPY[$i]}" = "true" ] ; then
        	CONFIG_IMAGE_INITRDPATH[$i]="copy"
	else
		CONFIG_IMAGE_INITRDPATH[$i]=$FILE_PATH
	fi
else
	continue
fi
done

# starting the work
(
test -z "$OPTION_TIMEOUT" || echo "timeout = $OPTION_TIMEOUT"
test -z "$OPTION_DEFAULT" || echo "default = $OPTION_DEFAULT"
test -z "$OPTION_ROOT"    || echo "root = $OPTION_ROOT"
test -z "$OPTION_APPEND"  || echo "append = $OPTION_APPEND"
test -z "$OPTION_INITRD"  || echo "initrd = $OPTION_INITRD"

for i in `seq 1 $CONFIG_IMAGE_COUNT` ; do
test -z "${CONFIG_IMAGE_OTHER[$i]}" || continue
if [ "${CONFIG_IMAGE_PATH[$i]}" = "copy" ] ; then
	echo "image = `basename ${CONFIG_IMAGE_FILE[$i]}`"
else
	echo "image = ${CONFIG_IMAGE_PATH[$i]}"
fi
test -z "${CONFIG_IMAGE_LABEL[$i]}"  || echo "    label = ${CONFIG_IMAGE_LABEL[$i]}"
test -z "${CONFIG_IMAGE_ROOT[$i]}"   || echo "    root = ${CONFIG_IMAGE_ROOT[$i]}"
test -z "${CONFIG_IMAGE_APPEND[$i]}" || echo "    append = ${CONFIG_IMAGE_APPEND[$i]}"
test -z "${CONFIG_IMAGE_INITRD[$i]}" || ( if [ "${CONFIG_IMAGE_INITRDPATH[$i]}" = "copy" ] ; then
	echo "    initrd = `basename ${CONFIG_IMAGE_INITRD[$i]}`" 
else 
	echo "    initrd = ${CONFIG_IMAGE_INITRDPATH[$i]}" 
fi )
done
) > /tmp/ppc_lilo/yaboot.conf

BOOT_DEVICEPATH=$($SHOW_OF_PATH_SH $OPTION_BOOT)
OTHER_DEVICEPATH=$($SHOW_OF_PATH_SH $OPTION_OTHER)

echo "BOOT_DEVICEPATH  =  $BOOT_DEVICEPATH"
echo "OTHER_DEVICEPATH  =  $OTHER_DEVICEPATH"
(echo "<CHRP-BOOT>
<COMPATIBLE>
iMac,1 PowerMac1,1 PowerBook1,1 PowerMac2,1 PowerMac3,1 PowerBook2,1 PowerBook3,1
</COMPATIBLE>
<DESCRIPTION>
Linux/PPC Yaboot bootloader
</DESCRIPTION>
<BOOT-SCRIPT>"
if [ "$CONFIG_PARSE_HASOTHER" = "true" ] ; then
echo "\" get-key-map\" \" keyboard\" open-dev \$call-method
dup 20 dump
5 + c@ 08 = if
\" Booting MacOS ...\" cr \" boot $OTHER_DEVICEPATH\\\\:tbxi\" eval
else
\" Booting Yaboot ...\" cr \" boot $BOOT_DEVICEPATH\\\\yaboot\" eval
then
</BOOT-SCRIPT>
</CHRP-BOOT>"
else
echo "\" Booting Yaboot ...\" cr \" boot $BOOT_DEVICEPATH\\\\yaboot\" eval
</BOOT-SCRIPT>
</CHRP-BOOT>"
fi) > /tmp/ppc_lilo/os-chooser

# umount the boot = partition, or exit if that fails
mount | grep -q "$OPTION_BOOT"
if [ "$?" = "0" ] ; then 
echo "unmount $OPTION_BOOT" ; umount $OPTION_BOOT || exit 1
fi
humount $OPTION_BOOT 2>/dev/null
humount $OPTION_BOOT 2>/dev/null

hmount $OPTION_BOOT || exit 1
if [ "$OPTION_BOOTFOLDER" != "" ] ; then
HFS_BOOTFOLDER="$OPTION_BOOTFOLDER"
else
HFS_BOOTFOLDER="$DEFAULT_BOOTFOLDER"
fi
hmkdir $HFS_BOOTFOLDER 2>/dev/null
hattrib -b $HFS_BOOTFOLDER
hcd $HFS_BOOTFOLDER
hcopy /tmp/ppc_lilo/os-chooser :os-chooser
hcopy /tmp/ppc_lilo/yaboot.conf :yaboot.conf
hcopy /boot/yaboot :yaboot
hcopy /boot/Finder.bin :Finder
hcopy /boot/System.bin :System
hattrib -t tbxi -c chrp os-chooser
hattrib -t FNDR -c MACS Finder
hattrib -t zsys -c MACS System
hattrib -t TEXT -c "R*ch" yaboot.conf
hattrib -t BINA -c UNIX yaboot

for i in `seq 1 $CONFIG_IMAGE_COUNT` ; do
test -z "${CONFIG_IMAGE_OTHER[$i]}" || continue
if [ "${CONFIG_IMAGE_PATH[$i]}" = "copy" ] ; then
        hcopy ${CONFIG_IMAGE_FILE[$i]} :`basename ${CONFIG_IMAGE_FILE[$i]}`
fi
test -z "${CONFIG_IMAGE_INITRD[$i]}" || (
 if [ "${CONFIG_IMAGE_INITRDPATH[$i]}" = "copy" ] ; then
        hcopy ${CONFIG_IMAGE_INITRD[$i]} :`basename ${CONFIG_IMAGE_INITRD[$i]}`
 fi )
done
hpwd
hls -ltr
humount
}


function check_arch () {
# check for the current ppc subarch

while read line; do
	case "$line" in
		*MacRISC*)	MACHINE="pmac" ;;
		*CHRP*)		MACHINE="chrp" ;;
		*PReP*)		MACHINE="prep" ;;
	esac
done < /proc/cpuinfo


if [ "$MACHINE" = "pmac" ] ; then
	if [ -f /proc/device-tree/openprom/model ] ; then
		echo `cat /proc/device-tree/openprom/model` > /tmp/ppc_lilo/openprom_model
		while read openfirmware ofversion; do
	        	case "$openfirmware" in
	                OpenFirmware)      MACHINE="pmac_new" ;;
	                Open)      MACHINE="pmac_old" ;;
	        	esac
		done < /tmp/ppc_lilo/openprom_model
	fi
fi

} #end function check_arch


function prepare_config_file () {
# strip comments and empty lines

grep -v "#.*$" < $CONFIG_LILO_CONF | grep -v "\(^\W$\|^$\)" | sed 's/=/ & /' > /tmp/ppc_lilo/config_tmp


} #end function prepare_config_file


function parse_config_file () {
# parse the lilo.conf and place it in CONFIG_IMAGE_FILE[]
# other vars:
# OPTION_BOOT contains the bootloader partition
# OPTION_OTHER contains the MacOS partition
# OPTION_BOOTFOLDER contains the MacOS folder with the bootstuff
# OPTION_ACTIVATE is a flag whether or not the boot partition must be set active
# OPTION_TIMEOUT contains the timeout variable in seconds
# OPTION_DEFAULT contains the default label
# OPTION_ROOT contains the global or local root= device
# OPTION_APPEND contains the global or local append= strings
# OPTION_INITRD containes the global or local initrd filename
# OPTION_IMAGE_COPY contains a flag to force copy to the boot partition
# internal vars:
# CONFIG_PARSE_HASIMAGE is a flag if we have a image section.
# CONFIG_IMAGE_FILE contains the kernel image for a section
# CONFIG_IMAGE_OTHER contains the device of MacOS
# CONFIG_IMAGE_COUNT is a simple counter of image sections


unset CONFIG_PARSE_HASIMAGE
CONFIG_IMAGE_COUNT=0
while read option sarator value ; do

#	echo option "$option"
#	echo sarator "$sarator"
#	echo value "$value"
	case "$option" in
		boot)
			if [ -z "$CONFIG_PARSE_HASIMAGE" ] ; then
				OPTION_BOOT="$value"
			else 
				echo boot option must not in an image section!
				exit 1
			fi
		;;
		activate)
			if [ -z "$CONFIG_PARSE_HASIMAGE" ] ; then
				OPTION_ACTIVATE="$value"
			else 
				echo activate option must not in an image section!
				exit 1
			fi
		;;
		bootfolder)
			if [ -z "$CONFIG_PARSE_HASIMAGE" ] ; then
				OPTION_BOOTFOLDER="$value"
			else 
				echo bootfolder option must not in an image section!
				exit 1
			fi
		;;
		timeout)
                       if [ -z "$CONFIG_PARSE_HASIMAGE" ] ; then
                                OPTION_TIMEOUT="$value"
                        else
                                echo timeout option must not in an image section!
                                exit 1
                        fi
		;;
		default)
                       if [ -z "$CONFIG_PARSE_HASIMAGE" ] ; then
                                OPTION_DEFAULT="$value"
                        else
                                echo default option must not in an image section!
                                exit 1
                        fi
		;;
		image)
			# check if previous image section has a label
			if [ ! -z "$CONFIG_PARSE_HASIMAGE" ] ; then
				if [ -z "${CONFIG_IMAGE_LABEL[$CONFIG_IMAGE_COUNT]}" ] ; then
				echo ERROR: config error, no label in image = ${CONFIG_IMAGE_FILE[$CONFIG_IMAGE_COUNT]}
				fi
			fi
			CONFIG_PARSE_HASIMAGE=true
			CONFIG_IMAGE_COUNT=$[CONFIG_IMAGE_COUNT+1]
			CONFIG_IMAGE_FILE[$CONFIG_IMAGE_COUNT]="$value"
		;;
		other)
                        # check if previous image section has a label
                        if [ ! -z "$CONFIG_PARSE_HASIMAGE" ] ; then
                                if [ -z "${CONFIG_IMAGE_LABEL[$CONFIG_IMAGE_COUNT]}" ] ; then
                                echo ERROR: config error, no label in image = ${CONFIG_IMAGE_FILE[$CONFIG_IMAGE_COUNT]}
                                fi
                        fi
                        CONFIG_PARSE_HASIMAGE=true
                        CONFIG_PARSE_HASOTHER=true
                        CONFIG_IMAGE_COUNT=$[CONFIG_IMAGE_COUNT+1]
                        CONFIG_IMAGE_OTHER[$CONFIG_IMAGE_COUNT]="$value"
                        OPTION_OTHER="$value"
                ;;
		root)
			if [ -z "$CONFIG_PARSE_HASIMAGE" ] ; then
				OPTION_ROOT="$value"
			else
				CONFIG_IMAGE_ROOT[$CONFIG_IMAGE_COUNT]="$value"
			fi
		;;
		copy)
			if [ -z "$CONFIG_PARSE_HASIMAGE" ] ; then
                                echo ERROR: config error, label option must be in an image section!
                                exit 1
			else
				CONFIG_IMAGE_COPY[$CONFIG_IMAGE_COUNT]="true"
			fi
		;;
		label)
                        if [ -z "$CONFIG_PARSE_HASIMAGE" ] ; then
                                echo ERROR: config error, label option must be in an image section!
                                exit 1
                        else
                                CONFIG_IMAGE_LABEL[$CONFIG_IMAGE_COUNT]="$value"
                        fi

		;;
		append)
                        if [ -z "$CONFIG_PARSE_HASIMAGE" ] ; then
                                OPTION_APPEND="$value"
                        else
                                CONFIG_IMAGE_APPEND[$CONFIG_IMAGE_COUNT]="$value"
                        fi

		;;
		initrd)
                        if [ -z "$CONFIG_PARSE_HASIMAGE" ] ; then
                                OPTION_INITRD="$value"
                        else
                                CONFIG_IMAGE_INITRD[$CONFIG_IMAGE_COUNT]="$value"
                        fi
		;;		
		*)
			echo !!!!!!!!!! unkown option $option !!!!!!!!!!!!!
		;;
	esac
done < /tmp/ppc_lilo/config_tmp


} #end function parse_config_file


function check_config_file () {


if test ! -b $OPTION_BOOT  ; then
echo ERROR: boot = $OPTION_BOOT is not a valid block device
exit -1 
fi

if [ -z "$CONFIG_PARSE_HASIMAGE" ] ; then
echo ERROR: no image section is specified
exit -1
fi

for i in `seq 1 $CONFIG_IMAGE_COUNT` ; do
if [ ! -f ${CONFIG_IMAGE_FILE[$i]} ] ; then
echo "ERROR: image = ${CONFIG_IMAGE_FILE[$i]} ist not a regular file"
exit -1
fi
if [ ! -f ${CONFIG_IMAGE_INITRD[$i]} ] ; then
echo "ERROR: initrd = ${CONFIG_IMAGE_INITRD[$i]} ist not a regular file"
exit -1
fi
done


OPTION_DEVICE=`echo "$OPTION_BOOT"|sed 's/[0-9]*$/ &/'|cut -d " " -f 1`
OPTION_NODENAME=`echo "$OPTION_DEVICE"|sed 's/[0-9]*$/ &/'|cut -d "/" -f 3`
OPTION_PARTITION=`echo "$OPTION_BOOT"|sed 's/[0-9]*$/ &/'|cut -d " " -f 2`



} #end function check_config_file



function prepare_envoirement () {

rm -f /tmp/ppc_lilo/*
mkdir -p /tmp/ppc_lilo/ramdisk

} #end function prepare_envoirement

#
# here we go
#

prepare_envoirement
prepare_config_file
parse_config_file
check_config_file
check_arch

case "$MACHINE" in
	pmac_new)  running_on_pmac_new ;;
	pmac_old)  running_on_pmac_old ;;
	chrp)	   running_on_chrp     ;;
	prep)	   running_on_prep     ;;
esac

#
#for i in `seq 1 $CONFIG_IMAGE_COUNT`;do
#	echo section $i:
#	echo image  ${CONFIG_IMAGE_FILE[$i]}
#	echo label  ${CONFIG_IMAGE_LABEL[$i]}
#	echo append ${CONFIG_IMAGE_APPEND[$i]}
#	echo initrd ${CONFIG_IMAGE_INITRD[$i]}
#	echo root   ${CONFIG_IMAGE_ROOT[$i]}
#done
#
# set | less
