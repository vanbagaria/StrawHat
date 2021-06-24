#ifndef __SGE_ANIMATEDSPRITE_H__
#define __SGE_ANIMATEDSPRITE_H__

#include "SGE_Texture.h"
#include <stdbool.h>

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