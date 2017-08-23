#!/bin/bash
if [ -f $1 ]; then #variabel för den filen du skriver in och kollar om det är en vanlig fil
    echo "$1 är en vanlig fil" #berättar om det är en vanlig fil
elif [ -b $1 ]; then #kontrollerar om filen är en blockdevice
    echo "$1 är en block device" #berättar om filen är en blockdevice
elif [ -h $1 ]; then #kontrollerar om filen är en länk
    echo "$1 är en länk" #berättar om filen är en länk
elif [ -d $1 ]; then #kontrollerar om filen är en katalog
    echo "$1 är en katalog" # berättar om filen är en katalog
else #annars
    echo "filen existerar inte" #berättar att filen inte finns
fi
exit 0
