#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "circle.h"

static int screen_width, screen_height;

#if 0
static void _unmap_rgb(SDL_Color *color, uint8_t *r, uint8_t *g, uint8_t *b)
{
	*r = color->r;
	*g = color->g;
	*b = color->b;
}

static SDL_Color _map_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	SDL_Color color = {0};
	color.r = r;
	color.g = g;
	color.b = b;

	return color;
}
#endif

static void c_init_circle(struct c_circle *circle, 
    int x, int y, int radius, struct c_color color) 
{
	circle->x = x;
	circle->y = y;
	circle->radius = radius;
	circle->color = color;
}

static void c_draw_filled_circle(struct c_circle *circle)
{
}

static void c_action(struct c_circle *circle)
{
	int factor = get_rand() % (MIN(screen_width, screen_height) / 2);
	//int factor = get_rand() % (MAX(screen_width, screen_height) * 2);
	int tmp; 
	enum c_actions action = get_rand() % 3;

	factor = MAX(factor, 1);

	int r_factor = get_rand() % 2 ? factor : -factor;
	unsigned char c_factor = get_rand() % 50;

	c_factor = MAX(c_factor, 1);

	switch(action) {
	case DF_MOVE:
		circle->undo.type = DF_MOVE;
		int dir = get_rand() % 4;
		switch(dir) {
		case DF_LEFT:
			circle->x = MAX(0, circle->x - factor);
			break;
		case DF_RIGHT:
			circle->x = MIN(screen_width, circle->x + factor);
			break;
		case DF_UP:
			circle->y = MAX(0, circle->y - factor);
			break;
		case DF_DOWN:
			circle->y = MIN(screen_height, circle->y + factor);
			break;
		}
		break;
	case DF_RESIZE:
		circle->radius += r_factor;
		circle->radius = MAX(1, circle->radius);
		tmp = MIN(screen_width, screen_height) / 1;
		circle->radius = MIN(tmp, circle->radius);
		break;
	case DF_RECOLOR:
		tmp = get_rand() % 3;
		unsigned char r,g,b, new;
		_unmap_rgb(circle->color, &r, &g, &b);
		switch(tmp) {
		case DF_RED:
			if(get_rand() % 2) {
				new = MIN(255, r + c_factor);
			} else {
				new = MAX(0, r - c_factor);
			}
			circle->color = _map_rgb(new, g, b); 
			break;
		case DF_GREEN:
			if(get_rand() % 2) {
				new = MIN(255, g + c_factor);
			} else {
				new = MAX(0, g - c_factor);
			}
			circle->color = _map_rgb(r, new, b); 
			break;
		case DF_BLUE:
			if(get_rand() % 2) {
				new = MIN(255, b + c_factor);
			} else {
				new = MAX(0, b - c_factor);
			}
			circle->color = _map_rgb(r, g, new); 
			break;
		}
	}      
}

static void init_c_bitmap_wh(struct c_bitmap **c_bmp, int width, int height)
{
	*c_bmp = malloc(sizeof(struct c_bitmap));
	(*c_bmp)->bmp = al_create_bitmap(width, height);
	(*c_bmp)->score = 0;
}

static void init_c_bitmap(struct c_bitmap **c_bmp)
{
	init_c_bitmap_wh(c_bmp, screen_width, screen_height);
}

// we score the image.  lower = better.
static void c_assign_score(struct c_bitmap *bmp, struct c_bitmap *ref)
{
	int ix, jx, diff = 0;
	ALLEGRO_COLOR bpc, rpc;
	int64_t total_difference = 0;
	for(ix = 0; ix < screen_width; ix++) {
		for(jx = 0; jx < screen_height; jx++) {
			unsigned char br, bg, bb, rr, rg, rb;
			bpc = al_get_pixel(bmp->bmp, ix, jx);
			rpc = al_get_pixel(ref->bmp, ix, jx);
			_unmap_rgb(bpc, &br, &bg, &bb);
			_unmap_rgb(rpc, &rr, &rg, &rb);
			diff = abs(br - rr) + 
						 abs(bg - rg) + 
						 abs(bb - rb);
			total_difference += diff;
		}
	}
	bmp->score = total_difference;
}

static void c_absolute_score(struct c_bitmap *bmp)
{
	int ix, jx, diff = 0;
	int64_t total = 0;
	ALLEGRO_COLOR bpc;
	for(ix = 0; ix < screen_width; ix++) {
		for(jx = 0; jx < screen_height; jx++) {
			unsigned char br, bg, bb;
			bpc = al_get_pixel(bmp->bmp, ix, jx);
			_unmap_rgb(bpc, &br, &bg, &bb);
			diff = br + bg + bb; 
			total += diff;
		}
	}
	bmp->score = total;
}

static struct c_bitmap *c_clone_bitmap(struct c_bitmap *bmp)
{
	struct c_bitmap *new_bmp
	    = malloc(sizeof(struct c_bitmap));
	new_bmp->score = bmp->score;
	new_bmp->bmp = al_clone_bitmap(bmp->bmp);
	return new_bmp;
}

static void free_c_bitmap(struct c_bitmap *bmp)
{
	al_destroy_bitmap(bmp->bmp);
	free(bmp);
}

int main(int argc, char **argv)
{
	display = NULL;
	struct c_bitmap *bmp, *test_bmp, *reference, *best;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	//ALLEGRO_JOYSTICK *joystick = NULL;
	ALLEGRO_TIMER *timer;

	struct c_circle circles[NUM_CIRCLES];
	memset(&circles, 0, sizeof(circles));

	char filename[100] = {0};

	bool exit = false;

	if(!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return ERROR;
	}
	al_set_new_display_flags(ALLEGRO_PROGRAMMABLE_PIPELINE);
	if(!al_init_image_addon()) {
		fprintf(stderr, "failed to initialize allegro image library!\n");
		return ERROR;
	}

	if(!al_install_keyboard()) {
		fprintf(stderr, "failed to initialize the keyboard!\n");
		return ERROR;
	}
#if 0
	if(!al_install_joystick()) {
		fprintf(stderr, "failed to initialize the joystick!\n");
	}
	al_reconfigure_joysticks();
	joystick = al_get_joystick(al_get_num_joysticks() - 1);
#endif
	
	reference = malloc(sizeof(struct c_bitmap));
	//reference->bmp = al_load_bitmap("67116_616011725802_4571816_n.jpg");
	reference->bmp = al_load_bitmap("sasuke.jpg");
	if(!reference->bmp) {
		printf("error calling al_load_bitmap, code = %d\n", al_get_errno());
		return ERROR;
	}

	screen_width = al_get_bitmap_width(reference->bmp);
	screen_height = al_get_bitmap_height(reference->bmp);
	
	c_absolute_score(reference); 
	best = c_clone_bitmap(reference);
	best->score = INT64_MAX;

	//ALLEGRO_COLOR reference_color[screen_width][screen_height];
	//c_bitmap_to_allegro_color(reference, &reference_color);

	init_c_bitmap(&bmp);

	display = al_create_display(screen_width, screen_height);

	if(!display) {
		fprintf(stderr, "failed to create display!\n");
		return ERROR;
	}

	al_clear_to_color(_map_rgb(0, 0, 0));

	al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ANY_32_WITH_ALPHA);
 
	event_queue = al_create_event_queue();
	if(!event_queue) {
		fprintf(stderr, "failed to create event_queue!\n");
		return ERROR;
	}

	timer = al_create_timer(0.1);
	frag_init();

	al_register_event_source(event_queue, al_get_keyboard_event_source());
	//al_register_event_source(event_queue, al_get_joystick_event_source());

	al_set_target_bitmap(bmp->bmp);

	int ix = 0, jx = 0;
	uint64_t count = 0, total_attempts = 0;

	al_clear_to_color(_map_rgb(0, 0, 0));
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE);
	// starting state for the image
	for(ix = 0; ix < NUM_CIRCLES; ix++) {
		int c1 = (get_rand() % 2) + 1;
		int c2 = (get_rand() % 2) + 1;
		int c3 = (get_rand() % 2) + 1;
		c_init_circle(&circles[ix], 
							     get_rand() % screen_width,
							     get_rand() % screen_height,
							     get_rand() % 5 + 3,
							     _map_rgb(c1, c2, c3));
#if 0
		int c1 = (int)eric[ix].r;
		int c2 = (int)eric[ix].g;
		int c3 = (int)eric[ix].b;
		c_init_circle(&circles[ix], 
							     eric[ix].x,
							     eric[ix].y,
							     eric[ix].radius,
							     _map_rgb(c1, c2, c3));
#endif
	}
	for(ix = 0; ix < NUM_CIRCLES; ix++) {
		c_draw_filled_circle(&circles[ix]);
	}

	al_start_timer(timer);
	c_assign_score(bmp, reference);
	al_stop_timer(timer);

	printf("assign_score initial time: %ld seconds\n", al_get_timer_count(timer) / 10);
	al_set_timer_count(timer, 0);

	int jiggle = 5, max_rollover = 8, same = 0;
	int64_t current_failures = 0;
	struct c_circle_save best_error;
	memset(&best_error, 0, sizeof(best_error));
	best_error.score = INT64_MAX; 
	//struct cmp_result result = {0};
	while(!exit) {
		count++;
		total_attempts++;
		if(jx == 0 && same < NUM_CIRCLES) {
			same = 0;
		}
		//test_bmp = c_clone_bitmap(bmp);
		init_c_bitmap(&test_bmp);
		al_set_target_bitmap(test_bmp->bmp);
		al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);
		al_clear_to_color(_map_rgb(0, 0, 0));
		al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE);
		c_action(&circles[jx]);
		for(ix = 0; ix < NUM_CIRCLES; ix++) {
			c_draw_filled_circle(&circles[ix]);
		}    
		al_start_timer(timer);
		c_assign_score(test_bmp, reference);
		//result = c_score_cmp(bmp, test_bmp, reference, &circles[jx]);
		al_stop_timer(timer);
		printf("-= Generation (%lu) =-\n", count);
		printf("Total Attempts: %lu\n", total_attempts);
		printf("assign_score time: %ld seconds\n", al_get_timer_count(timer) / 10);

		err("test_bmp->score = %ld\n", test_bmp->score); 
		err("bmp->score = %ld\n", bmp->score);
		err("test_bmp->score - bmp->score = %ld\n", test_bmp->score - bmp->score);
		err("best->score = %ld\n", best->score);
#if 0    
		err("result.ix_begin = %d\n", result.ix_begin);
		err("result.jx_begin = %d\n", result.jx_begin);
		err("result.ix_end = %d\n", result.ix_end);
		err("result.jx_end = %d\n", result.jx_end);
		err("circles[%d].x = %d\n", jx, circles[jx].x);
		err("circles[%d].y = %d\n", jx, circles[jx].y);
		err("result.radius = %d\n", result.radius);
		err("result.tdiff = %lu\n", result.tdiff);
		err("result.bdiff = %lu\n", result.bdiff);
#endif
		err("current_failures: %ld\n", current_failures);
		err("same: %d\n", same);
		err("best_error.score: %ld\n", best_error.score - bmp->score);
		err("jiggle: %d\n", jiggle);
		err("fib(jiggle): %ld\n", fib(jiggle));
		err("max_rollover: %d\n", max_rollover);
		printf("===================\n");
		al_set_timer_count(timer, 0);
		if((test_bmp->score > bmp->score) && (current_failures < fib(jiggle))) {
#if 0      
		//if((current_failures < fib(jiggle))) {
			current_failures++;
			if(best_error.score > test_bmp->score) {
				memcpy(&best_error.circle, &circles[jx], sizeof(circles[jx]));
				best_error.score = test_bmp->score;
			} 
				else if(best_error.score == test_bmp->score) {
				if(best_error.circle.radius > circles[jx].radius) {
					printf("%c[%d;%d;%dmradius switch from %d to %d\n%c[%d;%d;%dm", 
							   0x1B, 0, 31, 40,
							   best_error.circle.radius,
							   circles[jx].radius,
							   0x1B, 0, 37, 40);
					memcpy(&best_error.circle, &circles[jx], sizeof(circles[jx]));
				}
			}
#endif
			c_undo_action(&circles[jx]);
			free_c_bitmap(test_bmp);
			count--;
			current_failures++;
			al_set_target_bitmap(bmp->bmp);
			al_clear_to_color(_map_rgb(0, 0, 0));
			al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE);
			for(ix = 0; ix < NUM_CIRCLES; ix++) {
				c_draw_filled_circle(&circles[ix]);
			}
			continue;
		} else {//if(test_bmp->score == bmp->score && same < NUM_CIRCLES) {
			//same++;
			//memset(&best_error, 0, sizeof(best_error));
			//best_error.score = INT64_MAX; 
			current_failures = 0;
			free_c_bitmap(bmp);
			bmp = test_bmp;
			//jx = (jx + 1) % NUM_CIRCLES;
#if 0      
			al_set_target_backbuffer(display);
			al_clear_to_color(_map_rgb(0, 0, 0));
			al_draw_bitmap(bmp->bmp, 0, 0, 0);
			al_flip_display();
#endif
			//continue;
		}// else {
#if 0       
			//if(current_failures >= fib(jiggle)) {
				jiggle = (jiggle + 1) % max_rollover;
				if(jiggle == 0) {
					//max_rollover = MIN(++max_rollover, 12);
					max_rollover = 8;
					jiggle = 5;
				}
				same = 0;
				if(best_error.score <= bmp->score) {
					memcpy(&circles[jx], &best_error.circle, sizeof(circles[jx]));
					al_set_target_bitmap(test_bmp->bmp);
					al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);
					al_clear_to_color(_map_rgb(0, 0, 0));
					al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE);
					for(ix = 0; ix < NUM_CIRCLES; ix++) {
						c_draw_filled_circle(&circles[ix]);
					}    
					c_assign_score(test_bmp, reference);
					memset(&best_error, 0, sizeof(best_error));
					best_error.score = INT64_MAX; 
					current_failures = 0;
					free_c_bitmap(bmp);
					bmp = test_bmp;
				} /*else {
					printf("this one sucks, skipping\n");
					c_assign_score(bmp, reference);
					memset(&best_error, 0, sizeof(best_error));
					best_error.score = INT64_MAX; 
					current_failures = 0;
				}*/
			//}
#endif
		//memset(&best_error, 0, sizeof(best_error));
		//best_error.score = INT64_MAX; 
		if(bmp->score < best->score) {
			free_c_bitmap(best);
			best = c_clone_bitmap(bmp);
			FILE *best_file = fopen("output/best_circle_config.txt", "w");
			int circ_index;
			for(circ_index = 0; circ_index < NUM_CIRCLES; circ_index++) {
				unsigned char ber, beg, beb;
				_unmap_rgb(circles[circ_index].color, &ber, &beg, &beb);
				fprintf(best_file, "circles[%d].x = %d\n", circ_index,
							  circles[circ_index].x);
				fprintf(best_file, "circles[%d].y = %d\n", circ_index,
							  circles[circ_index].y);
				fprintf(best_file, "circles[%d].radius = %d\n", circ_index,
							  circles[circ_index].radius);
				fprintf(best_file, "circles[%d].color = #%02x%02x%02x\n", circ_index,
							  ber, beg, beb);
				fprintf(best_file, "==========\n");
			}
			fclose(best_file);
		}
		jx = (jx + 1) % NUM_CIRCLES;
		al_set_target_backbuffer(display);
		al_clear_to_color(_map_rgb(0, 0, 0));
		al_draw_bitmap(bmp->bmp, 0, 0, 0);
		al_flip_display();
		//if((count - 1) % 10 == 0) {
			snprintf(filename, sizeof(filename), "output/%lu.png", count);
			al_save_bitmap(filename, bmp->bmp);
		//}
	}
	//al_save_bitmap("saved.png", bmp->bmp);
	al_destroy_display(display);
	return SUCCESS;
}
