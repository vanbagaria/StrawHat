#ifndef __SGE_H__
#define __SGE_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

#include "SGE_GameState.h"

/* Contains globally accessible engine data */
typedef struct
{
	/* SDL Library versions */
	SDL_version SDL_Version_C;
	SDL_version SDL_Version_DLL;
	SDL_version SDL_IMG_Version_C;
	const SDL_version *SDL_IMG_Version_DLL;
	SDL_version SDL_TTF_Version_C;
	const SDL_version *SDL_TTF_Version_DLL;
	SDL_version SDL_MIX_Version_C;
	const SDL_version *SDL_MIX_Version_DLL;
	
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Event event;
	unsigned int randSeed;

	/* Current keyboard state */
	const Uint8 *keyboardState;
	/* Current mouse position */
	int mouse_x, mouse_y;
	/* Default engine font */
	TTF_Font *defaultFont;
	/* Default background fill color */
	SDL_Color defaultScreenClearColor;
	
	int screenWidth;
	int screenHeight;
	SDL_Point screenCenter;
	bool isRunning;
	bool isFullscreen;
	bool isVsyncOn;
	
	/* Frame timing data */
	int frameStartTime;
	int fps;
	int lastRenderTime;
	int perFrameTime;
	bool frameRateCap;
	double delta;
	unsigned int lastFrameTime;
} SGE_EngineData;

SGE_EngineData *SGE_GetEngineData();
SGE_GameState *SGE_GetCurrentState();
SGE_EngineData *SGE_Init(const char *title, int screenWidth, int screenHeight);
void SGE_Run(const char *startStateName);
void SGE_Quit();
void SGE_ToggleFullscreen();
void SGE_ToggleVsync();
void SGE_SetTargetFPS(int fps);
void SGE_SetBackgroundColor(SDL_Color color);
void SGE_ClearScreen(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void SGE_ClearScreenColor(SDL_Color color);
bool SGE_CheckRectsCollision(const SDL_Rect *r1, const SDL_Rect *r2);
bool SGE_isMouseOver(SDL_Rect *rect);

/* Color constants */

extern const SDL_Color SGE_COLOR_WHITE;
extern const SDL_Color SGE_COLOR_BLACK;
extern const SDL_Color SGE_COLOR_GRAY;
extern const SDL_Color SGE_COLOR_RED;
extern const SDL_Color SGE_COLOR_GREEN;
extern const SDL_Color SGE_COLOR_BLUE;
extern const SDL_Color SGE_COLOR_YELLOW;
extern const SDL_Color SGE_COLOR_PINK;
extern const SDL_Color SGE_COLOR_AQUA;
extern const SDL_Color SGE_COLOR_LIGHT_GRAY;
extern const SDL_Color SGE_COLOR_LIGHT_PURPLE;
extern const SDL_Color SGE_COLOR_DARK_RED;
extern const SDL_Color SGE_COLOR_CERISE;
extern const SDL_Color SGE_COLOR_ORANGE;
extern const SDL_Color SGE_COLOR_INDIGO;
extern const SDL_Color SGE_COLOR_PURPLE;

#endif