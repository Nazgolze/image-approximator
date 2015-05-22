#include <stdlib.h>
#include <stdbool.h>
#include "circle.h"

static int _circle_compare(const struct ia_circle *c1,
    const struct ia_circle *c2)
{
	if(c1->x < c2->x) {
		return -1;
	} else if(c1->x > c2->x) {
		return 1;
	}

	if(c1->y < c2->y) {
		return -1;
	} else if(c1->y > c2->y) {
		return 1;
	}

	if(c1->radius < c2->radius) {
		return -1;
	} else if(c1->radius > c2->radius) {
		return 1;
	}

	if(c1->color.r < c2->color.r) {
		return -1;
	} else if(c1->color.r > c2->color.r) {
		return 1;
	}

	if(c1->color.g < c2->color.g) {
		return -1;
	} else if(c1->color.g > c2->color.g) {
		return 1;
	}

	if(c1->color.b < c2->color.b) {
		return -1;
	} else if(c1->color.b > c2->color.b) {
		return 1;
	}

	return 0;
}

void init_circles(struct ia_circles *circles, int start,
    uint64_t number_circles)
{
	//num_circles = screen_width * screen_height / 10 * 5;
	struct ia_circle *circle;
	circle = calloc(number_circles, sizeof(struct ia_circle));

	if(!circle) {
		printfd("memory allocation error");
		memset(circles, 0, sizeof(struct ia_circles));
		return;
	}
	circles->circles = circle;
	circles->num_circles = number_circles;

	if(start == IA_RANDOM) {
		for(int ix = 0; ix < circles->num_circles; ix++) {
			circles->circles[ix].x = get_rand() % screen_width;
			circles->circles[ix].y = get_rand() % screen_height;
			circles->circles[ix].color.r = get_rand() % 5;
			circles->circles[ix].color.g = get_rand() % 5;
			circles->circles[ix].color.b = get_rand() % 5;
			circles->circles[ix].radius = get_rand() % 5 + 5;
		}
	}

	return;
}

void sort_circles(struct ia_circles *circles)
{
	if(!circles) {
		printfd("circles is null");
		return;
	}
	if(circles->num_circles == 0) {
		return;
	}
	if(!circles->circles) {
		printfd("circles->circles is null");
		return;
	}

	qsort(circles->circles, circles->num_circles, sizeof(struct ia_circle),
	    (__compar_fn_t)_circle_compare);
}

#if 0
const char *circles_to_svg(struct ia_circles *circles);
#endif
