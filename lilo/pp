--- work-pkg/lilo.d/lilo-0.1.1/show_of_path.sh	2004-10-07 17:24:13.000000000 +0200
+++ /bin/show_of_path.sh	2004-08-17 10:22:26.000000000 +0200
@@ -229,13 +229,21 @@
 	    file_of_hw_path="${file_of_hw_devtype##/proc/device-tree}/disk@$of_disk_ide_channel"
 	    ;;
         scsi)
-	    file_of_hw_path=$(printf "%s/sd@%x,%x"  "${file_of_hw_devtype##/proc/device-tree}" $of_disk_scsi_id $of_disk_scsi_lun)
+	    if [ -d ${file_of_hw_devtype}/disk ]; then
+		of_disk_scsi_dir=disk
+	    elif [ -d ${file_of_hw_devtype}/sd ]; then
+		of_disk_scsi_dir=sd
+	    else
+		echo >&2 "ERROR: Could not find a known hard disk directory under '${file_of_hw_devtype}'"
+		exit 1
+	    fi	
+	    file_of_hw_path=$(printf "%s/%s@%x,%x"  "${file_of_hw_devtype##/proc/device-tree}" $of_disk_scsi_dir $of_disk_scsi_id $of_disk_scsi_lun)
 	    ;;
         sata|pci-ide)
 	    file_of_hw_path=$of_device_path
 	    ;;
         vscsi)
	    (( of_disk_vscsi_nr = ( (2 << 14) | (of_disk_scsi_chan<<5) |  (of_disk_scsi_id<<8) |  of_disk_scsi_lun ) <<48 )); #
 	    if [ -d ${file_of_hw_devtype}/disk ]; then
 		of_disk_vscsi_dir=disk
 	    elif [ -d ${file_of_hw_devtype}/sd ]; then
