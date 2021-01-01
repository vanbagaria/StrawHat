#include "SGE_GameState.h"
#include "SGE.h"
#include "SGE_Logger.h"
#include "string.h"

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

void SGE_SwitchToState(SGE_GameState *nextState)
{
	if(nextState == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to switch to NULL state!");
		return;
	}
	
	SGE_printf("\n");
	SGE_LogPrintLine(SGE_LOG_DEBUG, "Quitting State: \"%s\"...", SGE_GetCurrentState()->name);
	SGE_GetCurrentState()->quit();
	SGE_LogPrintLine(SGE_LOG_DEBUG, "Finished Quitting State.");
	
	SGE_SetStateFunctions(SGE_GetCurrentState(), nextState->name, nextState->init, nextState->quit, nextState->handleEvents, nextState->update, nextState->render);
	
	SGE_printf("\n");
	SGE_LogPrintLine(SGE_LOG_DEBUG, "Initializing state: \"%s\"...", SGE_GetCurrentState()->name);
	if(!SGE_GetCurrentState()->init())
	{
		SGE_GetEngineData()->isRunning = false;
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed Initializing State!");
	}
	else
	{
		SGE_LogPrintLine(SGE_LOG_DEBUG, "Finished Initializing State.\n");
	}
}