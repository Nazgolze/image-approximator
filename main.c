#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include <sys/param.h>

#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL_render.h>

//#include "circle.h"

#include "common.h"

#define ERROR -1
#define SUCCESS 0

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define ONE_TIME printf("%s-%d\n", __FUNCTION__, __LINE__); fflush(stdout);

int main(int argc, char **argv)
{
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
	return SUCCESS;
}
