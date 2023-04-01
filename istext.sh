# Return true if specified file is UTF-8 text.
# en_US.utf8 support must be installed, check with "locale -a".
istext() { ! LC_ALL=en_US.utf8 grep -q '[^[:print:][:space:]]' $1; }

# Return true if specified file is plain ASCII text.
isplain() { ! LC_ALL=C grep -q '[^[:print:][:space:]]' $1; }

for f in /etc/passwd ./utf8.txt /bin/ls; do
    if istext $f; then
        if isplain $f; then
            echo "$f is plain text"
        else
            echo "$f is UTF-8 text"
        fi
    else
        echo "$f is not text"
    fi
done
