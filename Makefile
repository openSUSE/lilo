# $Id$
all export:
	set -e ; \
	tmpdir=`mktemp -d /tmp/temp.XXXXXX`/lilo ; \
	lv=`cat version` ; \
	yv=`cat yabootversion` ; \
	svn export . $$tmpdir ; \
	cd $$tmpdir ; \
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
	/work/src/bin/check_if_valid_source_dir
