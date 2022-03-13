/**
 * \file SGE.h
 * \brief The main include header for the SGE framework.
 * 
 * This header contains general system functions for the SGE core module.
 */

#ifndef __SGE_H__
#define __SGE_H__

#include "SDL.h"
#include "SGE_Scene.h"
#include <stdbool.h>

/**
 * \brief Configuration structure that contains initialization settings for SGE_Init().
 * 
 */
typedef struct
{
    int x; /**< The x position of the game window relative to top-left of display. */
    int y; /**< The y position of the game window relative to top-left of display. */
    bool fullscreen; /**< Whether or not the game window should be full screen. */
    bool resizable; /**< Whether or not the game window should be resizable. */
    bool borderless; /**< Whether or not the game window has border decorations. */
    bool vsync; /**< Whether or not the game window should have vertical sync enabled. */
} SGE_InitConfig;

/**
 * \brief Creates and returns an SGE init configuration with default values.
 * 
 * The default configuration values are:
 * x = SDL_WINDOWPOS_CENTERED;
 * y = SDL_WINDOWPOS_CENTERED;
 * fullscreen = false;
 * resizable = false;
 * borderless = false;
 * vsync = true;
 * 
 * \return An SGE_InitConfig structure filled with default initialization values.
 */
SGE_InitConfig SGE_CreateInitConfig();

/**
 * \brief Initializes SGE and creates the game window, must be called once before calling SGE_Start().
 * 
 * SGE_Start() must be called after a call to this function to start a user registered game scene.
 * 
 * \param title The title of the game window.
 * \param width The width of the game window in pixels.
 * \param height The height of the game window in pixels.
 * \param config The init configuration settings, NULL for default settings.
 * \return true if initialization is successful, false if there was an error.
 * 
 * \sa SGE_CreateInitConfig
 * \sa SGE_Start
 */
bool SGE_Init(const char *title, int width, int height, SGE_InitConfig *config);

/**
 * \brief Uninitializes SGE and destroys the game window, must be called if SGE has been initialized with SGE_Init().
 * 
 */
void SGE_Quit();

/**
 * \brief Starts a registered game scene after SGE is initialized with SGE_Init(). 
 *        Must be called once, after SGE is initialized and all scenes have been registered with SGE_AddScene().
 * 
 * Once this function is called, all control is given to the entry scene. Thus, all furthur logic should be in the game scenes.
 * 
 * \param entrySceneName The name of a game scene registered with SGE_AddScene()
 * 
 * \sa SGE_Init
 * \sa SGE_AddScene
 */
void SGE_Start(const char *entrySceneName);

/**
 * \brief Stops the currently running game scene. Must be called from inside of a game scene after SGE_Start() has been called.
 * 
 */
void SGE_Stop();

/**
 * \brief Sets the background color of the game window to the given RGB color.
 * 
 * \param r The red component of the color. (0 to 255)
 * \param g The green component of the color. (0 to 255)
 * \param b The blue component of the color. (0 to 255)
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
 * \param fps The maximum frames per second. Turns off limiter if 0.
 */
void SGE_SetFPSLimit(int fps);

/**
 * \brief Returns the current frame rate limit of the game.
 * 
 * \return Maximum FPS or 0 if limiter is turned off.
 */
int SGE_GetFPSLimit();

/**
 * \brief Sets whether V-SYNC is enabled for the game.
 * 
 */
void SGE_SetVSync(bool enabled);

/**
 * \brief Returns whether vsync is currently turned on.
 * 
 * \return true if vsync is on, false if vsync is off.
 */
bool SGE_IsVsync();

/**
 * \brief Toggles fullscreen mode for the game.
 * 
 */
void SGE_ToggleFullscreen();

/**
 * \brief Returns whether the game is currently in full screen mode.
 * 
 * \return true if game is in full screen mode, false if game is in windowed mode.
 */
bool SGE_IsFullscreen();

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
 * \brief Returns the name of the currently active game scene.
 * 
 * \return The name of the current game scene. 
 */
const char *SGE_GetCurrentSceneName();

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