#!/bin/bash
#
#
#     this does not work anymore:
# 		SCR::Execute (.target.bash, "/bin/addRamDisk64 /boot/initrd $(rpm
#     -ql kernel-iseries64 | grep ^/lib/modules/.\\\*/System.map$)  $(rpm -ql
#     kernel-iseries64 | grep ^/lib/modules/.\\\*/vmlinux$) " + iseries_bootbinary + "
#     >> /var/log/YaST2/y2loglilo_iseries_bootbinary 2>&1 " );
#     vmlinux is not in /lib/modules
#
#     fix:
#     create a mkzimage script, it takes --vmlinux [--initrd] and --output  as
#     argument, its a simple frontend for the /lib/lilo/*/mkzimage*.sh scripts.
#     in case of iseries, use nm to create a System.map from the ELF vmlinux file and
#     feed it to addRamdisk.
#     addRamdisk is not in /bin/ anymore.
#
#     so yast should call
#     /bin/mkzimage [--target iseries64] -k /boot/$bootfile -i /boot/$initrd -o $tmp/$foo
#     mkzimage will create the $foo file, which can be written with dd.
#


#
#     taken from: Bugzilla Bug 35570 - iseries initrd handling
#





#
#
# Local variables:
#     mode: ksh
#     ksh-indent: 4
#     ksh-multiline-offset: 2
#     ksh-if-re: "\\s *\\b\\(if\\)\\b[^=]"
# End:
#



  