#include "map.h"
#include "body.h"
#include "util.h"
#include "math.h"
#include <stdio.h>

#define MAX_AI 16
#define CHASE_BEFORE_DELTA 100
#define CHASE_SPEED 0.0015f
#define CHASE_UPDATE 100
#define ATTACK_DISTANCE 0.8f
#define ATTACK_BEFORE 200
#define ATTACK_DURATION 200
#define ATTACK_DURATION_DELTA 100
#define SPREAD_DELAY 150
#define SPREAD_RAD 0.6f
#define DAMAGE_CUBE 0.5f
#define RECOVER_SPEED 0.035f

const static struct types {
	enum head head;
	uint32_t col_pants, col_torso;
	float scale, speed, detection;
	int health;
} const ai_types[] = {
	[MAI_THUG1] =	{ HEAD_2, 0x595d77, 0x3c476a, 0.6f, 1.0f, 8.0f, 220 },
	[MAI_THUG2] =	{ HEAD_3, 0x3c476a, 0x595d77, 0.5f, 1.5f, 10.0f, 80 },
	[MAI_THUG3] =	{ HEAD_2, 0x1c273a, 0x292d37, 0.5f, 1.2f, 8.0f, 80 },
	[MAI_SLEEPER] =	{ HEAD_2, 0x595d77, 0x3c476a, 0.6f, 1.0f, 1.9f, 220 },
	[MAI_RUNNER] =	{ HEAD_2, 0x4a1c27, 0x57393d, 0.5f, 2.0f, 32.0f, 50 },
	[MAI_BOSS] =	{ HEAD_3, 0x1c274a, 0x393d57, 1.0f, 1.5f, 32.0f, 600 },
};

enum state {
	STATE_IDLE,
	STATE_CHASE,
	STATE_ATTACK,
	STATE_DEAD,
	STATE_RECOVER,
};

static struct ai {
	enum map_ai type;
	enum state state;
	float x, y, dir, dx, dy;
	struct body body;
	uint32_t block;
	float speed_mul;
	float detection;
	int health;

	union {
		struct {
			int counter;
			int dir_update;
		} chase;
		struct {
			int counter;
		} recover;
	};
} ai[MAX_AI];


// Virtual table
static void frame_idle(struct ai *a, int ms);
static void frame_chase(struct ai *a, int ms);
static void frame_attack(struct ai *a, int ms);
static void frame_dead(struct ai *a, int ms);
static void frame_recover(struct ai *a, int ms);

static const struct {
	void (*frame) (struct ai*, int);
} const ai_state_class[] = {
	[STATE_IDLE] =	{ frame_idle },
	[STATE_CHASE] =	{ frame_chase },
	[STATE_ATTACK] = { frame_attack },
	[STATE_DEAD] = { frame_dead },
	[STATE_RECOVER] = { frame_recover },
};

static int body_count, spread_timer;
static float px, py;

void
mapai_reset()
{
	px = -50.0f;
	py = -50.0f;
	body_count = 0;
	spread_timer = 0;
}

void
mapai_info_player(float x, float y)
{
	px = x;
	py = y;
}

void
mapai_spawn(enum map_ai mai, int x, int y, float dir)
{
	struct ai *a = ai + body_count++;
	a->state = STATE_IDLE;
	a->type = mai;
	a->x = (float) x + 0.5f;
	a->y = (float) y + 0.5f;
	a->dir = dir;
	a->block = 0;
	a->detection = ai_types[mai].detection;

	body_reset(&a->body,
			ai_types[mai].col_pants,
			ai_types[mai].col_torso,
			ai_types[mai].head, ai_types[mai].scale);

	a->speed_mul = ai_types[mai].speed;
	body_set_state(&a->body, BS_IDLE, 0);
	a->health = ai_types[mai].health;
}

static void
enter_chase(struct ai *a)
{
	a->state = STATE_CHASE;
	a->dx = 0.0f;
	a->dy = 0.0f;
	a->chase.counter = 0;
	a->chase.dir_update = 0;
	a->block = CHASE_BEFORE_DELTA * (util_rng() & 3);
	body_set_state(&a->body, BS_WALK, 0);
}

static void
enter_attack(struct ai *a)
{
	a->state = STATE_ATTACK;
	a->block = ATTACK_BEFORE;
	body_set_state(&a->body, BS_IDLE, 0);
}

static void
enter_death(struct ai *a)
{
	a->state = STATE_DEAD;
}

static void
get_hit(struct ai *a, int dmg)
{
	a->health -= dmg;
	if (a->health <= 0) {
		enter_death(a);
		return;
	}

	a->state = STATE_RECOVER;
	a->recover.counter = dmg << 2;
	a->block = 0;
}

static float
dist_player(float x, float y)
{
	float dx, dy;
	dx = px - x;
	dy = py - y;
	return sqrtf(dx*dx + dy*dy);
}

static void
update_dxy(struct ai *a)
{
	a->dx = cosf(a->dir);
	a->dy = sinf(a->dir);
}

static void
frame_idle(struct ai *a, int ms)
{
	float cube;
	cube = fabs(a->x - px) + fabs(a->y - py);

	if (cube < a->detection) {
		enter_chase(a);
		return;
	}
	a->block = 1000;
}

static void
frame_chase(struct ai *a, int ms)
{
	while (a->chase.dir_update > CHASE_UPDATE) {
		a->dir = -math_angle(a->x, a->y, px, py) + M_PI_2;
		update_dxy(a);

		if (dist_player(a->x, a->y) < ai_types[a->type].scale + 0.2f) {
			enter_attack(a);
			return;
		}

		a->chase.dir_update -= CHASE_UPDATE;
	}

	float lx, ly;
	lx = a->dx * (float) ms * CHASE_SPEED * a->speed_mul;
	ly = a->dy * (float) ms * CHASE_SPEED * a->speed_mul;
	map_move(&a->x, &a->y, lx, ly);

	a->chase.counter += ms;
	a->chase.dir_update += ms;

	body_set_state(&a->body, BS_WALK, a->chase.counter >> 7);
}

static enum bstate
rng_attack()
{
	switch (util_rng() % 2) {
		default:
		case 0 : return BS_PUNCH1;
		case 1 : return BS_PUNCH2;
	}
}

static void
frame_attack(struct ai *a, int ms)
{
	body_set_state(&a->body,
			rng_attack(),
			0);
	map_damage(
			a->x + a->dx,
			a->y + a->dy,
			ai_types[a->type].scale,
			15);
	a->block = ATTACK_DURATION + ATTACK_DURATION_DELTA * (util_rng() % 3);
	a->state = STATE_CHASE;
}

static void
frame_dead(struct ai *a, int ms)
{
	body_set_state(&a->body, BS_DEAD, 0);
	a->block = 1000;
}

static void
frame_recover(struct ai *a, int ms)
{
	float dx, dy;

	a->recover.counter -= ms;
	if (a->recover.counter <= 0) {
		enter_chase(a);
		return;
	}

	dx = cosf(a->dir) * RECOVER_SPEED;
	dy = sinf(a->dir) * RECOVER_SPEED;

	map_move(&a->x, &a->y, -dx, -dy);
	body_set_state(&a->body, BS_RECOVER, 0);
}

static void
frame(struct ai *a, int ms)
{
	ai_state_class[a->state].frame(a, ms);
}

static void
//bound_check(struct ai *a0, struct ai *a1)
bound_check(float *x0, float *y0, float *x1, float *y1)
{
	float cube;
	float ax, ay, bx, by;
	ax = *x0;
	ay = *y0;
	bx = *x1;
	by = *y1;

	cube = fabs(ax - bx) + fabs(ay - by);
	if (cube >= SPREAD_RAD) return;

	float pyt;
	pyt = math_dist(ax, ay, bx, by);
	if (pyt >= SPREAD_RAD) return;

	float mx, my, rx, ry, rad;
	mx = 0.5f * (ax + bx);
	my = 0.5f * (ay + by);
	rx = ax - mx;
	ry = ay - my;
	rad = sqrtf(rx*rx + ry*ry);

	if (rad > 0.1f) {
		rad = 0.5f*SPREAD_RAD / rad;
		rx *= rad;
		ry *= rad;

		ax = mx;
		ay = my;
		bx = mx;
		by = my;
		map_move(&ax, &ay, rx, ry);
		map_move(&bx, &by, -rx, -ry);

		*x0 = ax;
		*y0 = ay;
		*x1 = bx;
		*y1 = by;
	}
}

static void
spread()
{
	struct ai *itr0, *itr1, *end;

	itr0 = ai;
	end = itr0 + body_count;
	while (itr0 < end) {
		itr1 = itr0 + 1;
		while (itr1 < end) {
			bound_check(
					&itr0->x, &itr0->y,
					&itr1->x, &itr1->y);
			itr1++;
		}
		itr0++;
	}
}

int
mapai_frame(int ms)
{
	int i, alive;
	struct ai *itr = ai;

	alive = 0;
	for (i=0; i<body_count; i++) {
		if (itr->block > 0) {
			itr->block -= ms;
			if (itr->block < 0) itr->block = 0;

		} else {

			frame(itr, ms);
		}
		body_set_position(&itr->body,
				itr->x,
				itr->y,
				itr->dir);

		if (itr->state != STATE_DEAD) alive++;
		itr++;
	}

	spread_timer -= ms;
	if (spread_timer <= 0) {
		spread();
		spread_timer += SPREAD_DELAY;
	}

	return alive;
}

void
mapai_render()
{
	int i;
	struct ai *itr = ai;
	for (i=0; i<body_count; i++) {
		body_render(&itr->body);
		itr++;
	}
}

void
mapai_spread(float *x, float *y)
{
	struct ai *itr, *end;

	itr = ai;
	end = itr + body_count;
	while (itr < end) {
		bound_check(
				x, y,
				&itr->x, &itr->y);
		itr++;
	}
}

static void
damage(struct ai *a, int dmg)
{
	get_hit(a, dmg);
}

int
mapai_damage(float x, float y, float rad, int dmg)
{
	int i, hits = 0;
	struct ai *itr = ai;
	for (i=0; i<body_count; i++) {
		if (fabs(x - itr->x) < rad && fabs(y - itr->y) < rad) {
			damage(itr, dmg);
			hits++;
		}
		itr++;
	}
	return hits;
}

