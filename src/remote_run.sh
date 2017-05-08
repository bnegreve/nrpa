#!/bin/bash


NRPA_PATH=$1
shift
BIN=$1
shift
REST=$*

echo "Moving to $NRPA_PATH"
echo "Running $BIN $REST"




cd $NRPA_PATH
./$BIN $REST
