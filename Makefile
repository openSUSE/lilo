all export:
	tmpdir=`mktemp -d /tmp/temp.XXXXXX`/lilo || exit 1;bv=`cat bversion`;lv=`cat version`;svn export . $$tmpdir;cd $$tmpdir;mv -v lilo lilo-$$lv;tar cfvj lilo-$$lv.tar.bz2 lilo-$$lv;mv -v boot-header boot-header-$$bv;tar cfvj boot-header-$$bv.tar.bz2 boot-header-$$bv;rm -rf bversion version Makefile boot-header-$$bv lilo-$$lv;pwd;ls -la;/work/src/bin/check_if_valid_source_dir
