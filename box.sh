# +---------------+
# | text in a box |
# +---------------+
box() { local h="$*"; h=$(awk NF=${#h}+1 OFS=- <<<'');  printf "+-%s-+\n| %s |\n+-%s-+\n" "$h" "$*" "$h"; }

# Try it with command line args
box "$*"
