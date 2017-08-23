#!/bin/bash
# task5.bash

echo -n "Enter an IPv4 address here: " # Request an IP from the user
read ipaddress # Read and store the input

ipaddr="${ipaddress%.*}" # Remove the last octate from the users input
alive=0 # Store the number of responding hosts
dead=0 # Store the number of non-responding hosts

echo "We'll ping all IPs in the range $ipaddr.0 to $ipaddr.255, including $ipaddress"

for ip in {0..255} # Ping every IP in the range 0 - 255
do
	fping -c1 -t400 $ipaddr.$ip 2>/dev/null 1>/dev/null # Using fping with 1 ping per address and 150ms timeout
	if [ "$?" = 0 ] # Response was 0, which means reachable
	then
		echo "$ipaddr.$ip is alive!"
		((alive++))
	else # Response was not 0, which means unreachable
		((dead++))
	fi # Finished
done
echo "$alive hosts responded / $dead hosts did not respond." # Echo the number of hosts responding/not responding
