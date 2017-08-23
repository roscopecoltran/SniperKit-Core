#!/bin/bash
fil="recept.txt" #variable 
if [ -e $fil ]; then # kontrollerar om filen finns
    echo "filen finns" # berättar för användaren att filen finns
else  # annars
    echo "filen finns inte" #berättar den att filen inte finns
fi
    if [ -w $fil ]; then #kontrollerar om filen är skrivbar
        echo "filen är skrivbar" #berättar för användaren att den är skrivbar
    fi
        if [ -r $fil ]; then #kontrollerar om filen är läsbar
            echo "filen är läsbar" #berättar att filen är läsbar
        fi
        sleep 3 #väntar 3 sekunder innan den går vidare
        vim $fil #öppnar filen i vim
        exit 0
