#include "splash.h"
#include "util.h"
#include "media.h"
#include "input.h"
#include "story.h"
#include "font.h"
#include "config.h"
#include <stdio.h>
#include <GL/gl.h>

#define DEFAULT_FADE 500

static int do_clouds;
static const char *text;
static int fade_ms;
static struct texture background, clouds;
static uint32_t counter;
static uint32_t cloud_colors;

struct text_template {
	int x, y;
	const char *text;
};

static const struct text_template *text_template = NULL;

static const struct text_template const intro_text[] = {
	{ 20, 20, "Cyborg" },
	{ 20, 40, "4chan Amateur Game Dev General" },
	{ 20, 60, "Boot comp June 2013" },

	{ 20, 100, "Complete each level by beating all" },
	{ 20, 120, "the enemies" },

	{ 20, 340, "Keys:" },
	{ 20, 360, "Arrows - Move" },
	{ 20, 380, "Q/W - Punch with right/left arm" },
	{ 20, 400, "F4 - Toggle fullscreen" },
	{ 20, 420, "ESC - Quit" },
	{ 20, 440, "Space - Skip splash screen" },
	{ 0, 0, NULL },
};

static const struct text_template const outro_text[] = {
	{ 240, 140, "GOOD JOB" },
	{ 140, 340, "GAME OVER" },
	{ 0, 0, NULL },
};

void
splash_load_global()
{
	texture_load(&clouds, FILE_CLOUDS, FILE_CLOUDS_LEN, 1, TF_JPEG);
}

void
splash_set(const struct splash *splash)
{
	uint8_t flags;

	// Defaults
	counter = 0;
	fade_ms = DEFAULT_FADE;
	text = NULL;
	do_clouds = 0;

	flags = splash->flags;
	cloud_colors = splash->cloud_colors;

	if (flags & SPLASH_IMAGE) {
		texture_load(&background,
				splash->image_offset, splash->image_len, 1, TF_JPEG);
	}

	switch (splash->text_template) {
		default :
		case SPLASH_TEXT_TEMPLATE_INTRO : text_template = intro_text; break;
		case SPLASH_TEXT_TEMPLATE_OUTRO : text_template = outro_text; break;
	}

	if (flags & SPLASH_CLOUDS) do_clouds = 1;
	if (flags & SPLASH_FADE) fade_ms = splash->fade_ms;
}

void
splash_free()
{
	texture_free(&background);
}

static void
frame(int ms)
{
	counter += ms;

	if (input_get_trigger(IK_SKIP)) story_next();
}

static float
get_cloud_prog(uint32_t time)
{
	return 0.0001f * (float) time;
}

static void
draw_fonts()
{
	const struct text_template *itr = text_template;

	while (itr->text != NULL) {
		font_printf(
				itr->x,
				itr->y,
				itr->text);
		itr++;
	}
}

static void
render()
{
	float cp;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	cp = get_cloud_prog(counter);

	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_2D);
	texture_bind(&background, 0);

	glBegin(GL_QUADS);
	glColor4ub(0xff, 0xff, 0xff, 0xff);
	glTexCoord2i(0, 1); glVertex2i(-1, -1);
	glTexCoord2i(1, 1); glVertex2i( 1, -1);
	glTexCoord2i(1, 0); glVertex2i( 1,  1);
	glTexCoord2i(0, 0); glVertex2i(-1,  1);
	glEnd();

	if (do_clouds) {
		glEnable(GL_BLEND);
		texture_bind(&clouds, 0);

		glBegin(GL_QUADS);
		glColor4ub(
				(cloud_colors >> 24),
				(cloud_colors >> 16),
				(cloud_colors >> 8),
				0xff);

		glTexCoord2f(cp + 0.0f, 1.0f); glVertex2i(-1, -1);
		glTexCoord2f(cp + 1.0f, 1.0f); glVertex2i( 1, -1);
		glColor4ub(0, 0, 0, 0);
		glTexCoord2f(cp + 1.0f, 0.0f); glVertex2i( 1,  0);
		glTexCoord2f(cp + 0.0f, 0.0f); glVertex2i(-1,  0);
		glEnd();
	}

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glDisable(GL_TEXTURE_2D);
	glColor4ub(0xff, 0xff, 0xff, 0xff);
	util_screen_matrix();
	draw_fonts();
	glPopMatrix();

	glPopAttrib();
}

void
splash_step(int ms)
{
	frame(ms);
	render();
}

