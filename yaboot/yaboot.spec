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
Distribution: SuSE Linux 7.0a (PPC)
Name:         yaboot
Release:      16
Packager:     feedback@suse.de

Version:      0.5
Requires:	hfsutils
Copyright: GPL
Group: unsorted
Summary:      YaBoot - OF boot loader for PowerMac
Buildroot: /var/tmp/buildroot-yaboot
Source0: yaboot-0.5.tar.gz
Source1: yaboot_0.5.gz
Source2: os-chooser.txt
Source3: ybin-0.11.tar.gz
Source4: show_of_path.sh
Patch0: yaboot-0.5.dif
Patch1: ybin-0.11.dif
%prep
%setup -c -T -a 0 -a 3
pwd
cd yaboot-0.5
%patch0
cd ..
cd ybin-0.11
%patch1
cd ..

%build
cd yaboot-0.5
sed 's/^DEBUG.*$/DEBUG = 1/' Makefile > Makefile.debug
sed 's/^DEBUG.*$/DEBUG = 0/' Makefile > Makefile.nodebug
make -f Makefile.debug
mv yaboot yaboot.debug
make clean
make -f Makefile.nodebug

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/boot/
mkdir -p $RPM_BUILD_ROOT/etc/
mkdir -p $RPM_BUILD_ROOT/sbin/
mkdir -p $RPM_BUILD_ROOT/usr/sbin/
mkdir -p $RPM_BUILD_ROOT%{_defaultdocdir}/yaboot/
mkdir -p $RPM_BUILD_ROOT/var/lib/yaboot/
cd yaboot-0.5
cp -av yaboot $RPM_BUILD_ROOT/boot/
cp -av yaboot.debug $RPM_BUILD_ROOT/boot/
cp -av COPYING  $RPM_BUILD_ROOT%{_defaultdocdir}/yaboot
cd ..
cd ybin-0.11
cp -av README* changelog BUGS TODO INSTALL $RPM_BUILD_ROOT%{_defaultdocdir}/yaboot
cp -av menu_ofboot.b $RPM_BUILD_ROOT/boot
cp -av ofboot.b $RPM_BUILD_ROOT/boot
cp -av yaboot.conf ybin.conf $RPM_BUILD_ROOT/etc
cp -av ybin $RPM_BUILD_ROOT/sbin
cp -av mkofboot $RPM_BUILD_ROOT/sbin
cp -av ybin.hfs $RPM_BUILD_ROOT/var/lib/yaboot/
cd ..
cp -av $RPM_SOURCE_DIR/yaboot_0.5.gz $RPM_BUILD_ROOT%{_defaultdocdir}/yaboot
cp -av $RPM_SOURCE_DIR/os-chooser.txt $RPM_BUILD_ROOT/var/lib/yaboot/os-chooser
cp -av $RPM_SOURCE_DIR/show_of_path.sh $RPM_BUILD_ROOT/usr/sbin/
chmod 755 $RPM_BUILD_ROOT/usr/sbin/show_of_path.sh
%{?suse_check}

%files
%doc %{_defaultdocdir}/yaboot
/boot/yaboot
/boot/yaboot.debug
/var/lib/yaboot
/sbin/ybin
/sbin/mkofboot
/etc/yaboot.conf
/etc/ybin.conf
/boot/ofboot.b
/boot/menu_ofboot.b
/usr/sbin/show_of_path.sh

%description
Yaboot is an OpenFirmware based bootloader for newworld machines.
Netbooting works via bootp/tftp (including loading the ramdisk).
This packages includes ybin, the yaboot installer.
Documentation is available at /usr/doc/packages/yaboot

Authors:
--------
    Benjamin Herrenschmidt <bh40@calva.net>
    Ethan Benson <erbenson@alaska.net>

SuSE series: a


%changelog -n yaboot
* Wed Apr 26 2000 - olh@suse.de
- get rid of strings in show_of_path.sh
* Thu Apr 13 2000 - olh@suse.de
- include ybin in the package
  move show_of_path.sh from aaa_base to this package
* Thu Apr 06 2000 - uli@suse.de
- fixed initrd loading bug introduced by conf hfs/kernel ext2
  workaround triggered when not loading from system folder
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
