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
        case $s in 0) return 0 ;; 0:*) $out <<< "${s:2}"; return 0 ;; 1*) return 1 ;; *) echo "coproc error"; exit 1 ;; esac
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
        local OPTARG OPTIND i=""; while getopts ":i:" o; do case $o in i) i="-i $OPTARG";; *);; esac; done
        __cpio "catch {close $i}; catch {wait -nowait $i}; puts 0"
    }

    # send [options] text - write /text to the current spawnid. Whitespace is retained, escaped
    # characters are expanded. Options:
    #    -i spawnid : write to specified spawnid instead
    send() {
        local OPTARG OPTIND i="" s; while getopts ":i:" o; do case $o in i) i="-i $OPTARG";; *);; esac; done
        s="${*:$OPTIND}"; __cpio "send $i [dec $(printf "%s" "${s@E}" | __enc)]; puts 0"
    }

    # expect [options] regex [...regex]
    # Given a list of regexes, wait for one to match the output from the current or specified
    # spawnids and print the associated regex index (0-based). Or print "timeout", or "eof" if the
    # spawned process exits.  If multiple regexes could match on the same character the lowest index
    # wins. Options:
    #   -g         : prevent ^, $, and . from matching "\n" (like grep)
    #   -i spawnid : spawnid(s) to listen to, instead of the default (as a quoted string if multiple)
    #   -t timeout : timeout seconds instead of 10
    # See https://www.tcl-lang.org/man/tcl/TclCmd/re_syntax.htm for more information on the tcl ARE
    # syntax.
    expect() {
        local OPTARG OPTIND cmd=() to="" n=0 p="" s
        while getopts ":gi:t:" s; do case $s in g)p="(?n)";; i)cmd=("-i [list $OPTARG]");; t)to=$OPTARG;; *);; esac; done
        shift $((OPTIND-1)); for s; do cmd+=("-regex [dec $(printf "%s" "$p$s" | __enc)] {set result $((n++))}"); done
        [[ $to =~ ^[1-9][0-9]*$ ]] || to=10 # valid or 10
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
        __cpio -d "if {$n >= \$matches} {puts \"1:$n >= \$matches\"}
                   {puts \"0:[enc [string map {\"\r\n\" \"\n\" \"\n\r\" \"\n\" \"\r\" \"\n\"} \$expect_out($n,string)]]\"}"
    }

    # Print the spawnid that produced the match or eof from the last expect.
    matchid() { __cpio 'puts "0:$expect_out(spawn_id)"'; }

    # 1 = show regex processing info on stderr, 0 = turn it off
    debre() { __cpio "exp_internal ${1:-1}; puts 0"; }

    # 1  = show spawned process i/o on stderr, 0 = turn it off
    debsp() { local s="log_file"; ((${1:-1})) && s+=";log_file -a -leaveopen stderr"; __cpio "$s; puts 0"; }
}

# In practice you copy the block above to your script or source it from a separate file.

# POC: telnet to http hosts in parallel, send them all GET requests and scrape the "Date:" from
# their response headers.

set -ueE

declare -A spawned # an array of [spawnid]=hostname

# first telnet to all
for h in facebook.com google.com instagram.com x.com youtube.com ; do
    if spawn telnet $h 80; then
        spawned[$(spawnid)]=$h
    else
        echo  "telnet $h failed" >&2
    fi
done

# wait for each to connect and send GET request
eval "declare -A connecting=(${spawned[*]@K})" # dupe the array
while [[ ${connecting[*]} ]]; do
    r=$(expect -i "${!connecting[*]}" "Connected to") || exit 1
    case $r in
        0)  # connected
            m=$(matchid)
            send -i $m "GET / HTTP/1.0\r\r"
            unset "connecting[$m]"
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
            m=$(matchid)
            echo "Connection to ${connecting[$m]} dropped" >&2
            close -i $m
            unset "connecting[$m]" "spawned[$m]"
            ;;
    esac
done

# Wait for each to send "Date: xxx"
while [[ ${spawned[*]} ]]; do
    r=$(expect -g -i "${!spawned[*]}" "^[Dd]ate:\s+(.+)") || exit 1
    case $r in
        0)  # matched
            m=$(matchid)
            printf "%-16s: %s\n" ${spawned[$m]} "$(match 1)"
            close -i $m
            unset "spawned[$m]"
            ;;
        timeout)
            for h in "${spawned[@]}"; do echo "No response from $h"; done
            exit 1
            ;;
        eof)
            m=$(matchid)
            echo "Connection to ${spawned[$m]} dropped" >&2
            close -i $m
            unset "spawned[$m]"
            ;;
    esac
done
