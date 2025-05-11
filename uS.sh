#!/bin/bash

# Epoch time in microseconds
uS() { echo ${EPOCHREALTIME/.}; }

# Given strftime format string and epoch time in microseconds, print formatted time. Format string
# may additionally contain "%@" which is replaced with microseconds 000000 to 999999.
strfuS() { local f=${1//%@/$(printf "%06d" $(($2 % 10**6)))}; printf "%($f)T" $(($2 / 10**6)); }

start=$(uS)
echo "$start uS is $(strfuS "%F %T.%@" $start)"

echo "Please wait..."
sleep 2.5

elapsed=$(($(uS) - start))
echo "$elapsed uS elapsed after sleep 2.5 ($(TZ=UTC strfuS "%T.%@" $elapsed))"
