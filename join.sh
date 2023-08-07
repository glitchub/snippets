#!/bin/bash

# Given "join" and some number of strings, print the strings separated by "join".
join() { local a=("${@:3}"); printf %s "$2" "${a[@]/#/$1}"; }

# test with command line args
join "$@"
