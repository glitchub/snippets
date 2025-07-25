#!/bin/bash

{
    # table func [-N] "head" ["head" ... "head"]
    # Given a function name, optional indent, and one or more table column headers, generate function
    # 'func'. If 'func' is invoked without argument then the table header is printed. Otherwise the
    # arguments are output as a new line of table data values.  Each column is exactly as wide as the
    # corresponding head, pad it with spaces as needed. If the first character of head is '<' then the
    # column data will be left justfied, if '>' or neither the the column data is right justified. The
    # leading '<' or '>' is removed.  If the first argument is "-N" where N is a decimal value, then the
    # table is indented that many spaces. '>' cabn be used as an escape.
    table()
    {
	local name=$1; shift
	local in=""; [[ $1 =~ ^\-[1-9][0-9]*$ ]] && { in=$(printf "%${1:1}s" ""); shift; }
	local head="$in" line="$in" data="$in" i=0 j d t
	for t; do
	    ((i++)) && { head+=" |"; line+="-+"; data+=" |"; }
	    [[ "$t" == \<* ]] && j="-" || j=""; t="${t#[<>]}"
	    head+=" $t"
	    for ((d=0; d<=${#t}; d++)); do line+="-"; done
	    data+=" %$j${#t}s"
	done
	line+="-"
	eval "function $name { local ${head@A} ${line@A} ${data@A}; ((\$#)) && printf \"\$data\\n\" \"\${@}\" || printf \"%s\\n%s\\n\" \"\$head\" \"\$line\"; }"
    }
}

# Test output:
#     left head   |   right head |   center head   |   left data
#    -------------+--------------+-----------------+-------------
#          data 1 |       data 2 |          data 3 | data 4
#          more 1 |       more 2 |                 | more 4
#             one |              |                 |
#          last 1 |       last 2 |          last 3 |

# define the function
table testable -4 "left head  " "  right head"  "  center head  "  "<  left data"
declare -f testable
echo

# write the head
testable
# write lines of data
testable "data 1" "data 2" "data 3"     "data 4"
testable "more 1" "more 2" ""           "more 4"
testable "one"
testable "last 1" "last 2" "last 3"
