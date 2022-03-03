#include "SGE.h"
#include "SGE_GameState.h"
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

static SDL_Window   *window   = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Event event;

static const Uint8 *keyboardState = NULL;

static SDL_Point mousePosition;

static int screenWidth  = 0;
static int screenHeight = 0;

/* Default engine font */
static TTF_Font *defaultFont;

/* Default background fill color */
static SDL_Color defaultScreenClearColor;

static unsigned int randSeed = 0;

static bool isVsyncOn    = false;
static bool isRunning    = true;
static bool isFullscreen = false;

/* Frame timing data */
static unsigned int FPSLimit       = 0;
static unsigned int perFrameTime   = 0;
static unsigned int frameStartTime = 0;
static unsigned int lastRenderTime = 0;
static unsigned int lastFrameTime  = 0;
static bool frameRateCap = false;
static double deltaTime = 0;

/* Current state function pointers */
static char currentStateName[20] = "SGE";
static void (*currentStateInit)()         = NULL;
static void (*currentStateQuit)()         = NULL;
static void (*currentStateHandleEvents)() = NULL;
static void (*currentStateUpdate)()       = NULL;
static void (*currentStateRender)()       = NULL;

/* Fallbacks defined in SGE_GameState.c */
bool SGE_FallbackInit();
void SGE_FallbackQuit();
void SGE_FallbackHandleEvents();
void SGE_FallbackUpdate();
void SGE_FallbackRender();

/* Defined in SGE_GameState.c */
void SGE_CreateStateList();
void SGE_DestroyStateList();
void SGE_InitState(const char *name);
void SGE_QuitState(const char *name);
void SGE_SwitchStates();

/* Returns the name of the current state */
const char *SGE_GetCurrentStateName()
{
	return currentStateName;
}

/**
 * \brief Sets the current state function pointers.
 * 
 * \param name The name of the current state.
 * \param init The init function of the state.
 * \param quit The quit function of the state.
 * \param handleEvents The handleEvents function of the state.
 * \param update The update function of the state.
 * \param render The render function of the state.
 */
void SGE_SetCurrentStateFunctions(const char *name, bool (*init)(), void (*quit)(), void (*handleEvents)(), void (*update)(), void (*render)())
{
	strncpy(currentStateName, name, 20);
	currentStateInit = (init == NULL) ? SGE_FallbackInit : init;
	currentStateQuit = (quit == NULL) ? SGE_FallbackQuit : quit;
	currentStateHandleEvents = (handleEvents == NULL) ? SGE_FallbackHandleEvents : handleEvents;
	currentStateUpdate = (update == NULL) ? SGE_FallbackUpdate : update;
	currentStateRender = (render == NULL) ? SGE_FallbackRender : render;
}

/**
 * \brief Sets the current state function pointers to a registered state's functions.
 * 
 * This function is defined in SGE_GameState.c, it is called by SGE_Run() to determine 
 * the first state that SGE should start with.
 * 
 * \param name The name of a registered state.
 */
void SGE_SetCurrentStateFunctionsFromList(const char *name);

/* Defined in SGE_GUI.c */
bool SGE_GUI_Init();
void SGE_GUI_Quit();
void SGE_GUI_HandleEvents();
void SGE_GUI_Update();
void SGE_GUI_Render();

/*  Updates the current control list pointer in SGE_GUI.c */
void SGE_GUI_UpdateCurrentState(const char *nextState);

/**
 * \brief Updates the SDL_Renderer to be used by SGE_Graphics.c
 * 
 * This function is defined in SGE_Graphics.c, it is called whenever the SDL renderer
 * is created, so the graphics functions can have access to the renderer.
 */
void SGE_Graphics_UpdateSDLRenderer();

SDL_Renderer *SGE_GetSDLRenderer()
{
	return renderer;
}

SDL_Event *SGE_GetSDLEvent()
{
	return &event;
}

double SGE_GetDeltaTime()
{
	return deltaTime;
}

bool SGE_VsyncIsOn()
{
	return isVsyncOn;
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
	return screenWidth / 2;
}

void SGE_SetBackgroundColor(SDL_Color color)
{
	defaultScreenClearColor = color;
}

 bool SGE_Init(const char *title, int width, int height)
{
	screenWidth = width;
	screenHeight = height;
	
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_WEBP);
	TTF_Init();

	window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
	if(window == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed to create Game Window! SDL_Error: %s", SDL_GetError());
		isRunning = SDL_FALSE;
		return false;
	}
	
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if(renderer == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed to create Game Renderer! SDL_Error: %s", SDL_GetError());
		isRunning = SDL_FALSE;
		return false;
	}

	SGE_Graphics_UpdateSDLRenderer();
	
	defaultFont = TTF_OpenFont("assets/FreeSans.ttf", 24);
	if(defaultFont == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed to load default font! TTF_Error: %s", TTF_GetError());
		isRunning = false;
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
	
	SGE_LogPrintLine(SGE_LOG_INFO, "Straw Hat Game Engine Version 1.0");
	SGE_LogPrintLine(SGE_LOG_DEBUG, "Platform: %s", SDL_GetPlatform());
	//SGE_LogPrintLine(SGE_LOG_DEBUG, "Video Driver: %s", SDL_GetCurrentVideoDriver());
	//SGE_LogPrintLine(SGE_LOG_DEBUG, "Audio Driver: %s", SDL_GetCurrentAudioDriver());

	SDL_RendererInfo rendererInfo;
	SDL_GetRendererInfo(renderer, &rendererInfo);
	SGE_LogPrintLine(SGE_LOG_DEBUG, "Renderer: %s", rendererInfo.name);

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

	SGE_CreateStateList();
	
	if(!SGE_GUI_Init())
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed to initialize SGE GUI!");
		isRunning = SDL_FALSE;

		SDL_DestroyRenderer(renderer);
		renderer = NULL;
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
	
	return true;
}

void SGE_Run(const char *startStateName)
{
	SGE_SetCurrentStateFunctionsFromList(startStateName);
	SGE_GUI_UpdateCurrentState(currentStateName);
	SGE_InitState(currentStateName);

	while(isRunning)
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
				isRunning = false;
			}
			SGE_GUI_HandleEvents();
			currentStateHandleEvents();
		}
		
		/* Logic Updates */
		SGE_GUI_Update();
		currentStateUpdate();
		
		/* Rendering */
		SGE_ClearScreen(defaultScreenClearColor);
		currentStateRender();
		SGE_GUI_Render();
		SDL_RenderPresent(renderer);

		SGE_SwitchStates();
		
		if(frameRateCap)
		{			
			/* Cap the framerate by delaying the next frame */
			lastRenderTime = SDL_GetTicks() - frameStartTime;
			if(lastRenderTime < perFrameTime)
			{
				SDL_Delay(perFrameTime - lastRenderTime);
			}
		}
	}
	
	SGE_QuitState(currentStateName);
	SGE_SetCurrentStateFunctions("SGE", NULL, NULL, NULL, NULL, NULL);
	
	SGE_DestroyStateList();
	SGE_GUI_Quit();
	
	Mix_CloseAudio();
	Mix_Quit();
	TTF_CloseFont(defaultFont);
	
	SDL_DestroyRenderer(renderer);
	renderer = NULL;
	SDL_DestroyWindow(window);
	window = NULL;
	
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	SGE_LogPrintLine(SGE_LOG_INFO, "Quit SGE.");
}

void SGE_Quit()
{
	SGE_LogPrintLine(SGE_LOG_INFO, "SGE Quit Requested...");
	isRunning = false;
}

/*
 * Toggles the window to fullscreen or windowed modes.
*/
void SGE_ToggleFullscreen()
{
	if(!isFullscreen)
	{
		if(SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN) == 0)
		{
			isFullscreen = SDL_TRUE;
			SGE_LogPrintLine(SGE_LOG_INFO, "engine->isFullscreen: Toggled to %s", isFullscreen ? "true" : "false");
		}
		else
		{
			SGE_LogPrintLine(SGE_LOG_WARNING, "Failed to enter Fullscreen! SDL_Error: %s", SDL_GetError());
		}
	}
	else
	{
		if(SDL_SetWindowFullscreen(window, 0) == 0)
		{
			isFullscreen = SDL_FALSE;
			SGE_LogPrintLine(SGE_LOG_INFO, "engine->isFullscreen: Toggled to %s", isFullscreen ? "true" : "false");
		}
		else
		{
			SGE_LogPrintLine(SGE_LOG_WARNING, "Failed to leave Fullscreen! SDL_Error: %s", SDL_GetError());
		}
	}
}

/*
 * Toggles the vsync by destroying and recreating the current renderer and all it's created textures.
 * Will reset the current state and destroy all gui controls to initialize them again.
*/
void SGE_ToggleVsync()
{
	/* Save the current renderer's draw blend mode and draw color */
	SDL_BlendMode blendMode;
	SDL_GetRenderDrawBlendMode(renderer, &blendMode);
	SDL_Color drawColor;
	SDL_GetRenderDrawColor(renderer, &drawColor.r, &drawColor.g, &drawColor.b, &drawColor.a);
	
	/* Quit the current state and the GUI to free all textures */
	SGE_QuitState(currentStateName);
	SGE_GUI_Quit();
	
	SDL_DestroyRenderer(renderer);
	renderer = NULL;
	
	/* Toggle the vsync variable */
	isVsyncOn = !isVsyncOn;
	
	/* Recreate the renderer */
	if(isVsyncOn)
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	else
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	
	/* Set the new renderer's blend mode and color to the old one's */
	SDL_SetRenderDrawBlendMode(renderer, blendMode);
	SDL_SetRenderDrawColor(renderer, drawColor.r, drawColor.g, drawColor.b, drawColor.a);

	SGE_Graphics_UpdateSDLRenderer();
	
	/* Reinitialize the GUI and the current state */
	SGE_GUI_Init();
	SGE_InitState(currentStateName);
	
	if(isVsyncOn)
	{
		frameRateCap = false;
		SGE_LogPrintLine(SGE_LOG_INFO, "Turned OFF frame rate cap!");
	}
	SGE_LogPrintLine(SGE_LOG_INFO, "Toggled VSYNC %s!\n", isVsyncOn ? "ON" : "OFF");
}

/*
 * Sets the frame rate cap to "fps", or disables capping if "fps" is 0.
*/
void SGE_SetTargetFPS(int fps)
{
	if(isVsyncOn)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Can't set FPS, Vsync is ON!");
		return;
	}
	
	if(fps == 0)
	{
		frameRateCap = false;
		SGE_LogPrintLine(SGE_LOG_INFO, "Turned OFF frame rate cap!");
	}
	else
	{
		frameRateCap = true;
		FPSLimit = fps;
		perFrameTime = 1000 / fps;
		SGE_LogPrintLine(SGE_LOG_INFO, "Target FPS set to %d", FPSLimit);
	}
}

/*
 * Checks for collision between two rectangles.
*/
bool SGE_CheckRectsCollision(const SDL_Rect *r1, const SDL_Rect *r2)
{
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
	if(SDL_PointInRect(&mousePosition, rect))
	{
		return true;
	}
	return false;
}
