#
# spec file for package lilo (Version 0.0.8)
#
# Copyright (c) 2002 SuSE Linux AG, Nuernberg, Germany.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# please send bugfixes or comments to feedback@suse.de.
#

# neededforbuild  tetex
# usedforbuild    aaa_base aaa_dir aaa_version autoconf automake base bash bindutil binutils bison bzip compat cpio cpp cracklib cyrus-sasl db devs diffutils e2fsprogs file fileutils findutils flex gawk gcc gdbm gdbm-devel gettext glibc glibc-devel glibc-locale gpm grep groff gzip kbd less libgcc libstdc++ libtool libxcrypt libz m4 make man mktemp modutils ncurses ncurses-devel net-tools netcfg pam pam-devel pam-modules patch perl permissions ps rcs readline rpm sed sendmail sh-utils shadow strace syslogd sysvinit tar te_ams te_latex tetex texinfo textutils timezone unzip util-linux vim

Name:         lilo
Group:        System/Boot
License:      BSD License and BSD-like, Other License(s), see package
Obsoletes:    yaboot activate quik 
Requires:     hfsutils
Summary:      LInux LOader
Version:      0.0.8
Release:      251
Source0:      lilo-0.0.6.tar.bz2
Patch0:       lilo-0.0.6.dif
Source1:      compatible_machines.txt
Source3:      lilo-21.tar.gz
Source5:      yaboot-1.3.6.tar.gz
Patch5:       yaboot-1.3.6.dif
Patch6:       yaboot-symlink-fix.diff
BuildRoot:    %{_tmppath}/%{name}-%{version}-build
# get rid of /usr/lib/rpm/brp-strip-debug 
# it kills the zImage.chrp-rs6k 
%define __os_install_post %{nil}

%description
The LInux-LOader: LILO boots Linux from your hard drive.
It can also boot other operating systems such as MS-DOS and OS/2,
and can even boot DOS from the second hard drive.
The configuration file is /etc/lilo.conf.

The PowerPC variant can be used on new PowerMacs and CHRP machines.

The ix86 variant comes along with Memtest86, offering an image that
can be booted instead of a real OS and doing a memory test.

Authors:
--------
    John Coffman <JohnInSD@san.rr.com>
    Werner Almesberger <Werner.Almesberger@epfl.ch>
    PowerPC part:
    Paul Mackeras <paulus@samba.org>
    Cort Dougan <cort@fsmlabs.com>
    Benjamin Herrenschmidt <benh@kernel.crashing.org>
    Memtest86:
    Chris Brady <crsbrady@earthlink.net>

SuSE series: a

%prep
%setup -q -T -c -a 0 -a 3 -a 5
mv lilo-0.0.6	lilo.ppc
mv yaboot-1.3.6 yaboot
cd yaboot
%patch5
%patch6 -p1
cd ..

%build
cd yaboot
make clean
make DEBUG=1 VERSION=1.3.6.SuSE yaboot
mv second/yaboot yaboot.debug
make clean
make DEBUG=0 VERSION=1.3.6.SuSE yaboot
mv second/yaboot yaboot
make clean
make DEBUG=1 VERSION=1.3.6.SuSE yaboot.chrp
mv second/yaboot.chrp yaboot.chrp.debug
make clean
make DEBUG=0 VERSION=1.3.6.SuSE yaboot.chrp
mv second/yaboot.chrp yaboot.chrp
cd ..
cd lilo
make activate
cd ..

%install
rm -rfv $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/boot
mkdir -p $RPM_BUILD_ROOT/etc
mkdir -p $RPM_BUILD_ROOT/sbin
mkdir -p $RPM_BUILD_ROOT/bin
mkdir -p $RPM_BUILD_ROOT/%{_docdir}/lilo/activate
cd lilo.ppc
chmod 755 show_of_path.sh
chmod 754 lilo.{old,new}
cp -av lilo.old $RPM_BUILD_ROOT/sbin/lilo.old
cp -av lilo.new $RPM_BUILD_ROOT/sbin/lilo
cp -av show_of_path.sh $RPM_BUILD_ROOT/bin
cp -av Finder.bin $RPM_BUILD_ROOT/boot
cp -av System.bin $RPM_BUILD_ROOT/boot
cp -av %{SOURCE1} $RPM_BUILD_ROOT/boot
cp -av lilo.conf $RPM_BUILD_ROOT/etc
cp -av README* $RPM_BUILD_ROOT%{_docdir}/lilo/
cp -av COPYING $RPM_BUILD_ROOT%{_docdir}/lilo/
cp -av lilo.changes $RPM_BUILD_ROOT%{_docdir}/lilo/
cd ..
cd yaboot
cp -av yaboot yaboot.debug $RPM_BUILD_ROOT/boot/
cp -av yaboot.chrp* $RPM_BUILD_ROOT/boot/
cd ..
cd lilo
install -oroot -groot activate $RPM_BUILD_ROOT/sbin
install -d -m 755 $RPM_BUILD_ROOT%{_docdir}/lilo/activate
install -m 644 CHANGES COPYING INCOMPAT README $RPM_BUILD_ROOT%{_docdir}/lilo/activate
rm -f doc/*.dvi
make -C doc all
( cd doc ; install -m 644 tech.dvi $RPM_BUILD_ROOT%{_docdir}/lilo/activate/ )
( cd doc ; install -m 644 user.dvi $RPM_BUILD_ROOT%{_docdir}/lilo/activate/ )
( cd doc ; dvips tech.dvi )
( cd doc ; gzip -f tech.ps )
( cd doc ; install -m 644 tech.ps.gz $RPM_BUILD_ROOT%{_docdir}/lilo/activate/ )
( cd doc ; dvips user.dvi )
( cd doc ; gzip -f user.ps )
( cd doc ; install -m 644 user.ps.gz $RPM_BUILD_ROOT%{_docdir}/lilo/activate/ )
cd ..

%files
/boot/Finder.bin
/boot/System.bin
/boot/compatible_machines.txt
/boot/yaboot
/boot/yaboot.debug
/boot/yaboot.chrp*
%config(noreplace)/etc/lilo.conf
/sbin/activate
/sbin/lilo*
/bin/show_of_path.sh
%doc %{_docdir}/lilo

%changelog -n lilo
* Wed Jul 03 2002 - olh@suse.de
- use MacRISC as compatible string on pmac
* Sat Jun 29 2002 - olh@suse.de
- add yaboot-symlink-fix.diff (#16742), allow symlinks on reiserfs
* Mon Jan 14 2002 - olh@suse.de
- remove exit 0, no quoting for lilo.conf variables
* Fri Jan 11 2002 - olh@suse.de
- build a boot image with initrd=  on iSeries, when specified
* Thu Jan 10 2002 - olh@suse.de
- do not write to slot C on iSeries
* Wed Jan 09 2002 - olh@suse.de
- do not write to slot A on iSeries
* Wed Jan 09 2002 - olh@suse.de
- add root= option on iseries
* Tue Jan 08 2002 - olh@suse.de
- do not honor the boot-file/bootargs content in yaboot
* Tue Jan 08 2002 - olh@suse.de
- run lilo on iSeries
* Thu Dec 20 2001 - olh@suse.de
- do not call rpm postinstall
* Thu Dec 13 2001 - olh@suse.de
- update to yaboot 1.3.6, remove some partition type braindamage
* Tue Dec 04 2001 - olh@suse.de
- add new PowerBook
* Tue Dec 04 2001 - olh@suse.de
- running lilo on iSeries
* Tue Nov 13 2001 - olh@suse.de
- double MAX_TOKEN size to allow large init-message
* Thu Nov 08 2001 - olh@suse.de
- the last yaboot patch screwed the older changes, apply them again
* Thu Nov 08 2001 - olh@suse.de
- add patch to allow loading for zImage.initrd on ppc64
* Mon Oct 15 2001 - olh@suse.de
- fix yaboot.conf creation, sysmap= was wrong
* Mon Oct 15 2001 - olh@suse.de
- add patch from jeffm to fix image loading on chrp+reiserfs
* Mon Oct 08 2001 - olh@suse.de
- do nothing on oldworld when called with --lilo-rootdrive
  might be bad for your MacOS partition
* Wed Oct 03 2001 - olh@suse.de
- use --lilo-rootdrive when df / produce bogus output
  needed for installer
* Mon Sep 24 2001 - olh@suse.de
- do not exit when sysmap can not be loaded
* Sat Sep 22 2001 - olh@suse.de
- fix spacebar message
* Fri Sep 21 2001 - olh@suse.de
- change type of Mac OS Rom and install
* Fri Sep 21 2001 - olh@suse.de
- update yaboot to 1.2.5, fixes for ppc64
* Thu Sep 20 2001 - olh@suse.de
- fix sysmap for pmac
* Wed Sep 19 2001 - olh@suse.de
- fix sysmap path for chrp
* Sat Sep 15 2001 - olh@suse.de
- make spacebar useage more clear
* Fri Sep 14 2001 - olh@suse.de
- improve handling of spacebar in os-chooser
* Wed Sep 05 2001 - olh@suse.de
- fix specfile
* Wed Sep 05 2001 - olh@suse.de
- update yaboot to 1.2.3, use new os-chooser version per default
* Tue Aug 14 2001 - olh@suse.de
- fix filelist
* Mon Aug 13 2001 - olh@suse.de
- update compatible_machines.txt comment
* Mon Aug 13 2001 - olh@suse.de
- update lilo.sh, fix handling of HFSBOOTFOLDER
  use a different way to query keys in os-chooser script
  fix yaboot, netboot was broken in last update
* Wed Aug 08 2001 - olh@suse.de
- update compatible_machines.txt
* Wed Aug 08 2001 - olh@suse.de
- move compatible_machines.txt
  dont force copy on new Macs until yaboot is fixed
* Thu Aug 02 2001 - olh@suse.de
- fix birec calculation, needs still work for HFS load ...
* Wed Aug 01 2001 - olh@suse.de
- set sysmap_base to 0
* Tue Jul 31 2001 - olh@suse.de
- add yaboot-fix.diff for reiserfs mount
* Tue Jul 31 2001 - olh@suse.de
- update default lilo.conf
* Tue Jul 31 2001 - olh@suse.de
- update to current yaboot patch for reiserfs (jeffm)
  add sysmap loading patch and update version to 1.2.2
* Mon Jul 23 2001 - olh@suse.de
- fix the default= option when it is macos, use first kernel label
* Mon Jul 02 2001 - olh@suse.de
- bring back some modifications
  always copy the files on the new MacRISC2 machines
* Thu Jun 28 2001 - olh@suse.de
- fix yaboot.c  file_close
* Thu Jun 28 2001 - olh@suse.de
- miboot can read a config file now
* Mon Jun 18 2001 - olh@suse.de
- honor default= line in lilo.conf for macos booting
* Sat Jun 02 2001 - olh@suse.de
- update yaboot to 1.2.1, add reiserfs patches
* Fri Mar 09 2001 - olh@suse.de
- rename os-chooser to Mac OS Rom
  add some support for Mac OS X to lilo.conf
* Mon Mar 05 2001 - olh@suse.de
- add PowerMac4,1 for new flower power iMacs
* Tue Feb 27 2001 - olh@suse.de
- add PowerMac3,4 for new G4/466
* Tue Feb 27 2001 - olh@suse.de
- enable initrd creation again, loop-6 fix most problems
* Tue Feb 27 2001 - olh@suse.de
- update to 0.0.7
  update yaboot to 1.1.1, obsoletes chrp64 binary
  change /sbin/lilo to handle the new files
* Thu Feb 15 2001 - olh@suse.de
- disable misleading debug printf in yaboot
* Wed Feb 14 2001 - olh@suse.de
- handle first generation iMac in lilo
* Sun Feb 11 2001 - olh@suse.de
- handle /dev/hde in show_of_path.sh
* Sun Feb 11 2001 - olh@suse.de
- skip initrd creation with 2.4 kernel and old pmacs
  until the loop device is fixed
* Sun Feb 11 2001 - olh@suse.de
- add small fixes for chrp to yaboot
* Wed Jan 31 2001 - olh@suse.de
- add " screen" output to lilo itself
* Tue Jan 30 2001 - olh@suse.de
- activate partitions via nvsetenv on new PowerMacs
* Tue Jan 30 2001 - olh@suse.de
- avoid screen garbage on chrp serial console
* Tue Jan 30 2001 - olh@suse.de
- disable " screen" output, doesnt work anyway
* Sun Dec 17 2000 - olh@suse.de
- use yaboot 0.9 on pmac and chrp
* Sun Dec 17 2000 - olh@suse.de
- add support for System.map loading (sysmap=)
* Fri Dec 01 2000 - olh@suse.de
- remove quik, build debug binaries on CHRP
* Tue Oct 24 2000 - olh@suse.de
- clear BSS on chrp, fix typo in /sbin/lilo, use always yaboot.conf
* Thu Oct 12 2000 - olaf@suse.de
- update to yaboot 0.9 for chrp only, allows bootable CDs
* Mon Oct 09 2000 - olh@suse.de
- add POWER3 support to install yaboot.chrp{,.64}
* Fri Sep 29 2000 - olh@suse.de
- disable debug on chrp
* Wed Sep 27 2000 - olh@suse.de
- exit when no boot= is specified
* Wed Sep 27 2000 - olh@suse.de
- disable debug in yaboot.chrp, fix <NULL> output in defaultimage
* Thu Sep 21 2000 - olh@suse.de
- fix show_of_path, lilo.sh and yaboot.c at once
* Thu Sep 21 2000 - olh@suse.de
- update yaboot for chrp, update lilo to handle chrp
* Mon Sep 11 2000 - olh@suse.de
- add video=platinumfb:cmode:8 to System.bin, prevents garbage
* Sun Sep 10 2000 - olh@suse.de
- update to 0.0.6, update yaboot to 0.8
* Thu Jul 20 2000 - olh@suse.de
- update lilo to 0.0.3
* Wed Jul 19 2000 - olh@suse.de
- update lilo to 0.0.2, adapt quik
* Thu Jul 13 2000 - olh@suse.de
- update README and System.bin
* Wed Jul 12 2000 - olh@suse.de
- initial ppc release
