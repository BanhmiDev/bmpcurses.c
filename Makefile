CC=gcc
LDFLAGS=-lncurses -lm
CFLAGS=-std=gnu11

all:
	$(CC) bmpcurses.c bmpcurses.h -o bmpcurses $(LDFLAGS) $(CFLAGS)
