CC=gcc-6

all:
	$(CC) -g -o IRC src/*.c -O0 -Wall -Wextra -pedantic -lncurses
