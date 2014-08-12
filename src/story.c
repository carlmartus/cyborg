#include "story.h"
#include "splash.h"
#include "map.h"
#include "media.h"
#include "util.h"
#include "sound.h"
#include "dialogtree.h"
#include <stdlib.h>

#define SBEGIN(enu, str) \
{ .type=enu, .str=(struct str)
#define SEND },

#define SSINGLE(enu) \
{ .type=enu },

static void setup_map0();
static void setup_map1();
static void setup_map2();
static void setup_map_boss();
//static void setup_map_escape();

struct index {
	int index;
};

struct dtree {
	const struct dialog *root;
	int elements;
};

static const struct dialog const dia1[] = {
	{ FILE_TITLE, FILE_TITLE_LEN,
		0, "A",
		0, "B",
		0, "C" },
};

struct story_elem {
	enum story_type type;
	union {
		struct splash splash;
		struct map map;
		struct overlay overlay;
		struct file file;
		struct index index;
		struct dtree dtree;
	};

} const story[] = {

	SBEGIN(STORY_MUSIC_PLAY, file) {
		.offset = FILE_INTRO,
		.len = FILE_INTRO_LEN,
	} SEND

	SBEGIN(STORY_OVERLAY, overlay) {
		.color = 0x00ffffff,
		.fadein = 5000,
	} SEND

	SBEGIN(STORY_SPLASH, splash) {
		.flags = SPLASH_IMAGE|SPLASH_FADE|SPLASH_CLOUDS,
		.text_template = SPLASH_TEXT_TEMPLATE_INTRO,
		.fade_ms = 2000,
		.image_offset = FILE_TITLE,
		.image_len = FILE_TITLE_LEN,
		.cloud_colors = 0xffaa8800,
	} SEND

	SSINGLE(STORY_MUSIC_STOP)

	SBEGIN(STORY_QUEUE_SFX, index) {
		.index = SFX_BLONG
	} SEND

	// Map 0
	SBEGIN(STORY_OVERLAY, overlay) {
		.color = 0x20dc8484,
		.fadein = 2000,
	} SEND

	SBEGIN(STORY_MAP, map) {
		.map_offset = FILE_MAP0,
		.map_len = FILE_MAP0_LEN,
		.player_x = 2.5f,
		.player_y = 2.5f,
		.exit_x = 1,
		.exit_y = 1,
		.setup = setup_map0,
	} SEND

	SBEGIN(STORY_QUEUE_SFX, index) {
		.index = SFX_BLONG
	} SEND

	// Map 1
	SBEGIN(STORY_OVERLAY, overlay) {
		.color = 0x40dc8484,
		.fadein = 1000,
	} SEND

	SBEGIN(STORY_MAP, map) {
		.map_offset = FILE_MAP1,
		.map_len = FILE_MAP1_LEN,
		.player_x = 5.5f,
		.player_y = 5.5f,
		.exit_x = 22,
		.exit_y = 37,
		.setup = setup_map1,
	} SEND

	SBEGIN(STORY_QUEUE_SFX, index) {
		.index = SFX_BLONG
	} SEND

	// Map 2
	SBEGIN(STORY_OVERLAY, overlay) {
		.color = 0x20ff88ff,
		.fadein = 1000,
	} SEND

	SBEGIN(STORY_MAP, map) {
		.map_offset = FILE_MAP2,
		.map_len = FILE_MAP2_LEN,
		.player_x = 1.5f,
		.player_y = 1.5f,
		.exit_x = 22,
		.exit_y = 37,
		.setup = setup_map2,
	} SEND

	SBEGIN(STORY_DIALOG, dtree) {
		.root = dia1,
	} SEND

	SBEGIN(STORY_QUEUE_SFX, index) {
		.index = SFX_BLONG
	} SEND

	SBEGIN(STORY_OVERLAY, overlay) {
		.color = 0x20ffff88,
		.fadein = 1000,
		.flags = OVERLAY_RAIN,
	} SEND

	/*
	SBEGIN(STORY_MAP, map) {
		.map_offset = FILE_ESCAPE,
		.map_len = FILE_ESCAPE_LEN,
		.player_x = 2.5f,
		.player_y = 2.5f,
		.setup = setup_map_escape,
	} SEND */

	SBEGIN(STORY_OVERLAY, overlay) {
		.color = 0x30222255,
		.fadein = 1000,
		.flags = OVERLAY_RAIN,
	} SEND

	SBEGIN(STORY_MAP, map) {
		.map_offset = FILE_BOSS,
		.map_len = FILE_BOSS_LEN,
		.player_x = 5.5f,
		.player_y = 5.5f,
		.setup = setup_map_boss,
	} SEND


	SBEGIN(STORY_SPLASH, splash) {
		.flags = SPLASH_IMAGE|SPLASH_FADE|SPLASH_CLOUDS,
		.text_template = SPLASH_TEXT_TEMPLATE_OUTRO,
		.fade_ms = 500,
		.image_offset = FILE_TITLE,
		.image_len = FILE_TITLE_LEN,
		.cloud_colors = 0xffffff00,
	} SEND
};

static int cur = -1;
static int alive = 1;

static int
set(int id)
{
	switch (story[id].type) {
		case STORY_SPLASH : splash_set(&story[id].splash); return 0;
		case STORY_MAP :	map_set(&story[id].map); return 0;
		case STORY_OVERLAY : overlay_set(&story[id].overlay); return 1;
		case STORY_MUSIC_PLAY : sound_music(
								   story[id].file.offset,
								   story[id].file.len); return 1;
		case STORY_MUSIC_STOP : sound_music_stop(); return 1;
		case STORY_QUEUE_SFX : sound_queue(story[id].index.index); return 1;
		default : return 1;
	}
}

static void
unset(int id)
{
	switch (story[id].type) {
		case STORY_SPLASH : splash_free(); break;
		case STORY_MAP :	map_free(); break;
		default : break;
	}
}

void
story_set(int id)
{
	cur = id;
	if (set(id)) story_next();
}

void
story_next()
{
	if (cur >= 0) unset(cur);
	cur++;
	if (cur >= ARR_SIZE(story)) exit(0);
	if (set(cur)) story_next();
}

void
story_step(int ms)
{
	switch (story[cur].type) {
		case STORY_SPLASH : splash_step(ms); break;
		case STORY_MAP :	map_step(ms); break;
		default :
			break;
	}
}

void
story_repeat()
{
	unset(cur);
	if (set(cur)) story_next();
}

int
story_alive()
{
	return alive;
}

static void
setup_map0()
{
	mapai_spawn(MAI_THUG2, 12, 2, 0.0f);
	mapai_spawn(MAI_THUG3, 12, 7, 4.0f);
	mapai_spawn(MAI_THUG1, 12, 12, 0.0f);
}

static void
setup_map1()
{
	mapai_spawn(MAI_THUG1, 19, 2, 0.7f);
	mapai_spawn(MAI_THUG2, 16, 13, 0.3f);
	mapai_spawn(MAI_THUG2, 8, 18, 0.3f);
	mapai_spawn(MAI_THUG1, 18, 24, 0.3f);
	mapai_spawn(MAI_THUG1, 36, 22, 0.3f);
	mapai_spawn(MAI_THUG1, 33, 19, 0.3f);

	// Alley
	mapai_spawn(MAI_THUG1, 11, 29, 1.0f);
	mapai_spawn(MAI_THUG3, 12, 29, 2.0f);
}

static void
setup_map2()
{
	mapai_spawn(MAI_SLEEPER, 15, 13, 0.5f);
	mapai_spawn(MAI_THUG1, 6, 29, 1.5f);
	mapai_spawn(MAI_THUG3, 26, 34, 1.5f);

	// Poll area
	mapai_spawn(MAI_THUG3, 34, 39, 1.5f);
	mapai_spawn(MAI_THUG1, 37, 27, 1.5f);
	mapai_spawn(MAI_THUG2, 29, 37, 1.5f);
}

static void
setup_map_boss()
{
	mapai_spawn(MAI_BOSS, 16, 13, 0.3f);
}

/*
static void
setup_map_escape()
{
	mapai_spawn(MAI_THUG1, 1, 14, 0.0f);
	mapai_spawn(MAI_THUG1, 6, 14, 0.0f);
}*/

