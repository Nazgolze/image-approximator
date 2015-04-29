#ifndef CIRCLE_H
#define CIRCLE_H

#include <stdint.h>

#include <sys/param.h>

#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL_render.h>

#include "common.h"

enum c_actions {
	DF_RESIZE,
	DF_MOVE,
	DF_RECOLOR
};

enum c_color {
	DF_RED,
	DF_GREEN,
	DF_BLUE
};

enum c_direction {
	DF_LEFT,
	DF_RIGHT,
	DF_UP,
	DF_DOWN
};

struct cmp_result {
	int ix_begin;
	int jx_begin;
	int ix_end;
	int jx_end;
	int radius;
	uint64_t tdiff;
	uint64_t bdiff;
};

struct c_circle {
	int x;
	int y;
	int radius;
	SDL_Color color;
	SDL_Renderer *renderer;
};

struct c_circle_save {
	struct c_circle circle;
	int64_t score;
};

struct c_bitmap {
	ALLEGRO_BITMAP *bmp;
	int64_t score;
};

#endif
