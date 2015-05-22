#define GL_GLEXT_PROTOTYPES
#define _GNU_SOURCE
#include <sys/param.h>

#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>
//#include "circle.h"


#include "common.h"
#include "circle.h"
#include "image.h"

#define ONE_TIME printf("%s-%d\n", __FUNCTION__, __LINE__); fflush(stdout);

static int global_idx = 1;
static bool ready = false;
static bool first_render = true;

//static bool is_written = false, rerender = false;

static struct option _long_options[] = {
	{"circles", required_argument, 0, 'c'},
	{"image", required_argument, 0, 'i'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
};

static void draw_circles(struct ia_circles *circles)
{
	int idx, jdx;
	float x1,y1,x2,y2;
	double radius;

	for(idx = 0; idx < circles->num_circles; idx++) {
		glBegin(GL_POLYGON);
		x1 = (float)circles->circles[idx].x;
		y1 = (float)circles->circles[idx].y;
		glColor3ub(circles->circles[idx].color.r,
		    circles->circles[idx].color.g,
		    circles->circles[idx].color.b);
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
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
	draw_circles(l_circles);

	glDisable(GL_BLEND);

	glFlush();
}

static struct img_bitmap *_img = NULL, *_img_prev = NULL;
static int _circle_index = 0;
static int _max_allow = 0;
static void render(void)
{
	char *end_time_str;
	struct timespec tp1, tp2;
	int error = SUCCESS;
	
	if(first_render) {
		_img = img_from_GL();
		img_assign_score(_img, reference_image);
		_img_prev = img_clone(_img);
		first_render = false;
		return;
	}
	start_time(&tp1);
	// stochastic hill climbing algorithm
	img_free(_img);
	ia_random_action(&circles.circles[_circle_index]);
	_render(&circles);
	_img = img_from_GL();
	img_assign_score(_img, reference_image);

	printf("num_circles = %lu\n", circles.num_circles);
	printf("_img->score: %ld\n", _img->score);
	printf("_img_prev->score: %ld\n", _img_prev->score);
	printf("diff: %ld\n", _img->score - _img_prev->score);
	if((_img->score - _img_prev->score) > _max_allow) {
		_max_allow++;
		_max_allow = MIN(_max_allow, circles.num_circles);
		img_free(_img);
		_img = img_clone(_img_prev);
		printf("rejecting change for circle[%d]\n", _circle_index);
		memcpy(&circles.circles[_circle_index], &circles_prev.circles[_circle_index],
		    sizeof(struct ia_circle));
	} else {
		_max_allow = 0;
		img_free(_img_prev);
		_img_prev = img_clone(_img);
		memcpy(&circles_prev.circles[_circle_index], &circles.circles[_circle_index],
		    sizeof(struct ia_circle));
		char *fn;
		error = asprintf(&fn, "image%d.png", global_idx);
		if(error == ERROR) {
			printfe("%s", strerror(errno));
			abort();
		}
		//al_save_bitmap(fn, _img->bmp);
		free(fn);
		global_idx++;
		_circle_index++;
		_circle_index %= circles.num_circles;
	}

	end_time_str = end_time(&tp1, &tp2, "time");
	printf("%s\n", end_time_str);
	free(end_time_str);
}

static void handle_keys(unsigned char key, int x, int y)
{
	if(key == 'q') {
#if 0
		glDeleteVertexArrays(3,vao);
		glDeleteProgram(p);
		glDeleteShader(v);
		glDeleteShader(f);
#endif
		exit(0);
	} else if(key == 'r') {
		ready = !ready;
	}
}

static void resize(int width, int height)
{
	glutReshapeWindow(screen_width, screen_height);
}

static void init(void)
{
	GLfloat w = screen_width;
	GLfloat h = screen_height;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0, w, 0, h, -1, 1);

	glViewport(0, 0, w, h);

}

int main(int argc, char **argv)
{
	print_level = IA_DEBUG;
	if(!al_init()) {
		printfe("failed to initialize allegro!\n");
		return ERROR;
}

	if(!al_init_image_addon()) {
		printfe("failed to initialize allegro image library!\n");
		return ERROR;
	}
	int c, num_circles = 0;
	while(true) {
		int option_index = 0;
		//int error;
		c = getopt_long(argc, argv, "hi:c:", _long_options, &option_index);

		if(c == -1)
			break;
		switch(c) {
		case 'h':
			//_print_help();
			return 0;
		case 'c':
			num_circles = strtol(optarg, NULL, 10);
			break;
		case 'i':
			reference_image = img_load((const char *)optarg);
			if(!reference_image) {
				printfe("unable to load reference image\n");
				return ERROR;
			}
			screen_width = al_get_bitmap_width(reference_image->bmp);
			screen_height = al_get_bitmap_height(reference_image->bmp);
			//_show_intermediate = true;
			break;
		default:
			return 0;
		}
	}
	if(num_circles == 0) {
		num_circles = screen_width * screen_height / 10 * 5;
	}

	init_circles(&circles, IA_RANDOM, num_circles);
	init_circles(&circles_prev, IA_RANDOM, num_circles);
	memcpy(circles_prev.circles, circles.circles,
	    num_circles * sizeof(struct ia_circle));

	printf("screen_width = %d\nscreen_height = %d\n", screen_width, screen_height);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	glutInitWindowSize(screen_width, screen_height);
	glutCreateWindow("Image Approximator");
	glViewport(0, 0, screen_width, screen_height);
	printf("%s\n", glGetString(GL_VERSION));
	glutDisplayFunc(render);
	glutKeyboardFunc(handle_keys);
	glutIdleFunc(render);
	glutReshapeFunc(resize);

	init();

	glutMainLoop();

	return SUCCESS;
}
