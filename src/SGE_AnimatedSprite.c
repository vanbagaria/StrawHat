#include "SGE_AnimatedSprite.h"
#include "SGE.h"
#include "SGE_Logger.h"
#include <SDL_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

SGE_AnimatedSprite *SGE_CreateAnimatedSprite(const char *path, int nFrames, int fps)
{
	SGE_AnimatedSprite *sprite = (SGE_AnimatedSprite *)malloc(sizeof(SGE_AnimatedSprite));
	sprite->frameCount = nFrames;
	sprite->currentFrame = 0;
	sprite->fps = fps;
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
	if(sprite != NULL)
	{
		if(!sprite->paused)
		{
			if(sprite->fps == 0)
			{
				sprite->fps = 1;
			}
			if(SGE_GetEngineData()->currentTime > sprite->lastDrawTime + 1000/sprite->fps)
			{
				sprite->currentFrame++;
				sprite->lastDrawTime = SGE_GetEngineData()->currentTime;
			}
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
		
		sprite->texture->x = sprite->x;
		sprite->texture->y = sprite->y;
		sprite->texture->w = sprite->w;
		sprite->texture->h = sprite->h;
		sprite->texture->rotation = sprite->rotation;
		sprite->texture->flip = sprite->flip;
		SGE_RenderTexture(sprite->texture);
	}
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
