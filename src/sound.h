#ifndef SOUND_H
#define SOUND_H
#include <stdint.h>

enum sfx {
	SFX_BLONG,
	SFX_HIT1,
	SFX_HIT2,
	SFX_HIT3,
	SFX_HIT4,
	SFX_HIT5,
	SFX_ONELINER1,
	SFX_SCREAM1,
	SFX_COUNT,
};

void sound_load_globals();
void sound_music(uint32_t file, uint32_t len);
void sound_music_stop();
void sound_queue(enum sfx sfx);

#endif

