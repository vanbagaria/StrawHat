#ifndef __SGE_H__
#define __SGE_H__

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

#include "SGE_GameState.h"

typedef struct
{
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
	const Uint8 *keyboardState;
	int mouse_x, mouse_y;
	
	TTF_Font *defaultFont;
	SDL_Color defaultScreenClearColor;
	
	int screenWidth;
	int screenHeight;
	SDL_Point screenCenter;
	bool isRunning;
	bool isFullscreen;
	bool isVsyncOn;
	
	int currentTime;
	int fps;
	int lastRenderTime;
	int perFrameTime;
	bool frameRateCap;
} SGE_EngineData;

SGE_EngineData *SGE_GetEngineData();
SGE_GameState *SGE_GetCurrentState();
SGE_EngineData *SGE_Init(const char *title, int screenWidth, int screenHeight);
void SGE_Run(SGE_GameState *startState);
void SGE_ToggleFullscreen();
void SGE_ToggleVsync();
void SGE_SetTargetFPS(int fps);
void SGE_ClearScreen(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void SGE_ClearScreenColor(SDL_Color color);
bool SGE_CheckRectsCollision(const SDL_Rect *r1, const SDL_Rect *r2);
bool SGE_isMouseOver(SDL_Rect *rect);

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

#endif