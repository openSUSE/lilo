#
# spec file for package lilo (Version 0.0.1)
# 
# Copyright  (c)  2000  SuSE GmbH  Nuernberg, Germany.
#
# please send bugfixes or comments to feedback@suse.de.
#

# neededforbuild  ext2fs_d tetex
# usedforbuild    aaa_base aaa_dir autoconf automake base bash bindutil binutils bison bzip compress cpio cracklib devs diff ext2fs ext2fs_d file fileutil find flex gawk gcc gdbm gettext gpm gppshare groff gzip kbd less libc libtool libz lx_suse make mktemp modules ncurses net_tool netcfg nkita nkitb nssv1 pam patch perl pgp ps rcs rpm sendmail sh_utils shadow shlibs strace syslogd sysvinit te_ams te_latex tetex texinfo textutil timezone unzip util vim xdevel xf86 xshared

Vendor:       SuSE GmbH, Nuernberg, Germany
Distribution: SuSE Linux 7.0a (PPC)
Name:         lilo
Release:      0
Packager:     feedback@suse.de

Copyright:      GPL
Obsoletes:	yaboot activate quik 
Requires:	hfsutils
Summary:      LILO
Version:      0.0.1
Group:		bootloader
Source0: 	lilo-0.0.1.tar.gz
Patch0:		lilo-0.0.1.dif
Source1:	yaboot-0.5.tar.gz
Patch1:		yaboot-0.5.dif
Source2:	quik-2.0.tar.gz
Patch2:		quik-2.0.dif
Source3:	lilo-21.tar.gz
Buildroot:	/var/tmp/buildroot-lilo

%description
The LInux-LOader: LILO boots Linux from your hard drive.
It can also boot other operating systems such as MS-DOS and OS/2,
and can even boot DOS from the second hard drive.
The configuration file is /etc/lilo.conf.
The PowerPC variant can be used on new PowerMacs and CHRP machines.

Authors:
--------
    Werner Almesberger <almesber@di.epfl.ch>
    PowerPC part:
    Paul Mackeras <paulus@linuxcare.com.au>
    Cort Dougan <cort@fsmlabs.com>
    Benjamin Herrenschmidt <bh40@calva.net>

SuSE series: a

%prep
%setup -q -T -c -a 0 -a 1 -a 2 -a 3
cd yaboot-0.5
%patch1
cd ..
cd quik-2.0
%patch2
cd ..

%build
cd yaboot-0.5
sed 's/^DEBUG.*$/DEBUG = 1/' Makefile > Makefile.debug
sed 's/^DEBUG.*$/DEBUG = 0/' Makefile > Makefile.nodebug
make -f Makefile.debug
mv yaboot yaboot.debug
make clean
make -f Makefile.nodebug
cd ..
cd quik-2.0
make
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
cd lilo-0.0.1
chmod 755 show_of_path.sh
chmod 754 lilo.sh
cp -av lilo.sh $RPM_BUILD_ROOT/sbin/lilo
cp -av show_of_path.sh $RPM_BUILD_ROOT/bin
cp -av Finder.bin $RPM_BUILD_ROOT/boot
cp -av System.bin $RPM_BUILD_ROOT/boot
cp -av lilo.conf $RPM_BUILD_ROOT/etc
cp -av README $RPM_BUILD_ROOT%{_docdir}/lilo/
cp -av COPYING $RPM_BUILD_ROOT%{_docdir}/lilo/
cd ..
cd yaboot-0.5
cp -av yaboot yaboot.debug $RPM_BUILD_ROOT/boot/
cd ..
cd quik-2.0
cp -av second/second $RPM_BUILD_ROOT/boot/
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
%{?suse_check}

%files
/boot/second
/boot/Finder.bin
/boot/System.bin
/boot/yaboot
/boot/yaboot.debug
%config(noreplace)/etc/lilo.conf
/sbin/activate
/sbin/lilo
/bin/show_of_path.sh
%doc %{_docdir}/lilo

%changelog -n lilo
* Thu Jul 13 2000 - olh@suse.de
- update README and System.bin
* Wed Jul 12 2000 - olh@suse.de
- initial ppc release
