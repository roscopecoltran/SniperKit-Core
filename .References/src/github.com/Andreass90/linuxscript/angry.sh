#!/bin/bash
Tal2="10" #Variable
Tal3="1" #Variable
echo "skriv valfritt tal mellan 1 och 10" #skriv ett tal mellan 1 o 10
read Tal # skriv in talet
if [ "$Tal" -gt "$Tal2" ]; then #kontrollerar om talet är större än 10
    echo "idiot! Kan du inte läsa?" #säger till om du gjort fel
elif [ "$Tal" -lt "$Tal3" ]; then #kontrollerar om talet är mindre än 1
    echo "idiot! Kan du inte läsa?" #säger till om du gjort fel
else # annars
    echo "Duktig" #berättar att du varit duktig
fi
        exit 0
