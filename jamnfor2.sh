#!/bin/bash
echo "mata in två tal med mellanrum emellan" #uppmanar användaren att skriv 
read Tal1 Tal2 #tar två nummer
if [ "$Tal1" -lt "$Tal2" ]; then #jämnför nummerna om dem är less than
    echo "$Tal1 är mindre än $Tal2" #berättar om den är mindre än
elif [ "$Tal1" -gt "$Tal2" ]; then #jämnför om det är större
    echo "$Tal1 är större än $Tal2" #berättar om det är större än
elif [ "$Tal1" -eq "$Tal2" ]; then #jämnför om dem är lika
    echo "$Tal1 är samma som $Tal2" #berättar om dem är lika
fi
exit 0
