#!/bin/bash

if [ $# -eq 1 ];
then
    # Your code here. (1/4)
	grep -E "WARN|ERROR" mos.log > bug.txt
else
    case $2 in
    "--latest")
        # Your code here. (2/4)
	tail -n 5 mos.log 
    ;;
    "--find")
        # Your code here. (3/4)
	str=$3
	touch $3.txt
	grep  $3 $1 > $3.txt
    ;;
    "--diff")
        # Your code here. (4/4)
	diff -s $1 $3 > /dev/null
	if [ $? -eq 0 ]
	then
		echo same
	else
		echo different
	fi 
    ;;
    esac
fi
