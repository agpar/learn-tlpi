#!/bin/bash

LIB_DIR=/home/alex/Documents/learning/linux_programming_interface/exercises/lib/
if [ $# -eq 2 ]; then
    OUT=$2
else
    OUT=a.out
fi

gcc $1 $LIB_DIR/error_functions.o $LIB_DIR/get_num.o -Wall -I$LIB_DIR -o $OUT
