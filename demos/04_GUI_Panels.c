#include "SGE.h"
#include "SGE_GUI.h"
#include <stdio.h>

SGE_EngineData *SGE = NULL;
SGE_GameState level;

/* A panel with a button */
SGE_WindowPanel *buttonPanel = NULL;
SGE_Button *button = NULL;

/* A panel with a checkbox and a label */
SGE_WindowPanel *checkBoxPanel = NULL;
SGE_CheckBox *checkBox = NULL;
SGE_TextLabel *checkBoxLabel = NULL;

/* A panel with a slider and a label */
SGE_WindowPanel *sliderPanel = NULL;
SGE_Slider *slider = NULL;
char sliderValueStr[10];
SGE_TextLabel *sliderValueLabel = NULL;

/* Labels for information */
char mouseInfoStr[100];
SGE_TextLabel *mouseInfoLabel = NULL;
char windowInfoStr[100];
SGE_TextLabel *windowInfoLabel = NULL;
SGE_TextLabel *panelsListLabel = NULL;

/* Callback Function with void data pointer */
void toggleAlpha(void *panel)
{
	/* Type cast void pointer before use */
	SGE_WindowPanel *myPanel = (SGE_WindowPanel *)panel;
	if(myPanel->alpha > 200)
	{
		myPanel->alpha = 175;
	}
	else
	{
		myPanel->alpha = 255;
	}
}

bool LevelInit()
{
	SGE->defaultScreenClearColor = SGE_COLOR_GRAY;
	SGE_SetTargetFPS(120);
	
	/* Set up panels and their controls */
	buttonPanel = SGE_CreateWindowPanel("Button", 100, 200, 320, 240);
	button = SGE_CreateButton("Ghost me", 110, 100, buttonPanel);
	button->onMouseUp = toggleAlpha;
	button->onMouseUp_data = buttonPanel;
	
	checkBoxPanel = SGE_CreateWindowPanel("Checkbox", 500, 200, 320, 240);
	checkBoxLabel = SGE_CreateTextLabel("Ghost me?: ", 90, 100, SGE_COLOR_WHITE, checkBoxPanel);
	checkBox = SGE_CreateCheckBox(checkBoxLabel->x + checkBoxLabel->textImg->w, checkBoxLabel->y, checkBoxPanel);
	checkBox->onMouseUp = toggleAlpha;
	checkBox->onMouseUp_data = checkBoxPanel;
	
	sliderPanel = SGE_CreateWindowPanel("Slider", 900, 200, 320, 240);
	slider = SGE_CreateSlider(110, 100, sliderPanel);
	SGE_SliderSetValue(slider, 0.75);
	sprintf(sliderValueStr, "%.2f", slider->value);
	sliderValueLabel = SGE_CreateTextLabel(sliderValueStr, 50, 90, SGE_COLOR_WHITE, sliderPanel);
	SGE_TextLabelSetMode(sliderValueLabel, SGE_TEXT_MODE_SHADED);
	
	SGE_SetActiveWindowPanel(buttonPanel);

	/* Set up Labels */
	sprintf(mouseInfoStr, "Mouse X: %d, Mouse Y: %d", SGE->mouse_x, SGE->mouse_y);
	mouseInfoLabel = SGE_CreateTextLabel(mouseInfoStr, 0, 0, SGE_COLOR_GREEN, NULL);
	SGE_TextLabelSetMode(mouseInfoLabel, SGE_TEXT_MODE_SHADED);
	
	sprintf(windowInfoStr, "Top Panel: {%s} X: %d, Y: %d", SGE_GetActiveWindowPanel()->titleStr, SGE_GetActiveWindowPanel()->border.x, SGE_GetActiveWindowPanel()->border.y);
	windowInfoLabel = SGE_CreateTextLabel(windowInfoStr, 0, mouseInfoLabel->y + mouseInfoLabel->textImg->h, SGE_COLOR_RED, NULL);
	SGE_TextLabelSetMode(windowInfoLabel, SGE_TEXT_MODE_SHADED);
	
	SGE_SetTextureWordWrap(800);
	panelsListLabel = SGE_CreateTextLabel(SGE_GetPanelListAsStr(), 0, windowInfoLabel->y + windowInfoLabel->textImg->h, SGE_COLOR_RED, NULL);
	SGE_TextLabelSetMode(panelsListLabel, SGE_TEXT_MODE_SHADED);
	
	return true;
}

void LevelUpdate()
{
	/* Update Labels */
	sprintf(mouseInfoStr, "Mouse X: %d, Mouse Y: %d", SGE->mouse_x, SGE->mouse_y);
	SGE_TextLabelSetText(mouseInfoLabel, mouseInfoStr);
	
	sprintf(windowInfoStr, "Top Panel: {%s} X: %d, Y: %d", SGE_GetActiveWindowPanel()->titleStr, SGE_GetActiveWindowPanel()->border.x, SGE_GetActiveWindowPanel()->border.y);
	SGE_TextLabelSetText(windowInfoLabel, windowInfoStr);
	
	sprintf(sliderValueStr, "%.2f", slider->value);
	SGE_TextLabelSetText(sliderValueLabel, sliderValueStr);
	
	SGE_TextLabelSetText(panelsListLabel, SGE_GetPanelListAsStr());
	
	/* Update panel alpha using slider value */
	sliderPanel->alpha = slider->value * 255;
}

int main(int argc, char **argv)
{
	SGE = SGE_Init("SGE GUI Demo", 1280, 720);
	SGE_SetStateFunctions(&level, "Level", LevelInit, NULL, NULL, LevelUpdate, NULL);
	SGE_Run(&level);
	return 0;
}

