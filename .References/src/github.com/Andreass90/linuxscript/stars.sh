#!/bin/bash
function stars {
until [ $Num -lt 0 ]; do
echo -ne "*"
((Num--))
done
}
read -p "Hur många stjärnor vill du ha? " Num
stars
echo "
Klart"
exit 0
