CC=gcc
CFLAGS=-g -Wall

targets=cachesim

all: $(targets)

cachesim: cachesim.c 
	$(CC) $(CFLAGS) $@.c -o $@ -lm

clean: 
	rm $(targets) *.o *.*out *.csv
