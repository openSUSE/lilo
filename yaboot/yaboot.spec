#
# spec file for package yaboot (Version 0.5)
# 
# Copyright  (c)  2000  SuSE GmbH  Nuernberg, Germany.
#
# please send bugfixes or comments to feedback@suse.de.
#

# neededforbuild  ext2fs_d
# usedforbuild    aaa_base aaa_dir base bash bindutil binutils bison bzip compress cpio cracklib devs diff ext2fs ext2fs_d file fileutil find flex gawk gcc gdbm gettext gpm gppshare groff gzip kbd less libc libz lx_suse make mktemp modules ncurses net_tool netcfg nkita nkitb nssv1 pam patch perl pgp ps rcs rpm sendmail sh_utils shadow shlibs strace syslogd sysvinit texinfo textutil timezone unzip util vim xdevel xf86 xshared

Vendor:       SuSE GmbH, Nuernberg, Germany
Distribution: SuSE Linux 6.4 (PPC)
Name:         yaboot
Release:      1
Packager:     feedback@suse.de

Summary:      YaBoot - OF boot loader for PowerMac
Version:      0.5
Copyright: GPL
Group: Unsorted
Source0: yaboot_0.5.src.tgz
Source1: yaboot_0.5.gz
Patch0: yaboot_0.5_bootpartition.diff

%description
Yaboot is an OpenFirmware based bootloader for newworld machines.
Netbooting works via bootp/tftp (including loading the ramdisk).

Authors:
--------
    Benjamin Herrenschmidt <bh40@calva.net>

SuSE series: a

%prep
%setup -n yaboot
%patch0 -p1

%build
sed 's/^DEBUG.*$/DEBUG = 1/' Makefile > Makefile.debug
sed 's/^DEBUG.*$/DEBUG = 0/' Makefile > Makefile.nodebug
make -f Makefile.debug
mv yaboot yaboot.debug
make clean
make -f Makefile.nodebug

%install
cp -a $RPM_SOURCE_DIR/yaboot_0.5.gz .
%{?suse_check}

%files
%doc COPYING yaboot yaboot.debug yaboot_0.5.gz

%changelog -n yaboot
* Fri Mar 10 2000 - olh@suse.de
- assume that the kernel is on the same partition than yaboot.conf
* Mon Mar 06 2000 - olh@suse.de
- update to version 0.5, provide also a debug binary
* Mon Jan 17 2000 - olh@suse.de
- add yaboot to SuSE PPC dist
