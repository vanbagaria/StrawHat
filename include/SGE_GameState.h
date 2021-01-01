#ifndef __SGE_GAMESTATE_H__
#define __SGE_GAMESTATE_H__

#include <stdbool.h>
#include <stddef.h>

typedef struct
{
	char name[20];
	bool (*init)();
	void (*quit)();
	void (*handleEvents)();
	void (*update)();
	void (*render)();
} SGE_GameState;

void SGE_SetStateFunctions(
	SGE_GameState *state, const char *name,
	bool (*init)(),
	void (*quit)(),
	void (*handleEvents)(),
	void (*update)(),
	void (*render)());

void SGE_SwitchToState(SGE_GameState *nextState);

#endif