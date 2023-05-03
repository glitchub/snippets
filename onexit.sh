#!/bin/bash -u

# A stacked exit handler for bash. Commands prefaced with "onexit" are executed when the script
# exits, in reverse order of definition. Enclose the command in "double-quotes" if it includes shell
# meta-characters, or 'single-quotes' to defer variable expansion until exit.

onexit() { onexit=("$*" "${onexit[@]}"); }; trap 'eval ${onexit[*]/%/;}' EXIT

# POC

onexit echo First in, last out

# create a temp directory
dir=$(mktemp -d) || exit 1
echo Created $dir

# test if the directory exists on exit (it should not)
onexit "[[ -d $dir ]] && echo Oops, $dir was not removed || echo $dir removed as expected"

# remove the directory on exit, this happens before the test above
onexit rm -rf $dir

# do stuff with the directory
touch $dir/xyzzy || exit 1

onexit echo Last in, first out

echo "This will run on exit:"
printf "  %s\n" "${onexit[@]}"
