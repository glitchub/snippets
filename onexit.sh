#!/bin/bash

# Stacked exit handler for bash. Commands defined with "onexit" or "onerror" are performed at exit,
# in reverse order of definition. "onerror" commands are only performed if exit status is non-zero.
# "popexit" removes the previous onexit or onerror definition. Any arbitrary command pipeline is
# allowed but must not end with ";". Normal eval quoting rules apply.
trap '__oxs=$?;trap "" int;trap - debug return err;set +ueET;eval ${__oxc[*]}' exit
onexit() { __oxc=("$*;" "${__oxc[@]:-}");}
onerror() { onexit "((__oxs))&&{ $*;}";}
popexit() { __oxc=("${__oxc[@]:1}");}
trap 'echo $0: line $LINENO: unexpected error >&2' err # report unexpected errors
set -eEu # exit on unexpected error or undefined var
trap exit int # make sure ^C trips onerror

# 'bash onexit.sh' runs the test cases, also illustrates unusual "errexit" behavior.

die() { echo "$*" >&2; exit 1; }

if ((!$#)); then
    trap - int

    # Tests that should return 0
    for ((n = 0; n < 20; n++)); do
        echo Test case $n
        bash $0 $n && xs=0 || xs=$?
        echo Exit $xs
        ((xs == 255)) && break
        ((xs)) && die "Expected exit zero"
        echo
    done
    echo

    # Tests that should not return 0
    for ((n = 100; n < 120; n++)); do
        echo Test case $n
        bash $0 $n && xs=1 || xs=$?
        echo Exit $xs
        ((xs == 255)) && break
        ((xs)) || die "Expected exit non-zero"
        echo
    done
    exit 0
fi

onexit echo First in, last out
onerror echo Exit status is non-zero
onexit echo Last in, first out

func1() { (exit 42); }
func2() { return 42; }

if (($1 < 100)); then
    case $1 in
        0) echo Normal exit ;;
        1) echo Normal arithmetic non-zero; ((x=1)) ;;
        2) echo Normal logic true; false || true ;;
        3) echo Normal logic false; false && true ;;
        4) echo Normal right-most pipeline; func2 | cat ;;
        5) echo Normal ignored error; ! ((x=0)) ;;
        *) echo The last \"normal\" case returns 255; popexit; popexit; exit 255 ;;
    esac
    # should always get here
    exit 0
fi

# shellcheck disable=2154,2034 # ignore nosuchvar and x
case $(($1)) in
    100) echo Error -e; false ;;
    101) echo Error -u; echo $nosuchvar ;;
    102) echo Error invalid command; nosuchcmd ;;
    103) echo Error function; func1 ;;
    104) echo Error function return; func2 ;;
    105) echo Error arithmetic zero; ((x=0)) ;;
    106) echo Error logic false; true && false ;;
    107) echo Error logic false; false || false ;;
    108) echo Error right-most pipeline; echo x | func2 ;;
    109) echo Error pipefail; set -o pipefail; func2 | cat ;;
    110) onerror "echo Sleeping, ^C shouldn\\'t work...; sleep 2; echo Continuing"; echo Press ^C or ^\\...; sleep 1000d ;;
      *) echo The last \"error\" case returns 255; popexit; popexit; exit 255 ;;
esac
echo "SHOULDN'T GET HERE"
exit 255
