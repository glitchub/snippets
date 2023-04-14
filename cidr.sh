#!/bin/bash

# Given a network prefix 0-32, print netmask "0.0.0.0" to "255.255.255.255"
netmask() {
    printf "%d.%d.%d.%d" $(n=$1; for i in {1..4}; do echo $(((n<=0) ? 0 : (n>=8) ? 255 : (0xff00>>n) & 255)); ((n-=8)); done)
}

# Given a CIDR address string, validate and normalize, or return false if invalid
cidr() {
    [[ $1 =~ ^([0-9]+)\.([0-9]+)\.([0-9]+).([0-9]+)(/([0-9]+))?$ ]] &&
        ((BASH_REMATCH[1]<=255 && BASH_REMATCH[2]<=255 && BASH_REMATCH[3]<=255 && BASH_REMATCH[4]<=255 && BASH_REMATCH[6]<=32)) ||
            return 1
    printf "%d.%d.%d.%d/%d" ${BASH_REMATCH[*]:1:4} ${BASH_REMATCH[6]:-32}
}

# Test with supplied arg
cidr=$(cidr $1) || { echo "Invalid CIDR '$1'"; exit 1; }
echo "cidr='$cidr' ip='${cidr%/*}' prefix='${cidr#*/}' netmask='$(netmask ${cidr#*/})'"
