#ifndef MAP_H
#define MAP_H
#include "story.h"

void map_load_globals();
void map_set(const struct map *m);
void map_free();
void map_step(int ms);
int map_move(float *x, float *y, float dx, float dy);
void map_damage(float x, float y, float scale, int dmg);
void map_set_exit(int x, int y);

// Ai
enum map_ai {
	MAI_THUG1,
	MAI_THUG2,
	MAI_THUG3,
	MAI_SLEEPER,
	MAI_RUNNER,
	MAI_BOSS,
};

void mapai_reset();
void mapai_info_player(float x, float y);
void mapai_spawn(enum map_ai mai, int x, int y, float dir);
int mapai_frame(int ms);
void mapai_render();
void mapai_spread(float *x, float *y);
int mapai_damage(float x, float y, float rad, int dmg);

#endif

