# same as bash echo, but single quote any args that contain whitespace or either quote
qecho() { local a=() t; for t; do [[ $t =~ [[:space:]\'\"] ]] && a+=("${t@Q}") || a+=("$t"); done; echo "${a[@]}"; }

# same as bash echo, but double-quote any args that contain whitespace or either quote
qqecho() { local a=() t; for t; do [[ $t =~ [[:space:]\'\"] ]] && a+=("\"${t//\"/\\\"}\"") || a+=("$t"); done; echo "${a[@]}"; }

# E.G.
# $ bash qecho.sh This is "a test".
# This is 'a test.'
# This is "a test."
# $ bash qecho.sh This isn\'t \"just\" "a test".
# This 'isn'\''t' '"just"' 'a test.'
# This "isn't" "\"just\"" "a test."
qecho "$@"
qqecho "$@"
