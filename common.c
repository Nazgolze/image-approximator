#include <stdlib.h>
#include <stdio.h>

#include "common.h"

#define RAND_SIZE 262144
static int _random_index = 0;
static uint32_t _random_data[RAND_SIZE];

static void _populate_random_data()
{
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
		srandom(_random_data[0]);
		for(int ix = elements_read; ix < RAND_SIZE; ix++) {
			_random_data[ix] = random();
		}
	}
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
