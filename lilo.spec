#
# spec file for package lilo
#
# Copyright (c) 2011 SUSE LINUX Products GmbH, Nuernberg, Germany.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

# norootforbuild

Url:            http://lilo.go.dyndns.org/

Name:           lilo
ExclusiveArch:  ppc ppc64 %ix86 x86_64
%define yaboot_vers 22.8-r1190
Group:          System/Boot
License:        BSD3c
Summary:        The Linux Loader, a Boot Menu
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Obsoletes:      yaboot activate quik
%ifarch ppc ppc64
%if 0%{?suse_version} > 1020
BuildRequires:  dtc
%endif
Requires:       hfsutils
Requires:       dosfstools
Requires:       gawk
Requires:       sed
Requires:       coreutils
# for relinking the prep/chrp images in lilo
%ifarch ppc ppc64
Requires:       gcc
%endif
# for nvram
%if 0%{?suse_version} > 1000
Requires:       powerpc-utils >= 1.2.6
%else
Requires:       util-linux
%endif
Requires:       binutils
Requires:       parted
%endif
%ifarch %ix86 x86_64
BuildRequires:  bin86
BuildRequires:  nasm
%endif
%ifarch %ix86
BuildRequires:  device-mapper
BuildRequires:  device-mapper-devel
%endif
%ifarch x86_64
BuildRequires:  glibc-devel-32bit
%if 0%{?suse_version} > 1010
BuildRequires:  device-mapper-32bit
BuildRequires:  device-mapper-devel-32bit
BuildRequires:  gcc-32bit
%endif
%endif
# note: already outdated; download fresh sources from: https://alioth.debian.org/frs/?group_id=100507
Version:        22.8
Release:        58
Source0:        lilo-ppc-%{version}.tar.bz2
Source1:        http://penguinppc.org/projects/yaboot/yaboot-%{yaboot_vers}.tar.bz2
Source86:       lilo-%{version}.src.tar.bz2
Patch8601:      lilo.x86.mount_by_persistent_name.patch
Patch8602:      lilo.x86.array-bounds.patch
Patch8603:      lilo.x86.division-by-zero.patch
Patch8604:      lilo.x86.checkit.patch
Patch8605:      lilo-no-build-date.patch
Patch8606:      lilo.ppc.nvram-fix.patch	
Patch8607:      yaboot-libgcc.patch
Patch8608:      lilo-libgcc.patch
# $Id: lilo.spec 1188 2008-12-09 14:29:53Z olh $

%description
LILO boots Linux from your hard drive. It can also boot other operating
systems, such as MS-DOS and OS/2, and can even boot DOS from the second
hard drive. The configuration file is /etc/lilo.conf.

The PowerPC variant can be used on new PowerMacs and CHRP machines.

The ix86 variant comes with Memtest86, offering an image that can be
booted to perform a memory test.

%prep
%setup -q -T -c -a 0 -a 1 -a 86
mv lilo-ppc-%{version} lilo.ppc
mv yaboot-%{yaboot_vers} yaboot
pushd lilo-%{version}
%patch8601 -p1
%patch8602 -p1
%patch8603 -p1
%patch8604 -p1
%patch8605
popd
%patch8606
pushd yaboot
%patch8607 -p1
popd
pushd lilo.ppc
%patch8608 -p1
popd

%build
%ifarch %ix86 x86_64
pushd lilo-%{version}
cflags="$RPM_OPT_FLAGS -fno-strict-aliasing"
%ifarch x86_64
cflags="$cflags -m32"
%endif
make CC="gcc $cflags" MAN_DIR=/usr/share/man all activate
popd
# powerpc
%else
pushd yaboot
#
make clean
make DEBUG=1 VERSION=%{yaboot_vers}.SuSE yaboot HOSTCFLAGS="$RPM_OPT_FLAGS -U_FORTIFY_SOURCE -g"
mv second/yaboot yaboot.debug
mv second/yaboot.chrp yaboot.chrp.debug
#
make clean
make DEBUG=0 VERSION=%{yaboot_vers}.SuSE yaboot HOSTCFLAGS="$RPM_OPT_FLAGS -U_FORTIFY_SOURCE -g"
mv second/yaboot yaboot
mv second/yaboot.chrp yaboot.chrp
mv second/yaboot.a second/crt0.o .
#
popd
#
pushd lilo.ppc
pushd bootheader
make HOST_CFLAGS="$RPM_OPT_FLAGS -U_FORTIFY_SOURCE -g"
popd
popd
%endif

%install
%ifarch %ix86 x86_64
pushd lilo-%{version}
make MAN_DIR=/usr/share/man install ROOT=$RPM_BUILD_ROOT
install -m 0755 activate $RPM_BUILD_ROOT/sbin
rm -rfv $RPM_BUILD_ROOT/boot
mkdir -p $RPM_BUILD_ROOT/boot
cp -av *.b $RPM_BUILD_ROOT/boot
popd
%else
# powerpc
# get rid of /usr/lib/rpm/brp-strip-debug 
# it kills the zImage.chrp-rs6k 
export NO_BRP_STRIP_DEBUG=true
# do not strip binaries, keep debug info
export NO_DEBUGINFO_STRIP_DEBUG=true
#
mkdir -p $RPM_BUILD_ROOT/lib/lilo/pmac
mkdir -p $RPM_BUILD_ROOT/lib/lilo/chrp
mkdir -p $RPM_BUILD_ROOT/sbin
mkdir -p $RPM_BUILD_ROOT/bin
mkdir -p $RPM_BUILD_ROOT/bin
mkdir -p $RPM_BUILD_ROOT/%{_docdir}/lilo/activate
mkdir -p $RPM_BUILD_ROOT%{_mandir}/man8
mkdir -p $RPM_BUILD_ROOT%{_mandir}/man5
pushd lilo.ppc
cp -av lilo.new $RPM_BUILD_ROOT/sbin/lilo
cp -av lilo-pmac.lib $RPM_BUILD_ROOT/lib/lilo/lilo-pmac.lib
cp -av lilo-chrp.lib $RPM_BUILD_ROOT/lib/lilo/lilo-chrp.lib
cp -av lilo-iseries.lib $RPM_BUILD_ROOT/lib/lilo/lilo-iseries.lib
cp -av show_of_path.sh $RPM_BUILD_ROOT/bin
cp -av Finder.bin $RPM_BUILD_ROOT/lib/lilo/pmac
cp -av System.bin $RPM_BUILD_ROOT/lib/lilo/pmac
cp -av os-badge-icon $RPM_BUILD_ROOT/lib/lilo/pmac
cp -av README* $RPM_BUILD_ROOT%{_docdir}/lilo/
cp -av COPYING $RPM_BUILD_ROOT%{_docdir}/lilo/
cp -av man/lilo.conf.5 $RPM_BUILD_ROOT%{_mandir}/man5
cp -av man/lilo.8 $RPM_BUILD_ROOT%{_mandir}/man8
cp -av man/show_of_path.sh.8 $RPM_BUILD_ROOT%{_mandir}/man8
pushd bootheader
make install DESTDIR=$RPM_BUILD_ROOT
popd
popd
pushd yaboot
cp -av yaboot yaboot.debug $RPM_BUILD_ROOT/lib/lilo/pmac
cp -av yaboot.chrp* $RPM_BUILD_ROOT/lib/lilo/chrp
cp -av crt0.o $RPM_BUILD_ROOT/lib/lilo/chrp/yaboot.crt0.o
cp -av ld.script $RPM_BUILD_ROOT/lib/lilo/chrp/yaboot.ld.script
cp -av yaboot.a $RPM_BUILD_ROOT/lib/lilo/chrp/
cp -av make_yaboot.sh $RPM_BUILD_ROOT/lib/lilo/scripts/
cp -av man/bootstrap.8 man/yaboot.8 $RPM_BUILD_ROOT%{_mandir}/man8
cp -av man/yaboot.conf.5 $RPM_BUILD_ROOT%{_mandir}/man5
popd
#powerpc
%endif

%triggerpostun  -- lilo < 0.0.10
# for manual updates
if [ -f /etc/lilo.conf.rpmsave -a ! -f /etc/lilo.conf ] ; then
mv -v /etc/lilo.conf.rpmsave /etc/lilo.conf
fi
exit 0

%files
%defattr (-,root,root)
%ifarch %ix86 x86_64
/sbin/*
/usr/sbin/*
/boot/*.b
%else
#powerpc
%dir /lib/lilo
%dir /lib/lilo/pmac
%dir /lib/lilo/prep
%dir /lib/lilo/chrp
%dir /lib/lilo/ps3
%dir /lib/lilo/common
%dir /lib/lilo/scripts
%dir /lib/lilo/utils
#
%attr(755,root,root) /bin/mkzimage_cmdline
%attr(755,root,root) %config /bin/show_of_path.sh
%attr(755,root,root) %config /bin/mkzimage
%attr(755,root,root) %config /lib/lilo/pmac/os-badge-icon
%attr(644,root,root) /lib/lilo/pmac/*.bin
%attr(644,root,root) /lib/lilo/pmac/yaboot*
%attr(644,root,root) /lib/lilo/ps3/*
%attr(644,root,root) /lib/lilo/*/*.o
%attr(644,root,root) /lib/lilo/*/*.a
%attr(644,root,root) %config /lib/lilo/*/*ld.script*
%attr(644,root,root) %config /lib/lilo/lilo-*.lib
%attr(644,root,root) /lib/lilo/chrp/yaboot.chrp*
%attr(755,root,root) %config /lib/lilo/scripts/*.sh
%attr(755,root,root) /lib/lilo/utils/*
%attr(755,root,root) %config /sbin/lilo
%doc %{_docdir}/lilo
%endif
%doc %{_mandir}/*/*

%changelog
