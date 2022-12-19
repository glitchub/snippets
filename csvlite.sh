#!/bin/bash -u

die() { echo "$@" >&2; exit 1; }

usage() { die "\
Usage:

    $0 [options] file.csv ['command' [... 'command']]

Perform sqlite3 commands against the specified csv file. The first line of the file is assumed to contain column
names.

Options are:

    -o file  - write resulting csv to the specified file on exit
    -r       - read-only, do not update database on exit
    -t name  - name of generated sql table, default is 'csv'

If commands are specified then they are executed in the order given. Exit due to error will cause the script to
abort.

Otherwise an interactive shell is opened. Exit due to ^\ or other signal will cause script to abort.

After commands are processed or shell terminates, if -r was specified then the script exits.

If -o was not given then 'file.csv' will be updated if the database was changed.

If -o was given then the designated file will be unconditionally written.

Note that csv is a pure-text format, therefore all columns are treated as TEXT, there are no indexes or keys, and NULL
fields are converted to empty strings.

Example:
    $ printf 'num,name\\n9,nine\\n10,ten\\n12,twelve\\n20,twenty\\n50,fifty\\n' > test.csv
    $ ./csvlite '.headers on' '.mode column' 'select * from csv where cast(num as integer) >= 10 and name like \"tw%\"' test.csv
    num         name
    ----------  ----------
    12          twelve
    20          twenty
";}

set -o noglob

out=
table=csv
ro=0
while getopts ":o:rt:" o; do case $o in
    o) out=$OPTARG ;;
    r) ro=1 ;;
    t) table=$OPTARG ;;
    *) usage ;;
esac; done
shift $((OPTIND-1))

(($#)) || usage
csvfile=$1
[[ -s $csvfile ]] || die "No such file $csvfile"

db=$(mktemp -t csvlite.XXXXXXXX)
trap "rm -f $db" EXIT

# read csv to database
sqlite3=$(type -P sqlite3) || die "Need executable sqlite3"
$sqlite3 -bail -csv $db ".import $csvfile $table" || die "Unable to import csv from $csvfile"
md5=$(md5sum $db)

if (($# > 1)); then
    # perform commands, die on error
    $sqlite3 -bail $db "${@:2}" || die "sqlite3 command failed"
else
    # interactive, die on ^\, etc
    $sqlite3 $db -cmd '.prompt "csvlite> " "> "'
    (($? >= 128)) && die "sqlite3 abnormal exit"
fi

# maybe update
if ((!ro)) && [[ $out || $md5 != $(md5sum $db) ]]; then
    $sqlite3 -csv $db ".headers on" "select * from $table" > ${out:-$csvfile}
fi
