all export:
	tmpdir=`mktemp -d /tmp/temp.XXXXXX`/lilo || exit 1 ; \
	lv=`cat version` ; \
	svn export . $$tmpdir ; \
	cd $$tmpdir ; \
	mv -v lilo lilo-$$lv ; \
	tar cfvj lilo-$$lv.tar.bz2 lilo-$$lv ; \
	rm -rf version Makefile lilo-$$lv ; \
	pwd ; \
	ls -la ; \
	/work/src/bin/check_if_valid_source_dir
