#ifndef DIALOGTREE_H
#define DIALOGTREE_H
#include "story.h"
#include <stdint.h>

void dialog_set(struct dialog *root);
void dialog_unset();
void dialog_frame(int ms);
void dialog_render();

#endif

