#!/bin/bash

# Stacked exit handler for bash. Commands defined with "onexit" or "onerror" are performed at exit,
# in reverse order of definition. "onerror" commands are only performed if exit status is non-zero.
# "popexit [N]" removes the last N onexit or onerror definitions (default 1). Any arbitrary command
# pipeline is allowed but must not end with ";". Normal eval quoting rules apply.
trap '__oxs=$?;trap - debug return err;set +ueET;eval ${__oxc[*]}' EXIT
onexit() { __oxc=("$*;" "${__oxc[@]}");}
onerror() { onexit "((__oxs))&&{ $*;}";}
popexit() { __oxc=("${__oxc[@]:${1:-1}}");}

trap "echo Abort; exit 130" int quit # make sure ^C and ^\ go through the exit handlers

# The rest of this file is a test, run it with 'bash onexit.sh; echo $?'.

set -ueE

trap 'echo ERROR on line $LINENO' err

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

onexit 'echo "Last in, first out (if exit code 5)"'

onexit echo This only appears because exit code is not 5
onexit echo This only appears because the exit code is not 4 or 5

(($#)) && ex=$1 || ex=$((RANDOM % 7))

# shellcheck disable=SC2154
case $ex in
    0) printf "\n0 ---- Normal exit\n\n"; exit 0 ;;
    1) printf "\n1 ---- Abort due to -e\n\n"; false ;;
    2) printf "\n2 ---- Abort due to -u\n\n"; echo $nosuchvar ;;
    3) printf "\n3 ---- Press ^C or ^\...\n\n"; sleep 1000d ;;
    4) printf "\n4 ---- Pop last message\n\n"; popexit 1 ; exit $ex ;;
    5) printf "\n5 ---- Pop last 2 messages\n\n"; popexit 2 ; exit $ex ;;
    6) printf "\n6 ---- List handlers:\n"; printf ">  %s\n" "${__oxc[@]}"; printf "\n"; exit $ex ;;
    *) printf "\n* ---- Error exit $ex\n\n"; exit $ex ;;
esac
echo "SHOULDN'T GET HERE"
exit 99
