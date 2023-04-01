# print current bash function definitions, one per line, in an "eval"able fashion
declare -f | awk 's==""&&!/\(\)/{next} $0=="}"{print s"; }";s="";next} $1=="}"{s=s"; "} {gsub(/^ *| *$/,""); s=s$0" "}'
