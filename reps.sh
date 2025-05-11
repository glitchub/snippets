# N repstring [endstring]
# print repstring N times, followed by endstring. The usual string escapes are supported.
reps() { awk NF=$1+1 OFS="$2" ORS="${3:-}" <<< '' 2>/dev/null; }

# Test:

# $ bash ./reps.sh 4 "This is a test\n"
# This is a test
# This is a test
# This is a test
# This is a test

# $ bash ./reps.sh 10 "hello" '!\n'
# hellohellohellohellohellohellohellohellohellohello

reps "$@"
