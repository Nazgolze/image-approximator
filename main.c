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
	{"file", required_argument, 0, 'f'},
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
	glutReshapeWindow(ia_cfg.screen_width, ia_cfg.screen_height);
}

static void init(void)
{
	GLfloat w = ia_cfg.screen_width;
	GLfloat h = ia_cfg.screen_height;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0, w, 0, h, -1, 1);

	glViewport(0, 0, w, h);

}

/**
 * Print the command help
 *
 * @param argc Number of arguments
 * @param argv The arguments array
 */
static void _print_help(
	int argc,
	char **argv
	)
{
	if(!argc) {
		abort();
	}
	printf("Usage: %s [-i <img>] [-c <num>] [-f <config>]\n\n", argv[0]);
	printf("Optional Argument:\n");
	printf("-i, --image <img>: Source image\n");
	printf("-c, --circles <num>: Number of circles to use\n");
	printf("-f, --file <config>: Config file to use\n");
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
	ia_cfg_init();

	int c;
	while(true) {
		int option_index = 0;
		c = getopt_long(argc, argv, "hi:c:f:", _long_options, &option_index);
		if(c == -1) {
			break;
		}
		switch(c) {
			case 'h':
				_print_help(argc, argv);
				return 0;
			case 'c':
				ia_cfg.num_circles = strtol(optarg, NULL, 10);
				break;
			case 'f':
				ia_cfg_read(optarg);
				break;
			case 'i':
				ia_cfg.reference_image = img_load((const char *)optarg);
				if(!ia_cfg.reference_image) {
					printfe("unable to load reference image\n");
					ia_cfg_free();
					return ERROR;
				}
				ia_cfg.screen_width = al_get_bitmap_width(ia_cfg.reference_image->bmp);
				ia_cfg.screen_height = al_get_bitmap_height(ia_cfg.reference_image->bmp);
				//_show_intermediate = true;
				break;
			default:
				break;
		}
	}
	if(!ia_cfg.reference_image) {
		_print_help(argc, argv);
		ia_cfg_free();
		return 1;
	}
	if(ia_cfg.num_circles == 0) {
		ia_cfg.num_circles = ia_cfg.screen_width * ia_cfg.screen_height / 10 * 5;
	}
	ia_cfg_print();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	glutInitWindowSize(ia_cfg.screen_width, ia_cfg.screen_height);
	glutCreateWindow("Image Approximator");
	glViewport(0, 0, ia_cfg.screen_width, ia_cfg.screen_height);
	printf("%s\n", glGetString(GL_VERSION));
	glutDisplayFunc(render);
	glutKeyboardFunc(handle_keys);
	glutIdleFunc(render);
	glutReshapeFunc(resize);

	init();

	init_ga(ia_cfg.num_circles, ia_cfg.num_init);
	struct ia_circles *awesome = do_ga();
	printf("%ld\n", awesome->img->score);
	ia_cfg_free();
	return SUCCESS;

	// FIXME: What is this for. It will never be run??
	glutMainLoop();
	return SUCCESS;
}
