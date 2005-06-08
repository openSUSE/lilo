SUBMIT_DIR=/work/src/done/SLES9-SP3
BUILD_DIST=sles9-beta-ppc

.PHONY:	export build submit clean

all:
	@echo "Choose one target out of 'export', 'build', 'submit' or 'clean'"
	@echo

export:	.exportdir

build:	.built

submit:	.submitted


# worker targets

.exportdir:	lilo.changes
	@rm -f .build .submitted
	set -e ; \
	tmpdir=`mktemp -d /tmp/temp.XXXXXX`/lilo ;\
	lv=`cat version` ; \
	svn export . $$tmpdir ; \
	cd $$tmpdir ; \
	mv -v lilo lilo-$$lv ; \
	tar cfvj lilo-$$lv.tar.bz2 lilo-$$lv ; \
	rm -rf version Makefile lilo-$$lv ; \
	pwd ; \
	ls -la ; \
	if /work/src/bin/check_if_valid_source_dir; then cd -; echo $$tmpdir > $@; fi


.built:	.exportdir
	@rm -f .submitted
	@echo "Trying to compile lilo package under $$(<.exportdir)"
	if { cd $$(<.exportdir); export BUILD_DIST=$(BUILD_DIST); sudo build; }; then touch $@; else echo Compile failed; exit 1; fi

.submitted: .built
	@echo "Target 'submit' will copy $$(<.exportdir) to $(SUBMIT_DIR)"
	@echo "Please confirm or abort"
	@select s in submit abort;do [ "$$s" == submit ] && break || exit 1; done
	echo cp -a $$(<.exportdir) $(SUBMIT_DIR)
	@touch $@

clean:
	rm -f .exportdir .build .submitted
