# norootforbuild

Url:            http://lilo.go.dyndns.org/

Name:           lilo
ExclusiveArch:  ppc ppc64 %ix86 x86_64
%define yaboot_vers 22.8-r1151
Group:          System/Boot
License:        BSD 3-Clause
Summary:        The Linux Loader, a Boot Menu
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Obsoletes:      yaboot activate quik 
%ifarch ppc ppc64
%if 0%{?suse_version} > 1020
BuildRequires:  dtc
%endif
Requires:       hfsutils
Requires:       dosfstools
Requires:       gawk sed coreutils
# for nvsetenv
%if 0%{?suse_version} > 1000
Requires:       powerpc-utils
%else
Requires:       util-linux
%endif
Requires:       binutils
Requires:       parted
%endif
%ifarch %ix86 x86_64
BuildRequires:  bin86 nasm
%endif
%ifarch x86_64
BuildRequires:  gcc-32bit glibc-devel-32bit libgcc42-32bit libmudflap42-32bit
%endif
Version:        0
Release:        0
Source0:        lilo-ppc-%{version}.tar.bz2
Source1:        http://penguinppc.org/projects/yaboot/yaboot-%{yaboot_vers}.tar.bz2
Source86:       lilo-%{version}.src.tar.bz2
Patch8601:      lilo.x86.mount_by_persistent_name.patch
Patch8602:      lilo.x86.array-bounds.patch
Patch8603:      lilo.x86.division-by-zero.patch
# $Id$

%description
lilo for ppc

%prep
%setup -q -T -c -a 0 -a 1 -a 86
mv lilo-ppc-%{version} lilo.ppc
mv yaboot-%{yaboot_vers} yaboot
cd lilo-%{version}
%patch8601 -p1
%patch8602 -p1
%patch8603 -p1

%build
%ifarch %ix86 x86_64
cd lilo-%{version}
cflags="$RPM_OPT_FLAGS -fno-strict-aliasing"
%ifarch x86_64
cflags="$cflags -m32"
%endif
make CC="gcc $cflags" MAN_DIR=/usr/share/man all activate
# powerpc
%else
cd yaboot
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
cd ..
#
cd lilo.ppc
cd bootheader
make HOST_CFLAGS="$RPM_OPT_FLAGS -U_FORTIFY_SOURCE -g"
%endif

%install
%ifarch %ix86 x86_64
cd lilo-%{version}
make MAN_DIR=/usr/share/man install ROOT=$RPM_BUILD_ROOT
install -m 0755 activate $RPM_BUILD_ROOT/sbin
rm -rfv $RPM_BUILD_ROOT/boot
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
cd lilo.ppc
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
cd bootheader
make install DESTDIR=$RPM_BUILD_ROOT
cd ..
cd ..
cd yaboot
cp -av yaboot yaboot.debug $RPM_BUILD_ROOT/lib/lilo/pmac
cp -av yaboot.chrp* $RPM_BUILD_ROOT/lib/lilo/chrp
cp -av crt0.o $RPM_BUILD_ROOT/lib/lilo/chrp/yaboot.crt0.o
cp -av ld.script $RPM_BUILD_ROOT/lib/lilo/chrp/yaboot.ld.script
cp -av yaboot.a $RPM_BUILD_ROOT/lib/lilo/chrp/
cp -av make_yaboot.sh $RPM_BUILD_ROOT/lib/lilo/scripts/
cp -av man/bootstrap.8 man/yaboot.8 $RPM_BUILD_ROOT%{_mandir}/man8
cp -av man/yaboot.conf.5 $RPM_BUILD_ROOT%{_mandir}/man5
cd ..
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
