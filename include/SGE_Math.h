#ifndef __SGE_MATH_H__
#define __SGE_MATH_H__

#include <stdbool.h>

typedef struct SGE_Timer
{
	unsigned int startTime;
	unsigned int pauseTime;
	bool paused;
} SGE_Timer;

unsigned int SGE_GetTimerTime(SGE_Timer *timer);
void SGE_StartTimer(SGE_Timer *timer);
void SGE_PauseTimer(SGE_Timer *timer);
void SGE_ResumeTimer(SGE_Timer *timer);

int SGE_MathRandRangeInt(int lowerRange, int higherRange);

#endif