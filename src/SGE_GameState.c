#include "SGE.h"
#include "SGE_GameState.h"
#include "SGE_Logger.h"
#include "SGE_GUI.h"
#include <stdio.h>
#include <string.h>

/* A list of a state's GUI controls, defined in SGE_GUI.c */

typedef struct SGE_GUI_ControlList SGE_GUI_ControlList;
SGE_GUI_ControlList *SGE_CreateGUIControlList();
void SGE_DestroyGUIControlList(SGE_GUI_ControlList *controls);
void SGE_GUI_FreeControlList(SGE_GUI_ControlList *controls);

/* Used to update the current control list pointer in SGE_GUI.c */
void SGE_GUI_UpdateCurrentState(const char *nextState);

/**
 * \brief Internally used game state structure.
 * 
 */
typedef struct
{
	char name[20]; /**< The unique name of the state */
	SGE_GUI_ControlList* controls; /**< List of state GUI controls */
	bool loaded; /**< True if the state's init function was called */

	bool (*init)(); /**< The state init function */
	void (*quit)(); /**< The state quit function */
	void (*handleEvents)(); /**< The state event handling function */
	void (*update)(); /**< The state update function */
	void (*render)(); /**< The state render function */
} SGE_GameState;

/* The internal list of registered states */
static SGE_LinkedList *stateList = NULL;

/* This is not NULL when a SGE_SwitchToState() call is made */
static SGE_GameState *nextSwitchState = NULL;

/* Flag for whether to free the current state when switching to next state */
static bool switchQuitCurrent = false;

/* A string representing the names of all registered states */
static char listString[1000];

/**
 * \brief Callback function for SGE_LLProcess() that finds a given state in the state list.
 * 
 * \param list The state list.
 * \param currentNode The current state when iterating over the list.
 * \param processData The name of the state to find.
 * \return The address of the SGE_GameState for the given name or NULL if not found in list.
 */
static void *SGE_FindState(SGE_LinkedList *list, SGE_LLNode* currentNode, void *processData)
{
	SGE_GameState *currentListState = currentNode->data;
	const char *stateName = processData;

	if(strcmp(currentListState->name, stateName) == 0)
	{
		// Stop LLProcess() from iterating furthur by returning non NULL value.
		return currentNode->data;
	}

	// Keep iterating over the list by returning NULL
	return NULL;
}

/**
 * \brief Returns the address of a registered state from the state list.
 * 
 * \param name The name of a registered state.
 * \return The address of the internal SGE_GameState strucuture.
 */
static SGE_GameState *SGE_GetStateFromList(const char *name)
{
	SGE_GameState *state = SGE_LLProcess(stateList, SGE_FindState, (void*)name);
	if(state == NULL)
	{
		// SGE_LogPrintLine(SGE_LOG_WARNING, "%s: State not in list!", __FUNCTION__);
		return NULL;
	}
	return state;
}

bool SGE_StateIsRegistered(const char *name)
{
	if(!stateList)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "%s(): State list is not initialized.", __FUNCTION__);
		return false;
	}

	if(SGE_LLProcess(stateList, SGE_FindState, (void*)name))
	{
		return true;
	}
	return false;
}

/**
 * \brief Returns the SGE_GUI_ControlList structure for a registered state.
 * 
 * \param name The name of the registered state whose GUI control list should be returned.
 * \return The address of the GUI control list of the registered state.
 */
SGE_GUI_ControlList *SGE_GetStateGUIControlList(const char *name)
{
	SGE_GameState *state = SGE_GetStateFromList(name);
	if(state == NULL)
	{
		return NULL;
	}
	return state->controls;
}

/**
 * \brief Callback for LLProcess() that prints a state's name to the state list string.
 * 
 * \param list The state list.
 * \param currentNode The current state when iterating with LLProcess()
 * \param processData Unused.
 * \return NULL to keep iterating over entire list.
 */
static void *SGE_PrintStateName(SGE_LinkedList *list, SGE_LLNode *currentNode, void *processData)
{
	SGE_GameState *currentListState = currentNode->data;
	strcat(listString, currentListState->name);
	if(currentNode->next != NULL)
	{
		strcat(listString, ", ");
	}
	return NULL;
}

/**
 * \brief Updates the string containing the list of registered state's names.
 * 
 */
void SGE_PrintStates()
{
	sprintf(listString, "{");
	SGE_LLProcess(stateList, SGE_PrintStateName, NULL);
	strcat(listString, "}");
	//SGE_LogPrintLine(SGE_LOG_DEBUG, "Registered SGE States: %s", listString);
}

const char *SGE_GetStateNames()
{
	if(!stateList)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "%s(): State list is not initialized.", __FUNCTION__);
		sprintf(listString, "{}");
		return listString;
	}
	SGE_PrintStates();
	return listString;
}

int SGE_GetStateCount()
{
	if(!stateList)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "%s(): State list is not initialized.", __FUNCTION__);
		return 0;
	}
	return stateList->count;
}

/* Fallbacks used when a state is created with NULL function pointers */

bool SGE_FallbackInit()
{
	SGE_LogPrintLine(SGE_LOG_DEBUG, "Init is set to NULL, using fallback!");
	return true;
}

void SGE_FallbackQuit()
{
	SGE_LogPrintLine(SGE_LOG_DEBUG, "Quit is set to NULL, using fallback!");
}

void SGE_FallbackHandleEvents(){}
void SGE_FallbackUpdate(){}
void SGE_FallbackRender(){}

/* A fallback state used when the entry state is not found by SGE_Run() */

static TTF_Font *font = NULL;

static bool SGE_FallbackState_Init()
{
	font = TTF_OpenFont("assets/FreeSansBold.ttf", 32);
	SGE_SetTextureWordWrap(SGE_GetScreenWidth());
	SGE_TextLabel *warningLabel = SGE_CreateTextLabelCustom("Straw Hat Game Engine", 0, 0, SGE_COLOR_WHITE, font, NULL);
	SGE_TextLabelSetPosition(warningLabel, SGE_GetScreenCenterX() - warningLabel->boundBox.w / 2, SGE_GetScreenCenterY() - warningLabel->boundBox.h / 2);
	return true;
}

static void SGE_FallbackState_Quit()
{
	TTF_CloseFont(font);
}

/**
 * \brief Fills an SGE_GameState structure
 * 
 * \param state Address of the SGE_GameState structure to be filled.
 * \param name The name of the game state.
 * \param init Address of the initialization function for the state.
 * \param quit Address of the clean up function for the state.
 * \param handleEvents Address of the event handling function for the state.
 * \param update Address of the frame update function for the state.
 * \param render Address of the frame render function for the state.
 */
static void SGE_SetStateFunctions(SGE_GameState *state, const char *name, bool (*init)(), void (*quit)(), void (*handleEvents)(), void (*update)(), void (*render)())
{
	if(state == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to set functions for NULL state!");
		return;
	}
	
	strncpy(state->name, name, 20);
	state->init = (init == NULL) ? SGE_FallbackInit : init;
	state->quit = (quit == NULL) ? SGE_FallbackQuit : quit;
	state->handleEvents = (handleEvents == NULL) ? SGE_FallbackHandleEvents : handleEvents;
	state->update = (update == NULL) ? SGE_FallbackUpdate : update;
	state->render = (render == NULL) ? SGE_FallbackRender : render;
}

/*
 * Sets the current state function pointers in SGE.c
 * This function is defined in SGE.c
 */
void SGE_SetCurrentStateFunctions(const char *name, bool (*init)(), void (*quit)(), void (*handleEvents)(), void (*update)(), void (*render)());

/**
 * \brief Sets the current state function pointers to a registered state's functions to set it as the first game state started by SGE_Run().
 * 
 * \param name The name of a registered state.
 */
void SGE_SetEntryStateFromList(const char *name)
{
	SGE_GameState *state = SGE_GetStateFromList(name);
	if(state == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Entry state \"%s\" not found, creating fallback!", name);
		SGE_AddState("Fallback State", SGE_FallbackState_Init, SGE_FallbackState_Quit, NULL, NULL, NULL);
		SGE_SetCurrentStateFunctions("Fallback State", SGE_FallbackState_Init, SGE_FallbackState_Quit, NULL, NULL, NULL);
		SGE_GUI_UpdateCurrentState("Fallback State");
		return;
	}
	SGE_SetCurrentStateFunctions(state->name, state->init, state->quit, state->handleEvents, state->update, state->render);
}

/**
 * \brief Calls a state's init function and sets it's loaded flag to true on success.
 * 
 * Calls SGE_Quit() if the state failed to load.
 * 
 * \param state The name of the state to initialize.
 */
void SGE_InitState(const char *name)
{
	SGE_GameState *state = SGE_GetStateFromList(name);
	if(state == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to init NULL state!");
		return;
	}

	SGE_LogPrintLine(SGE_LOG_INFO, "Initializing state: \"%s\"...", state->name);
	if(!state->init())
	{
		SGE_Quit();
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed Initializing State!");
	}
	else
	{
		state->loaded = true;
		SGE_LogPrintLine(SGE_LOG_INFO, "Finished Initializing State.");
		SGE_printf(SGE_LOG_DEBUG, "\n");
	}
}

/**
 * \brief Calls a state's quit function, deletes it's GUI controls and sets it's loaded flag to false.
 * 
 * \param state The name of the state to quit.
 */
void SGE_QuitState(const char *name)
{
	SGE_GameState *state = SGE_GetStateFromList(name);
	if(state == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to quit NULL state!");
		return;
	}

	SGE_LogPrintLine(SGE_LOG_INFO, "Quitting state: \"%s\"...", state->name);
	state->quit();
	SGE_GUI_FreeControlList(state->controls);
	state->loaded = false;
	SGE_LogPrintLine(SGE_LOG_INFO, "Finished Quitting State.");
	SGE_printf(SGE_LOG_DEBUG, "\n");
}

void SGE_SwitchToState(const char *nextStateName, bool quitCurrent)
{
	if(!stateList)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "%s(): State list is not initialized.", __FUNCTION__);
		return;
	}

	SGE_GameState *nextState = SGE_GetStateFromList(nextStateName);
	if(nextState == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to switch to NULL state!");
		return;
	}
	nextSwitchState = nextState;
	switchQuitCurrent = quitCurrent;
}

/**
 * \brief Internally switches the current state.
 * 
 * The SGE_SwitchToState() function sets a flag that is read by this function at the
 * end of the frame to trigger a state switch.
 * 
 */
void SGE_SwitchStates()
{
	if(nextSwitchState == NULL)
	{
		return;
	}

	if(switchQuitCurrent)
	{
		SGE_QuitState(SGE_GetCurrentStateName());
	}
	
	SGE_SetCurrentStateFunctions(nextSwitchState->name, nextSwitchState->init, nextSwitchState->quit, nextSwitchState->handleEvents, nextSwitchState->update, nextSwitchState->render);
	SGE_GUI_UpdateCurrentState(nextSwitchState->name);

	if(!SGE_StateIsLoaded(SGE_GetCurrentStateName()))
	{
		SGE_InitState(SGE_GetCurrentStateName());
	}

	nextSwitchState = NULL;
}

bool SGE_StateIsLoaded(const char *name)
{
	if(!stateList)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "%s(): State list is not initialized.", __FUNCTION__);
		return false;
	}

	SGE_GameState *state = SGE_GetStateFromList(name);
	if(state != NULL)
	{
		return state->loaded;
	}

	SGE_LogPrintLine(SGE_LOG_WARNING, "%s(): \"%s\" not in state list!", __FUNCTION__, name);
	return false;
}

void SGE_AddState(const char *name, bool (*init)(), void (*quit)(), void (*handleEvents)(), void (*update)(), void (*render)())
{
	if(!stateList)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "%s(): State list is not initialized.", __FUNCTION__);
		return;
	}

	if(SGE_StateIsRegistered(name))
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Failed to add state \"%s\", name already exists!", name);
		return;
	}

	SGE_GameState *state = (SGE_GameState*)malloc(sizeof(SGE_GameState));
	SGE_SetStateFunctions(state, name, init, quit, handleEvents, update, render);
	state->controls = SGE_CreateGUIControlList();
	state->loaded = false;
	SGE_LLPush(stateList, state);
	SGE_LogPrintLine(SGE_LOG_INFO, "Registered state: %s", name);
}

/**
 * \brief Deallocator callback used by the state list to unregister all states.
 *        This is called for each state in the list when the state list is destroyed.
 *        It calls the quit function for any loaded states and destroys their GUI control list.
 * 
 * \param data The current state when iterating over state list with LLFree()
 */
void SGE_LLFreeState(void *data)
{
	SGE_GameState *state = data;
	if(state->loaded)
	{
		SGE_QuitState(state->name);
	}
	SGE_DestroyGUIControlList(state->controls);
	state->controls = NULL;
	SGE_LogPrintLine(SGE_LOG_DEBUG, "Unregistered state: %s", state->name);
	free(state);
}

/**
 * \brief Creates the state list, a list of all registered states.
 * 
 */
void SGE_CreateStateList()
{
	if(stateList)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Can't create state list, state list already exists.");
		return;
	}
	stateList = SGE_LLCreate(SGE_LLFreeState);
}

/**
 * \brief Destroys the state list, calling SGE_FreeState() for all states and freeing the state list itself.
 *        The state list is NULL after a call to this function, meaning no state operations are valid.
 * 
 */
void SGE_DestroyStateList()
{
	stateList = SGE_LLDestroy(stateList);
}

/**
 * \brief Callback for LLProcess() that quits a loaded state.
 * 
 */
void *SGE_LLQuitLoadedState(SGE_LinkedList *list, SGE_LLNode *currentNode, void *processData)
{
	SGE_GameState *currentListState = currentNode->data;
	if(currentListState->loaded)
	{
		SGE_QuitState(currentListState->name);
	}
	return NULL;
}

/**
 * \brief Quits all game states that are currently loaded.
 * 
 */
void SGE_QuitLoadedStates()
{
	SGE_LLProcess(stateList, SGE_LLQuitLoadedState, NULL);
}