#!/bin/bash

# expect [options] [--] regex [...regex]
# Wait for one of the specified regexes to arrive on stdin and print its 0-based index, followed by
# the matching string and any submatches, one per line, and return 0.
# At least two lines are output.
# If multiple regexes match, whichever is specified first wins.
# Return 1 on timeout or 2 on eof (with no output).
# Options:
#   -d        debug pattern matching on stderr
#   -t XXX    timeout seconds, default no timeout
expect()
{
    local timeout=-1 opt OPTARG OPTIND script debug="" args
    while getopts ":dt:v" opt; do case $opt in
        d) debug='puts stderr "expect $args"; exp_internal 1' ;;
        t) timeout=$OPTARG ;;
        *) echo "invalid option" >&2; return 1 ;;
    esac; done
    shift $((OPTIND-1))
    args=$(printf "{%s} " "$@")
    exec {script}<<EOT
        set args {}
        set i 0
        foreach arg [list $args] { lappend args -regex \$arg [list puts \$i]; incr i }
        lappend args timeout {exit 1} eof {exit 2}
        $debug
        set timeout $timeout
        expect -brace \$args
        catch { for {set n 0} {\$n < 10} {incr n} {puts [string trim \$expect_out(\$n,string)]} }
EOT
    command expect /dev/fd/$script
    exec {script}<&-
}

# POC
# Things to match
regexes=(
    "X|x"       # any x
    "z z"       # "z z"
    "something"
    "AA(.*)"    # anything starting with "AA"
    ".+"        # anything else
)
while true; do
    echo "Type something without an 'x': "
    readarray -t resp < <(expect "${regexes[@]}")
    if ((!${#resp[@]})); then
        echo "Too slow!"
        read -n1000 -t.001 # flush partial stdin
        exit 1
    fi
    case ${resp[0]} in
        0) echo "*Without* an x?" ;;
        1) echo "Bzz zt!" ;;
        2) echo "Very funny." ;;
        3) echo "You screamed '${resp[2]}'!"; break; ;;
        4) echo "You said '${resp[1]}'."; break ;;
        *) echo "Shouldn't get here"; exit 1 ;;
    esac
done
