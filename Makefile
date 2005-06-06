COMMIT_DIR=/work/src/done/SLES9-SP2
BUILD_DIST=sles9-beta-ppc

.PHONY:	export build commit clean

all:
	@echo "Choose one target out of 'export', 'build', 'commit' or 'clean'"
	@echo

export:	.exportdir

build:	.built

commit:	.commited


# worker targets

.exportdir:	lilo.changes
	@rm -f .build .committed
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
	if /work/src/bin/check_if_valid_source_dir; then cd -; echo $$tmpdir > .exportdir; fi


.built:	.exportdir
	@rm -f .commited
	@echo "Trying to compile lilo package under $$(<.exportdir)"
	if { cd $$(<.exportdir); export BUILD_DIST=$(BUILD_DIST); sudo build; }; then touch .built; else echo Compile failed; exit 1; fi

.commited: .built
	@echo "Target 'commit' will copy $$(<.exportdir) to $(COMMIT_DIR)"
	@echo "Please confirm or abort"
	@select s in commit abort;do [ "$$s" == commit ] && break || exit 1; done
	echo cp -a $$(<.exportdir) $(COMMIT_DIR)
	@touch .commited

clean:
	rm -f .exportdir .build .committed
