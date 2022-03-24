#!/bin/sh
# i hate makefiles
[ -e "c8" ] && rm c8

cc -o c8 src/c8.c $(pkg-config allegro-5 allegro_primitives-5 --libs --cflags) -Iinclude/ -g -Wall -std=c99
