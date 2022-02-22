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

/** 
 * \brief Returns a random integer between and including lowerRange and higherRange.
 * 
 * \param lowerRange Lowest random integer
 * \param higherRange Highest random integer
 * 
 * \return Integer value between lowerRange and higherRange
 */
int SGE_MathRandRangeInt(int lowerRange, int higherRange);

/**
 * \brief Truncates a given float to the given number of places after the decimal point.
 * 
 * \param value The float to truncate.
 * \param places The number of required digits after the decimal point.
 * 
 * \return The passed float with extra digits after the decimal point removed.
 */
float SGE_truncf(float value, float places);

#endif