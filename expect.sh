#!/bin/bash

# Expect functionality in bash

# Private symbols start with "__", avoid direct reference.

__expect=$(type -pf expect 2>/dev/null) || { echo "Requires executable 'expect'" >&2; exit 1; }

__start() {
    declare -gi __started
    ((__started)) || coproc __coproc {
        # start expect with this that interacts with __cpio
        exec {exp}<<EOT
            log_user 0; set stty_init {-opost -echo}
            proc encode {s} {binary encode base64 [encoding convertto utf-8 \$s]}
            proc decode {s} {encoding convertfrom utf-8 [binary decode base64 \$s]}
            while {[gets stdin line] >= 0} {if [catch {eval \$line} err] {puts "2 \$err"}}
EOT
        exec $__expect -f /dev/fd/$exp
    }
    __started=1
}

# Send stdin to the coprocess and get response to __reply, or die.
__reply=""
__cpio() {
    local timeout=${1:-10} in xs
    readarray -t in
    # echo ">>> ${in[*]}" >&2
    echo "${in[*]}" >&${__coproc[1]} || { echo "coproc write failed" >&2; exit 1; }
    IFS= read -r -t $timeout __reply  <&${__coproc[0]} || { echo "coproc read failed" >&2; exit 1; }
    # echo "<<< $__reply" >&2
    xs=${__reply%% *}      # first token is exit status, 2 for tcl error
    __reply="${__reply#* }" # the rest is the response or error string
    return $xs
}

# Pass arbitrary strings between bash and expect without quoting/whitespace issues
__encode () { $__expect -c 'fconfigure stdin -translation binary; puts -nonewline [binary encode base64 [encoding convertto utf-8 [read -nonewline stdin]]]; flush stdout'; }
__decode () { $__expect -c 'fconfigure stdout -translation binary; puts -nonewline [encoding convertfrom utf-8 [binary decode base64 [read -nonewline stdin]]]; flush stdout'; }

# "Public" functions, all return true if success and false if error. These are roughly analogous to
# expect functions of the same name.

# spawn program [args] - spawn the specified program to background
spawn() {
    __start
    readarray -t args < <(for a; do printf "%s" "$a" | __encode; echo; done) # encode each arg discretely
    __cpio <<< "if [spawn -noecho {*}[lmap arg {${args[*]}} {binary decode base64 \$arg}]] {puts 0} {puts 1}";
}

# Print the current spawn id (the most recently spawned program)
spawnid() { __cpio <<< "puts \"0 \$spawn_id\"" && echo ${__reply}; }

# close [spawnid] - close the current/specified spawnid
# shellcheck disable=2120 # arg is optional
close() { __cpio <<< "close ${1:+-i $1}; wait -nowait ${1:+-i $1}; puts 0"; }

# send [spawnid] << text - write stdin to the current/specified spawnid
# shellcheck disable=2120 # arg is optional
send() { __cpio <<< "send ${1:+-i $1} [decode $(printf "%b" "$(cat)" | __encode)]; puts 0"; }

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
    n=0; for arg in "${args[@]}"; do cmd+=("-regex [decode $(printf "%b" "$arg" | __encode)] {set result {0 $((n++))}}"); done
    __cpio $((timeout+1)) <<EOT || return $?
        array unset expect_out; set matches 0;
        expect -timeout $timeout ${cmd[*]} timeout {set result {0 timeout}} eof {set result {0 eof}};
        set matches [llength [lsearch -all [array names expect_out] *string]]; puts \$result
EOT
    echo $__reply
}

# match [n] - print nth match string from last expect, 0 for the match itself (default), otherwise the regex submatch.
# shellcheck disable=2120 # arg is optional
match() { __cpio <<< "if {${1:-0} < \$matches} {puts \"0 [encode [string map {\"\r\n\" \"\n\" \"\n\r\" \"\n\" \"\r\" \"\n\"} \$expect_out(${1:-0},string)]]]\"} {puts 1}" && __decode <<< "$__reply"; }

# Print the spawnid that produced the match or eof from the last expect.
matchid() { __cpio <<< "puts \"0 \$expect_out(spawn_id)\"}" && echo "$__reply"; }

# Show expect regex debug info on stderr
debex() { __start; __cpio <<< "exp_internal 1; puts 0"; }

# Show raw spawned process I/O on stderr
debsp() { __start; __cpio <<< "log_file -a -leaveopen stderr; puts 0"; }

############################
# POC: telnet to google.com port 80, send a GET request and scrape the current time from the response
# header.
die() { echo "$*" >&2; exit 1; }
spawn telnet google.com 80 >/dev/null || die "spawn failed"
m=$(expect "Connected to.*") && [[ $m == 0 ]] || die "connect failed ($m)"
send <<< "GET / HTTP/1.0\r\r" || die "send failed"
m=$(expect "(?n)^Date:\s+(.+)") && [[ $m == 0 ]] || die "expect failed ($m)"
match 1
close
