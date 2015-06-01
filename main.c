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
#include "ga.h"

#define ONE_TIME printf("%s-%d\n", __FUNCTION__, __LINE__); fflush(stdout);

static bool ready = false;

//static bool is_written = false, rerender = false;

static struct option _long_options[] = {
	{"circles", required_argument, 0, 'c'},
	{"image", required_argument, 0, 'i'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
};

static void render(void)
{
	return;
	char *end_time_str;
	struct timespec tp1, tp2;
	//int error = SUCCESS;
	
	start_time(&tp1);

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
	reference_image = NULL;
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
	if(!reference_image) {
		abort();
	}
	if(num_circles == 0) {
		num_circles = screen_width * screen_height / 10 * 5;
	}
#if 0
	init_circles(&circles, IA_RANDOM, num_circles);
	init_circles(&circles_prev, IA_RANDOM, num_circles);
	memcpy(circles_prev.circles, circles.circles,
	    num_circles * sizeof(struct ia_circle));
#endif
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

	init_ga(num_circles, 500);
	struct ia_circles *awesome = do_ga();
	printf("%ld\n", awesome->img->score);
	return SUCCESS;
	glutMainLoop();

	return SUCCESS;
}
