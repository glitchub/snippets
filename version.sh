#!/bin/bash

# Version string comparison in bash.
# Given "required" and "actual" version strings, return true if the required version is less than or equal to actual version.
# A third argument, if given, is a regex bracket expression (wthout the brackets) that defines the characters to be treated
# as field separators. Default is "_.-".
version()
{
    (($# < 2)) && return 1
    (
        shopt -s extglob
        req=($1)
        act=($2)
        sep="${3:-_.-}"
        readarray -t req < <(egrep -o "[^$sep]+" <<<"$1")
        readarray -t act < <(egrep -o "[^$sep]+" <<<"$2")
        for r in "${req[@]}"; do
            a=${act:-}
            [[ $a ]] || return 1
            if [[ $r =~ ^[0-9]+$ && $a =~ ^[0-9]+$ ]]; then
                # remove leading zeros and compare numerically
                r=${r/#+(0)/}
                a=${a/#+(0)/}
                (( r > a )) && exit 1
                (( r < a )) && exit 0
            else
                # compare lexically
                [[ $r > $a ]] && exit 1
                [[ $r < $a ]] && exit 0
            fi
            act=(${act[@]:1}) # shift act
        done
        exit 0
    )
}

# test
try() { version "$@" && r="<=" || r=">"; printf "%s %s %s %s\n" "$1" "$r" "$2" "${3+with separator \"$3\"}"; }
try 1-101-x      1-11-x
try 1-101-x      1-11-x   "X"
try 1.1.1        1.1.1.2
try 1.1.1.2      1.1.1
try 1.1.123      1.1.1.77
try 1.1.123      1.1.1.77 "X"       # Bogus separator compares the entire string
try 11.22.33.43  11.22.33.44
try 11.22.33.44  11.22.33.44
try 11.22.33.45  011.022.033.044
try 1.1a-47      1.1b
try 123          123a
try 1-2-3        1-2-3a
try 2-alpha-44   2-44
try 2-beta       2-alpha-44
try 2-beta       2-alpha-44 "-a-z"  # ignoring alpha characters
try 3.10         3.0000011
try 3.10         3.0000011 "X"
try "5[11]"      "5[100]"           # lexical
try "5[11]"      "5[100]" "]["      # numeric (note [^][] is a legal bracket expression but [^[]] is not)
try 5678         45678              # numeric
try 5678a        45678              # lexical
try "prog 1.2.a" "prog 1.2"
