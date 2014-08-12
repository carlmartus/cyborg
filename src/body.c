#include "body.h"
#include "util.h"
#include "media.h"
#include <GL/gl.h>
#include <math.h>

enum leg_frames {
	AL_STAND = 0,
	AL_WALK1 = 1,
	AL_WALK2 = 2,
	AL_WALK3 = 3,
	AL_WALK4 = 4,
	AL_WALK5 = 5,
	AL_WALK6 = 6,
	AL_DEAD = 7,
};

enum torso_frames {
	AT_STAND1 = 0,
	AT_STAND2 = 1,
	AT_PUNCH1 = 2,
	AT_PUNCH2 = 3,
	AT_RECOVER = 4,
	AT_DEAD = 5,
};

static const enum leg_frames anim_legs[] = {
	AL_STAND,
	AL_WALK1,
	AL_WALK2,
	AL_WALK3,
	AL_WALK2,
	AL_WALK1,
	AL_STAND,
	AL_WALK4,
	AL_WALK5,
	AL_WALK6,
	AL_WALK5,
	AL_WALK4,
};

static struct texture tex_legs, tex_torso, tex_heads;

void
body_load_globals()
{
	texture_load(&tex_legs, FILE_LEGS1, FILE_LEGS1_LEN, 3, TF_PNG);
	texture_load(&tex_torso, FILE_TORSO, FILE_TORSO_LEN, 3, TF_PNG);
	texture_load(&tex_heads, FILE_HEADS, FILE_HEADS_LEN, 2, TF_PNG);
}

void
body_reset(struct body *b,
		uint32_t legcol,
		uint32_t torsocol,
		enum head h, float scale)
{
	b->leg_color = legcol;
	b->torso_color = torsocol;
	b->head = h;
	b->scale = scale;
}

static int
get_leg_animation(enum bstate bs, int ms)
{
	switch (bs) {
		case BS_WALK : return anim_legs[ms % ARR_SIZE(anim_legs)];
		case BS_DEAD : return AL_DEAD;
		case BS_IDLE :
		default : return AL_STAND;
	}
}

static int
get_torso_animation(enum bstate bs, int ms)
{
	switch (bs) {
		case BS_WALK : return ms & 4 ? AT_STAND1 : AT_STAND2;
		case BS_PUNCH1 : return AT_PUNCH1;
		case BS_PUNCH2 : return AT_PUNCH2;
		case BS_RECOVER : return AT_RECOVER;
		case BS_DEAD : return AT_DEAD;
		default : return AT_STAND1;
	}
}

static void
render_layer(uint32_t col, float scale)
{
	glColor3ubv((const GLubyte*) &col);

	glBegin(GL_QUADS);
	glTexCoord2i(0, 1); glVertex2f(-scale, -scale);
	glTexCoord2i(1, 1); glVertex2f( scale, -scale);
	glTexCoord2i(1, 0); glVertex2f( scale,  scale);
	glTexCoord2i(0, 0); glVertex2f(-scale,  scale);
	glEnd();
}

void
body_render(struct body *b)
{
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_2D);
	//glEnable(GL_ALPHA_TEST);

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glPushMatrix();
	glTranslatef(b->x, b->y, 0.0f);
	glRotatef(
			180.0f * b->dir / M_PI,
			0.0f, 0.0f, 1.0f);

	texture_bind(&tex_legs, get_leg_animation(b->state, b->ms));
	render_layer(b->leg_color, b->scale);

	texture_bind(&tex_torso, get_torso_animation(b->state, b->ms));
	render_layer(b->torso_color, b->scale);

	if (b->state != BS_DEAD) {
		texture_bind(&tex_heads, b->head);
		render_layer(0xffffffff, b->scale);
	}

	//glColor4ub(0xff, 0xff, 0xff, 0xff);

	glPopAttrib();
	glPopMatrix();
}

void
body_set_state(struct body *b, enum bstate bs, int ms)
{
	b->ms = ms;
	b->state = bs;
}

void
body_set_position(struct body *b, float x, float y, float dir)
{
	b->x = x;
	b->y = y;
	b->dir = dir;
}

