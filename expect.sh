#!/bin/bash

{
    # This block provides expect functionality in bash, including "spawn", "send", and "expect".
    # More information in the "public functions" section below.

    # Private symbols start with "__", avoid direct reference.

    __expect=$(type -pf expect 2>/dev/null) || { echo "Requires executable 'expect'" >&2; exit 1; }
    __base64=$(type -pf base64 2>/dev/null) || { echo "Requires executable 'base64'" >&2; exit 1; }
    __enc() { $__base64 -w0; }
    __dec() { $__base64 -d; }
    __started=0 __cpout=0 __cpin=0 __debio=0
    __cpio() {
        if ((!__started)); then
            coproc {
                exec $__expect -c 'log_user 0; set stty_init {-opost -echo}
                                   proc enc {s} {binary encode base64 [encoding convertto utf-8 $s]}
                                   proc dec {s} {encoding convertfrom utf-8 [binary decode base64 $s]}
                                   while {[gets stdin line] >= 0} {if [catch {eval $line} err] {puts "1:$err"}}'
            }
            exec {__cpout}<&"${COPROC[0]}" {__cpin}>&"${COPROC[1]}"
            __started=1
        fi
        local OPTARG OPTIND to=10 out=cat s
        # -d = decode response, -t n = set timeout
        while getopts ":dt:" s; do case $s in d)out="__dec";; t)to=$OPTARG;; *);; esac; done
        # shellcheck disable=2048 # compressing whitespace
        s=$(set -f; echo ${*:$OPTIND}); ((__debio)) && echo ">>> $s" >&2
        echo "$s" >&${__cpin} || { echo "coproc write failed" >&2; exit 1; }
        read -r -t $to -u $__cpout s || { echo "coproc read failed" >&2; exit 1; }; ((__debio)) && echo "<<< $s" >&2
        case $s in 0) return 0 ;; 0:*) printf "%s" "${s:2}" | $out; return 0 ;; 1*) return 1 ;; *) echo "coproc error"; exit 1 ;; esac
    }

    # Public functions, all return true if success and false if error. These are roughly analogous to
    # expect functions of the same name.

    # spawn program [args] - start specified program in the background
    spawn() {
        local prog; readarray -t prog< <(for a; do printf "%s" "$a" | __enc; echo; done)
        __cpio "if [spawn -noecho {*}[lmap arg {${prog[*]}} {dec \$arg}]] {puts 0} {puts 1}"
    }

    # Print the current spawn id (the id of the most recently spawned program)
    spawnid() { __cpio 'puts "0:$spawn_id"'; }

    # close [options] - close the current spawnid, in theory this will cause the process to
    # exit. Then reap the child if possible. Options:
    #   -i spawnid : close the specified spawnid instead
    close() {
        local OPTARG OPTIND i=""
        while getopts ":i:" o; do case $o in i) i="-i $OPTARG";; *);; esac; done
        __cpio "catch {close $i}; catch {wait -nowait $i}; puts 0"
    }

    # send [options] text - write /text to the current spawnid. Whitespace is retained, escaped
    # characters are expanded. Options:
    #    -i spawnid : write to specified spawnid instead
    #    -s seconds : send 1 char per specified seconds (i.e. ".025")
    send() {
        local OPTARG OPTIND id="" slow="" o
        while getopts ":i:s:" o; do case $o in i)id="-i $OPTARG";; s) slow=$OPTARG;; *);; esac; done
        shift $((OPTIND-1))
        __cpio "${slow:+"set send_slow {1 $slow};"} send ${slow:+-s} $id [dec $(printf "%s" "${*@E}" | __enc)]; puts 0"
    }

    # expect [options] regex [...regex]
    # Given a list of regexes, wait for one to match the output from the current or specified
    # spawnids and print the associated regex index (0-based). Or print "timeout", or "eof" if the
    # spawned process exits. If multiple regexes could match the lowest index wins. Options:
    #   -i spawnid(s) : spawnid(s) to listen to instead of the default (can be given multiple times)
    #   -m            : match can span multiple lines
    #   -t timeout    : timeout seconds instead of 10
    # See https://www.tcl-lang.org/man/tcl/TclCmd/re_syntax.htm for more information on the tcl ARE
    # syntax.
    expect() {
        local OPTARG OPTIND cmd=() ids="" to=10 m="(?n)" o n=0
        while getopts ":i:mt:" o; do case $o in i)ids+="$OPTARG ";; m)m="";; t)to=$OPTARG;; *);; esac; done
        ((to <= 0)) && { echo "timeout"; return 1; }
        [[ $ids ]] && cmd+=("-i [list $ids]")
        shift $((OPTIND-1))
        for o; do cmd+=("-regex [dec $(printf "%s" "$m$o" | __enc)] {set result $((n++))}"); done
        __cpio -t $((to+1)) "array unset expect_out; set matches 0;
                             expect -timeout $to ${cmd[*]} timeout {set result timeout} eof {set result eof};
                             set matches [llength [lsearch -all [array names expect_out] *string]];
                             puts \"0:\$result\""
    }

    # match [n] - print nth match string from last expect, 0 for the match itself (default),
    # otherwise the regex submatch.
    # shellcheck disable=2120 # arg is optional
    match() {
        local n=${1:-0}
        __cpio -d 'if {'$n' >= $matches} {puts "1:'$n' >= $matches"} else {puts "0:[enc [string map {"\r" ""} $expect_out('$n',string)]]"}'
    }

    # Print the spawnid that produced the match or eof from the last expect.
    matchid() { __cpio 'puts "0:$expect_out(spawn_id)"'; }

    # flush [options] - discard spawn output until idle for 1 second. Options:
    #  -i spawnid(s) : spawnid(s) to flush instead of default (can be given multiple times)
    #  -t timeout    : timeout seconds instead of 1
    #  -n lines      : only flush specified number of lines
    # Return true on idle, false on eof.
    flush() {
        local OPTARG OPTIND ids="" to=1 lns=0 pat=".+" o
        while getopts ":i:n:t:" o; do case $o in i)ids+="$OPTARG ";; n)lns=$OPTARG;; t)to=$OPTARG;; *);; esac; done
        ((lns)) && pat=".*?\n"
        while true; do case $(expect -m ${ids:+-i "$ids"} -t $to $pat) in eof) return 1 ;; timeout) return 0 ;; esac; ((!--lns)) && return 0; done
    }

    # show [options]
    # Enable various forms of debug outout to stderr. Options:
    #   -a  : same as -drs, i.e. show all
    #   -d  : show tcl coprocess I/O
    #   -r  : show regex debug output
    #   -s  : show output from spawned processes (default if no other option is given)
    # shellcheck disable=2120 # args are optional
    __shown=0
    show() {
        local OPTARG OPTIND o what=0
        while getopts ":adrs" o; do case $o in a)what=7;; d)((what|=1));; r)((what|=2));; s)((what|=4));; *);; esac; done
        ((what)) || what=4; o=$((what & ~__shown)); __shown=$((__shown | what))
        ((o & 1)) && __debio=1; ((o & 2)) && __cpio "exp_internal 1; puts 0"; ((o & 4)) && __cpio "log_file -a -leaveopen stderr; puts 0"; true
    }

    # noshow [options] - Same as above, but disable. Default -a if none given.
    # shellcheck disable=2120 # args are optional
    noshow() {
        local OPTARG OPTIND o what=0
        while getopts ":adrs" o; do case $o in a)what=7;; d)((what|=1));; r)((what|=2));; s)((what|=4));; *);; esac; done
        ((what)) || what=7; o=$((what & __shown)); __shown=$((__shown & ~what))
        ((o & 1)) && __debio=0; ((o & 2)) && __cpio "exp_internal 0; puts 0"; ((o & 4)) && __cpio "log_file; puts 0"; true
    }
}

# In practice you copy the block above to your script or source it from a separate file.

# POC: telnet to http hosts in parallel, send them all GET requests and scrape the "Date:" from
# their response headers.

set -ueE
trap 'die Error line $LINENO' err
(($#)) && show "$@"

declare -A spawned # an array of [spawnid]=hostname

# first telnet to all
for h in facebook.com google.com instagram.com x.com youtube.com ; do
    if spawn telnet $h 80; then
        spawned[$(spawnid)]=$h
    else
        echo "telnet $h failed" >&2
    fi
done

# wait for each to connect and send GET request
eval "declare -A connecting=(${spawned[*]@K})" # dupe the array
while [[ ${connecting[*]} ]]; do
    r=$(expect -i "${!connecting[*]}" "Connected to") || exit 1
    case $r in
        0)  # connected
            i=$(matchid)
            send -i $i "GET / HTTP/1.0\r\r"
            unset "connecting[$i]"
            ;;
        timeout)
            for i in "${!connecting[@]}"; do
                echo "${connecting[$i]} failed to connect" >&2
                close -i $i
                unset "spawned[$i]"
            done
            break
            ;;
        eof)
            i=$(matchid)
            echo "Connection to ${connecting[$i]} dropped" >&2
            close -i $i
            unset "connecting[$i]" "spawned[$i]"
            ;;
    esac
done

# Wait for each to send "Date: xxx"
while [[ ${spawned[*]} ]]; do
    r=$(expect -i "${!spawned[*]}" "^[Dd]ate:\s+(.+?)\s*$") || exit 1
    case $r in
        0)  # matched
            i=$(matchid)
            printf "%-16s: %s\n" ${spawned[$i]} "$(match 1)"
            close -i $i
            unset "spawned[$i]"
            ;;
        timeout)
            for h in "${spawned[@]}"; do echo "No response from $h"; done
            exit 1
            ;;
        eof)
            i=$(matchid)
            echo "Connection to ${spawned[$i]} dropped" >&2
            close -i $i
            unset "spawned[$i]"
            ;;
    esac
done
noshow
