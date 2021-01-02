#include "SGE.h"
#include "SGE_GUI.h"
#include "SGE_Logger.h"
#include <stdio.h>

SGE_EngineData *SGE = NULL;
SGE_GameState level;

SGE_WindowPanel *panel;

SGE_TextInputBox *inputBox;
SGE_TextLabel    *inputBoxLabel;

char options[4][LIST_OPTION_LENGTH] = {"Option 1", "Option 2", "Option 3", "Option 4"};
SGE_ListBox   *listBox;
SGE_TextLabel *listBoxLabel;

bool LevelInit()
{
	SGE->defaultScreenClearColor = SGE_COLOR_GRAY;
	SGE_SetTargetFPS(60);
	
	panel = SGE_CreateWindowPanel("Panel", 200, 100, 640, 480);
	
	inputBox      = SGE_CreateTextInputBox(100, 200, 100, panel);
	inputBoxLabel = SGE_CreateTextLabel("Enter Text: ", 0, 0, SGE_COLOR_WHITE, panel);
	SGE_TextLabelSetPositionNextTo(inputBoxLabel, inputBox->boundBox, SGE_CONTROL_DIRECTION_LEFT, 0, 0);
	
	listBox      = SGE_CreateListBox(4, options, 0, 0, panel);
	SGE_ListBoxSetPositionNextTo(listBox, inputBox->boundBox, SGE_CONTROL_DIRECTION_DOWN, 0, 5);
	listBoxLabel = SGE_CreateTextLabel("Select Option: ", 0, 0, SGE_COLOR_WHITE, panel);
	SGE_TextLabelSetPositionNextTo(listBoxLabel, listBox->boundBox, SGE_CONTROL_DIRECTION_LEFT, 0, 0);
	return true;
}

int main(int argc, char **argv)
{
	SGE = SGE_Init("SGE ListBox & TextBox", 1280, 720);
	SGE_SetStateFunctions(&level, "Level", LevelInit, NULL, NULL, NULL, NULL);
	SGE_Run(&level);
	return 0;
}
