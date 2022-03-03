#ifndef __SGE_GAMESTATE_H__
#define __SGE_GAMESTATE_H__

#include <stdbool.h>

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
 * \brief Checks if a given state has been registered in the state list.
 * 
 * \param name Name of the state to check for.
 * \return true if name exists in state list, false otherwise.
 */
bool SGE_StateIsRegistered(const char *name);

/**
 * \brief Returns whether a state has been loaded or freed.
 * 
 * \param name The name of a registered state.
 * \return true if the state's init function has been successfully called,
 *         false if the state's quit function has been called.
 */
bool SGE_StateIsLoaded(const char *name);

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

#endif