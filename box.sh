# +---------------+
# | text in a box |
# +---------------+
box() { local h="$*"; h=$(eval printf "%.0s-" {1..${#h}});  printf "+-%s-+\n| %s |\n+-%s-+\n" "$h" "$*" "$h"; }

# Try it with command line args
box "$*"
