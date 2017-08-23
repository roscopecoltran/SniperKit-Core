#!/bin/bash

## BASH IO

#Yes it is possible, just redirect the output to a file:
someCommand > someFile.txt  

#Or if you want to append data:
someCommand >> someFile.txt

#If you want stderr too use this:
someCommand &> someFile.txt  

#or this to append:
someCommand &>> someFile.txt  