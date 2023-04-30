#!/bin/bash -u

# get local IP information
ipinfo() {
    # get default route device and gateway
    read dev gw < <(ip -4 -N -j route show default | jq -r '.[0] | [.dev, .gateway] | join(" ")')
    [[ $gw ]] || exit 1

    # get address that routes to the gateway
    ip=$(ip -N -j route get $gw | jq -r '.[].prefsrc//empty')
    [[ $ip ]] || exit 1

    # get device mac and address netmask prefix
    read mac nm < <(ip -N -j addr show $dev | jq -r '.[] | [.address, (.addr_info | .[] | select(.local=="'$ip'") | .prefixlen)] | join(" ")')
    [[ $nm ]] || exit 1

    echo $ip $nm $gw $dev $mac
}

read address prefix gateway device mac < <(ipinfo) || { echo "Unable to get IP info"; exit 1; }

echo "Address : $address/$prefix"
echo "Gateway : $gateway"
echo "Via     : $device ($mac)"
