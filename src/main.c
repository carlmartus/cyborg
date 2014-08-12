#include <stdint.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <SDL/SDL.h>
#include "config.h"
#include "story.h"
#include "splash.h"
#include "map.h"
#include "body.h"
#include "input.h"
#include "font.h"
#include "util.h"
#include "sound.h"

static int run;
static const uint32_t tickmin = 1000 / FPS_MAX;
static SDL_Surface *screen;

enum input_key
key_translate(SDLKey key)
{
	switch (key) {
		case SDLK_LEFT : return IK_NAV_LEFT;
		case SDLK_UP : return IK_NAV_FORWARD;
		case SDLK_RIGHT : return IK_NAV_RIGHT;
		case SDLK_DOWN : return IK_NAV_BACKWARD;

		case SDLK_F4 : return IK_FULLSCREEN;
		case SDLK_ESCAPE : return IK_QUIT;
		case SDLK_SPACE : return IK_SKIP;
		case SDLK_q : return IK_PUNCH1;
		case SDLK_w : return IK_PUNCH2;
		case SDLK_e : return IK_KICK;
		default : return IK_NONE;
	}
}

static void
set_kstate(SDLKey key, int state)
{
	enum input_key ik = key_translate(key);
	if (ik != IK_NONE) {
		input_set_key(ik, state);
	}
}

static void
check_main_keys()
{
	if (input_get_trigger(IK_QUIT)) run = 0;
	if (input_get_trigger(IK_FULLSCREEN)) SDL_WM_ToggleFullScreen(screen);
}

static void
update_mouse(SDL_Event event)
{
	input_set_mouse(
			event.motion.x,
			event.motion.y,
			event.motion.xrel,
			event.motion.yrel);
}

static void
events()
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_KEYDOWN : set_kstate(event.key.keysym.sym, 1); break;
			case SDL_KEYUP : set_kstate(event.key.keysym.sym, 0); break;
			case SDL_MOUSEMOTION : update_mouse(event); break;

			case SDL_QUIT :
				run = 0;
				break;
		}
	}
}

int
main(int argc, char **argv)
{
	uint32_t tick0, tick1, tickdiff;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

	// Video
	//SDL_WM_GrabInput(SDL_GRAB_ON);
	SDL_ShowCursor(SDL_DISABLE);
	screen = SDL_SetVideoMode(640, 480, 32, SDL_OPENGL);
	SDL_WM_SetCaption("Cyborg", NULL);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glAlphaFunc(GL_GEQUAL, 0.5);
	SDL_GL_SwapBuffers();
	//SDL_Delay(500);

	font_load_globals();
	splash_load_global();
	map_load_globals();
	body_load_globals();
	input_reset();
	sound_load_globals();

	run = 1;

	if (argc > 1) {
		int start_level = atoi(argv[1]);
		printf("start level %d\n", start_level);
		story_set(start_level);
	} else {
		story_set(0);
	}

	tick1 = SDL_GetTicks();
	while (run) {

		events();
		check_main_keys();

		glClear(GL_COLOR_BUFFER_BIT);
		story_step(tickmin);
		overlay_render(tickmin);
		SDL_GL_SwapBuffers();
		input_step();

		tick0 = tick1;
		tick1 = SDL_GetTicks();

		tickdiff = tick1 - tick0;
		if (tickdiff < tickmin) {
			//printf("Sleep %d\n", tickmin - tickdiff);
			SDL_Delay(tickmin - tickdiff);
		}
	}

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	SDL_GL_SwapBuffers();

	SDL_Quit();
	return 0;
}

