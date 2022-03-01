#ifndef __SGE_H__
#define __SGE_H__

#include "SDL.h"
#include <stdbool.h>

#include "SGE_GameState.h"

bool SGE_Init(const char *title, int width, int height);
void SGE_Run(const char *startStateName);
void SGE_Quit();

void SGE_SetBackgroundColor(SDL_Color color);
void SGE_SetTargetFPS(int fps);
void SGE_ToggleVsync();
bool SGE_VsyncIsOn();
void SGE_ToggleFullscreen();

int SGE_GetScreenWidth();
int SGE_GetScreenHeight();
int SGE_GetScreenCenterX();
int SGE_GetScreenCenterY();
int SGE_GetMouseX();
int SGE_GetMouseY();
SDL_Point SGE_GetMousePosition();
bool SGE_KeyIsPressed(SDL_Scancode scancode);
double SGE_GetDeltaTime();

SGE_GameState *SGE_GetCurrentState();
SDL_Renderer *SGE_GetSDLRenderer();
SDL_Event *SGE_GetSDLEvent();

/**
 * \brief Checks if the current mouse cursor position is inside a given rectangle.
 * 
 * \param rect The rectangle for which to check if the mouse cursor is inside.
 * \return true if mouse cursor is inside rect,
 *         false otherwise.
 */
bool SGE_MouseInRect(const SDL_Rect *rect);

bool SGE_CheckRectsCollision(const SDL_Rect *r1, const SDL_Rect *r2);

#endif