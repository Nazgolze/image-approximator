#define GL_GLEXT_PROTOTYPES
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>

#include "circle.h"
#include "image.h"

#define COMPARE_VAL(a, b) \
	do { \
		if(a < b) \
			return -1; \
		if(a > b) \
			return 1; \
	} while(0)

static int _circle_compare(const struct ia_circle *c1,
    const struct ia_circle *c2)
{
	COMPARE_VAL(c1->radius, c2->radius);

	// Color
	COMPARE_VAL(c1->color.r, c2->color.r);
	COMPARE_VAL(c1->color.g, c2->color.g);
	COMPARE_VAL(c1->color.b, c2->color.b);
	COMPARE_VAL(c1->color.a, c2->color.a);

	// Position
	COMPARE_VAL(c1->x, c2->x);
	COMPARE_VAL(c1->y, c2->y);

	return 0;
}

static void _draw_circles(struct ia_circles *circles)
{
	int idx, jdx;
	float x1,y1,x2,y2;
	double radius;

	for(idx = 0; idx < circles->num_circles; idx++) {
		glBegin(GL_POLYGON);
		x1 = (float)circles->circles[idx].x;
		y1 = (float)circles->circles[idx].y;
		glColor4ub(circles->circles[idx].color.r,
		    circles->circles[idx].color.g,
		    circles->circles[idx].color.b,
		    circles->circles[idx].color.a);
		radius = (double)circles->circles[idx].radius;
		float theta = 0;
		float theta2 = 6 * 3.14159265359 / 180;

		for(jdx = 0; jdx < 60; jdx++) {
			x2 = x1 + radius*sin(theta + theta2);
			y2 = y1 + radius*cos(theta + theta2);
			theta += theta2;
			glVertex2f(x2,y2);
		}
#if 0
		for(angle = 1.0f; angle < 361.0f; angle += 1.71) {
			x2 = x1 + sin(angle) * radius;
			y2 = y1 + cos(angle) * radius;
			glVertex2f(x2,y2);
		}
#endif
		glEnd();
	}

}

static void _render(struct ia_circles *l_circles)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
#if 0	
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
#endif
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	_draw_circles(l_circles);

	glDisable(GL_BLEND);
	glFlush();
}

void refresh_circles(struct ia_circles *l_circles)
{
	if(ia_cfg.quit) {
		return;
	}
	img_free(l_circles->img);
	_render(l_circles);
	l_circles->img = img_from_GL();
	img_assign_score(l_circles->img, ia_cfg.reference_image);
}


void init_circles(struct ia_circles *circles,
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

	for(int ix = 0; ix < circles->num_circles; ix++) {
		circles->circles[ix].x = get_rand() % ia_cfg.screen_width;
		circles->circles[ix].y = get_rand() % ia_cfg.screen_height;
		circles->circles[ix].color.r = get_rand() % 255;
		circles->circles[ix].color.g = get_rand() % 255;
		circles->circles[ix].color.b = get_rand() % 255;
		circles->circles[ix].color.a = get_rand() % 50;
		circles->circles[ix].radius = get_rand() % 5 + 5;
	}
	circles->my_index = -1;
	circles->father_index = -1;
	circles->mother_index = -1;
	
	_render(circles);
	circles->img = img_from_GL(); 

	return;
}

void sort_circles(struct ia_circles *circles)
{
	if(ia_cfg.quit) {
		return;
	}
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

struct ia_circles *clone_circles(struct ia_circles *c)
{
	struct ia_circles *ret_circles = NULL;
	if(!c) {
		return NULL;
	}

	ret_circles = calloc(1, sizeof(struct ia_circles));
	ret_circles->num_circles = c->num_circles;
	ret_circles->img = img_clone(c->img);
	ret_circles->circles = calloc(c->num_circles,
	    sizeof(struct ia_circle));
	memcpy(ret_circles->circles, c->circles,
	    ret_circles->num_circles * sizeof(struct ia_circle));
	ret_circles->my_index = c->my_index;
	ret_circles->father_index = c->father_index;
	ret_circles->mother_index = c->mother_index;

	return ret_circles;
}

void free_circles(struct ia_circles *c)
{
	if(!c) {
		return;
	}
	free(c->circles);
	img_free(c->img);
}

#if 0
const char *circles_to_svg(struct ia_circles *circles);
#endif
