#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

// macro definitions
#define ERROR -1
#define SUCCESS 0

#define printfe(...) fprintf(stderr, ##__VA_ARGS__); fflush(stderr);

#ifdef DEBUG
#define printfd(fmt, ...) printf("%s-%d: " fmt "\n", __FUNCTION__, __LINE__,  ##__VA_ARGS__)
#else
#define printfd(fmt, ...)
#endif


// data definitions
struct ia_color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct ia_circle {
	int x;
	int y;
	int radius;
	struct ia_color color;
};

struct ia_circles {
	struct ia_circle *circles;
	uint64_t num_circles;
};

enum ia_actions {
	IA_RESIZE,
	IA_MOVE,
	IA_RECOLOR
};

enum ia_color_enum {
	IA_RED,
	IA_GREEN,
	IA_BLUE
};

enum ia_direction {
	IA_LEFT,
	IA_RIGHT,
	IA_UP,
	IA_DOWN
};

enum ia_circle_start {
	IA_NULL,
	IA_RANDOM
};

// global variables
int screen_width, screen_height;
struct img_bitmap *reference_image, *best_image;
struct ia_circles circles, circles_prev;

// function declarations
uint32_t get_rand(void);
void start_time(struct timespec *);
char *end_time(struct timespec *, struct timespec *, char *, ...);
void ia_random_action(struct ia_circle *);
#endif
