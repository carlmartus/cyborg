#ifndef STORY_H
#define STORY_H
#include <stdint.h>

enum story_type {
	STORY_EXIT,
	STORY_SPLASH,
	STORY_OVERLAY,
	STORY_MAP,
	STORY_MUSIC_PLAY,
	STORY_MUSIC_STOP,
	STORY_QUEUE_SFX,
	STORY_DIALOG,
};

#define SPLASH_IMAGE 1
#define SPLASH_CLOUDS 2
#define SPLASH_FADE 8

enum splash_text_template {
	SPLASH_TEXT_TEMPLATE_INTRO,
	SPLASH_TEXT_TEMPLATE_OUTRO,
};

struct splash {
	uint8_t flags;
	uint32_t image_offset, image_len;
	uint32_t cloud_colors;
	int fade_ms;
	enum splash_text_template text_template;
};

typedef void (*map_setup_t) ();

// Map
struct map {
	int zero_block;
	uint32_t map_offset, map_len;
	float player_x, player_y;
	int exit_x, exit_y;
	map_setup_t setup;
};

// Set screen overlay
#define OVERLAY_RAIN 1

struct overlay {
	uint8_t flags;
	uint32_t color;
	int fadein;
};

// Dialog tree
struct dialog {
	uint32_t file, file_length;
	int jump0;
	const char *text0;
	int jump1;
	const char *text1;
	int jump2;
	const char *text2;
};

// File
struct file {
	uint32_t offset, len;
};

void story_set(int id);
void story_next();
void story_step(int ms);
void story_repeat();
int story_alive();

#endif

