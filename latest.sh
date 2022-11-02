#!/bin/bash

# Given one or more version strings, print the latest
latest() { printf '%s\n' "$@" | LC_ALL=C sort -rV | head -n1; }

# print "testcase", "testcase" -> "latest"
test() { printf '"%s"%s -> "%s"\n' "$1" "$(printf ', "%s"' "${@:2}")" "$(latest "$@")"; }
test 1-101-x 1-11-x
test 1-101-x 1-11-x 1-0101-y
test 1-101-x 1-11-x 1-0101-y 1-102-a
test 1.1.1 1.1.1.2
test 1.1.123 1.1.1.77
test 11.22.33.43 011.022.033.044 11.22.33.44
test 11.22.33.43 011.022.033.044 11.22.33.44 011.022.033.045
test 11_22_33_44 11-22-33-44 11.22.33.44
test 1.1a-47 1.1b
test 1.1a-47 1.1b 1.01b
test 123 123a
test 123 123a 1-2-3 1-2-3a
test 2-alpha-44 2-44
test 2-bbbb 2-aaaa-44
test 3.10 3.011 2.010
test 5[11] 5[100]
test 5[11] 5[100] 5-11
test 5[11] 5[100] 5-11 5.100
test 5678 5678a
test 5678 5678a 45678 45678a
test "prog 1.2.a" "prog 1.2"
test "w x.z.y" "w x z y" "w x.z y"
test "w.x.z.y" " w.x.z.y" "w.x.z.y " " w.x.z.y "
test "5.0.017(2)-alpha" "5.0.17(1)-release"
test "5.0.017(2)-alpha" "5.0.17(1)-release" "5.1.1(1)-release"
test "5.1.4(1)-beta" "05.01.04(01)-release"
test "xxx. .yyy" xxx..yyy "xxx.  .yyy" xxx--yyy
test "xxx. .yyy" xxx..zzz "xxx.  .zzz" xxx.zzz.zzz "xxx. .zzz"
