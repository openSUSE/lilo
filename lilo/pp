--- /sbin/lilo.orig     1970-01-01 00:27:47.899924456 +0000
+++ /sbin/lilo  1970-01-01 00:30:04.354946736 +0000
@@ -373,6 +373,7 @@

        [[ "${REPLY:0:1}" == [1234D] ]] || continue
        read  minor start end type fs_n_flags <<< "$REPLY"
+       (( minor > 4 )) && continue
        case $minor in
            1|2|3|4)
                p_start[$minor]=${start/.}
