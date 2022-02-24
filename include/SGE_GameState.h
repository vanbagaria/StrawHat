#ifndef __SGE_GAMESTATE_H__
#define __SGE_GAMESTATE_H__

#include <stdbool.h>
#include <stddef.h>

/**
 * \brief Internally used game state structure
 * 
 */
typedef struct
{
	char name[20]; /**< The unique name of the state */
	bool (*init)(); /**< The state init function */
	void (*quit)(); /**< The state quit function */
	void (*handleEvents)(); /**< The state event handling function */
	void (*update)(); /**< The state update function */
	void (*render)(); /**< The state render function */
} SGE_GameState;

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
void SGE_SetStateFunctions(
	SGE_GameState *state, const char *name,
	bool (*init)(),
	void (*quit)(),
	void (*handleEvents)(),
	void (*update)(),
	void (*render)());

/**
 * \brief Register a new state and add it to the state list.
 * 
 * A state is a named collection of five functions that act as a sub-application.
 * Only one state is active as the current state of the engine.
 * The current state can be switched using the SGE_SwitchToState() function.
 * 
 * \param name The name of the game state.
 * \param init The initialization function for the state.
 * \param quit The clean up function for the state.
 * \param handleEvents The event handling function for the state.
 * \param update The frame update function for the state.
 * \param render The frame render function for the state.
 */
void SGE_AddState(const char *name, bool (*init)(), void (*quit)(), void (*handleEvents)(), void (*update)(), void (*render)());

/**
 * \brief Switch the current game state.
 * 
 * Switches the current state to another state while freeing the current state,
 * or by keeping the current state loaded. Keeping a state loaded will allow fast
 * switching to that state as the init function will be skipped for a loaded state.
 * It is safe to never quit a state before switching, since all loaded states are quit
 * when the application exits.
 * 
 * \param nextStateName The name of the state to switch to.
 * \param quitCurrent  Whether to call the current state's quit function before switching.
 */
void SGE_SwitchToState(const char *nextStateName, bool quitCurrent);

/**
 * \brief Calls a state's init function and sets it's loaded flag to true on success.
 * 
 * Calls SGE_Quit() if the state failed to load.
 * 
 * \param state The address of the state to initialize.
 */
void SGE_InitState(SGE_GameState *state);

/**
 * \brief Calls a state's quit function and sets it's loaded flag to false.
 * 
 * \param state The address of the state to quit.
 */
void SGE_QuitState(SGE_GameState *state);

/**
 * \brief Get the internal SGE_GameState structure for a registered state.
 * 
 * \param name The name of a registered game state.
 * \return The address of the SGE_GameState structure of the registered state.
 */
SGE_GameState *SGE_GetState(const char *name);

/**
 * \brief Returns a string containing a list of names of all registered states.
 * 
 * \return String of names of all registered states.
 */
const char *SGE_GetStateNames();

/**
 * \brief Gets the total number of registered states.
 * 
 * \return Number of registered states.
 */
int SGE_GetStateCount();

/**
 * \brief Returns whether a state has been loaded or freed.
 * 
 * \param name The name of a registered state.
 * \return true if the state's init function has been successfully called,
 *         false if the state's quit function has been called.
 */
bool SGE_StateIsLoaded(const char *name);

/**
 * \brief Free's the list of all registered states and call's their quit functions.
 * 
 */
void SGE_FreeStateList();

typedef struct SGE_GUI_ControlList SGE_GUI_ControlList; // SGE_GUI.h

/**
 * \brief Returns the SGE_GUI_ControlList structure for a registered state.
 * 
 * \param name The name of the registered state whose GUI control list should be returned.
 * \return The address of the GUI control list of the registered state.
 */
SGE_GUI_ControlList *SGE_GetStateGUIList(const char *name);

#endif