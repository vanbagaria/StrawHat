#ifndef __SGE_AUDIO_H__
#define __SGE_AUDIO_H__

#include "SDL_mixer.h"

/**
 * \brief Represents a small sound effect played once or looped a few times.
 * 
 */
typedef Mix_Chunk SGE_Sfx;

/**
 * \brief Represents a long music stream set to play once, and it continues to play until the end or is looped.
 * 
 */
typedef Mix_Music SGE_Music;

/**
 * \brief Loads a sound effect from an audio file.
 * 
 * \param file Path to an audio file to load into memory.
 * \return Pointer to loaded sound effect.
 */
SGE_Sfx *SGE_LoadSfx(const char *file);

/**
 * \brief Loads a music stream from a file.
 * 
 * \param file Path to an audio file to stream from disk.
 * \return Pointer to a loaded music stream that will be streamed when played.
 */
SGE_Music *SGE_LoadMusic(const char *file);

/**
 * \brief Plays a sound effect and repeats 'loops' number of times.
 * 
 * \param sfx A pointer to a loaded sound effect. 
 * \param loops The number of times to repeat playback of the sound
 *              where, loop = 0 means play once and loop = -1 means loop indefinitely.
 */
void SGE_PlaySfx(SGE_Sfx *sfx, int loops);

/**
 * \brief Plays a music stream and repeats 'loops' number of times.
 * 
 * \param music A pointer to a loaded music stream.
 * \param loops The number of times to repeat playback of the music stream
 *              where, loop = 0 means play once and loop = -1 means loop indefinitely.
 */
void SGE_PlayMusic(SGE_Music *music, int loops);

/**
 * \brief Free a loaded sound effect.
 * 
 * \param sfx Pointer to a loaded sound effect.
 */
void SGE_FreeSfx(SGE_Sfx *sfx);

/**
 * \brief Frees a loaded music stream.
 * 
 * \param music Pointer to a loaded music stream.
 */
void SGE_FreeMusic(SGE_Music *music);

#endif