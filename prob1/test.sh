#!/bin/bash

gcc -Wall -g linked.c -o linkedDEBUG
for name in ./input*;
do
	valgrind  ./linkedDEBUG name &>my.out
	memOut=[sed -n 12p my.out]
	if [["$memOut" = "==4255== All heap blocks were freed -- no leaks are possible"]];
	then
		echo No memory errors for test $name
	else 
		echo Memory error found for test $name
	fi
done

rm linkedDEBUG

