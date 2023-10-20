#!/bin/bash

# Stacked exit handler for bash. Commands defined with 'onexit' or 'onerror' are performed at exit,
# in reverse order of definition. 'onerror' commands are only performed if exit status is non-zero.
# NOTE: single quoted command strings are expanded during exit, unquoted or double quoted command
# strings are expanded at the time of definition. Strings must not end with ';'!
trap '__=$?; set +ue; eval $onexit; exit $__' EXIT
onexit() { onexit="$*; ${onexit:-}"; }
onerror() { onexit="((\$__)) && { $*; }; ${onexit:-}"; }

# The rest of this file is a test, run it with 'bash onexit.sh; echo $?'.

set -ue

onexit echo First in, last out.
onerror 'echo This only appears because the exit status is $__; echo And also this!'
onexit 'echo Flag=$-; nosuchcommand; test $nosuchvar; echo Ignore the above!'

# create a temp directory
dir=$(mktemp -td onexit.poc.XXXX) || exit 1

# Various exit commands, handled in reverse order!
onexit "[[ -d $dir ]] && echo 'Oops, $dir was not removed!' || echo '$dir has been removed'"
onexit rm -rf $dir
onexit "echo Temp directory $dir:; ls -al $dir"
onexit cat $dir/tmpout
onexit "echo This was redirected at exit >$dir/tmpout"

# do stuff with the directory
touch $dir/xyzzy || exit 1

onexit echo Last in, first out

ex=$((RANDOM % 5))
case $ex in
    0) printf "\n---- Normal exit\n\n"; exit 0 ;;
    2) printf "\n---- Abort due to -e\n\n"; false ;;
    3) printf "\n---- Abort due to -u\n\n"; echo $nosuchvar ;;
    *) printf "\n---- Error exit $ex\n\n"; exit $ex ;;
esac
echo "SHOULDN'T GET HERE"
exit 99
