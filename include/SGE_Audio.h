#ifndef __SGE_AUDIO_H__
#define __SGE_AUDIO_H__

#include "SDL_mixer.h"

/* Represents a small sound effect played once or looped a few times */
typedef Mix_Chunk SGE_Sfx;

/* Represents a long music stream set to play once and it continues to play until the end or is looped */
typedef Mix_Music SGE_Music;

/* Load a sound effect from a file */
SGE_Sfx *SGE_LoadSfx(const char *file);

/* Load a music stream from a file */
SGE_Music *SGE_LoadMusic(const char *file);

/* Play a sound effect and repeat 'loops' number of times, with loop = 0 meaning play once and loop = -1 meaning loop indefinitely */
void SGE_PlaySfx(SGE_Sfx *sfx, int loops);

/* Play a music stream and repeat 'loops' number of times, with loop = 0 meaning play once and loop = -1 meaning loop indefinitely */
void SGE_PlayMusic(SGE_Music *music, int loops);

/* Free a sound effect */
void SGE_FreeSfx(SGE_Sfx *sfx);

/* Free a music stream */
void SGE_FreeMusic(SGE_Music *music);

#endif