# $Id$
SUBMIT_DIR=/work/src/done/PPC
SUBMIT_DIR2=/work/src/done/SLES10
BUILD=/work/src/bin/build
BUILD_DIST=ppc
BUILD_ROOT=/abuild/buildsystem.$$HOST.$$LOGNAME
BUILD_DIR=$(BUILD_ROOT)/usr/src/packages/RPMS
PKG=lilo
D=
T=

.PHONY:	export build submit rpm clean

all:
	@echo "Choose one target out of 'export', 'build', 'submit' or 'clean'"
	@echo

export:	.exportdir

build:	.built

rpm:	.built
	@cp -av $(BUILD_ROOT)/usr/src/packages/RPMS/ppc/lilo* .
	
submit:	.submitted


# worker targets

.exportdir:	lilo.changes version
	@rm -f .built .submitted
	set -e ; \
	export LANG=C ; export LC_ALL=C ; export TZ=UTC ; \
	if test "$(T)" = "" ; then \
	tmpdir=`mktemp -d /tmp/temp.XXXXXX`/lilo ; \
	else tmpdir="$(T)" ; rm -rf "$$tmpdir" ; mkdir "$$tmpdir" ; tmpdir="$$tmpdir/lilo" ; \
	fi  ; \
	lv=`cat version` ; \
	yv=$$lv-r`svn info yaboot | sed -n "/^Last Changed Rev:[[:blank:]]\+/s@^[^:]\+:[[:blank:]]\+@@p"` ; \
	yt=`svn info yaboot | sed -n "/^Last Changed Date:[[:blank:]]\+/{s@^[^:]\+:[[:blank:]]\+@@;s@[[:blank:]]\+(.*)@@p}"` ; \
	svn export . $$tmpdir ; \
	svn log -v yaboot > $$tmpdir/yaboot/Changelog.SuSE ; \
	touch -d "$$yt" $$tmpdir/yaboot/Changelog.SuSE ; \
	cd $$tmpdir ; \
	chmod -R a+rX .. ; \
	rm -f yaboot*.tar.gz ; \
	mv -f yaboot/Makefile yaboot/Makefile~ ; \
	sed "/^VERSION/s@^.*@VERSION = $$yv@" yaboot/Makefile~ > yaboot/Makefile ; \
	touch -r yaboot/Makefile~  yaboot/Makefile ; \
	rm -f yaboot/Makefile~ ; \
	mv -v yaboot yaboot-$$yv ; \
	tar cfj yaboot-$$yv.tar.bz2 `find yaboot-$$yv -type f -o -type l | sort -u`; \
	touch -d "$$yt"  yaboot-$$yv.tar.bz2 ; \
	touch --reference=lilo/lilo.new lilo.spec ; \
	sed -i "s:@VERSION@:$$lv:" lilo/lilo.new ; \
	touch --reference=lilo.spec lilo/lilo.new ; \
	mv -v lilo lilo-ppc-$$lv ; \
	mv -v x86/lilo-$$lv.src.tar.bz2 x86/*.patch . ; \
	tar cfj lilo-ppc-$$lv.tar.bz2 lilo-ppc-$$lv ; \
	sed -i "s/^Version:.*/Version: $$lv/" lilo.spec ; \
	sed -i "s/^%define yaboot_vers.*/%define yaboot_vers $$yv/" lilo.spec ; \
	rm -rf version Makefile lilo-ppc-$$lv lilo.spec.in x86 \
	yaboot-$$yv ; \
	if test "$(D)" != "" ; then \
	echo '#!/bin/bash' > get_release_number.sh ; \
	echo 'env -i - date -u +%Y%m%d%H%M' >> get_release_number.sh ; \
	fi ; \
	pwd ; \
	ls -la ; \
	if /work/src/bin/check_if_valid_source_dir; then cd -; echo $$tmpdir > $@; else exit 1 ; fi

.built:	.exportdir
	@rm -f .submitted
	@echo "Trying to compile lilo package under $$(<.exportdir)"
	mypwd=`pwd` ; if { cd $$(<.exportdir); export BUILD_DIST=$(BUILD_DIST) BUILD_ROOT=$(BUILD_ROOT); sudo $(BUILD); }; then touch $${mypwd}/$@; else echo Compile failed; exit 1; fi

.submitted: .built
	@echo "Target 'submit' will copy $$(<.exportdir) to $(SUBMIT_DIR)"
	@echo "Please confirm or abort"
	@select s in submit abort;do [ "$$s" == submit ] && break || exit 1; done
	cp -av $$(<.exportdir) $(SUBMIT_DIR)
	@cd $(SUBMIT_DIR)/$(PKG); distmail
ifneq ($(SUBMIT_DIR2),)
	cp -av $$(<.exportdir) $(SUBMIT_DIR2)
	@cd $(SUBMIT_DIR2)/$(PKG); distmail
endif  
	@touch $@

clean:
	rm -f .exportdir .built .submitted

