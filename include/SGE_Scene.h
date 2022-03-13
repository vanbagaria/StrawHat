#ifndef __SGE_SCENE_H__
#define __SGE_SCENE_H__

#include <stdbool.h>

/**
 * \brief Register a new scene and add it to the scene list.
 * 
 * A scene is a named collection of five functions that act as a sub-application.
 * Only one scene is active as the current scene of the engine.
 * The current scene can be switched using the SGE_SwitchToScene() function.
 * 
 * \param name The name of the game scene.
 * \param init The initialization function for the scene.
 * \param quit The clean up function for the scene.
 * \param handleEvents The event handling function for the scene.
 * \param update The frame update function for the scene.
 * \param render The frame render function for the scene.
 */
void SGE_AddScene(const char *name, bool (*init)(), void (*quit)(), void (*handleEvents)(), void (*update)(), void (*render)());

/**
 * \brief Switch the current game scene.
 * 
 * Switches the current scene to another scene while freeing the current scene,
 * or by keeping the current scene loaded. Keeping a scene loaded will allow fast
 * switching to that scene as the init function will be skipped for a loaded scene.
 * It is safe to never quit a scene before switching, since all loaded scenes are quit
 * when the application exits.
 * 
 * \param nextSceneName The name of the scene to switch to.
 * \param quitCurrent  Whether to call the current scene's quit function before switching.
 */
void SGE_SwitchToScene(const char *nextSceneName, bool quitCurrent);

/**
 * \brief Checks if a given scene has been registered in the scene list.
 * 
 * \param name Name of the scene to check for.
 * \return true if name exists in scene list, false otherwise.
 */
bool SGE_SceneIsRegistered(const char *name);

/**
 * \brief Returns whether a scene has been loaded or freed.
 * 
 * \param name The name of a registered scene.
 * \return true if the scene's init function has been successfully called,
 *         false if the scene's quit function has been called.
 */
bool SGE_SceneIsLoaded(const char *name);

/**
 * \brief Returns a string containing a list of names of all registered scenes.
 * 
 * \return String of names of all registered scenes.
 */
const char *SGE_GetSceneNames();

/**
 * \brief Gets the total number of registered scenes.
 * 
 * \return Number of registered scenes.
 */
int SGE_GetSceneCount();

#endif