#!/bin/bash

# This can be used instead of the bash $SECONDS variable when it's important not to be affected by
# changes to the system clock, or if you need it to advance relative to the time it's set rather
# than in sync with the system seconds. Replace existing use of "$SECONDS" with "$(seconds)", and
# "SECONDS=nnn" with "seconds nnn". Note unlike $SECONDS it reports system uptime by default so
# should explicitly be set to 0 (or something) before using.
seconds() {
    declare -gi __hundreds;
    local u; read u _ < /proc/uptime; u=${u/./}
    [[ $1 ]] && __hundreds=$((u-($1*100))) || echo $(((u-__hundreds)/100))
}

seconds 0
sleep 1
echo "$(seconds)"
sleep 1
echo "$(seconds)"

echo "Setting to 12345"
seconds 12345
sleep 1

echo "$(seconds)"
sleep 1
echo "$(seconds)"
