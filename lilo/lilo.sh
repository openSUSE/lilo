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
DEFAULT_BOOTFOLDER=suseboot
COMPATIBLE_LIST=/boot/compatible_machines.txt





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

echo Installing /boot/yaboot.chrp onto $P
dd if=/boot/yaboot.chrp of=$P

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
echo Installing /boot/yaboot.chrp onto $OPTION_BOOT
dd if=/boot/yaboot.chrp of=$OPTION_BOOT
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
# starting the work and write the yaboot.conf
(
test -z "$OPTION_TIMEOUT" || echo "timeout = $OPTION_TIMEOUT"
test -z "$OPTION_DEFAULT" || echo "default = $OPTION_DEFAULT"
test -z "$OPTION_ROOT"    || echo "root = $OPTION_ROOT"
test -z "$OPTION_APPEND"  || echo "append = $OPTION_APPEND"
test -z "$OPTION_INITRD"  || echo "initrd = $OPTION_INITRD"

for i in `seq 1 $CONFIG_IMAGE_COUNT` ; do
test -z "${CONFIG_IMAGE_OTHER[$i]}" || continue
	echo "image = ${CONFIG_IMAGE_FILE[$i]}"
test -z "${CONFIG_IMAGE_LABEL[$i]}"  || echo "    label = ${CONFIG_IMAGE_LABEL[$i]}"
test -z "${CONFIG_IMAGE_ROOT[$i]}"   || echo "    root = ${CONFIG_IMAGE_ROOT[$i]}"
test -z "${CONFIG_IMAGE_APPEND[$i]}" || echo "    append = ${CONFIG_IMAGE_APPEND[$i]}"
test -z "${CONFIG_IMAGE_SYSMAP[$i]}" || echo "    sysmap = ${CONFIG_IMAGE_SYSMAPPATH[$i]}"
test -z "${CONFIG_IMAGE_INITRD[$i]}" || echo "    initrd = ${CONFIG_IMAGE_INITRD[$i]}"
done
) > /tmp/ppc_lilo/yaboot.conf
cp -av /tmp/ppc_lilo/yaboot.conf /etc/yaboot.conf
}


function running_on_prep () {

echo running on prep
#only the device is given and dd to the raw device is a bad idea

if [ "$OPTION_PARTITION" = "" ] ; then
  echo guess the prep boot device 
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

  echo Installing /boot/zImage.prep onto $P
dd if=/boot/zImage.prep of=$P
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
  echo Installing /boot/zImage.prep onto $OPTION_BOOT
dd if=/boot/zImage.prep of=$OPTION_BOOT
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


function running_on_pmac_old () {

echo running on pmac_old
echo generating /tmp/ppc_lilo/miboot.conf ...
echo

# starting the work
(
test -z "$OPTION_TIMEOUT" || echo "timeout = $OPTION_TIMEOUT"
test -z "$OPTION_DEFAULT" || echo "default = $OPTION_DEFAULT"
test -z "$OPTION_ROOT"    || echo "root = $OPTION_ROOT"
test -z "$OPTION_APPEND"  || echo "append = $OPTION_APPEND"
test -z "$OPTION_INITRD"  || echo "initrd = ${OPTION_BOOTFOLDER}:`basename ${OPTION_INITRD}`"
echo
for i in `seq 1 $CONFIG_IMAGE_COUNT` ; do
test -z "${CONFIG_IMAGE_OTHER[$i]}" || continue
echo "image = ${OPTION_BOOTFOLDER}:`basename ${CONFIG_IMAGE_FILE[$i]}`"
test -z "${CONFIG_IMAGE_LABEL[$i]}"  || echo "    label = ${CONFIG_IMAGE_LABEL[$i]}"
test -z "${CONFIG_IMAGE_ROOT[$i]}"   || echo "    root = ${CONFIG_IMAGE_ROOT[$i]}"
test -z "${CONFIG_IMAGE_APPEND[$i]}" || echo "    append = ${CONFIG_IMAGE_APPEND[$i]}"
test -z "${CONFIG_IMAGE_INITRD[$i]}" || echo "    initrd = ${OPTION_BOOTFOLDER}:`basename ${CONFIG_IMAGE_INITRD[$i]}`"
echo
done
echo
) > /tmp/ppc_lilo/miboot.conf


# umount the boot = partition, or exit if that fails
mount | grep -q "$OPTION_BOOT"
if [ "$?" = "0" ] ; then 
echo "unmount $OPTION_BOOT" ; umount $OPTION_BOOT || exit 1
fi
humount $OPTION_BOOT 2>/dev/null
humount $OPTION_BOOT 2>/dev/null

hmount $OPTION_BOOT  || exit 1
if [ "$OPTION_BOOTFOLDER" != ":" ] ; then
HFS_BOOTFOLDER="$OPTION_BOOTFOLDER"
else
HFS_BOOTFOLDER="$DEFAULT_BOOTFOLDER"
fi
echo using bootfolder \'$HFS_BOOTFOLDER\' on volume `hpwd` on $OPTION_BOOT 
hmkdir $HFS_BOOTFOLDER 2>/dev/null
hattrib -b $HFS_BOOTFOLDER
hcd $HFS_BOOTFOLDER
hcopy /boot/Finder.bin :Finder
hcopy /boot/System.bin :System
hcopy -r /tmp/ppc_lilo/miboot.conf :
hattrib -t TEXT -c "R*ch" :miboot.conf
hattrib -t FNDR -c MACS Finder
hattrib -t zsys -c MACS System

for i in `seq 1 $CONFIG_IMAGE_COUNT` ; do
test -z "${CONFIG_IMAGE_OTHER[$i]}" || continue
hcopy ${CONFIG_IMAGE_FILE[$i]} :`basename ${CONFIG_IMAGE_FILE[$i]}`
test -z "${CONFIG_IMAGE_INITRD[$i]}" || { 
hcopy ${CONFIG_IMAGE_INITRD[$i]} :`basename ${CONFIG_IMAGE_INITRD[$i]}`
}
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
if [ "$FILE_PATH" = "" -o "${CONFIG_IMAGE_COPY[$i]}" = "true" -o "$COPY_BOOT_FILES" = "true" ] ; then
        CONFIG_IMAGE_PATH[$i]="copy"
else
	CONFIG_IMAGE_PATH[$i]=$FILE_PATH
fi
unset FILE_PATH
if [ ! -z "${CONFIG_IMAGE_INITRD[$i]}" ] ; then 
	FILE_PATH=$($SHOW_OF_PATH_SH ${CONFIG_IMAGE_INITRD[$i]}|grep -v /pci[0-9])
	if [ "$FILE_PATH" = "" -o "${CONFIG_IMAGE_COPY[$i]}" = "true" -o "$COPY_BOOT_FILES" = "true" ] ; then
        	CONFIG_IMAGE_INITRDPATH[$i]="copy"
	else
		CONFIG_IMAGE_INITRDPATH[$i]=$FILE_PATH
	fi
elif [ ! -z "${CONFIG_IMAGE_SYSMAP[$i]}" ] ; then 
	FILE_PATH=$($SHOW_OF_PATH_SH ${CONFIG_IMAGE_SYSMAP[$i]}|grep -v /pci[0-9])
	if [ "$FILE_PATH" = "" -o "${CONFIG_IMAGE_COPY[$i]}" = "true" -o "$COPY_BOOT_FILES" = "true" ] ; then
		CONFIG_IMAGE_SYSMAPPATH[$i]="copy"
	else
		CONFIG_IMAGE_SYSMAPPATH[$i]=$FILE_PATH
	fi
else
	continue
fi
done

# starting the work
(
test -z "$OPTION_TIMEOUT" || echo "timeout = $OPTION_TIMEOUT"
test -z "$OPTION_DEFAULT" || {
if [ "$OPTION_DEFAULT" = "macos" -o "$OPTION_DEFAULT" = "macosx" ] ; then
# yaboot.conf gets the first available imag= label as default
for i in `seq 1 $CONFIG_IMAGE_COUNT` ; do
test -z "${CONFIG_IMAGE_OTHER[$i]}" || continue
echo "default = ${CONFIG_IMAGE_LABEL[$i]}"
done
else
# a image = label is the default
echo "default = $OPTION_DEFAULT"
fi
}
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
test -z "${CONFIG_IMAGE_SYSMAP[$i]}" || echo "    sysmap = ${CONFIG_IMAGE_SYSMAPPATH[$i]}"
test -z "${CONFIG_IMAGE_INITRD[$i]}" || ( if [ "${CONFIG_IMAGE_INITRDPATH[$i]}" = "copy" ] ; then
	echo "    initrd = `basename ${CONFIG_IMAGE_INITRD[$i]}`" 
else 
	echo "    initrd = ${CONFIG_IMAGE_INITRDPATH[$i]}" 
fi )
done
) > /tmp/ppc_lilo/yaboot.conf

# get the COMPATIBLE_MACHINES 
prepare_compatible_pmacs

BOOT_DEVICEPATH=$($SHOW_OF_PATH_SH $OPTION_BOOT)
OTHER_DEVICEPATH=$($SHOW_OF_PATH_SH $OPTION_OTHER)

echo "BOOT_DEVICEPATH  =  $BOOT_DEVICEPATH"
echo "OTHER_DEVICEPATH  =  $OTHER_DEVICEPATH"
(echo "<CHRP-BOOT>
<COMPATIBLE>
`echo $COMPATIBLE_MACHINES`
</COMPATIBLE>
<DESCRIPTION>
Linux/PPC Yaboot bootloader
</DESCRIPTION>
<BOOT-SCRIPT>"
if [ "$CONFIG_PARSE_HASOTHER" = "true" ] ; then
echo "\" get-key-map\" \" keyboard\" open-dev \$call-method
dup 20 dump
5 + c@ 08 = if"
unset LOOPBLAH
MY_MACOS_STRING=$(for i in `seq 1 $CONFIG_IMAGE_COUNT`;do
 if [ ! -z "${CONFIG_IMAGE_OTHER[$i]}" -a -z "$LOOPBLAH" ] ; then
 LOOPBLAH=true
   if [ "${CONFIG_IMAGE_LABEL[$i]}" = "macosx" ] ; then
     echo "\" Booting Mac OS X ...\" cr \" boot $OTHER_DEVICEPATH,System\\Library\\CoreServices\\BootX\" eval"
   else
     echo "\" Booting MacOS ...\" cr \" boot $OTHER_DEVICEPATH,\\\\:tbxi\" eval"
   fi
 fi
done)
MY_YABOOT_STRING="\" Booting Yaboot ...\" cr \" boot $BOOT_DEVICEPATH,\\\\yaboot\" eval "
if [ "$OPTION_DEFAULT" = "macos" -o "$OPTION_DEFAULT" = "macosx" ] ; then
# macos or macos is the default
echo "\" screen\" output"
echo $MY_YABOOT_STRING
echo "else"
echo $MY_MACOS_STRING

else
#yaboot is the default
echo $MY_MACOS_STRING
echo "else"
echo "\" screen\" output"
echo $MY_YABOOT_STRING
fi
echo "then
</BOOT-SCRIPT>
<OS-BADGE-ICONS>
1010
000000000000F8FEACF6000000000000
0000000000F5FFFFFEFEF50000000000
00000000002BFAFEFAFCF70000000000
0000000000F65D5857812B0000000000
0000000000F5350B2F88560000000000
0000000000F6335708F8FE0000000000
00000000005600F600F5FD8100000000
00000000F9F8000000F5FAFFF8000000
000000008100F5F50000F6FEFE000000
000000F8F700F500F50000FCFFF70000
00000088F70000F50000F5FCFF2B0000
0000002F582A00F5000008ADE02C0000
00090B0A35A62B0000002D3B350A0000
000A0A0B0B3BF60000505E0B0A0B0A00
002E350B0B2F87FAFCF45F0B2E090000
00000007335FF82BF72B575907000000
000000000000ACFFFF81000000000000
000000000081FFFFFFFF810000000000
0000000000FBFFFFFFFFAC0000000000
000000000081DFDFDFFFFB0000000000
000000000081DD5F83FFFD0000000000
000000000081DDDF5EACFF0000000000
0000000000FDF981F981FFFF00000000
00000000FFACF9F9F981FFFFAC000000
00000000FFF98181F9F981FFFF000000
000000ACACF981F981F9F9FFFFAC0000
000000FFACF9F981F9F981FFFFFB0000
00000083DFFBF981F9F95EFFFFFC0000
005F5F5FDDFFFBF9F9F983DDDD5F0000
005F5F5F5FDD81F9F9E7DF5F5F5F5F00
0083DD5F5F83FFFFFFFFDF5F835F0000
000000FBDDDFACFBACFBDFDFFB000000
000000000000FFFFFFFF000000000000
0000000000FFFFFFFFFFFF0000000000
0000000000FFFFFFFFFFFF0000000000
0000000000FFFFFFFFFFFF0000000000
0000000000FFFFFFFFFFFF0000000000
0000000000FFFFFFFFFFFF0000000000
0000000000FFFFFFFFFFFFFF00000000
00000000FFFFFFFFFFFFFFFFFF000000
00000000FFFFFFFFFFFFFFFFFF000000
000000FFFFFFFFFFFFFFFFFFFFFF0000
000000FFFFFFFFFFFFFFFFFFFFFF0000
000000FFFFFFFFFFFFFFFFFFFFFF0000
00FFFFFFFFFFFFFFFFFFFFFFFFFF0000
00FFFFFFFFFFFFFFFFFFFFFFFFFFFF00
00FFFFFFFFFFFFFFFFFFFFFFFFFF0000
000000FFFFFFFFFFFFFFFFFFFF000000
</OS-BADGE-ICONS>
</CHRP-BOOT>"
else
echo "\" screen\" output
\" Booting Yaboot ...\" cr \" boot $BOOT_DEVICEPATH,\\\\yaboot\" eval
</BOOT-SCRIPT>
<OS-BADGE-ICONS>
1010
000000000000F8FEACF6000000000000
0000000000F5FFFFFEFEF50000000000
00000000002BFAFEFAFCF70000000000
0000000000F65D5857812B0000000000
0000000000F5350B2F88560000000000
0000000000F6335708F8FE0000000000
00000000005600F600F5FD8100000000
00000000F9F8000000F5FAFFF8000000
000000008100F5F50000F6FEFE000000
000000F8F700F500F50000FCFFF70000
00000088F70000F50000F5FCFF2B0000
0000002F582A00F5000008ADE02C0000
00090B0A35A62B0000002D3B350A0000
000A0A0B0B3BF60000505E0B0A0B0A00
002E350B0B2F87FAFCF45F0B2E090000
00000007335FF82BF72B575907000000
000000000000ACFFFF81000000000000
000000000081FFFFFFFF810000000000
0000000000FBFFFFFFFFAC0000000000
000000000081DFDFDFFFFB0000000000
000000000081DD5F83FFFD0000000000
000000000081DDDF5EACFF0000000000
0000000000FDF981F981FFFF00000000
00000000FFACF9F9F981FFFFAC000000
00000000FFF98181F9F981FFFF000000
000000ACACF981F981F9F9FFFFAC0000
000000FFACF9F981F9F981FFFFFB0000
00000083DFFBF981F9F95EFFFFFC0000
005F5F5FDDFFFBF9F9F983DDDD5F0000
005F5F5F5FDD81F9F9E7DF5F5F5F5F00
0083DD5F5F83FFFFFFFFDF5F835F0000
000000FBDDDFACFBACFBDFDFFB000000
000000000000FFFFFFFF000000000000
0000000000FFFFFFFFFFFF0000000000
0000000000FFFFFFFFFFFF0000000000
0000000000FFFFFFFFFFFF0000000000
0000000000FFFFFFFFFFFF0000000000
0000000000FFFFFFFFFFFF0000000000
0000000000FFFFFFFFFFFFFF00000000
00000000FFFFFFFFFFFFFFFFFF000000
00000000FFFFFFFFFFFFFFFFFF000000
000000FFFFFFFFFFFFFFFFFFFFFF0000
000000FFFFFFFFFFFFFFFFFFFFFF0000
000000FFFFFFFFFFFFFFFFFFFFFF0000
00FFFFFFFFFFFFFFFFFFFFFFFFFF0000
00FFFFFFFFFFFFFFFFFFFFFFFFFFFF00
00FFFFFFFFFFFFFFFFFFFFFFFFFF0000
000000FFFFFFFFFFFFFFFFFFFF000000
</OS-BADGE-ICONS>
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

if [ "$OPTION_ACTIVATE" = "yes" ] ; then
  NV_BOOT_PATH=$($SHOW_OF_PATH_SH $OPTION_BOOT)
  echo set OF boot-device $NV_BOOT_PATH",\\\\:tbxi"
  nvsetenv boot-device $NV_BOOT_PATH",\\\\:tbxi"
fi


}


function check_arch () {
# check for the current ppc subarch
unset COPY_BOOT_FILES
while read line; do
	case "$line" in
		*MacRISC2*)	MACHINE="pmac" ; COPY_BOOT_FILES="true" ;;
		*MacRISC*)	MACHINE="pmac" ;;
		*CHRP*)		MACHINE="chrp" ;;
		*PReP*)		MACHINE="prep" ;;
	esac
done < /proc/cpuinfo


if [ "$MACHINE" = "pmac" ] ; then
	if [ -f /proc/device-tree/openprom*/model ] ; then
		echo `cat /proc/device-tree/openprom*/model` > /tmp/ppc_lilo/openprom_model
		while read openfirmware ofversion; do
	        	case "$openfirmware" in
	                iMac,1|OpenFirmware)      MACHINE="pmac_new" ;;
	                Open)      MACHINE="pmac_old" ;;
	        	esac
		done < /tmp/ppc_lilo/openprom_model
	fi
fi
} #end function check_arch

function prepare_compatible_pmacs () {
# strip comments and empty lines

COMPATIBLE_MACHINES=$(sed 's-#.*$--' < $COMPATIBLE_LIST | grep -v "\(^\W$\|^$\)" )
# echo $COMPATIBLE_MACHINES

} #end function prepare_compatible_pmacs

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
				OPTION_ACTIVATE="yes"
			else 
				echo activate option must not in an image section!
				exit 1
			fi
		;;
		progressbar)
			# do nothing
		;;
		bootfolder)
			if [ -z "$CONFIG_PARSE_HASIMAGE" ] ; then
				OPTION_BOOTFOLDER=":${value}"
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
		sysmap)
                        if [ -z "$CONFIG_PARSE_HASIMAGE" ] ; then
                                echo "ERROR: sysmap can not be global"
				exit 1
                        else
                                CONFIG_IMAGE_SYSMAP[$CONFIG_IMAGE_COUNT]="$value"
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
			exit 1
		;;
	esac
done < /tmp/ppc_lilo/config_tmp


} #end function parse_config_file


function check_config_file () {


if test ! -b $OPTION_BOOT  ; then
echo ERROR: boot = $OPTION_BOOT is not a valid block device
exit -1 
fi

if [ -z "$OPTION_BOOT" ] ; then
echo "ERROR: boot= is not specified!"
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
if [ ! -f ${CONFIG_IMAGE_SYSMAP[$i]} ] ; then
echo "ERROR: sysmap = ${CONFIG_IMAGE_SYSMAP[$i]} ist not a regular file"
exit -1
fi
done


OPTION_DEVICE=`echo "$OPTION_BOOT"|sed 's/[0-9]*$/ &/'|cut -d " " -f 1`
OPTION_NODENAME=`echo "$OPTION_DEVICE"|sed 's/[0-9]*$/ &/'|cut -d "/" -f 3`
OPTION_PARTITION=`echo "$OPTION_BOOT"|sed 's/[0-9]*$/ &/'|cut -d " " -f 2`



} #end function check_config_file



function prepare_envoirement () {

rm -f /tmp/ppc_lilo/*
mkdir -p /tmp/ppc_lilo/

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