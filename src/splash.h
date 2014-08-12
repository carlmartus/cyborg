#ifndef SPLASH_H
#define SPLASH_H
#include "story.h"

void splash_load_global();
void splash_set(const struct splash *splash);
void splash_free();
//void splash_frame(int ms);
//void splash_render();
void splash_step(int ms);

#endif

