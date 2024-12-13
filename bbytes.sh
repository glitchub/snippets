#!/bin/bash

# Print number as fractional kibibytes, mebibytes, etc (powers of 2) and return 0
# EG 1048576 -> "1.00 MiB"
# Optional second arg is number of digits after the decimal, default 2
# Return 1 if arguments are invalid.
bbytes() {
    local num=$1 sig=${2:-2} div=$((2**50)) suf
    [[ $num =~ ^(0|[1-9][0-9]*)$ && $sig =~ ^(0|[1-9][0-9]*)$ ]] || return 1
    ((num < 1024)) && { echo "$num bytes"; return 0; }
    for suf in PiB TiB GiB MiB KiB; do
        ((num >= div)) && break
        ((div /= 1024))
    done
    ((sig)) && printf "%d.%0*d %s\n" "$((num/div))" "$sig" "$(((num%div)*(10**sig)/div))" "$suf" || printf "%d %s\n" "$((num/div))" "$suf"
    return 0
}

# Print number as fractional kilobytes, megabytes, etc (powers of 10) and return 0
# EG 1000000 -> "1.00 MB"
# Optional second arg is number of digits after the decimal, default 2
# Return 1 if arguments are invalid.
dbytes() {
    local num=$1 sig=${2:-2} div=$((10**15)) suf
    [[ $num =~ ^(0|[1-9][0-9]*)$ && $sig =~ ^(0|[1-9][0-9]*)$ ]] || return 1
    ((num < 1000)) && { echo "$num bytes"; return 0; }
    for suf in PB TB GB MB KB; do
        ((num >= div)) && break
        ((div /= 1000))
    done
    ((sig)) && printf "%d.%0*d %s\n" "$((num/div))" "$sig" "$(((num%div)*(10**sig)/div))" "$suf" || printf "%d %s\n" "$((num/div))" "$suf"
    return 0
}

b=$(bbytes "$@") || { echo "Invalid"; exit; }
echo "bbytes: $b"
echo "dbytes: $(dbytes "$@")"
