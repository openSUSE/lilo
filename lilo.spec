#
# spec file for package lilo (Version 0.0.7)
# 
# Copyright  (c)  2001  SuSE GmbH  Nuernberg, Germany.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
# 
# please send bugfixes or comments to feedback@suse.de.
#

# neededforbuild  tetex
# usedforbuild    aaa_base aaa_dir autoconf automake base bash bindutil binutils bison bzip compress cpio cpp cracklib cyrus-sasl db devs diffutils e2fsprogs file fileutils findutils flex gawk gcc gdbm gdbm-devel gettext glibc glibc-devel gpm gppshare grep groff gzip kbd less libtool libz m4 make man mktemp modutils ncurses ncurses-devel net-tools netcfg pam pam-devel patch perl ps rcs readline rpm sendmail sh-utils shadow strace syslogd sysvinit te_ams te_latex tetex texinfo textutils timezone unzip util-linux vim

Name:         lilo
Group: 	System Environment/Base
Copyright:      GPL
Obsoletes:	yaboot activate quik 
Requires:	hfsutils
Summary:      LInux LOader
Version:      0.0.7
Release:      25
Source0: 	lilo-0.0.6.tar.gz
#Patch0:		lilo-0.0.6.dif
Source3:	lilo-21.tar.gz
Source4:	linuxrc-1.1.13.olh.tar.gz
Source5:	yaboot-1.2.1.tar.gz
Patch5:		yaboot-1.2.1.dif
Buildroot:	/var/tmp/buildroot-lilo

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
    Paul Mackeras <paulus@linuxcare.com.au>
    Cort Dougan <cort@fsmlabs.com>
    Benjamin Herrenschmidt <bh40@calva.net>
    Memtest86:
    Chris Brady <cbrady@sgi.com>

SuSE series: a

%prep
%setup -q -T -c -a 0 -a 3 -a 4 -a 5
mv lilo-0.0.6	lilo.ppc
mv linuxrc-1.1.13.olh	linuxrc
mv yaboot-1.2.1 yaboot
cd yaboot
%patch5
cd ..

%build
cd yaboot
make DEBUG=1 VERSION=1.2.1.SuSE
mv yaboot yaboot.debug
mv yaboot.chrp yaboot.chrp.debug
make clean
make DEBUG=0 VERSION=1.2.1.SuSE
cd ..
cd lilo
make activate
cd ..
cd linuxrc
make
mkdir ramdisk
dd if=/dev/zero of=./initrd.pmacold bs=1024 count=1024
mke2fs -q -F -b 1024 -m 0  ./initrd.pmacold
mount -o rw,loop ./initrd.pmacold ./ramdisk
cd ramdisk
mkdir dev
mkdir proc
cp -av ../linuxrc .
chmod -v 755 linuxrc
cp -av /dev/{null,zero,ram,ram0,ram1,ram2,ramdisk,fb0,console} dev/
cd ..
umount ./ramdisk
gzip -v9 initrd.pmacold

%install
rm -rfv $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/boot
mkdir -p $RPM_BUILD_ROOT/etc
mkdir -p $RPM_BUILD_ROOT/sbin
mkdir -p $RPM_BUILD_ROOT/bin
mkdir -p $RPM_BUILD_ROOT/%{_docdir}/lilo/activate
cd lilo.ppc
chmod 755 show_of_path.sh
chmod 754 lilo.sh
cp -av lilo.sh $RPM_BUILD_ROOT/sbin/lilo
cp -av show_of_path.sh $RPM_BUILD_ROOT/bin
cp -av Finder.bin $RPM_BUILD_ROOT/boot
cp -av System.bin $RPM_BUILD_ROOT/boot
cp -av lilo.conf $RPM_BUILD_ROOT/etc
touch $RPM_BUILD_ROOT/etc/quik.conf
cp -av README* $RPM_BUILD_ROOT%{_docdir}/lilo/
cp -av COPYING $RPM_BUILD_ROOT%{_docdir}/lilo/
cp -av lilo.changes $RPM_BUILD_ROOT%{_docdir}/lilo/
cd ..
cd yaboot
cp -av yaboot yaboot.debug $RPM_BUILD_ROOT/boot/
cp -av yaboot.chrp* $RPM_BUILD_ROOT/boot/
cd ..
cd linuxrc
cp -av initrd.pmacold.gz $RPM_BUILD_ROOT/boot
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
/boot/initrd.pmacold.gz
/boot/yaboot
/boot/yaboot.debug
/boot/yaboot.chrp*
%config(noreplace)/etc/lilo.conf
/sbin/activate
/sbin/lilo
/bin/show_of_path.sh
%doc %{_docdir}/lilo

%changelog -n lilo
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
