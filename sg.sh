#!/bin/bash

# Given a block device name, print the scsi generic equivalent and return true.
# This is highly dependent on how the kernel constructs sysfs. YMMV.
get_scsi_generic()
{
    local sg=$(readlink /sys/block/${1##*/}/device/generic)
    sg=/dev/${sg##*/}
    [[ -c "$sg" ]] && echo $sg
}

sg=$(get_scsi_generic $1) && echo $1 maps to $sg
