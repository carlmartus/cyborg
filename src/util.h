#ifndef UTIL_H
#define UTIL_H
#include "story.h"
#include <stdint.h>

#define ARR_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

enum texture_format {
	TF_JPEG,
	TF_PNG,
};

struct texture {
	int w, h, grid, count;
	unsigned int opengl;
};

// math
float math_angle(float ax, float ay, float bx, float by);
float math_dist(float ax, float ay, float bx, float by);
float math_fisqrtf(float x);

// texture
void texture_load(struct texture *tx, uint32_t offset, uint32_t len,
		int grid, enum texture_format fmt);
void texture_free(struct texture *tx);
void texture_bind(struct texture *tx, int frame);

// screen
void util_screen_matrix();

// misc
void util_load_globals();
void *util_chunk(uint32_t offset, uint32_t len);
void util_crash(const char *reason);
void util_render_block();
int util_rng();

// overlay
void overlay_set(const struct overlay *ol);
void overlay_set_fade(int timeout);
void overlay_render();

#endif

