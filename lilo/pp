--- lilo.new	2004-05-13 15:34:00.000000000 +0200
+++ lilo.fixed	2004-05-13 16:02:45.000000000 +0200
@@ -144,7 +137,7 @@
     echo running on chrp
 
     # find all PReP boot and FAT partitions
-    local -a prep_part fat_part
+    local -a prep_part fat_part fat32_part
 
     while read; do
 	local device start end blocks id system
@@ -159,9 +152,12 @@
 	  6)  # FAT16 partition
 	      fat_part=(${fat_part[*]} $device)
 	      ;;
+	  c) # FAT32 partition
+	      fat32_part=(${fat32_part[*]} $device)
+	      ;;
 	esac
	# please note: an empty boot= will cause a full disk scan of
	# the system
     done < <( $FDISK -l $OPTION_DEVICE )
 
     # check all mounted local file systems
@@ -188,7 +180,14 @@
 	       	[ "$OPTION_BOOT" ] || OPTION_BOOT=$fat_part
 	       	;;
             0)
-	        error "in config file, boot = $OPTION_BOOT is not 41 PReP nor FAT"  1
+		# special case: YaST2 does not give the option for FAT16 but
+		# only for FAT32 so allow that as a special case
+		
+		if [ ${#fat32_part[*]} == 1 ]; then
+		    OPTION_BOOT=$fat32_part
+		else
+	            error "in config file, boot = $OPTION_BOOT is not 41 PReP nor FAT"  1
+		fi
 	       	;;
 	    *)
 	       	error "in config file, guessing of boot partition failed" 2
@@ -197,7 +196,7 @@
     else
 	#we have the device, but better safe than sorry
 	echo Boot target is $OPTION_DEVICE
-	if [[ "${prep_part[*]} ${fat_part[*]}" != *$OPTION_BOOT* ]]; then
+	if [[ "${prep_part[*]} ${fat_part[*]} ${fat32_part[*]}" != *$OPTION_BOOT* ]]; then
 	    error "in config file, boot = $OPTION_BOOT is not 41 PReP nor FAT"  1
 	fi
     fi
@@ -238,7 +237,7 @@
 	    #
 	    #  is no longer needed due to olh 9.3.2004
 	    # still needed, might be a bug in yaboot 26.3.2004
-	    if [ -n "${CONFIG_IMAGE_INITRD[$i]}" -a -d /proc/ppc64 ]; then
+	    if false && [ -n "${CONFIG_IMAGE_INITRD[$i]}" -a -d /proc/ppc64 ]; then
 		if ! bash /lib/lilo/chrp/chrp64/addRamdisk.sh $TEMP \
 		  ${CONFIG_IMAGE_FILE[$i]} \
 		  ${CONFIG_IMAGE_INITRD[$i]} \
@@ -283,6 +282,37 @@
 		continue
 	    fi
 
+	    if [ "${CONFIG_IMAGE_INITRD[$i]}" -a -f "${CONFIG_IMAGE_INITRD[$i]}" ]; then
+		local image="${CONFIG_IMAGE_INITRD[$i]}"
+		local path_image=$(get_of_path $image)
+		local devnr_image=$(device_of_file $image) 
+		
+		# check whether the image file lies on a file system that is
+		# readable by yaboot
+		if [[ "${fsys[$devnr_image]}" == @(ext2|ext3|msdos|vfat|reiserfs|iso9660|xfs) ]]; then
+		    # Is boot image file on the same device from where yaboot started?
+		    if [ "${path_image:0:${#path_boot}}" = "$path_boot" ]; then
+			CONFIG_IMAGE_INITRD[$i]="$image"
+			# test "${CONFIG_IMAGE_LABEL[$i]}" = "$OPTION_DEFAULT" && echo " *" || echo
+		    else
+			CONFIG_IMAGE_INITRD[$i]="${path_image}"
+		    fi
+		else
+		    # image file needs to be copied to the boot device which has to
+		    # be a FAT file system and be large enough to hold all data
+		    let initrd_nr++
+		    if [ "${CONFIG_IMAGE_LABEL[$i]}" = "$OPTION_DEFAULT" ]; then
+			initrd_default=$initrd_nr
+		    fi
+		    initrd_copy[$initrd_nr]="$image"
+		    CONFIG_IMAGE_INITRD[$i]=$(printf "initrd.%03d\n" $initrd_nr)
+		fi
+	    else
+		echo >&2 "Warning: initrd ${CONFIG_IMAGE_INITRD[$i]} in ${CONFIG_IMAGE_LABEL[$i]} is missing"
+		echo >&2 "         Entry ignored"
+		continue
+	    fi
+
 	    echo "image = ${CONFIG_IMAGE_FILE[$i]}"
 	    test -z "${CONFIG_IMAGE_LABEL[$i]}"  || echo "    label = ${CONFIG_IMAGE_LABEL[$i]}"
 	    test -z "${CONFIG_IMAGE_ROOT[$i]}"   || echo "    root = ${CONFIG_IMAGE_ROOT[$i]}"
