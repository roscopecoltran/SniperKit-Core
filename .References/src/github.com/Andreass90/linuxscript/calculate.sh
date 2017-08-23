#!/bin/bash
function plus { #Skapar funktionen plus
Sum=$[ $1+$2 ]
echo "$1+$2=$Sum"
}
function minus { #SKapar funktionen minus
Diff=$[ $1-$2 ]
echo "$1-$2=$Diff"
}
function Kvot { #Skapar funktionen kvot
Kvot=$[ $1/$2 ]
echo "$1/$2=$Kvot"
}
function Add { #Skapar funktionen Add
Add=$[ $1*$2 ]
echo "$1*$2=$Add"
}
read -p "Mata in två tal " Tal1 Tal2 #Ber användaren mata in två tal
if [ -z $Tal1 ]; then #Om du inte matat in ett tal
echo "Du måste mata in två tal" #Får du ett felmeddelande
elif [ -z $Tal2 ]; then #Samma som ovan
echo "Du måste mata in två tal"
exit 1
fi 
if ! [ $Tal1 -eq $Tal1 &> /dev/null ]; then #Om det är något annat än tal
echo "Bara siffror tack" #Får du felmeddelande
elif ! [ $Tal2 -eq $Tal2 &> /dev/null ]; then #Samma som ovan
echo "Bara siffor tack"
else
plus $Tal1 $Tal2 #Ger resulat av plus
minus $Tal1 $Tal2 #Ger resulat av minus
Kvot $Tal1 $Tal2 #Ger resulat av kvot
Add $Tal1 $Tal2 #Ger resulatat av add
fi
exit 0
