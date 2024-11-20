# echo, but single quote any args that contain whitespace
qecho() { local a=(); while (($#)); do [[ $1 =~ [[:space:]] ]] && a+=("'$1'") || a+=("$1"); shift; done; echo "${a[@]}"; }

# echo, but double-quote any args that contain whitespace
qqecho() { local a=(); while (($#)); do [[ $1 =~ [[:space:]] ]] && a+=("\"$1\"") || a+=("$1"); shift; done; echo "${a[@]}"; }

# test with command-line args
qecho "$@"
qqecho "$@"
