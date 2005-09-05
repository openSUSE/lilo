# norootforbuild
# neededforbuild  

Name:         lilo
%define lilo_vers  0.1.2
%define yaboot_vers 1.3.11
Group:        System/Boot
License:      BSD, Other License(s), see package
Obsoletes:    yaboot activate quik 
Requires:     hfsutils
Requires:     dosfstools
Requires:     /bin/awk /usr/bin/od /bin/sed /usr/bin/stat /bin/pwd /bin/ls
Summary:      The LInux LOader, a boot menu
Requires:     binutils
Version:      0.0.15
Release:      0
Source0:      lilo-%{lilo_vers}.tar.bz2
Source1:      http://penguinppc.org/projects/yaboot/yaboot-%{yaboot_vers}.tar.gz
Patch5:       yaboot-1.3.6.dif
Patch6:       yaboot-1.3.11-fat.dif
Patch7:       yaboot-hole_data-journal.diff
Patch8:       yaboot-1.3.11-add-ibm-rpa-note.patch
Patch20:      yaboot-1.3.11-initrd-claim-loop.patch
BuildRoot:    %{_tmppath}/%{name}-%{version}-build
# get rid of /usr/lib/rpm/brp-strip-debug 
# it kills the zImage.chrp-rs6k 
%define __os_install_post %{nil}

%description
lilo for ppc

%prep
%setup -q -T -c -a 0 -a 1
mv lilo-%{lilo_vers} lilo.ppc
mv yaboot-%{yaboot_vers} yaboot
cd yaboot
%patch5
%patch7 -p1
cp second/yaboot.c second/yaboot_fat.c
%patch6 -p1
%patch8 -p1
%patch20 -p1
cd ..
find lilo.ppc/lib -name "*.sh" | xargs -r chmod 755
find lilo.ppc/lib -name addnote | xargs -r chmod 755
find lilo.ppc/lib -name hack-coff | xargs -r chmod 755
find lilo.ppc/lib -name mkprep | xargs -r chmod 755

%build
cd yaboot
make clean
make DEBUG=1 VERSION=1.3.11.SuSE yaboot
mv second/yaboot yaboot.debug
make clean
make DEBUG=0 VERSION=1.3.11.SuSE yaboot
mv second/yaboot yaboot
make clean
make DEBUG=1 VERSION=1.3.11.SuSE yaboot.chrp
mv second/yaboot.chrp yaboot.chrp.debug
make clean
make DEBUG=0 VERSION=1.3.11.SuSE yaboot.chrp
mv second/yaboot.chrp yaboot.chrp
make clean
make DEBUG=0 VERSION=1.3.11.SuSE yaboot.fat
mv second/yaboot.fat yaboot.fat
# :-)
gcc -Wall $RPM_OPT_FLAGS -s -o ../lilo.ppc/lib/chrp/chrp64/addnote util/addnote.c
cd ..
cd lilo.ppc
gcc -Wall $RPM_OPT_FLAGS -s -o iseries-addRamDisk lilo-addRamDisk.c
gcc -Wall $RPM_OPT_FLAGS -s -o iseries-addSystemMap lilo-addSystemMap.c
gcc -Wall $RPM_OPT_FLAGS -s -o mkzimage_cmdline mkzimage_cmdline.c

%install
rm -rfv $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/lib/lilo/chrp
mkdir -p $RPM_BUILD_ROOT/lib/lilo/iseries
mkdir -p $RPM_BUILD_ROOT/lib/lilo/pmac
mkdir -p $RPM_BUILD_ROOT/sbin
mkdir -p $RPM_BUILD_ROOT/bin
mkdir -p $RPM_BUILD_ROOT/%{_docdir}/lilo/activate
cd lilo.ppc
cp -a iseries-* $RPM_BUILD_ROOT/lib/lilo/iseries
cp -a mkzimage_cmdline $RPM_BUILD_ROOT/lib/lilo/chrp
cp -a lib/* $RPM_BUILD_ROOT/lib/lilo
chmod 755 show_of_path.sh
chmod 754 lilo.{old,new}
cp -av lilo.old $RPM_BUILD_ROOT/sbin/lilo.old
cp -av lilo.new $RPM_BUILD_ROOT/sbin/lilo
cp -av lilo-pmac.lib $RPM_BUILD_ROOT/lib/lilo/lilo-pmac.lib
cp -av lilo-chrp.lib $RPM_BUILD_ROOT/lib/lilo/lilo-chrp.lib
cp -av lilo-iseries.lib $RPM_BUILD_ROOT/lib/lilo/lilo-iseries.lib
cp -av firmware_status.chrp $RPM_BUILD_ROOT/lib/lilo/chrp
cp -av show_of_path.sh $RPM_BUILD_ROOT/bin
cp -av Finder.bin $RPM_BUILD_ROOT/lib/lilo/pmac
cp -av System.bin $RPM_BUILD_ROOT/lib/lilo/pmac
cp -av os-badge-icon $RPM_BUILD_ROOT/lib/lilo/pmac
cp -av README* $RPM_BUILD_ROOT%{_docdir}/lilo/
cp -av COPYING $RPM_BUILD_ROOT%{_docdir}/lilo/
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
/lib/lilo
/sbin/lilo*
/bin/show_of_path.sh
%doc %{_docdir}/lilo

