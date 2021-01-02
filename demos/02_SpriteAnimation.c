#include "SGE.h"
#include "SGE_AnimatedSprite.h"

static SGE_EngineData *SGE = NULL;
static SGE_GameState level;
static SGE_AnimatedSprite* idleSprite = NULL;
static SGE_AnimatedSprite* walkSprite = NULL;

bool init()
{
	/* Load a spritesheet with 13 frames at 24 FPS */
	idleSprite = SGE_CreateAnimatedSprite("assets/SpriteIdle.png", 13, 24);
	/* Load a spritesheet with 24 frames at 24 FPS */
	walkSprite = SGE_CreateAnimatedSprite("assets/SpriteWalk.png", 24, 24);
	
	/* Set idleSprite's scale to 2x */
	idleSprite->w *= 2;
	idleSprite->h *= 2;
	
	walkSprite->x = idleSprite->x + idleSprite->w;
	
	return true;
}

void quit()
{
	SGE_FreeAnimatedSprite(walkSprite);
	SGE_FreeAnimatedSprite(idleSprite);
}

void render()
{
	SGE_ClearScreenColor(SGE_COLOR_GRAY);
	SGE_RenderAnimatedSprite(idleSprite);
	SGE_RenderAnimatedSprite(walkSprite);
}

int main(int argc, char **argv)
{
	SGE = SGE_Init("SGE Sprite Demo", 1280, 720);
	SGE_SetStateFunctions(&level, "Level", init, quit, NULL, NULL, render);
	SGE_Run(&level);
	return 0;
}