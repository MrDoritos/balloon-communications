#!/bin/bash
ARGS=' '
LIBS='-lpthread -lncurses'
FILES='main.cpp'
INCLUDE='-I./'
ERROROUT="/tmp/$RANDOM.txt"
FILE='a.out'
BINARY="-o $FILE"
g++ $ARGS $LIBS $FILES $INCLUDE $BINARY $@ 2> $ERROROUT
if [ $? == 0 ]
	then
	echo No Errors!
	echo g++ -\> $FILE
	else
	less $ERROROUT
fi

