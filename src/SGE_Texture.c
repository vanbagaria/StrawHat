#include "SGE_Texture.h"
#include "SGE.h"
#include "SGE_Logger.h"

#include <SDL2/SDL_image.h>

#include <stdlib.h>
#include <stdio.h>

/* Background color for Shaded text mode. Default is transparent white. */
static SDL_Color fontBGColor = {255, 255, 255, 1};

/* Word wrap in pixels for Blended text mode. */
static int wordWrap = 500;

void SGE_SetTextureFontBGColor(SDL_Color bgColor)
{
	fontBGColor = bgColor;
}

void SGE_SetTextureWordWrap(int wrap)
{
	wordWrap = wrap;
}

void SGE_EmptyTextureData(SGE_Texture *gTexture)
{
	gTexture->x = 0;
	gTexture->y = 0;
	gTexture->w = 0;
	gTexture->h = 0;
	gTexture->original_w = 0;
	gTexture->original_h = 0;
	
	gTexture->rotation = 0;
	gTexture->flip = SDL_FLIP_NONE;
	gTexture->texture = NULL;
	
	gTexture->destRect.x = 0;
	gTexture->destRect.y = 0;
	gTexture->destRect.w = 0;
	gTexture->destRect.h = 0;
	
	gTexture->clipRect.x = 0;
	gTexture->clipRect.y = 0;
	gTexture->clipRect.w = 0;
	gTexture->clipRect.h = 0;
}

SGE_Texture* SGE_LoadTexture(const char *path)
{
	SGE_Texture *gTexture = (SGE_Texture*)malloc(sizeof(SGE_Texture));
	
	SGE_EmptyTextureData(gTexture);
	
	SDL_Surface *tempSurface = NULL;
	tempSurface = IMG_Load(path);
	if(tempSurface == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed to load image: %s!", path, IMG_GetError());
		SGE_LogPrintLine(SGE_LOG_ERROR, "IMG_Error: %s", IMG_GetError());
		free(gTexture);
		return NULL;
	}
	
	gTexture->w = tempSurface->w;
	gTexture->h = tempSurface->h;
	gTexture->original_w = gTexture->w;
	gTexture->original_h = gTexture->h;
	gTexture->destRect.w = gTexture->w;
	gTexture->destRect.h = gTexture->h;
	gTexture->clipRect.w = gTexture->w;
	gTexture->clipRect.h = gTexture->h;
	
	gTexture->texture = SDL_CreateTextureFromSurface(SGE_GetEngineData()->renderer, tempSurface);
	SDL_FreeSurface(tempSurface);
	if(gTexture->texture == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed to create texture from image!");
		SGE_LogPrintLine(SGE_LOG_ERROR, "SDL_Error: %s", SDL_GetError());
		free(gTexture);
		return NULL;
	}
	
	return gTexture;
}

SGE_Texture* SGE_CreateTextureFromText(const char *text, TTF_Font *font, SDL_Color fg, SGE_TextRenderMode textMode)
{
	SGE_Texture *gTexture = (SGE_Texture*)malloc(sizeof(SGE_Texture));
	
	SGE_EmptyTextureData(gTexture);
	
	SDL_Surface *tempSurface = NULL;
	
	if(textMode == SGE_TEXT_MODE_SOLID)
	{
		tempSurface = TTF_RenderText_Solid(font, text, fg);
	}
	else if(textMode == SGE_TEXT_MODE_SHADED)
	{
		tempSurface = TTF_RenderText_Shaded(font, text, fg, fontBGColor);
	}
	else
	{
		tempSurface = TTF_RenderText_Blended_Wrapped(font, text, fg, wordWrap);
	}
	
	if(tempSurface == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed to render text surface!");
		SGE_LogPrintLine(SGE_LOG_ERROR, "TTF_Error: %s", TTF_GetError());
		free(gTexture);
		return NULL;
	}
	
	gTexture->w = tempSurface->w;
	gTexture->h = tempSurface->h;
	gTexture->original_w = gTexture->w;
	gTexture->original_h = gTexture->h;
	gTexture->destRect.w = gTexture->w;
	gTexture->destRect.h = gTexture->h;
	gTexture->clipRect.w = gTexture->w;
	gTexture->clipRect.h = gTexture->h;
	
	gTexture->texture = SDL_CreateTextureFromSurface(SGE_GetEngineData()->renderer, tempSurface);
	SDL_FreeSurface(tempSurface);
	if(gTexture->texture == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed to create texture from image!");
		SGE_LogPrintLine(SGE_LOG_ERROR, "SDL_Error: %s", SDL_GetError());
		free(gTexture);
		return NULL;
	}
	
	return gTexture;
}

void SGE_UpdateTextureFromText(SGE_Texture *gTexture, const char *text, TTF_Font *font, SDL_Color fg, SGE_TextRenderMode textMode)
{
	SDL_DestroyTexture(gTexture->texture);
	gTexture->texture = NULL;
	
	SDL_Surface *tempSurface = NULL;
	
	if(textMode == SGE_TEXT_MODE_SOLID)
	{
		tempSurface = TTF_RenderText_Solid(font, text, fg);
	}
	else if(textMode == SGE_TEXT_MODE_SHADED)
	{
		tempSurface = TTF_RenderText_Shaded(font, text, fg, fontBGColor);
	}
	else
	{
		tempSurface = TTF_RenderText_Blended_Wrapped(font, text, fg, wordWrap);
	}
	
	if(tempSurface == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed to render text surface!");
		SGE_LogPrintLine(SGE_LOG_ERROR, "TTF_Error: %s", TTF_GetError());
	}
	
	gTexture->w = tempSurface->w;
	gTexture->h = tempSurface->h;
	gTexture->original_w = gTexture->w;
	gTexture->original_h = gTexture->h;
	gTexture->destRect.w = gTexture->w;
	gTexture->destRect.h = gTexture->h;
	gTexture->clipRect.w = gTexture->w;
	gTexture->clipRect.h = gTexture->h;
	
	gTexture->texture = SDL_CreateTextureFromSurface(SGE_GetEngineData()->renderer, tempSurface);
	SDL_FreeSurface(tempSurface);
	if(gTexture->texture == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed to create texture from image!");
		SGE_LogPrintLine(SGE_LOG_ERROR, "SDL_Error: %s", SDL_GetError());
	}
}

void SGE_FreeTexture(SGE_Texture *gTexture)
{
	if(gTexture != NULL)
	{
		SDL_DestroyTexture(gTexture->texture);
		free(gTexture);
	}
	else
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to free NULL texture!");
	}
}

void SGE_RenderTexture(SGE_Texture *gTexture)
{
	gTexture->destRect.x = gTexture->x;
	gTexture->destRect.y = gTexture->y;
	gTexture->destRect.w = gTexture->w;
	gTexture->destRect.h = gTexture->h;
	SDL_RenderCopyEx(SGE_GetEngineData()->renderer, gTexture->texture, &gTexture->clipRect, &gTexture->destRect, gTexture->rotation, NULL, gTexture->flip);
}

void SGE_SetTextureColor(SGE_Texture *gTexture, Uint8 red, Uint8 green, Uint8 blue)
{
	SDL_SetTextureColorMod(gTexture->texture, red, green, blue);
}

void SGE_SetTextureBlendMode(SGE_Texture *gTexture, SDL_BlendMode blending)
{
	SDL_SetTextureBlendMode(gTexture->texture, blending);
}

void SGE_SetTextureAlpha(SGE_Texture *gTexture, Uint8 alpha)
{
	SDL_SetTextureAlphaMod(gTexture->texture, alpha);
}
