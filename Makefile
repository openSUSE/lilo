# $Id$
SUBMIT_DIR=/work/src/done/PPC
SUBMIT_DIR2=/work/src/done/SLES10
BUILD=/work/src/bin/build
BUILD_DIST=ppc
BUILD_ROOT=/abuild/buildsystem.$$HOST.$$LOGNAME
BUILD_DIR=$(BUILD_ROOT)/usr/src/packages/RPMS
PKG=lilo
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

.exportdir:	lilo.changes version Makefile
	@rm -f .built .submitted
	set -e ; \
	export LANG=C ; export LC_ALL=C ; export TZ=UTC ; \
	if test "$(T)" = "" ; then \
	tmpdir=`mktemp --directory --tmpdir=/dev/shm`/lilo ; mkdir $$tmpdir ; \
	else tmpdir="$(T)" ; rm -rf "$$tmpdir" ; mkdir "$$tmpdir" ; tmpdir="$$tmpdir/lilo" ; \
	fi  ; \
	lv=`cat version` ; \
	yv=$$lv-` git log --pretty=format:%cd.%h%n -n1 --date=format:%y%m%d HEAD -- yaboot ` ; \
	yt=` git log --pretty=format:%cI%n --max-count=1 HEAD -- yaboot ` ; \
	git archive --format=tar --prefix= HEAD | ( cd $$tmpdir ; tar xfa - ) ; \
	git log HEAD -- yaboot > $$tmpdir/yaboot/Changelog.SuSE ; \
	touch -d "$$yt" $$tmpdir/yaboot/Changelog.SuSE ; \
	pushd $$tmpdir ; \
	chmod -R a+rX . ; \
	rm -f yaboot*.tar.gz ; \
	mv -f yaboot/Makefile yaboot/Makefile~ ; \
	sed "/^VERSION/s@^.*@VERSION = $$yv@" yaboot/Makefile~ > yaboot/Makefile ; \
	touch -r yaboot/Makefile~  yaboot/Makefile ; \
	rm -f yaboot/Makefile~ ; \
	mv -v yaboot yaboot-$$yv ; \
	tar cfa yaboot-$$yv.tar.xz `find yaboot-$$yv -type f -o -type l | sort -u`; \
	touch -d "$$yt"  yaboot-$$yv.tar.xz ; \
	touch --reference=lilo/lilo.new lilo.spec ; \
	sed -i "s:@VERSION@:$$lv:" lilo/lilo.new ; \
	touch --reference=lilo.spec lilo/lilo.new ; \
	mv -v lilo lilo-ppc-$$lv ; \
	mv -v x86/lilo-$$lv.tar.gz x86/*.patch . ; \
	tar cfa lilo-ppc-$$lv.tar.xz lilo-ppc-$$lv ; \
	sed -i "s/^Version:.*/Version:        $$lv/" lilo.spec ; \
	sed -i "s/^%define[[:blank:]]\+yaboot_vers.*/%define yaboot_vers $$yv/" lilo.spec ; \
	rm -rf version Makefile lilo-ppc-$$lv lilo.spec.in x86 \
	yaboot-$$yv ; \
	pwd ; \
	ls -la ; \
	if /work/src/bin/check_if_valid_source_dir; then popd ; echo $$tmpdir > $@; else exit 1 ; fi

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
	@touch $@

clean:
	rm -f .exportdir .built .submitted

