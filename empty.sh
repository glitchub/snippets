#!/bin/bash

# Return true if given directory is empty
empty() { [[ -d $1 && -z $(shopt -s nullglob dotglob; echo $1/*) ]]; }

# Test with "bash empty.sh ~/*"
for o; do empty $o && echo $o is an empty directory; done
