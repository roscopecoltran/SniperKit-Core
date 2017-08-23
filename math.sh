#!/bin/bash
read -p "Skriv in ett tal? " Tal1
read -p "Skriv in ett annat tal? " Tal2
let Summa="Tal1+Tal2"
let Diff="Tal1-Tal2"
let Add="Tal1*Tal2"
let Kvot="Tal1/Tal2"
let Rest="Tal1%Tal2"
echo "$Tal1+$Tal2=$Summa"
echo "$Tal1-$Tal2=$Diff"
echo "$Tal1*$Tal2=$Add"
echo "$Tal1/$Tal2=$Kvot. $Rest blir över"
