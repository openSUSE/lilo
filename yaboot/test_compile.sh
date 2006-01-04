#!/bin/bash
# $Id$
unset LANG
unset ${!LC_*}
numcpus=`grep -Ec '^cpu[0-9]' /proc/stat || echo 1`
i=0
for CONFIG_COLOR_TEXT in y n
do
	for CONFIG_FS_REISERFS in y n
	do
		for CONFIG_FS_XFS in y n
		do
			for CONFIG_SET_COLORMAP in y n
			do
				for USE_MD5_PASSWORDS in y n
				do
					for DEBUG in 0 1 2
					do
						make clean -j $numcpus &> /dev/null
						: $(( i++ ))
						echo config: \
							CONFIG_COLOR_TEXT=$CONFIG_COLOR_TEXT \
							CONFIG_COLOR_TEXT=$CONFIG_COLOR_TEXT \
							CONFIG_FS_REISERFS=$CONFIG_FS_REISERFS \
							CONFIG_FS_XFS=$CONFIG_FS_XFS \
							CONFIG_SET_COLORMAP=$CONFIG_SET_COLORMAP \
							USE_MD5_PASSWORDS=$USE_MD5_PASSWORDS \
							DEBUG=$DEBUG > log.$i.txt
						make \
							-j $numcpus \
							CONFIG_COLOR_TEXT=$CONFIG_COLOR_TEXT \
							CONFIG_COLOR_TEXT=$CONFIG_COLOR_TEXT \
							CONFIG_FS_REISERFS=$CONFIG_FS_REISERFS \
							CONFIG_FS_XFS=$CONFIG_FS_XFS \
							CONFIG_SET_COLORMAP=$CONFIG_SET_COLORMAP \
							USE_MD5_PASSWORDS=$USE_MD5_PASSWORDS \
							DEBUG=$DEBUG >> log.$i.txt  2>&1
						echo $? \
							CONFIG_COLOR_TEXT=$CONFIG_COLOR_TEXT \
							CONFIG_COLOR_TEXT=$CONFIG_COLOR_TEXT \
							CONFIG_FS_REISERFS=$CONFIG_FS_REISERFS \
							CONFIG_FS_XFS=$CONFIG_FS_XFS \
							CONFIG_SET_COLORMAP=$CONFIG_SET_COLORMAP \
							USE_MD5_PASSWORDS=$USE_MD5_PASSWORDS \
							DEBUG=$DEBUG
					done
				done
			done
		done
	done
done
grep -w Error log.*.txt || echo no errors in $i combinations
