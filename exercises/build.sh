#!/bin/bash

# Usage: ./build.sh {file.c} {exec_name}

# Update this to point to the directory with library files.
LIB_DIR=/home/alex/Documents/learning/linux_programming_interface/exercises/lib/

gcc $@ $LIB_DIR/error_functions.o $LIB_DIR/get_num.o -g -Wall -I$LIB_DIR 
