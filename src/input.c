#include "input.h"

static unsigned char frame = 1;
static struct mouse_state ms;
static int kstates[IK_COUNT] = { [0 ... IK_COUNT-1] = 0 };

void
input_reset()
{
	ms.x = 0;
	ms.y = 0;
	ms.dx = 0;
	ms.dy = 0;
}

void
input_set_mouse(int x, int y, int dx, int dy)
{
	ms.x = x;
	ms.y = y;
	ms.dx = dx;
	ms.dy = dy;
}

struct mouse_state
input_get_mouse()
{
	return ms;
}

int
input_get_key(enum input_key key)
{
	return kstates[key];
}

int
input_get_trigger(enum input_key key)
{
	return kstates[key] == frame;
}

void
input_set_key(enum input_key key, int on)
{
	kstates[key] = on ? frame : 0;
}

void
input_step()
{
	frame += 2; // Får aldrig bli 0
	ms.dx = 0;
	ms.dy = 0;
}

