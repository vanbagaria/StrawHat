#include "SGE_Math.h"
#include <stdlib.h>

int SGE_MathRandRangeInt(int lowerRange, int higherRange)
{
	higherRange++;
	return (rand() % (higherRange - lowerRange)) + lowerRange;
}