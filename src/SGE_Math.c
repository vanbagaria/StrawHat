#include "SGE_Math.h"
#include <stdlib.h>
#include <SDL2/SDL.h>

int SGE_MathRandRangeInt(int lowerRange, int higherRange)
{
	higherRange++;
	return (rand() % (higherRange - lowerRange)) + lowerRange;
}

float SGE_truncf(float value, float places)
{
    return value - (SDL_fmodf(value, 1.0f / SDL_powf(10.0f, places)));
}

unsigned int SGE_GetTimerTime(SGE_Timer *timer)
{
	if(timer->paused)
	{
		return timer->pauseTime;
	}
	return SDL_GetTicks() - timer->startTime;
}

void SGE_StartTimer(SGE_Timer *timer)
{
	timer->startTime = SDL_GetTicks();
	timer->paused = false;
}

void SGE_PauseTimer(SGE_Timer *timer)
{
	timer->pauseTime = SDL_GetTicks() - timer->startTime;
	timer->paused = true;
}

void SGE_ResumeTimer(SGE_Timer *timer)
{
	timer->startTime = SDL_GetTicks() - timer->pauseTime;
	timer->paused = false;
}