#include "map.h"
#include "util.h"
#include "media.h"
#include "config.h"
#include "body.h"
#include "types.h"
#include "input.h"
#include "font.h"
#include "sound.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>

#define BODY_TORSO 0x4488AA
#define BODY_LEGS 0xAA8844

#define MOUSE_SENS 0.02f
#define MOVE_SPEED 0.003f

#define NAV_SENS 5.0f
#define BLOCK_PUNCH 250

#define STAMINA_PUNCH 20
#define STAMINA_TICK 150

typedef unsigned short cell_t;

static enum {
	STEP_NONE,
	STEP_NEXT,
	STEP_REPEAT,
} step_next;
static struct { unsigned short w, h; } *header;
static cell_t *cells;
static struct texture blocks;
static struct vec2f camera;
static float cam_zoom;
static struct vec2f cam_min, cam_max;

static int exit_x, exit_y;

// Player states
static float player_x, player_y, player_dx, player_dy, player_dir;
static int player_block = 0;
static int walk_anim, next_flip;
static struct body player_body;
static int player_hp, stamina, stamina_regen, stamina_tick;

#define BLOCK_BLOCKING 1

enum {
	BL_GROUND1 = 0,
	BL_SOLID1 = 1,
	BL_WATER1 = 4,
	BL_GROUND1_BURN = 5,
	BL_SPACE = 6,
	BL_GROUND1_BARRELS = 7,
	BL_JUNGLE_SOLID = 8,
};

static const struct {
	uint8_t flags;
} const block_settings[] = {
	[BL_GROUND1] = { 0 },
	[BL_SOLID1] = { BLOCK_BLOCKING },
	[BL_WATER1] = { BLOCK_BLOCKING },
	[BL_GROUND1_BURN] = { 0 },
	[BL_SPACE] = { BLOCK_BLOCKING },
	[BL_GROUND1_BARRELS] = { BLOCK_BLOCKING },
	[BL_JUNGLE_SOLID] = { BLOCK_BLOCKING },
};

void
map_load_globals()
{
	texture_load(&blocks, FILE_TILES, FILE_TILES_LEN, 8, TF_JPEG);
}

void
map_set(const struct map *m)
{
	void *ptr;

	walk_anim = 0;
	player_x = m->player_x;
	player_y = m->player_y;
	player_dir = 0.1f;
	next_flip = 0;
	player_hp = 100;
	stamina = 100;
	stamina_regen = 0;
	exit_x = m->exit_x;
	exit_y = m->exit_y;
	step_next = STEP_NONE;

	ptr = util_chunk(m->map_offset, m->map_len);
	header = ptr;
	cells = (cell_t*) (header + 1);

	body_reset(&player_body, BODY_LEGS, BODY_TORSO, HEAD_1, 0.5f);

	cam_zoom = 5.0f;

	mapai_reset();
	if (m->setup) m->setup();
}

void
map_free()
{
	free(header);
}

static void
set_camera()
{
	glLoadIdentity();
	glOrtho(
			cam_min.x, cam_max.x,
			cam_max.y, cam_min.y,
			-1.0, 1.0);
}

static void
inflict_damage(float range, int damage)
{
	//damage = 200; // TMP
	mapai_damage(
			player_x + player_dx * range,
			player_y + player_dy * range,
			range,
			damage);
}

static void
recieve_damage(int dmg)
{
	if (player_hp > 0) player_hp -= dmg;

	if (player_hp <= 0) {
		body_set_state(&player_body, BS_DEAD, 0);
		player_block = 2500;
	} else {
		body_set_state(&player_body, BS_RECOVER, 0);
		player_block = 400;
	}
}

static int
check_exit()
{
	if ((int) player_x == exit_x && (int) player_y == exit_y) {
		step_next = STEP_NEXT;
		return 1;
	}
	return 0;
}

static void
control_player(int ms)
{
	float lx, ly, ldir;

	stamina_tick += ms;

	if (player_hp <= 0) {
		step_next = STEP_REPEAT;
		return;
	}

	while (stamina_tick > STAMINA_TICK) {
		stamina += stamina_regen;
		stamina_tick -= STAMINA_TICK;
	}
	if (stamina > 100) stamina = 100;

	ldir = 0.0f;
	ldir -= input_get_key(IK_NAV_LEFT) ? NAV_SENS : 0.0f;
	ldir += input_get_key(IK_NAV_RIGHT) ? NAV_SENS : 0.0f;
	player_dir += ldir * (float) ms * 0.0008f;
	player_dx = cosf(player_dir);
	player_dy = sinf(player_dir);

	while (next_flip-- > 0) player_dir += M_PI;
	next_flip = 0;

	if (input_get_trigger(IK_PUNCH1) &&
			stamina >= STAMINA_PUNCH) {
		stamina_regen = 0;
		stamina -= STAMINA_PUNCH;

		body_set_state(&player_body, BS_PUNCH1, 0);
		sound_queue(SFX_HIT1);
		player_block = BLOCK_PUNCH;
		inflict_damage(0.6, 60);

	} else if (input_get_trigger(IK_PUNCH2) &&
			stamina >= STAMINA_PUNCH) {
		stamina_regen = 0;
		stamina -= STAMINA_PUNCH;

		body_set_state(&player_body, BS_PUNCH2, 0);
		sound_queue(SFX_HIT2);
		player_block = BLOCK_PUNCH;
		inflict_damage(0.6, 60);

	} else if (input_get_key(IK_NAV_FORWARD)) {
		lx = player_dx * MOVE_SPEED * (float) ms;
		ly = player_dy * MOVE_SPEED * (float) ms;

		map_move(&player_x, &player_y, lx, ly);
		if (check_exit()) return;

		//player_x += lx;
		//player_y += ly;
		walk_anim += ms;
		stamina_regen = 1;
		body_set_state(&player_body, BS_WALK, walk_anim >> 7);
	} else {
		stamina_regen = 2;
		body_set_state(&player_body, BS_IDLE, 0);
	}


	body_set_position(&player_body, player_x, player_y, player_dir);
}

static void
update_camera()
{
	float rx, ry;
	camera.x = player_x;
	camera.y = player_y;

	rx = cam_zoom * SCREEN_RATIO;
	ry = cam_zoom;
	cam_min.x = camera.x - rx;
	cam_min.y = camera.y - ry;
	cam_max.x = camera.x + rx;
	cam_max.y = camera.y + ry;
}

static void
frame(int ms)
{
	if (mapai_frame(ms) == 0) step_next = STEP_NEXT;

	if (input_get_trigger(IK_NAV_BACKWARD)) next_flip++;

	if (player_block <= 0) {
		control_player(ms);
		mapai_info_player(player_x, player_y);
		mapai_spread(&player_x, &player_y);
	} else {
		player_block -= ms;
	}

	update_camera();
}

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)
static void
render_world()
{
	int i, x, y, w, h, x0, x1, y0, y1;
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_2D);

	glColor4ub(0xff, 0xff, 0xff, 0xff);

	w = header->w;
	h = header->h;

	x0 = MAX(0, (int) cam_min.x);
	y0 = MAX(0, (int) cam_min.y);
	x1 = MIN(w, 1 + (int) cam_max.x);
	y1 = MIN(h, 1 + (int) cam_max.y);

	for (y=y0; y<y1; y++) {
		for (x=x0; x<x1; x++) {
			i = y*w + x;
			texture_bind(&blocks, cells[i]);

			glBegin(GL_QUADS);
			glTexCoord2i(0, 0); glVertex2i(x, y);
			glTexCoord2i(1, 0); glVertex2i(x+1, y);
			glTexCoord2i(1, 1); glVertex2i(x+1, y+1);
			glTexCoord2i(0, 1); glVertex2i(x, y+1);
			glEnd();
		}
	}

	glPopAttrib();
}

static void
render_player()
{
	body_render(&player_body);
}

static void
render_hud()
{
	util_screen_matrix();
	font_set_color(0xffffff);
	font_printf(10, 10, "Health: %d", player_hp);
	font_printf(10, 40, "Stamina: %d", stamina);
}

static void
render()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	set_camera();

	glMatrixMode(GL_MODELVIEW);
	render_world();
	mapai_render();
	render_player();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	render_hud();
}

void
map_step(int ms)
{
	frame(ms);
	render();

	switch (step_next) {
		case STEP_NEXT :
			step_next = STEP_NONE;
			story_next();
			break;
		case STEP_REPEAT :
			step_next = STEP_NONE;
			story_repeat();
			break;
		default : break;
	}
}

static int
is_blocking(int x, int y)
{
	if (x < 1 || x >= header->w-1 || y < 1 || y >= header->h-1) return 1;
	return (block_settings[cells[y*header->w + x]].flags & BLOCK_BLOCKING) > 0;
}

int
map_move(float *x, float *y, float dx, float dy)
{
	int x0, y0, x1, y1;
	x0 = (int) *x;
	y0 = (int) *y;
	x1 = (int) (*x + dx);
	y1 = (int) (*y + dy);

	if (x0 != x1 || y0 != y1) {
		if (is_blocking(x1, y1)) {

			if (x0 == x1) {
				*x += dx;
			} else if (y0 == y1) {
				*y += dy;
			} else if (x0 != x1 && y0 != y1) {
				if (!is_blocking(x0, y1)) *y += dy;
				if (!is_blocking(x1, y0)) *x += dx;
			}

			return 1;
		}
	}

	*x += dx;
	*y += dy;
	return 0;
}

void
map_damage(float x, float y, float scale, int dmg)
{
	float dx, dy, rad;
	if (player_hp <= 0) return;
	dx = player_x - x;
	dy = player_y - y;
	rad = sqrtf(dx*dx + dy*dy);
	if (rad <= scale) {
		recieve_damage(dmg);
	}
}

void
map_set_exit(int x, int y)
{
	exit_x = x;
	exit_y = y;
}

