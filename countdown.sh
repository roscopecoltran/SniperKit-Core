#!/bin/bash
Tal1="100" #Variabler
Tal2="10" #Varibaler
read -p "Skriv ett nummer mellan 10 och 100 " Nummer #Skriv in ett tal 
if [ $Nummer -lt "$Tal2" ]; then #Om talet än mindre än tal2 
echo "Wrong number" # Så säger vi fel nummer
elif [ $Nummer -gt "$Tal1" ]; then #Om det är större än tal ett 
echo "Wrong number" #Så säger vi fel nummer
else #Annars
until [ "$Nummer" -eq 0 ] ; do #Ränkar vi ner
	echo "Countdown: $Nummer"
Nummer=$[Nummer -1]
done
fi
exit 0
