#define GL_GLEXT_PROTOTYPES
#include <sys/param.h>

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <getopt.h>

#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>
//#include "circle.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "common.h"

#define ERROR -1
#define SUCCESS 0

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 320

//#define NUM_CIRCLES 1
#define NUM_CIRCLES SCREEN_WIDTH * SCREEN_HEIGHT / 10 * 5

#define ONE_TIME printf("%s-%d\n", __FUNCTION__, __LINE__); fflush(stdout);

struct c_color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct c_circle {
	int x;
	int y;
	int radius;
	struct c_color color;
};

static uint32_t i = 1;
static bool direction = false;

static bool ready = false;

static bool is_written = false, rerender = false;

static struct c_circle circles[NUM_CIRCLES] = {0};


static struct option _long_options[] = {
	{"image", required_argument, 0, 'i'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
};


static void fix(unsigned char *image)
{
	int count = 0, idx;
	unsigned char temp_image[SCREEN_WIDTH * 3] = {0};

	for(idx = SCREEN_HEIGHT - 1; idx >= 0 && count < idx; idx--) {
		memcpy(&temp_image,
		    image + (SCREEN_WIDTH * 3 * count),
		    SCREEN_WIDTH * 3);
		memcpy(image + (SCREEN_WIDTH * 3 * count),
		    image + (SCREEN_WIDTH * 3 * idx),
		    SCREEN_WIDTH * 3);
		memcpy(image + (SCREEN_WIDTH * 3 * idx),
		    &temp_image,
		    SCREEN_WIDTH * 3);
		count++;
	}
	printf("idx = %d\ncount = %d\n", idx, count);
}

static void save_image_from_buffer(char *filename)
{
	glPixelStorei(GL_PACK_ALIGNMENT,1);

	unsigned char *image;
	unsigned int size = SCREEN_WIDTH * SCREEN_HEIGHT * 3;
	image = malloc(size);
	glReadBuffer(GL_FRONT_LEFT);
	//glReadBuffer(GL_BACK);
	//glReadBuffer(GL_FRONT_AND_BACK);
	glReadPixels(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
	    GL_RGB, GL_UNSIGNED_BYTE, image);
	fix(image);

	ALLEGRO_BITMAP *bmp;
	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
	bmp = al_create_bitmap(SCREEN_WIDTH, SCREEN_HEIGHT);

	al_set_target_bitmap(bmp);

	ALLEGRO_LOCKED_REGION *locked;
	locked = al_lock_bitmap(bmp, ALLEGRO_PIXEL_FORMAT_BGR_888,
	    ALLEGRO_LOCK_WRITEONLY);

	locked->data = image;
	al_unlock_bitmap(bmp);

	al_save_bitmap("image.png", bmp);

	FILE *write;
	write = fopen("image.data", "wb");
	fwrite(image, 1, size, write);
	fclose(write);

	al_destroy_bitmap(bmp);
}

static void draw_circles(void)
{
	int idx, jdx;
	float x1,y1,x2,y2;
	float angle;
	double radius;

	for(idx = 0; idx < NUM_CIRCLES; idx++) {
		glBegin(GL_POLYGON);
		x1 = (float)circles[idx].x;
		y1 = (float)circles[idx].y;
		glColor3ub(circles[idx].color.r,
		    circles[idx].color.g,
		    circles[idx].color.b);
		radius = (double)circles[idx].radius;
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

static void render(void)
{

	if(!is_written && rerender) {
		save_image_from_buffer(NULL);
		is_written = true;
	}

	switch(i) {
	case 1:
		direction = false;
		break;
	case MIN(SCREEN_WIDTH, SCREEN_HEIGHT):
		direction = true;
		break;
	}

	if(direction) {
		i -= 2;
	} else {
		i += 2;
	}

	if(i > MIN(SCREEN_WIDTH, SCREEN_HEIGHT)) {
		i = MIN(SCREEN_WIDTH, SCREEN_HEIGHT);
	} else if(i < 1) {
		i = 1;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);

#if 0
	glPointSize(10.0);
	glBegin(GL_POINTS);
	glColor3ub(i, 0, 0);
	glVertex2f(0, 0.5);
	glEnd();
#endif


	draw_circles();

#if 0
	glBegin(GL_TRIANGLES);
	glColor3ub(255, 0, 0);
	glVertex2f(-1, -1);
	qlVertex2f(1, -1);
	glVertex2f(0, 1);
	glEnd();
#endif
	glDisable(GL_BLEND);

	glutSwapBuffers();
	rerender = true;

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
	glutReshapeWindow(SCREEN_WIDTH, SCREEN_HEIGHT);
}

static void init(void)
{
	GLfloat w = SCREEN_WIDTH;
	GLfloat h = SCREEN_HEIGHT;
	GLfloat aspect_ratio = w / h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0, w, 0, h, -1, 1);

	glViewport(0, 0, w, h);

	for(int ix = 0; ix < NUM_CIRCLES; ix++) {
		circles[ix].x = get_rand() % SCREEN_WIDTH;
		circles[ix].y = get_rand() % SCREEN_HEIGHT;
		circles[ix].color.r = get_rand() % 5;
		circles[ix].color.g = get_rand() % 5;
		circles[ix].color.b = get_rand() % 5;
		circles[ix].radius = get_rand() % 5 + 5;
	}
}

int main(int argc, char **argv)
{

	int c;
	while(true) {
		int option_index = 0;
		c = getopt_long(argc, argv, "hi:", _long_options, &option_index);

		if(c == -1)
			break;
		switch(c) {
		case 'h':
			//_print_help();
			return 0;
		case 'i':
			printf("optarg = %s\n", (char *)optarg);
			return 0;
			//_show_intermediate = true;
			break;
		default:
			return 0;
		}
	}

	if(!al_init()) {
		printfe("failed to initialize allegro!\n");
		return ERROR;
}

	if(!al_init_image_addon()) {
		printfe("failed to initialize allegro image library!\n");
		return ERROR;
	}
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	glutCreateWindow("Image Approximator");
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	printf("%s\n", glGetString(GL_VERSION));
	glutDisplayFunc(render);
	glutKeyboardFunc(handle_keys);
	glutIdleFunc(render);
	glutReshapeFunc(resize);

	init();

	glutMainLoop();

#if 0
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Texture *texture = NULL;
	SDL_Surface *screen_surface = NULL;
	SDL_Event e;
	bool quit = false;

	if(SDL_Init(SDL_INIT_VIDEO)) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return ERROR;
	}
	window = SDL_CreateWindow("Image Approximator", 0,
	    0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if(!window) {
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		SDL_Quit();
		return ERROR;
	}

	screen_surface = SDL_GetWindowSurface(window);
	texture = SDL_CreateTextureFromSurface(renderer, screen_surface);

	//SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_ADD);

	while(!quit) {
		SDL_PollEvent(&e);
		if(e.type == SDL_KEYUP) {
			quit = true;
		}
		//_randomize_rect(&rect);
		//filledCircleColor(renderer, get_rand() % SCREEN_WIDTH, get_rand() % SCREEN_HEIGHT,
		//    get_rand() % (MAX(SCREEN_WIDTH, SCREEN_HEIGHT) / 3), get_rand());
		filledCircleColor(renderer, 320, 240, 300, get_rand());
		//SDL_FillRect(screen_surface, &rect, SDL_MapRGB(screen_surface->format, get_rand() % 128, get_rand() % 128, get_rand() % 128));


	        //SDL_RenderClear(renderer);
	        //SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		sleep(1);
		//SDL_UpdateWindowSurface(window);
	}
	SDL_DestroyWindow(window);
	SDL_Quit();
	//printf("Hello World\n");
#endif
	return SUCCESS;
}
