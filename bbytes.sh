#!/bin/bash

# Convert convert to fractional kibibytes, mebibytes, etc (powers of 2), ie 1048576 -> 2.00MiB
# If given, second arg if given is number of signficant digits after the decimal, default 2
bbytes() {
    [[ $1 =~ [^0-9] ]] && { echo $1; return; }
    local n=$((10#$1)) s=${2:-2} f="%d%0s%s\n" p=0 d=$((2**60))
    ((s)) && { f="%d.%0${s}d%s\n" p=$((2**s)); }
    ((n < 1024)) && { printf $f $n 0 ""; return; }
    for m in EiB PiB TiB GiB MiB KiB; do
        ((n >= d)) && break
        ((d /= 1024))
    done
    printf $f $((n/d)) $(((n % d)* p / d)) $m
}

# Print numeric arg, convert to fractional kilobytes, megabytes, etc (powers of 10), ie 1000000 -> 2.00MB
# If given, second arg if given is number of signficant digits after the decimal, default 2
dbytes() {
    [[ $1 =~ [^0-9] ]] && { echo $1; return; }
    local n=$((10#$1)) s=${2:-2} f="%d%0s%s\n" p=0 d=$((10**18))
    ((s)) && { f="%d.%0${s}d%s\n" p=$((2**s)); }
    ((n < 1000)) && { printf $f $n 0 ""; return; }
    for m in EB PB TB GB MB KB; do
        ((n >= d)) && break
        ((d /= 1000))
    done
    printf $f $((n/d)) $(((n%d)*(10**s)/d)) $m
}

echo "bbytes: $(bbytes $1 $2)"
echo "dbytes: $(dbytes $1 $2)"
