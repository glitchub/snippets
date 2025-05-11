#!/bin/bash -u

# list of network interfaces
iflist() { find /sys/class/net -mindepth 1 -maxdepth 1  -type l -printf "%f\n"; }

# true if interface $1 is a physical device
ifphys() { ifexists $1 && ! [[ $(readlink /sys/class/net/$1) =~ virtual ]]; }

# true if interface $1 exists
ifexists() { [[ -d /sys/class/net/$1 ]] 2>/dev/null; }

# true if interface $1 is up
ifup() { [[ $(cat /sys/class/net/$1/operstate 2>/dev/null) == up ]]; }

# true if interface $1 is linked
iflinked() { (($(cat /sys/class/net/$1/carrier 2>/dev/null))); }

# get interface $1 IP, index $2 (default 0 or "" for all)
ifip() { ip -4 -j addr show dev $1 | jq -r '.['${2-0}'].addr_info[]|[.local,.prefixlen]|join("/") // empty' 2>/dev/null; }
ifip6() { ip -6 -j addr show dev $1 | jq -r '.['${2-0}'].addr_info[]|[.local,.prefixlen]|join("/") // empty' 2>/dev/null; }

# interface $1 default route
ifgw() { ip -j route show default dev $1 | jq -r '.[].gateway // empty'; }

### test
show() { printf "  %-10s: %s\n" "$1" "${*:2}"; }
yn() { "$@" && echo yes || echo no; }

for i in $(iflist); do
    echo "$i:"
    show "Physical?" "$(yn ifphys $i)"
    if ifup $i; then
        show "Linked?" "$(yn iflinked $i)"
        show "IP" "$(ifip $i)"
        show "IPv6" "$(ifip6 $i)"
        show "Gateway" "$(ifgw $i)"
    fi
done
