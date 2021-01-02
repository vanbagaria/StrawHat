#include "SGE.h"
#include "SGE_Texture.h"
#include "SGE_Logger.h"
#include "SGE_Math.h"
#include "SGE_GUI.h"
#include <stdio.h>

/* Maximum length of the snake */
#define SNAKE_MAX_NODES 1000
/* Maximum no of turns saved */
#define SNAKE_MAX_TURNS 200
/* Size of snake node square in pixels */
#define SNAKE_NODE_SIZE 30
/* Speed of snake in pixels, should be same as node size for proper movement */
#define SNAKE_SPEED 30
/* Milliseconds of delay in updates, allows for boost speed up */
#define LEVEL_UPDATE_SPEED_MS 100
/* Size of the food square in pixels */
#define SNAKE_FOOD_SIZE 30

typedef enum
{
	SNAKE_DIRECTION_NONE,
	SNAKE_DIRECTION_UP,
	SNAKE_DIRECTION_DOWN,
	SNAKE_DIRECTION_LEFT,
	SNAKE_DIRECTION_RIGHT
} SnakeDirection;

typedef struct
{
	SDL_Rect rect;
	SnakeDirection direction;
} SnakeNode;

/* A snake structure holds a list of all snake nodes and one of all turn points as well as the current color of the snake */
typedef struct
{
	SnakeNode nodes[SNAKE_MAX_NODES];
	int nodeCount;
	SnakeNode turns[SNAKE_MAX_TURNS];
	int turnCount;
	SDL_Color color;
} Snake;

void snakeInit(Snake *snake);
void snakeAddNode(Snake *snake);
void snakeAddTurn(Snake *snake, SnakeDirection direction);
void snakeRemoveTurn(Snake *snake);
void snakeUpdate(Snake *snake);
void snakeRender(Snake *snake);

/* Level State Functions */
bool LevelInit();
void LevelQuit();
void LevelHandleEvents();
void LevelUpdate();
void LevelRender();

/* SGE Data and level state data*/
SGE_EngineData *SGE = NULL;
SGE_GameState level;

/* The snek */
Snake snake;

/* The food */
SDL_Rect food;
SDL_Color foodColor;

SDL_Color screenColor;
int score = 0;
int levelUpdateSpeed = LEVEL_UPDATE_SPEED_MS;
int lastTime = 0;

bool debugMode = false;
char debugInfoStr[50];
SGE_Texture *debugInfoImage = NULL;

char scoreInfoStr[50];
SGE_Texture *scoreInfoImage = NULL;

char helpInfoStr[150];
SGE_Texture *helpInfoImage = NULL;

bool paused = false;
bool showMouse = false;

SGE_WindowPanel *pauseWindow = NULL;
SGE_Button *backButton = NULL;
SGE_TextLabel *fullscreenLabel = NULL;
SGE_CheckBox *fullscreenCheckBox = NULL;
SGE_TextLabel *showMouseLabel = NULL;
SGE_CheckBox *showMouseCheckBox = NULL;

/* Entry Point */
int main(int argc, char **argv)
{
	SGE = SGE_Init("SGE Snake", 1280, 720);
	SGE_SetTargetFPS(120);
	
	SGE_SetStateFunctions(&level, "Level", LevelInit, LevelQuit, LevelHandleEvents, LevelUpdate, LevelRender);
	SGE_Run(&level);
	return 0;
}

void togglePaused(void *data)
{
	paused = !paused;
	pauseWindow->isVisible = !pauseWindow->isVisible;
	if(pauseWindow->isVisible)
		SDL_ShowCursor(true);
	else
		SDL_ShowCursor(showMouse);
}

void fullscreenCheckBoxCallback(void *data)
{
	SGE_ToggleFullscreen();
}

void showMouseCheckBoxCallback(void *data)
{
	showMouse = !showMouse;
	SDL_ShowCursor(showMouse);
}

bool LevelInit()
{
	snakeInit(&snake);
	screenColor = SGE_COLOR_GRAY;
	score = 0;
	/* Create Score Information Texture */
	sprintf(scoreInfoStr, "Score: %d", score);
	scoreInfoImage = SGE_CreateTextureFromText(scoreInfoStr, SGE->defaultFont, SGE_COLOR_WHITE, SGE_TEXT_MODE_SOLID);
	scoreInfoImage->x = 20;
	scoreInfoImage->y = 20;
	
	/* Create Help Information Texture */
	SGE_SetTextureWordWrap(750);
	sprintf(helpInfoStr, "[R]: Reset [Shift]: Speed Boost [B]: Change BG Color [C]: Change Color [D]: Debug mode [Space]: Add Node (Debug)");
	helpInfoImage = SGE_CreateTextureFromText(helpInfoStr, SGE->defaultFont, SGE_COLOR_WHITE, SGE_TEXT_MODE_BLENDED);
	helpInfoImage->x = SGE->screenWidth - helpInfoImage->w - 20;
	helpInfoImage->y = SGE->screenHeight - helpInfoImage->h - 20;
	SGE_SetTextureAlpha(helpInfoImage, 150);
	
	/* Create Debug Information Texture */
	sprintf(debugInfoStr, "x: %d, y:%d Nodes: %d Turns: %d", snake.nodes[0].rect.x, snake.nodes[0].rect.y, snake.nodeCount, snake.turnCount);
	debugInfoImage = SGE_CreateTextureFromText(debugInfoStr, SGE->defaultFont, SGE_COLOR_GREEN, SGE_TEXT_MODE_SOLID);
	debugInfoImage->x = scoreInfoImage->x;
	debugInfoImage->y = scoreInfoImage->y + scoreInfoImage->h + 5;
	
	/* Hide the cursor */
	SDL_ShowCursor(false);
	
	/* Initialize the food position and color */
	food.x = 200;
	food.y = 200;
	food.w = SNAKE_FOOD_SIZE;
	food.h = SNAKE_FOOD_SIZE;

	foodColor.r = 255;
	foodColor.g = 0;
	foodColor.b = 180;
	foodColor.a = 255;
	
	/* Create the pause window panel */
	pauseWindow = SGE_CreateWindowPanel("Pause", -1, -1, SGE->screenWidth, SGE->screenHeight);
	pauseWindow->isVisible = false;
	pauseWindow->isMovable = false;
	pauseWindow->isResizable = false;
	pauseWindow->isMinimizable = false;
	pauseWindow->alpha = 150;
	
	pauseWindow->backgroundColor = SGE_COLOR_GRAY;
	pauseWindow->borderColor = SGE_COLOR_BLACK;
	
	fullscreenLabel = SGE_CreateTextLabel("Fullscreen: ", SGE->screenWidth / 2, SGE->screenHeight / 2, SGE_COLOR_WHITE, pauseWindow);
	fullscreenCheckBox = SGE_CreateCheckBox(fullscreenLabel->x + fullscreenLabel->boundBox.w, fullscreenLabel->y, pauseWindow);
	fullscreenCheckBox->onMouseUp = fullscreenCheckBoxCallback;
	
	showMouseLabel = SGE_CreateTextLabel("Show Mouse: ", fullscreenLabel->x, fullscreenLabel->y + fullscreenLabel->boundBox.h + 25, SGE_COLOR_WHITE, pauseWindow);
	showMouseCheckBox = SGE_CreateCheckBox(showMouseLabel->x + showMouseLabel->boundBox.w, showMouseLabel->y, pauseWindow);
	showMouseCheckBox->onMouseUp = showMouseCheckBoxCallback;
	
	backButton = SGE_CreateButton("Back", showMouseLabel->x , showMouseLabel->y + showMouseLabel->boundBox.h + 25, pauseWindow);
	backButton->onMouseUp = togglePaused;
	backButton->normalColor = SGE_COLOR_GRAY;
	
	return true;
}

void LevelQuit()
{
	/* Free all textures */
	SGE_FreeTexture(debugInfoImage);
	SGE_FreeTexture(helpInfoImage);
	SGE_FreeTexture(scoreInfoImage);
}

void LevelHandleEvents()
{
	if(SGE->event.type == SDL_KEYDOWN)
	{
		/* Toggle debug mode when the 'D' key is pressed */
		if(SGE->event.key.keysym.sym == SDLK_d)
		{
			debugMode = !debugMode;
		}
		
		if(SGE->event.key.keysym.sym == SDLK_f)
		{
			SGE_ToggleFullscreen();
		}
		
		if(SGE->event.key.keysym.sym == SDLK_ESCAPE)
		{
			togglePaused(NULL);
		}
		
		if(!paused)
		{
			/* Add a snake node when the 'Space' key is pressed and debug mode is enabled */
			if(debugMode)
			{
				if(SGE->event.key.keysym.sym == SDLK_SPACE)
				{
					snakeAddNode(&snake);
				}
			}
			
			/* Speed up when the 'Shift' key is pressed */
			if(SGE->event.key.keysym.sym == SDLK_LSHIFT)
			{
				levelUpdateSpeed = LEVEL_UPDATE_SPEED_MS / 2;
			}
			
			/* Reset by reloading everything when the 'R' key is pressed */
			if(SGE->event.key.keysym.sym == SDLK_r)
			{
				SGE_SwitchToState(&level);
			}
			
			/* Change screen background to a random color when the 'B' key is pressed*/
			if(SGE->event.key.keysym.sym == SDLK_b)
			{
				screenColor.r = SGE_MathRandRangeInt(0, 255);
				screenColor.g = SGE_MathRandRangeInt(0, 255);
				screenColor.b = SGE_MathRandRangeInt(0, 255);
			}
			
			/* Change snake color to a random color when the 'C' key is pressed*/
			if(SGE->event.key.keysym.sym == SDLK_c)
			{
				snake.color.r = SGE_MathRandRangeInt(0, 255);
				snake.color.g = SGE_MathRandRangeInt(0, 255);
				snake.color.b = SGE_MathRandRangeInt(0, 255);
			}
			
			/* Add a turn point when one of the 'UP/DOWN/LEFT/RIGHT' keys is pressed */
			if(SGE->event.key.keysym.sym == SDLK_UP)
			{
				/* If there is only one node (head), then don't check for opposing keys, just add a turn point */
				if(snake.nodeCount == 1)
				{
					snakeAddTurn(&snake, SNAKE_DIRECTION_UP);
				}
				/* If there are more than one nodes, then only add a turn point if key direction is not opposite */
				else if(snake.nodes[0].direction != SNAKE_DIRECTION_DOWN)
				{
					snakeAddTurn(&snake, SNAKE_DIRECTION_UP);
				}
			}
			else if(SGE->event.key.keysym.sym == SDLK_DOWN)
			{
				if(snake.nodeCount == 1)
				{
					snakeAddTurn(&snake, SNAKE_DIRECTION_DOWN);
				}
				else if(snake.nodes[0].direction != SNAKE_DIRECTION_UP)
				{
					snakeAddTurn(&snake, SNAKE_DIRECTION_DOWN);
				}
			}
			else if(SGE->event.key.keysym.sym == SDLK_LEFT)
			{
				if(snake.nodeCount == 1)
				{
					snakeAddTurn(&snake, SNAKE_DIRECTION_LEFT);
				}
				else if(snake.nodes[0].direction != SNAKE_DIRECTION_RIGHT)
				{
					snakeAddTurn(&snake, SNAKE_DIRECTION_LEFT);
				}
			}
			else if(SGE->event.key.keysym.sym == SDLK_RIGHT)
			{
				if(snake.nodeCount == 1)
				{
					snakeAddTurn(&snake, SNAKE_DIRECTION_RIGHT);
				}
				else if(snake.nodes[0].direction != SNAKE_DIRECTION_LEFT)
				{
					snakeAddTurn(&snake, SNAKE_DIRECTION_RIGHT);
				}
			}
		}
	}
	else if(SGE->event.type == SDL_KEYUP)
	{
		/* Slow down when the 'Shift' key is released */
		if(SGE->event.key.keysym.sym == SDLK_LSHIFT)
		{
			levelUpdateSpeed = LEVEL_UPDATE_SPEED_MS;
		}
	}
}

void LevelUpdate()
{
	if(!paused)
	{
		/* Delay the Level update based on the levelUpdateSpeed variable */
		if(SGE->currentTime > lastTime + levelUpdateSpeed)
		{
			snakeUpdate(&snake);
			/* For update delay */
			lastTime = SGE->currentTime;
		}
		
		/* If Snake ate the food, spawn it elsewhere at random with a different color and increase the score */
		if(SGE_CheckRectsCollision(&food, &snake.nodes[0].rect))
		{
			food.x = SGE_MathRandRangeInt(0, SGE->screenWidth - food.w);
			food.y = SGE_MathRandRangeInt(0, SGE->screenHeight - food.h);
			
			foodColor.r = SGE_MathRandRangeInt(0, 255);
			foodColor.g = SGE_MathRandRangeInt(0, 255);
			foodColor.b = SGE_MathRandRangeInt(0, 255);
			
			score++;
			snakeAddNode(&snake);
		}
		
		/* Information Texture Updates */
		sprintf(scoreInfoStr, "Score: %d", score);
		SGE_UpdateTextureFromText(scoreInfoImage, scoreInfoStr, SGE->defaultFont, SGE_COLOR_WHITE, SGE_TEXT_MODE_SOLID);
		
		if(debugMode)
		{
			sprintf(debugInfoStr, "X: %d, Y:%d, Nodes: %d, Turns: %d", snake.nodes[0].rect.x, snake.nodes[0].rect.y, snake.nodeCount, snake.turnCount);
			SGE_UpdateTextureFromText(debugInfoImage, debugInfoStr, SGE->defaultFont, SGE_COLOR_GREEN, SGE_TEXT_MODE_SOLID);			
		}
	}
}

void LevelRender()
{
	SGE_ClearScreenColor(screenColor);
	
	snakeRender(&snake);
	
	/* Render the food using two rects, a colored filled rect and a white non-filled rect */
	SDL_SetRenderDrawColor(SGE->renderer, foodColor.r, foodColor.g, foodColor.b, foodColor.a);
	SDL_RenderFillRect(SGE->renderer, &food);
	SDL_SetRenderDrawColor(SGE->renderer, 255, 255, 255, 255);
	SDL_RenderDrawRect(SGE->renderer, &food);
	
	/* Render Information Textures */
	SGE_RenderTexture(helpInfoImage);
	SGE_RenderTexture(scoreInfoImage);
	if(debugMode)
	{
		SGE_RenderTexture(debugInfoImage);
	}
}

/* Initializes a snake structure */
void snakeInit(Snake *snake)
{
	/* Fill the nodes list with empty information */
	int i = 0;
	for(i = 0; i < SNAKE_MAX_NODES; i++)
	{
		snake->nodes[i].rect.x = 0;
		snake->nodes[i].rect.y = 0;
		snake->nodes[i].rect.w = 0;
		snake->nodes[i].rect.h = 0;
		snake->nodes[i].direction = SNAKE_DIRECTION_NONE;
	}
	snake->nodeCount = 0;
	
	/* Fill the turns list with empty information */
	for(i = 0; i < SNAKE_MAX_TURNS; i++)
	{
		snake->turns[i].rect.x = 0;
		snake->turns[i].rect.y = 0;
		snake->turns[i].rect.w = 0;
		snake->turns[i].rect.h = 0;
		snake->turns[i].direction = SNAKE_DIRECTION_NONE;
	}
	snake->turnCount = 0;
	
	/* Add the snake head as the first node in the list at a random screen position with random direction */
	snake->nodes[0].rect.x = SGE_MathRandRangeInt(0, SGE_GetEngineData()->screenWidth - SNAKE_NODE_SIZE);
	snake->nodes[0].rect.y = SGE_MathRandRangeInt(0, SGE_GetEngineData()->screenHeight - SNAKE_NODE_SIZE);
	snake->nodes[0].rect.w = SNAKE_NODE_SIZE;
	snake->nodes[0].rect.h = SNAKE_NODE_SIZE;
	snake->nodes[0].direction = SGE_MathRandRangeInt(1, 4);
	snake->nodeCount = 1;
	
	/* Set the default snake color */
	snake->color = SGE_COLOR_RED;
}

/* Adds a node to a snake's node list */
void snakeAddNode(Snake *snake)
{
	/* Stack Overflow! */
	if(snake->nodeCount == SNAKE_MAX_NODES)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Max snake nodes reached, can't add more!");
		return;
	}
	
	/* Set the new node's position based on the last node in the list  */
	switch(snake->nodes[snake->nodeCount-1].direction)
	{
		case SNAKE_DIRECTION_NONE:
		break;
		
		case SNAKE_DIRECTION_UP:
		snake->nodes[snake->nodeCount].rect.x = snake->nodes[snake->nodeCount - 1].rect.x;
		snake->nodes[snake->nodeCount].rect.y = snake->nodes[snake->nodeCount - 1].rect.y + SNAKE_NODE_SIZE;
		break;
		
		case SNAKE_DIRECTION_DOWN:
		snake->nodes[snake->nodeCount].rect.x = snake->nodes[snake->nodeCount - 1].rect.x;
		snake->nodes[snake->nodeCount].rect.y = snake->nodes[snake->nodeCount - 1].rect.y - SNAKE_NODE_SIZE;
		break;
		
		case SNAKE_DIRECTION_LEFT:
		snake->nodes[snake->nodeCount].rect.x = snake->nodes[snake->nodeCount - 1].rect.x + SNAKE_NODE_SIZE;
		snake->nodes[snake->nodeCount].rect.y = snake->nodes[snake->nodeCount - 1].rect.y;
		break;
		
		case SNAKE_DIRECTION_RIGHT:
		snake->nodes[snake->nodeCount].rect.x = snake->nodes[snake->nodeCount - 1].rect.x - SNAKE_NODE_SIZE;
		snake->nodes[snake->nodeCount].rect.y = snake->nodes[snake->nodeCount - 1].rect.y;
		break;
	}
	
	snake->nodes[snake->nodeCount].rect.w = SNAKE_NODE_SIZE;
	snake->nodes[snake->nodeCount].rect.h = SNAKE_NODE_SIZE;
	/* Set new node's direction based on last node */
	snake->nodes[snake->nodeCount].direction = snake->nodes[snake->nodeCount-1].direction;
	snake->nodeCount++;
}

/* Adds a turn point to a snake's turns list at the snake head's position with the given direction.
 * A turn point is a point at which a node will change it's direction to match the turn point's direction. */
void snakeAddTurn(Snake *snake, SnakeDirection direction)
{
	/* Stack overflow! */
	if(snake->turnCount == SNAKE_MAX_TURNS)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Max snake turns reached, can't add more!");
		return;
	}
	
	/* Don't allow a new turn point at the same location */
	if(snake->turnCount >= 1)
	{
		if(snake->nodes[0].rect.x == snake->turns[snake->turnCount-1].rect.x && snake->turns[snake->turnCount-1].rect.y == snake->nodes[0].rect.y)
		{
			SGE_LogPrintLine(SGE_LOG_WARNING, "Turn point repeated! Ignoring.");
			return;
		}
	}
	
	/* Set Turn point at snake head's position with the given direction */
	snake->turns[snake->turnCount].rect.x = snake->nodes[0].rect.x;
	snake->turns[snake->turnCount].rect.y = snake->nodes[0].rect.y;
	snake->turns[snake->turnCount].rect.w = SNAKE_NODE_SIZE;
	snake->turns[snake->turnCount].rect.h = SNAKE_NODE_SIZE;
	snake->turns[snake->turnCount].direction = direction;
	snake->turnCount++;
}

/* Removes the oldest point in the snake's turns list.
 * The turns list in snake works as a queue with turnCount as it's tail index.*/
void snakeRemoveTurn(Snake *snake)
{
	/* Move all elements in queue up one index so tail index can be freed */
	int i = 0;
	for(i = 0; i < snake->turnCount; i++)
	{
		snake->turns[i] = snake->turns[i + 1];
	}
	
	/* Decrease the tail by one so new elements can be added */
	snake->turnCount--;
}

/* The snakeUpdate function does the following:
 * Checks for collision of head node with all non-head nodes.
 * Checks for collision of each node with each turn point and change the node's direction if at a turn point.
 * Removes the oldest turn point in turns list when the last snake node passes through that point.
 * Moves the snake nodes in their current directions.
 * Checks if a node moved out of screen bounds, resets it to other side if it did. */
void snakeUpdate(Snake *snake)
{
	int i = 0;
	int j = 0;
	
	/* Checks for collision of head node with all non-head nodes */
	for(i = 1; i < snake->nodeCount; i++)
	{
		if(snake->nodes[0].rect.x == snake->nodes[i].rect.x)
		{
			if(snake->nodes[0].rect.y == snake->nodes[i].rect.y)
				SGE_SwitchToState(&level);
		}
	}
	
	for(i=0; i < snake->nodeCount; i++)
	{
		for(j=0; j < snake->turnCount; j++)
		{
			/* Checks for collision of each node with each turn point and change the node's direction if at a turn point */
			if(snake->nodes[i].rect.x == snake->turns[j].rect.x && snake->nodes[i].rect.y == snake->turns[j].rect.y)
			{
				snake->nodes[i].direction = snake->turns[j].direction;
				
				/* Removes the oldest turn point in turns list when the last snake node passes through that point */
				if(i == snake->nodeCount - 1)
				{
					snakeRemoveTurn(snake);
				}
			}
		}
		
		/* Moves the snake nodes in their current directions */
		switch(snake->nodes[i].direction)
		{
			case SNAKE_DIRECTION_NONE:
			break;
			
			case SNAKE_DIRECTION_UP:
			snake->nodes[i].rect.y -= SNAKE_SPEED;
			break;
			
			case SNAKE_DIRECTION_DOWN:
			snake->nodes[i].rect.y += SNAKE_SPEED;
			break;
			
			case SNAKE_DIRECTION_LEFT:
			snake->nodes[i].rect.x -= SNAKE_SPEED;
			break;
			
			case SNAKE_DIRECTION_RIGHT:
			snake->nodes[i].rect.x += SNAKE_SPEED;
			break;
		}
		
		/* Checks if a node moved out of screen bounds, resets it to other side if it did */
		if(snake->nodes[i].rect.x + snake->nodes[i].rect.w / 2 < 0)
		{
			snake->nodes[i].rect.x = SGE->screenWidth - snake->nodes[i].rect.w;
		}
		else if(snake->nodes[i].rect.x >= SGE->screenWidth)
		{
			snake->nodes[i].rect.x = 0;
		}
		if(snake->nodes[i].rect.y + snake->nodes[i].rect.h / 2 < 0)
		{
			snake->nodes[i].rect.y = SGE->screenHeight - snake->nodes[i].rect.h;
		}
		else if(snake->nodes[i].rect.y >= SGE->screenHeight)
		{
			snake->nodes[i].rect.y = 0;
		}
	}
}

/* Renders the snake node rects and turn point rects */
void snakeRender(Snake *snake)
{
	int i = 0;
	
	for(i = 0; i < snake->nodeCount; i++)
	{
		/* Draws the striped pattern */
		if(i % 2 == 0)
		{
			SDL_SetRenderDrawColor(SGE_GetEngineData()->renderer,  snake->color.r / 2, snake->color.g / 2, snake->color.b / 2, snake->color.a / 2);
		}
		else
		{
			SDL_SetRenderDrawColor(SGE_GetEngineData()->renderer,  snake->color.r, snake->color.g, snake->color.b, snake->color.a);
		}
		SDL_RenderFillRect(SGE_GetEngineData()->renderer, &snake->nodes[i].rect);
	}
	
	/* Draws a white non-filled rect at the head's position */
	SDL_SetRenderDrawColor(SGE->renderer, 255, 255, 255, 255);
	SDL_RenderDrawRect(SGE->renderer, &snake->nodes[0].rect);
	
	/* Only draw the turn points if debug mode is enabled */
	if(debugMode)
	{
		for(i = 0; i < snake->turnCount; i++)
		{
			SDL_SetRenderDrawColor(SGE_GetEngineData()->renderer, 0, 255, 0, 255);
			SDL_RenderDrawRect(SGE_GetEngineData()->renderer, &snake->turns[i].rect);
		}
	}
}
