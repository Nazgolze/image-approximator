#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

// macro definitions
#define ERROR -1
#define SUCCESS 0

#define printfe(...) printfl(IA_ERR, ##__VA_ARGS__);
#define printfd(...) printfl(IA_DEBUG, ##__VA_ARGS__);
#define printfi(...) printfl(IA_INFO, ##__VA_ARGS__);

#define streq(str1,str2) !strcmp(str1,str2)

#define GEN_SIZE ia_cfg.num_sets
#define OUTPUT_PATH "output"

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
	struct img_bitmap *img;
	int64_t my_index;
	int64_t father_index;
	int64_t mother_index;
};

struct img_bitmap {
	ALLEGRO_BITMAP *bmp;
	int64_t score;
};

enum ia_actions {
	IA_RESIZE,
	IA_MOVE,
	IA_RECOLOR
};

enum ia_user_actions {
	IA_USER_NONE = 0,
	IA_USER_MUTATE,
	IA_USER_SAVE,
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

enum ia_print_level {
	IA_INFO,
	IA_ERR,
	IA_DEBUG,
};

// Settings
struct ia_cfg_st {
	int screen_width; /**< The screen width */
	int screen_height; /**< The screen height */

	struct img_bitmap *reference_image;
	struct img_bitmap *best_image;
	enum ia_print_level print_level;

	int num_circles;
	int num_sets; /**< # sets of circles */
	int num_init; /**< # sets to init */
	int mutation; /**< How often to perform a mutation */
	uint64_t cur_gen; /**< Current generation */
	int64_t cur_gen_score; /**< Generation[0] score */

	FILE *log;

	// Internal settings
	bool quit;
	enum ia_user_actions action;

	struct ia_circles *seed1;
	struct ia_circles *seed2;
	struct ia_circles *seed3;
	struct ia_circles *seed4;
	struct ia_circles *best;
	struct ia_circles *ret;
};
struct ia_cfg_st ia_cfg;

// function declarations
uint32_t get_rand(void);
void start_time(struct timespec *);
char *end_time(struct timespec *, struct timespec *, char *, ...);
void ia_random_action(struct ia_circle *);

int printfl(enum ia_print_level, const char *fmt, ...);
void strip_newline(char *);

void ia_cfg_init();
void ia_cfg_print();
void ia_cfg_free();
int ia_cfg_read(const char *);
#endif
