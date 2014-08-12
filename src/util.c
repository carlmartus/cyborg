#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <GL/gl.h>
#include <math.h>
#include "util.h"
#include "media.h"
#include "config.h"

#define ALPHA_COLOR (0xff00ff00)

//static uint32_t overlay_color = 0x44667799;
static uint32_t overlay_color = 0x00ffffff;
static int overlay_max, overlay_ms = 0;

static int rain_on;
static uint32_t rain_counter;
static struct texture rain_texture;

static void
clear_no_colors(SDL_Surface *s)
{
	int x, y;
	uint32_t *itr;

	for (y=0; y<s->h; y++) {
		itr = s->pixels;
		itr += s->w*y;

		for (x=0; x<s->w; x++) {
			if (*itr == ALPHA_COLOR) *itr = 0;
			itr++;
		}
	}
}

void
texture_load(struct texture *tx, uint32_t offset, uint32_t len,
		int grid, enum texture_format fmt)
{
	int bind, x, y, cellw, cellh;
	void *buf;
	SDL_RWops *ops;
	SDL_Surface *cell;
	SDL_Rect rect;
	SDL_Surface *surf;

	tx->grid = grid;
	tx->count = grid * grid;

	buf = util_chunk(offset, len);
	ops = SDL_RWFromMem(buf, len);

	switch (fmt) {
		case TF_JPEG :
			surf = IMG_LoadJPG_RW(ops);
			break;

		case TF_PNG :
			surf = IMG_LoadPNG_RW(ops);
			break;

		default :
			util_crash("Unknown format");
			return;
	}

	if (surf == NULL) util_crash("Cannot open image");

	glGenTextures(tx->count, &tx->opengl);

	cellw = surf->w / grid;
	cellh = surf->h / grid;

	for (y=0; y<grid; y++) {
		for (x=0; x<grid; x++) {

			cell = SDL_CreateRGBSurface(SDL_SWSURFACE,
					cellw, cellh, 32,
					0xff,
					0xff00,
					0xff0000,
					0xff000000);
			SDL_FillRect(cell, NULL, 0xffffffff);

			rect.x = x*cellw;
			rect.y = y*cellh;
			rect.w = cellw;
			rect.h = cellh;

			SDL_BlitSurface(surf, &rect, cell, NULL);
			clear_no_colors(cell);

			bind = tx->opengl + y*grid + x;
			glBindTexture(GL_TEXTURE_2D, bind);

			glTexImage2D(GL_TEXTURE_2D, 0, 4,
					cell->w, cell->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, cell->pixels);

			/*
			glTexImage2D(GL_TEXTURE_2D, 0, 4,
					surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels);*/

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			SDL_FreeSurface(cell);
		}
	}

	SDL_FreeSurface(surf);
	SDL_FreeRW(ops);
	free(buf);
}

void
texture_free(struct texture *tx)
{
	glDeleteTextures(1, &tx->opengl);
	tx->opengl = 0;
}

void
texture_bind(struct texture *tx, int frame)
{
	glBindTexture(GL_TEXTURE_2D, tx->opengl + frame);
}

void
util_screen_matrix()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, SCREEN_W, SCREEN_H, 0.0, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
}

void
util_load_globals()
{
	texture_load(&rain_texture, FILE_RAIN, FILE_RAIN_LEN, 1, TF_PNG);
}

void*
util_chunk(uint32_t offset, uint32_t len)
{
	FILE *fd;
	void *buf;

	buf = malloc(len);
	if (buf == NULL) util_crash("No memory");

	fd = fopen(MEDIA_FILE, "rb");
	if (fd == NULL) util_crash("Cannot open resource file");

	fseek(fd, offset, SEEK_SET);
	if (fread(buf, len, 1, fd) != 1) util_crash("Cannot read from file");

	fclose(fd);
	return buf;
}

void
util_crash(const char *reason)
{
	printf("CRASH [%s]\n", reason);
	exit(1);
}

void
util_render_block()
{
	glBegin(GL_QUADS);
	glTexCoord2i(0, 0); glVertex2i(-1, -1);
	glTexCoord2i(1, 0); glVertex2i( 1, -1);
	glTexCoord2i(1, 1); glVertex2i( 1,  1);
	glTexCoord2i(0, 1); glVertex2i(-1,  1);
	glEnd();
}

int
util_rng()
{
	return rand();
}

void
overlay_set(const struct overlay *ol)
{
	rain_counter = 0;
	rain_on = 0;
	overlay_set_fade(ol->fadein);
	overlay_color = ol->color;

	if (ol->flags & OVERLAY_RAIN) {
		rain_on = 1;
	}
}

void
overlay_set_fade(int timeout)
{
	overlay_max = timeout;
	overlay_ms = overlay_max;
}

void
overlay_render(int ms)
{
	float mul;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_BLEND);

	if (overlay_ms > 0) {
		overlay_ms -= ms;
		if (overlay_ms < 0) overlay_ms = 0;

		mul = (float) overlay_ms / (float) overlay_max;

		glColor4ub(0, 0, 0, (unsigned char) (255.0f * mul));
		util_render_block();
	}

	//glColor4ubv((const GLubyte*) &overlay_color);
	glColor4ub(
			overlay_color >> 16,
			overlay_color >> 8,
			overlay_color,
			overlay_color >> 24);
	util_render_block();

	if (rain_on) {
		float rain_v = rain_counter * 0.001f;
		rain_v -= floorf(rain_v);

		glEnable(GL_TEXTURE_2D);
		texture_bind(&rain_texture, 0);

		util_render_block();
		/*
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f,	rain_v);			glVertex2i(-1, -1);
		glTexCoord2f(10.0f,	rain_v);			glVertex2i( 1, -1);
		glTexCoord2f(10.0f,	rain_v + 10.0f);	glVertex2i( 1,  1);
		glTexCoord2f(0.0f,	rain_v + 10.0f);	glVertex2i(-1,  1);
		glEnd();*/

		rain_counter += ms;
	}

	glPopAttrib();
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

