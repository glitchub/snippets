#!/bin/bash

# Stacked exit handler for bash.
trap '__=$?; set +$-; eval $onexit; exit $__' EXIT

# Define a command to be performed on exit. They are executed in reverse order of definition.
onexit() { onexit="$*; ${onexit:-}"; }

# As above but only performed if exit status is non-zero.
onerror() { onexit="((\$__)) && { $*; }; ${onexit:-}"; }

# POC, test with 'bash onexit.sh; echo $?'

set -u

onexit echo First in, last out

onerror 'echo This only appears because the exit status is $__; echo And also this!'

# create a temp directory
dir=$(mktemp -td onexit.poc.XXXX) || exit 1

# Various exit commands, handled in reverse order! Quoting is only required if the command contains
# shell meta-characters, or if you want to defer variable expansion until exit.
onexit "[[ -d $dir ]] && echo 'Oops, $dir was not removed!' || echo '$dir has been removed'"
onexit rm -rf $dir
onexit "echo $dir:; ls -al $dir"
onexit cat $dir/tmpout
onexit "echo This was redirected at exit >$dir/tmpout"

# do stuff with the directory
touch $dir/xyzzy || exit 1

onexit echo Last in, first out

ex=$((RANDOM & 3))
printf "\nExiting with status $ex. Bye!\n\n"
exit $ex
