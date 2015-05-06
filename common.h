#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdio.h>

uint32_t get_rand(void);
#define printfe(...) fprintf(stderr, ##__VA_ARGS__); fflush(stderr);
#define printfd(fmt, ...) printf("%s-%d: " fmt, __FUNCTION__, __LINE__,  ##__VA_ARGS__);

#endif
