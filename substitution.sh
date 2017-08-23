#!/bin/bash
Testing1="Det var en gång en  "
Testing2="som bodde i  "
Testing3="Han tycker det är roligt att "
read -p "Det var en gång en ? " Test1
read -p "som bodde i en ? " Test2
read -p "Han tycker det är roligt att ? " Test3
echo  $Testing1 $Test1 $Testing2 $Test2
echo $Testing3 $Test3

read -p "Gillade du din saga eller du vill börja om J|N: " Val;
if [ "$Val" = "J" ]; then
read -p "Det var en gång en ? " Test1
read -p "som bodde i en ? " Test2
read -p "Han tycker det är roligt att ? " Test3
echo  $Testing1 $Test1 $Testing2 $Test2
echo $Testing3 $Test3

echo "Tack för du lekte"
elif [ "$Val" = "N" ]; then
echo "Tack för du lekte"
fi
exit 0

