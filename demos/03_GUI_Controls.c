#include "SGE.h"
#include "SGE_GUI.h"

SGE_EngineData *SGE = NULL;
SGE_GameState level;

SGE_Button    *button   = NULL;
SGE_CheckBox  *checkBox = NULL;
SGE_TextLabel *label    = NULL;
SGE_Slider    *slider   = NULL;

bool LevelInit()
{
	SGE->defaultScreenClearColor = SGE_COLOR_GRAY;
	SGE_SetTargetFPS(60);
	
	button   = SGE_CreateButton("Button", 0, 0, NULL);
	checkBox = SGE_CreateCheckBox(0, 0, NULL);
	label    = SGE_CreateTextLabel("Label", 0, 0, SGE_COLOR_WHITE, NULL);
	slider   = SGE_CreateSlider(0, 0, NULL);
	
	SGE_ButtonSetPosition(button, SGE->screenCenter.x - button->boundBox.w / 2, 50);
	SGE_CheckBoxSetPositionNextTo(checkBox, button->boundBox, SGE_CONTROL_DIRECTION_DOWN_CENTERED, 0, 10);
	SGE_TextLabelSetPositionNextTo(label, checkBox->boundBox, SGE_CONTROL_DIRECTION_DOWN_CENTERED, 0, 10);
	SGE_SliderSetPositionNextTo(slider, label->boundBox, SGE_CONTROL_DIRECTION_DOWN_CENTERED, 0, 10);
	
	return true;
}

int main(int argc, char **argv)
{
	SGE = SGE_Init("SGE Controls Demo", 640, 480);
	SGE_SetStateFunctions(&level, "Level", LevelInit, NULL, NULL, NULL, NULL);
	SGE_Run(&level);
	return 0;
}
