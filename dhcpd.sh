#!/bin/bash
dialog --inputbox "Ditt subnet?" 10 60 2> /tmp/subnet
	Subnet=`cat /tmp/subnet`
dialog --inputbox "Din nätmask?" 10 60 2> /tmp/netmask
	Netmask=`cat /tmp/netmask`
dialog --inputbox "Vart börjar din range?" 10 60 2> /tmp/range1
	Range1=`cat /tmp/range1`
dialog --inputbox "Vart slutar din range" 10 60 2> /tmp/range2
	Range2=`cat /tmp/range2`
dialog --inputbox "Vilken dns vill du ha?" 10 60 2> /tmp/dns
	Dns=`cat /tmp/dns`
dialog --inputbox "Vad är ip till din router?" 10 60 2> /tmp/router
	Router=`cat /tmp/router`
	echo "subnet $Subnet netmask $Netmask {" >/home/$USER/dhcpd.conf
	echo "range $Range1 $Range2;" >> /home/$USER/dhcpd.conf
	echo "option domain-name-server $Dns;" >> /home/$USER/ dhcpd.conf
	echo "option routers $Router" >> /home/$USER/dhcpd.conf
	echo "ddns-update-style none;" >> /home/$USER/dhcpd.conf
	echo "default-lease-time 34567;" >> /home/$USER/dhcpd.conf
	echo "max-lease-time 69120;" >> /home/$USER/dhcpd.conf
	echo "}" >> /home/$USER/dhcpd.conf
rm /tmp/subnet /tmp/netmask /tmp/range1 /tmp/range2 /tmp/dns /tmp/router
dialog --pause "Städar upp i tempfilerna" 10 60 5
dialog --msgbox "Din dhcpd.conf finns nu i /home/$USER/" 10 60
exit 0
