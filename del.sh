#!/bin/bash
if [ $EUID -ne 0 ]; then #Fungerar inte så som i labben med backticks
echo "Du måste vara root för att kunna köra $0"
else 
rm -rf var/tmp/*
echo "Städar upp i /tmp"
fi
exit 1
