CC = clang
CFLAGS = -Wall -Wextra -I..

all: curl_main html_main

curl_main: curl/main.c ../api.h
	$(CC) $(CFLAGS) -o ./curl/main $<

html_main: html/main.c ../api.h
	$(CC) $(CFLAGS) -o ./html/main $<

.PHONY: clean
clean:
	rm -rf ./curl/main ./html/main
