CC=gcc
CFLAGS=-O2 -pipe -D_REENTRANT -std=gnu11 -g -Wall -Wunused-variable -Wuninitialized -pedantic

LDFLAGS=-lSDL2_image -lSDL2_gfx -lSDL2 -lpthread
INCFLAGS=-I/usr/include/SDL2

OBJECTS=main.o common.o

all: main 

main: $(OBJECTS) Makefile
	$(CC) $(CFLAGS) $(INCFLAGS) $(LDFLAGS) $(OBJECTS) -fPIC -o main

%.o: %.c Makefile
	$(CC) $(CFLAGS) -fPIC -lc $(INCFLAGS) -c $<

clean:
	(rm -f *.o main)
