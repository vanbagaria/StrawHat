#include "SGE.h"
#include "SGE_GUI.h"
#include "SGE_Logger.h"

SGE_EngineData *SGE = NULL;

typedef enum
{
	RED,
	GREEN,
	BLUE,
	BLACK
} COLORS;

/* A panel with a ListBox */
SGE_WindowPanel *panel = NULL;
char colorOptions[4][LIST_OPTION_LENGTH] = {"Red", "Green", "Blue", "Black"};
SGE_ListBox *optionsBox;

void ChangeColor()
{
	switch(optionsBox->selection)
	{
		case RED:
		panel->borderColor = SGE_COLOR_RED;
		break;
		
		case GREEN:
		panel->borderColor = SGE_COLOR_GREEN;
		break;
		
		case BLUE:
		panel->borderColor = SGE_COLOR_BLUE;
		break;
		
		case BLACK:
		panel->borderColor = SGE_COLOR_BLACK;
		break;
	}
}

bool LevelInit()
{
	SGE_SetBackgroundColor(SGE_COLOR_GRAY);
	SGE_SetTargetFPS(60);
	
	/* Set up panels and controls */
	panel = SGE_CreateWindowPanel("Select Color:", 100, 200, 320, 240);
	optionsBox = SGE_CreateListBox(4, colorOptions, 50, 100, panel);
	optionsBox->onSelectionChange = ChangeColor;
	
	SGE_WindowPanelSetPosition(panel, SGE->screenCenter.x - panel->border.w / 2, SGE->screenCenter.y - panel->border.h / 2);
	return true;
}

int main(int argc, char **argv)
{
	SGE = SGE_Init("SGE ListBox Demo", 800, 600);
	SGE_AddState("ListBox Demo", LevelInit, NULL, NULL, NULL, NULL);
	SGE_Run("ListBox Demo");
	return 0;
}
