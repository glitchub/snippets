#!/bin/bash

# Print randomly selected line from stdin
random() { awk 'BEGIN{srand('$SRANDOM')}{if(rand()*NR<1)s=$0}END{print s}'; }

# spew random dictionary words
while true; do random < /usr/share/dict/words; done
