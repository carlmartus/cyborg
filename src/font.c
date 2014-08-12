#include "font.h"
#include "util.h"
#include "media.h"
#include <stdio.h>
#include <stdarg.h>
#include <GL/gl.h>

#define MAP_A 0
#define MAP_a 30
#define MAP_0 60

#define FONT_SIZE 16
#define GRID_SIZE 10
#define GRID_SHIFT 3
#define GRID_SIZE_INV (1.0f / (float) GRID_SIZE)

static struct texture tex_font;

void
font_load_globals()
{
	texture_load(&tex_font, FILE_FONT, FILE_FONT_LEN, 1, TF_PNG);
}

void
font_set_color(uint32_t hex)
{
	glColor3ubv((const GLubyte*) &hex);
}

static int
remap_font(char ch)
{
	if (ch >= 'A' && ch <= 'Z') return MAP_A + (ch - 'A');
	if (ch >= 'a' && ch <= 'z') return MAP_a + (ch - 'a');
	if (ch >= '0' && ch <= '9') return MAP_0 + (ch - '0');
	switch (ch) {
		case ' ' : return 26;
		case ':' : return 27;
		case '-' : return 28;
		case '/' : return 29;
		default : return 0;
	}
}

static void
draw_block(int x, int y, int bx, int by)
{
	float u0, v0, u1, v1;
	u0 = GRID_SIZE_INV * (float) bx;
	v0 = GRID_SIZE_INV * (float) by;
	u1 = u0 + GRID_SIZE_INV;
	v1 = v0 + GRID_SIZE_INV;

	glTexCoord2f(u0, v0); glVertex2i(x, y);
	glTexCoord2f(u1, v0); glVertex2i(x + FONT_SIZE, y);
	glTexCoord2f(u1, v1); glVertex2i(x + FONT_SIZE, y + FONT_SIZE);
	glTexCoord2f(u0, v1); glVertex2i(x, y + FONT_SIZE);
}

void
font_printf(int x, int y, const char *fmt, ...)
{
	int ch;
	char *itr;
	char buff[1000];

	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(buff, sizeof(buff), fmt, argptr);
	va_end(argptr);

	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	texture_bind(&tex_font, 0);

	itr = buff;
	glBegin(GL_QUADS);
	while (*itr != '\0') {
		ch = remap_font(*itr++);

		draw_block(
				x++,
				y,
				ch % GRID_SIZE,
				ch / GRID_SIZE);
		x += FONT_SIZE;
	}

	glEnd();
	glPopAttrib();
}

