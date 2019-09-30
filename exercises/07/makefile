CC = gcc
LIB_DIR=/home/alex/Documents/learning/linux_programming_interface/exercises/lib
CFLAGS = -Wall -g -I$(LIB_DIR)
OBJ = $(LIB_DIR)/error_functions.o $(LIB_DIR)/get_num.o
BUILD = $(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

mmalloc: $(OBJ) mmalloc.c
	$(BUILD)

free_and_sbrk: $(OBJ) free_and_sbrk.c
	$(BUILD)
