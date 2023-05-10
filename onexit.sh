#!/bin/bash

# A stacked exit handler for bash.

# onexit "command" - push command to the onexit stack, to be executed in reverse order of definition
# when this script exits.
onexit() { onexit=("$*" "${onexit[@]}"); }
trap '__=$?; set +etu; eval "${onexit[@]/%/;}"; exit $__' EXIT

# popexit [N] - pop the last N commands from the onexit stack, or -N for all except for the first N.
# Default 1. An arbitrarily large N pops them all.
popexit() { onexit=("${onexit[@]: ${1:-1}}"); }

# dumpexit - dump the current onexit stack
dumpexit() { printf "%s\n" "${onexit[@]}"; }

# POC

set -u

onexit echo First in, last out

# create a temp directory
dir=$(mktemp -td onexit.poc.XXXX) || exit 1

# test if the directory exists on exit (it should not)
onexit "[[ -d $dir ]] && echo 'Oops, $dir was not removed!' || echo '$dir has been removed'"

# remove the directory on exit, this happens before the test above
onexit rm -rf $dir

# do stuff with the directory
touch $dir/xyzzy || exit 1

# show directory contents before removal
onexit "echo $dir:; ls -al $dir"

onexit A transient entry that will be popped
onexit Another...
popexit 2

onexit echo Last in, first out

onexit Another to be popped...
onexit Another...
onexit Another...
onexit Another...
onexit Another...
onexit Another...
# keep only the first 5
popexit -5

printf "These commands will run on exit:\n\n"

dumpexit

ex=$((RANDOM & 7))
printf "\nExit status will be $ex. Bye!\n\n"
exit $ex
