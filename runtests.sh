#!/bin/sh

export LD_LIBRARY_PATH=lib/
for i in `ls tests`
do
    ./tests/$i
done		

