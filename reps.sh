# reps N repstring
# Print repstring N times, the usual string escapes are supported.
reps() { awk NF=$1 O{F,R}S="$2" <<<''; }

# Test:

# $ bash ./reps.sh 4 "This is a test\n"
# This is a test
# This is a test
# This is a test
# This is a test

# $ bash ./reps.sh 10 hello
# hellohellohellohellohellohellohellohellohellohello

reps "$@"
