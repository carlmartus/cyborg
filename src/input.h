#ifndef INPUT_H
#define INPUT_H

enum input_key {
	IK_NONE,
	IK_NAV_FORWARD,
	IK_NAV_BACKWARD,
	IK_NAV_LEFT,
	IK_NAV_RIGHT,
	IK_QUIT,
	IK_SKIP,
	IK_FULLSCREEN,
	IK_PUNCH1,
	IK_PUNCH2,
	IK_KICK,
	IK_COUNT,
};

struct mouse_state {
	int x, y, dx, dy;
};

void input_reset();
void input_set_mouse(int x, int y, int dx, int dy);
struct mouse_state input_get_mouse();
int input_get_key(enum input_key key);
int input_get_trigger(enum input_key key);
void input_set_key(enum input_key key, int on);
void input_step();

#endif

