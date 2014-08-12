#ifndef BODY_H
#define BODY_H
#include <stdint.h>

enum torso {
	TORSO_1,
	TORSO_COUNT,
};

enum head {
	HEAD_1,
	HEAD_2,
	HEAD_3,
	HEAD_COUNT,
};

enum bstate {
	BS_IDLE,
	BS_WALK,
	BS_RECOVER,
	BS_DEAD,
	BS_PUNCH1,
	BS_PUNCH2,
};

struct body {
	uint32_t leg_color, torso_color;
	enum head head;

	float scale, x, y, dir;

	enum bstate state;
	int ms;
};

void body_load_globals();
void body_reset(struct body *b,
		uint32_t legcol, uint32_t torsocol,
		enum head h, float scale);
void body_render(struct body *b);
void body_set_state(struct body *b, enum bstate bs, int ms);
void body_set_position(struct body *b, float x, float y, float dir);

#endif

