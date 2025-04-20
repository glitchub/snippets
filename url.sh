#!/bin/bash

# Given an URL, decompose it into the $URL associative array and return true, or false if URL is invalid.
deurl()
{
    [[ "$*" =~ ^([^:@/\ ]+)://(([^@/\ ]+)?@)?([^:@/\ ]+)(:([0-9]*))?(/([^?#]*))?(\?([^#]*))?(#(.*))?$ ]] &&
    declare -gA URL=(
        [scheme]=${BASH_REMATCH[1]} [auth]=${BASH_REMATCH[3]} [host]=${BASH_REMATCH[4]} [port]=${BASH_REMATCH[6]}
        [path]=${BASH_REMATCH[8]} [query]=${BASH_REMATCH[10]} [fragment]=${BASH_REMATCH[12]}
    )
}

# Recompose url from $URL, presumably after changing something.
reurl()
{
    local parts=(
        "${URL[scheme]}" "${URL[auth]:+${URL[auth]}@}" "${URL[host]}" "${URL[port]:+:${URL[port]}}"
        "${URL[path]:+/${URL[path]}}" "${URL[query]:+?${URL[query]}}" "${URL[fragment]:+#${URL[fragment]}}"
    )
    printf "%s://%s%s%s%s%s%s%s" "${parts[@]}"
}

# test
if deurl "${1:-https://user:pass@www.example.co.uk:443/blog/article/search?docid=720&hl=en#dayone}"; then
    echo "URL       = $(reurl)"
    echo "Scheme    = ${URL[scheme]}"
    echo "Auth      = ${URL[auth]}"
    echo "Host      = ${URL[host]}"
    echo "Port      = ${URL[port]}"
    echo "Path      = ${URL[path]}"
    echo "Query     = ${URL[query]}"
    echo "Fragment  = ${URL[fragment]}"
else
    echo "URL is invalid"
fi
