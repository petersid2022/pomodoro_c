.PHONY: all clean

CC = cc
CFLAGS = -std=c99 -Wall -Wextra -pedantic -Wunused-result -Wno-unused-variable
DEBUGFLAGS = -O0 -g -S -s

all: main main.s
default: main

main: main.c
	$(CC) $(CFLAGS) -o main main.c

main.s: main.c
	$(CC) $(CFLAGS) $(DEBUGFLAGS) -o main.s main.c

clean:
	rm -f main main.s
