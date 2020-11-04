#!/bin/bash

# chr() and ord() functions in bash

# Convert the value to an octal escape sequence, then evaluate the escape
# sequence as a character.
chr() { printf "$(printf '\\%03o' "$1")"; }

# Leverage a magic printf hack, the string 'X with a single leading quote is
# evaluated as the ordinal value of X. Works with gnu printf as well as the
# bash builtin.
ord() { printf %d "'$1"; }

set LC_ALL=C

# Convert 0 through 255 to characters, then dump as hex to show chr does not
# introduce conversion errors
for n in {0..255}; do
    chr $n
done | hexdump -e '16/1 "%02X " "\n"'
echo

# Same as above, but convert the characters back to values with ord(). Note
# printf will complain about NULL and other control characters, in general ord
# should only be used with printable characters.
for n in {0..255}; do
    printf "%02X " $(ord "$(chr $n)")
done | fold -w48
echo
