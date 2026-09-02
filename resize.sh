# Query the controlling terminal for its current window size and set the tty size to match.

# This is required when logging into a system via serial console. It's not usually needed with ssh
# and telnet because those protocols can track the window size automatically.

# The terminal must be attached to /dev/tty and support VT100/xterm-ish escape sequences:

#   "\e7"           - save cursor position
#   "\e[999;999H"   - move cursor to lower right hand corner
#   "\e[6n"         - return current cursor position
#   "\e8"           - restore saved position

# ("\e" indicates the escape character). The terminal must return the requested cursor position in
# form "\e[rrr;cccR", where rrr and ccc are the 1-based row and column values.

# Two resize functions are provided: a bash-specific version, and a POSIX-compliant version. Both
# require a non-POSIX stty executable that supports the 'cols' and 'rows' options (e.g. GNU or
# busybox).

# The functions return true if success, or false if stdin is not a tty, stty is the wrong version,
# or the terminal failed to respond. In the latter case there may be a one-second delay before
# return.

# Run 'bash -f resize.sh' to test the bash version, or 'dash -f resize.sh' to test the posix version.

if [ "$BASH" ]; then

    echo "Testing bash resize"

    resize()
    {
        local save res rows cols
        res=false
        if save=$(stty -g 2>/dev/null); then
            stty -echo
            printf "\e7\e[999;999H\e[6n\e8" >&0
            IFS="[;" read -t1 -r -dR _ rows cols && stty rows "$rows" cols "$cols" 2>/dev/null && res=true
            stty $save
        fi
        $res
    }

else

    echo "Testing posix resize"

    resize()
    {
        res=false
        if save=$(stty -g 2>/dev/null); then
            stty -icanon -echo min 0 time 10
            printf "\0337\033[999;999H\033[6n\0338" >&0
            rsp=$(dd bs=10 count=1 2>/dev/null)
            case $rsp in
                *'['*';'*R)
                    rows=${rsp%;*}; rows=${rows#*[}
                    cols=${rsp#*;}; cols=${cols%R}
                    stty rows "$rows" cols "$cols" 2>/dev/null && res=true
                    ;;
            esac
            stty $save
        fi
        $res
    }

fi

if resize; then
    echo "Resized to $(stty size)"
else
    echo "Resize failed"
fi
