#ifndef __SGE_GAMESTATE_H__
#define __SGE_GAMESTATE_H__

#include <stdbool.h>
#include <stddef.h>

/* Internally used state structure */
typedef struct
{
	char name[20];
	bool (*init)();
	void (*quit)();
	void (*handleEvents)();
	void (*update)();
	void (*render)();
} SGE_GameState;

/* Fills an SGE_GameState structure */
void SGE_SetStateFunctions(
	SGE_GameState *state, const char *name,
	bool (*init)(),
	void (*quit)(),
	void (*handleEvents)(),
	void (*update)(),
	void (*render)());

/* 
* Register a new state and add it to the state list.
* A state is a named collection of five functions that act as a sub-application.
* Only one state is active as the current state of the engine.
* The current state can be switched using the SGE_SwitchToState() function.
*/
void SGE_AddState(const char *name, bool (*init)(), void (*quit)(), void (*handleEvents)(), void (*update)(), void (*render)());

/* 
* Switch the current state to another state while freeing the current state's memory
* or by keeping the current state in memory.
*/
void SGE_SwitchToState(const char *nextStateName, bool quitCurrent);

/* Internally switches the current state */
void SGE_SwitchStates();

void SGE_InitState(SGE_GameState *state);
void SGE_QuitState(SGE_GameState *state);


/* Get the current state as an internal SGE_GameState */
SGE_GameState *SGE_GetState(const char *name);

/* Gets a string representing the list of registered states */
const char *SGE_GetStateNames();

/* Updates the string representing the list of registered states */
void SGE_PrintStates();

/* Gets the total number of registered states */
int SGE_GetStateCount();

/* 
* Internally sets a state's loaded flag.
* The loaded flag indicates whether a registered state's memory has been loaded or freed with init() or quit() respectively.
*/
void SGE_SetStateLoaded(const char *name, bool loaded);

/* Returns whether a state's memory has been loaded or freed with init() or quit() respectively. */
bool SGE_StateIsLoaded(const char *name);

/* Internally frees the memory for all registered states that are loaded */
void SGE_FreeLoadedStates();

/* Free's the actual list that maintains the registered states */
void SGE_FreeStateList();

typedef struct SGE_GUI_ControlList SGE_GUI_ControlList; // SGE_GUI.h

/* Returns the list of GUI controls for a particular state  */
SGE_GUI_ControlList *SGE_GetStateGUIList(const char *name);

#endif