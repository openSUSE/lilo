#
# spec file for package yaboot (Version 0.4)
# 
# Copyright  (c)  2000  SuSE GmbH  Nuernberg, Germany.
#
# please send bugfixes or comments to feedback@suse.de.
#

# neededforbuild  ext2fs_d
# usedforbuild    aaa_base aaa_dir base bash bindutil binutils bison bzip compress cpio cracklib devs diff egcs ext2fs ext2fs_d file fileutil find flex gawk gdbm gettext gpm gppshare groff gzip kbd ldso less libc libz lx_suse make mktemp modules ncurses net_tool netcfg nkita nkitb nssv1 pam patch perl pgp ps rcs rpm sendmail sh_utils shadow shlibs strace syslogd sysvinit texinfo textutil timezone unzip util vim xdevel xf86 xshared

Vendor:       SuSE GmbH, Nuernberg, Germany
Distribution: SuSE Linux 6.3 (PPC)
Name:         yaboot
Release:      0
Packager:     feedback@suse.de

Summary:      YaBoot - OF boot loader for PowerMac
Version:      0.4
Copyright: GPL
Group: Unsorted
Source0: yaboot_0.4.src.tgz
Source1: yaboot_0.4.gz

%description
Yaboot is an OpenFirmware based bootloader for newworld machines.
Netbooting works via bootp/tftp (including loading the ramdisk).
Version 0.4 fixes a problem with kernels containing more than one ELF section.

Authors:
--------
    Benjamin Herrenschmidt <bh40@calva.net>

%prep
%setup -n yaboot

%build
make

%install
cp -a $RPM_SOURCE_DIR/yaboot_0.4.gz .
%{?suse_check}

%files
%doc COPYING yaboot yaboot_0.4.gz
# our binary doesn't work ...

