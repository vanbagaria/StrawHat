/**
 * \file SGE_Graphics.h
 * \brief The include header for the SGE Graphics module.
 * 
 * This header contains graphics related functions for the SGE Graphics module.
 */

#ifndef __SGE_GRAPHICS_H__
#define __SGE_GRAPHICS_H__

#include "SDL.h"
#include "SDL_ttf.h"
#include <stdbool.h>

/**
 * \name SGE Color Constants
 *  
 * SDL_Color constants that contain RGB values for various colors.
 */
/**@{*/
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
/**@}*/

/***********************
 * Rendering Functions
 ***********************/

/**
 * \brief Fills the screen with the given RGB color.
 * 
 * \param r The red component of the color. (0 to 255)
 * \param g The green component of the color. (0-255)
 * \param b The blue component of the color. (0-255)
 */
void SGE_ClearScreen(Uint8 r, Uint8 g, Uint8 b);

/**
 * \brief Fills the screen with the given SDL_Color. Can use color constants from SGE_Graphics.h
 * 
 * \param color The SDL_Color to fill the screen with.
 */
void SGE_ClearScreenSDL(SDL_Color color);

/**
 * \brief Sets the color used for drawing to the given RGBA color.
 * 
 * \param r The red component of the color. (0 to 255)
 * \param g The green component of the color. (0-255)
 * \param b The blue component of the color. (0-255)
 * \param a The transparency of the drawn color.
 * 
 * \sa SGE_SetDrawBlendMode
 */
void SGE_SetDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/**
 * \brief Sets the color used for drawing to the given SDL_Color. Can use color constants from SGE_Graphics.h
 * 
 * \param color The SDL_Color to use as the drawing color.
 */
void SGE_SetDrawColorSDL(SDL_Color color);

/**
 * \brief Sets the blend mode used for drawing.
 * 
 * \param mode The SDL_BlendMode used for drawing.
 */
void SGE_SetDrawBlendMode(SDL_BlendMode mode);

/**
 * \brief Sets the screen clipping rectangle for drawing.
 *        Anything outside the rectangle is not drawn on screen.
 * 
 * \param rect Pointer to the rectangle to set as the screen clipping region. NULL to disable clipping.
 */
void SGE_SetDrawClipRect(const SDL_Rect *rect);

/**
 * \brief Draws a point on the screen at the given position.
 * 
 * \param x The x position of the point to draw.
 * \param y The y position of the point to draw.
 */
void SGE_DrawPoint(float x, float y);

/**
 * \brief Draws a line on the screen between the two given points.
 * 
 * \param x1 The x position of the start point.
 * \param y1 The y position of the start point.
 * \param x2 The x position of the end point.
 * \param y2 The y position of the end point.
 */
void SGE_DrawLine(float x1, float y1, float x2, float y2);

/**
 * \brief Draws a given rectangle on the screen.
 * 
 * \param x The x position of the center of the rectangle.
 * \param y The y position of the center of the rectangle.
 * \param w The width of the rectangle.
 * \param h The height of the rectangle.
 */
void SGE_DrawRect(float x, float y, float w, float h);

/**
 * \brief Draws a given SDL_Rect on the screen.
 * 
 * \param rect Pointer to the SDL_Rect to draw.
 */
void SGE_DrawRectSDL(const SDL_Rect *rect);

/**
 * \brief Draws a filled rectangle on the screen.
 * 
 * \param x The x position of the center of the rectangle.
 * \param y The y position of the center of the rectangle.
 * \param w The width of the rectangle.
 * \param h The height of the rectangle.
 */
void SGE_DrawFillRect(float x, float y, float w, float h);

/**
 * \brief Draws a given SDL_Rect filled on the screen.
 * 
 * \param rect Pointer to the SDL_Rect to draw.
 */
void SGE_DrawFillRectSDL(const SDL_Rect *rect);

/*****************************************
 * Custom Circle Drawing Function (Slow)
 *****************************************/

/**
 * \name SGE Circle Quality Constants 
 * 
 * Good default values for circle drawing quality.
 */
/**@{*/
#define SGE_CIRCLE_QUALITY_LOW    1.6f
#define SGE_CIRCLE_QUALITY_MEDIUM 3.1f
#define SGE_CIRCLE_QUALITY_HIGH   6.2f
/**@}*/

/**
 * \brief Maximum resolution for the circumference of a drawn circle.
 *        Resolution is the number of pixels drawn, it is calculated as (radius x quality).
 *        A circle radius over 1000 px will limit the circumference to 6200 px.
 * 
 */
#define SGE_CIRCLE_MAX_RESOLUTION 6200

/**
 * \brief Draws a circle on the screen.
 * 
 * \param x The x position of the circle center.
 * \param y The y position of the circle center.
 * \param radius The radius of the circle.
 * \param quality Quality factor for the drawn circle.
 *                The number of pixels drawn will be (radius x quality).
 *                Use constants SGE_CIRCLE_QUALITY_LOW, SGE_CIRCLE_QUALITY_MEDIUM and SGE_CIRCLE_QUALITY_HIGH.
 * 
 */
void SGE_DrawCircle(float x, float y, float radius, float quality);

/*********************
 * Texture Rendering
 *********************/

typedef struct
{
	int x, y, w, h;
	int original_w, original_h;
	int rotation;
	SDL_Texture *texture;
	SDL_RendererFlip flip;
	SDL_Rect clipRect;
	SDL_Rect destRect;
} SGE_Texture;

typedef enum
{
	SGE_TEXT_MODE_SOLID,
	SGE_TEXT_MODE_SHADED,
	SGE_TEXT_MODE_BLENDED
} SGE_TextRenderMode;

void SGE_SetTextureFontBGColor(SDL_Color bgColor);
void SGE_SetTextureWordWrap(int wrap);
SGE_Texture* SGE_LoadTexture(const char *path);
SGE_Texture* SGE_CreateTextureFromText(const char *text, TTF_Font *font, SDL_Color fg, SGE_TextRenderMode textMode);
void SGE_UpdateTextureFromText(SGE_Texture *gTexture, const char *text, TTF_Font *font, SDL_Color fg, SGE_TextRenderMode textMode);
void SGE_FreeTexture(SGE_Texture *gTexture);
void SGE_RenderTexture(SGE_Texture *gTexture);
void SGE_SetTextureColor(SGE_Texture *gTexture, Uint8 red, Uint8 green, Uint8 blue);
void SGE_SetTextureBlendMode(SGE_Texture *gTexture, SDL_BlendMode blending);
void SGE_SetTextureAlpha(SGE_Texture *gTexture, Uint8 alpha);

/**********************
 * SGE_AnimatedSprite
 **********************/

typedef struct
{
	/* Sprite transform */
	int x, y, w, h;
	int rotation;
	int flip;

	/* Texture Atlas */
	SGE_Texture *texture;
	
	int frameCount;
	int currentFrame;
	int increment;
	int fps;
	int lastDrawTime;
	bool paused;
} SGE_AnimatedSprite;

SGE_AnimatedSprite *SGE_CreateAnimatedSprite(const char *path, int nFrames, int fps);
void SGE_FreeAnimatedSprite(SGE_AnimatedSprite *sprite);
void SGE_RenderAnimatedSprite(SGE_AnimatedSprite *sprite);
void SGE_RestartAnimatedSprite(SGE_AnimatedSprite *sprite, int frame);
void SGE_SetAnimatedSpriteFPS(SGE_AnimatedSprite *sprite, int fps);

void SGE_CreateSpriteSheet(const char *folderPath, int nFrames);

#endif