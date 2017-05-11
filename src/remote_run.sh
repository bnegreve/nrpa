#!/bin/bash


NRPA_PATH=$1
shift
BIN=$1
shift
REST=$*

echo "Moving to $NRPA_PATH"
echo "Running $BIN $REST"

cd $NRPA_PATH
TMP=$(mktemp)
./$BIN $REST > $TMP
TIMERSTATS=$(cat $TMP | grep "Timer stats filename" | cut -d ':' -f 2)
ITERSTATS=$(cat $TMP | grep "Iter stats filename" | cut -d ':' -f 2)
cat $TMP
if [ ! -z $TIMERSTATS ] && [ -f $TIMERSTATS ]; then
    echo "Timer stats: ";
    cat $TIMERSTATS | awk '{ print "timer_stats: ", $0 }';
fi 
if [ ! -z $ITERSTATS ] && [ -f $ITERSTATS ]; then
    echo "Iter stats: "; 
    cat $ITERSTATS | awk '{ print "iter_stats: ", $0 }';
fi 

rm $TMP
