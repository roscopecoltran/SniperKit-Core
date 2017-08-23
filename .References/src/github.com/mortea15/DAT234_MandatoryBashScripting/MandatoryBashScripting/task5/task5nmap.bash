#!/bin/bash
# task5nmap.bash

echo -n "Enter an IPv4 address here:"
read ipaddress

ipaddr="${ipaddress%.*}"

echo "We'll ping all IPs in the range $ipaddr.0-$ipaddr.255, including $ipaddress"

nmap -sn $ipaddr.*
