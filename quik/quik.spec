#
# spec file for package quik (Version 2.0)
# 
# Copyright  (c)  2000  SuSE GmbH  Nuernberg, Germany.
#
# please send bugfixes or comments to feedback@suse.de.
#

# neededforbuild  ext2fs_d
# usedforbuild    aaa_base aaa_dir base bash bindutil binutils bison bzip compress cpio cracklib devs diff ext2fs ext2fs_d file fileutil find flex gawk gcc gdbm gettext gpm gppshare groff gzip kbd less libc libz lx_suse make mktemp modules ncurses net_tool netcfg nkita nkitb nssv1 pam patch perl pgp ps rcs rpm sendmail sh_utils shadow shlibs strace syslogd sysvinit texinfo textutil timezone unzip util vim xdevel xf86 xshared

Vendor:       SuSE GmbH, Nuernberg, Germany
Distribution: SuSE Linux 6.4 (PPC)
Name:         quik
Release:      12
Packager:     feedback@suse.de

Summary:      quik - bootloader for CHRP machines
Version:      2.0
Copyright: GPL
Group: Base
URL: http://www.ppc.kernel.org/
Source: ftp://ftp.ppc.kernel.org/pub/linuxppc/quik/quik-2.0.tar.gz
Patch: quik-2.0.dif
BuildRoot: /var/tmp/buildroot-quik

%description
The quik package provides the functionality neccessary for booting a
Linux/PPC PowerMac or CHRP system from disk.

Authors:
--------
    Cort Dougan <cort@ppc.kernel.org>

SuSE series: a

%prep
%setup
%patch -p1

%build
make

%install
make install DESTDIR="$RPM_BUILD_ROOT"
%{?suse_check}

%post
if [ ! -z "`grep CHRP /proc/cpuinfo 2>/dev/null`" ] ; then
	echo CHRP system detected.
#	PART=`fdisk -l | fgrep "PPC PReP"`
	PART=`cfdisk -P s | fgrep "PPC PReP"`
	if [ -z "$PART" ] ; then
	    echo '*********************************************************'
	    echo '* You must create a PPC PReP Boot partition (type 0x41) *'
	    echo '* for the CHRP bootloader to be installed.              *'
	    echo '*********************************************************'
	    exit -1
	fi
	if [ `echo "$PART" | wc -l` != 1 ] ; then
	    echo '**************************************************************'
	    echo '* There are more than 1 PPC PReP Boot partitions (type 0x41) *'
	    echo '* on this system.  Aborting install rather than picking one. *'
	    echo '**************************************************************'
	    exit -1
	fi
	if [ -z "`echo "$PART" | grep "Boot (80)"`" ] ; then
	    echo '***************************************************************'
	    echo '* The PPC PReP Boot partition (type 0x41) is not marked as    *'
	    echo '* bootable.  You will need to mark this partition as bootable *'
	    echo '* before Quik can be installed onto it.                       *'
	    echo '**************************************************************'
	    exit -1
	fi
	P=`echo "$PART" | awk '{print $1}'`
	# assume /dev/sda!!!
	if [ "${P}" != "1" ] ; then
	    echo '***************************************************************'
	    echo '* You do not have sda1 setup as the boot partition.  This     *'
	    echo '* will work but Quik will not know where the configuration    *'
	    echo '* file is.	                                                *'
	    echo '***************************************************************'
	fi
	P=/dev/sda${P}
	echo Installing onto $P
	dd if=boot/second of=$P
fi

%clean
rm -rf $RPM_BUILD_ROOT

%files
/sbin/quik
/boot/first.b
/boot/second.b
/boot/second
%config(noreplace) /etc/quik.conf
/usr/share/man/man?/*

%changelog -n quik
* Thu Mar 23 2000 - olh@suse.de
- load the kernel from the same partition as quik.conf
* Thu Mar 16 2000 - olh@suse.de
- search for /etc/quik.conf on all partitions, not only on fixed #2
* Sun Feb 06 2000 - olh@suse.de
- fix to find proper partition number
  /usr/man -> /usr/share/man
* Mon Jan 24 2000 - olh@suse.de
- add quik to SuSE PPC dist
