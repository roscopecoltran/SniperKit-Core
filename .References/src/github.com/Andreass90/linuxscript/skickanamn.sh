#!/bin/bash
read -p "ge mig ett namn ? " Username # frågar efter ett namn
echo  $Username >> namn.txt #skickar namnet till namn.txt
echo "Ditt namn finns nu lagrat i namn.txt" # berättar att namnet nu finns i namn.txt
if [ -e namn.txt ]; then # kollar om namn.txt finns
        cat namn.txt # visar listar på namn om den finns
 else
    echo "filen finns inte" # skriver ut filen finns inget om den inte finns
     exit 1 #skickar error till ditt shell
fi 
exit 0
