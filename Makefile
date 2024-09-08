# Define the compiler and flags
CC = cc
CFLAGS = -Wall -Wextra -pedantic -Wunused-result -Wno-unused-variable
OPTFLAGS = -O2
DEBUGFLAGS = -O0

# Define the targets
.PHONY: all clean

all: main main.s

main: main.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -o main main.c

main.s: main.c
	$(CC) $(CFLAGS) $(DEBUGFLAGS) -S -o main.s main.c

clean:
	rm -f main main.s
