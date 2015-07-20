CC=gcc
CFLAGS=-O2 -pipe -D_REENTRANT -std=gnu11 -g -Wall -Wunused-variable -Wuninitialized -pedantic
#CFLAGS=-pipe -D_REENTRANT -std=gnu11 -g -Wall -Wunused-variable -Wuninitialized -pedantic

#LDFLAGS=-lSDL2_image -lSDL2_gfx -lSDL2 -lpthread
#INCFLAGS=-I/usr/include/SDL2

INCFLAGS=-I/usr/include/libdrm
LDFLAGS=-lGLU -lGL -lglut -lallegro -lallegro_image -lbsd -lm -lpthread

OBJECTS=main.o common.o circle.o image.o ga.o console.o

all: main

main: $(OBJECTS) Makefile
	$(CC) $(CFLAGS) $(INCFLAGS) $(OBJECTS) -fPIC $(LDFLAGS) -o main

%.o: %.c Makefile
	$(CC) $(CFLAGS) -fPIC -lc $(INCFLAGS) -c $<

clean:
	(rm -f *.o main)
