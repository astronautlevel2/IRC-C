CC=gcc-6

all:
	$(CC) -o IRC src/*.c -Wall -Wextra -pedantic -lncurses
