# Return true if specified file is UTF-8 text.
# en_US.utf8 support must be installed, check with "locale -a".
istext() { ! LC_ALL=en_US.utf8 grep -q '[^[:print:][:space:]]' $1; }

# Return true if specified file is plain ASCII text.
isplain() { ! LC_ALL=C grep -q '[^[:print:][:space:]]' $1; }

# Return true if specified file starts with UTF-8 byte order mark.
# https://en.wikipedia.org/wiki/Byte_order_mark
hasbom() { awk '/^\xef\xbb\xbf/{exit 0}{exit 1}' $1; }

# test with specified or default files
[ $# = 0 ] && set /etc/passwd ./utf8.txt /bin/ls
for f in "$@";do
    if ! [ -f $f ]; then
        echo "$f is not a file"
    elif istext "$f"; then
        if isplain "$f"; then
            echo "$f is plain text"
        elif hasbom "$f"; then
            echo "$f is UTF-8 text with a BOM"
        else
            echo "$f is UTF-8 text"
        fi
    else
        echo "$f is not text"
    fi
done
