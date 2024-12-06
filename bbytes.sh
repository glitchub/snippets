#!/bin/bash

# Print number as fractional kibibytes, mebibytes, etc (powers of 2), ie 1048576 -> "2.00 MiB"
# If given, second arg if given is number of signficant digits after the decimal, default 2
bbytes() {
    [[ $1 =~ [^0-9] ]] && { echo "$1"; return; }
    local n=$((10#$1)) s=${2:-2}
    ((n < 1024)) && { echo "$n bytes"; return; }
    local d=$((2**50))
    for m in PiB TiB GiB MiB KiB; do
        ((n >= d)) && break
        ((d /= 1024))
    done
    # s is precision, 0 = no decimal
    ((s)) && printf "%d.%0*d %s\n" "$((n/d))" "$s" "$(((n%d)*(10**s)/d))" "$m" || printf "%d %s\n" "$((n/d))" "$m";
}

# Print number as fractional kilobytes, megabytes, etc (powers of 10), ie 1000000 -> "2.00 MB"
# If given, second arg if given is number of signficant digits after the decimal, default 2
dbytes() {
    [[ $1 =~ [^0-9] ]] && { echo "$1"; return; }
    local n=$((10#$1)) s=${2:-2}
    ((n < 1000)) && { echo "$n bytes"; return; }
    local d=$((10**15))
    for m in PB TB GB MB KB; do
        ((n >= d)) && break
        ((d /= 1000))
    done
    # s is precision, 0 = no decimal
    ((s)) && printf "%d.%0*d %s\n" $((n/d)) $s $(((n%d)*(10**s)/d)) $m || printf "%d %s\n" $((n/d)) $m;
}

echo "bbytes: $(bbytes $1 $2)"
echo "dbytes: $(dbytes $1 $2)"
