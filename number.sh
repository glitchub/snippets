#!/bin/bash

# Given a numeric string, possibly in form "base#N" or "0xN", if valid echo the decimal value and
# return true, or echo $2 and return false. Does not choke on "value too great for base".

number() { local n=$1; [[ $n == ?*#?* || $n == 0x?* ]] || n="10#$n"; read n < <( ( echo $((n)) ) 2>/dev/null ) && echo $n || { echo ${2:-}; false; }; }

# Test with $1 and $2 from the command line
n=$(number $*) && echo "Number is $n" || echo "Number is invalid (returned default '$n')"
