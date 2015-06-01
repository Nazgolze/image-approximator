#ifndef CIRCLE_H
#define CIRCLE_H

#include "common.h"

void init_circles(struct ia_circles *, uint64_t);
void sort_circles(struct ia_circles *);
void refresh_circles(struct ia_circles *);
struct ia_circles *clone_circles(struct ia_circles *);
void free_circles(struct ia_circles *);
#endif
