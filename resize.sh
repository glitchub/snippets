# Query the controlling terminal for its current window size and set the tty
# size to match.

# This is required when logging into a system via serial console. It's not
# usually needed with ssh and telnet because those protocols can track the
# window size automatically.

# This script provides the same functionality as the resize executable, so
# if you already have that then you don't need this.

# This script attempts to be POSIX-compliant, unless $BASH is set in which case
# it uses various bash-isms. (For this reason there is no hash-bang at the top
# and the script runs under whatever shell is used to launch it -- this is
# purely for demonstration!)

# In any case it requires a non-POSIX stty executable that supports the 'cols'
# and 'rows' options (e.g. GNU or busybox).

# The terminal must be attached to /dev/tty and support VT100/xterm-ish escape
# sequences:

#   "\e7"           - save cursor position
#   "\e[9999;9999H" - move cursor to lower right hand corner
#   "\e[6n"         - return current cursor position
#   "\e8"           - restore saved position

# ("\e" indicates the escape character). The terminal must return the requested
# cursor position in form "\e[rrr;cccR", where rrr and ccc are the 1-based row
# and column values.

# In normal operation resize should return immediately with no output. If
# there's a one-second pause it means the terminal did not return the expected
# sequence and the size was not set.

# Use 'stty size' to test that the tty size was in fact set correctly. If the
# shell is paying attention then 'echo $LINES $COLUMNS' should produce the same
# thing.

{
    if [ $BASH ]; then
        echo "bash"
        printf "\e7\e[9999;9999H\e[6n\e8"
        IFS="[;" read -t1 -rsdR x r c && [[ $r && $c ]] && stty rows $r cols $c
    else
        echo "posix"
        save=$(stty -g)
        stty -icanon -echo min 0 time 10
        (
            rows=""
            cols=""
            getch() { c=$(dd bs=1 count=1 2>/dev/null); [ -z "$c" ] && exit; [ "$c" = "$1" ]; }
            printf "\e7\e[9999;9999H\e[6n\e8"
            until getch "["; do :; done
            until getch ";"; do rows="$rows$c"; done
            until getch "R"; do cols="$cols$c"; done
            stty rows $rows cols $cols
        )
        stty $save
    fi
} <> /dev/tty
