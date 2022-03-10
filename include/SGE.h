/**
 * \file SGE.h
 * \brief The main include header for the SGE framework.
 * 
 * This header contains general system functions for the SGE core module.
 */

#ifndef __SGE_H__
#define __SGE_H__

#include "SDL.h"
#include "SGE_GameState.h"
#include <stdbool.h>

/**
 * \brief Initializes SGE and creates the game window, must be called once before calling SGE_Run().
 * 
 * SGE_Run() must be called after a call to this function to start a user registered game state.
 * 
 * \param title The title of the game window.
 * \param width The width of the game window in pixels.
 * \param height The height of the game window in pixels.
 * \return true if initialization is successful, false if there was an error.
 * 
 * \sa SGE_Run
 */
bool SGE_Init(const char *title, int width, int height);

/**
 * \brief Starts a registered game state after SGE is initialized with SGE_Init(). 
 *        Must be called once, after SGE is initialized and all states have been registered with SGE_AddState().
 * 
 * Once this function is called, all control is given to the entry state. Thus, all furthur logic should be in the game states.
 * This function uninitializes SGE when a quit is requested, so it must be called if SGE has been initialized. 
 * 
 * \param entryStateName The name of a game state registered with SGE_AddState()
 * 
 * \sa SGE_Init
 * \sa SGE_AddState
 */
void SGE_Run(const char *entryStateName);

/**
 * \brief Stops the currently running game state and deinitializes SGE.
 *        Must be called from inside of a game state.
 * 
 * This function will quit all states that are currently loaded, unregister all states that have been registered 
 * and destroy the game window.
 */
void SGE_Quit();

/**
 * \brief Sets the background color of the game window to the given RGB color.
 * 
 * \param r The red component of the color. (0 to 255)
 * \param g The green component of the color. (0-255)
 * \param b The blue component of the color. (0-255)
 */
void SGE_SetBackgroundColor(Uint8 r, Uint8 g, Uint8 b);

/**
 * \brief Sets the background color of the game window to the given SDL_Color. Can use color constants from SGE_Graphics.h
 * 
 * \param color The SDL_Color to set as the background color.
 */
void SGE_SetBackgroundColorSDL(SDL_Color color);

/**
 * \brief Limits the frame rate of the game.
 * 
 * \param fps The target frames per second. Turns off limiter if 0.
 */
void SGE_SetTargetFPS(int fps);

/**
 * \brief Toggles vsync on and off for the game.
 *        Do not call this from a state's init or quit functions,
 *        as it free's all loaded states and reinitalizes the current one.
 * 
 */
void SGE_ToggleVsync();

/**
 * \brief Returns whether vsync is currently turned on.
 * 
 * \return true if vsync is on, false if vsync is off.
 */
bool SGE_VsyncIsOn();

/**
 * \brief Toggles fullscreen for the game window.
 * 
 */
void SGE_ToggleFullscreen();

/**
 * \brief Returns the width of the game window.
 * 
 * \return The width of the game window, in pixels.
 */
int SGE_GetScreenWidth();

/**
 * \brief Returns the height of the game window.
 * 
 * \return The height of the game window, in pixels.
 */
int SGE_GetScreenHeight();

/**
 * \brief Returns the horizontal center of the game window.
 * 
 * \return The x-coordinate of the center point of the game window.
 */
int SGE_GetScreenCenterX();

/**
 * \brief Returns the vertical center of the game window.
 * 
 * \return The y-coordinate of the center point of the game window.
 */
int SGE_GetScreenCenterY();

/**
 * \brief Returns the current, horizontal mouse position in the game window.
 * 
 * \return The x-coordinate of the current mouse position.
 */
int SGE_GetMouseX();

/**
 * \brief Returns the current, vertical mouse position in the game window.
 * 
 * \return The y-coordinate of the current mouse position.
 */
int SGE_GetMouseY();

/**
 * \brief Returns the current position of the mouse in the game window as an SDL_Point.
 * 
 * \return The current mouse position.
 */
SDL_Point SGE_GetMousePosition();

/**
 * \brief Checks if a key is currently held down.
 * 
 * \param scancode The SDL_Scancode of the key to check for.
 * \return true if the key is pressed, false if the key is not pressed.
 */
bool SGE_KeyIsPressed(SDL_Scancode scancode);

/**
 * \brief Returns the time that passed since the start of the previous frame of the game.
 *        Can be used for frame rate independent calculations.
 * 
 * \return The time that has passed since the previous frame, in seconds.
 */
double SGE_GetDeltaTime();

/**
 * \brief Returns the name of the currently active game state.
 * 
 * \return The name of the current game state. 
 */
const char *SGE_GetCurrentStateName();

/**
 * \brief Returns the SDL_Renderer used for rendering by the game window.
 * 
 * \return The SDL renderer used by graphics functions.
 */
SDL_Renderer *SGE_GetSDLRenderer();

/**
 * \brief Returns the SDL_Event used for event handling by the game window.
 *        It contains information about the current event. 
 * 
 * \return The address of the event structure.
 */
SDL_Event *SGE_GetSDLEvent();

/**
 * \brief Checks if the current mouse cursor position is inside a given rectangle.
 * 
 * \param rect The rectangle for which to check if the mouse cursor is inside.
 * \return true if mouse cursor is inside rect,
 *         false otherwise.
 */
bool SGE_MouseInRect(const SDL_Rect *rect);

/**
 * \brief Checks if one rectangle intersects another rectangle.
 * 
 * \param r1 First rectanagle.
 * \param r2 Second rectangle.
 * \return true if rectangles intersect, false otherwise.
 */
bool SGE_RectInRect(const SDL_Rect *r1, const SDL_Rect *r2);

#endif