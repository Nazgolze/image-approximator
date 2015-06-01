#include <sys/param.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "ga.h"
#include "circle.h"
#include "image.h"

#define GEN_SIZE 2700

static bool _is_initialized = false;
static int _num_gen;
static struct ia_circles *circle_pool = NULL;

static int _random_compare(const void *d1, const void *d2)
{
	int ret = (int)get_rand() % 3;
	ret--;
	return ret;
}

static int _score_compare(struct ia_circles *c1, struct ia_circles *c2)
{
	if(c1->img->score < c2->img->score) {
		return -1;
	} else if(c1->img->score > c2->img->score) {
		return 1;
	}
	return 0;
}

static int _score_compare2(struct ia_circles **c1, struct ia_circles **c2)
{
	if((*c1)->img->score < (*c2)->img->score) {
		return -1;
	} else if((*c1)->img->score > (*c2)->img->score) {
		return 1;
	}
	return 0;
}

static void _mutate(struct ia_circles *circles)
{
	// basically a stochastic hill climbing algorithm, with some
	// allowance for errors 
	int idx, max_tries = 0;
	for(idx = 0; idx < circles->num_circles; idx++) {
		struct ia_circles *circles_prev = clone_circles(circles);
		ia_random_action(&circles->circles[idx]);
		refresh_circles(circles);
		refresh_circles(circles_prev);
		struct img_bitmap *_img = circles->img;
		struct img_bitmap *_img_prev = circles_prev->img;

		//printf("_img->score = %ld\n", _img->score);
		//printf("_img_prev->score = %ld\n", _img_prev->score);
		if((_img->score - _img_prev->score) > 0) {
			max_tries++;
			memcpy(&circles->circles[idx],
			    &circles_prev->circles[idx],
			    sizeof(struct ia_circle));
			refresh_circles(circles);
			if(max_tries <= MIN(max_tries, circles->num_circles)) {
				idx--;
			} else {
				max_tries = 0;
			}
		} else {
			max_tries = 0;
		}
		free_circles(circles_prev);
		free(circles_prev);
	}
}

static void _mini_mutate(struct ia_circles *circles, int *indices,
    int indices_len)
{
	int idx, max_tries = 0;
	for(idx = 0; idx < indices_len; idx++) {
		struct ia_circles *circles_prev = clone_circles(circles);
		ia_random_action(&circles->circles[indices[idx]]);
		refresh_circles(circles);
		refresh_circles(circles_prev);
		struct img_bitmap *_img = circles->img;
		struct img_bitmap *_img_prev = circles_prev->img;

		//printf("_img->score = %ld\n", _img->score);
		//printf("_img_prev->score = %ld\n", _img_prev->score);
		if((_img->score - _img_prev->score) > 0) {
			max_tries++;
			memcpy(&circles->circles[indices[idx]],
			    &circles_prev->circles[indices[idx]],
			    sizeof(struct ia_circle));
			refresh_circles(circles);
			if(max_tries <= MIN(max_tries, circles->num_circles)) {
				idx--;
			} else {
				max_tries = 0;
			}
		} else {
			max_tries = 0;
		}
		free_circles(circles_prev);
		free(circles_prev);
	}

}

static void _seed_mutate(struct ia_circles *c)
{

	int idx, indices_num, *indices;
	indices_num = get_rand() % 30;
	indices_num += 20;
	indices = malloc(indices_num * sizeof(int));

	for(idx = 0; idx < indices_num; idx++) {
		indices[idx] = get_rand() % c->num_circles;
	}
	_mini_mutate(c, indices, indices_num);
	refresh_circles(c);
	free(indices);
}

#define CROSS_X      0x01
#define CROSS_Y      0x02
#define CROSS_RADIUS 0x04
#define CROSS_R      0x08
#define CROSS_G      0x10
#define CROSS_B      0x20
#define CROSS_ALL    0x3f

#define NEW_AVG(avg) avg = get_rand() % 2 ? true : false
#define CROSS_ATTR(c1, c2, attr, attr_type, mask, chkmask)                     \
	do {                                                                   \
		NEW_AVG(avg);                                                  \
		if((mask & chkmask) && avg) {                                  \
			c1->attr = c2->attr = ((c1->attr + c2->attr) / 2);     \
		} else if(mask & chkmask) {                                    \
			attr_type temp = c1->attr;                             \
			c1->attr = c2->attr;                                   \
			c2->attr = temp;                                       \
		}                                                              \
	} while(false)
	
static void _cross(struct ia_circle *c1, struct ia_circle *c2)
{
	uint32_t mask = 0;
	while(mask == 0) {
		mask = get_rand() % (CROSS_ALL + 1);
	}

	bool avg;

	if((mask & CROSS_ALL) == CROSS_ALL) {
		struct ia_circle temp;
		memcpy(&temp, c1, sizeof(struct ia_circle));
		memcpy(c1, c2, sizeof(struct ia_circle));
		memcpy(c2, &temp, sizeof(struct ia_circle));
		return;
	}

	CROSS_ATTR(c1, c2, x, int, mask, CROSS_X);
	CROSS_ATTR(c1, c2, y, int, mask, CROSS_Y);
	CROSS_ATTR(c1, c2, radius, int, mask, CROSS_RADIUS);
	CROSS_ATTR(c1, c2, color.r, uint8_t, mask, CROSS_R);
	CROSS_ATTR(c1, c2, color.g, uint8_t, mask, CROSS_G);
	CROSS_ATTR(c1, c2, color.b, uint8_t, mask, CROSS_B);
}

static struct ia_circles *_crossover(struct ia_circles *c1,
    struct ia_circles *c2)
{
	struct ia_circles *c1c = clone_circles(c1);
	struct ia_circles *c2c = clone_circles(c2);
	struct ia_circles *ret = NULL;

	// for now, between 10% and 20% will get swapped
	int *indices = NULL, idx;
	int indices_num = get_rand() % (c1c->num_circles / 10);
	indices_num += c1c->num_circles / 10;
	indices = malloc(indices_num * sizeof(int)); // do null check at some point

	for(idx = 0; idx < indices_num; idx++) {
		indices[idx] = get_rand() % c1c->num_circles;
	}

	for(idx = 0; idx < indices_num; idx++) {
		_cross(&c1c->circles[indices[idx]],
		    &c2c->circles[indices[idx]]);
	}
	refresh_circles(c1c);
	refresh_circles(c2c);
	// pick the best one, free the other
	if(c1c->img->score < c2c->img->score) {
		ret = c1c;
		free_circles(c2c);
		free(c2c);
	} else {
		ret = c2c;
		free_circles(c1c);
		free(c1c);
	}
	free(indices);
	refresh_circles(ret);

	if(get_rand() % 51503 == 4) {
		while(ret->img->score > MIN(c1->img->score, c2->img->score)) {
			_mutate(ret);
			refresh_circles(ret);
		}
		printf("we have a winner\n");
	}

	return ret;
}

static struct ia_circles **_new_generation(struct ia_circles *c1,
    struct ia_circles *c2, struct ia_circles *c3)
{
	int idx;
	struct ia_circles **generation;
	generation = calloc(GEN_SIZE, sizeof(struct ia_circles *));

	for(idx = 0; idx < (GEN_SIZE / 3); idx++) {
		generation[idx] = _crossover(c1, c2);
	}
	for(idx = (GEN_SIZE / 3); idx < (GEN_SIZE / 3 * 2); idx++) {
		generation[idx] = _crossover(c1, c3);
	}
	for(idx = (GEN_SIZE / 3 * 2); idx < GEN_SIZE; idx++) {
		generation[idx] = _crossover(c2, c3);
	}

	qsort(generation, GEN_SIZE, sizeof(struct ia_circles *),
	    (__compar_fn_t)_score_compare2);

	for(idx = 0; idx < GEN_SIZE; idx++) {
		sort_circles(generation[idx]);
	}

	return generation;
}

void _free_generation(struct ia_circles **generation)
{
	int idx;
	for(idx = 0; idx < GEN_SIZE; idx++) {
		free_circles(generation[idx]);
	}
}

void init_ga(int num_circles, int num_gen)
{
	char *end_time_str;
	struct timespec tp1, tp2;
	int idx;
	if(_is_initialized)
		return;

	printfl(IA_INFO,
	    "initializing for genetic algorithm, this may take some time....");
	_is_initialized = true;
	_num_gen = num_gen;
	circle_pool = calloc(num_gen, sizeof(struct ia_circles));
	start_time(&tp1);

	for(idx = 0; idx < num_gen; idx++) {
		init_circles(&circle_pool[idx], num_circles);
	}

	for(idx = 0; idx < num_gen; idx++) {
		//for(jdx = 0; jdx < 2; jdx++) {
		//	_seed_mutate(&circle_pool[idx]);
		//}
		refresh_circles(&circle_pool[idx]);
		sort_circles(&circle_pool[idx]);
	}
	
	end_time_str = end_time(&tp1, &tp2, "time");
	printf("%s\nsee, that didn't take too long, did it?\n", end_time_str);
	free(end_time_str);
}

struct ia_circles *do_ga()
{
	uint64_t counter = 0;
	bool found_perfect_solution = false;
	struct ia_circles **generation;
	struct ia_circles *seed1, *seed2 = NULL, *seed3, *best, *ret;
	qsort(circle_pool, _num_gen, sizeof(struct ia_circles),
	    (__compar_fn_t)_score_compare);
	int idx;
	for(idx = 0; idx < _num_gen; idx++) {
		printf("circles_pool[%d] = %lu\n", idx,
		    circle_pool[idx].img->score);
	}

	generation = _new_generation(&circle_pool[0], &circle_pool[1],
	    &circle_pool[_num_gen - 1]); 

	best = clone_circles(generation[0]);

	for(idx = 0; idx < GEN_SIZE; idx++) {
		printf("%lu generation[%d] = %lu\n", counter, idx,
		    generation[idx]->img->score);
	}

#if 0
	for(idx = 0; idx < 10; idx++) {
		_mutate(generation[0]);
		_mutate(generation[1]);
	}
#endif
	do {
		counter++;
		seed1 = clone_circles(generation[0]);
		seed2 = clone_circles(generation[1]);
		if(abs(seed1->img->score - generation[GEN_SIZE / 2]->img->score) < 1000) {
			free_circles(seed2);
			init_circles(seed2, seed2->num_circles);
		}
		if(best->img->score < seed1->img->score && counter > 10) {
			seed3 = clone_circles(best);
		} else {
			free_circles(best);
			free(best);
			best = clone_circles(seed1);
			seed3 = clone_circles(generation[get_rand() % (GEN_SIZE / 2) + 2]);
		}

		if(generation[0]->img->score == generation[1]->img->score) {
			_seed_mutate(seed1);
			_seed_mutate(seed2);
			_seed_mutate(seed3);
		}
		refresh_circles(seed1);
		refresh_circles(seed2);
		refresh_circles(seed3);
#if 0
		while(seed1->img->score > best->img->score) {
			_seed_mutate(seed1);
			refresh_circles(seed1);
		}
		while(seed2->img->score > best->img->score) {
			_seed_mutate(seed2);
			refresh_circles(seed2);
		}
		while(seed3->img->score > best->img->score) {
			_seed_mutate(seed3);
			refresh_circles(seed3);
		}
#endif
		sort_circles(seed1);
		sort_circles(seed2);
		sort_circles(seed3);

		if(counter % 100 == 0) {
			printf("mutation round\n");
			for(idx = 0; idx < (counter / 100) + 2; idx++) {
				_mutate(seed1);
				_mutate(seed2);
				_mutate(seed3);
			}
			sort_circles(seed1);
			sort_circles(seed2);
			sort_circles(seed3);
			refresh_circles(seed1);
			refresh_circles(seed2);
			refresh_circles(seed3);
		}

		_free_generation(generation);
		free(generation);

		generation = _new_generation(seed1, seed2, seed3);
		
		free_circles(seed1);
		free_circles(seed2);
		free_circles(seed3);
		free(seed1);
		free(seed2);
		free(seed3);
	
		for(idx = 0; idx < 5; idx++) {
			printf("%lu generation[%d] = %lu\n", counter, idx,
			    generation[idx]->img->score);
		}
		if(generation[0]->img->score == 0) {
			found_perfect_solution = true;
		}
	} while(found_perfect_solution == false);
	ret = clone_circles(generation[0]);
	_free_generation(generation);
	free(generation);

	return ret;
}
