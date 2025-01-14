#!/bin/bash

# Expect functionality in bash. Running this script executes example code at the bottom.
# In practice, this entire block is copied to your script or sourced from a separate file.

{
    # This block provides functions "spawn", "send", "expect", "close" which are roughly analogous
    # to expect functions of the same name, as well as some supporting functions which are not. More
    # information in the comments below.

    # Private symbols start with "__", avoid direct reference.

    __expect=$(type -pf expect 2>/dev/null) || { echo "Requires executable 'expect'" >&2; exit 1; }

    # start expect coprocess
    __started=0
    __start() {
        if ((!__started)); then
            local cp exp; coproc cp {
                exec {exp}<<<"
                    log_user 0; set stty_init {-opost -echo}
                    proc enc {s} {binary encode base64 [encoding convertto utf-8 \$s]}; proc dec {s} {encoding convertfrom utf-8 [binary decode base64 \$s]}
                    while {[gets stdin line] >= 0} {if [catch {eval \$line} err] {puts \"2 \$err\"}}
                "
                exec $__expect -f /dev/fd/$exp
            }
            exec {__cpout}<&"${cp[0]}" {__cpin}>&"${cp[1]}"; __started=1
        fi
    }

    # Send stdin to the coprocess and get response to __reply. Dies if expect coprocess does.
    __reply=""
    __debio=0
    __cpio() {
        local to=${1:-10} in rs
        readarray -t in
        ((__debio)) && echo ">>> ${in[*]}" >&2
        echo "${in[*]}" >&${__cpin} || { echo "coproc write failed" >&2; exit 1; }
        IFS= read -r -t $to rs  <&${__cpout} || { echo "coproc read failed" >&2; exit 1; }
        ((__debio)) && echo "<<< $rs" >&2
        __reply="${rs#* }"
        return ${rs%% *}
    }

    __enc () { $__expect -c "
            fconfigure stdin -translation binary
            puts -nonewline [binary encode base64 [encoding convertto utf-8 [read -nonewline stdin]]]
            flush stdout"
    }
    __dec () { $__expect -c "
            fconfigure stdout -translation binary
            puts -nonewline [encoding convertfrom utf-8 [binary decode base64 [read -nonewline stdin]]]
            flush stdout"
    }

    # "Public" functions, all return true if success and false if error.

    # spawn program [args] - start specified program in the background
    spawn() {
        __start
        readarray -t args < <(for a; do printf "%s" "$a" | __enc; echo; done)
        __cpio <<< "if [spawn -noecho {*}[lmap arg {${args[*]}} {dec \$arg}]] {puts 0} {puts 1}"
    }

    # Print the current spawn id (i.e. the id of the most recently spawned program). This is a
    # string which can be passed to other functions.
    spawnid() { __cpio <<< 'puts "0 $spawn_id"' && echo $__reply; }

    # close [spawnid] - close the current or specified spawnid, which in theory will cause that process to
    # exit. Reap child if possible.
    # shellcheck disable=2120 # arg is optional
    close() { __cpio <<< "catch {close ${1:+-i $1}}; wait -nowait ${1:+-i $1}; puts 0"; }

    # send [spawnid] < text - write stdin to the current or specified spawnid
    # shellcheck disable=2120 # arg is optional
    send() { __cpio <<< "send ${1:+-i $1} [dec $(printf "%b" "$(cat)" | __enc)]; puts 0"; }

    # expect [timeout] [spawnid [ ...spawnid] --] regex [...regex]
    # Given optional timeout in seconds, optional list of spawnids followed by --, and a list of
    # regexes, wait for one of the regexes to match the output from the current or specified
    # spawnids and print the associated regex index (0-based). Or print "timeout", or "eof" if
    # a spawned process exits. If multiple regexes could match on the same character the lowest regex
    # wins. Timeout is set from the first arg if numeric, otherwise defaults to 10. Regexes are made
    # line-sensitive by default, unless they start with an ARE embedded option string. See
    # https://www.tcl-lang.org/man/tcl/TclCmd/re_syntax.htm for more information on the ARE syntax.
    expect() {
        local args=() cmd=() n=0 a to=10
        [[ $1 =~ ^[1-9][0-9]*$ ]] && { to=$1; shift; }
        for a; do
            if ((!n)) && [[ $a == -- ]]; then cmd=("-i [list ${args[*]}]"); args=(); n=1; else args+=("$a"); fi
        done
        n=0
        for a in "${args[@]}"; do
            [[ $a =~ ^\(\?[[:alpha:]]+\) ]] || a="(?n)$a"
            cmd+=("-regex [dec $(__enc <<< "$a")] {set result $((n++))}")
        done
        __cpio $((to+1)) <<<"
            array unset expect_out; set matches 0;
            expect -timeout $to ${cmd[*]} timeout {set result timeout} eof {set result eof};
            set matches [llength [lsearch -all [array names expect_out] *string]];
            puts \"0 \$result\"
        " || return $?
        echo $__reply
    }

    # match [n] - print nth match string from last expect, 0 for the match itself (default), otherwise the regex submatch.
    # shellcheck disable=2120 # arg is optional
    match() {
        __cpio <<<"
            if {${1:-0} < \$matches} {
                puts \"0 [enc [string map {\"\r\n\" \"\n\" \"\n\r\" \"\n\" \"\r\" \"\n\"} \$expect_out(${1:-0},string)]]]\"
            } {puts 1}
        " || return $?
        __dec <<< $__reply
    }

    # Print the spawnid that produced the match or eof from the last expect.
    matchid() { __cpio <<< 'puts "0 [enc $expect_out(spawn_id)]"' || return $?; __dec <<< $__reply; }

    # debug stuff
    debre() { __start; __cpio <<< 'exp_internal 1; puts 0'; }                   # write regex processing debug to stderr
    debsp() { __start; __cpio <<< 'log_file -a -leaveopen stderr; puts 0'; }    # write spawned process i/o to stderr
}

# Example/test: telnet to http hosts in parallel, send them all GET requests and scrape
# "Date:" from their response headers.

declare -A spawned # an array of [spawnid]=hostname

# first telnet to all
for h in chatgpt.com facebook.com google.com instagram.com reddit.com whatsapp.com x.com yahoo.com youtube.com ; do
    if spawn telnet $h 80; then
        spawned[$(spawnid)]=$h
    else
        echo  "telnet $h failed" >&2
    fi
done

# wait for each to connect and send GET request
eval "declare -A connecting=(${spawned[*]@K})" # dupe the array
while [[ ${connecting[*]} ]]; do
    r=$(expect ${!connecting[*]} -- 'Connected to')
    case $r in
        0)  # connected
            m=$(matchid)
            send $m <<< "GET / HTTP/1.0\r\r"
            unset "connecting[$m]"
            ;;
        timeout)
            for i in "${!connecting[@]}"; do
                echo "${connecting[$i]} failed to connect" >&2
                close $i
                unset "spawned[$i]"
            done
            break
            ;;
        eof)
            m=$(matchid)
            echo "Connection to ${connecting[$m]} dropped" >&2
            close $m
            unset "connecting[$m]" "spawned[$m]"
            ;;
    esac
done

# Wait for each to send "Date: xxx"
while [[ ${spawned[*]} ]]; do
    r=$(expect ${!spawned[*]} -- "^[Dd]ate:\s+(.+)")
    case $r in
        0)  # matched
            m=$(matchid)
            printf "%-16s: %s\n" ${spawned[$m]} "$(match 1)"
            close $m
            unset "spawned[$m]"
            ;;
        timeout)
            for h in "${spawned[@]}"; do echo "No response from $h"; done
            exit 1
            ;;
        eof)
            m=$(matchid)
            echo "Connection to ${spawned[$m]} dropped" >&2
            close $m
            unset "spawned[$m]"
            ;;
    esac
done
