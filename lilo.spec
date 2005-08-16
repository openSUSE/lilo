# norootforbuild
# neededforbuild  

Name:         lilo
%define yaboot_vers 0
Group:        System/Boot
License:      BSD, Other License(s), see package
Summary:      The LInux LOader, a boot menu
BuildRoot:    %{_tmppath}/%{name}-%{version}-build
Obsoletes:    yaboot activate quik 
Requires:     hfsutils
Requires:     dosfstools
Requires:     /bin/awk /usr/bin/od /bin/sed /usr/bin/stat /bin/pwd /bin/ls
Requires:     binutils
Version:      0
Release:      0
Source0:      lilo-%{version}.tar.bz2
Source1:      http://penguinppc.org/projects/yaboot/yaboot-%{yaboot_vers}.tar.gz
Patch10:       yaboot-%{yaboot_vers}.patch

# $Id$
%description
lilo for ppc

%prep
%setup -q -T -c -a 0 -a 1
mv lilo-%{version} lilo.ppc
mv yaboot-%{yaboot_vers} yaboot
cd yaboot
%patch10 -p1
cd ..

%build
cd yaboot
#
make clean
make DEBUG=0 VERSION=%{yaboot_vers}.FAT.SuSE YABOOT_FAT=1 yaboot HOSTCFLAGS="$RPM_OPT_FLAGS -g"
mv second/yaboot.chrp yaboot.fat
#
make clean
make DEBUG=1 VERSION=%{yaboot_vers}.SuSE yaboot HOSTCFLAGS="$RPM_OPT_FLAGS -g"
mv second/yaboot yaboot.debug
mv second/yaboot.chrp yaboot.chrp.debug
#
make clean
make DEBUG=0 VERSION=%{yaboot_vers}.SuSE yaboot HOSTCFLAGS="$RPM_OPT_FLAGS -g"
mv second/yaboot yaboot
mv second/yaboot.chrp yaboot.chrp
#
cd ..
#
cd lilo.ppc
cd bootheader
make HOST_CFLAGS="$RPM_OPT_FLAGS -g"

%install
# get rid of /usr/lib/rpm/brp-strip-debug 
# it kills the zImage.chrp-rs6k 
export NO_BRP_STRIP_DEBUG=true
#
rm -rfv $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/lib/lilo/pmac
mkdir -p $RPM_BUILD_ROOT/lib/lilo/chrp
mkdir -p $RPM_BUILD_ROOT/sbin
mkdir -p $RPM_BUILD_ROOT/bin
mkdir -p $RPM_BUILD_ROOT/bin
mkdir -p $RPM_BUILD_ROOT/%{_docdir}/lilo/activate
cd lilo.ppc
chmod 755 show_of_path.sh
chmod 754 lilo.{old,new}
cp -av lilo.old $RPM_BUILD_ROOT/sbin/lilo.old
cp -av lilo.new $RPM_BUILD_ROOT/sbin/lilo
cp -av lilo-pmac.lib $RPM_BUILD_ROOT/lib/lilo/lilo-pmac.lib
cp -av lilo-chrp.lib $RPM_BUILD_ROOT/lib/lilo/lilo-chrp.lib
cp -av lilo-iseries.lib $RPM_BUILD_ROOT/lib/lilo/lilo-iseries.lib
cp -av firmware_status.chrp $RPM_BUILD_ROOT/lib/lilo/chrp/
cp -av show_of_path.sh $RPM_BUILD_ROOT/bin
cp -av Finder.bin $RPM_BUILD_ROOT/lib/lilo/pmac
cp -av System.bin $RPM_BUILD_ROOT/lib/lilo/pmac
cp -av os-badge-icon $RPM_BUILD_ROOT/lib/lilo/pmac
cp -av README* $RPM_BUILD_ROOT%{_docdir}/lilo/
cp -av COPYING $RPM_BUILD_ROOT%{_docdir}/lilo/
cd bootheader
make install DESTDIR=$RPM_BUILD_ROOT
cd ..
cd ..
cd yaboot
cp -av yaboot yaboot.debug $RPM_BUILD_ROOT/lib/lilo/pmac
cp -av yaboot.chrp* yaboot.fat $RPM_BUILD_ROOT/lib/lilo/chrp
cd ..

%triggerpostun  -- lilo < 0.0.10
# for manual updates
if [ -f /etc/lilo.conf.rpmsave -a ! -f /etc/lilo.conf ] ; then
mv -v /etc/lilo.conf.rpmsave /etc/lilo.conf
fi
exit 0

%files
%defattr (-,root,root)
%dir /lib/lilo
%dir /lib/lilo/pmac
%dir /lib/lilo/prep
%dir /lib/lilo/chrp
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
%attr(644,root,root) /lib/lilo/*/*.o
%attr(644,root,root) /lib/lilo/*/*.a
%attr(644,root,root) %config /lib/lilo/*/*ld.script*
%attr(644,root,root) /lib/lilo/lilo-*.lib
%attr(644,root,root) /lib/lilo/chrp/yaboot.*
%attr(644,root,root) /lib/lilo/chrp/firmware_status.chrp
%attr(755,root,root) %config /lib/lilo/scripts/*.sh
%attr(755,root,root) /lib/lilo/utils/*
%attr(755,root,root) %config /sbin/lilo.old
%attr(755,root,root) %config /sbin/lilo

%doc %{_docdir}/lilo
