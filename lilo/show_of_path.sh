#!/bin/bash
# vim: syntax=off
# set -ex
#
# $Id$
# find a OF bootpath on Apple PowerMacintosh Newworld machines
# Copyright (C) 2000, 2004 Olaf Hering <olh@suse.de>,
#               2005 Joachim Plack <jplack@suse.de>
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

# check for requirements:
#   /proc
#   /sys

trap '{
    cd /
    [ $_sysfs_mounted ] && umount /sys
    [ $_proc_mounted ] && umount /proc
}' EXIT

# assert that /proc is mounted, else try to mount, on fail complain
if test -d /proc/1; then
    :
elif mount -t proc proc /proc; then
    _proc_mounted=1
else
    error "proc not mounted and attempt to mount /proc failed"
fi


# assert that /sys is mounted, else try to mount, on fail complain
if test -d /sys/block; then
    :
elif mount -t sysfs sysfs /sys; then
    _sysfs_mounted=1
else
    error "sysfs not mounted on /sys and attempt to mount failed" "may be no kernel 2.6.x?"
fi


shopt -s extglob
read d myversion d <<< "$Date$"


function error() {
    [ "$quietmode" ] && set --
    while [ "$1" ]; do
	echo 1>&2 "ERROR: $1"
	shift;
    done
    exit 1
}



function read_qbyte() {
    local file=$1;
    local count=$2;

    [ -r $file ] || return;
    while (( count-- )) && read addr val; do
	echo $val
    done < <(od --read-bytes=$[4*count] --width=4 -t x4 $file)
}


function read_int() {
    [ "$1" ] || return;
    echo $(( 0x$(read_qbyte "$1" 1) ))
}

function get_port () {
	local variant orig_offset
	local i port offset

	variant=$1
	orig_offset=$2

	offset=$orig_offset
	for i in $variant[0-9]*
	do
		: i $i
		port="${i#$variant}"
		if test "$port" -lt "$offset"
		then
			offset=$port
		fi
	done
	if test "$port" != "0"
	then
		echo $(( $orig_offset - $offset ))
	fi
}

# if no file path is given on cmd line check for root file system
file=/
#
sysfs_ide_media_type=
of_ide_media_type=
ide_port=
ide_channel=

if [ "$#" -gt 0 ] ; then
    until  [ "$#" = 0 ] ; do
	case "$1" in
	    --version|-v)
		echo $myversion
		exit 0
		;;
  	    --help|-h)
		echo "show OpenFirmware device path for a file or a device node"
		echo "usage: ${0##*/} [--quiet|-q] [/]|[/boot/vmlinux]"
		exit 0
		;;
	    --quiet|-q)
		quietmode=1
		;;
	    --debug|-d)
	        debug=1
		;;
	    *)
	       	file=$1
	       	break
	       	;;
	esac
	shift
    done
fi

if [ "$debug" ]; then    
    function dbg_show() {
	while [ "$1" ]; do
	    echo $1 = ${!1}
	    shift
	done
    }
else
    function dbg_show() {
	:
    }
fi

# check if we run on a OldWorld PowerMacintosh
if [ -f /proc/device-tree/openprom/model ] &&
   [[ "$(</proc/device-tree/openprom/model)" == Open\ * ]];
then 
    error "This machine is an Oldworld, no need for firmware pathnames"
fi


if [ -b $file ] ; then
    read i i i i file_major file_minor i <<< $(ls -lL "$file")
    file_major="${file_major%%,*}"
    file_minor="${file_minor##*,}"
    file=/
else
    mystat="`type -p stat`"
    if [ -z "$mystat" ] || ! [ -x "$mystat" ] ; then
	error "GNU stat required"
    fi

    file_majorminor=$($mystat --format="%d" "$file")
    file_major="$[file_majorminor >> 8]"
    file_minor="$[file_majorminor & 255]"

    file_mountp=$file
    while [ "$file_mountp" -a $($mystat --format="%d" "/${file_mountp%/*}") == "$file_majorminor" ]; do
	file_mountp="${file_mountp%/*}"
    done
fi
file_majorminor=$file_major:$file_minor
dbg_show file_majorminor

file_sysfs_path=

for i in $(find /sys/block -name dev); do
    : looking at $i
    if [ "$(< $i)" = "$file_majorminor" ] ; then file_sysfs_path=$i ; break ; fi
done

if [ -z "$file_sysfs_path" ] ; then
    error "can not find major:minor $file_majorminor for $file"
fi

dbg_show file_sysfs_path

file_sysfs_dir="${file_sysfs_path%/dev}"
dbg_show file_sysfs_dir
if [ ! -L "$file_sysfs_dir/device" ] ; then
    # maybe a partition
    if [ ! -L "$file_sysfs_dir/../device" ] ; then
	if [ -d "$file_sysfs_dir/md" ] ; then
	    # TODO: think about whether we enable reading of
	    # $(<md/level) == raid0 through the first
	    # partition/device (md/rd0/block) of the soft raid
	    # array, but til then I´d consider this a hack, maybe a
	    # special parameter --raid is an option 
	    error "soft raid (${file_sysfs_dir##*/}) is not readable by open firmware"
	elif [[ "$file_sysfs_dir" == */dm-* ]]; then
	    error "mapped devices like ${file_sysfs_dir##*/} are not readable by open firmware"
	else
	    error "driver for sysfs path $file_sysfs_dir has no full sysfs support"
	fi
    fi
    file_partition="${file_sysfs_dir##*[a-z]}"
    dbg_show file_partition
    file_sysfs_dir="${file_sysfs_dir%/*}"
    dbg_show file_sysfs_dir
fi

cd "$file_sysfs_dir/device"
file_full_sysfs_path="`pwd -P`"
file_storage_type=
cd "$file_full_sysfs_path"
if test -f ieee1394_id ; then
	file_storage_type=sbp2
	read ieee1394_id < ieee1394_id
	ieee1394_id=${ieee1394_id%%:*}
	until test -f devspec
	do
		cd ..
		if test "$PWD" = "/"
		then
			break
		fi
	done
else
case "$file_full_sysfs_path" in
    */ide+([0-9])/+([0-9.]))
	if test -f media
	then
		read sysfs_ide_media_type < media
	fi
	: sysfs_ide_media_type $sysfs_ide_media_type
	file_storage_type=ide
	ide_port="${file_full_sysfs_path##*/ide}"
	ide_port="${ide_port%%/*}"
	ide_channel="${file_full_sysfs_path##*.}"
	cd ../..
	if [[ "$ide_channel" == */* ]]; then
	    ide_channel="${ide_channel%%/*}"
	    cd ../..
	fi
	dbg_show ide_port
	dbg_show ide_channel
	;;
    */host+([0-9])/+([0-9]):+([0-9]):+([0-9]):+([0-9]))
      	# file_storage_type=scsi !! or vscsi, will be determined later
	declare spec="${file_full_sysfs_path##*/host+([0-9])/}"

	read of_disk_scsi_host of_disk_scsi_chan of_disk_scsi_id of_disk_scsi_lun <<< ${spec//:/ }
	dbg_show of_disk_scsi_host of_disk_scsi_chan of_disk_scsi_id of_disk_scsi_lun
	cd ../..
	;;
    */host+([0-9])/target+([0-9:])/+([0-9]):+([0-9]):+([0-9]):+([0-9]))
	# new sysfs layout starting with kernel 2.6.10
	declare spec="${file_full_sysfs_path##*/host+([0-9])\/target+([0-9:])/}"

	read of_disk_scsi_host of_disk_scsi_chan of_disk_scsi_id of_disk_scsi_lun <<< ${spec//:/ }
	dbg_show of_disk_scsi_host of_disk_scsi_chan of_disk_scsi_id of_disk_scsi_lun
	cd ../../..
	;;
    */host+([0-9])/rport-+([0-9]):+([0-9])-+([0-9])/target+([0-9:])/+([0-9]):+([0-9]):+([0-9]):+([0-9]))
	# new sysfs layout starting with kernel 2.6.15
	declare spec="${file_full_sysfs_path##*/host+([0-9])\/rport-+([-0-9:])\/target+([0-9:])/}"

	: spec $spec
	read of_disk_scsi_host of_disk_scsi_chan of_disk_scsi_id of_disk_scsi_lun <<< ${spec//:/ }
	dbg_show of_disk_scsi_host of_disk_scsi_chan of_disk_scsi_id of_disk_scsi_lun
	cd ../../../..
	;;
    */host+([0-9])/session+([0-9])/target+([0-9:])/+([0-9]):+([0-9]):+([0-9]):+([0-9]))
	# iscsi 2.6.16.21 sles10 ga
	declare spec="${file_full_sysfs_path##*/host+([0-9])\/session+([-0-9:])\/target+([0-9:])/}"

	dbg_show spec
	read of_disk_scsi_host of_disk_scsi_chan of_disk_scsi_id of_disk_scsi_lun <<< ${spec//:/ }
	dbg_show of_disk_scsi_host of_disk_scsi_chan of_disk_scsi_id of_disk_scsi_lun
	cd ../..
	;;
    *)
        # TODO check the rest of the (hardware) world
	: file_full_sysfs_path $file_full_sysfs_path
	error "could not gather enough information to create open firmware path to device"
esac
# ieee1394_id
fi 

# iscsi has no devspec pointer into the OF device tree
if [ -f devspec ] ; then
    read file_of_hw_devtype < devspec
    file_of_hw_devtype=/proc/device-tree${file_of_hw_devtype}
    dbg_show file_of_hw_devtype
    if ! [ -f ${file_of_hw_devtype}/device_type ]; then
	if ! [ -d ${file_of_hw_devtype}/sas ]; then
                # check for scsi@$of_disk_scsi_chan/device_type else bail out ..
                file_of_hw_devtype=$(printf "%s/scsi@%x" $file_of_hw_devtype $of_disk_scsi_chan)
                dbg_show file_of_hw_devtype
        else
                # check for sas/device_type else bail out ..
                file_of_hw_devtype=$(printf "%s/sas" $file_of_hw_devtype)
                dbg_show file_of_hw_devtype
        fi
    fi
    if ! [ -f ${file_of_hw_devtype}/device_type ] ; then
	error "no device_type found in ${file_of_hw_devtype}"
    fi

    case "$(< ${file_of_hw_devtype}/device_type)" in
	k2-sata-root)
	    : found k2-sata-root, guessing channel
	    counter=0
	    for i in host+([0-9]); do
		: working on virtual scsi host $i
		case "$file_full_sysfs_path" in
		    */$i/*)
		        : found $i $counter
			break
			;;
		    *)
		        ;;
		esac
		(( counter++ ))
	    done
	    of_device_path=`grep -l block ${file_of_hw_devtype}/k2-sata*@$counter/*/device_type`
	    of_device_path=${of_device_path%/device_type}
	    of_device_path=${of_device_path##/proc/device-tree}
	    file_storage_type=sata
	    ;;
	scsi*)
	    file_storage_type=scsi
	    ;;
	sas*)
	    file_storage_type=sas
	    ;;
	spi)
	    file_storage_type=spi-ide
	    ;;
	ide|ata)
	    # TODO
	    # check for right file-storage_type == ide ??
	    file_storage_type=ide
	    ;;
	pci-ide|pci-ata)
	    of_device_path=`grep -l block ${file_of_hw_devtype}/*@*/*/device_type`
	    of_device_path=${of_device_path%/device_type}
	    of_device_path=${of_device_path##/proc/device-tree}
	    # TODO
	    # check for right file-storage_type == ide ??
	    file_storage_type=pci-ide
	    ;;
	vscsi)
	    file_storage_type=vscsi
	    ;;
	fcp)
	    declare of_disk_fc_wwpn

	    # modprobe scsi_transport_fc  ## loaded through dependencies
	    port=$(
		printf "/sys/class/fc_transport/target%d:%d:%d/port_name" \
		    $of_disk_scsi_host $of_disk_scsi_chan \
		    $of_disk_scsi_id \
	    )

	    if ! [ -f "$port" ]; then
		port=$(
		    printf "/sys/class/fc_transport/%d:%d:%d:%d/port_name" \
			$of_disk_scsi_host $of_disk_scsi_chan \
			$of_disk_scsi_id $of_disk_scsi_lun \
		)
	    fi

	    if [ -f "$port" ]; then
		# read the appropriate /sys/class/fc_transport/*/port_name
		of_disk_fc_wwpn=$(< $port)
		of_disk_fc_wwpn=${of_disk_fc_wwpn#0x} 	# remove leading 0x
	    elif [ -f info ]; then
	        # this is currently only tested on emulex cards, cnd it´s a fall back
		# FC HBA should support fc_transport layer!
		scsi_id=0
	        while read; do
		    [[ "$REPLY" == *WWPN* ]] || continue
		    dbg_show scsi_id
		    (( scsi_id++ == of_disk_scsi_id )) || continue
		    dbg_show REPLY
		    read of_disk_fc_wwpn d <<< "${REPLY#*WWPN}"
		    of_disk_fc_wwpn="${of_disk_fc_wwpn//:}" 	# remove all colons within WWPN string
		    break
	        done < info
	    else
 		error "could not find an 'info' file nor an fc_transport layer for FC adapter"
	    fi
	    [ "$of_disk_fc_wwpn" ] || error "could not get a WWPN for that FC disk"
	    file_storage_type=fcp
	    ;;
	ieee1394)
	    ;;
	*)
	    error "Unknown device type $(< ${file_of_hw_devtype}/device_type)"
	    ;;
    esac

    case "$file_storage_type" in
        ide)
	    file_of_hw_path="${file_of_hw_devtype##/proc/device-tree}/disk@$ide_channel"
	    ;;
        scsi)
	    if [ -d ${file_of_hw_devtype}/disk ]; then
		of_disk_scsi_dir=disk
	    elif [ -d ${file_of_hw_devtype}/sd ]; then
		of_disk_scsi_dir=sd
	    else
		error "Could not find a known hard disk directory under '${file_of_hw_devtype}'"
	    fi
	    file_of_hw_path=$(
		printf "%s/%s@%x,%x"  "${file_of_hw_devtype##/proc/device-tree}" \
		    $of_disk_scsi_dir $of_disk_scsi_id $of_disk_scsi_lun
	    )
	    ;;
        sas)
	    if [ -d ${file_of_hw_devtype}/disk ]; then
		of_disk_scsi_dir=disk
	    else
		error "Could not find a known hard disk directory under '${file_of_hw_devtype}'"
	    fi
	    (( of_disk_addr = ( (of_disk_scsi_chan<<16) |  (of_disk_scsi_id<<8) |  of_disk_scsi_lun ) )); #

	    file_of_hw_path=$(
		printf "%s/%s@%x,%x"  "${file_of_hw_devtype##/proc/device-tree}" \
		    $of_disk_scsi_dir $of_disk_addr $of_disk_scsi_lun
	    )
	    ;;
        fcp)
	    declare of_disk_fc_dir
	    declare of_disk_fc_lun=$of_disk_scsi_lun

	    if [ -d ${file_of_hw_devtype}/disk ]; then
		of_disk_fc_dir=disk
	    elif [ -d ${file_of_hw_devtype}/sd ]; then
		of_disk_fc_dir=sd
	    else
		error "Could not find a known hard disk directory under '${file_of_hw_devtype}'"
	    fi
	   
	    device_id=$(read_int ${file_of_hw_devtype}/device-id)
	    vendor_id=$(read_int ${file_of_hw_devtype}/vendor-id)

	    dbg_show device_id vendor_id

	    lun_format="%016x"	# fallback LUN encoding
	    if (( vendor_id == 0x10df )); then
		# PCI_VENDOR_ID_EMULEX==0x10df
		id=$(printf "%04x" $device_id)
		if [[ $id == @(f901|f981|f982|fa00|fa01) ]]; then
		    warning "Emulex FC HBA with device id 0x$id not yet tested." \
			    "Reboot may fail."
		fi
		if [[ $id == @(f900|f901|f980|f981|f982|fa00|fa01|fd00) ]]; then
		    # TODO: may be check /sys/class/scsi_host/hostX/lpfc_max_luns
		    lun_format="%x000000000000"
		fi
	    elif (( vendor_id == 0x1077 )); then
		# PCI_VENDOR_ID_QLOGIC==0x1077
		lun_format="%04x000000000000"
	    fi

	    file_of_hw_path=$(
		printf "%s/%s@%s,${lun_format}" \
		    "${file_of_hw_devtype##/proc/device-tree}" \
		    $of_disk_fc_dir $of_disk_fc_wwpn $of_disk_fc_lun
	    )
	    ;;
        sata|pci-ide)
	    file_of_hw_path=$of_device_path
	    ;;
	spi-ide)
	    case "$sysfs_ide_media_type" in
		cdrom)
		of_ide_media_type=cdrom
		;;
		disk)
		of_ide_media_type=disk
		;;
		*)
		of_ide_media_type=unhandled
		;;
	    esac
	    file_of_hw_path="${file_of_hw_devtype##/proc/device-tree}/$of_ide_media_type@$ide_port,$ide_channel"
	    ;;
        vscsi)
	    (( of_disk_vscsi_nr = ( (2 << 14) | (of_disk_scsi_chan<<5) |  (of_disk_scsi_id<<8) |  of_disk_scsi_lun ) <<48 )); #
	    if [ -d ${file_of_hw_devtype}/disk ]; then
		of_disk_vscsi_dir=disk
	    elif [ -d ${file_of_hw_devtype}/sd ]; then
		of_disk_vscsi_dir=sd
	    else
		error "Could not find a known hard disk directory under '${file_of_hw_devtype}'"
	    fi	
	    file_of_hw_path=$(
		printf "%s/%s@%lx"  "${file_of_hw_devtype##/proc/device-tree}" \
		    $of_disk_vscsi_dir $of_disk_vscsi_nr
	    )
	    ;;
	sbp2)
	    # /proc/device-tree/pci@f4000000/firewire@e/node@0001d20000038f29/sbp-2@c000/disk@0
	    # MacOS: boot-device=fw/node@1d20000038f29/sbp-2@c000/@0:7,\\:tbxi
	    file_of_hw_path="${file_of_hw_devtype##/proc/device-tree}"/node@${ieee1394_id}/sbp-2/disk@0
	    ;;
	*)
	    error "Internal error, can't handle storage type '${file_storage_type}'"
	    ;;
    esac
else # no 'devspec' found
   case "$file_full_sysfs_path" in
	*/host+([0-9])/session+([0-9])/*)
	if test -f /etc/initiatorname.iscsi
	then
		iscsi_initiatorname_conf=/etc/initiatorname.iscsi
	elif test -f /etc/iscsi/initiatorname.iscsi
	then
		iscsi_initiatorname_conf=/etc/iscsi/initiatorname.iscsi
	else
		error "initiatorname.iscsi config file not found"
	fi
	iscsi_session="${file_full_sysfs_path%/*}"
	iscsi_session="${iscsi_session%/*}"
	iscsi_session="${iscsi_session##*/}"
	iscsi_connection="${iscsi_session#session*}"
	# FIXME
	iscsi_network_interface="` ip -o link show up | awk ' BEGIN { FS=":" ; foo="" } ; /link\/ether/ { if (foo == "") { foo=$2 } } ; END { print foo } ' `"
	set -- $iscsi_network_interface
	iscsi_network_interface=$1
	iscsi_network_card="` cat /sys/class/net/$iscsi_network_interface/device/devspec `"
	iscsi_itname="` awk ' BEGIN { FS="=" ; foo="" } ; /^InitiatorName=/{ if (foo == "") { foo=$2 } } ; END { print foo } ' "$iscsi_initiatorname_conf" `"
	iscsi_ciaddr="` ip addr show dev $iscsi_network_interface | awk ' BEGIN { foo="" } ; / inet /{ if (foo == "") { foo=$2 } } ; END { print foo } ' `"
	iscsi_giaddr="` ip route show dev $iscsi_network_interface | awk ' BEGIN { foo="" } ; /default via /{ if (foo == "") { foo=$3 } } ; END { print foo } ' `"
	if test -z "$iscsi_giaddr"
	then
		iscsi_giaddr=0.0.0.0
	fi
	# FIXME
	case "$iscsi_ciaddr" in
		*/8)  iscsi_subnet_mask=255.0.0.0 ;;
		*/16) iscsi_subnet_mask=255.255.0.0 ;;
		*/24) iscsi_subnet_mask=255.255.255.0 ;;
		*)    iscsi_subnet_mask=0.0.0.0 ;;
	esac
	iscsi_ciaddr="${iscsi_ciaddr%/*}"
	iscsi_siaddr="` cat connection$iscsi_connection:0/iscsi_connection:connection$iscsi_connection:0/persistent_address `"
	iscsi_iname="` cat iscsi_session:$iscsi_session/targetname `"
	iscsi_iport="` cat connection$iscsi_connection:0/iscsi_connection:connection$iscsi_connection:0/persistent_port `"
	#FIXME
	iscsi_ilun="${file_full_sysfs_path##*:}"
	iscsi_ilun="` printf '%x%012x' $iscsi_ilun 0 `"
	file_of_hw_path="$iscsi_network_card:iscsi,itname=$iscsi_itname,ciaddr=$iscsi_ciaddr,giaddr=$iscsi_giaddr,subnet-mask=$iscsi_subnet_mask,siaddr=$iscsi_siaddr,iname=$iscsi_iname,iport=$iscsi_iport,ilun=$iscsi_ilun"
	;;
	*)
    echo >&2 "WARNING: No devspec file found for $file_full_sysfs_path"

	dbg_show file_full_sysfs_path
	# find the path via the device-tree
	dev_vendor="$(< vendor)"
	dev_vendor=$(($dev_vendor))
	dev_device="$(< device)"
	dev_device=$(($dev_device))
	dev_subsystem_vendor="$(< subsystem_vendor)"
	dev_subsystem_vendor=$(($dev_subsystem_vendor))
	dev_subsystem_device="$(< subsystem_device)"
	dev_subsystem_device=$(($dev_subsystem_device))

	for i in `find /proc/device-tree -name vendor-id`
	do
	  : looking at $i
	  dev_of_pci_id=$(read_int "$i")
	  if [ "$dev_of_pci_id" != "$dev_vendor" ] ; then continue ; fi
	  if [ ! -f "${i%/*}/device-id" ] ; then continue ; fi
	  dev_of_pci_id=$(read_int "${i%/*}/device-id")
	  
	  if [ "$dev_of_pci_id" != "$dev_device" ] ; then continue ; fi
	  if [ -f "${i%/*}/subsystem-vendor-id" ] ; then
	      dev_of_pci_id=$(read_int "${i%/*}/subsystem-vendor-id")
	      if [ "$dev_of_pci_id" != "$dev_subsystem_vendor" ] ; then continue ; fi
	      dev_of_pci_id=$(read_int "${i%/*}/subsystem-id")
	      if [ "$dev_of_pci_id" != "$dev_subsystem_device" ] ; then continue ; fi
	  fi
	  : found $i
	  if [ -z "$of_device_list" ] ; then
	      of_device_list="${i%/*}"
	  else
	      of_device_list="$of_device_list ${i%/*}"
	  fi
	done
	dbg_show of_device_list
	case "$of_device_list" in
	    *\ *)
		: more than one controler found, fun
		for i in $of_device_list
		do
		: working on $i
		read dummy high low < <(read_qbyte $i/assigned-addresses 3)
		addr="0x${high}${low}"

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
		for i in host+([0-9]); do
		    : working on virtual scsi host $i
		    case "$file_full_sysfs_path" in
		        */$i/*)
			    : found $i $counter
			    break
			    ;;
			*) ;;
		    esac
		    let counter++
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
		file_of_hw_path="${of_device_path##/proc/device-tree}@$ide_channel"
		;;
	    scsi)
		file_of_hw_path=$(printf  "%s/sd@%x,%x"  "${of_device_path##/proc/device-tree}" $of_disk_scsi_id $of_disk_scsi_lun)
		;;
	    sas)
		(( of_disk_addr = ( (of_disk_scsi_chan<<16) |  (of_disk_scsi_id<<8) |  of_disk_scsi_lun ) )); #
		file_of_hw_path=$(printf  "%s/disk@%x,%x"  "${of_device_path##/proc/device-tree}" $of_disk_addr $of_disk_scsi_lun)
		;;
	    sata)
		file_of_hw_path="${of_device_path##/proc/device-tree}"
		;;
	    fcp)
	        # TODO is that true? current version is a copy from scsi
	        dir=disk
		file_of_hw_path=$(printf  "%s/${dir}@%016x,%016x"  "${of_device_path##/proc/device-tree}" $of_disk_scsi_id $of_disk_scsi_lun)
		;;
	esac
	;;
	# no "devspec" available
    esac
fi

#
# done
#

# print the resulting open firmware path
if [ "$file" != "/" ] ; then
    echo ${file_of_hw_path}${file_partition:+:$file_partition},${file#$file_mountp}
else
    echo ${file_of_hw_path}${file_partition:+:$file_partition}
fi


#
#
# Local variables:
#     mode: sh
#     mode: font-lock
#     mode: auto-fill
#     sh-indent: 4
#     sh-multiline-offset: 2
#     sh-if-re: "\\s *\\b\\(if\\)\\b[^=]"
#     fill-column: 78
# End:
#
