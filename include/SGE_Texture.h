#ifndef __SGE_TEXTURE__
#define __SGE_TEXTURE__

#include <SDL.h>
#include <SDL_ttf.h>

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

#endif