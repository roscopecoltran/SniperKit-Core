#!/bin/bash
usdollar=6.6 #Aktuell dollar kurs
read -p "Vill du konvertera ifrån kronor eller dollar [SEK|US]: " konvertera; # frågar vilken valuta du vill konvertera
if [ "$konvertera" = "US" ]; then # om du väljer dollar
echo "hur många dollar vill du konvertera till kronor? " #frågr hur mycket
read Dollar1 
kronor=$(echo " scale=4; $Dollar1 *  $usdollar" | bc); #räknar ut med bc eftersom bash inte kan hantera flyttal. Tack till gusnan
echo "$Dollar1 dollar  är $kronor kronor" # Visar resultatet
elif [ "$konvertera" = "SEK" ]; then # Om du valt sek
echo "hur många kronor vill du konvertera till dollar? " # hur mycket du ska konvertera
read Kronor1
dollar=$(echo " scale=4; $Kronor1 / $usdollar" | bc); # räknar ut
echo "$Kronor1 kronor är $dollar dollar" # visar
fi # avslutar ifsatsen
exit 0 # avslutar scriptet
