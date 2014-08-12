#include "sound.h"
#include "util.h"
#include "media.h"
#include <stdlib.h>
#include <SDL/SDL.h>

// Maximum amount of sounds to be played at once
#define QUEUE_MAX 3

static const struct lib_file {
	uint32_t offset, length;
} lib_files[] = {
	[SFX_BLONG] = { FILE_BLONG, FILE_BLONG_LEN },
	[SFX_HIT1] = { FILE_HIT1, FILE_HIT1_LEN },
	[SFX_HIT2] = { FILE_HIT2, FILE_HIT2_LEN },
	[SFX_HIT3] = { FILE_HIT3, FILE_HIT3_LEN },
	[SFX_HIT4] = { FILE_HIT4, FILE_HIT4_LEN },
	[SFX_HIT5] = { FILE_HIT5, FILE_HIT5_LEN },
	[SFX_SCREAM1] = { FILE_SCREAM1, FILE_SCREAM1_LEN },
	[SFX_ONELINER1] = { FILE_ONELINER1, FILE_ONELINER1_LEN },
};

static struct sfx_lib {
	uint8_t *buf;
	uint32_t length;
} lib[SFX_COUNT];

static struct queue {
	int on;
	uint8_t *itr, *end;
} queue[QUEUE_MAX];

static SDL_AudioSpec audio_spec;
static uint32_t music_length;
static uint8_t *music_buf, *music_itr, *music_end;

static void
audio_callback(void *data, uint8_t *out, int len)
{
	uint8_t *mput, *mend, *qitr, *qend;
	if (music_itr != NULL) {
		mput = out;
		mend = music_itr + len;

		if (mend > music_end) mend = music_end;

		while (music_itr < mend) *mput++ = *music_itr++;
	}

	int i;
	for (i=0; i<QUEUE_MAX; i++) {
		if (!queue[i].on) continue;

		mput = out;
		qitr = queue[i].itr;
		qend = qitr + len;
		if (qend > queue[i].end) {
			qend = queue[i].end;
			queue[i].on = 0;
		}

		while (qitr < qend) *mput++ += *qitr++;
		queue[i].itr = qitr;
	}
}

static void
setup_audio()
{

	SDL_AudioSpec desire;
	desire.freq = 8000;
	desire.format = AUDIO_S16;
	desire.channels = 1;
	desire.samples = 1024;
	desire.callback = audio_callback;

	if (SDL_OpenAudio(&desire, &audio_spec) < 0) {
		util_crash("Cannot open audio device");
	}
}

static struct sfx_lib
load_wav(uint32_t offset, uint32_t len)
{
	struct sfx_lib ret;
	void *buf = util_chunk(offset, len);
	SDL_RWops *rwops = SDL_RWFromMem(buf, len);

	if (SDL_LoadWAV_RW(rwops, 0, &audio_spec,
				&ret.buf, &ret.length) == 0) {
		util_crash("Cannot load wav music");
	}

	free(buf);
	SDL_FreeRW(rwops);
	return ret;
}

static void
setup_library()
{
	int i;
	for (i=0; i<SFX_COUNT; i++) {
		lib[i] = load_wav(
				lib_files[i].offset,
				lib_files[i].length);
	}
}

void
sound_load_globals()
{
	int i;
	for (i=0; i<QUEUE_MAX; i++) queue[i].on = 0;
	music_itr = NULL;
	setup_audio();
	setup_library();
}

void
sound_music(uint32_t file, uint32_t len)
{
	struct sfx_lib sf;

	sf = load_wav(file, len);

	music_buf = sf.buf;
	music_length = sf.length;
	music_itr = music_buf;
	music_end = music_itr + music_length;
	SDL_PauseAudio(0);
}

void
sound_music_stop()
{
	music_itr = NULL;
	free(music_buf);
	music_buf = NULL;
	music_length = 0;
}

static int
get_free_queue()
{
	int i;
	for (i=0; i<QUEUE_MAX; i++) if (!queue[i].on) return i;
	return -1;
}

void
sound_queue(enum sfx sfx)
{
	int q = get_free_queue();
	if (q < 0) return;

	queue[q].on = 1;
	queue[q].itr = lib[sfx].buf;
	queue[q].end = queue[q].itr + lib[sfx].length;
}

