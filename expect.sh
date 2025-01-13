#!/bin/bash

# This script provides expect functionality in bash. Insert your program at the bottom, or source
# this script into another.

# Private symbols start with "__", avoid direct reference.

__expect=$(type -pf expect 2>/dev/null) || { echo "Requires executable expect" >&2; exit 1; }

__coproc_stdout="" __coproc_stdin=""
__start() {
    declare -gi __started
    if ((!__started)); then
        local exp CP
        # This code is run by the expect coprocess and interacts with __coproc()
        exec {exp}<<EOT
            log_user 0
            set stty_init {-opost -echo}
            proc encode {s} {binary encode base64 [encoding convertto utf-8 \$s]}
            proc decode {s} {encoding convertfrom utf-8 [binary decode base64 \$s]}
            while {[gets stdin line] >= 0} {if [catch {eval \$line} err] {puts "2 \$err"}}
EOT
        coproc CP { exec $__expect -f /dev/fd/$exp; }
        exec {exp}<&- {__coproc_stdout}<&"${CP[0]}" {__coproc_stdin}>&"${CP[1]}"
        __started=1
    fi
}

# Send expect script on stdin to the coprocess and read the response or timeout. Die if the
# coprocess does.
__resp=""
__coproc() {
    local timeout=${1:-10} in xs
    readarray -t in
    # echo ">>> ${in[*]}" >&2
    echo "${in[*]}" >&$__coproc_stdin || { echo "coproc write failed" >&2; exit 1; }
    IFS= read -r -t $timeout __resp  <&$__coproc_stdout || { echo "coproc read failed" >&2; exit 1; }
    # echo "<<< $__resp" >&2
    xs=${__resp%% *}      # first char is exit status
    __resp="${__resp#* }" # the rest is the actual response (could be a tcl error!)
    return $xs
}

# These are used to pass arbitrary strings between bash and expect without quoting/whitespace issues
__encode () { $__expect -c 'fconfigure stdin -translation binary; puts -nonewline [binary encode base64 [encoding convertto utf-8 [read -nonewline stdin]]]; flush stdout'; }
__decode () { $__expect -c 'fconfigure stdout -translation binary; puts -nonewline [encoding convertfrom utf-8 [binary decode base64 [read -nonewline stdin]]]; flush stdout'; }

# "Public" functions, all return true if success and false if error. These are roughly analogous to
# expect functions of the same name.

# spawn program [args] - spawn the specified program to background
spawn() {
    __start
    readarray -t args < <(for a; do printf "%s" "$a" | __encode; echo; done) # encode each arg discretely
    __coproc <<< "if [spawn -noecho {*}[lmap arg {${args[*]}} {binary decode base64 \$arg}]] {puts 0} {puts 1}";
}

# Print the current spawn id (the most recently spawned program)
spawnid() { __coproc <<< "puts \"0 \$spawn_id\"" && echo ${__resp}; }

# close [spawnid] - close the current/specified spawnid
# shellcheck disable=2120 # arg is optional
close() { __coproc <<< "close ${1:+-i $1}; wait -nowait ${1:+-i $1}; puts 0"; }

# send [spawnid] << text - write stdin to the current/specified spawnid
# shellcheck disable=2120 # arg is optional
send() { __coproc <<< "send ${1:+-i $1} [string map {{\n} \"\n\" {\r} \"\r\" {\t} \"\t\"} [decode $(__encode)]]; puts 0"; }

# expect [timeout] [spawnid [ ...spawnid] --] regex [...regex]
# Given optional timeout in seconds, optional list of spawnids followed by --, and list of regexes,
# wait for one of the regexes to match the output from the current or specified spawnids, print the
# associated regex index (0-based), or "timeout" or "eof" (if spawnid is closed), and return true.
# If multiple regexes could match on the same character the lowest regex wins. Timeout is set from
# the first arg if numeric, otherwise defaults to 10.
# See https://www.tcl-lang.org/man/tcl/TclCmd/re_syntax.htm for information on tcl extended regex
# syntax.
expect() {
    local args=() cmd=() n=0 arg="" timeout=10
    [[ $1 =~ ^[1-9][0-9]+$ ]] && { timeout=$1; shift; } # assume first arg is timeout if numeric
    for arg; do if ((!n)) && [[ $arg == -- ]]; then cmd=("-regex [list ${args[*]}]"); args=(); n=1; else args+=("$arg"); fi; done
    n=0; for arg in "${args[@]}"; do cmd+=("-regex [string map {{\n} \"\n\" {\r} \"\r\" {\t} \"\t\"} [decode $(__encode <<< "$arg")]] {set result {0 $((n++))}}"); done
    __coproc $((timeout+1)) <<EOT || return $?
        array unset expect_out; set matches 0;
        expect -timeout $timeout ${cmd[*]} timeout {set result {0 timeout}} eof {set result {0 eof}};
        set matches [llength [lsearch -all [array names expect_out] *string]]; puts \$result
EOT
    echo $__resp
}

# match [n] - print nth match string from last expect, 0 for the match itself (default), otherwise the regex submatch.
# shellcheck disable=2120 # arg is optional
match() { __coproc <<< "if {${1:-0} < \$matches} {puts \"0 [encode [string map {\"\r\n\" \"\n\" \"\n\r\" \"\n\" \"\r\" \"\n\"} \$expect_out(${1:-0},string)]]]\"} {puts 1}" && __decode <<< "$__resp"; }

# Print the spawnid that produced the match or eof from the last expect.
matchid() { __coproc <<< "puts \"0 \$expect_out(spawn_id)\"}" && echo "$__resp"; }

# Show expect regex debug info on stderr
debex() { __start; __coproc <<< "exp_internal 1; puts 0"; }

# Show raw spawned process I/O on stderr
debsp() { __start; __coproc <<< "log_file -a -leaveopen stderr; puts 0"; }

############################
# Insert your prorgram here.
############################

# Telnet to google.com port 80, send a GET request and scrape the server time from the response
# header.
die() { echo "$*" >&2; exit 1; }
spawn telnet google.com 80 >/dev/null || die "spawn failed"
m=$(expect "Connected to.*") && [[ $m == 0 ]] || die "connect failed ($m)"
send <<< "GET / HTTP/1.0\r\r" || die "send failed"
m=$(expect "(?n)^Date:\s+(.+)") && [[ $m == 0 ]] || die "get failed ($m)"
match 1
close
