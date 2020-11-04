#!/bin/bash

# chr() and ord() functions in bash

# Convert value 0-255 to a character. The character can reliably be written to
# a file (or stdout), but not all characters can be stored as strings (notably,
# chr(0) and chr(10)).
chr() { printf $(printf \\%03o $1); }

# Convert a character to a value, this takes advantage of a little-known printf
# trick.
ord() { printf %d "'$1"; }

cat <<EOT
Testing chr and ord for all values 0 to 255.

The value 0 will trigger a bash runtime error but pass anyway, by accident.

The value 10 will fail because bash swallows the resulting LF.

So the result should be "255/256 passed":

EOT

passed=0
for n in {0..255}; do

    # Convert to char and then feed through hexdump to capture the value of the
    # resulting character.
    c=$(chr $n | hexdump -e '1/1 "%u"')

    # Convert to char and then back to ord. Note the argument to ord must be quoted!
    o=$(ord "$(chr $n)")

    # all three values should be the same
    if ((n == c && n == o)); then
        ((passed++))
    else
        echo "chr($n) produced character with value $c, ord(chr($n)) produced value $o"
    fi
done
echo
echo "$passed/256 passed"
