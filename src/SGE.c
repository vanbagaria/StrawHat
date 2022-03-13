#include "SGE.h"
#include "SGE_Scene.h"
#include "SGE_Logger.h"
#include "SGE_GUI.h"

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>

/* SDL Library versions */
static SDL_version SDL_Version_C;
static SDL_version SDL_IMG_Version_C;
static SDL_version SDL_TTF_Version_C;
static SDL_version SDL_MIX_Version_C;
static SDL_version SDL_Version_DLL;
static const SDL_version *SDL_IMG_Version_DLL;
static const SDL_version *SDL_TTF_Version_DLL;
static const SDL_version *SDL_MIX_Version_DLL;

static SDL_Window *window   = NULL;
static SDL_Event event;

static const Uint8 *keyboardState = NULL;

static SDL_Point mousePosition;

static int screenWidth  = 0;
static int screenHeight = 0;

/* Default engine font */
static TTF_Font *defaultFont;

/* Default background fill color, anything other than black has performance impact. */
static SDL_Color defaultScreenClearColor;

static unsigned int randSeed = 0;

static bool isSGEInit       = false;
static bool shouldQuit      = false;
static bool isRunning       = false;
static bool isVsyncOn       = false;
static bool isFullscreen    = false;

/* Frame timing data */
static unsigned int FPSLimit       = 0;
static unsigned int perFrameTime   = 0;
static unsigned int frameStartTime = 0;
static unsigned int lastRenderTime = 0;
static unsigned int lastFrameTime  = 0;
static bool isFrameRateCapped = false;
static double deltaTime = 0;

/* Current scene function pointers */
static char currentSceneName[20] = "SGE";
static bool (*currentSceneInit)()         = NULL;
static void (*currentSceneQuit)()         = NULL;
static void (*currentSceneHandleEvents)() = NULL;
static void (*currentSceneUpdate)()       = NULL;
static void (*currentSceneRender)()       = NULL;

/* Fallbacks defined in SGE_GameScene.c */
bool SGE_FallbackInit();
void SGE_FallbackQuit();
void SGE_FallbackHandleEvents();
void SGE_FallbackUpdate();
void SGE_FallbackRender();

/* Defined in SGE_GameScene.c */
void SGE_CreateSceneList();
void SGE_DestroySceneList();
void SGE_InitScene(const char *name);
void SGE_QuitScene(const char *name);
void SGE_SwitchScenes();
void SGE_QuitLoadedScenes();

/* Returns the name of the current scene */
const char *SGE_GetCurrentSceneName()
{
	return currentSceneName;
}

/**
 * \brief Sets the current scene function pointers.
 * 
 * \param name The name of the current scene.
 * \param init The init function of the scene.
 * \param quit The quit function of the scene.
 * \param handleEvents The handleEvents function of the scene.
 * \param update The update function of the scene.
 * \param render The render function of the scene.
 */
void SGE_SetCurrentSceneFunctions(const char *name, bool (*init)(), void (*quit)(), void (*handleEvents)(), void (*update)(), void (*render)())
{
	strncpy(currentSceneName, name, 20);
	currentSceneInit = (init == NULL) ? SGE_FallbackInit : init;
	currentSceneQuit = (quit == NULL) ? SGE_FallbackQuit : quit;
	currentSceneHandleEvents = (handleEvents == NULL) ? SGE_FallbackHandleEvents : handleEvents;
	currentSceneUpdate = (update == NULL) ? SGE_FallbackUpdate : update;
	currentSceneRender = (render == NULL) ? SGE_FallbackRender : render;
}

/*
 * Sets the current scene function pointers to a registered scene's functions to set it as the first scene started by SGE_Run().
 * This function is defined in SGE_GameScene.c, it is called by SGE_Run() to determine 
 * the first scene that SGE should start with.
 */
void SGE_SetEntrySceneFromList(const char *name);

/* Defined in SGE_Graphics.c */
bool SGE_Graphics_Init();
void SGE_Graphics_Quit();
bool SGE_Graphics_SetVSync();

/* Defined in SGE_GUI.c */
bool SGE_GUI_Init();
void SGE_GUI_Quit();
void SGE_GUI_HandleEvents();
void SGE_GUI_Update();
void SGE_GUI_Render();

/*  Updates the current control list pointer in SGE_GUI.c */
void SGE_GUI_UpdateCurrentScene(const char *nextScene);

SDL_Event *SGE_GetSDLEvent()
{
	return &event;
}

double SGE_GetDeltaTime()
{
	return deltaTime;
}

int SGE_GetFPSLimit()
{
    return FPSLimit;
}

bool SGE_IsVsync()
{
	return isVsyncOn;
}

bool SGE_IsFullscreen()
{
	return isFullscreen;
}

bool SGE_KeyIsPressed(SDL_Scancode scancode)
{
	if(keyboardState[scancode])
	{
		return true;
	}
	return false;
}

SDL_Point SGE_GetMousePosition()
{
	return mousePosition;
}

int SGE_GetMouseX()
{
	return mousePosition.x;
}

int SGE_GetMouseY()
{
	return mousePosition.y;
}

int SGE_GetScreenWidth()
{
	return screenWidth;
}

int SGE_GetScreenHeight()
{
	return screenHeight;
}

int SGE_GetScreenCenterX()
{
	return screenWidth / 2;
}

int SGE_GetScreenCenterY()
{
	return screenHeight / 2;
}

void SGE_SetBackgroundColor(Uint8 r, Uint8 g, Uint8 b)
{
	defaultScreenClearColor.r = r;
	defaultScreenClearColor.g = g;
	defaultScreenClearColor.b = b;
}

void SGE_SetBackgroundColorSDL(SDL_Color color)
{
	defaultScreenClearColor = color;
}

SGE_InitConfig SGE_CreateInitConfig()
{
    SGE_InitConfig config;
    config.x = SDL_WINDOWPOS_CENTERED;
    config.y = SDL_WINDOWPOS_CENTERED;
    config.fullscreen = false;
    config.resizable = false;
	config.borderless = false;
    config.vsync = true;
	return config;
}

 bool SGE_Init(const char *title, int width, int height, SGE_InitConfig *config)
{
	if(isSGEInit)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Ignoring attempt to initialize SGE when already initialized.");
		return false;
	}

	SGE_InitConfig conf;
	if(!config)
	{
		conf = SGE_CreateInitConfig();
	}
	else
	{
		conf = *(config);
	}

	SDL_WindowFlags windowFlags = SDL_WINDOW_SHOWN;
	if(conf.fullscreen)
	{
		windowFlags |= SDL_WINDOW_FULLSCREEN;
	}
	if(conf.resizable)
	{
		windowFlags |= SDL_WINDOW_RESIZABLE;
	}
	if(conf.borderless)
	{
		windowFlags |= SDL_WINDOW_BORDERLESS;
	}
	if(conf.vsync)
	{
		isVsyncOn = true;
	}

	shouldQuit = false;
	screenWidth = width;
	screenHeight = height;
	
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_WEBP);
	TTF_Init();

	SGE_LogPrintLine(SGE_LOG_INFO, "Straw Hat Game Engine");
	SGE_LogPrintLine(SGE_LOG_DEBUG, "Platform: %s", SDL_GetPlatform());

	window = SDL_CreateWindow(title, conf.x, conf.y, screenWidth, screenHeight, windowFlags);
	if(window == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed to create Game Window! SDL_Error: %s", SDL_GetError());
		shouldQuit = true;
		return false;
	}

	if(!SGE_Graphics_Init(window, conf.vsync))
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed to create Game Renderer! SDL_Error: %s", SDL_GetError());
		shouldQuit = true;
		return false;
	}
	
	defaultFont = TTF_OpenFont("assets/FreeSans.ttf", 24);
	if(defaultFont == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed to load default font! TTF_Error: %s", TTF_GetError());
		shouldQuit = true;
		return false;
	}

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
	
	defaultScreenClearColor.r = 0;
	defaultScreenClearColor.g = 200;
	defaultScreenClearColor.b = 255;
	defaultScreenClearColor.a = 255;

	keyboardState = SDL_GetKeyboardState(NULL);

	SDL_VERSION(&SDL_Version_C);
	SDL_GetVersion(&SDL_Version_DLL);
	
	SDL_IMAGE_VERSION(&SDL_IMG_Version_C);
	SDL_IMG_Version_DLL = IMG_Linked_Version();
	
	SDL_TTF_VERSION(&SDL_TTF_Version_C);
	SDL_TTF_Version_DLL = TTF_Linked_Version();
	
	SDL_MIXER_VERSION(&SDL_MIX_Version_C);
	SDL_MIX_Version_DLL = Mix_Linked_Version();
	
	SGE_LogPrintLine(SGE_LOG_DEBUG, "Video Driver: %s", SDL_GetCurrentVideoDriver());
	SGE_LogPrintLine(SGE_LOG_DEBUG, "Audio Driver: %s", SDL_GetCurrentAudioDriver());

	randSeed = time(NULL);
	srand(randSeed);
	SGE_LogPrintLine(SGE_LOG_DEBUG, "C Random Seed: %d Seconds from epoch.", randSeed);

	SGE_LogPrintLine(SGE_LOG_DEBUG, "SDL Version: %d.%d.%d (Compiled)", SDL_Version_C.major, SDL_Version_C.minor, SDL_Version_C.patch);
	SGE_LogPrintLine(SGE_LOG_DEBUG, "SDL Version: %d.%d.%d (Linked)", SDL_Version_DLL.major, SDL_Version_DLL.minor, SDL_Version_DLL.patch);
	
	SGE_LogPrintLine(SGE_LOG_DEBUG, "SDL Image Version: %d.%d.%d (Compiled)", SDL_IMG_Version_C.major, SDL_IMG_Version_C.minor, SDL_IMG_Version_C.patch);
	SGE_LogPrintLine(SGE_LOG_DEBUG, "SDL Image Version: %d.%d.%d (Linked)", SDL_IMG_Version_DLL->major, SDL_IMG_Version_DLL->minor, SDL_IMG_Version_DLL->patch);
	
	SGE_LogPrintLine(SGE_LOG_DEBUG, "SDL TTF Version: %d.%d.%d (Compiled)", SDL_TTF_Version_C.major, SDL_TTF_Version_C.minor, SDL_TTF_Version_C.patch);
	SGE_LogPrintLine(SGE_LOG_DEBUG, "SDL TTF Version: %d.%d.%d (Linked)", SDL_TTF_Version_DLL->major, SDL_TTF_Version_DLL->minor, SDL_TTF_Version_DLL->patch);
	
	SGE_LogPrintLine(SGE_LOG_DEBUG, "SDL Mixer Version: %d.%d.%d (Compiled)", SDL_MIX_Version_C.major, SDL_MIX_Version_C.minor, SDL_MIX_Version_C.patch);
	SGE_LogPrintLine(SGE_LOG_DEBUG, "SDL Mixer Version: %d.%d.%d (Linked)", SDL_MIX_Version_DLL->major, SDL_MIX_Version_DLL->minor, SDL_MIX_Version_DLL->patch);

	SGE_printf(SGE_LOG_DEBUG, "\n");

	SGE_CreateSceneList();
	
	if(!SGE_GUI_Init())
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed to initialize SGE GUI!");
		shouldQuit = true;

		SGE_Graphics_Quit();
		SDL_DestroyWindow(window);
		window = NULL;

		Mix_CloseAudio();
		Mix_Quit();
		TTF_CloseFont(defaultFont);
		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
		return false;
	}
	
	isSGEInit = true;
	return true;
}

void SGE_Start(const char *entrySceneName)
{
	if(!isSGEInit)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "%s(): Cannot start scene \"%s\", SGE is not initialized.", __FUNCTION__, entrySceneName);
		return;
	}

	if(isRunning)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "%s(): Cannot start scene \"%s\", SGE is already running.", __FUNCTION__, entrySceneName);
		return;
	}

	isRunning = true;

	SGE_SetEntrySceneFromList(entrySceneName);
	SGE_GUI_UpdateCurrentScene(currentSceneName);
	SGE_InitScene(currentSceneName);

	while(!shouldQuit)
	{
		/* Calculate Delta time */
		frameStartTime = SDL_GetTicks();
		deltaTime = (frameStartTime - lastFrameTime) / 1000.0;
		lastFrameTime = frameStartTime;
		
		/* Event Handling */
		SDL_GetMouseState(&mousePosition.x, &mousePosition.y);
		while(SDL_PollEvent(&event) != 0)
		{
			if(event.type == SDL_QUIT)
			{
				shouldQuit = true;
			}
			SGE_GUI_HandleEvents();
			currentSceneHandleEvents();
		}
		
		/* Logic Updates */
		SGE_GUI_Update();
		currentSceneUpdate();
		
		/* Rendering */
		SGE_ClearScreenSDL(defaultScreenClearColor);
		currentSceneRender();
		SGE_GUI_Render();
		SDL_RenderPresent(SGE_GetSDLRenderer());

		SGE_SwitchScenes();
		
		if(isFrameRateCapped)
		{			
			/* Cap the framerate by delaying the next frame */
			lastRenderTime = SDL_GetTicks() - frameStartTime;
			if(lastRenderTime < perFrameTime)
			{
				SDL_Delay(perFrameTime - lastRenderTime);
			}
		}
	}
	
	SGE_QuitScene(currentSceneName);
	SGE_SetCurrentSceneFunctions("SGE", NULL, NULL, NULL, NULL, NULL);
	
	isRunning = false;
}

void SGE_Stop()
{
	shouldQuit = true;
}

void SGE_Quit()
{
	if(!isSGEInit)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Cannot quit, SGE is not initialized.");
		return;
	}

	SGE_DestroySceneList();
	SGE_GUI_Quit();
	
	Mix_CloseAudio();
	Mix_Quit();
	TTF_CloseFont(defaultFont);
	
	SGE_Graphics_Quit();
	SDL_DestroyWindow(window);
	window = NULL;
	
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();

	isSGEInit = false;
	SGE_LogPrintLine(SGE_LOG_INFO, "Quit SGE.");
}

/*
 * Toggles the window to fullscreen or windowed modes.
*/
void SGE_ToggleFullscreen()
{
	if(!isSGEInit)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Cannot toggle Full Screen, SGE is not initialized!");
		return;
	}

	if(!isFullscreen)
	{
		if(SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN) == 0)
		{
			isFullscreen = true;
			SGE_LogPrintLine(SGE_LOG_INFO, "Toggled Full Screen ON.");
		}
		else
		{
			SGE_LogPrintLine(SGE_LOG_WARNING, "Failed to enter Full Screen! SDL_Error: %s", SDL_GetError());
		}
	}
	else
	{
		if(SDL_SetWindowFullscreen(window, 0) == 0)
		{
			isFullscreen = false;
			SGE_LogPrintLine(SGE_LOG_INFO, "Toggled Full Screen OFF.");
		}
		else
		{
			SGE_LogPrintLine(SGE_LOG_WARNING, "Failed to leave Full Screen! SDL_Error: %s", SDL_GetError());
		}
	}
}

void SGE_SetVSync(bool enabled)
{
	if(!isSGEInit)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Cannot set V-SYNC, SGE is not initialized!");
		return;
	}

	if(SGE_Graphics_SetVSync(enabled))
	{
		isVsyncOn = enabled;
		SGE_LogPrintLine(SGE_LOG_INFO, "Set V-SYNC %s.", enabled ? "ON" : "OFF");
	}
	else
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Failed to set V-SYNC!");
	}
}

/*
 * Sets the frame rate limit to "fps", or disables limit if "fps" is 0.
*/
void SGE_SetFPSLimit(int fps)
{
	if(fps == 0)
	{
		isFrameRateCapped = false;
		FPSLimit = 0;
		SGE_LogPrintLine(SGE_LOG_INFO, "Turned off frame rate limiter.");
	}
	else
	{
		isFrameRateCapped = true;
		FPSLimit = fps;
		perFrameTime = 1000 / fps;
		SGE_LogPrintLine(SGE_LOG_INFO, "Target FPS set to %d.", FPSLimit);
	}
}

bool SGE_RectInRect(const SDL_Rect *r1, const SDL_Rect *r2)
{
	if(r1 == NULL || r2 == NULL)
	{
		return false;
	}

	/* Can be replaced with SDL_HasIntersection() */
	if(((r1->x >= r2->x) && (r1->x <= (r2->x + r2->w))) || (((r1->x + r1->w) >= r2->x) && ((r1->x + r1->w) <= (r2->x + r2->w))))
	{
		if(((r1->y >= r2->y) && (r1->y <= (r2->y + r2->h))) || (((r1->y + r1->h) >= r2->y) && ((r1->y + r1->h) <= (r2->y + r2->h))))
		{
			return true;
		}
	}
	return false;
}

bool SGE_MouseInRect(const SDL_Rect *rect)
{
	if(rect == NULL)
	{
		return false;
	}

	if(SDL_PointInRect(&mousePosition, rect))
	{
		return true;
	}
	return false;
}
