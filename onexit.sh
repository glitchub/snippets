#!/bin/bash

# Stacked exit handler for bash. Commands defined with "onexit" or "onerror" are performed at exit,
# in reverse order of definition. "onerror" commands are only performed if exit status is non-zero.
# Any arbitrary command pipeline is allowed but must not end with ";". Normal eval quoting rules
# apply.
trap '__exs=$?;trap debug;trap return;trap err;set +ueET;eval $__oxc' EXIT
onexit() { __oxc="$*;${__oxc:-}";}
onerror() { onexit "((__exs))&&{ $*;}";}

# The rest of this file is a test, run it with 'bash onexit.sh; echo $?'.

set -ueE

trap "echo ERROR on line $LINENO" err

onexit echo First in, last out.
onexit 'false; echo FALSE=$?'

onerror 'echo This only appears because the exit status is non-zero; echo And also this!'
onexit 'echo Flag=$-; nosuchcommand; test $nosuchvar; echo Ignore the errors above!'

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

(($#)) && ex=$1 || ex=$((RANDOM % 4))

# shellcheck disable=SC2154
case $ex in
    0) printf "\n0 ---- Normal exit\n\n"; exit 0 ;;
    1) printf "\n1 ---- Abort due to -e\n\n"; false ;;
    2) printf "\n2 ---- Abort due to -u\n\n"; echo $nosuchvar ;;
    *) printf "\n* ---- Error exit $ex\n\n"; exit $ex ;;
esac
echo "SHOULDN'T GET HERE"
exit 99
