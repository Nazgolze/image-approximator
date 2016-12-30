#define GL_GLEXT_PROTOTYPES
#define _GNU_SOURCE
#include <sys/param.h>
#include <sys/stat.h>

#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>

#include "common.h"
#include "image.h"

static int width, height;
static struct option _long_options[] = {
	{"base", required_argument, 0, 'b'},
	{"image", required_argument, 0, 'i'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
};

static void _print_help(
	int argc,
	char **argv
	)
{
	if(!argc) {
		abort();
	}
	printf("Usage: %s [-b <img>] [-i <img>]\n\n", argv[0]);
	printf("Optional Argument:\n");
	printf("-b, --base <img>: Source image\n");
	printf("-i, --image <img>: Image to compare against\n");
}

static void render(void)
{
	return;
}

static void resize(int width, int height)
{
	//glutReshapeWindow(ia_cfg.screen_width, ia_cfg.screen_height);
}

static void handle_keys(unsigned char key, int x, int y)
{
	return;
}

static void init(void)
{
	GLfloat w = width;
	GLfloat h = height;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1, 1);

	glViewport(0, 0, w, h);
}

int main(int argc, char **argv)
{
	if(!al_init()) {
		printfe("failed to initialize allegro!\n");
		return ERROR;
	}
	if(!al_init_image_addon()) {
		printfe("failed to initialize allegro image library!\n");
		return ERROR;
	}

	int c;

	struct img_bitmap *base, *image;

	while(true) {
		int option_index = 0;
		c = getopt_long(argc, argv, "hi:b:", _long_options, &option_index);
		if(c == -1) {
			break;
		}
		struct img_bitmap **img_ptr;
		switch(c) {
			case 'h':
				_print_help(argc, argv);
				return 0;
			case 'b':
			case 'i':
				if(c == 'b') {
					img_ptr = &base;
				} else {
					img_ptr = &image;
				}
				*img_ptr = img_load((const char *)optarg);
				if(!*img_ptr) {
					printf("unable to load reference image\n");
					return ERROR;
				}
				width = al_get_bitmap_width((*img_ptr)->bmp);
				height = al_get_bitmap_height((*img_ptr)->bmp);
				//_show_intermediate = true;
				break;
			default:
				break;
		}
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	glutInitWindowSize(width, height);
	glutCreateWindow("Image Approximator");
	glViewport(0, 0, width, height);
	glutDisplayFunc(render);
	glutKeyboardFunc(handle_keys);
	glutIdleFunc(render);
	glutReshapeFunc(resize);

	init();
	ia_cfg.screen_width = width;
	ia_cfg.screen_height = height;
	img_assign_score(image, base);
	printf("%ld\n", image->score);

	return 0;
}
