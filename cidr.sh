#!/bin/bash -u

# Given a network prefix 0-32, print netmask "0.0.0.0" to "255.255.255.255"
netmask() {
    printf "%d.%d.%d.%d" $(n=$1; for i in {1..4}; do echo $(((n<=0) ? 0 : (n>=8) ? 255 : (0xff00>>n) & 255)); ((n-=8)); done)
}

# Given a CIDR address in format w.x.y.z[/prefix], and an optional default network prefix, validate
# and print normalized CIDR or return false. If /prefix is not specified, appends the default prefix
# 0 - 32, or 32. As a special case, if the default is given as -1 then CIDR prefixes are rejected
# and only the base IP is printed.
cidr() {
    [[ $1 =~ ^([0-9]+)\.([0-9]+)\.([0-9]+)\.([0-9]+)(/([0-9]+))?$ ]] &&
        ((BASH_REMATCH[1]<=255 && BASH_REMATCH[2]<=255 && BASH_REMATCH[3]<=255 && BASH_REMATCH[4]<=255 && ${BASH_REMATCH[6]:--1} <= ((${2:-32}<0)?-1:32) )) &&
                printf "%u.%u.%u.%u%s" ${BASH_REMATCH[*]:1:4} $( ((${2:-32} >= 0)) && printf "/%u" ${BASH_REMATCH[6]:-${2:-32}} )
}

# Test with supplied args
cidr=$(cidr $1 ${2:-}) || { echo "'$1' is invalid"; exit 1; }

if ((${2:-32} >= 0)); then
    echo "cidr='$cidr' ip='${cidr%/*}' prefix='${cidr#*/}' netmask='$(netmask ${cidr#*/})'"
else
    # 0 disables the network prefix
    echo "cidr='$cidr' ip='$cidr' prefix='N/A' netmask='N/A'"
fi
