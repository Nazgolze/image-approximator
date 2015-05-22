#include <stdlib.h>
#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>

#include "common.h"
#include "image.h"

static unsigned char *_get_raw_image()
{
	glPixelStorei(GL_PACK_ALIGNMENT,1);
	unsigned char *image;
	unsigned int size = screen_width * screen_height * 3;
	image = calloc(1, size);
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, screen_width, screen_height,
	    GL_RGB, GL_UNSIGNED_BYTE, image);
	return image;
}

static void _flip(unsigned char *image)
{
	int count = 0, idx;
	unsigned char temp_image[screen_width * 3];
	memset(&temp_image, 0, sizeof(temp_image));

	for(idx = screen_height - 1; idx >= 0 && count < idx; idx--) {
		memcpy(&temp_image,
		    image + (screen_width * 3 * count),
		    screen_width * 3);
		memcpy(image + (screen_width * 3 * count),
		    image + (screen_width * 3 * idx),
		    screen_width * 3);
		memcpy(image + (screen_width * 3 * idx),
		    &temp_image,
		    screen_width * 3);
		count++;
	}
	//printf("idx = %d\ncount = %d\n", idx, count);
}

void img_free(struct img_bitmap *bmp)
{
	al_destroy_bitmap(bmp->bmp);
	free(bmp);
}

struct img_bitmap *img_clone(struct img_bitmap *bmp)
{
	struct img_bitmap *new_bmp
	    = malloc(sizeof(struct img_bitmap));
	new_bmp->score = bmp->score;
	new_bmp->bmp = al_clone_bitmap(bmp->bmp);
	return new_bmp;
}

struct img_bitmap *img_load(const char *filename)
{
	struct img_bitmap *image = calloc(1, sizeof(struct img_bitmap));
	if(image == NULL) {
		printfd("memory allocation error");
		return NULL;
	}
	image->bmp = al_load_bitmap(filename);
	if(image->bmp == NULL) {
		printfd("unable to open %s", filename);
		free(image);
		image = NULL;
	}
	return image;
}

struct img_bitmap *img_from_GL()
{
	unsigned char *raw = _get_raw_image();
	struct img_bitmap *image = calloc(1, sizeof(struct img_bitmap));
	if(!image) {
		return NULL;
	}
	_flip(raw);

	ALLEGRO_BITMAP *bmp;
	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
	bmp = al_create_bitmap(screen_width, screen_height);

	al_set_target_bitmap(bmp);

	ALLEGRO_LOCKED_REGION *locked;
	locked = al_lock_bitmap(bmp, ALLEGRO_PIXEL_FORMAT_BGR_888,
	    ALLEGRO_LOCK_WRITEONLY);

	al_free(locked->data);
	locked->data = raw;
	al_unlock_bitmap(bmp);
	image->bmp = bmp;
	return image;
}

void img_assign_score(struct img_bitmap *img, struct img_bitmap *ref)
{
	int idx, jdx, diff = 0;
	int64_t total_difference = 0;
	ALLEGRO_COLOR bpc, rpc;
	for(idx = 0; idx < screen_width; idx++) {
		for(jdx = 0; jdx < screen_height; jdx++) {
			unsigned char br, bg, bb, rr, rg, rb;
			bpc = al_get_pixel(img->bmp, idx, jdx);
			rpc = al_get_pixel(ref->bmp, idx, jdx);
			al_unmap_rgb(bpc, &br, &bg, &bb);
			al_unmap_rgb(rpc, &rr, &rg, &rb);
			diff = abs(br - rr) +
			    abs(bg - rg) +
			    abs(bb - rb);
			total_difference += diff;
		}
	}
	img->score = total_difference;
}
