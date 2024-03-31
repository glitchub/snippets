# Bash functions to valid CIDRs and expand network prefixes

# Given a CIDR address in format w.x.y.z[/prefix], and an optional default network prefix, validate
# and print normalized CIDR or return false. If /prefix is not specified, appends the default prefix
# or 32. If the default is < 0 then any prefix is deleted.
cidr() {(
    [[ ${1:-} =~ ^0*([0-9]+)\.0*([0-9]+)\.0*([0-9]+)\.0*([0-9]+)(/0*([0-9]+))?$ ]] || exit 1
    d=${2:-32}
    set -- ${BASH_REMATCH[*]:1:4} ${BASH_REMATCH[6]:-$d}
    (( $1<=255 && $2<=255 && $3<=255 && $4<=255 && $5<=32 )) || exit 1
    if ((d >= 0)); then
        printf "%u.%u.%u.%u/%u" $*
    else
        printf "%u.%u.%u.%u" ${*:1:4}
    fi
)}

# Given a valid CIDR and an IP, return true if both are in the same subnet
reachable()
{(
    [[ $1 =~ ^([0-9]+)\.([0-9]+)\.([0-9]+)\.([0-9]+)/([0-9]+)$ ]] || exit 1
    ip1=$(( 0x$(printf "%02X" ${BASH_REMATCH[*]:1:4}) ))
    nm=$(((2 ** 32) - (2 ** (32 - ${BASH_REMATCH[5]}))))
    [[ $2 =~ ^([0-9]+)\.([0-9]+)\.([0-9]+)\.([0-9]+)$ ]] || exit 1
    ip2=$(( 0x$(printf "%02X" ${BASH_REMATCH[*]:1:4}) ))
    (( (ip1 & nm) == (ip2 & nm) ))
)}


################################################

# Tests

set -u # complain about unset variables

die() { echo $* >&2; exit 1; }

cidrtests=(

    # Each string has two or three words. The first is the expected output, or - if cidr() is
    # expected to fail, the second is the cidr argument string, the third is the optional default
    # prefix.

    # expect             cidr            default
    " -                                     "
    " 1.2.3.4/32         1.2.3.4            "
    " 1.2.3.4/24         01.02.03.04     24 "
    " 1.2.8.9/24         01.02.08.09     24 "
    " 1.2.3.4/24         1.2.3.4/24      10 "
    " 1.2.3.4/10         1.2.3.4         10 "
    " 1.2.3.4            1.2.3.4         -1 "
    " 1.2.3.4            1.2.3.4/10      -1 "
    " 255.255.255.255/32 255.255.255.255    "
    " 0.0.0.0/0          0.0.0.0/0       32 "
    " -                  hello           23 "
    " -                  1.2.3.400        1 "
    " -                  1.2.3.4/33         "
    " -                  1.2.3.4         33 "
    " -                  1.2.3.4.5       11 "
    " -                  1.2.3.4//32     22 "
)

for testcase in "${cidrtests[@]}"; do
    set -- $testcase
    if cidr=$(cidr ${*:2}); then
        if [[ $cidr != $1 ]]; then
            [[ $1 == - ]] || die "Expected 'cidr ${*:2}' to return '$1' but got '$cidr' instead"
            die "Expected 'cidr ${*:2}' to fail but got '$cidr' instead"
        fi
    else
        [[ $1 == - ]] || die "Expected 'cidr ${*:2}' to return '$1' but it failed instead"
    fi
done

reachtests=(

    # Each string has three words, the first is expected result 0 (true) or 1 (false), the second is
    # a CIDRcidr, the this is an IP

    # result           cidr            ip
    " 0                1.1.1.1/24      1.1.1.255        "
    " 1                1.1.1.1/24      1.1.2.1          "
    " 1                1.1.1.1/24      invalid          "
    " 1                1.1.1           1.1.1.1          "
    " 0                1.1.1.1/0       255.255.255.255  "
    " 0                1.1.1.1/0       0.0.0.0          "
    " 0                1.1.1.1/32      1.1.1.1          "
    " 1                1.1.1.1/32      1.1.1.2          "
)

for testcase in "${reachtests[@]}"; do
    set -- $testcase
    reachable $2 $3
    res=$?
    (( res == $1 )) || die "Expected 'reachable $2 $3' to return $1, but it returned $res instead"
done

echo "All tests passed"
