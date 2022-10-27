#!/bin/bash

# Given "join" and some number of strings, print the strings separated by "join".
# Whitespace is retained.
join() { printf "%s" "${2-}"; (($#>2)) && printf -- "${1//%/%%}%s" "${@:3}"; }

# test with command line args
join "$@"
echo
