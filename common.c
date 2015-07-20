#define _GNU_SOURCE
#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>

#include <bsd/stdlib.h>

#include "common.h"

#define RAND_SIZE 262144
static int _random_index = 0;
static uint32_t _random_data[RAND_SIZE];

static void _populate_random_data()
{
	arc4random_buf(_random_data, sizeof(_random_data));
#if 0
	FILE *dev_urandom = NULL;
	size_t elements_read = 0;
	dev_urandom = fopen("/dev/urandom", "r");
	if(dev_urandom) {
		elements_read = fread(&_random_data, sizeof(uint32_t), RAND_SIZE, dev_urandom);
		fclose(dev_urandom);
	} else {
		printfe("can't get /dev/urandom, failing to srandom....\n");
		srandom(_random_data[0]);
		for(int ix = 0; ix < RAND_SIZE; ix++) {
			_random_data[ix] = random();
		}
	}
	if(elements_read < RAND_SIZE && elements_read > 0) {
		printfe("unable to fully read /dev/urandom.  failing back to srandom\n");
		uint32_t tmp;
		time_t tmp2;
		tmp2 = time(&tmp2);
		tmp ^= (uint32_t)tmp2;

		srandom(tmp);
		for(int ix = elements_read; ix < RAND_SIZE; ix++) {
			_random_data[ix] = random();
		}
	}
#endif
}

uint32_t get_rand()
{
	uint32_t random_int;
	if(_random_index == 0) {
		_populate_random_data();
	}
	random_int = _random_data[_random_index];
	_random_index = (_random_index + 1) % RAND_SIZE;
	return random_int;
}

void start_time(struct timespec *tp1)
{
	clock_gettime(CLOCK_MONOTONIC, tp1);
}

char *end_time(struct timespec *tp1, struct timespec *tp2, char *fmt, ...)
{
	clock_gettime(CLOCK_MONOTONIC, tp2);

	char *info_str;
	int error = SUCCESS;
	va_list ap;
	va_start(ap, fmt);
	error = vasprintf(&info_str, fmt, ap);
	if(error == ERROR)
		printfe("%s", strerror(errno));
	va_end(ap);

	char nanosec_str[12] = {0};
	char *tmp;
	int tmp_len;
	int jx;
	
	long seconds, nanoseconds;
	if(tp2->tv_nsec < tp1->tv_nsec) {
		nanoseconds = labs(tp2->tv_nsec - tp1->tv_nsec);
		seconds = tp2->tv_sec - tp1->tv_sec - 1;
	} else {
		nanoseconds = tp2->tv_nsec - tp1->tv_nsec;
		seconds = tp2->tv_sec - tp1->tv_sec;
	}
	
	error = asprintf(&tmp, "%ld", nanoseconds);
	if(error == ERROR)
		printfe("%s", strerror(errno));
	tmp_len = strlen(tmp);
	jx = 9;
	for(int ix = tmp_len; ix >= 0; ix--) {
		nanosec_str[jx] = tmp[ix];
		jx--;
	}
	for(int ix = 0; nanosec_str[ix] == 0; ix++) {
		nanosec_str[ix] = '0';
	}
	free(tmp);
	
	error = asprintf(&tmp, "%s: %ld.%ss", info_str, seconds, nanosec_str);
	if(error == ERROR)
		printfe("%s", strerror(errno));
	free(info_str);
	
	return tmp;
}

void ia_random_action(struct ia_circle *circle)
{
	int screen_factor = MIN(ia_cfg.screen_width, ia_cfg.screen_height);
	int factor = get_rand() % (screen_factor / 2);
	//int factor = get_rand() % (MAX(screen_width, screen_height) * 2);
	int tmp;
	enum ia_actions action = get_rand() % 3;

	factor = MAX(factor, 1);

	int r_factor = get_rand() % 2 ? factor : -factor;
	unsigned char c_factor = get_rand() % 50;

	c_factor = MAX(c_factor, 1);

	int dir = get_rand() % 4;
	switch(action) {
	case IA_MOVE:
		switch(dir) {
		case IA_LEFT:
			circle->x = MAX(0, circle->x - factor);
			break;
		case IA_RIGHT:
			circle->x = MIN(ia_cfg.screen_width, circle->x + factor);
			break;
		case IA_UP:
			circle->y = MAX(0, circle->y - factor);
			break;
		case IA_DOWN:
			circle->y = MIN(ia_cfg.screen_height, circle->y + factor);
			break;
		}
		break;
	case IA_RESIZE:
		circle->radius += r_factor;
		circle->radius = MAX(1, circle->radius);
		tmp = MIN(ia_cfg.screen_width, ia_cfg.screen_height) / 1;
		circle->radius = MIN(tmp, circle->radius);
		break;
	case IA_RECOLOR:
		tmp = get_rand() % 3;
		unsigned char r,g,b, new;
		r = circle->color.r;
		g = circle->color.g;
		b = circle->color.b;
		switch(tmp) {
		case IA_RED:
			if(get_rand() % 2) {
				new = MIN(255, r + c_factor);
			} else {
				new = MAX(0, r - c_factor);
			}
			circle->color.r = new;
			break;
		case IA_GREEN:
			if(get_rand() % 2) {
				new = MIN(255, g + c_factor);
			} else {
				new = MAX(0, g - c_factor);
			}
			circle->color.g = new;
			break;
		case IA_BLUE:
			if(get_rand() % 2) {
				new = MIN(255, b + c_factor);
			} else {
				new = MAX(0, b - c_factor);
			}
			circle->color.b = new;
			break;
		}
	}
}

int printfl(enum ia_print_level pl, const char *fmt, ...)
{
	int error = SUCCESS;
	char str[2048] = {0};
	va_list ap;
	va_start(ap, fmt);
	error = vsnprintf(str, sizeof(str), fmt, ap);
	if(error == ERROR)
		return error;
	va_end(ap);
	if(!ia_cfg.log) {
		printf("[ERROR no ia_cfg.log]: %s\n", str);
		fflush(stdout);
		return error;
	}

	if(pl == IA_INFO && ia_cfg.print_level >= IA_INFO) {
		fprintf(ia_cfg.log, "%s\n", str);
	} else if(pl == IA_ERR && ia_cfg.print_level >= IA_ERR) {
		fprintf(stderr, "Error: %s\n", str);
		fprintf(ia_cfg.log, "Error: %s\n", str);
		fflush(stderr);
	} else if(pl == IA_DEBUG && ia_cfg.print_level >= IA_DEBUG) {
		fprintf(ia_cfg.log, "%s-%d: %s\n", __FUNCTION__, __LINE__, str);
	}
	fflush(ia_cfg.log);

	return error;
}

/**
 * Prints to a message to the console and to the log via INFO level
 */
int printf_console(const char *fmt, ...)
{
	int error = SUCCESS;
	char str[2048] = {0};
	va_list ap;
	va_start(ap, fmt);
	error = vsnprintf(str, sizeof(str), fmt, ap);
	if(error == ERROR)
		return error;
	va_end(ap);
	printfi("%s", str);
	printf("\nCONSOLE: %s\n>> ", str);
	fflush(stdout);
	return error;
}

/**
 * Strip newline from the passed in string
 *
 * @param str The string to update
 */
void strip_newline(char *str)
{
	if(!str) {
		return;
	}
	int last = strlen(str) - 1;
	if(str[last] == '\n' || str[last] == '\r') {
		str[last] = '\0';
	}
}

/**
 * Initialize the config data
 */
void ia_cfg_init()
{
	memset(&ia_cfg, 0, sizeof(struct ia_cfg_st));
	ia_cfg.print_level = IA_DEBUG;
	ia_cfg.num_circles = 0;
	ia_cfg.num_sets = 2700;
	ia_cfg.num_init = 500;
	ia_cfg.mutation = 100;

	ia_cfg.log = fopen("log", "w");
	if(!ia_cfg.log) {
		printfe("Failed to open log\n");
	}
}

/**
 * Print the config settings
 */
void ia_cfg_print()
{
	printfi("Screen: width: %d, height: %d\n",
		ia_cfg.screen_width,
		ia_cfg.screen_height);
	printfi("Circles: %d, Sets: %d, Init sets: %d\n",
		ia_cfg.num_circles,
		ia_cfg.num_sets,
		ia_cfg.num_init);
	printfi("Mutation round every %d generations\n",
		ia_cfg.mutation);
}

/**
 * Free the config data
 */
void ia_cfg_free()
{
	if(ia_cfg.reference_image) {
		// Free the image
	}
	if(ia_cfg.best_image) {
		// Free the image
	}
	if(ia_cfg.log) {
		fclose(ia_cfg.log);
	}
}

/**
 * Read the config from a path
 *
 * @param path The path of the config
 */
int ia_cfg_read(
	const char *path
	)
{
	FILE *fp = fopen(path, "r");
	if(!fp) {
		printfe("No such config %s\n", path);
		return ERROR;
	}
	char line[512];
	char *key, *val, *savep;
	char *sep = " = ";
	while(fgets(line, sizeof(line), fp)) {
		strip_newline(line);
		if(!strlen(line)) {
			continue;
		}
		if(line[0] == '#') {
			continue;
		}
		key = strtok_r(line, sep, &savep);
		val = strtok_r(NULL, sep, &savep);
		if(!key || !val) {
			printfe("Failed to parse line [%s]. Skipping\n", line);
			continue;
		}
		if(streq(key, "print_level")) {
			if(streq(val, "IA_INFO")) {
				ia_cfg.print_level = IA_INFO;
			} else if(streq(val, "IA_ERR")) {
				ia_cfg.print_level = IA_ERR;
			} else if(streq(val, "IA_DEBUG")) {
				ia_cfg.print_level = IA_DEBUG;
			}
		} else if(streq(key, "num_init")) {
			ia_cfg.num_init = strtol(val, (char **)NULL, 10);
		} else if(streq(key, "num_sets")) {
			ia_cfg.num_sets = strtol(val, (char **)NULL, 10);
		} else if(streq(key, "num_circles")) {
			ia_cfg.num_circles = strtol(val, (char **)NULL, 10);
		} else if(streq(key, "mutation")) {
			ia_cfg.mutation = strtol(val, (char **)NULL, 10);
		} else {
			printfe("Unable to handle %s\n", key);
		}
	}
	return SUCCESS;
}
