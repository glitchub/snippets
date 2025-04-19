#!/bin/bash

spin()
{
    declare -gi __spinst;
    local spinner=('⠊⠁' '⠈⠑' ' ⠱' ' ⡰' '⢀⡠' '⢄⡀' '⢆ ' '⠎ ')
    printf '%s\b\b' "${spinner[$((__spinst=(__spinst+1)%8))]}"
}

printf "Hit enter... ";

tput -S <<<"civis bold setaf 15"            # cursor off, bold white
while ! read -s -t.1; do spin; done         # spin until done
tput -S <<<"cnorm sgr0 el"                  # restore
echo

echo "Done"
