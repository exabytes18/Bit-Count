CC:=gcc
OPTS:=-O3 -std=c99 -pedantic -Wall -Wextra -march=native

all: bitcount

bitcount: bitcount.c
	$(CC) $(OPTS) -o $@ $^

clean:
	-rm -rf bitcount

