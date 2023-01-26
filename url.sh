#!/bin/bash

# Given an URL, decompose it into the $URL associative array and return true, or false if URL is invalid.
# Spaces are allowed in the final part, %-encoded characters are not considered.
url()
{
    [[ "$*" =~ ^([^:@/\ ]+)://(([^:@/\ ]+)(:([^@/\ ]*))?@)?([^:@/\ ]+)(:([0-9]*))?(/(.*))?$ ]] &&
    declare -gA URL=([scheme]=${BASH_REMATCH[1]} [username]=${BASH_REMATCH[3]} [password]=${BASH_REMATCH[5]}
                     [hostname]=${BASH_REMATCH[6]} [port]=${BASH_REMATCH[8]} [path]=${BASH_REMATCH[10]} )
}

if !(($#)); then
    echo "Specify an URL!"
elif url "$*"; then
    echo "Scheme   = ${URL[scheme]}"
    echo "Username = ${URL[username]}"
    echo "Password = ${URL[password]}"
    echo "Hostname = ${URL[hostname]}"
    echo "Port     = ${URL[port]}"
    echo "Path     = ${URL[path]}"
else
    echo "URL is invalid"
fi
