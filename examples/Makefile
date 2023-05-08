CC = clang
CFLAGS = -Wall -Wextra -I..

all: main

main: main.c ../api.h
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -rf main
