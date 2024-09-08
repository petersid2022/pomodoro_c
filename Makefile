.PHONY: all clean

CC = cc
CFLAGS = -static -pedantic -std=c99 -Wall -Wextra -pedantic -Wunused-result -Wno-unused-variable
OPTFLAGS = -O3
DEBUGFLAGS = -O0

all: main main.s

main: main.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -o main main.c

main.s: main.c
	$(CC) $(CFLAGS) $(DEBUGFLAGS) -S -o main.s main.c

clean:
	rm -f main main.s
