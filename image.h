#ifndef IMAGE_H
#define IMAGE_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "common.h"

void img_free(struct img_bitmap *);
struct img_bitmap *img_clone(struct img_bitmap *);
struct img_bitmap *img_load(const char *);
struct img_bitmap *img_from_GL(void);
void img_assign_score(struct img_bitmap *, struct img_bitmap *);

#endif
