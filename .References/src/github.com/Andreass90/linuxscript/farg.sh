#!/bin/bash
Farg="röd" # berättar vilken färg som ska väljas om inget väljs
read -t 5 -p "vilken är din favoritfärg ? " Farg # frågar vilken din favoritfärg är och sätter tiden till 5 sekunder
Farg=${Farg:="röd"} 
echo -e  "\nDin favoritfärg är $Farg" # berättar vilken din favoritfärg är
exit 0
