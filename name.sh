#!/bin/bash
read -p "Vad heter du? " Username #frågar vad du heter
read -p "Hur gammal är du? " Alder #frågar hur gammal du är
read -p "vart bor du? " Stad #frågar vart du bor
echo Hej $Username du är $Alder år och bor i $Stad # ta infon ifrån det andra och berättar för dig
exit 0
