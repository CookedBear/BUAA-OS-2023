#!/bin/bash
#First you can use grep (-n) to find the number of lines of string.
#Then you can use awk to separate the answer.
grep -n $2 $1 > k.txt
awk 'BEGIN{FS=":"}{print $1}' k.txt > $3
