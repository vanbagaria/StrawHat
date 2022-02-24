#include "SGE.h"
#include "SGE_GameState.h"
#include "SGE_Logger.h"
#include "SGE_GUI.h"
#include <stdio.h>
#include <string.h>

/* The internal list of registered states */
typedef struct SGE_StateList {
	SGE_GameState state;
	SGE_GUI_ControlList guiList;
	bool loaded;
	struct SGE_StateList *next;
} SGE_StateList;

static SGE_StateList *stateList = NULL;

/* This is not NULL when a SGE_SwitchToState() call is made */
static SGE_GameState *nextSwitchState = NULL;
/* Flag for whether to free the current state when switching to next state */
static bool switchQuitCurrent = false;

/* A string representing the names of all registered states */
static char listString[1000];
static int stateCount = 0;

/* Zeros a state list node */
static void SGE_InitStateGUIList(SGE_StateList *state)
{
	state->guiList.buttonCount = 0;
	state->guiList.checkBoxCount = 0;
	state->guiList.sliderCount = 0;
	state->guiList.labelCount = 0;
	state->guiList.listBoxCount = 0;
	state->guiList.textInputBoxCount = 0;
	
	state->guiList.panelCount = 0;
}

/* Returns the given state as a state list node */
static SGE_StateList *SGE_GetStateAsList(const char *name)
{
	SGE_StateList *current = stateList;
	if(current == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "%s: State list is empty!", __FUNCTION__);
		return NULL;
	}

	while(current != NULL)
	{
		if(strcmp(current->state.name, name) == 0)
		{
			return current;
		}
		current = current->next;
	}

	return NULL;
}

SGE_GUI_ControlList *SGE_GetStateGUIList(const char *name)
{
	SGE_StateList *currentState = SGE_GetStateAsList(name);
	if(currentState == NULL)
	{
		return NULL;
	}
	return &currentState->guiList;
}

const char *SGE_GetStateNames()
{
	SGE_PrintStates();
	return listString;
}

int SGE_GetStateCount()
{
	return stateCount;
}

/* Fallbacks used when a state is created with NULL function pointers */
bool SGE_FallbackInit()
{
	SGE_LogPrintLine(SGE_LOG_WARNING, "Init is set to NULL, using fallback!");
	return true;
}

void SGE_FallbackQuit()
{
	SGE_LogPrintLine(SGE_LOG_WARNING, "Quit is set to NULL, using fallback!");
}

void SGE_FallbackHandleEvents(){}
void SGE_FallbackUpdate(){}
void SGE_FallbackRender(){}

void SGE_SetStateFunctions(SGE_GameState *state, const char *name, bool (*init)(), void (*quit)(), void (*handleEvents)(), void (*update)(), void (*render)())
{
	if(state == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to set functions for NULL state!");
		return;
	}
	
	strcpy(state->name, name);
	state->init = (init == NULL) ? SGE_FallbackInit : init;
	state->quit = (quit == NULL) ? SGE_FallbackQuit : quit;
	state->handleEvents = (handleEvents == NULL) ? SGE_FallbackHandleEvents : handleEvents;
	state->update = (update == NULL) ? SGE_FallbackUpdate : update;
	state->render = (render == NULL) ? SGE_FallbackRender : render;
}

void SGE_SwitchToState(const char *nextStateName, bool quitCurrent)
{
	SGE_GameState *nextState = SGE_GetState(nextStateName);
	if(nextState == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to switch to NULL state!");
		return;
	}
	nextSwitchState = nextState;
	switchQuitCurrent = quitCurrent;
}

void SGE_SwitchStates()
{
	if(nextSwitchState == NULL)
	{
		return;
	}

	if(switchQuitCurrent)
	{
		SGE_QuitState(SGE_GetCurrentState());
	}
	
	SGE_SetStateFunctions(SGE_GetCurrentState(), nextSwitchState->name, nextSwitchState->init, nextSwitchState->quit, nextSwitchState->handleEvents, nextSwitchState->update, nextSwitchState->render);
	SGE_GUI_UpdateCurrentState(nextSwitchState->name);

	if(!SGE_StateIsLoaded(SGE_GetCurrentState()->name))
	{
		SGE_InitState(SGE_GetCurrentState());
	}

	nextSwitchState = NULL;
}

void SGE_InitState(SGE_GameState *state)
{
	SGE_LogPrintLine(SGE_LOG_INFO, "Initializing state: \"%s\"...", state->name);
	if(!state->init())
	{
		SGE_Quit();
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed Initializing State!");
	}
	else
	{
		SGE_SetStateLoaded(state->name, true);
		SGE_LogPrintLine(SGE_LOG_INFO, "Finished Initializing State.");
		SGE_printf(SGE_LOG_DEBUG, "\n");
	}
}

void SGE_QuitState(SGE_GameState *state)
{
	SGE_LogPrintLine(SGE_LOG_INFO, "Quitting state: \"%s\"...", state->name);
	state->quit();
	SGE_GUI_FreeState(state->name);
	SGE_SetStateLoaded(state->name, false);
	SGE_LogPrintLine(SGE_LOG_INFO, "Finished Quitting State.");
	SGE_printf(SGE_LOG_DEBUG, "\n");
}

/* Prints the list of currently registered game states to debug output */
void SGE_PrintStates()
{
	SGE_StateList *current = stateList;
	if(current == NULL)
	{
		//SGE_LogPrintLine(SGE_LOG_DEBUG, "Registered SGE States: {}");
		return;
	}
	
	sprintf(listString, "{");
	while(current->next != NULL)
	{
		strcat(listString, current->state.name);
		strcat(listString, ", ");
		current = current->next;
	}
	strcat(listString, current->state.name);
	strcat(listString, "}");
	//SGE_LogPrintLine(SGE_LOG_DEBUG, "Registered SGE States: %s", listString);
}

void SGE_SetStateLoaded(const char *name, bool loaded)
{
	SGE_StateList *state = SGE_GetStateAsList(name);
	if(state != NULL)
	{
		state->loaded = loaded;
	}
	else
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "%s: \"%s\" not in state list!", __FUNCTION__, name);
	}
}

bool SGE_StateIsLoaded(const char *name)
{
	SGE_StateList *state = SGE_GetStateAsList(name);
	if(state != NULL)
	{
		return state->loaded;
	}

	SGE_LogPrintLine(SGE_LOG_WARNING, "%s: \"%s\" not in state list!", __FUNCTION__, name);
	return false;
}

void SGE_AddState(const char *name, bool (*init)(), void (*quit)(), void (*handleEvents)(), void (*update)(), void (*render)())
{
	SGE_StateList *current = stateList;

	/* List is empty, so add first state */
	if(current == NULL)
	{
		current = (SGE_StateList*)malloc(sizeof(SGE_StateList));

		SGE_SetStateFunctions(&current->state, name, init, quit, handleEvents, update, render);
		SGE_InitStateGUIList(current);
		current->loaded = false;
		current->next = NULL;
		
		stateList = current;
	}
	else /* Add state to the end of list */
	{
		SGE_StateList *exists = SGE_GetStateAsList(name);
		if(exists)
		{
			SGE_LogPrintLine(SGE_LOG_WARNING, "Failed to add state \"%s\", name already exists!", name);
			return;
		}

		while(current->next != NULL)
		{
			current = current->next;
		}
		current->next = (SGE_StateList*)malloc(sizeof(SGE_StateList));
		current = current->next;

		SGE_SetStateFunctions(&current->state, name, init, quit, handleEvents, update, render);
		SGE_InitStateGUIList(current);
		current->loaded = false;
		current->next = NULL;
	}
	
	stateCount++;
	SGE_LogPrintLine(SGE_LOG_INFO, "Registered state: %s", name);
}

SGE_GameState *SGE_GetState(const char *name)
{
	SGE_StateList *state = SGE_GetStateAsList(name);
	if(state == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "%s: \"%s\" not in state list!", __FUNCTION__, name);
		return NULL;
	}
	return &state->state;
}

void SGE_FreeLoadedStates()
{
	SGE_StateList *current = stateList;
	if(current == NULL)
	{
		return;
	}

	SGE_LogPrintLine(SGE_LOG_DEBUG, "Quitting loaded GameStates...");
	while(current != NULL)
	{
		if(current->loaded)
		{
			SGE_QuitState(&current->state);
		}
		current = current->next;
	}
	SGE_LogPrintLine(SGE_LOG_DEBUG, "Finished quitting loaded GameStates...");
	SGE_printf(SGE_LOG_DEBUG, "\n");
}

void SGE_FreeStateList()
{
	SGE_StateList *current = stateList;
	if(current == NULL)
	{
		return;
	}

	SGE_StateList *tempNext = current->next;
	while(current != NULL)
	{
		tempNext = current->next;
		SGE_LogPrintLine(SGE_LOG_DEBUG, "Unregistered state: %s", current->state.name);
		free(current);
		current = tempNext;
	}
}