#ifndef __SGE_GRAPHICS_H__
#define __SGE_GRAPHICS_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

/*******************
 * Color constants
 *******************/

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

/***********************
 * Rendering Functions
 ***********************/

void SGE_ClearScreenRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void SGE_ClearScreen(SDL_Color color);
void SGE_SetDrawColorRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void SGE_SetDrawColor(SDL_Color color);
void SGE_DrawRect(SDL_Rect *rect);
void SGE_DrawFillRect(SDL_Rect *rect);
void SGE_DrawLine(int x1, int y1, int x2, int y2);

/**
 * \brief Updates the SDL_Renderer to be used by SGE_Graphics.c
 * 
 */
void SGE_Graphics_UpdateSDLRenderer();

/****************
 * SGE_Texture
 ****************/

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