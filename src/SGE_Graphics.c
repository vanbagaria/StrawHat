#include "SGE_Graphics.h"
#include "SGE.h"
#include "SGE_Logger.h"

#include "SDL_image.h"

#include <stdlib.h>
#include <stdio.h>

/*******************
 * Color constants
 *******************/

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

/**
 * \brief The SDL renderer used by all graphics functions.
 * 
 */
static SDL_Renderer *renderer = NULL;

SDL_Renderer *SGE_GetSDLRenderer()
{
	return renderer;
}

/***********************************
 * SGE Graphics Internal Functions
 ***********************************/

/**
 * \brief Initiallizes the game renderer.
 * 
 * \param window Pointer to the game SDL window.
 * \param vsync Whether to use V-SYNC.
 * \return true on success, false on failure.
 */
bool SGE_Graphics_Init(SDL_Window *window, bool vsync)
{
	SDL_RendererFlags rendererFlags = SDL_RENDERER_ACCELERATED;
	SDL_RendererInfo rendererInfo;
	
	if(vsync)
	{
		rendererFlags |= SDL_RENDERER_PRESENTVSYNC;
	}
	
	renderer = SDL_CreateRenderer(window, -1, rendererFlags);
	if(renderer == NULL)
	{
		return false;
	}
	
	SDL_GetRendererInfo(renderer, &rendererInfo);
	SGE_LogPrintLine(SGE_LOG_DEBUG, "Renderer: %s", rendererInfo.name);
	return true;
}

/**
 * \brief De-initializes the game renderer.
 * 
 */
void SGE_Graphics_Quit()
{
	SDL_DestroyRenderer(renderer);
	renderer = NULL;
}

/**
 * \brief Sets whether V-SYNC is enabled for the game renderer.
 * 
 * \param enabled Whether to enable or disable V-SYNC.
 * \return true on success, false on failure.
 */
bool SGE_Graphics_SetVSync(bool enabled)
{
	return !SDL_RenderSetVSync(renderer, enabled);
}

/***********************
 * Drawing Functions
 ***********************/

void SGE_ClearScreen(Uint8 r, Uint8 g, Uint8 b)
{
	SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
}

void SGE_ClearScreenSDL(SDL_Color color)
{
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
}

void SGE_SetDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void SGE_SetDrawColorSDL(SDL_Color color)
{
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void SGE_SetDrawBlendMode(SDL_BlendMode mode)
{
	SDL_SetRenderDrawBlendMode(renderer, mode);
}

void SGE_SetDrawClipRect(const SDL_Rect *rect)
{
	SDL_RenderSetClipRect(renderer, rect);
}

void SGE_DrawPoint(float x, float y)
{
	SDL_RenderDrawPointF(renderer, x, y);
}

void SGE_DrawLine(float x1, float y1, float x2, float y2)
{
	SDL_RenderDrawLineF(renderer, x1, y1, x2, y2);
}

void SGE_DrawRect(float x, float y, float w, float h)
{
	SDL_FRect rect = {x - (w / 2), y - (h / 2), w, h};
	SDL_RenderDrawRectF(renderer, &rect);
}

void SGE_DrawRectSDL(const SDL_Rect *rect)
{
	SDL_RenderDrawRect(renderer, rect);
}

void SGE_DrawFillRect(float x, float y, float w, float h)
{
	SDL_FRect rect = {x - (w / 2), y - (h / 2), w, h};
	SDL_RenderFillRectF(renderer, &rect);
}

void SGE_DrawFillRectSDL(const SDL_Rect *rect)
{
	SDL_RenderFillRect(renderer, rect);
}

/*****************************************
 * Custom Circle Drawing Function (Slow)
 *****************************************/

/**
 * \brief The calculated pixels for the circle cicrumference.
 * 
 */
static SDL_FPoint circlePoints[SGE_CIRCLE_MAX_RESOLUTION];

void SGE_DrawCircle(float x, float y, float radius, float quality)
{
    // Calculate the number of pixels to draw and limit it if too high
    float resolution = fabsf(radius * quality);
    if(resolution > SGE_CIRCLE_MAX_RESOLUTION)
    {
        resolution = SGE_CIRCLE_MAX_RESOLUTION;
    }
    
    float angle = 0.0f;
    float step = (2 * M_PI) / resolution; // Number of steps increase with resolution, slowing down the drawing.
    int i = 0;
    for(angle = 0; angle < 2 * M_PI; angle += step)
    {
        if(i >= SGE_CIRCLE_MAX_RESOLUTION)
        {
            i = SGE_CIRCLE_MAX_RESOLUTION - 1;
        }
        circlePoints[i].x = x + (radius * SDL_sinf(angle));
        circlePoints[i].y = y + (radius * SDL_cosf(angle));
        i++;
    }
    SDL_RenderDrawPointsF(renderer, circlePoints, resolution);
}

/****************
 * SGE_Texture
 ****************/

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
	
	gTexture->texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
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
	
	gTexture->texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
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
	
	gTexture->texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
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
	SDL_RenderCopyEx(renderer, gTexture->texture, &gTexture->clipRect, &gTexture->destRect, gTexture->rotation, NULL, gTexture->flip);
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

/**********************
 * SGE_AnimatedSprite
 **********************/

SGE_AnimatedSprite *SGE_CreateAnimatedSprite(const char *path, int nFrames, int fps)
{
	SGE_AnimatedSprite *sprite = (SGE_AnimatedSprite *)malloc(sizeof(SGE_AnimatedSprite));
	sprite->frameCount = nFrames;
	sprite->currentFrame = 0;
	sprite->fps = fps;
	sprite->increment = 1;
	sprite->lastDrawTime = 0;
	
	sprite->texture = SGE_LoadTexture(path);
	if(sprite->texture == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_ERROR, "Failed to load texture for AnimatedSprite!");
		free(sprite);
		return NULL;
	}
	
	sprite->texture->clipRect.w = sprite->texture->original_w / sprite->frameCount;
	sprite->texture->w = sprite->texture->clipRect.w;
	
	sprite->x = sprite->texture->x;
	sprite->y = sprite->texture->y;
	sprite->w = sprite->texture->w;
	sprite->h = sprite->texture->h;
	sprite->rotation = 0;
	sprite->flip = SDL_FLIP_NONE;
	
	return sprite;
}

void SGE_FreeAnimatedSprite(SGE_AnimatedSprite *sprite)
{
	if(sprite != NULL)
	{
		SGE_FreeTexture(sprite->texture);
		free(sprite);
	}
	else
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to free NULL AnimatedSprite!");
	}
}

void SGE_RenderAnimatedSprite(SGE_AnimatedSprite *sprite)
{
	if(!sprite->paused)
	{
		if(SDL_GetTicks() > sprite->lastDrawTime + 1000/sprite->fps)
		{
			sprite->currentFrame += sprite->increment;
			sprite->lastDrawTime = SDL_GetTicks();
		}
	
		sprite->texture->clipRect.x = sprite->currentFrame * sprite->texture->clipRect.w;
		if(sprite->currentFrame < 0)
		{
			sprite->texture->clipRect.x = sprite->texture->original_w - sprite->texture->clipRect.w;
			sprite->currentFrame = sprite->frameCount - 1;
		}
		if(sprite->currentFrame > sprite->frameCount - 1)
		{
			sprite->texture->clipRect.x = 0;
			sprite->currentFrame = 0;
		}
	}
	
	sprite->texture->x = sprite->x;
	sprite->texture->y = sprite->y;
	sprite->texture->w = sprite->w;
	sprite->texture->h = sprite->h;
	sprite->texture->rotation = sprite->rotation;
	sprite->texture->flip = sprite->flip;
	SGE_RenderTexture(sprite->texture);
}

void SGE_RestartAnimatedSprite(SGE_AnimatedSprite *sprite, int frame)
{
	sprite->currentFrame = frame - 1;
	sprite->lastDrawTime = SDL_GetTicks();
}

void SGE_SetAnimatedSpriteFPS(SGE_AnimatedSprite *sprite, int fps)
{
	if(fps == 0)
	{
		sprite->paused = true;
		return;
	}

	if(fps < 0)
	{
		if(sprite->increment > 0)
		{
			sprite->increment = -sprite->increment;
		}
	}
	else
	{
		sprite->increment = SDL_abs(sprite->increment);
	}	
	sprite->fps = SDL_abs(fps);
}

/*
 * Give this function the no of frames and the path to the folder containing seperate images
 * with names starting from 1.png, 2.png, ... [nFrames].png
 * It will save atlas.png in the same folder.
*/
void SGE_CreateSpriteSheet(const char *folderPath, int nFrames)
{
	char path[100];
	sprintf(path, folderPath);
	strcat(path, "/1.png");
	
	SDL_Surface *frame = IMG_Load(path);
	SDL_Surface *atlasSurface = SDL_CreateRGBSurface(0, nFrames * frame->w, frame->h, 32, 0, 0, 0, 0);
	SDL_Rect dest = {0, 0, frame->w, frame->h};
	
	int i = 0;
	for(i = 0; i < nFrames; i++)
	{
		sprintf(path, "%s/%d.png", folderPath, i + 1);
		frame = IMG_Load(path);
		dest.x = i * 51;
		SDL_BlitSurface(frame, NULL, atlasSurface, &dest);
		SDL_FreeSurface(frame);
	}
	
	sprintf(path, folderPath);
	strcat(path, "/atlas.png");
	IMG_SavePNG(atlasSurface, path);
	SDL_FreeSurface(atlasSurface);
}
