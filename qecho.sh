# same as bash echo, but single quote any args that contain whitespace
qecho() { local a=() t; for t; do [[ $t =~ [[:space:]] ]] && a+=("'$t'") || a+=("$t"); done; echo "${a[@]}"; }

# same as bash echo, but double-quote any args that contain whitespace
qqecho() { local a=() t; for t; do [[ $t =~ [[:space:]] ]] && a+=("\"$t\"") || a+=("$t"); done; echo "${a[@]}"; }

# test with command-line args
qecho "$@"
qqecho "$@"
