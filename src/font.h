#ifndef FONT_H
#define FONT_H
#include <stdint.h>

void font_load_globals();
void font_set_color(uint32_t hex);
void font_printf(int x, int y, const char *fmt, ...);

#endif

