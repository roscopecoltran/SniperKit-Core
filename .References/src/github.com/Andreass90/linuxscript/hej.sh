#!/bin/bash
now=$(date +"%D %T") # variablen now är dator med daum och tid
git add -A 
git commit -m "$now" # commitar med kommentar med tid och datum
git push # pushar
exit 0
