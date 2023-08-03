#!/bin/bash
# List specified files in the style of "ls -al", sort of.

ll() {
    (
        for name in "$@"; do
            [[ -e $name ]] || continue

            # stat -c options:
            # %a - access rights in octal (note '#' and '0' printf flags)
            # %A - access rights in human readable form
            # %b - number of blocks allocated (see %B)
            # %B - the size in bytes of each block reported by %b
            # %C - SELinux security context string
            # %d - device number in decimal
            # %D - device number in hex
            # %f - raw mode in hex
            # %F - file type
            # %g - group ID of owner
            # %G - group name of owner
            # %h - number of hard links
            # %i - inode number
            # %m - mount point
            # %n - file name
            # %N - quoted file name with dereference if symbolic link
            # %o - optimal I/O transfer size hint
            # %s - total size, in bytes
            # %t - major device type in hex, for character/block device special files
            # %T - minor device type in hex, for character/block device special files
            # %u - user ID of owner
            # %U - user name of owner
            # %w - time of file birth, human-readable; - if unknown
            # %W - time of file birth, seconds since Epoch; 0 if unknown
            # %x - time of last access, human-readable
            # %X - time of last access, seconds since Epoch
            # %y - time of last data modification, human-readable
            # %Y - time of last data modification, seconds since Epoch
            # %z - time of last status change, human-readable
            # %Z - time of last status change, seconds since Epoch

            read perm mode group size major minor user date time < <(stat -c "%A %f %G %s %t %T %U %.19y" $name)
            device="$((0x$major)), $((0x$minor))"

            show() { printf "%s\t%s\t%s\t%s\t%s\t%s\t%s%s\n" "$perm" "$user" "$group" "$size" "$date" "$time" "$name" "$1"; }

            case $((0x$mode >> 12)) in
                1)  show "|" ;;                  # pipe
                2)  size=$device show ;;         # char dev
                4)  show "/" ;;                  # dir
                6)  size=$device show ;;         # block dev
                8)  show ;;                      # file
                10) show " -> $(readlink $1)" ;; # symlink
                12) show "=" ;;                  # socket
            esac
        done
    ) | column -t -s $'\t'
}

ll "$@"
