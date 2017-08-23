#!/bin/bash
if [ $EUID -ne 0 ]; then #Kontrollerar om du är root
echo "Du måste vara root för att kunna installera program" #Är du inte root kan du inte köra programmet
exit 1
fi
	dialog --backtitle "Debian dpkg installer" --title "Vill du köra skriptet?" --yesno "Vill du köra scriptet som installerar dpkgfiler?" 10 60 #Om du väljer ja skickas du vidare väljer du nej kommer programmet avslutas
if [ $? -eq 0 ]; then #Skickar dig vidare om du väljer ja
	dialog --menu "Välj vad du vill göra" 10 60 2 Installera "Installera paket" Installerade "Kolla vilka du har installerarade" 2> /tmp/dpkg #En meny om du vill installera program eller se installerade.
Val=`cat /tmp/dpkg` #Sparar valet i en fil som heter dpkg
else #Anars
	dialog --backtitle "Då avslutar vi" --msgbox "Då avslutar vi" 10 60 #Avslutar vi
clear #Städar terminalen efter avslutat skript
exit 0
fi
#om valet är installera så ska den
	if [ $Val = Installera ]; then
	dialog --checklist "Välj flaggor , Det flesta flaggor kräver att du använder -i som är installera" 20 100 3 -i "installera" on -G "Installera inte programmet om det redan finns en nyare version" off -E "Installerar inte om samma version finns" on 2> /tmp/dpkg1 
	dialog  --inputbox "Paketnamn" 10 60 2> /tmp/testing2
Testing2=`cat /tmp/testing2`
	find / -name $Testing2*.deb 1> /tmp/testing
Val2=`cat /tmp/testing`
	sed 's/"//g' /tmp/dpkg1 > /tmp/dpkg2
Val3=`cat /tmp/dpkg2`
	dpkg $Val3 $Val2 &> ~/programlogg
dialog --backtitle "Det gick bra" --msgbox "Det sparades en logg på installationen i ~/." 10 60
dialog --textbox ~/programlogg 20 100 
	elif [ $Val = Installerade ]; then
	dpkg -l | awk '{ print $2 }' | tail -f -n +7|nl > /tmp/lista
dialog --title "Installerade program"  --textbox /tmp/lista 40 200
fi
clear
rm /tmp/*
exit 0
