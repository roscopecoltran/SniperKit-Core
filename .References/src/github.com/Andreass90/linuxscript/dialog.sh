#!/bin/bash
dialog --menu "Välj din webläsare 10 60 2 Chrome "Webbläsaren Chrome" Safari "Webbläsaren safari" 2> /tmp/browser
Val=`cat /tmp/browser`
$Val
exit 0
