#!/bin/tclsh

# table func [-N] "head" ["head" ... "head"]
# Given a function name, optional indent, and one or more table column headers, generate function
# 'func'. If 'func' is invoked without argument then the table header is printed. Otherwise the
# arguments are output as a new line of table data values.  Each column is exactly as wide as the
# corresponding head, pad it with spaces as needed. If the first character of head is '<' then the
# column data will be left justfied, if '>' (or neither) the column data is right justified. The
# leading '<' or '>' is always removed. If the first argument is "-N" where N is a decimal value,
# then the table will be indented that many spaces.
proc table {func args} {
    set indent ""
    if {[regexp {^-(\d+)$} [lindex $args 0] - n]} {
        set indent [string repeat " " $n ]
        set args [lrange $args 1 end]
    }

    set nargs [llength $args]
    set head $indent
    set line $indent
    set data $indent
    foreach a $args {
        if { $head != $indent } {
            set head "$head |"
            set line "$line-+"
            set data "$data |"
        }
        regexp {^([<>]?)(.+)$} $a - j t
        set head "$head $t"
        set line "$line-[string repeat "-" [string length $t]]"
        if { $j == "<" } { set j "-" } { set j "" }
        set data "$data %$j[string length $t]s"
    }
    eval "proc ::$func {args} {if {\$args != {}} {lappend args [string repeat { {}} [expr $nargs - 1]]; puts \[format {$data} {*}\[lrange \$args 0 [expr $nargs - 1 ]\]\]} else {puts {$head}; puts {$line-}}}"
}

# Test output:
#     left head   |   right head |   center head   |   left data
#    -------------+--------------+-----------------+-------------
#          data 1 |       data 2 |          data 3 | data 4
#          more 1 |       more 2 |                 | more 4
#             one |              |                 |
#          last 1 |       last 2 |          last 3 |

table testable -4 "left head  " "  right head"  "  center head  "  "<  left data"
puts [info body testable]
puts ""
testable
testable "data 1" "data 2" "data 3" "data 4"
testable "more 1" "more 2" ""       "more 4" "this will be chopped"
testable "one"
testable "last 1" "last 2" "last 3"
