#!/bin/bash

# Given a block device name, print the scsi generic equivalent and return true.
# This is highly dependent on how the kernel constructs sysfs. YMMV.
get_scsi_generic()
{
    local sg=$(shopt -s globstar; set -- /sys/devices/**/scsi_generic/*/device/block/${1##*/}; set -f; IFS=/; set -- $1; echo /dev/${*: -4:1})
    [[ -c "$sg" ]] && echo $sg
}

sg=$(get_scsi_generic $1) && echo $1 maps to $sg
