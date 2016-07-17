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
//#include "circle.h"


#include "common.h"
#include "circle.h"
#include "image.h"
#include "ga.h"
#include "console.h"

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
	printfi("%s\n", end_time_str);
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

void _print_input_help()
{
	printf("mutate: Enter mutation phase\n");
	printf("save: Save the current image and state\n");
	printf("--------------------------\n");
	printf("help: Show the input help\n");
	printf("exit or quit: Exit the program\n");

}

void *_user_input(void *arg)
{
	char line[512];
	console_init();
	while(1) {
		console_draw(NULL);
		if(fgets(line, sizeof(line), stdin)) {
			strip_newline(line);
			if(!strlen(line)) {
				continue;
			}
			if(streq(line, "quit") || streq(line, "exit")) {
				printf("Exiting the program, please wait....\n");
				ia_cfg.quit = true;
				return NULL;
			} else if(streq(line, "help")) {
				_print_input_help();
			} else if(streq(line, "mutate")) {
				ia_cfg.action = IA_USER_MUTATE;
			} else if(strstr(line, "save")) {
				ia_cfg.action = IA_USER_SAVE;
			}
		}
	}
	return NULL;
}

void *_run(void *arg)
{
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	glutInitWindowSize(ia_cfg.screen_width, ia_cfg.screen_height);
	glutCreateWindow("Image Approximator");
	glViewport(0, 0, ia_cfg.screen_width, ia_cfg.screen_height);
	printfi("%s\n", glGetString(GL_VERSION));
	glutDisplayFunc(render);
	glutKeyboardFunc(handle_keys);
	glutIdleFunc(render);
	glutReshapeFunc(resize);

	init();
	printfi("Num circles: %d\n", ia_cfg.num_circles);
	init_ga(ia_cfg.num_circles);
	struct ia_circles *awesome = do_ga();
	printf("%ld\n", awesome->img->score);
	ia_cfg_free();
	return NULL;
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
	if(nice(10) == -1) {
		printf("Error being nice\n");
	}
	if(!al_init()) {
		printfe("failed to initialize allegro!\n");
		return ERROR;
	}
	if(!al_init_image_addon()) {
		printfe("failed to initialize allegro image library!\n");
		return ERROR;
	}
	ia_cfg_init();

        // Create the output directory
        struct stat statbuf;
        if(stat(OUTPUT_PATH, &statbuf) != 0) {
                if(mkdir(OUTPUT_PATH,
                        S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP) < 0) {
                        printfe("Failed to created output directory %s",
                                OUTPUT_PATH);
                        return ERROR;
                }
        }

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

	// Create 2 threads
	pthread_t th_input, th_run;
	pthread_create(&th_input, NULL, _user_input, NULL);
	pthread_create(&th_run, NULL, _run, NULL);

	pthread_join(th_input, NULL);
	pthread_join(th_run, NULL);
	return SUCCESS;
}
