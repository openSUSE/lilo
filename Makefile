# $Id$
SUBMIT_DIR=/work/src/done/STABLE
BUILD=/work/src/bin/build
BUILD_DIST=ppc
BUILD_ROOT=/abuild/buildsystem.$$HOST.$$LOGNAME
BUILD_DIR=$(BUILD_ROOT)/usr/src/packages/RPMS


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

.exportdir:	lilo.changes
	@rm -f .built .submitted
	set -e ; \
	export LANG=C ; export LC_ALL=C ; export TZ=UTC ; \
	tmpdir=`mktemp -d /tmp/temp.XXXXXX`/lilo ; \
	lv=`cat version` ; \
	yv=`cat yabootversion` ; \
	svn export . $$tmpdir ; \
	svn log -v yaboot > $$tmpdir/yaboot/Changelog.SuSE ; \
	cd $$tmpdir ; \
	chmod -R a+rX .. ; \
	tar xfz yaboot-$$yv.tar.gz ; \
	diff -purN yaboot-$$yv yaboot > yaboot-$$yv.patch || : ; \
	mv -v lilo lilo-$$lv ; \
	tar cfvj lilo-$$lv.tar.bz2 lilo-$$lv ; \
	mv lilo.spec lilo.spec.in ; \
	sed "s/^Version:.*/Version: $$lv/" < lilo.spec.in > lilo.spec ; \
	mv lilo.spec lilo.spec.in ; \
	sed "s/^%define yaboot_vers.*/%define yaboot_vers $$yv/" < lilo.spec.in > lilo.spec ; \
	rm -rf version Makefile lilo-$$lv lilo.spec.in \
	yaboot-$$yv yaboot yabootversion ; \
	pwd ; \
	ls -la ; \
	if /work/src/bin/check_if_valid_source_dir; then cd -; echo $$tmpdir > $@; fi


.built:	.exportdir
	@rm -f .submitted
	@echo "Trying to compile lilo package under $$(<.exportdir)"
	if { cd $$(<.exportdir); export BUILD_DIST=$(BUILD_DIST) BUILD_ROOT=$(BUILD_ROOT); sudo $(BUILD); }; then touch $@; else echo Compile failed; exit 1; fi

.submitted: .built
	@echo "Target 'submit' will copy $$(<.exportdir) to $(SUBMIT_DIR)"
	@echo "Please confirm or abort"
	@select s in submit abort;do [ "$$s" == submit ] && break || exit 1; done
	cp -av $$(<.exportdir) $(SUBMIT_DIR)
	@touch $@

clean:
	rm -f .exportdir .built .submitted
