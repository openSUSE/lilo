# $Id$
SUBDIRS = chrp common prep scripts utils man ps3
SRCROOT = .
export LANG=C
export LC_ALL=C

all:

%: local-%
	@set -e; for d in $(SUBDIRS); do $(MAKE) -C $$d $@; done

local-install:

local-all:

local-clean:

local-spotless: local-clean
	rm -f *~ tags

.PHONY: tags
tags:
	ctags `find \( -path '*/.pc' -o -path '*/.svn' \) -prune -o -name "*.[chS]"`

-include MCONFIG
