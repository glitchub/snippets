# source to print all bash function definitions, one per line, in an "eval"able fashion
declare -f | awk -n '$0=="}" {print s"; }"; s=""; next} $1=="}" {s=s"; "} {gsub(/^ *| *$/,""); s=s$0" "}'
