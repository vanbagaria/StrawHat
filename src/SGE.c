#include "SGE.h"
#include "SGE_GameState.h"
#include "SGE_Logger.h"
#include "SGE_Texture.h"
#include "SGE_GUI.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <stdlib.h>
#include <time.h>

const SDL_Color SGE_COLOR_WHITE        = {255, 255, 255, 255};
const SDL_Color SGE_COLOR_BLACK        = {  0,   0,   0, 255};
const SDL_Color SGE_COLOR_GRAY         = { 50,  50,  50, 255};
const SDL_Color SGE_COLOR_RED          = {255,   0,   0, 255};
const SDL_Color SGE_COLOR_GREEN        = {  0, 255,   0, 255};
const SDL_Color SGE_COLOR_BLUE         = {  0,   0, 255, 255};
const SDL_Color SGE_COLOR_YELLOW       = {255, 255,   0, 255};
const SDL_Color SGE_COLOR_PINK         = {255,   0, 255, 255};
const SDL_Color SGE_COLOR_AQUA         = {  0, 255, 255, 255};
const SDL_Color SGE_COLOR_LIGHT_GRAY   = {195, 195, 195, 255};
const SDL_Color SGE_COLOR_LIGHT_PURPLE = {200, 191, 231, 255};
const SDL_Color SGE_COLOR_DARK_RED     = {136,   0,  21, 255};
const SDL_Color SGE_COLOR_CERISE       = {222,  49,  99, 255};
const SDL_Color SGE_COLOR_ORANGE       = {255, 127,   0, 255};
const SDL_Color SGE_COLOR_INDIGO       = { 63,  72, 204, 255};
const SDL_Color SGE_COLOR_PURPLE       = {163,  73, 164, 255};

/* Global engine data */
static SGE_EngineData engine;

/* Current state */
static SGE_GameState currentState;

SGE_EngineData *SGE_GetEngineData()
{
	return &engine;
}

SGE_GameState *SGE_GetCurrentState()
{
	return &currentState;
}

SGE_EngineData *SGE_Init(const char *title, int screenWidth, int screenHeight)
{
	engine.window = NULL;
	engine.renderer = NULL;
	engine.screenWidth = screenWidth;
	engine.screenHeight = screenHeight;
	engine.screenCenter.x = screenWidth / 2;
	engine.screenCenter.y = screenHeight / 2;
	engine.isRunning = true;
	engine.isFullscreen = false;
	engine.isVsyncOn = false;
	engine.keyboardState = NULL;
	engine.mouse_x = 0;
	engine.mouse_y = 0;
	engine.defaultFont = NULL;
	
	engine.frameStartTime = 0;
	engine.fps = 60;
	engine.lastRenderTime = 0;
	engine.perFrameTime = 1000 / engine.fps;
	engine.frameRateCap = false;

	engine.delta = 0;
	engine.lastFrameTime = 0;
	
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_WEBP);
	TTF_Init();

	engine.window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, engine.screenWidth, engine.screenHeight, SDL_WINDOW_SHOWN);
	if(engine.window == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed to create Game Window! SDL_Error: %s", SDL_GetError());
		engine.isRunning = SDL_FALSE;
		return NULL;
	}
	
	engine.renderer = SDL_CreateRenderer(engine.window, -1, SDL_RENDERER_ACCELERATED);
	if(engine.renderer == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed to create Game Renderer! SDL_Error: %s", SDL_GetError());
		engine.isRunning = SDL_FALSE;
		return NULL;
	}
	
	engine.defaultFont = TTF_OpenFont("assets/FreeSans.ttf", 24);
	if(engine.defaultFont == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed to load default font! TTF_Error: %s", TTF_GetError());
		engine.isRunning = false;
		return NULL;
	}

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
	
	engine.defaultScreenClearColor.r = 0;
	engine.defaultScreenClearColor.g = 200;
	engine.defaultScreenClearColor.b = 255;
	engine.defaultScreenClearColor.a = 255;

	SDL_VERSION(&engine.SDL_Version_C);
	SDL_GetVersion(&engine.SDL_Version_DLL);
	
	SDL_IMAGE_VERSION(&engine.SDL_IMG_Version_C);
	engine.SDL_IMG_Version_DLL = IMG_Linked_Version();
	
	SDL_TTF_VERSION(&engine.SDL_TTF_Version_C);
	engine.SDL_TTF_Version_DLL = TTF_Linked_Version();
	
	SDL_MIXER_VERSION(&engine.SDL_MIX_Version_C);
	engine.SDL_MIX_Version_DLL = Mix_Linked_Version();
	
	SGE_SetStateFunctions(&currentState, "SGE", NULL, NULL, NULL, NULL, NULL);
	SGE_LogPrintLine(SGE_LOG_INFO, "Straw Hat Game Engine Version 1.0");
	SGE_LogPrintLine(SGE_LOG_DEBUG, "Platform: %s", SDL_GetPlatform());
	//SGE_LogPrintLine(SGE_LOG_DEBUG, "Video Driver: %s", SDL_GetCurrentVideoDriver());
	//SGE_LogPrintLine(SGE_LOG_DEBUG, "Audio Driver: %s", SDL_GetCurrentAudioDriver());

	SDL_RendererInfo rendererInfo;
	SDL_GetRendererInfo(engine.renderer, &rendererInfo);
	SGE_LogPrintLine(SGE_LOG_DEBUG, "Renderer: %s", rendererInfo.name);

	engine.randSeed = time(NULL);
	srand(engine.randSeed);
	SGE_LogPrintLine(SGE_LOG_DEBUG, "C Random Seed: %d Seconds from epoch.", engine.randSeed);

	SGE_LogPrintLine(SGE_LOG_DEBUG, "SDL Version: %d.%d.%d (Compiled)", engine.SDL_Version_C.major, engine.SDL_Version_C.minor, engine.SDL_Version_C.patch);
	SGE_LogPrintLine(SGE_LOG_DEBUG, "SDL Version: %d.%d.%d (Linked)", engine.SDL_Version_DLL.major, engine.SDL_Version_DLL.minor, engine.SDL_Version_DLL.patch);
	
	SGE_LogPrintLine(SGE_LOG_DEBUG, "SDL Image Version: %d.%d.%d (Compiled)", engine.SDL_IMG_Version_C.major, engine.SDL_IMG_Version_C.minor, engine.SDL_IMG_Version_C.patch);
	SGE_LogPrintLine(SGE_LOG_DEBUG, "SDL Image Version: %d.%d.%d (Linked)", engine.SDL_IMG_Version_DLL->major, engine.SDL_IMG_Version_DLL->minor, engine.SDL_IMG_Version_DLL->patch);
	
	SGE_LogPrintLine(SGE_LOG_DEBUG, "SDL TTF Version: %d.%d.%d (Compiled)", engine.SDL_TTF_Version_C.major, engine.SDL_TTF_Version_C.minor, engine.SDL_TTF_Version_C.patch);
	SGE_LogPrintLine(SGE_LOG_DEBUG, "SDL TTF Version: %d.%d.%d (Linked)", engine.SDL_TTF_Version_DLL->major, engine.SDL_TTF_Version_DLL->minor, engine.SDL_TTF_Version_DLL->patch);
	
	SGE_LogPrintLine(SGE_LOG_DEBUG, "SDL Mixer Version: %d.%d.%d (Compiled)", engine.SDL_MIX_Version_C.major, engine.SDL_MIX_Version_C.minor, engine.SDL_MIX_Version_C.patch);
	SGE_LogPrintLine(SGE_LOG_DEBUG, "SDL Mixer Version: %d.%d.%d (Linked)", engine.SDL_MIX_Version_DLL->major, engine.SDL_MIX_Version_DLL->minor, engine.SDL_MIX_Version_DLL->patch);

	SGE_printf(SGE_LOG_DEBUG, "\n");
	
	if(!SGE_GUI_Init())
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed to initialize SGE GUI!");
		engine.isRunning = SDL_FALSE;
		Mix_CloseAudio();
		Mix_Quit();
		TTF_CloseFont(engine.defaultFont);
		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
		return NULL;
	}
	
	return &engine;
}

void SGE_Run(const char *startStateName)
{
	SGE_GameState *startState = SGE_GetState(startStateName);
	if(startState != NULL)
	{
		SGE_SetStateFunctions(&currentState, startState->name, startState->init, startState->quit, startState->handleEvents, startState->update, startState->render);
	}
	else
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "No state set, creating fallback!");
		SGE_AddState("No State", NULL, NULL, NULL, NULL, NULL);
		startState = SGE_GetState("No State");
		SGE_SetStateFunctions(&currentState, startState->name, startState->init, startState->quit, startState->handleEvents, startState->update, startState->render);
	}
	
	SGE_GUI_UpdateCurrentState(currentState.name);
	SGE_InitState(&currentState);
	
	while(engine.isRunning)
	{
		/* Calculate Delta time */
		engine.frameStartTime = SDL_GetTicks();
		engine.delta = (engine.frameStartTime - engine.lastFrameTime) / 1000.0;
		engine.lastFrameTime = engine.frameStartTime;
		
		/* Event Handling */
		SDL_GetMouseState(&engine.mouse_x, &engine.mouse_y);
		while(SDL_PollEvent(&engine.event) != 0)
		{
			engine.keyboardState = SDL_GetKeyboardState(NULL);
			if(engine.event.type == SDL_QUIT)
			{
				engine.isRunning = false;
			}
			SGE_GUI_HandleEvents();
			currentState.handleEvents();
		}
		
		/* Logic Updates */
		SGE_GUI_Update();
		currentState.update();
		
		/* Rendering */
		SGE_ClearScreenColor(engine.defaultScreenClearColor);
		currentState.render();
		SGE_GUI_Render();
		SDL_RenderPresent(engine.renderer);

		SGE_SwitchStates();
		
		if(engine.frameRateCap)
		{			
			/* Cap the framerate by delaying the next frame */
			engine.lastRenderTime = SDL_GetTicks() - engine.frameStartTime;
			if(engine.lastRenderTime < engine.perFrameTime)
			{
				SDL_Delay(engine.perFrameTime - engine.lastRenderTime);
			}
		}
	}
	
	SGE_QuitState(&currentState);
	SGE_SetStateFunctions(&currentState, "SGE", NULL, NULL, NULL, NULL, NULL);
	
	SGE_FreeLoadedStates();
	SGE_FreeStateList();
	SGE_GUI_Quit();
	
	Mix_CloseAudio();
	Mix_Quit();
	TTF_CloseFont(engine.defaultFont);
	
	SDL_DestroyRenderer(engine.renderer);
	engine.renderer = NULL;
	SDL_DestroyWindow(engine.window);
	engine.window = NULL;
	
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	SGE_LogPrintLine(SGE_LOG_INFO, "Quit SGE.");
}

void SGE_Quit()
{
	SGE_LogPrintLine(SGE_LOG_INFO, "SGE Quit Requested...");
	engine.isRunning = false;
}

/*
 * Toggles the window to fullscreen or windowed modes.
*/
void SGE_ToggleFullscreen()
{
	if(!engine.isFullscreen)
	{
		if(SDL_SetWindowFullscreen(engine.window, SDL_WINDOW_FULLSCREEN) == 0)
		{
			engine.isFullscreen = SDL_TRUE;
			SGE_LogPrintLine(SGE_LOG_INFO, "engine->isFullscreen: Toggled to %s", engine.isFullscreen ? "true" : "false");
		}
		else
		{
			SGE_LogPrintLine(SGE_LOG_WARNING, "Failed to enter Fullscreen! SDL_Error: %s", SDL_GetError());
		}
	}
	else
	{
		if(SDL_SetWindowFullscreen(engine.window, 0) == 0)
		{
			engine.isFullscreen = SDL_FALSE;
			SGE_LogPrintLine(SGE_LOG_INFO, "engine->isFullscreen: Toggled to %s", engine.isFullscreen ? "true" : "false");
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
	SDL_GetRenderDrawBlendMode(engine.renderer, &blendMode);
	SDL_Color drawColor;
	SDL_GetRenderDrawColor(engine.renderer, &drawColor.r, &drawColor.g, &drawColor.b, &drawColor.a);
	
	/* Quit the current state and the GUI to free all textures */
	SGE_QuitState(&currentState);
	SGE_GUI_Quit();
	
	SDL_DestroyRenderer(engine.renderer);
	engine.renderer = NULL;
	
	/* Toggle the vsync variable */
	engine.isVsyncOn = !engine.isVsyncOn;
	
	/* Recreate the renderer */
	if(engine.isVsyncOn)
		engine.renderer = SDL_CreateRenderer(engine.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	else
		engine.renderer = SDL_CreateRenderer(engine.window, -1, SDL_RENDERER_ACCELERATED);
	
	/* Set the new renderer's blend mode and color to the old one's */
	SDL_SetRenderDrawBlendMode(engine.renderer, blendMode);
	SDL_SetRenderDrawColor(engine.renderer, drawColor.r, drawColor.g, drawColor.b, drawColor.a);
	
	/* Reinitialize the GUI and the current state */
	SGE_GUI_Init();
	SGE_InitState(&currentState);
	
	if(engine.isVsyncOn)
	{
		engine.frameRateCap = false;
		SGE_LogPrintLine(SGE_LOG_INFO, "Turned OFF frame rate cap!");
	}
	SGE_LogPrintLine(SGE_LOG_INFO, "Toggled VSYNC %s!\n", engine.isVsyncOn ? "ON" : "OFF");
}

/*
 * Sets the frame rate cap to "fps", or disables capping if "fps" is 0.
*/
void SGE_SetTargetFPS(int fps)
{
	if(engine.isVsyncOn)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Can't set FPS, Vsync is ON!");
		return;
	}
	
	if(fps == 0)
	{
		engine.frameRateCap = false;
		SGE_LogPrintLine(SGE_LOG_INFO, "Turned OFF frame rate cap!");
	}
	else
	{
		engine.frameRateCap = true;
		engine.fps = fps;
		engine.perFrameTime = 1000 / fps;
		SGE_LogPrintLine(SGE_LOG_INFO, "Target FPS set to %d", engine.fps);
	}
}

void SGE_SetBackgroundColor(SDL_Color color)
{
	engine.defaultScreenClearColor = color;
}

void SGE_ClearScreen(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	SDL_SetRenderDrawColor(engine.renderer, r, g, b, a);
	SDL_RenderClear(engine.renderer);
}

void SGE_ClearScreenColor(SDL_Color color)
{
	SDL_SetRenderDrawColor(engine.renderer, color.r, color.g, color.b, color.a);
	SDL_RenderClear(engine.renderer);
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

/*
 * Checks for collision between a rectangle and the current mouse cursor position.
*/
bool SGE_isMouseOver(SDL_Rect *rect)
{
	/* Can be replaced with SDL_PointInRect() */
	if(SGE_GetEngineData()->mouse_x > rect->x && SGE_GetEngineData()->mouse_x < (rect->x + rect->w))
	{
		if(SGE_GetEngineData()->mouse_y > rect->y && SGE_GetEngineData()->mouse_y < (rect->y + rect->h))
			return true;
	}
	return false;
}
