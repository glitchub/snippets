# +---------------+
# | text in a box |
# +---------------+
box() { local h="$*"; h=$(awk NF=${#h} O{F,R}S=- <<<'');  printf "+-%s-+\n| %s |\n+-%s-+\n" "$h" "$*" "$h"; }

# Try it with command line args
box "$*"
