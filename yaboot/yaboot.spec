#
# spec file for package yaboot (Version 0.5)
# 
# Copyright  (c)  2000  SuSE GmbH  Nuernberg, Germany.
#
# please send bugfixes or comments to feedback@suse.de.
#

# neededforbuild  ext2fs_d
# usedforbuild    aaa_base aaa_dir autoconf automake base bash bindutil binutils bison bzip compress cpio cracklib devs diff ext2fs ext2fs_d file fileutil find flex gawk gcc gdbm gettext gpm gppshare groff gzip kbd less libc libtool libz lx_suse make mktemp modules ncurses net_tool netcfg nkita nkitb nssv1 pam patch perl pgp ps rcs rpm sendmail sh_utils shadow shlibs strace syslogd sysvinit texinfo textutil timezone unzip util vim xdevel xf86 xshared

Vendor:       SuSE GmbH, Nuernberg, Germany
Distribution: SuSE Linux 6.4 (PPC)
Name:         yaboot
Release:      10
Packager:     feedback@suse.de

Summary:      YaBoot - OF boot loader for PowerMac
Version:      0.5
Copyright: GPL
Group: Unsorted
Source0: yaboot-0.5.tar.gz
Source1: yaboot_0.5.gz
Source2: os-chooser.txt
Patch0: yaboot-0.5.dif

%description
Yaboot is an OpenFirmware based bootloader for newworld machines.
Netbooting works via bootp/tftp (including loading the ramdisk).

Authors:
--------
    Benjamin Herrenschmidt <bh40@calva.net>

SuSE series: a

%prep
%setup
%patch0

%build
sed 's/^DEBUG.*$/DEBUG = 1/' Makefile > Makefile.debug
sed 's/^DEBUG.*$/DEBUG = 0/' Makefile > Makefile.nodebug
make -f Makefile.debug
mv yaboot yaboot.debug
make clean
make -f Makefile.nodebug

%install
cp -a $RPM_SOURCE_DIR/yaboot_0.5.gz .
mkdir -p /var/lib/yaboot/
cp -a $RPM_SOURCE_DIR/os-chooser.txt /var/lib/yaboot/os-chooser
%{?suse_check}

%files
%doc COPYING yaboot yaboot.debug yaboot_0.5.gz
/var/lib/yaboot

%changelog -n yaboot
* Thu Apr 06 2000 - olh@suse.de
- provide the os-chooser script for the MacOS side
* Fri Mar 31 2000 - olh@suse.de
- extended Welcome message
* Wed Mar 22 2000 - uli@suse.de
- fixed previous fix
- added workaround for "conf from hfs, kernel from ext2" case
* Tue Mar 21 2000 - uli@suse.de
- load images from same folder as yaboot.conf
  add asterisk for default config
* Mon Mar 20 2000 - uli@suse.de
- fix default defdevice
* Fri Mar 10 2000 - olh@suse.de
- assume that the kernel is on the same partition than yaboot.conf
* Mon Mar 06 2000 - olh@suse.de
- update to version 0.5, provide also a debug binary
* Mon Jan 17 2000 - olh@suse.de
- add yaboot to SuSE PPC dist
