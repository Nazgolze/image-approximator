#include <sys/param.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "ga.h"
#include "circle.h"
#include "image.h"

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
	if(!circles) {
		printfe("circles: is NULL");
		return;
	}
	int idx, max_tries = 0;
	for(idx = 0; idx < circles->num_circles; idx++) {
		if(ia_cfg.quit) {
			return;
		}
		printfd("Mutating circle %d/%d\n",
			idx,
			circles->num_circles);
		struct ia_circles *circles_prev = clone_circles(circles);
		ia_random_action(&circles->circles[idx]);
		refresh_circles(circles);
		refresh_circles(circles_prev);
		struct img_bitmap *_img = circles->img;
		struct img_bitmap *_img_prev = circles_prev->img;

		//printfi("_img->score = %ld\n", _img->score);
		//printfi("_img_prev->score = %ld\n", _img_prev->score);
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
		if(ia_cfg.quit) {
			return;
		}
		struct ia_circles *circles_prev = clone_circles(circles);
		ia_random_action(&circles->circles[indices[idx]]);
		refresh_circles(circles);
		refresh_circles(circles_prev);
		struct img_bitmap *_img = circles->img;
		struct img_bitmap *_img_prev = circles_prev->img;

		//printfi("_img->score = %ld\n", _img->score);
		//printfi("_img_prev->score = %ld\n", _img_prev->score);
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
		printf("we have a winner\n");
		while(ret->img->score > MIN(c1->img->score, c2->img->score)) {
			_mutate(ret);
			refresh_circles(ret);
		}
	}
	ret->mother_index = c1->my_index;
	ret->father_index = c2->my_index;

	return ret;
}

static struct ia_circles **_new_generation(struct ia_circles *c1,
    struct ia_circles *c2, struct ia_circles *c3, struct ia_circles *c4)
{
	int idx;
	struct ia_circles **generation;
	generation = calloc(GEN_SIZE, sizeof(struct ia_circles *));

	for(idx = 0; idx < (GEN_SIZE / 6); idx++) {
		generation[idx] = _crossover(c1, c2);
	}
	for(idx = (GEN_SIZE / 6); idx < (GEN_SIZE / 3); idx++) {
		generation[idx] = _crossover(c1, c3);
	}
	for(idx = (GEN_SIZE / 3); idx < (GEN_SIZE / 2); idx++) {
		generation[idx] = _crossover(c1, c4);
	}
	for(idx = (GEN_SIZE / 2); idx < ((GEN_SIZE * 2) / 3); idx++) {
		generation[idx] = _crossover(c2, c3);
	}
	for(idx = ((GEN_SIZE * 2) / 3); idx < ((GEN_SIZE * 5) / 6); idx++) {
		generation[idx] = _crossover(c2, c4);
	}
	for(idx = ((GEN_SIZE * 5) / 6); idx < GEN_SIZE; idx++) {
		generation[idx] = _crossover(c3, c4);
	}

	qsort(generation, GEN_SIZE, sizeof(struct ia_circles *),
	    (__compar_fn_t)_score_compare2);

	for(idx = 0; idx < GEN_SIZE; idx++) {
		sort_circles(generation[idx]);
		generation[idx]->my_index = idx;
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
	printfi("%s\nsee, that didn't take too long, did it?\n", end_time_str);
	free(end_time_str);
}

void _mutation(int counter)
{
	if(ia_cfg.quit) {
		return;
	}
	int idx;
	printfi("mutation round\n");
	for(idx = 0; idx < (counter / ia_cfg.mutation) + 2; idx++) {
		_mutate(ia_cfg.seed1);
		_mutate(ia_cfg.seed2);
		_mutate(ia_cfg.seed3);
	}
	sort_circles(ia_cfg.seed1);
	sort_circles(ia_cfg.seed2);
	sort_circles(ia_cfg.seed3);
	refresh_circles(ia_cfg.seed1);
	refresh_circles(ia_cfg.seed2);
	refresh_circles(ia_cfg.seed3);
}

/**
 * Save the generation circles to svg
 *
 * @param gen The generation to save
 * @param img_path Where to save it
 */
void _save_generation_svg(
	struct ia_circles *gen,
	const char *img_path
	)
{
	if(!gen) {
		printfe("No generation struct provided");
		return;
	}
	int ix;
	struct ia_circle *circle;
	FILE *fp = fopen(img_path, "w");
	if(!fp) {
		printfe("Failed to write to %s\n", img_path);
		return;
	}
	fprintf(fp, "<!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.0//EN' 'http://www.w3.org/TR/SVG/DTD/svg10.dtd'>\n");
	fprintf(fp, "<svg width='%d' height='%d' xmlns='http://www.w3.org/2000/svg'>\n",
		ia_cfg.screen_width,
		ia_cfg.screen_height);
	for(ix = 0; ix < gen->num_circles; ix++) {
		circle = &gen->circles[ix];
		fprintf(fp, "<circle cx='%d' cy='%d' r='%d' fill='rgb(%u,%u,%u)'/>\n",
			circle->x,
			circle->y,
			circle->radius,
			circle->color.r,
			circle->color.g,
			circle->color.b);
	}
	fprintf(fp, "</svg>\n");
	fclose(fp);
}

/**
 * Save the generation of circles.
 *
 * This always saves generation[0] since that is the generation
 * that is used to compare at the end of the loop whether we have
 * reached the perfect score.
 *
 * This will save 2 files for a generation:
 * - generation-X.jpg: The bitmap image
 * - generation-X.svg: The circle layout
 */
void _save_generation(
	struct ia_circles **generation,
	uint64_t counter
	)
{
	if(!generation) {
		printfe("No generation struct provided");
		return;
	}
	struct ia_circles *gen = NULL;
	char img_path[64];
	printf_console("Saving generation %lu\n", counter);
	gen = generation[0];
	// Save the image
	snprintf(img_path, sizeof(img_path),
		"%s/generation-%lu.jpg",
		OUTPUT_PATH, counter);
	al_save_bitmap(img_path, gen->img->bmp);
	// Save the circle layout
	snprintf(img_path, sizeof(img_path),
		"%s/generation-%lu.svg",
		OUTPUT_PATH, counter);
	_save_generation_svg(gen, img_path);
}

struct ia_circles *do_ga()
{
	uint64_t counter = 0;
	bool found_perfect_solution = false;
	bool perform_save = false;
	struct ia_circles **generation;
	//struct ia_circles *seed1, *seed2 = NULL, *seed3, *seed4, *best, *ret;
	qsort(circle_pool, _num_gen, sizeof(struct ia_circles),
	    (__compar_fn_t)_score_compare);
	int idx;
	for(idx = 0; idx < _num_gen; idx++) {
		printfi("circles_pool[%d] = %lu\n", idx,
		    circle_pool[idx].img->score);
	}

	generation = _new_generation(&circle_pool[0], &circle_pool[1],
	    &circle_pool[_num_gen - 1], &circle_pool[2]); 

	ia_cfg.best = clone_circles(generation[0]);

	for(idx = 0; idx < GEN_SIZE; idx++) {
		printfi("%lu generation[%d] = %lu\n", counter, idx,
		    generation[idx]->img->score);
	}

	do {
		counter++;
		perform_save = false;
		ia_cfg.seed1 = clone_circles(generation[0]);
		ia_cfg.seed2 = clone_circles(generation[1]);
		if(abs(ia_cfg.seed1->img->score - generation[GEN_SIZE / 2]->img->score) < 1000) {
			free_circles(ia_cfg.seed2);
			init_circles(ia_cfg.seed2, ia_cfg.seed2->num_circles);
		}
		if(ia_cfg.best->img->score < ia_cfg.seed1->img->score && counter > 10) {
			ia_cfg.seed3 = clone_circles(ia_cfg.best);
		} else {
			free_circles(ia_cfg.best);
			free(ia_cfg.best);
			ia_cfg.best = clone_circles(ia_cfg.seed1);
			ia_cfg.seed3 = clone_circles(generation[get_rand() % (GEN_SIZE / 2) + 2]);
		}

		ia_cfg.seed4 = clone_circles(
		    generation[(get_rand() % (GEN_SIZE / 2)) + (GEN_SIZE / 2)]);

		if(generation[0]->img->score == generation[1]->img->score) {
			_seed_mutate(ia_cfg.seed1);
			_seed_mutate(ia_cfg.seed2);
			_seed_mutate(ia_cfg.seed3);
		}
		refresh_circles(ia_cfg.seed1);
		refresh_circles(ia_cfg.seed2);
		refresh_circles(ia_cfg.seed3);
		refresh_circles(ia_cfg.seed4);
		sort_circles(ia_cfg.seed1);
		sort_circles(ia_cfg.seed2);
		sort_circles(ia_cfg.seed3);
		sort_circles(ia_cfg.seed4);

		switch(ia_cfg.action) {
			case IA_USER_MUTATE:
				_mutation(counter);
				break;
			case IA_USER_SAVE:
				perform_save = true;
				break;
			default:
				break;
		}
		ia_cfg.action = IA_USER_NONE;

		if(counter % ia_cfg.mutation == 0) {
			_mutation(counter);
		}

		_free_generation(generation);
		free(generation);

		generation = _new_generation(ia_cfg.seed1, ia_cfg.seed2, ia_cfg.seed3, ia_cfg.seed4);
		if(perform_save) {
			_save_generation(generation, counter);
		}

		free_circles(ia_cfg.seed1);
		free_circles(ia_cfg.seed2);
		free_circles(ia_cfg.seed3);
		free_circles(ia_cfg.seed4);
		free(ia_cfg.seed1);
		free(ia_cfg.seed2);
		free(ia_cfg.seed3);
		free(ia_cfg.seed4);

		for(idx = 0; idx < 5; idx++) {
			printfi("%lu generation[%d] = %lu; mother = %ld, father = %ld\n",
			    counter, idx, generation[idx]->img->score,
			    generation[idx]->mother_index,
			    generation[idx]->father_index);
		}
		if(generation[0]->img->score == 0) {
			found_perfect_solution = true;
		}
	} while(found_perfect_solution == false && !ia_cfg.quit);
	ia_cfg.ret = clone_circles(generation[0]);
	_free_generation(generation);
	free(generation);

	return ia_cfg.ret;
}
