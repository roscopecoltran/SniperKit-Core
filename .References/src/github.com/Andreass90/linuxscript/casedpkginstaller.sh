#!/bin/bash
#Ett skript som hjälper dig installera .debpaket med hjälp utav dialog

function checking { #Funktion som kontroller om dialog är installerat
dpkg-query -W -f='${Status}\n' dialog > /tmp/kontroll
Testing2=`cat /tmp/kontroll`
if [ "$Testing2" = "install ok installed" ]; then
checkroot
elif [ "$Testing2" = "unknown ok not-installed" ]; then
echo -e  "Det krävs ett program som heter dialog för att köra skriptet.\nVill du installera det?"
select yn in "Yes" "No"; do
    case $yn in
        Yes ) sudo apt-get install dialog; break;;
        No ) exit;;
    esac
done
checkroot
fi
}

function run1 {	#Skapar en funktion som frågar om du vill köra skriptet
	dialog --backtitle "Debian dpkg installer" --title "Vill du köra skriptet?"\
	 --yesno "Vill du köra scriptet som installerar dpkgfiler?" 10 60
}


function run2 { #Skapar en funktion som låter dig välja mellan att installera nya paket eller se dem som finns installerade.
	dialog --menu "Välj vad du vill göra" 10 60 2 Installera "Installera paket"\
	 Installerade "Kolla vilka du har installerarade" 2> /tmp/dpkg
	Val=`cat /tmp/dpkg` 
}

function run3 { #Funktion som berättar att vi avslutar programmet samt rensar terminalen
	dialog --backtitle "Då avslutar vi" --msgbox "Då avslutar vi" 10 60
	clear
	exit 0
}

function install { #Funktion som ber dig välja flaggor till din installation och sparar dem sen i en fil som vi kan hämta när det är dax att installera
	dialog --checklist "Välj flaggor , Det flesta flaggor kräver att du använder -i som är installera" 20 100 3\
	 -i "installera" on -G "Installera inte programmet om det redan finns en nyare version" off\
	 -E "Installerar inte om samma version finns" on 2> /tmp/dpkg1 
	if [ $? = 1 ]; then
	run3
	fi
	dialog  --inputbox "Paketnamn" 10 60 2> /tmp/testing2
	Testing2=`cat /tmp/testing2`
	find / -name $Testing2*.deb 1> /tmp/testing
	Val2=`cat /tmp/testing`
	sed 's/"//g' /tmp/dpkg1 > /tmp/dpkg2
	Val3=`cat /tmp/dpkg2`
	dpkg $Val3 $Val2 1> /tmp/programlogg 2> /tmp/logg2
}

function installed {	#Funktion som visar installerade program
	dpkg -l | awk '{ print $2 }' | tail -f -n +7|nl > /tmp/lista
	dialog --title "Installerade program"  --textbox /tmp/lista 40 200
}


function close { #Funktionen som ska köras när programmet avslutar , antingen visar det en logg hur det gick eller så berättar den att filen inte finns.
	Filen=`cat /tmp/programlogg`
	if [ -s $Filen ]; then
	dialog --msgbox "Hittade inga program med det namnet eller så har du redan det installerat" 10 60
	else
	dialog --title "Installationen gick bra" --textbox /tmp/programlogg 20 100
	fi
}

function clean { #Funktion som städar /tmp-katalogen samt rensar terminalen
	sudo rm -f /tmp/*
	clear
}

function cancel { #Funktion som rensar terminalen samt avbryter
	clear
	exit 0
}

function checkroot {
	if [ $EUID -ne 0 ]; then #Kontrollerar om du är root
	dialog --msgbox "Du måste tyvärr vara root för att köra detta program" 10 60
	cancel
	else 
	run1
	fi
	if [ $? = 0 ]; then
	run2
	elif [ $? = 1 ]; then
	run3
	fi
	if [ $Val = Installera ]; then 
	install
	close
	elif [ $Val = Installerade ]; then
	installed
	else
	run3
	fi
	clean
}

checking
exit 0 


