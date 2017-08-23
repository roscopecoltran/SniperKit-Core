!/bin/bash
dialog --backtitle "Webbrowser" --title "Vill du välja webbläsare?" --yesno "Vill du välja webbläsare?" 10 60 #Ger en dialog som ger dig ett val 
if [ $? -eq 0 ]; then #Om du väljer ja
	dialog --menu "Välj din webbläsare" 10 60 3 chromium-browser "Webbläsaren Chrome" firefox "Webbläsaren Firefox" lynx "Webbläsaren Lynx" 2>/tmp/browser
	Val=`cat /tmp/browser` # Ger en meny där du kan välja webbläsare 
	$Val
else #Om du väljer nej
	echo "Då avslutar vi" #Berättar att vi avslutar
fi
exit 0
