#include <SGE_Audio.h>
#include <SGE_Logger.h>

SGE_Sfx *SGE_LoadSfx(const char *file)
{
    SGE_Sfx *sfx = Mix_LoadWAV(file);
    if(sfx == NULL)
    {
        SGE_LogPrintLine(SGE_LOG_ERROR, "Failed to load SFX: %s", file);
        SGE_LogPrintLine(SGE_LOG_ERROR, "SDL_Mixer Error: %s", Mix_GetError());
    }
    return sfx;
}

SGE_Music *SGE_LoadMusic(const char *file)
{
    SGE_Music *music = Mix_LoadMUS(file);
    if(music == NULL)
    {
        SGE_LogPrintLine(SGE_LOG_ERROR, "Failed to load Music Stream: %s", file);
        SGE_LogPrintLine(SGE_LOG_ERROR, "SDL_Mixer Error: %s", Mix_GetError());
    }
    return music;
}

void SGE_PlaySfx(SGE_Sfx *sfx, int loops)
{
    Mix_PlayChannel(-1, sfx, loops);
}

void SGE_PlayMusic(SGE_Music *music, int loops)
{
    Mix_PlayMusic(music, loops);
}

void SGE_FreeSfx(SGE_Sfx *sfx)
{
    Mix_FreeChunk(sfx);
}

void SGE_FreeMusic(SGE_Music *music)
{
    Mix_FreeMusic(music);
}