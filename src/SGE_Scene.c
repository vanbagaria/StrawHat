#include "SGE.h"
#include "SGE_Scene.h"
#include "SGE_Logger.h"
#include "SGE_GUI.h"
#include <stdio.h>
#include <string.h>

/* A list of a scene's GUI controls, defined in SGE_GUI.c */

typedef struct SGE_GUI_ControlList SGE_GUI_ControlList;
SGE_GUI_ControlList *SGE_CreateGUIControlList();
void SGE_DestroyGUIControlList(SGE_GUI_ControlList *controls);
void SGE_GUI_FreeControlList(SGE_GUI_ControlList *controls);

/* Used to update the current control list pointer in SGE_GUI.c */
void SGE_GUI_UpdateCurrentScene(SGE_GUI_ControlList *controls);

/**
 * \brief Internally used game scene structure.
 * 
 */
typedef struct
{
	char name[20]; /**< The unique name of the scene */
	SGE_GUI_ControlList* controls; /**< List of scene GUI controls */
	bool loaded; /**< True if the scene's init function was called */

	bool (*init)(); /**< The scene init function */
	void (*quit)(); /**< The scene quit function */
	void (*handleEvents)(); /**< The scene event handling function */
	void (*update)(); /**< The scene update function */
	void (*render)(); /**< The scene render function */
} SGE_Scene;

/* The internal list of registered scenes */
static SGE_LinkedList *sceneList = NULL;

/* The current scene */
static SGE_Scene currentScene = {"SGE", NULL, NULL, NULL, NULL, NULL};

/* This is not NULL when a SGE_SwitchToScene() call is made */
static SGE_Scene *nextSwitchScene = NULL;

/* Flag for whether to free the current scene when switching to next scene */
static bool switchQuitCurrent = false;

/* A string representing the names of all registered scenes */
static char listString[1000];

const char *SGE_GetCurrentSceneName()
{
	return currentScene.name;
}

/* Fallbacks used when a scene is created with NULL function pointers */

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

/**
 * \brief Fills an SGE_Scene structure
 * 
 * \param scene Address of the SGE_Scene structure to be filled.
 * \param name The name of the game scene.
 * \param init Address of the initialization function for the scene.
 * \param quit Address of the clean up function for the scene.
 * \param handleEvents Address of the event handling function for the scene.
 * \param update Address of the frame update function for the scene.
 * \param render Address of the frame render function for the scene.
 */
static void SGE_SetSceneFunctions(SGE_Scene *scene, const char *name, bool (*init)(), void (*quit)(), void (*handleEvents)(), void (*update)(), void (*render)())
{
	if(scene == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to set functions for NULL scene!");
		return;
	}
	
	strncpy(scene->name, name, 20);
	scene->init = (init == NULL) ? SGE_FallbackInit : init;
	scene->quit = (quit == NULL) ? SGE_FallbackQuit : quit;
	scene->handleEvents = (handleEvents == NULL) ? SGE_FallbackHandleEvents : handleEvents;
	scene->update = (update == NULL) ? SGE_FallbackUpdate : update;
	scene->render = (render == NULL) ? SGE_FallbackRender : render;
}

static void SGE_SetCurrentScene(SGE_Scene *scene)
{
	SGE_SetSceneFunctions(&currentScene, scene->name, scene->init, scene->quit, scene->handleEvents, scene->update, scene->render);
	currentScene.controls = scene->controls;
	SGE_GUI_UpdateCurrentScene(scene->controls);
}

/**
 * \brief Callback function for SGE_LLProcess() that finds a given scene in the scene list.
 * 
 * \param list The scene list.
 * \param currentNode The current scene when iterating over the list.
 * \param processData The name of the scene to find.
 * \return The address of the SGE_Scene for the given name or NULL if not found in list.
 */
static void *SGE_FindScene(SGE_LinkedList *list, SGE_LLNode* currentNode, void *processData)
{
	SGE_Scene *currentListScene = currentNode->data;
	const char *sceneName = processData;

	if(strcmp(currentListScene->name, sceneName) == 0)
	{
		// Stop LLProcess() from iterating furthur by returning non NULL value.
		return currentNode->data;
	}

	// Keep iterating over the list by returning NULL
	return NULL;
}

/**
 * \brief Returns the address of a registered scene from the scene list.
 * 
 * \param name The name of a registered scene.
 * \return The address of the internal SGE_Scene strucuture.
 */
static SGE_Scene *SGE_GetSceneFromList(const char *name)
{
	SGE_Scene *scene = SGE_LLProcess(sceneList, SGE_FindScene, (void*)name);
	if(scene == NULL)
	{
		// SGE_LogPrintLine(SGE_LOG_WARNING, "%s: Scene not in list!", __FUNCTION__);
		return NULL;
	}
	return scene;
}

bool SGE_SceneIsRegistered(const char *name)
{
	if(!sceneList)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "%s(): Scene list is not initialized.", __FUNCTION__);
		return false;
	}

	if(SGE_LLProcess(sceneList, SGE_FindScene, (void*)name))
	{
		return true;
	}
	return false;
}

/**
 * \brief Returns the SGE_GUI_ControlList of the current scene.
 * 
 * \return The address of the GUI control list of the current scene.
 */
SGE_GUI_ControlList *SGE_GetCurrentGUIControlList()
{
	return currentScene.controls;
}

/**
 * \brief Callback for LLProcess() that prints a scene's name to the scene list string.
 * 
 * \param list The scene list.
 * \param currentNode The current scene when iterating with LLProcess()
 * \param processData Unused.
 * \return NULL to keep iterating over entire list.
 */
static void *SGE_PrintSceneName(SGE_LinkedList *list, SGE_LLNode *currentNode, void *processData)
{
	SGE_Scene *currentListScene = currentNode->data;
	strcat(listString, currentListScene->name);
	if(currentNode->next != NULL)
	{
		strcat(listString, ", ");
	}
	return NULL;
}

/**
 * \brief Updates the string containing the list of registered scene's names.
 * 
 */
void SGE_PrintScenes()
{
	sprintf(listString, "{");
	SGE_LLProcess(sceneList, SGE_PrintSceneName, NULL);
	strcat(listString, "}");
	//SGE_LogPrintLine(SGE_LOG_DEBUG, "Registered SGE Scenes: %s", listString);
}

const char *SGE_GetSceneNames()
{
	if(!sceneList)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "%s(): Scene list is not initialized.", __FUNCTION__);
		sprintf(listString, "{}");
		return listString;
	}
	SGE_PrintScenes();
	return listString;
}

int SGE_GetSceneCount()
{
	if(!sceneList)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "%s(): Scene list is not initialized.", __FUNCTION__);
		return 0;
	}
	return sceneList->count;
}

/* A fallback scene used when the entry scene is not found for SGE_Start(). */

static TTF_Font *font = NULL;

static bool SGE_FallbackScene_Init()
{
	font = TTF_OpenFont("assets/FreeSansBold.ttf", 32);
	SGE_SetTextureWordWrap(SGE_GetScreenWidth());
	SGE_TextLabel *warningLabel = SGE_CreateTextLabelCustom("Straw Hat Game Engine", 0, 0, SGE_COLOR_WHITE, font, NULL);
	SGE_TextLabelSetPosition(warningLabel, SGE_GetScreenCenterX() - warningLabel->boundBox.w / 2, SGE_GetScreenCenterY() - warningLabel->boundBox.h / 2);
	return true;
}

static void SGE_FallbackScene_Quit()
{
	TTF_CloseFont(font);
}

/**
 * \brief Initializes the first scene to start with SGE_Start() from the scene list.
 * 
 * \param name The name of a registered scene.
 */
void SGE_SetEntrySceneFromList(const char *name)
{
	SGE_Scene *scene = SGE_GetSceneFromList(name);
	if(scene == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Entry scene \"%s\" not found, creating fallback!", name);
		SGE_AddScene("Fallback Scene", SGE_FallbackScene_Init, SGE_FallbackScene_Quit, NULL, NULL, NULL);
		SGE_SetCurrentScene(SGE_GetSceneFromList("Fallback Scene"));
		return;
	}
	SGE_SetCurrentScene(scene);
}

/**
 * \brief Calls a scene's init function and sets it's loaded flag to true on success.
 * 
 * Calls SGE_Stop() if the scene failed to load.
 * 
 * \param name The name of the scene to initialize.
 */
void SGE_InitScene(const char *name)
{
	SGE_Scene *scene = SGE_GetSceneFromList(name);
	if(scene == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to initialize NULL scene!");
		return;
	}

	SGE_LogPrintLine(SGE_LOG_INFO, "Initializing scene: \"%s\"...", scene->name);
	if(!scene->init())
	{
		SGE_Stop();
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed initializing scene!");
	}
	else
	{
		scene->loaded = true;
		SGE_LogPrintLine(SGE_LOG_INFO, "Finished initializing scene.");
		SGE_printf(SGE_LOG_DEBUG, "\n");
	}
}

/**
 * \brief Calls a scene's quit function, deletes it's GUI controls and sets it's loaded flag to false.
 * 
 * \param name The name of the scene to quit.
 */
void SGE_QuitScene(const char *name)
{
	SGE_Scene *scene = SGE_GetSceneFromList(name);
	if(scene == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to quit NULL scene!");
		return;
	}

	SGE_LogPrintLine(SGE_LOG_INFO, "Quitting scene: \"%s\"...", scene->name);
	scene->quit();
	SGE_GUI_FreeControlList(scene->controls);
	scene->loaded = false;
	SGE_LogPrintLine(SGE_LOG_INFO, "Finished quitting scene.");
	SGE_printf(SGE_LOG_DEBUG, "\n");
}

void SGE_SwitchToScene(const char *nextSceneName, bool quitCurrent)
{
	if(!sceneList)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "%s(): Scene list is not initialized.", __FUNCTION__);
		return;
	}

	SGE_Scene *nextScene = SGE_GetSceneFromList(nextSceneName);
	if(nextScene == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to switch to NULL scene!");
		return;
	}
	nextSwitchScene = nextScene;
	switchQuitCurrent = quitCurrent;
}

/**
 * \brief Internally switches the current scene.
 * 
 * The SGE_SwitchToScene() function sets a flag that is read by this function at the
 * end of the frame to trigger a scene switch.
 * 
 */
void SGE_SwitchScenes()
{
	if(nextSwitchScene == NULL)
	{
		return;
	}

	if(switchQuitCurrent)
	{
		SGE_QuitScene(currentScene.name);
	}
	
	SGE_SetCurrentScene(nextSwitchScene);

	if(!SGE_SceneIsLoaded(currentScene.name))
	{
		SGE_InitScene(currentScene.name);
	}

	nextSwitchScene = NULL;
}

bool SGE_SceneIsLoaded(const char *name)
{
	if(!sceneList)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "%s(): Scene list is not initialized.", __FUNCTION__);
		return false;
	}

	SGE_Scene *scene = SGE_GetSceneFromList(name);
	if(scene != NULL)
	{
		return scene->loaded;
	}

	SGE_LogPrintLine(SGE_LOG_WARNING, "%s(): \"%s\" not in scene list!", __FUNCTION__, name);
	return false;
}

void SGE_AddScene(const char *name, bool (*init)(), void (*quit)(), void (*handleEvents)(), void (*update)(), void (*render)())
{
	if(!sceneList)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "%s(): Cannot add scene, scene list is not initialized.", __FUNCTION__);
		return;
	}

	if(SGE_SceneIsRegistered(name))
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Failed to add scene \"%s\", name already exists!", name);
		return;
	}

	SGE_Scene *scene = (SGE_Scene*)malloc(sizeof(SGE_Scene));
	SGE_SetSceneFunctions(scene, name, init, quit, handleEvents, update, render);
	scene->controls = SGE_CreateGUIControlList();
	scene->loaded = false;
	SGE_LLPush(sceneList, scene);
	SGE_LogPrintLine(SGE_LOG_INFO, "Registered scene: %s", name);
}

/**
 * \brief Deallocator callback used by the scene list to unregister all scenes.
 *        This is called for each scene in the list when the scene list is destroyed.
 *        It calls the quit function for any loaded scenes and destroys their GUI control list.
 * 
 * \param data The current scene when iterating over scene list with LLFree()
 */
void SGE_LLFreeScene(void *data)
{
	SGE_Scene *scene = data;
	if(scene->loaded)
	{
		SGE_QuitScene(scene->name);
	}
	SGE_DestroyGUIControlList(scene->controls);
	scene->controls = NULL;
	SGE_LogPrintLine(SGE_LOG_DEBUG, "Unregistered scene: %s", scene->name);
	free(scene);
}

/**
 * \brief Creates the scene list, a list of all registered scenes.
 * 
 */
static void SGE_CreateSceneList()
{
	if(sceneList)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Can't create scene list, scene list already exists.");
		return;
	}
	sceneList = SGE_LLCreate(SGE_LLFreeScene);
}

/**
 * \brief Destroys the scene list, calling SGE_FreeScene() for all scenes and freeing the scene list itself.
 *        The scene list is NULL after a call to this function, meaning no scene operations are valid.
 * 
 */
static void SGE_DestroySceneList()
{
	sceneList = SGE_LLDestroy(sceneList);
}

/**
 * \brief Callback for LLProcess() that quits a loaded scene.
 * 
 */
void *SGE_LLQuitLoadedScene(SGE_LinkedList *list, SGE_LLNode *currentNode, void *processData)
{
	SGE_Scene *currentListScene = currentNode->data;
	if(currentListScene->loaded)
	{
		SGE_QuitScene(currentListScene->name);
	}
	return NULL;
}

/**
 * \brief Quits all game scenes that are currently loaded.
 * 
 */
void SGE_QuitLoadedScenes()
{
	SGE_LLProcess(sceneList, SGE_LLQuitLoadedScene, NULL);
}

/*
 * Scene module functions used by SGE.c
 */

void SGE_Scene_Init()
{
	SGE_SetSceneFunctions(&currentScene, "SGE", NULL, NULL, NULL, NULL, NULL);
	SGE_CreateSceneList();
}

void SGE_Scene_Quit()
{
	SGE_SetSceneFunctions(&currentScene, "SGE", NULL, NULL, NULL, NULL, NULL);
	SGE_DestroySceneList();
}

void SGE_Scene_HandleEvents()
{
	SGE_GUI_ControlList_HandleEvents(currentScene.controls);
	currentScene.handleEvents();
}

void SGE_Scene_Update()
{
	SGE_GUI_ControlList_Update(currentScene.controls);
	currentScene.update();
}

void SGE_Scene_Render()
{
	currentScene.render();
	SGE_GUI_ControlList_Render(currentScene.controls);
}