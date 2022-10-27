#!/bin/bash -eu

# Show the result of integer division and modulus for various tools or languages.
# Numerator and divisor can be specified on the command line, defaults are -255
# and 8.
#
# Without arguments, expect this output:
#
#              -255 / 8   -255 % 8
#            ---------- ----------
#    awk        -31.875         -7
#    bash           -31         -7
#    bc             -31         -7
#    c              -31         -7
#    dc             -31         -7
#    expr           -31         -7
#    nodejs     -31.875         -7
#    perl       -31.875          1
#    python2        -32          1
#    python3    -31.875          1
#    rust           -31         -7
#    tcl            -32          1
#
# Note this script assumes that all the prerequisites are already installed and
# will rudely choke if not.

set -o pipefail

die() { echo $* >&2; exit 1; }

# Set numerator and divisor
numerator=${1:--255}
divisor=${2:-8}
# sanity
[[ $numerator =~ ^-?[0-9]+$ && $divisor =~ ^-?[0-9]+$ ]] || die "Must specify integers"
(($divisor)) || die "Divisor cannot be 0"

# true if $1 is executable
can() { type -p $1 &>/dev/null; }

# scratch directory, deleted at exit
tmp=$(mktemp -d)
trap 'rm -rf $tmp' EXIT

# build C binary
can gcc && printf "%s\n" "#include <stdio.h>" "void main(void) {printf(\"%d %d\n\", $numerator/$divisor, $numerator%$divisor);}" | gcc -x c -o $tmp/c.out -

# build rust binary
can rustc && printf "%s\n" "fn main() { println!(\"{} {}\", $numerator/$divisor, $numerator%$divisor); }" | rustc -o $tmp/rust.out -

# max column width
width=$((${#divisor} + ${#numerator} + 5))

# $width worth of dashes
bar() { eval printf -- "-%.0s" {1..$width}; }

# print formatted row
row() { printf "%-7s %${width}s %${width}s\n" "$1" "${2::$width}" "${3::$width}"; }

# if command is runnable return true, else print row placeholders and return false
try() { can $1 && return 0; echo "??? ???"; false; }

# heading
echo
row ""      "$numerator / $divisor" "$numerator % $divisor"
row ""      $(bar) $(bar)

# try each command of interest
row awk     $(try awk           && awk "BEGIN{print $numerator/$divisor, $numerator%$divisor; exit}")
row bash    $((numerator / divisor)) $((numerator % divisor))
row bc      $(try bc            && bc <<<"$numerator/$divisor; $numerator%$divisor")
row c       $(try $tmp/c.out    && $tmp/c.out)
row dc      $(try dc            && dc -e "${numerator//-/_} ${divisor//-/_} / p ${numerator//-/_} ${divisor//-/_} % p") # dc requires '_' for a negative sign
row expr    $(try expr          && expr $numerator / $divisor) $(expr $numerator % $divisor)
row nodejs  $(try nodejs        && nodejs <<<"console.log($numerator/$divisor, $numerator%$divisor)")
row perl    $(try perl          && perl -e "print $numerator/$divisor, ' ', $numerator%$divisor")
row python2 $(try python2       && python2 -c "print $numerator/$divisor, $numerator%$divisor")
row python3 $(try python3       && python3 -c "print($numerator/$divisor, $numerator%$divisor)")
row rust    $(try $tmp/rust.out && $tmp/rust.out)
row tcl     $(try tclsh         && tclsh <<<"puts \"[expr $numerator / $divisor] [expr $numerator % $divisor]\"")

echo
