#include "SGE_GUI.h"
#include "SGE_Logger.h"
#include "SGE.h" // To be removed
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/* Default GUI Fonts */
static TTF_Font *buttonFont = NULL;
static TTF_Font *panelTitleFont = NULL;
static TTF_Font *labelFont = NULL;
static TTF_Font *textBoxFont = NULL;
static TTF_Font *listBoxFont = NULL;

/**
 * \brief A list of GUI controls that is held by each state in SGE.
 * 
 * Each state holds a copy of this structure to maintain it's controls.
 * The SGE GUI holds a reference to the current state's control list to be used by
 * GUI functions.
 */
typedef struct SGE_GUI_ControlList
{
	/* Panel Stack */
	SGE_WindowPanel *panels[STATE_MAX_PANELS];
	int panelCount;

	/* Parentless Controls */

	SGE_Button *buttons[STATE_MAX_BUTTONS];
	int buttonCount;
	SGE_CheckBox *checkBoxes[STATE_MAX_CHECKBOXES];
	int checkBoxCount;
	SGE_TextLabel *labels[STATE_MAX_LABELS];
	int labelCount;
	SGE_Slider *sliders[STATE_MAX_SLIDERS];
	int sliderCount;
	SGE_TextInputBox *textInputBoxes[STATE_MAX_TEXT_INPUT_BOXES];
	int textInputBoxCount;
	SGE_ListBox *listBoxes[STATE_MAX_LISTBOXES];
	int listBoxCount;
} SGE_GUI_ControlList;

/* List pointer for current state's controls */
static SGE_GUI_ControlList *currentStateControls = NULL;

/* 
* List for debug state's controls.
* The debug state is a set of controls that overlay on top of the current engine state.
* These controls include labels with FPS information, the debug panel and more.
* The debug state is toggled with the F1 key.
*/
static SGE_GUI_ControlList debugStateControls;

/* Toggle to disable debug state */
static bool showDebugState = false;
static SGE_TextLabel *debugStateLabel;

/* The debug panel, toggled with F2 */
static SGE_WindowPanel *debugPanel;

/* Toggle for displaying control boundboxes */
static bool showControlBounds = false;
static SGE_CheckBox *showBoundsChkBox;
static SGE_TextLabel *showBoundsLabel;
static SDL_Color controlBoundsColor;

/* Toggle for displaying frame rate information */
static bool showFrameInfo = true;
static SGE_CheckBox *showFrameInfoChkBox;
static SGE_TextLabel *showFrameInfoLbl;

/* List of panels in current state as a string */
static char panelsListStr[200];

static SGE_TextLabel *panelListLabel;
static SGE_TextLabel *stateListLabel;

/* Frame rate counter */
static unsigned int frameCounter;
static unsigned int countedFPS;
static unsigned int lastFPSCountTime;

/* Frame info labels */
static SGE_TextLabel *deltaLabel;
static SGE_TextLabel *fpsLabel;
static SGE_TextLabel *vsyncLabel;

/* Frame info labels update interval in ms */
static unsigned int labelUpdateInterval = 250;
static unsigned int lastLabelUpdateTime;

static void SGE_GUI_ControlList_HandleEvents(SGE_GUI_ControlList *controls);
static void SGE_GUI_ControlList_Update(SGE_GUI_ControlList *controls);
static void SGE_GUI_ControlList_Render(SGE_GUI_ControlList *controls);
static void SGE_GUI_FreeControlList(SGE_GUI_ControlList *controls);

/**
 * \brief Creates a new SGE_GUI_ControlList for a game state.
 * 
 * \return The address of the created list of GUI controls.
 */
SGE_GUI_ControlList *SGE_CreateGUIControlList()
{
	SGE_GUI_ControlList *controls = (SGE_GUI_ControlList*)malloc(sizeof(SGE_GUI_ControlList));
	controls->buttonCount = 0;
	controls->checkBoxCount = 0;
	controls->sliderCount = 0;
	controls->labelCount = 0;
	controls->listBoxCount = 0;
	controls->textInputBoxCount = 0;
	
	controls->panelCount = 0;
	return controls;
}

/**
 * \brief Destroys a game state's GUI control list.
 * 
 * This function will delete all the GUI controls that were created by the specified state.
 * It is called automatically when a state is freed by the SGE_QuitState() function.
 * 
 * \param controls A address of the address of the GUI control list that should be freed.
 */
void SGE_DestroyGUIControlList(SGE_GUI_ControlList **controls)
{
	if(*controls != NULL)
		SGE_GUI_FreeControlList(*controls);
	free(*controls);
	*(controls) = NULL;
}

/**
 * \brief Returns the SGE_GUI_ControlList pointer for a registered state.
 * 
 * This function is defined in SGE_GameState.c
 * 
 * \param name The name of the registered state whose GUI control list should be returned.
 * \return The address of the GUI control list of the registered state.
 */
SGE_GUI_ControlList *SGE_GetStateGUIControlList(const char *name);

/* GUI control functions */

static void SGE_DestroyButton(SGE_Button *button);
static void SGE_ButtonHandleEvents(SGE_Button *button);
static void SGE_ButtonUpdate(SGE_Button *button);
static void SGE_ButtonRender(SGE_Button *button);

static void SGE_DestroyCheckBox(SGE_CheckBox *checkBox);
static void SGE_CheckBoxHandleEvents(SGE_CheckBox *checkBox);
static void SGE_CheckBoxUpdate(SGE_CheckBox *checkBox);
static void SGE_CheckBoxRender(SGE_CheckBox *checkBox);

static void SGE_DestroyTextLabel(SGE_TextLabel *label);
static void SGE_TextLabelRender(SGE_TextLabel *label);

static void SGE_DestroySlider(SGE_Slider *slider);
static void SGE_SliderHandleEvents(SGE_Slider *slider);
static void SGE_SliderUpdate(SGE_Slider *slider);
static void SGE_SliderRender(SGE_Slider *slider);

static void SGE_DestroyTextInputBox(SGE_TextInputBox *textInputBox);
static void SGE_TextInputBoxHandleEvents(SGE_TextInputBox *textInputBox);
static void SGE_TextInputBoxUpdate(SGE_TextInputBox *textInputBox);
static void SGE_TextInputBoxRender(SGE_TextInputBox *textInputBox);

static void SGE_DestroyListBox(SGE_ListBox *listBox);
static void SGE_ListBoxHandleEvents(SGE_ListBox *listBox);
static void SGE_ListBoxUpdate(SGE_ListBox *listBox);
static void SGE_ListBoxRender(SGE_ListBox *listBox);

static void SGE_DestroyWindowPanel(SGE_WindowPanel *panel);
static void SGE_WindowPanelHandleEvents(SGE_WindowPanel *panel);
static void SGE_WindowPanelUpdate(SGE_WindowPanel *panel);
static void SGE_WindowPanelRender(SGE_WindowPanel *panel);
static void SGE_WindowPanelCalculateMCR(SGE_WindowPanel *panel, SDL_Rect boundBox);
static void SGE_WindowPanelShouldEnableHorizontalScroll(SGE_WindowPanel *panel);
static void SGE_WindowPanelShouldEnableVerticalScroll(SGE_WindowPanel *panel);

/* Handler for frame info labels toggle */
static void onShowFrameInfoToggle(void *data)
{
	if(!showFrameInfoChkBox->isChecked)
	{
		SGE_TextLabelSetVisible(deltaLabel, false);
		SGE_TextLabelSetVisible(fpsLabel, false);
		SGE_TextLabelSetVisible(vsyncLabel, false);
	}
	else
	{
		SGE_TextLabelSetVisible(deltaLabel, true);
		SGE_TextLabelSetVisible(fpsLabel, true);
		SGE_TextLabelSetVisible(vsyncLabel, true);
	}
}

static void SGE_GUI_DebugState_Init()
{
	SGE_GUI_ControlList *tempCurrentStateControls = currentStateControls;
	currentStateControls = &debugStateControls;

	debugStateLabel = SGE_CreateTextLabel("Debug State (F1)", 0, 0, SGE_COLOR_GREEN, NULL);
	SGE_TextLabelSetPosition(debugStateLabel, 0, SGE_GetScreenHeight() - debugStateLabel->boundBox.h);

	/* Create the debug panel */
	debugPanel = SGE_CreateWindowPanel("Debug Panel (F2)", 0, 0, 320, 240);
	debugPanel->isVisible = false;
	//debugPanel->borderColor = SGE_COLOR_GRAY;
	debugPanel->alpha = 200;
	//debugPanel->backgroundColor = SGE_COLOR_GRAY;
	//debugPanel->borderColor = SGE_COLOR_BLACK;
	debugPanel->isMovable = false;
	debugPanel->isMinimizable = false;
	debugPanel->isResizable = false;
	SGE_WindowPanelSetPosition(debugPanel, SGE_GetScreenWidth() - debugPanel->boundBox.w, SGE_GetScreenHeight() - debugPanel->boundBox.h);

	/* Create panel and state list labels */
	panelListLabel = SGE_CreateTextLabel(panelsListStr, 10, 10, SGE_COLOR_WHITE, debugPanel);
	panelListLabel->mode = SGE_TEXT_MODE_SHADED;
	stateListLabel = SGE_CreateTextLabel(" ", 10, 35, SGE_COLOR_WHITE, debugPanel);
	stateListLabel->mode = SGE_TEXT_MODE_SHADED;

	/* Create toggles */
	showBoundsLabel = SGE_CreateTextLabel("Show Bound Boxes:", 0, 0, SGE_COLOR_BLACK, debugPanel);
	SGE_TextLabelSetPositionNextTo(showBoundsLabel, stateListLabel->boundBox, SGE_CONTROL_DIRECTION_DOWN, 0, 10);
	showBoundsChkBox = SGE_CreateCheckBox(0, 0, debugPanel);
	SGE_CheckBoxSetPositionNextTo(showBoundsChkBox, showBoundsLabel->boundBox, SGE_CONTROL_DIRECTION_RIGHT_CENTERED, 15, 0);
	
	showFrameInfoLbl = SGE_CreateTextLabel("Show Frame Info:", 0, 0, SGE_COLOR_BLACK, debugPanel);
	SGE_TextLabelSetPositionNextTo(showFrameInfoLbl, showBoundsLabel->boundBox, SGE_CONTROL_DIRECTION_DOWN, 0, 10);
	showFrameInfoChkBox = SGE_CreateCheckBox(0, 0, debugPanel);
	SGE_CheckBoxSetPositionNextTo(showFrameInfoChkBox, showFrameInfoLbl->boundBox, SGE_CONTROL_DIRECTION_RIGHT_CENTERED, 15, 0);
	showFrameInfoChkBox->onMouseUp = onShowFrameInfoToggle;
	showFrameInfoChkBox->isChecked = true;

	/* Create frame info labels */
	deltaLabel = SGE_CreateTextLabel(" ", 0, 0, SGE_COLOR_WHITE, NULL);
	SGE_TextLabelSetPositionNextTo(deltaLabel, debugStateLabel->boundBox, SGE_CONTROL_DIRECTION_UP, 0, 0);
	SGE_TextLabelSetMode(deltaLabel, SGE_TEXT_MODE_SHADED);
	SGE_TextLabelSetBGColor(deltaLabel, SGE_COLOR_BLACK);

	fpsLabel = SGE_CreateTextLabel(" ", 0, 0, SGE_COLOR_WHITE, NULL);
	SGE_TextLabelSetPositionNextTo(fpsLabel, deltaLabel->boundBox, SGE_CONTROL_DIRECTION_UP, 0, 0);
	SGE_TextLabelSetMode(fpsLabel, SGE_TEXT_MODE_SHADED);
	SGE_TextLabelSetBGColor(fpsLabel, SGE_COLOR_BLACK);

	vsyncLabel = SGE_CreateTextLabel(" ", 0, 0, SGE_COLOR_WHITE, NULL);
	SGE_TextLabelSetPositionNextTo(vsyncLabel, fpsLabel->boundBox, SGE_CONTROL_DIRECTION_UP, 0, 0);
	SGE_TextLabelSetMode(vsyncLabel, SGE_TEXT_MODE_SHADED);
	SGE_TextLabelSetBGColor(vsyncLabel, SGE_COLOR_BLACK);

	currentStateControls = tempCurrentStateControls;
}

static void SGE_GUI_DebugState_Update()
{
	/* Sync toggles with checkboxes */
	showControlBounds = showBoundsChkBox->isChecked;
	showFrameInfo = showFrameInfoChkBox->isChecked;

	/* Update frame info labels */
	if(showFrameInfo)
	{
		/* Calculate the framerate */
		frameCounter++;
		if((SDL_GetTicks() - lastFPSCountTime) > 1000)
		{
			countedFPS = frameCounter;
			frameCounter = 0;
			lastFPSCountTime = SDL_GetTicks();
		}

		if((SDL_GetTicks() - lastLabelUpdateTime) > labelUpdateInterval)
		{
			SGE_TextLabelSetTextf(deltaLabel, "dt: %.3f s", SGE_GetDeltaTime());
			SGE_TextLabelSetTextf(fpsLabel, "fps: %d", countedFPS);
			if(SGE_VsyncIsOn()) {
				SGE_TextLabelSetTextf(vsyncLabel, "vsync: on");
			}
			else {
				SGE_TextLabelSetTextf(vsyncLabel, "vsync: off");
			}
			lastLabelUpdateTime = SDL_GetTicks();
		}
	}
}

const char *SGE_GetPanelListAsStr()
{
	return panelsListStr;
}

/* Prints a list of all panels in the current state onto a string */
static void printPanelsStr()
{
	int i = 0;
	char tempStr[100];

	SGE_WindowPanel **panels = currentStateControls->panels;
	int panelCount = currentStateControls->panelCount;

	if(panelCount > 0)
	{
		/* Don't update the panel list string if handling debug state controls */
		if(strcmp(panels[0]->titleStr, "Debug Panel") == 0)
		{
			return;
		}
		
		/* For more than one panels */
		if(panelCount > 1)
		{
			sprintf(panelsListStr, "Panels: {%s: %s, ", panels[0]->titleStr, panels[0]->isActive ? "Active" : "Inactive");
			for(i = 1; i < panelCount - 1; i++)
			{
				sprintf(tempStr, "%s: %s, ", panels[i]->titleStr, panels[i]->isActive ? "Active" : "Inactive");
				strcat(panelsListStr, tempStr);
			}
			sprintf(tempStr, "%s: %s}", panels[panelCount - 1]->titleStr, panels[panelCount - 1]->isActive ? "Active" : "Inactive");
			strcat(panelsListStr, tempStr);
		}
		else /* For single panel */
		{
			sprintf(panelsListStr, "Panels: {%s: %s}", panels[0]->titleStr, panels[0]->isActive ? "Active" : "Inactive");
		}
	}
	else /* For no panels */
	{
		sprintf(panelsListStr, "Panels: {}");
	}
}

/* Wrapper for SGE_LogPrintLine() */
static void SGE_GUI_LogPrintLine(SGE_LogLevel level, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	SGE_LogPrintLineCat(level, "GUI: ", format, &args);
	va_end(args);
}

/* Fallback handlers for GUI control events */

static void onDownFallbackCallback(void *data)
{
	SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "onDownFallback Called!");
}

static void onUpFallbackCallback(void *data)
{
	SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "onUpFallback Called!");
}

static void onSlideFallback(void *data)
{
	SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "onSlideCallback Called!");
}

static void onResizeFallback(void *data)
{
	// SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "onResizeCallback Called!");
}

static void onMoveFallback(void *data)
{
	// SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "onMoveCallback Called!");
}

static void onMinimizeFallback(void *data)
{
	SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "onMinimizeCallback Called!");
}

static void onMaximizeFallback(void *data)
{
	SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "onMaximizeCallback Called!");
}

static void onEnableFallback(void *data)
{
	SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "onEnableCallback Called!");
}

static void onDisableFallback(void *data)
{
	SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "onDisableCallback Called!");
}

static void onTextEnterFallback(void *data)
{
	SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "onTextEnterCallback Called!");
}

static void onTextDeleteFallback(void *data)
{
	SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "onTextDeleteCallback Called!");
}

static void onSelectionChangeFallback(void *data)
{
	SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "onSelectionChangeCallback Called!");
}

/* Main GUI functions called by SGE.c */

bool SGE_GUI_Init()
{
	SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Initializing SGE GUI...");

	strcpy(panelsListStr, "Panel List");
	controlBoundsColor = SGE_COLOR_CERISE;
	
	/* Open GUI Fonts */
	buttonFont = TTF_OpenFont("assets/FreeSansBold.ttf", 18);
	if(buttonFont == NULL)
	{
		SGE_GUI_LogPrintLine(SGE_LOG_ERROR, "Failed to load button font!");
		SGE_GUI_LogPrintLine(SGE_LOG_ERROR, "TTF_Error: %s", TTF_GetError());
		return false;
	}
	SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Opened Button font.");
	
	panelTitleFont = TTF_OpenFont("assets/FreeSansBold.ttf", 18);
	if(panelTitleFont == NULL)
	{
		SGE_GUI_LogPrintLine(SGE_LOG_ERROR, "Failed to load panel title font!");
		SGE_GUI_LogPrintLine(SGE_LOG_ERROR, "TTF_Error: %s", TTF_GetError());
		return false;
	}
	SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Opened Panel Title font.");
	
	labelFont = TTF_OpenFont("assets/FreeSans.ttf", 18);
	if(labelFont == NULL)
	{
		SGE_GUI_LogPrintLine(SGE_LOG_ERROR, "Failed to load default label font!");
		SGE_GUI_LogPrintLine(SGE_LOG_ERROR, "TTF_Error: %s", TTF_GetError());
		return false;
	}
	SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Opened default label font.");
	
	textBoxFont = TTF_OpenFont("assets/FreeSans.ttf", 18);
	if(textBoxFont == NULL)
	{
		SGE_GUI_LogPrintLine(SGE_LOG_ERROR, "Failed to load default textInputBox font!");
		SGE_GUI_LogPrintLine(SGE_LOG_ERROR, "TTF_Error: %s", TTF_GetError());
		return false;
	}
	SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Opened default textInputBox font.");
	
	listBoxFont = TTF_OpenFont("assets/FreeSans.ttf", 18);
	if(listBoxFont == NULL)
	{
		SGE_GUI_LogPrintLine(SGE_LOG_ERROR, "Failed to load default listBoxFont font!");
		SGE_GUI_LogPrintLine(SGE_LOG_ERROR, "TTF_Error: %s", TTF_GetError());
		return false;
	}
	SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Opened default listBoxFont font.");

	SGE_GUI_DebugState_Init();

	SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Finished Initializing SGE GUI.");
	SGE_printf(SGE_LOG_DEBUG, "\n");
	return true;
}

void SGE_GUI_Quit()
{
	SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Quitting SGE GUI...");
	
	/* Close all GUI Fonts */
	if(buttonFont != NULL)
	{
		TTF_CloseFont(buttonFont);
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Closed Button font.");
	}
	
	if(panelTitleFont != NULL)
	{
		TTF_CloseFont(panelTitleFont);
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Closed Panel title font.");
	}
	
	if(labelFont != NULL)
	{
		TTF_CloseFont(labelFont);
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Closed default label font.");
	}
	
	if(textBoxFont != NULL)
	{
		TTF_CloseFont(textBoxFont);
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Closed default textBoxFont font.");
	}
	
	if(listBoxFont != NULL)
	{
		TTF_CloseFont(listBoxFont);
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Closed default listBoxFont font.");
	}

	SGE_GUI_FreeControlList(&debugStateControls);
	
	SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Finished Quitting SGE GUI.");
	SGE_printf(SGE_LOG_DEBUG, "\n");
}

void SGE_GUI_HandleEvents()
{
	if(SGE_GetSDLEvent()->type == SDL_KEYDOWN)
	{
		/* Toggle debug state visibility */
		if(SGE_GetSDLEvent()->key.keysym.sym == SDLK_F1)
		{
			SGE_SetTextureWordWrap(500);
			SGE_TextLabelSetText(panelListLabel, panelsListStr);
			SGE_TextLabelSetText(stateListLabel, SGE_GetStateNames());
			showDebugState = !showDebugState;
		}
		
		/* Toggle debug panel visibility */
		if(showDebugState)
		{
			if(SGE_GetSDLEvent()->key.keysym.sym == SDLK_F2)
			{			
				debugPanel->isVisible = !debugPanel->isVisible;
			}
		}
	}
	
	SGE_GUI_ControlList_HandleEvents(currentStateControls);

	if(showDebugState)
	{
		SGE_GUI_ControlList *tempCurrentStateControls = currentStateControls;
		currentStateControls = &debugStateControls;
		SGE_GUI_ControlList_HandleEvents(&debugStateControls);
		currentStateControls = tempCurrentStateControls;
	}
}

void SGE_GUI_Update()
{
	SGE_GUI_ControlList_Update(currentStateControls);
	
	if(showDebugState)
	{
		SGE_GUI_ControlList *tempCurrentStateControls = currentStateControls;
		currentStateControls = &debugStateControls;
		SGE_GUI_DebugState_Update();
		SGE_GUI_ControlList_Update(&debugStateControls);
		currentStateControls = tempCurrentStateControls;
	}
}

void SGE_GUI_Render()
{
	SGE_GUI_ControlList_Render(currentStateControls);

	if(showDebugState)
	{
		SGE_GUI_ControlList *tempCurrentStateControls = currentStateControls;
		currentStateControls = &debugStateControls;
		SGE_GUI_ControlList_Render(&debugStateControls);
		currentStateControls = tempCurrentStateControls;
	}
}

/**
 * \brief Updates the control list to be used by SGE GUI functions.
 * 
 * This function is automatically called by SGE_SwitchStates() after a state switch is triggered
 * so the new state's SGE_GUI_ControlList is used by GUI functions.
 * It also resets the state of all the controls in the new state since a state switch might have
 * left the controls in an altered state. (e.g if a button is used to trigger a state switch)
 * 
 * \param nextState The name of the state that is being switched to.
 */
void SGE_GUI_UpdateCurrentState(const char *nextState)
{
	int i;
	currentStateControls = SGE_GetStateGUIControlList(nextState);
	if(currentStateControls == NULL)
	{
		SGE_LogPrintLine(SGE_LOG_WARNING, "Attempted switch to NULL GUI Control List!");
		return;
	}

	printPanelsStr();
	
	/* Reset control states */
	for(i = 0; i < currentStateControls->buttonCount; i++)
	{
		currentStateControls->buttons[i]->state = SGE_CONTROL_STATE_NORMAL;
	}

	for(i = 0; i < currentStateControls->checkBoxCount; i++)
	{
		currentStateControls->checkBoxes[i]->state = SGE_CONTROL_STATE_NORMAL;
	}

	for(i = 0; i < currentStateControls->sliderCount; i++)
	{
		currentStateControls->sliders[i]->state = SGE_CONTROL_STATE_NORMAL;
	}
}

static void SGE_GUI_ControlList_HandleEvents(SGE_GUI_ControlList *controls)
{
	int i = 0;

	SGE_WindowPanel **panels = controls->panels;
	int panelCount = controls->panelCount;
	
	/* Handle Events for parentless controls */
	for(i = 0; i < controls->buttonCount; i++)
	{
		SGE_ButtonHandleEvents(controls->buttons[i]);
	}
	
	for(i = 0; i < controls->checkBoxCount; i++)
	{
		SGE_CheckBoxHandleEvents(controls->checkBoxes[i]);
	}
	
	for(i = 0; i < controls->sliderCount; i++)
	{
		SGE_SliderHandleEvents(controls->sliders[i]);
	}
	
	for(i = 0; i < controls->textInputBoxCount; i++)
	{
		SGE_TextInputBoxHandleEvents(controls->textInputBoxes[i]);
	}
	
	for(i = 0; i < controls->listBoxCount; i++)
	{
		SGE_ListBoxHandleEvents(controls->listBoxes[i]);
	}
	
	/* Handle Events for all the panels themselves */
	for(i = 0; i < panelCount; i++)
	{
		if(panels[i]->isVisible)
			SGE_WindowPanelHandleEvents(panels[i]);
	}
	
	/* Handle Events for only the active panel's controls */
	if(panelCount != 0)
	{
		if(panels[panelCount - 1]->isVisible)
		{
			for(i = 0; i < panels[panelCount - 1]->buttonCount; i++)
			{
				SGE_ButtonHandleEvents(panels[panelCount - 1]->buttons[i]);
			}
			
			for(i = 0; i < panels[panelCount - 1]->checkBoxCount; i++)
			{
				SGE_CheckBoxHandleEvents(panels[panelCount - 1]->checkBoxes[i]);
			}
			
			for(i = 0; i < panels[panelCount - 1]->sliderCount; i++)
			{
				SGE_SliderHandleEvents(panels[panelCount - 1]->sliders[i]);
			}
			
			for(i = 0; i < panels[panelCount - 1]->textInputBoxCount; i++)
			{
				SGE_TextInputBoxHandleEvents(panels[panelCount - 1]->textInputBoxes[i]);
			}
			
			for(i = 0; i < panels[panelCount - 1]->listBoxCount; i++)
			{
				SGE_ListBoxHandleEvents(panels[panelCount - 1]->listBoxes[i]);
			}
		}
	}
}

static void SGE_GUI_ControlList_Update(SGE_GUI_ControlList *controls)
{
	int i = 0;
	int j = 0;

	SGE_WindowPanel **panels = controls->panels;
	int panelCount = controls->panelCount;

	panels = controls->panels;
	panelCount = controls->panelCount;
	
	/* Update the parentless controls */
	for(i = 0; i < controls->buttonCount; i++)
	{
		SGE_ButtonUpdate(controls->buttons[i]);
	}
	
	for(i = 0; i < controls->checkBoxCount; i++)
	{
		SGE_CheckBoxUpdate(controls->checkBoxes[i]);
	}
	
	for(i = 0; i < controls->sliderCount; i++)
	{
		SGE_SliderUpdate(controls->sliders[i]);
	}
	
	for(i = 0; i < controls->textInputBoxCount; i++)
	{
		SGE_TextInputBoxUpdate(controls->textInputBoxes[i]);
	}
	
	for(i = 0; i < controls->listBoxCount; i++)
	{
		SGE_ListBoxUpdate(controls->listBoxes[i]);
	}
	
	if(panelCount != 0)
	{
		for(i = 0; i < panelCount; i++)
		{
			if(panels[i]->isVisible)
			{
				SGE_WindowPanelUpdate(panels[i]);
				
				/* Update all panel's controls */
				for(j = 0; j < panels[i]->buttonCount; j++)
				{
					SGE_ButtonUpdate(panels[i]->buttons[j]);
				}
				
				for(j = 0; j < panels[i]->checkBoxCount; j++)
				{
					SGE_CheckBoxUpdate(panels[i]->checkBoxes[j]);
				}
				
				for(j = 0; j < panels[i]->sliderCount; j++)
				{
					SGE_SliderUpdate(panels[i]->sliders[j]);
				}
				
				for(j = 0; j < panels[i]->textInputBoxCount; j++)
				{
					SGE_TextInputBoxUpdate(panels[i]->textInputBoxes[j]);
				}
				
				for(j = 0; j < panels[i]->listBoxCount; j++)
				{
					SGE_ListBoxUpdate(panels[i]->listBoxes[j]);
				}
			}
		}
	}
}

static void SGE_GUI_ControlList_Render(SGE_GUI_ControlList *controls)
{
	int i = 0;
	
	/* Render all the panels and their child controls */
	for(i = 0; i < controls->panelCount; i++)
	{
		if(controls->panels[i]->isVisible)
		{
			SGE_WindowPanelRender(controls->panels[i]);
		}
	}
	
	/* Render the parentless controls */
	for(i = 0; i < controls->buttonCount; i++)
	{
		SGE_ButtonRender(controls->buttons[i]);
	}
	
	for(i = 0; i < controls->checkBoxCount; i++)
	{
		SGE_CheckBoxRender(controls->checkBoxes[i]);
	}
	
	for(i = 0; i < controls->labelCount; i++)
	{
		SGE_TextLabelRender(controls->labels[i]);
	}
	
	for(i = 0; i < controls->sliderCount; i++)
	{
		SGE_SliderRender(controls->sliders[i]);
	}
	
	for(i = 0; i < controls->textInputBoxCount; i++)
	{
		SGE_TextInputBoxRender(controls->textInputBoxes[i]);
	}
	
	for(i = 0; i < controls->listBoxCount; i++)
	{
		SGE_ListBoxRender(controls->listBoxes[i]);
	}
}

static void SGE_GUI_FreeControlList(SGE_GUI_ControlList *controls)
{
	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;
	int m = 0;
	int n = 0; // Some of these are unnecessary \/(-_-)\/

	for(i = 0; i < controls->buttonCount; i++)
	{
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Destroyed Control: {NULL}-><Button> %d", i + 1);
		SGE_DestroyButton(controls->buttons[i]);
		controls->buttons[i] = NULL;
	}
	controls->buttonCount = 0;

	for(i = 0; i < controls->checkBoxCount; i++)
	{
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Destroyed Control: {NULL}-><Checkbox> %d", i + 1);
		SGE_DestroyCheckBox(controls->checkBoxes[i]);
		controls->checkBoxes[i] = NULL;
	}
	controls->checkBoxCount = 0;

	for(i = 0; i < controls->labelCount; i++)
	{
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Destroyed Control: {NULL}-><TextLabel> %d", i + 1);
		SGE_DestroyTextLabel(controls->labels[i]);
		controls->labels[i] = NULL;
	}
	controls->labelCount = 0;

	for(i = 0; i < controls->sliderCount; i++)
	{
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Destroyed Control: {NULL}-><Slider> %d", i + 1);
		SGE_DestroySlider(controls->sliders[i]);
		controls->sliders[i] = NULL;
	}
	controls->sliderCount = 0;

	for(i = 0; i < controls->textInputBoxCount; i++)
	{
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Destroyed Control: {NULL}-><TextInputBox> %d", i + 1);
		SGE_DestroyTextInputBox(controls->textInputBoxes[i]);
		controls->textInputBoxes[i] = NULL;
	}
	controls->textInputBoxCount = 0;

	for(i = 0; i < controls->listBoxCount; i++)
	{
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Destroyed Control: {NULL}-><ListBox> %d", i + 1);
		SGE_DestroyListBox(controls->listBoxes[i]);
		controls->listBoxes[i] = NULL;
	}
	controls->listBoxCount = 0;

	/* Go through each panel in the panels stack and destroy it's child controls and then the panel itself */
	for(i = 0; i < controls->panelCount; i++)
	{
		for(j = 0; j < controls->panels[i]->buttonCount; j++)
		{
			SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Destroyed Control: %s->Button %d", controls->panels[i]->titleStr, j + 1);
			SGE_DestroyButton(controls->panels[i]->buttons[j]);
		}
		for(k = 0; k < controls->panels[i]->checkBoxCount; k++)
		{
			SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Destroyed Control: %s->CheckBox %d", controls->panels[i]->titleStr, k + 1);
			SGE_DestroyCheckBox(controls->panels[i]->checkBoxes[k]);
		}
		for(l = 0; l < controls->panels[i]->textLabelCount; l++)
		{
			SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Destroyed Control: %s->Label %d", controls->panels[i]->titleStr, l + 1);
			SGE_DestroyTextLabel(controls->panels[i]->textLabels[l]);
		}
		for(m = 0; m < controls->panels[i]->sliderCount; m++)
		{
			SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Destroyed Control: %s->Slider %d", controls->panels[i]->titleStr, m + 1);
			SGE_DestroySlider(controls->panels[i]->sliders[m]);
		}
		for(n = 0; n < controls->panels[i]->textInputBoxCount; n++)
		{
			SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Destroyed Control: %s->TextInputBox %d", controls->panels[i]->titleStr, n + 1);
			SGE_DestroyTextInputBox(controls->panels[i]->textInputBoxes[n]);
		}
		for(n = 0; n < controls->panels[i]->listBoxCount; n++)
		{
			SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Destroyed Control: %s->ListBox %d", controls->panels[i]->titleStr, n + 1);
			SGE_DestroyListBox(controls->panels[i]->listBoxes[n]);
		}
		
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Destroyed Panel: %s", controls->panels[i]->titleStr);
		SGE_DestroyWindowPanel(controls->panels[i]);
		controls->panels[i] = NULL;
	}
	controls->panelCount = 0;
}

/* GUI Control Functions */

SGE_Button *SGE_CreateButton(const char *text, int x, int y, struct SGE_WindowPanel *panel)
{
	SGE_Button *button = NULL;
	button = (SGE_Button*)malloc(sizeof(SGE_Button));
	
	if(panel != NULL)
	{
		if(panel->buttonCount == PANEL_MAX_BUTTONS)
		{
			SGE_GUI_LogPrintLine(SGE_LOG_WARNING, "Failed to add Button! Max amount of buttons [%d] in panel %s reached.", PANEL_MAX_BUTTONS, panel->titleStr);
			free(button);
			return NULL;
		}
		/* Add this new button to the top of the parent panel's buttons list */
		panel->buttons[panel->buttonCount] = button;
		panel->buttonCount += 1;
		panel->controlCount += 1;
		button->parentPanel = panel;
		button->alpha = button->parentPanel->alpha;
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Added Control: %s->Button %d", panel->titleStr, panel->buttonCount);
	}
	else
	{
		if(currentStateControls->buttonCount == STATE_MAX_BUTTONS)
		{
			SGE_GUI_LogPrintLine(SGE_LOG_WARNING, "Failed to add Button! Max amount of parentless buttons [%d] reached.", STATE_MAX_BUTTONS);
			free(button);
			return NULL;
		}
		/* Add this new button to the top of the parentless buttons list */
		currentStateControls->buttons[currentStateControls->buttonCount] = button;
		currentStateControls->buttonCount += 1;
		button->parentPanel = NULL;
		button->alpha = 255;
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Added Control: {NULL}->Button %d", currentStateControls->buttonCount);
	}
	
	button->x = x;
	button->y = y;
	
	button->state = SGE_CONTROL_STATE_NORMAL;
	
	button->normalColor = SGE_COLOR_DARK_RED;
	button->hoverColor = SGE_COLOR_GRAY;
	button->clickedColor = SGE_COLOR_LIGHT_GRAY;
	button->currentColor = button->normalColor;
	
	button->textImg = SGE_CreateTextureFromText(text, buttonFont, SGE_COLOR_WHITE, SGE_TEXT_MODE_BLENDED);
	if(buttonFont == NULL)
	{
		SGE_GUI_LogPrintLine(SGE_LOG_ERROR, "Failed to create button text image!");
		TTF_CloseFont(buttonFont);
		free(button);
		return NULL;
	}
	
	/* Calculate the bounding box based on parent panel position */
	if(button->parentPanel != NULL)
	{
		button->boundBox.x = button->x + button->parentPanel->background.x + button->parentPanel->x_scroll_offset;
		button->boundBox.y = button->y + button->parentPanel->background.y + button->parentPanel->y_scroll_offset;
	}
	else
	{
		button->boundBox.x = button->x;
		button->boundBox.y = button->y;
	}
	
	/* Calculate the position and size of the button */
	button->background.x = button->boundBox.x;
	button->background.y = button->boundBox.y;
	button->background.w = button->textImg->w + 20;
	button->background.h = button->textImg->h + 20;
	button->textImg->x = button->background.x + (button->background.w / 2) - button->textImg->w / 2;
	button->textImg->y = button->background.y + (button->background.h / 2) - button->textImg->h / 2;
	
	button->boundBox.w = button->background.w;
	button->boundBox.h = button->background.h;
	
	/* Set button callback functions to fallback */
	button->onMouseDown = onDownFallbackCallback;
	button->onMouseDown_data = NULL;
	button->onMouseUp = onUpFallbackCallback;
	button->onMouseUp_data = NULL;
	
	/* Recalculate the parent panel's MCR */
	if(panel != NULL)
	{
		SGE_WindowPanelCalculateMCR(panel, button->boundBox);
	}
	
	return button;
}

void SGE_DestroyButton(SGE_Button *button)
{
	if(button != NULL)
	{
		SGE_FreeTexture(button->textImg);
		free(button);
	}
}

void SGE_ButtonHandleEvents(SGE_Button *button)
{
	if(SGE_GetSDLEvent()->type == SDL_MOUSEBUTTONDOWN)
	{
		if(SGE_GetSDLEvent()->button.button == 1)
		{
			if(SGE_MouseInRect(&button->boundBox))
			{
				if(button->parentPanel != NULL)
				{
					if(SGE_MouseInRect(&button->parentPanel->background) && !SGE_MouseInRect(&button->parentPanel->horizontalScrollbarBG) && !SGE_MouseInRect(&button->parentPanel->verticalScrollbarBG))
					{
						button->state = SGE_CONTROL_STATE_CLICKED;
						button->onMouseDown(button->onMouseDown_data);
					}
				}
				else
				{
					button->state = SGE_CONTROL_STATE_CLICKED;
					button->onMouseDown(button->onMouseDown_data);
				}
			}
		}
	}
	else if(SGE_GetSDLEvent()->type == SDL_MOUSEBUTTONUP)
	{
		if(SGE_GetSDLEvent()->button.button == 1)
		{
			if(button->state == SGE_CONTROL_STATE_CLICKED)
			{
				if(SGE_MouseInRect(&button->boundBox))
				{
					if(button->parentPanel != NULL)
					{
						if(SGE_MouseInRect(&button->parentPanel->background) && !SGE_MouseInRect(&button->parentPanel->horizontalScrollbarBG)&& !SGE_MouseInRect(&button->parentPanel->verticalScrollbarBG))
						{
							button->state = SGE_CONTROL_STATE_HOVER;
							button->onMouseUp(button->onMouseUp_data);
						}
					}
					else
					{
						button->state = SGE_CONTROL_STATE_HOVER;
						button->onMouseUp(button->onMouseUp_data);
					}
				}
				else
				{
					button->state = SGE_CONTROL_STATE_NORMAL;
				}
			}
		}
	}
	else if(SGE_GetSDLEvent()->type == SDL_MOUSEMOTION)
	{
		if(button->state != SGE_CONTROL_STATE_CLICKED)
		{
			if(SGE_MouseInRect(&button->boundBox))
			{
				if(button->parentPanel != NULL)
				{
					if(SGE_MouseInRect(&button->parentPanel->background) && !SGE_MouseInRect(&button->parentPanel->horizontalScrollbarBG) && !SGE_MouseInRect(&button->parentPanel->verticalScrollbarBG))
						button->state = SGE_CONTROL_STATE_HOVER;
					else
						button->state = SGE_CONTROL_STATE_NORMAL;
				}
				else
					button->state = SGE_CONTROL_STATE_HOVER;
			}
			else
				button->state = SGE_CONTROL_STATE_NORMAL;
		}
	}
}

void SGE_ButtonUpdate(SGE_Button *button)
{
	if(button->state == SGE_CONTROL_STATE_NORMAL)
		button->currentColor = button->normalColor;
	else if(button->state == SGE_CONTROL_STATE_HOVER)
		button->currentColor = button->hoverColor;
	else if(button->state == SGE_CONTROL_STATE_CLICKED)
		button->currentColor = button->clickedColor;
	
	if(button->parentPanel != NULL)
	{
		button->boundBox.x = button->x + button->parentPanel->background.x + button->parentPanel->x_scroll_offset;
		button->boundBox.y = button->y + button->parentPanel->background.y + button->parentPanel->y_scroll_offset;
		button->background.x = button->boundBox.x;
		button->background.y = button->boundBox.y;
		button->textImg->x = button->background.x + (button->background.w / 2) - button->textImg->w / 2;
		button->textImg->y = button->background.y + (button->background.h / 2) - button->textImg->h / 2;
		button->alpha = button->parentPanel->alpha;
	}
}

void SGE_ButtonRender(SGE_Button *button)
{
	int i = 0;
	
	/* Draw filled button background */
	SGE_SetDrawColorRGBA(button->currentColor.r, button->currentColor.g, button->currentColor.b, button->alpha);
	SGE_DrawFillRect(&button->background);
	
	/* Draw button border */
	SGE_SetDrawColorRGBA(0, 0, 0, button->alpha);
	if(SGE_MouseInRect(&button->boundBox))
	{
		if(button->parentPanel != NULL)
		{
			if(SGE_MouseInRect(&button->parentPanel->background) && !SGE_MouseInRect(&button->parentPanel->horizontalScrollbarBG) && !SGE_MouseInRect(&button->parentPanel->verticalScrollbarBG))
				SGE_SetDrawColorRGBA(225, 225, 225, button->alpha);
			
			for(i = button->parentPanel->index + 1; i < currentStateControls->panelCount; i++)
			{
				if(SGE_MouseInRect(&currentStateControls->panels[i]->border))
					SGE_SetDrawColorRGBA(0, 0, 0, button->alpha);
			}
		}
		else
			SGE_SetDrawColorRGBA(225, 225, 225, button->alpha);
	}
	SGE_DrawRect(&button->background);
	
	/* Draw button text image */
	SGE_SetTextureAlpha(button->textImg, button->alpha);
	SGE_RenderTexture(button->textImg);
	
	if(showControlBounds)
	{
		SGE_SetDrawColorRGBA(controlBoundsColor.r, controlBoundsColor.g, controlBoundsColor.b, button->alpha);
		SGE_DrawRect(&button->boundBox);
	}
}

void SGE_ButtonSetPosition(SGE_Button *button, int x, int y)
{
	button->x = x;
	button->y = y;
	
	/* Calculate the bounding box based on parent panel position */
	if(button->parentPanel != NULL)
	{
		button->boundBox.x = button->x + button->parentPanel->background.x + button->parentPanel->x_scroll_offset;
		button->boundBox.y = button->y + button->parentPanel->background.y + button->parentPanel->y_scroll_offset;
	}
	else
	{
		button->boundBox.x = button->x;
		button->boundBox.y = button->y;
	}
	
	button->background.x = button->boundBox.x;
	button->background.y = button->boundBox.y;
	button->textImg->x = button->background.x + (button->background.w / 2) - button->textImg->w / 2;
	button->textImg->y = button->background.y + (button->background.h / 2) - button->textImg->h / 2;
	
	/* Recalculate the parent panel's MCR */
	if(button->parentPanel != NULL)
	{
		SGE_WindowPanelCalculateMCR(button->parentPanel, button->boundBox);
	}
}

SGE_CheckBox *SGE_CreateCheckBox(int x, int y, struct SGE_WindowPanel *panel)
{
	SGE_CheckBox *checkBox = NULL;
	checkBox = (SGE_CheckBox *)malloc(sizeof(SGE_CheckBox));
	
	if(panel != NULL)
	{
		if(panel->checkBoxCount == PANEL_MAX_CHECKBOXES)
		{
			SGE_GUI_LogPrintLine(SGE_LOG_WARNING, "Failed to add CheckBox! Max amount of checkboxes [%d] in panel %s reached.", PANEL_MAX_CHECKBOXES, panel->titleStr);
			free(checkBox);
			return NULL;
		}
		/* Add this new checkbox to the top of the parent panel's checkboxes list */
		panel->checkBoxes[panel->checkBoxCount] = checkBox;
		panel->checkBoxCount += 1;
		panel->controlCount += 1;
		checkBox->parentPanel = panel;
		checkBox->alpha = checkBox->parentPanel->alpha;
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Added Control: %s->CheckBox %d", panel->titleStr, panel->checkBoxCount);
	}
	else
	{
		if(currentStateControls->checkBoxCount == STATE_MAX_CHECKBOXES)
		{
			SGE_GUI_LogPrintLine(SGE_LOG_WARNING, "Failed to add CheckBox! Max amount of parentless checkboxes [%d] reached.", STATE_MAX_CHECKBOXES);
			free(checkBox);
			return NULL;
		}
		/* Add this new checkbox to the top of the parentless checkboxes list */
		currentStateControls->checkBoxes[currentStateControls->checkBoxCount] = checkBox;
		currentStateControls->checkBoxCount += 1;
		checkBox->parentPanel = NULL;
		checkBox->alpha = 255;
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Added Control: {NULL}->CheckBox %d", currentStateControls->checkBoxCount);
	}
	
	checkBox->x = x;
	checkBox->y = y;
	checkBox->size = 30;
	
	if(checkBox->parentPanel != NULL)
	{
		checkBox->boundBox.x = checkBox->x + checkBox->parentPanel->background.x + checkBox->parentPanel->x_scroll_offset;
		checkBox->boundBox.y = checkBox->y + checkBox->parentPanel->background.y + checkBox->parentPanel->y_scroll_offset;
	}
	else
	{
		checkBox->boundBox.x = checkBox->x;
		checkBox->boundBox.y = checkBox->y;
	}
	
	/* Calculate checkbox location and size */
	checkBox->bg.x = checkBox->boundBox.x;
	checkBox->bg.y = checkBox->boundBox.y;
	checkBox->bg.w = checkBox->size;
	checkBox->bg.h = checkBox->size;
	
	checkBox->boundBox.w = checkBox->bg.w;
	checkBox->boundBox.h = checkBox->bg.h;
	
	checkBox->check.w = checkBox->size - 10;
	checkBox->check.h = checkBox->size - 10;
	checkBox->check.x = checkBox->bg.x + (checkBox->bg.w / 2) - (checkBox->check.w / 2);
	checkBox->check.y = checkBox->bg.y + (checkBox->bg.h / 2) - (checkBox->check.h / 2);
	
	checkBox->checkColor = SGE_COLOR_DARK_RED;
	checkBox->isChecked = false;
	
	checkBox->state = SGE_CONTROL_STATE_NORMAL;
	checkBox->onMouseDown = onDownFallbackCallback;
	checkBox->onMouseDown_data = NULL;
	checkBox->onMouseUp = onUpFallbackCallback;
	checkBox->onMouseUp_data = NULL;
	
	/* Recalculate the parent panel's MCR */
	if(panel != NULL)
	{
		SGE_WindowPanelCalculateMCR(panel, checkBox->boundBox);
	}
	
	return checkBox;
}

void SGE_DestroyCheckBox(SGE_CheckBox *checkBox)
{
	if(checkBox != NULL)
	{
		free(checkBox);
	}
}

void SGE_CheckBoxHandleEvents(SGE_CheckBox *checkBox)
{
	if(SGE_GetSDLEvent()->type == SDL_MOUSEBUTTONDOWN)
	{
		if(SGE_GetSDLEvent()->button.button == 1)
		{
			if(SGE_MouseInRect(&checkBox->boundBox))
			{
				if(checkBox->parentPanel != NULL)
				{
					if(SGE_MouseInRect(&checkBox->parentPanel->background) && !SGE_MouseInRect(&checkBox->parentPanel->horizontalScrollbarBG) && !SGE_MouseInRect(&checkBox->parentPanel->verticalScrollbarBG))
					{
						checkBox->state = SGE_CONTROL_STATE_CLICKED;
						checkBox->onMouseDown(checkBox->onMouseDown_data);
					}
				}
				else
				{
					checkBox->state = SGE_CONTROL_STATE_CLICKED;
					checkBox->onMouseDown(checkBox->onMouseDown_data);
				}
			}
		}
	}
	else if(SGE_GetSDLEvent()->type == SDL_MOUSEBUTTONUP)
	{
		if(SGE_GetSDLEvent()->button.button == 1)
		{
			if(checkBox->state == SGE_CONTROL_STATE_CLICKED)
			{
				if(SGE_MouseInRect(&checkBox->boundBox))
				{
					checkBox->isChecked = !checkBox->isChecked;
					checkBox->state = SGE_CONTROL_STATE_HOVER;
					checkBox->onMouseUp(checkBox->onMouseUp_data);
				}
				else
				{
					checkBox->state = SGE_CONTROL_STATE_NORMAL;
				}
			}
		}
	}
	else if(SGE_GetSDLEvent()->type == SDL_MOUSEMOTION)
	{
		if(checkBox->state != SGE_CONTROL_STATE_CLICKED)
		{
			if(SGE_MouseInRect(&checkBox->boundBox))
			{
				if(checkBox->parentPanel != NULL)
				{
					if(SGE_MouseInRect(&checkBox->parentPanel->background) && !SGE_MouseInRect(&checkBox->parentPanel->horizontalScrollbarBG) && !SGE_MouseInRect(&checkBox->parentPanel->verticalScrollbarBG))
						checkBox->state = SGE_CONTROL_STATE_HOVER;
					else
						checkBox->state = SGE_CONTROL_STATE_NORMAL;
				}
				else
					checkBox->state = SGE_CONTROL_STATE_HOVER;
			}
			else
				checkBox->state = SGE_CONTROL_STATE_NORMAL;
		}
	}
}

void SGE_CheckBoxUpdate(SGE_CheckBox *checkBox)
{
	/* Recalculate checkbox location based on the parent panel */
	if(checkBox->parentPanel != NULL)
	{
		checkBox->boundBox.x = checkBox->x + checkBox->parentPanel->background.x + checkBox->parentPanel->x_scroll_offset;
		checkBox->boundBox.y = checkBox->y + checkBox->parentPanel->background.y + checkBox->parentPanel->y_scroll_offset;
		checkBox->bg.x = checkBox->boundBox.x;
		checkBox->bg.y = checkBox->boundBox.y;
		checkBox->check.x = checkBox->bg.x + (checkBox->bg.w / 2) - (checkBox->check.w / 2);
		checkBox->check.y = checkBox->bg.y + (checkBox->bg.h / 2) - (checkBox->check.h / 2);
		checkBox->alpha = checkBox->parentPanel->alpha;
	}
}

void SGE_CheckBoxRender(SGE_CheckBox *checkBox)
{
	int i = 0;
	
	/* Draw white checkbox filled background */
	SGE_SetDrawColorRGBA(255, 255, 255, checkBox->alpha);
	SGE_DrawFillRect(&checkBox->bg);
	
	/* Draw gray checkbox border */
	SGE_SetDrawColorRGBA(0, 0, 0, checkBox->alpha);
	if(SGE_MouseInRect(&checkBox->boundBox))
	{
		if(checkBox->parentPanel != NULL)
		{
			if(SGE_MouseInRect(&checkBox->parentPanel->background) && !SGE_MouseInRect(&checkBox->parentPanel->horizontalScrollbarBG) && !SGE_MouseInRect(&checkBox->parentPanel->verticalScrollbarBG))
				SGE_SetDrawColorRGBA(150, 150, 150, checkBox->alpha);
			
			for(i = checkBox->parentPanel->index + 1; i < currentStateControls->panelCount; i++)
			{
				if(SGE_MouseInRect(&currentStateControls->panels[i]->border))
					SGE_SetDrawColorRGBA(0, 0, 0, checkBox->alpha);
			}
		}
		else
			SGE_SetDrawColorRGBA(150, 150, 150, checkBox->alpha);
	}
	SGE_DrawRect(&checkBox->bg);
	
	/* Draw the check inside the background */
	if(checkBox->isChecked == true)
	{
		SGE_SetDrawColorRGBA(checkBox->checkColor.r, checkBox->checkColor.g, checkBox->checkColor.b, checkBox->alpha);
		SGE_DrawFillRect(&checkBox->check);
	}
	
	if(showControlBounds)
	{
		SGE_SetDrawColorRGBA(controlBoundsColor.r, controlBoundsColor.g, controlBoundsColor.b, checkBox->alpha);
		SGE_DrawRect(&checkBox->boundBox);
	}
}

void SGE_CheckBoxSetPosition(SGE_CheckBox *checkBox, int x, int y)
{
	checkBox->x = x;
	checkBox->y = y;
	
	/* Calculate the bounding box based on parent panel position */
	if(checkBox->parentPanel != NULL)
	{
		checkBox->boundBox.x = checkBox->x + checkBox->parentPanel->background.x + checkBox->parentPanel->x_scroll_offset;
		checkBox->boundBox.y = checkBox->y + checkBox->parentPanel->background.y + checkBox->parentPanel->y_scroll_offset;
	}
	else
	{
		checkBox->boundBox.x = checkBox->x;
		checkBox->boundBox.y = checkBox->y;
	}
	
	checkBox->bg.x = checkBox->boundBox.x;
	checkBox->bg.y = checkBox->boundBox.y;
	checkBox->check.x = checkBox->bg.x + (checkBox->bg.w / 2) - (checkBox->check.w / 2);
	checkBox->check.y = checkBox->bg.y + (checkBox->bg.h / 2) - (checkBox->check.h / 2);
	
	/* Recalculate the parent panel's MCR */
	if(checkBox->parentPanel != NULL)
	{
		SGE_WindowPanelCalculateMCR(checkBox->parentPanel, checkBox->boundBox);
	}
}

SGE_TextLabel *SGE_CreateTextLabelCustom(const char *text, int x, int y, SDL_Color color, TTF_Font *font, struct SGE_WindowPanel *panel)
{
	SGE_TextLabel *label = NULL;
	label = (SGE_TextLabel *)malloc(sizeof(SGE_TextLabel));
	
	if(panel != NULL)
	{
		if(panel->textLabelCount == PANEL_MAX_LABELS)
		{
			SGE_GUI_LogPrintLine(SGE_LOG_WARNING, "Failed to add Label! Max amount of labels [%d] in panel %s reached.", PANEL_MAX_LABELS, panel->titleStr);
			free(label);
			return NULL;
		}
		panel->textLabels[panel->textLabelCount] = label;
		panel->textLabelCount += 1;
		panel->controlCount += 1;
		label->parentPanel = panel;
		label->alpha = label->parentPanel->alpha;
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Added Control: %s->Label %d", panel->titleStr, panel->textLabelCount);
	}
	else
	{
		if(currentStateControls->labelCount == STATE_MAX_LABELS)
		{
			SGE_GUI_LogPrintLine(SGE_LOG_WARNING, "Failed to add Label! Max amount of parentless labels [%d] reached.", STATE_MAX_LABELS);
			free(label);
			return NULL;
		}
		currentStateControls->labels[currentStateControls->labelCount] = label;
		currentStateControls->labelCount += 1;
		label->parentPanel = NULL;
		label->alpha = 255;
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Added Control: {NULL}->Label %d", currentStateControls->labelCount);
	}
	
	label->x = x;
	label->y = y;
	
	strncpy(label->text, text, 200);
	label->font = font;
	label->fgColor = color;
	label->bgColor = SGE_COLOR_GRAY;
	label->showBG = false;
	label->mode = SGE_TEXT_MODE_BLENDED;
	label->isVisible = true;
	//SGE_SetTextureWordWrap();
	label->textImg = SGE_CreateTextureFromText(text, font, label->fgColor, label->mode);
	
	if(label->parentPanel != NULL)
	{
		label->boundBox.x = label->x + label->parentPanel->background.x + label->parentPanel->x_scroll_offset;
		label->boundBox.y = label->y + label->parentPanel->background.y + label->parentPanel->y_scroll_offset;
	}
	else
	{
		label->boundBox.x = label->x;
		label->boundBox.y = label->y;
	}
	
	label->textImg->x = label->boundBox.x;
	label->textImg->y = label->boundBox.y;
	label->boundBox.w = label->textImg->w;
	label->boundBox.h = label->textImg->h;
	
	/* Recalculate the parent panel's MCR */
	if(panel != NULL)
	{
		SGE_WindowPanelCalculateMCR(panel, label->boundBox);
	}
	
	return label;
}

SGE_TextLabel *SGE_CreateTextLabel(const char *text, int x, int y, SDL_Color color, struct SGE_WindowPanel *panel)
{
	return SGE_CreateTextLabelCustom(text, x, y, color, labelFont, panel);
}

SGE_TextLabel *SGE_CreateTextLabelf(int x, int y, SDL_Color color, struct SGE_WindowPanel *panel, const char *format, ...)
{
    static char str[200];

    va_list args;
	va_start(args, format);
    vsprintf(str, format, args);
    SGE_TextLabel *label = SGE_CreateTextLabel(str, x, y, color, panel);
    va_end(args);

    return label;
}

void SGE_DestroyTextLabel(SGE_TextLabel *label)
{
	if(label != NULL)
	{
		SGE_FreeTexture(label->textImg);
		free(label);
	}
}

void SGE_TextLabelRender(SGE_TextLabel *label)
{
	if(!label->isVisible)
	{
		return;
	}

	if(label->parentPanel != NULL)
	{
		label->boundBox.x = label->x + label->parentPanel->background.x + label->parentPanel->x_scroll_offset;
		label->boundBox.y = label->y + label->parentPanel->background.y + label->parentPanel->y_scroll_offset;
		label->textImg->x = label->boundBox.x;
		label->textImg->y = label->boundBox.y;
		label->alpha = label->parentPanel->alpha;
		label->bgColor.a = label->alpha;
		SGE_SetTextureAlpha(label->textImg, label->alpha);
	}
	
	if(label->showBG)
	{
		SGE_SetDrawColorRGBA(label->bgColor.r, label->bgColor.g, label->bgColor.b, label->bgColor.a);
		SGE_DrawFillRect(&label->boundBox);
	}
	
	SGE_RenderTexture(label->textImg);
	
	if(showControlBounds)
	{
		SGE_SetDrawColorRGBA(controlBoundsColor.r, controlBoundsColor.g, controlBoundsColor.b, label->alpha);
		SGE_DrawRect(&label->boundBox);
	}
}

void SGE_TextLabelSetPosition(SGE_TextLabel *label, int x, int y)
{
	label->x = x;
	label->y = y;
	
	/* Calculate the bounding box based on parent panel position */
	if(label->parentPanel != NULL)
	{
		label->boundBox.x = label->x + label->parentPanel->background.x + label->parentPanel->x_scroll_offset;
		label->boundBox.y = label->y + label->parentPanel->background.y + label->parentPanel->y_scroll_offset;
	}
	else
	{
		label->boundBox.x = label->x;
		label->boundBox.y = label->y;
	}
	
	label->textImg->x = label->boundBox.x;
	label->textImg->y = label->boundBox.y;
	
	/* Recalculate the parent panel's MCR */
	if(label->parentPanel != NULL)
	{
		SGE_WindowPanelCalculateMCR(label->parentPanel, label->boundBox);
	}
}

void SGE_TextLabelSetText(SGE_TextLabel *label, const char *text)
{
	strncpy(label->text, text, 200);
	SGE_UpdateTextureFromText(label->textImg, label->text, label->font, label->fgColor, label->mode);
	label->boundBox.w = label->textImg->w;
	label->boundBox.h = label->textImg->h;
	
	if(label->parentPanel != NULL)
	{
		SGE_WindowPanelCalculateMCR(label->parentPanel, label->boundBox);
	}
}

void SGE_TextLabelSetTextf(SGE_TextLabel *label, const char *format, ...)
{
    va_list args;
	va_start(args, format);

    vsnprintf(label->text, 200, format, args);
	SGE_UpdateTextureFromText(label->textImg, label->text, label->font, label->fgColor, label->mode);
	label->boundBox.w = label->textImg->w;
	label->boundBox.h = label->textImg->h;
	
	if(label->parentPanel != NULL)
	{
		SGE_WindowPanelCalculateMCR(label->parentPanel, label->boundBox);
	}

	va_end(args);
}

void SGE_TextLabelSetFGColor(SGE_TextLabel *label, SDL_Color fg)
{
	label->fgColor = fg;
	SGE_UpdateTextureFromText(label->textImg, label->text, labelFont, label->fgColor, label->mode);
}

void SGE_TextLabelSetBGColor(SGE_TextLabel *label, SDL_Color bg)
{
	label->bgColor = bg;
	if(!label->showBG)
	{
		label->showBG = true;
	}
}

void SGE_TextLabelSetMode(SGE_TextLabel *label, SGE_TextRenderMode mode)
{
	label->mode = mode;
	SGE_UpdateTextureFromText(label->textImg, label->text, labelFont, label->fgColor, label->mode);
}

void SGE_TextLabelSetAlpha(SGE_TextLabel *label, Uint8 alpha)
{
	SGE_SetTextureAlpha(label->textImg, alpha);
}

void SGE_TextLabelSetVisible(SGE_TextLabel *label, bool visible)
{
	label->isVisible = visible;
}

SGE_Slider *SGE_CreateSlider(int x, int y, struct SGE_WindowPanel *panel)
{
	SGE_Slider *slider = NULL;
	slider = (SGE_Slider *)malloc(sizeof(SGE_Slider));
	
	slider->x = x;
	slider->y = y;
	
	if(panel != NULL)
	{
		if(panel->sliderCount == PANEL_MAX_SLIDERS)
		{
			SGE_GUI_LogPrintLine(SGE_LOG_WARNING, "Failed to add Slider! Max amount of sliders [%d] in panel %s reached.", PANEL_MAX_SLIDERS, panel->titleStr);
			free(slider);
			return NULL;
		}
		panel->sliders[panel->sliderCount] = slider;
		panel->sliderCount += 1;
		panel->controlCount += 1;
		slider->parentPanel = panel;
		slider->alpha = slider->parentPanel->alpha;
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Added Control: %s->Slider %d", panel->titleStr, panel->sliderCount);
	}
	else
	{
		if(currentStateControls->sliderCount == STATE_MAX_SLIDERS)
		{
			SGE_GUI_LogPrintLine(SGE_LOG_WARNING, "Failed to add Slider! Max amount of parentless sliders [%d] reached.", STATE_MAX_SLIDERS);
			free(slider);
			return NULL;
		}
		currentStateControls->sliders[currentStateControls->sliderCount] = slider;
		currentStateControls->sliderCount += 1;
		slider->parentPanel = NULL;
		slider->alpha = 255;
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Added Control: {NULL}->Slider %d", currentStateControls->sliderCount);
	}
	
	if(slider->parentPanel != NULL)
	{
		slider->boundBox.x = slider->x + slider->parentPanel->background.x + slider->parentPanel->x_scroll_offset;
		slider->boundBox.y = slider->y + slider->parentPanel->background.y + slider->parentPanel->y_scroll_offset;
	}
	else
	{
		slider->boundBox.x = slider->x;
		slider->boundBox.y = slider->y;
	}
	
	slider->bar.w = 125;
	slider->bar.h = 5;
	slider->slider.w = 12;
	slider->slider.h = 25;
	
	slider->bar.x = slider->boundBox.x;
	slider->bar.y = slider->boundBox.y + (slider->slider.h / 2) - (slider->bar.h / 2);
	
	slider->x_offset = (double)(slider->slider.w) / (double)slider->bar.w;
	slider->value = 0.5f;
	slider->value_i = slider->value / (1 / (1 - slider->x_offset));
	slider->slider_xi = slider->bar.x + (slider->value_i * slider->bar.w);
	slider->slider.x = slider->slider_xi;
	slider->slider.y = slider->bar.y - (slider->slider.h / 2) + (slider->bar.h / 2);
	
	slider->sliderColor = SGE_COLOR_DARK_RED;
	slider->barColor = SGE_COLOR_WHITE;
	slider->move_dx = 0;
	
	slider->boundBox.w = slider->bar.w;
	slider->boundBox.h = slider->slider.h;
	
	slider->state = SGE_CONTROL_STATE_NORMAL;
	slider->onMouseDown = onDownFallbackCallback;
	slider->onMouseDown_data = NULL;
	slider->onMouseUp = onUpFallbackCallback;
	slider->onMouseUp_data = NULL;
	slider->onSlide = onSlideFallback;
	slider->onSlide_data = NULL;
	
	/* Recalculate the parent panel's MCR */
	if(panel != NULL)
	{
		SGE_WindowPanelCalculateMCR(panel, slider->boundBox);
	}
	
	return slider;
}

void SGE_DestroySlider(SGE_Slider *slider)
{
	if(slider != NULL)
	{
		free(slider);
	}
}

void SGE_SliderHandleEvents(SGE_Slider *slider)
{
	if(SGE_GetSDLEvent()->type == SDL_MOUSEBUTTONDOWN)
	{
		if(SGE_GetSDLEvent()->button.button == 1)
		{
			if(SGE_MouseInRect(&slider->slider))
			{
				if(slider->parentPanel != NULL)
				{
					if(SGE_MouseInRect(&slider->parentPanel->background) && !SGE_MouseInRect(&slider->parentPanel->horizontalScrollbarBG) && !SGE_MouseInRect(&slider->parentPanel->verticalScrollbarBG))
					{
						slider->state = SGE_CONTROL_STATE_CLICKED;
						slider->move_dx = SGE_GetMouseX() - slider->slider.x;
						slider->onMouseDown(slider->onMouseDown_data);
					}
				}
				else
				{
					slider->state = SGE_CONTROL_STATE_CLICKED;
					slider->move_dx = SGE_GetMouseX() - slider->slider.x;
					slider->onMouseDown(slider->onMouseDown_data);
				}
			}
			else if(SGE_MouseInRect(&slider->bar))
			{
				if(slider->parentPanel != NULL)
				{
					if(SGE_MouseInRect(&slider->parentPanel->background) && !SGE_MouseInRect(&slider->parentPanel->horizontalScrollbarBG) && !SGE_MouseInRect(&slider->parentPanel->verticalScrollbarBG))
					{
						slider->state = SGE_CONTROL_STATE_CLICKED;
						slider->slider_xi = SGE_GetMouseX() - (slider->slider.w / 2);
						slider->slider.x = slider->slider_xi;
						slider->move_dx = SGE_GetMouseX() - slider->slider.x;
						SGE_SliderUpdateValue(slider);
						slider->onMouseDown(slider->onMouseDown_data);
						slider->onSlide(slider->onSlide_data);
					}
				}
				else
				{
					slider->state = SGE_CONTROL_STATE_CLICKED;
					slider->slider_xi = SGE_GetMouseX() - (slider->slider.w / 2);
					slider->slider.x = slider->slider_xi;
					slider->move_dx = SGE_GetMouseX() - slider->slider.x;
					SGE_SliderUpdateValue(slider);
					slider->onMouseDown(slider->onMouseDown_data);
					slider->onSlide(slider->onSlide_data);
				}
			}
		}
	}
	else if(SGE_GetSDLEvent()->type == SDL_MOUSEBUTTONUP)
	{
		if(SGE_GetSDLEvent()->button.button == 1)
		{
			if(slider->state == SGE_CONTROL_STATE_CLICKED)
			{
				slider->onMouseUp(slider->onMouseUp_data);
				if(SGE_MouseInRect(&slider->slider))
				{
					slider->state = SGE_CONTROL_STATE_HOVER;
				}
				else
				{
					slider->state = SGE_CONTROL_STATE_NORMAL;
				}
			}
		}
	}
	else if(SGE_GetSDLEvent()->type == SDL_MOUSEMOTION)
	{
		if(slider->state == SGE_CONTROL_STATE_CLICKED)
		{
			slider->onSlide(slider->onSlide_data);
		}
	}
}

void SGE_SliderUpdate(SGE_Slider *slider)
{
	/* Move the slider to new position */
	if(slider->state == SGE_CONTROL_STATE_CLICKED)
	{
		/* This line causes the slider click bug that changes the value set using the
		 * SGE_SliderSetValue() function.
		 * Clicking the slider causes this line to recalculate the value of slider_xi set 
		 * by the function, this recalculation uses mouse_x and move_dx which are integers
		 * and so slider_xi loses it's decimal part.
		 * Possible fix is to skip recalculation if there is no difference in the result
		 * and slider_xi apart from the decimal part.
		*/
		if((SGE_GetMouseX() - slider->move_dx) != (int)slider->slider_xi)
		{
			slider->slider_xi = SGE_GetMouseX() - slider->move_dx;
			if(slider->slider_xi < slider->bar.x)
				slider->slider_xi = slider->bar.x;
			if(slider->slider_xi > slider->bar.x + slider->bar.w  - slider->slider.w)
				slider->slider_xi = slider->bar.x + slider->bar.w - slider->slider.w;
			
			slider->slider.x = slider->slider_xi;
			SGE_SliderUpdateValue(slider);
		}
	}
	
	/* Recalculate position based on panel */
	if(slider->parentPanel != NULL)
	{
		slider->boundBox.x = slider->x + slider->parentPanel->background.x + slider->parentPanel->x_scroll_offset;
		slider->boundBox.y = slider->y + slider->parentPanel->background.y + slider->parentPanel->y_scroll_offset;
		slider->bar.x = slider->boundBox.x;
		slider->bar.y = slider->boundBox.y + (slider->slider.h / 2) - (slider->bar.h / 2);
		slider->slider_xi = slider->bar.x + (slider->value_i * slider->bar.w);
		slider->slider.x = slider->slider_xi;
		slider->slider.y = slider->bar.y - (slider->slider.h / 2) + (slider->bar.h / 2);
		slider->alpha = slider->parentPanel->alpha;
	}
}

void SGE_SliderRender(SGE_Slider *slider)
{
	int i = 0;
	
	SGE_SetDrawColorRGBA(slider->barColor.r, slider->barColor.g, slider->barColor.b, slider->alpha);
	SGE_DrawFillRect(&slider->bar);
	SGE_SetDrawColorRGBA(0, 0, 0, slider->alpha);
	SGE_DrawRect(&slider->bar);
	
	SGE_SetDrawColorRGBA(slider->sliderColor.r, slider->sliderColor.g, slider->sliderColor.b, slider->alpha);
	SGE_DrawFillRect(&slider->slider);
	
	SGE_SetDrawColorRGBA(0, 0, 0, slider->alpha);
	if(SGE_MouseInRect(&slider->slider) || slider->state == SGE_CONTROL_STATE_CLICKED)
	{
		if(slider->parentPanel != NULL)
		{
			if(SGE_MouseInRect(&slider->parentPanel->background) && !SGE_MouseInRect(&slider->parentPanel->horizontalScrollbarBG) && !SGE_MouseInRect(&slider->parentPanel->verticalScrollbarBG))
				SGE_SetDrawColorRGBA(225, 225, 225, slider->alpha);
			
			for(i = slider->parentPanel->index + 1; i < currentStateControls->panelCount; i++)
			{
				if(SGE_MouseInRect(&currentStateControls->panels[i]->border))
					SGE_SetDrawColorRGBA(0, 0, 0, slider->alpha);
			}
		}
		else
			SGE_SetDrawColorRGBA(225, 225, 225, slider->alpha);
	}
	SGE_DrawRect(&slider->slider);
	
	if(showControlBounds)
	{
		SGE_SetDrawColorRGBA(controlBoundsColor.r, controlBoundsColor.g, controlBoundsColor.b, slider->alpha);
		SGE_DrawRect(&slider->boundBox);
	}
}

void SGE_SliderSetPosition(SGE_Slider *slider, int x, int y)
{
	slider->x = x;
	slider->y = y;
	
	/* Calculate the bounding box based on parent panel position */
	if(slider->parentPanel != NULL)
	{
		slider->boundBox.x = slider->x + slider->parentPanel->background.x + slider->parentPanel->x_scroll_offset;
		slider->boundBox.y = slider->y + slider->parentPanel->background.y + slider->parentPanel->y_scroll_offset;
	}
	else
	{
		slider->boundBox.x = slider->x;
		slider->boundBox.y = slider->y;
	}
	
	slider->bar.x = slider->boundBox.x;
	slider->bar.y = slider->boundBox.y + (slider->slider.h / 2) - (slider->bar.h / 2);
	slider->slider_xi = slider->bar.x + (slider->value_i * slider->bar.w);
	slider->slider.x = slider->slider_xi;
	slider->slider.y = slider->bar.y - (slider->slider.h / 2) + (slider->bar.h / 2);
	
	/* Recalculate the parent panel's MCR */
	if(slider->parentPanel != NULL)
	{
		SGE_WindowPanelCalculateMCR(slider->parentPanel, slider->boundBox);
	}
}

void SGE_SliderUpdateValue(SGE_Slider *slider)
{
	slider->value_i = (slider->slider_xi - slider->bar.x) / (double)slider->bar.w;
	/* Limit the value to be between 0 and 1 */
	if(slider->value_i > 1.0f - slider->x_offset)
	{
		slider->value_i = 1.0f - slider->x_offset;
	}
	if(slider->value_i < 0.0f)
	{
		slider->value_i = 0.0f;
	}
	slider->value = slider->value_i * (1 / (1 - slider->x_offset));
}

void SGE_SliderSetValue(SGE_Slider *slider, double value)
{
	slider->value = value;
	/* Limit the value to be between 0 and 1 */
	if(slider->value > 1.0f)
	{
		slider->value = 1.0f;
	}
	if(slider->value < 0.0f)
	{
		slider->value = 0.0f;
	}
	slider->value_i = slider->value / (1 / (1 - slider->x_offset));
	
	/* Recalculate slider position based on value */
	slider->slider_xi = slider->bar.x + (slider->value_i * slider->bar.w);
	slider->slider.x = slider->slider_xi;
}

SGE_TextInputBox *SGE_CreateTextInputBox(int maxTextLength, int x, int y, struct SGE_WindowPanel *panel)
{
	SGE_TextInputBox *textInputBox = NULL;
	textInputBox = (SGE_TextInputBox*)malloc(sizeof(SGE_TextInputBox));
	
	if(panel != NULL)
	{
		if(panel->textInputBoxCount == PANEL_MAX_TEXT_INPUT_BOXES)
		{
			SGE_GUI_LogPrintLine(SGE_LOG_WARNING, "Failed to add TextInputBox! Max amount of TextInputBoxes [%d] in panel %s reached.", PANEL_MAX_TEXT_INPUT_BOXES, panel->titleStr);
			free(textInputBox);
			return NULL;
		}
		/* Add this new textInputBox to the top of the parent panel's textInputBoxes list */
		panel->textInputBoxes[panel->textInputBoxCount] = textInputBox;
		panel->textInputBoxCount += 1;
		panel->controlCount += 1;
		textInputBox->parentPanel = panel;
		textInputBox->alpha = textInputBox->parentPanel->alpha;
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Added Control: %s->TextInputBox %d", panel->titleStr, panel->textInputBoxCount);
	}
	else
	{
		if(currentStateControls->textInputBoxCount == STATE_MAX_TEXT_INPUT_BOXES)
		{
			SGE_GUI_LogPrintLine(SGE_LOG_WARNING, "Failed to add TextInputBox! Max amount of parentless textInputBoxes [%d] reached.", STATE_MAX_TEXT_INPUT_BOXES);
			free(textInputBox);
			return NULL;
		}
		/* Add this new textInputBox to the top of the parentless buttons list */
		currentStateControls->textInputBoxes[currentStateControls->textInputBoxCount] = textInputBox;
		currentStateControls->textInputBoxCount += 1;
		textInputBox->parentPanel = NULL;
		textInputBox->alpha = 255;
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Added Control: {NULL}->TextInputBox %d", currentStateControls->textInputBoxCount);
	}
	
	textInputBox->x = x;
	textInputBox->y = y;
	
	/* Calculate the bounding box based on parent panel position */
	if(textInputBox->parentPanel != NULL)
	{
		textInputBox->boundBox.x = textInputBox->x + textInputBox->parentPanel->background.x + textInputBox->parentPanel->x_scroll_offset;
		textInputBox->boundBox.y = textInputBox->y + textInputBox->parentPanel->background.y + textInputBox->parentPanel->y_scroll_offset;
	}
	else
	{
		textInputBox->boundBox.x = textInputBox->x;
		textInputBox->boundBox.y = textInputBox->y;
	}
	
	textInputBox->textLengthLimit = maxTextLength;
	textInputBox->text = malloc(textInputBox->textLengthLimit * sizeof(char));
	textInputBox->text[0] = '\0';
	
	textInputBox->inputBox.x = textInputBox->boundBox.x;
	textInputBox->inputBox.y = textInputBox->boundBox.y;
	textInputBox->inputBox.w = 250;
	textInputBox->inputBox.h = 100;
	
	SGE_SetTextureWordWrap(textInputBox->inputBox.w);
	
	textInputBox->boundBox.w = textInputBox->inputBox.w;
	textInputBox->boundBox.h = textInputBox->inputBox.h;
	
	textInputBox->textImg = SGE_CreateTextureFromText(" ", textBoxFont, SGE_COLOR_BLACK, SGE_TEXT_MODE_BLENDED);
	textInputBox->textImg->x = textInputBox->inputBox.x + 5;
	textInputBox->textImg->y = textInputBox->inputBox.y + 5;
	
	textInputBox->cursor.x = textInputBox->textImg->x;
	textInputBox->cursor.y = textInputBox->textImg->y + 20;
	textInputBox->cursor.w = 10;
	textInputBox->cursor.h = 5;
	textInputBox->cursor_dx = 0;
	textInputBox->cursor_dy = 0;
	textInputBox->lastTextWidth = 0;
	textInputBox->currentCharWidth = textInputBox->textImg->w;
	textInputBox->characterWidthStack = SGE_LLCreate(NULL);
	textInputBox->lastSpacePosition = 0;

	textInputBox->showCursor = false;
	textInputBox->lastTime = 0;
	
	SDL_StopTextInput();
	textInputBox->isEnabled = false;
	
	textInputBox->onEnable = onEnableFallback;
	textInputBox->onEnable_data = NULL;
	textInputBox->onDisable = onDisableFallback;
	textInputBox->onDisable_data = NULL;
	textInputBox->onTextEnter = onTextEnterFallback;
	textInputBox->onTextEnter_data = NULL;
	textInputBox->onTextDelete = onTextDeleteFallback;
	textInputBox->onTextDelete_data = NULL;
	
	/* Recalculate the parent panel's MCR */
	if(panel != NULL)
	{
		SGE_WindowPanelCalculateMCR(panel, textInputBox->boundBox);
	}
	
	return textInputBox;
}

void SGE_DestroyTextInputBox(SGE_TextInputBox *textInputBox)
{
	if(textInputBox != NULL)
	{
		SGE_LLDestroy(textInputBox->characterWidthStack);
		free(textInputBox->text);
		SGE_FreeTexture(textInputBox->textImg);
		free(textInputBox);
	}
}

void SGE_TextInputBoxHandleEvents(SGE_TextInputBox *textInputBox)
{
	/* Enable or disable text input with mouse */
	if(SGE_GetSDLEvent()->type == SDL_MOUSEBUTTONDOWN)
	{
		if(SGE_MouseInRect(&textInputBox->inputBox))
		{
			if(textInputBox->parentPanel != NULL)
			{
				if(SGE_MouseInRect(&textInputBox->parentPanel->background) && !SGE_MouseInRect(&textInputBox->parentPanel->horizontalScrollbarBG) && !SGE_MouseInRect(&textInputBox->parentPanel->verticalScrollbarBG))
				{
					if(!textInputBox->isEnabled)
					{
						SDL_StartTextInput();
						textInputBox->isEnabled = true;
						textInputBox->onEnable(textInputBox->onEnable_data);
					}
				}
			}
			else
			{
				if(!textInputBox->isEnabled)
				{
					SDL_StartTextInput();
					textInputBox->isEnabled = true;
					textInputBox->onEnable(textInputBox->onEnable_data);
				}
			}
		}
		else
		{
			if(textInputBox->isEnabled)
			{
				SDL_StopTextInput();
				textInputBox->isEnabled = false;
				textInputBox->onDisable(textInputBox->onDisable_data);
			}
		}
	}

	/* Handle text input events */
	if(!textInputBox->isEnabled)
	{
		return;
	}
	
	static int lastTextWidth = 0;
	static int currentTextWidth = 0;
	static int lastTextHeight = 0;
	static int currentTextHeight = 0;
	static int cursor_advance_x = 0;

	switch(SGE_GetSDLEvent()->type)
	{
		case SDL_TEXTINPUT:
		int currentTextLength = strlen(textInputBox->text);
		if(currentTextLength < textInputBox->textLengthLimit - 1)
		{
			TTF_SizeText(textBoxFont, textInputBox->text, &lastTextWidth, NULL);
			lastTextHeight = textInputBox->textImg->h;

			strcat(textInputBox->text, SGE_GetSDLEvent()->text.text);
			SGE_SetTextureWordWrap(textInputBox->inputBox.w);
			SGE_UpdateTextureFromText(textInputBox->textImg, textInputBox->text, textBoxFont, SGE_COLOR_BLACK, SGE_TEXT_MODE_BLENDED);
			textInputBox->onTextEnter(textInputBox->onTextEnter_data);

			TTF_SizeText(textBoxFont, textInputBox->text, &currentTextWidth, NULL);
			currentTextHeight = textInputBox->textImg->h;

			cursor_advance_x = currentTextWidth - lastTextWidth;
			textInputBox->cursor_dx += cursor_advance_x;
			textInputBox->currentCharWidth = cursor_advance_x;
			SGE_LLPush(textInputBox->characterWidthStack, (void *)textInputBox->currentCharWidth);
			textInputBox->lastTextWidth = textInputBox->textImg->w;

			if(!strcmp(SGE_GetSDLEvent()->text.text, " "))
			{
				textInputBox->lastSpacePosition = currentTextLength;
			}

			// SGE_LogPrintLine(SGE_LOG_INFO, "current height: %d; last height: %d", currentTextHeight, lastTextHeight);
			if((currentTextHeight - lastTextHeight) >= TTF_FontLineSkip(textBoxFont))
			{
				int cursor_x_offset = 0;

				// This sets x offset to zero if the character that triggered a wrap is space itself.
				// Space indices should be cached so we can refer back to the last space index to solve this.
				// This cache can also be used for text delete events to roll back the last space position
				TTF_SizeText(textBoxFont, &textInputBox->text[textInputBox->lastSpacePosition + 1], &cursor_x_offset, NULL);
				SGE_LogPrintLine(SGE_LOG_INFO, "space str: %s; offset: %d", &textInputBox->text[textInputBox->lastSpacePosition + 1], cursor_x_offset);

				if(lastTextWidth - cursor_x_offset <= TTF_FontHeight(textBoxFont) || cursor_x_offset == 0)
				{
					cursor_x_offset = textInputBox->currentCharWidth;
					SGE_LogPrintLine(SGE_LOG_INFO, "offset str: %s; offset: %d", &textInputBox->text[currentTextLength], cursor_x_offset);
				}
				textInputBox->cursor_dx = cursor_x_offset;
				textInputBox->cursor_dy += currentTextHeight - lastTextHeight;
			}
		}
		else
			SGE_LogPrintLine(SGE_LOG_WARNING, "Max characters for textInputBox [%d] reached!", textInputBox->textLengthLimit);
		break;
		
		case SDL_KEYDOWN:
		if(SGE_GetSDLEvent()->key.keysym.sym == SDLK_BACKSPACE)
		{
			int i = strlen(textInputBox->text);
			if(i == 0) {
				return;
			}
			
			textInputBox->text[i - 1] = '\0';
			
			textInputBox->cursor_dx -= textInputBox->currentCharWidth;
			SGE_LLPop(textInputBox->characterWidthStack);

			SGE_SetTextureWordWrap(textInputBox->inputBox.w);
			if(i == 1)
			{
				SGE_UpdateTextureFromText(textInputBox->textImg, " ", textBoxFont, SGE_COLOR_BLACK, SGE_TEXT_MODE_BLENDED);
				textInputBox->currentCharWidth = 0;
				textInputBox->lastTextWidth = 0;
			}
			else
			{
				SGE_UpdateTextureFromText(textInputBox->textImg, textInputBox->text, textBoxFont, SGE_COLOR_BLACK, SGE_TEXT_MODE_BLENDED);
				textInputBox->currentCharWidth = (int) SGE_LLGetLast(textInputBox->characterWidthStack);
				textInputBox->lastTextWidth = textInputBox->textImg->w;
			}
			
			textInputBox->onTextDelete(textInputBox->onTextDelete_data);
		}
		else if(SGE_GetSDLEvent()->key.keysym.sym == SDLK_RETURN)
		{
			if(strlen(textInputBox->text) < textInputBox->textLengthLimit - 1)
			{
				strcat(textInputBox->text, "\n");
				SGE_SetTextureWordWrap(textInputBox->inputBox.w);
				SGE_UpdateTextureFromText(textInputBox->textImg, textInputBox->text, textBoxFont, SGE_COLOR_BLACK, SGE_TEXT_MODE_BLENDED);
				textInputBox->onTextEnter(textInputBox->onTextEnter_data);
			}
		}
		break;
	}
}

void SGE_TextInputBoxUpdate(SGE_TextInputBox *textInputBox)
{
	if(textInputBox->parentPanel != NULL)
	{
		textInputBox->boundBox.x = textInputBox->x + textInputBox->parentPanel->background.x + textInputBox->parentPanel->x_scroll_offset;
		textInputBox->boundBox.y = textInputBox->y + textInputBox->parentPanel->background.y + textInputBox->parentPanel->y_scroll_offset;
		textInputBox->inputBox.x = textInputBox->boundBox.x;
		textInputBox->inputBox.y = textInputBox->boundBox.y;
		textInputBox->textImg->x = textInputBox->inputBox.x + 5;
		textInputBox->textImg->y = textInputBox->inputBox.y + 5;
		
		textInputBox->alpha = textInputBox->parentPanel->alpha;
		SGE_SetTextureAlpha(textInputBox->textImg, textInputBox->alpha);
	}

	textInputBox->cursor.x = textInputBox->textImg->x + textInputBox->cursor_dx;
	textInputBox->cursor.y = textInputBox->textImg->y + 20 + textInputBox->cursor_dy;
	
	if(SDL_GetTicks() - textInputBox->lastTime > 500)
	{
		textInputBox->showCursor = !textInputBox->showCursor;
		textInputBox->lastTime = SDL_GetTicks();
	}
}

void SGE_TextInputBoxRender(SGE_TextInputBox *textInputBox)
{
	if(textInputBox->parentPanel != NULL)
	{
		if(textInputBox->parentPanel->isMinimized)
			return;
	}
	
	SGE_SetDrawColorRGBA(150, 150, 150, textInputBox->alpha);
	SGE_DrawFillRect(&textInputBox->inputBox);

	if(textInputBox->parentPanel != NULL)
	{
		//SDL_Rect textClipRect = {};
		SGE_SetDrawClipRect(&textInputBox->inputBox);
	}
	else
	{
		SGE_SetDrawClipRect(&textInputBox->inputBox);
	}
	
	if(textInputBox->isEnabled)
	{
		if(textInputBox->showCursor)
		{
			SGE_SetDrawColorRGBA(150, 0, 0, textInputBox->alpha);
			SGE_DrawFillRect(&textInputBox->cursor);
			SGE_SetDrawColorRGBA(255, 255, 255, textInputBox->alpha);
			SGE_DrawRect(&textInputBox->cursor);
		}
	}
	
	SGE_RenderTexture(textInputBox->textImg);

	if(textInputBox->parentPanel != NULL)
	{
		SGE_SetDrawClipRect(&textInputBox->parentPanel->background);
	}
	else
	{
		SGE_SetDrawClipRect(NULL);
	}
	
	SGE_SetDrawColorRGBA(0, 0, 0, textInputBox->alpha);
	if(SGE_MouseInRect(&textInputBox->inputBox))
	{
		if(textInputBox->parentPanel != NULL)
		{
			if(SGE_MouseInRect(&textInputBox->parentPanel->background) && !SGE_MouseInRect(&textInputBox->parentPanel->horizontalScrollbarBG) && !SGE_MouseInRect(&textInputBox->parentPanel->verticalScrollbarBG))
				SGE_SetDrawColorRGBA(255, 255, 255, textInputBox->alpha);
			
			int i = 0;
			for(i = textInputBox->parentPanel->index + 1; i < currentStateControls->panelCount; i++)
			{
				if(SGE_MouseInRect(&currentStateControls->panels[i]->border))
					SGE_SetDrawColorRGBA(0, 0, 0, textInputBox->alpha);
			}
		}
		else
			SGE_SetDrawColorRGBA(255, 255, 255, textInputBox->alpha);
	}
	SGE_DrawRect(&textInputBox->inputBox);
	
	if(showControlBounds)
	{
		SGE_SetDrawColorRGBA(controlBoundsColor.r, controlBoundsColor.g, controlBoundsColor.b, textInputBox->alpha);
		SGE_DrawRect(&textInputBox->textImg->destRect);
		SGE_DrawRect(&textInputBox->boundBox);
	}
}

void SGE_TextInputBoxSetPosition(SGE_TextInputBox *textInputBox, int x, int y)
{
	textInputBox->x = x;
	textInputBox->y = y;
	
	/* Calculate the bounding box based on parent panel position */
	if(textInputBox->parentPanel != NULL)
	{
		textInputBox->boundBox.x = textInputBox->x + textInputBox->parentPanel->background.x + textInputBox->parentPanel->x_scroll_offset;
		textInputBox->boundBox.y = textInputBox->y + textInputBox->parentPanel->background.y + textInputBox->parentPanel->y_scroll_offset;
	}
	else
	{
		textInputBox->boundBox.x = textInputBox->x;
		textInputBox->boundBox.y = textInputBox->y;
	}
	
	textInputBox->inputBox.x = textInputBox->boundBox.x;
	textInputBox->inputBox.y = textInputBox->boundBox.y;
	textInputBox->textImg->x = textInputBox->inputBox.x + 5;
	textInputBox->textImg->y = textInputBox->inputBox.y + 5;
	textInputBox->cursor.x = textInputBox->textImg->x;
	textInputBox->cursor.y = textInputBox->textImg->y + 20;
	
	/* Recalculate the parent panel's MCR */
	if(textInputBox->parentPanel != NULL)
	{
		SGE_WindowPanelCalculateMCR(textInputBox->parentPanel, textInputBox->boundBox);
	}
}

void SGE_TextInputBoxClear(SGE_TextInputBox *textInputBox)
{
	textInputBox->text[0] = '\0';
	SGE_UpdateTextureFromText(textInputBox->textImg, " ", textBoxFont, SGE_COLOR_BLACK, SGE_TEXT_MODE_BLENDED);
	textInputBox->currentCharWidth = 0;
	textInputBox->lastTextWidth = 0;
	SGE_LLClear(textInputBox->characterWidthStack);
	textInputBox->cursor_dx = 0;
	textInputBox->cursor_dy = 0;
}

SGE_ListBox *SGE_CreateListBox(int listCount, char list[][LIST_OPTION_LENGTH], int x, int y, SGE_WindowPanel *panel)
{
	int i = 0;
	
	SGE_ListBox *listBox = NULL;
	listBox = (SGE_ListBox *)malloc(sizeof(SGE_ListBox));
	
	if(panel != NULL)
	{
		if(panel->listBoxCount == PANEL_MAX_LISTBOXES)
		{
			SGE_GUI_LogPrintLine(SGE_LOG_WARNING, "Failed to add ListBox! Max amount of listBoxes [%d] in panel %s reached.", PANEL_MAX_LISTBOXES, panel->titleStr);
			free(listBox);
			return NULL;
		}
		/* Add this new listBox to the top of the parent panel's listBoxes list */
		panel->listBoxes[panel->listBoxCount] = listBox;
		panel->listBoxCount += 1;
		panel->controlCount += 1;
		listBox->parentPanel = panel;
		listBox->alpha = listBox->parentPanel->alpha;
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Added Control: %s->ListBox %d", panel->titleStr, panel->listBoxCount);
	}
	else
	{
		if(currentStateControls->listBoxCount == STATE_MAX_LISTBOXES)
		{
			SGE_GUI_LogPrintLine(SGE_LOG_WARNING, "Failed to add ListBox! Max amount of parentless listBoxes [%d] reached.", STATE_MAX_LISTBOXES);
			free(listBox);
			return NULL;
		}
		/* Add this new listBox to the top of the parentless listBoxes list */
		currentStateControls->listBoxes[currentStateControls->listBoxCount] = listBox;
		currentStateControls->listBoxCount += 1;
		listBox->parentPanel = NULL;
		listBox->alpha = 255;
		SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Added Control: {NULL}->ListBox %d", currentStateControls->listBoxCount);
	}
	
	listBox->x = x;
	listBox->y = y;
	
	for(i = 0; i < listCount; i++)
	{
		strcpy(listBox->optionList[i], list[i]);
	}
	
	if(listBox->parentPanel != NULL)
	{
		listBox->boundBox.x = listBox->x + listBox->parentPanel->background.x + listBox->parentPanel->x_scroll_offset;
		listBox->boundBox.y = listBox->y + listBox->parentPanel->background.y + listBox->parentPanel->y_scroll_offset;
	}
	else
	{
		listBox->boundBox.x = listBox->x;
		listBox->boundBox.y = listBox->y;
	}
	
	listBox->selectionBox.x = listBox->boundBox.x;
	listBox->selectionBox.y = listBox->boundBox.y;
	listBox->selectionBox.w = 200;
	listBox->selectionBox.h = 25;
	
	listBox->boundBox.w = listBox->selectionBox.w;
	listBox->boundBox.h = listBox->selectionBox.h;
	
	listBox->selection = 0;
	listBox->selectionImg = SGE_CreateTextureFromText(list[listBox->selection], listBoxFont, SGE_COLOR_BLACK, SGE_TEXT_MODE_BLENDED);
	listBox->selectionImg->x = listBox->selectionBox.x + 2;
	listBox->selectionImg->y = listBox->selectionBox.y + 2;
	
	listBox->optionCount = listCount;
	for(i = 0; i < listBox->optionCount; i++)
	{
		listBox->optionBoxes[i].x = listBox->selectionBox.x;
		listBox->optionBoxes[i].y = listBox->selectionBox.y + (i * listBox->selectionBox.h) + listBox->selectionBox.h;
		listBox->optionBoxes[i].w = listBox->selectionBox.w;
		listBox->optionBoxes[i].h = listBox->selectionBox.h;
		
		listBox->optionImages[i] = SGE_CreateTextureFromText(list[i], listBoxFont, SGE_COLOR_BLACK, SGE_TEXT_MODE_BLENDED);
		listBox->optionImages[i]->x = listBox->optionBoxes[i].x + 2;
		listBox->optionImages[i]->y = listBox->optionBoxes[i].y + 2;
	}
	
	listBox->isOpen = false;
	
	listBox->onSelectionChange = onSelectionChangeFallback;
	listBox->onSelectionChange_data = NULL;
	
	/* Recalculate the parent panel's MCR */
	if(panel != NULL)
	{
		SGE_WindowPanelCalculateMCR(panel, listBox->boundBox);
	}
	
	return listBox;
}

void SGE_DestroyListBox(SGE_ListBox *listBox)
{
	int i = 0;
	
	if(listBox != NULL)
	{
		SGE_FreeTexture(listBox->selectionImg);
		for(i = 0; i < listBox->optionCount; i++)
		{
			SGE_FreeTexture(listBox->optionImages[i]);
		}
		free(listBox);
	}
}

void SGE_ListBoxHandleEvents(SGE_ListBox *listBox)
{
	if(SGE_GetSDLEvent()->type == SDL_MOUSEBUTTONDOWN)
	{
		if(SGE_MouseInRect(&listBox->selectionBox))
		{
			if(listBox->parentPanel != NULL)
			{
				if(SGE_MouseInRect(&listBox->parentPanel->background) && !SGE_MouseInRect(&listBox->parentPanel->horizontalScrollbarBG) && !SGE_MouseInRect(&listBox->parentPanel->verticalScrollbarBG))
					listBox->isOpen = !listBox->isOpen;
			}
			else
				listBox->isOpen = !listBox->isOpen;
		}
		
		if(listBox->isOpen)
		{
			int i = 0;
			for(i = 0; i < listBox->optionCount; i++)
			{
				if(SGE_MouseInRect(&listBox->optionBoxes[i]))
				{
					if(listBox->parentPanel != NULL)
					{
						if(SGE_MouseInRect(&listBox->parentPanel->background) && !SGE_MouseInRect(&listBox->parentPanel->horizontalScrollbarBG) && !SGE_MouseInRect(&listBox->parentPanel->verticalScrollbarBG))
						{
							if(i != listBox->selection)
							{
								listBox->selection = i;
								SGE_UpdateTextureFromText(listBox->selectionImg, listBox->optionList[i], listBoxFont, SGE_COLOR_BLACK, SGE_TEXT_MODE_BLENDED);
								listBox->onSelectionChange(listBox->onSelectionChange_data);
							}
							listBox->isOpen = false;
						}
					}
					else
					{
						if(i != listBox->selection)
						{
							listBox->selection = i;
							SGE_UpdateTextureFromText(listBox->selectionImg, listBox->optionList[i], listBoxFont, SGE_COLOR_BLACK, SGE_TEXT_MODE_BLENDED);
							listBox->onSelectionChange(listBox->onSelectionChange_data);
						}
						listBox->isOpen = false;
					}
				}
			}
		}
	}
}

void SGE_ListBoxUpdate(SGE_ListBox *listBox)
{
	if(listBox->parentPanel != NULL)
	{
		int i = 0;
		
		listBox->boundBox.x = listBox->x + listBox->parentPanel->background.x + listBox->parentPanel->x_scroll_offset;
		listBox->boundBox.y = listBox->y + listBox->parentPanel->background.y + listBox->parentPanel->y_scroll_offset;
		listBox->selectionBox.x = listBox->boundBox.x;
		listBox->selectionBox.y = listBox->boundBox.y;
		listBox->selectionImg->x = listBox->selectionBox.x + 2;
		listBox->selectionImg->y = listBox->selectionBox.y + 2;
		listBox->alpha = listBox->parentPanel->alpha;
		SGE_SetTextureAlpha(listBox->selectionImg, listBox->alpha);
		
		listBox->boundBox.h = listBox->selectionBox.h;
		if(listBox->isOpen)
		{
			for(i = 0; i < listBox->optionCount; i++)
			{
				listBox->optionBoxes[i].x = listBox->selectionBox.x;
				listBox->optionBoxes[i].y = listBox->selectionBox.y + (i * listBox->selectionBox.h) + listBox->selectionBox.h;
				listBox->optionImages[i]->x = listBox->optionBoxes[i].x + 2;
				listBox->optionImages[i]->y = listBox->optionBoxes[i].y + 2;
				SGE_SetTextureAlpha(listBox->optionImages[i], listBox->alpha);
				listBox->boundBox.h = listBox->selectionBox.h + (i + 1) * listBox->optionBoxes[i].h;
			}
		}
	}
}

void SGE_ListBoxRender(SGE_ListBox *listBox)
{
	SGE_SetDrawColorRGBA(255, 255, 255, listBox->alpha);
	SGE_DrawFillRect(&listBox->selectionBox);
	SGE_RenderTexture(listBox->selectionImg);
	
	SGE_SetDrawColorRGBA(0, 0, 0, listBox->alpha);
	if(SGE_MouseInRect(&listBox->selectionBox))
	{
		if(listBox->parentPanel != NULL)
		{
			if(SGE_MouseInRect(&listBox->parentPanel->background) && !SGE_MouseInRect(&listBox->parentPanel->horizontalScrollbarBG) && !SGE_MouseInRect(&listBox->parentPanel->verticalScrollbarBG))
				SGE_SetDrawColorRGBA(150, 150, 150, listBox->alpha);
			
			int i = 0;
			for(i = listBox->parentPanel->index + 1; i < currentStateControls->panelCount; i++)
			{
				if(SGE_MouseInRect(&currentStateControls->panels[i]->border))
					SGE_SetDrawColorRGBA(0, 0, 0, listBox->alpha);
			}
		}
		else
			SGE_SetDrawColorRGBA(150, 150, 150, listBox->alpha);
	}
	SGE_DrawRect(&listBox->selectionBox);
	
	if(showControlBounds)
	{
		SGE_SetDrawColorRGBA(controlBoundsColor.r, controlBoundsColor.g, controlBoundsColor.b, listBox->alpha);
		SGE_DrawRect(&listBox->boundBox);
	}
	
	if(listBox->isOpen)
	{
		int i = 0;
		for(i = 0; i < listBox->optionCount; i++)
		{
			SGE_SetDrawColorRGBA(255, 255, 255, listBox->alpha);
			if(SGE_MouseInRect(&listBox->optionBoxes[i]))
			{
				if(listBox->parentPanel != NULL)
				{
					if(SGE_MouseInRect(&listBox->parentPanel->background) && !SGE_MouseInRect(&listBox->parentPanel->horizontalScrollbarBG) && !SGE_MouseInRect(&listBox->parentPanel->verticalScrollbarBG))
						SGE_SetDrawColorRGBA(50, 50, 150, listBox->alpha);
					
					int i = 0;
					for(i = listBox->parentPanel->index + 1; i < currentStateControls->panelCount; i++)
					{
						if(SGE_MouseInRect(&currentStateControls->panels[i]->border))
							SGE_SetDrawColorRGBA(255, 255, listBox->alpha, listBox->alpha);
					}
				}
				else
					SGE_SetDrawColorRGBA(50, 50, 150, listBox->alpha);
			}
			else
				SGE_SetDrawColorRGBA(255, 255, 255, listBox->alpha);
			SGE_DrawFillRect(&listBox->optionBoxes[i]);
			
			SGE_SetDrawColorRGBA(0, 0, 0, listBox->alpha);
			SGE_DrawRect(&listBox->optionBoxes[i]);
			SGE_RenderTexture(listBox->optionImages[i]);
		}
	}
}

void SGE_ListBoxSetPosition(SGE_ListBox *listBox, int x, int y)
{
	listBox->x = x;
	listBox->y = y;
	
	/* Calculate the bounding box based on parent panel position */
	if(listBox->parentPanel != NULL)
	{
		listBox->boundBox.x = listBox->x + listBox->parentPanel->background.x + listBox->parentPanel->x_scroll_offset;
		listBox->boundBox.y = listBox->y + listBox->parentPanel->background.y + listBox->parentPanel->y_scroll_offset;
	}
	else
	{
		listBox->boundBox.x = listBox->x;
		listBox->boundBox.y = listBox->y;
	}
	
	listBox->selectionBox.x = listBox->boundBox.x;
	listBox->selectionBox.y = listBox->boundBox.y;
	listBox->selectionImg->x = listBox->selectionBox.x + 2;
	listBox->selectionImg->y = listBox->selectionBox.y + 2;
	
	int i = 0;
	for(i = 0; i < listBox->optionCount; i++)
	{
		listBox->optionBoxes[i].x = listBox->selectionBox.x;
		listBox->optionBoxes[i].y = listBox->selectionBox.y + (i * listBox->selectionBox.h) + listBox->selectionBox.h;
		listBox->optionImages[i]->x = listBox->optionBoxes[i].x + 2;
		listBox->optionImages[i]->y = listBox->optionBoxes[i].y + 2;
	}
	
	/* Recalculate the parent panel's MCR */
	if(listBox->parentPanel != NULL)
	{
		SGE_WindowPanelCalculateMCR(listBox->parentPanel, listBox->boundBox);
	}
}

SGE_MinimizeButton *SGE_CreateMinimizeButton(SGE_WindowPanel *panel)
{
	SGE_MinimizeButton *minimizeButton = (SGE_MinimizeButton *) malloc(sizeof(SGE_MinimizeButton));
	minimizeButton->parentPanel = panel;
	minimizeButton->buttonImg = SGE_LoadTexture("assets/minimize_icon.png");
	
	minimizeButton->boundBox.w = minimizeButton->buttonImg->w + 5;
	minimizeButton->boundBox.h = minimizeButton->buttonImg->h + 5;
	minimizeButton->boundBox.x = panel->background.x;
	minimizeButton->boundBox.y = panel->titleRect.y + (panel->titleRect.h / 2) - (minimizeButton->boundBox.h / 2);
	minimizeButton->buttonImg->x = minimizeButton->boundBox.x + (minimizeButton->boundBox.w / 2) - (minimizeButton->buttonImg->w / 2);
	minimizeButton->buttonImg->y = minimizeButton->boundBox.y + (minimizeButton->boundBox.h / 2) - (minimizeButton->buttonImg->h / 2);
	
	minimizeButton->state = SGE_CONTROL_STATE_NORMAL;
	minimizeButton->normalColor = SGE_COLOR_DARK_RED;
	minimizeButton->hoverColor = SGE_COLOR_GRAY;
	minimizeButton->clickedColor = SGE_COLOR_BLACK;
	minimizeButton->currentColor = minimizeButton->normalColor;
	
	return minimizeButton;
}

void SGE_DestroyMinimizeButton(SGE_MinimizeButton *minButton)
{
	if(minButton != NULL)
	{
		SGE_FreeTexture(minButton->buttonImg);
		free(minButton);
	}
}

void SGE_MinimizeButtonHandleEvents(SGE_MinimizeButton *minButton)
{
	int i = 0;
	
	if(SGE_GetSDLEvent()->type == SDL_MOUSEBUTTONDOWN)
	{
		if(SGE_GetSDLEvent()->button.button == 1)
		{
			if(SGE_MouseInRect(&minButton->boundBox))
			{
				minButton->state = SGE_CONTROL_STATE_CLICKED;
				for(i = minButton->parentPanel->index + 1; i < currentStateControls->panelCount; i++)
				{
					if(SGE_MouseInRect(&currentStateControls->panels[i]->border))
						minButton->state = SGE_CONTROL_STATE_NORMAL;
				}
			}
		}
	}
	else if(SGE_GetSDLEvent()->type == SDL_MOUSEBUTTONUP)
	{
		if(SGE_GetSDLEvent()->button.button == 1)
		{
			if(minButton->state == SGE_CONTROL_STATE_CLICKED)
			{
				if(SGE_MouseInRect(&minButton->boundBox))
				{
					minButton->state = SGE_CONTROL_STATE_HOVER;
					SGE_WindowPanelToggleMinimized(minButton->parentPanel);
				}
				else
				{
					minButton->state = SGE_CONTROL_STATE_NORMAL;
				}
			}
		}
	}
	else if(SGE_GetSDLEvent()->type == SDL_MOUSEMOTION)
	{
		if(minButton->state != SGE_CONTROL_STATE_CLICKED)
		{
			if(SGE_MouseInRect(&minButton->boundBox))
			{
				minButton->state = SGE_CONTROL_STATE_HOVER;
				for(i = minButton->parentPanel->index + 1; i < currentStateControls->panelCount; i++)
				{
					if(SGE_MouseInRect(&currentStateControls->panels[i]->border))
						minButton->state = SGE_CONTROL_STATE_NORMAL;
				}
			}
			else
				minButton->state = SGE_CONTROL_STATE_NORMAL;
		}
	}
}

void SGE_MinimizeButtonUpdate(SGE_MinimizeButton *minButton)
{
	if(minButton->state == SGE_CONTROL_STATE_NORMAL)
		minButton->currentColor = minButton->normalColor;
	else if(minButton->state == SGE_CONTROL_STATE_HOVER)
		minButton->currentColor = minButton->hoverColor;
	else if(minButton->state == SGE_CONTROL_STATE_CLICKED)
		minButton->currentColor = minButton->clickedColor;
}

void SGE_MinimizeButtonRender(SGE_MinimizeButton *minButton)
{
	int i = 0;
	
	SGE_SetDrawColorRGBA(minButton->currentColor.r, minButton->currentColor.g, minButton->currentColor.b, minButton->parentPanel->alpha);
	SGE_DrawFillRect(&minButton->boundBox);
	
	/* Draw button image */
	SGE_SetTextureAlpha(minButton->buttonImg, minButton->parentPanel->alpha);
	SGE_RenderTexture(minButton->buttonImg);
	
	/* Draw button border */
	SGE_SetDrawColorRGBA(0, 0, 0, minButton->parentPanel->alpha);
	if(SGE_MouseInRect(&minButton->boundBox))
	{
		SGE_SetDrawColorRGBA(225, 225, 225, minButton->parentPanel->alpha);
		
		for(i = minButton->parentPanel->index + 1; i < currentStateControls->panelCount; i++)
		{
			if(SGE_MouseInRect(&currentStateControls->panels[i]->border))
				SGE_SetDrawColorRGBA(0, 0, 0, minButton->parentPanel->alpha);
		}
	}
	SGE_DrawRect(&minButton->boundBox);
}

SGE_WindowPanel *SGE_CreateWindowPanel(const char *title, int x, int y, int w, int h) 
{
	int i = 0;

	SGE_WindowPanel **panels = currentStateControls->panels;
	int panelCount = currentStateControls->panelCount;
	
	if(panelCount == STATE_MAX_PANELS)
	{
		SGE_GUI_LogPrintLine(SGE_LOG_WARNING, "Failed to create panel %s! Max amount of Panels [%d] reached.", title, STATE_MAX_PANELS);
		return NULL;
	}
	
	/* Create the new panel, add it to the top of the panels stack and set it as the active window */
	SGE_WindowPanel *panel = NULL;
	panel = (SGE_WindowPanel *)malloc(sizeof(SGE_WindowPanel));
	
	panel->index = currentStateControls->panelCount;
	panels[currentStateControls->panelCount] = panel;
	currentStateControls->panelCount += 1;
	
	strncpy(panel->titleStr, title, 50);
	SGE_SetActiveWindowPanel(panel);
	SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Added Panel: %s", panel->titleStr);
	
	panel->titleTextImg = SGE_CreateTextureFromText(title, panelTitleFont, SGE_COLOR_WHITE, SGE_TEXT_MODE_BLENDED);
	if(panel->titleTextImg == NULL)
	{
		SGE_GUI_LogPrintLine(SGE_LOG_ERROR, "Failed to load render panel title text!");
		free(panel);
		return NULL;
	}
	
	panel->isVisible = true;
	panel->backgroundColor = SGE_COLOR_LIGHT_GRAY;
	panel->borderColor = SGE_COLOR_DARK_RED;
	panel->alpha = 255;
	panel->borderThickness = 5;
	panel->titleHeight = 30;
	
	panel->boundBox.x = x;
	panel->boundBox.y = y;
	panel->background.w = w;
	panel->background.h = h;
	
	/* Calculate the panel position and size */
	panel->border.x = panel->boundBox.x;
	panel->border.y = panel->boundBox.y;
	
	panel->border.w = panel->background.w + (2 * panel->borderThickness);
	panel->border.h = panel->background.h + (2 * panel->borderThickness) + panel->titleHeight - panel->borderThickness;
	
	panel->boundBox.w = panel->border.w;
	panel->boundBox.h = panel->border.h;
	
	panel->background.x = panel->border.x + panel->borderThickness;
	panel->background.y = panel->border.y + panel->borderThickness + panel->titleHeight - panel->borderThickness;
	
	panel->titleRect.x = panel->border.x;
	panel->titleRect.y = panel->border.y;
	panel->titleRect.w = panel->border.w;
	panel->titleRect.h = panel->titleHeight + panel->borderThickness / 2;
	
	panel->titleTextImg->x = panel->titleRect.x + (panel->titleRect.w / 2) - (panel->titleTextImg->w / 2);
	panel->titleTextImg->y = panel->titleRect.y + (panel->titleRect.h / 2) - (panel->titleTextImg->h / 2);
	
	panel->bgLocalCenter.x = panel->background.w / 2;
	panel->bgLocalCenter.y = panel->background.h / 2;
	panel->bgGlobalCenter.x = panel->bgLocalCenter.x + panel->background.x;
	panel->bgGlobalCenter.y = panel->bgLocalCenter.y + panel->background.y;
	
	panel->isMovable = true;
	panel->isMoving = false;
	panel->move_dx = 0, panel->move_dy = 0;
	
	panel->isResizable = true;
	panel->isResizing_vertical = false;
	panel->isResizing_horizontal = false;
	panel->resize_origin_x = 0, panel->resize_origin_y = 0;
	panel->resize_origin_w = 0, panel->resize_origin_h = 0;
	
	panel->resizeBar_vertical.w = 30;
	panel->resizeBar_vertical.h = 10;
	panel->resizeBar_vertical.x = panel->border.x + panel->border.w - panel->resizeBar_vertical.w;
	panel->resizeBar_vertical.y = panel->border.y + panel->border.h - panel->resizeBar_vertical.h;
	
	panel->resizeBar_horizontal.w = 10;
	panel->resizeBar_horizontal.h = 30;
	panel->resizeBar_horizontal.x = panel->border.x + panel->border.w - panel->resizeBar_horizontal.w;
	panel->resizeBar_horizontal.y = panel->border.y + panel->border.h - panel->resizeBar_horizontal.h;
	
	panel->minimizeButton = SGE_CreateMinimizeButton(panel);
	panel->isMinimizable = true;
	panel->isMinimized = false;
	panel->temp_border_w = 0;
	panel->temp_border_h = 0;
	panel->temp_background_w = 0;
	panel->temp_background_h = 0;
	
	panel->masterControlRect.x = 0;
	panel->masterControlRect.y = 0;
	panel->masterControlRect.w = 0;
	panel->masterControlRect.h = 0;
	
	panel->temp_horizontalScrollbarEnabled = false;
	panel->temp_verticalScrollbarEnabled = false;
	
	panel->isScrolling_horizontal = false;
	panel->scroll_dx = 0;
	panel->horizontalScrollbar_move_dx = 0;
	panel->x_scroll_offset = 0;
	
	panel->horizontalScrollbarEnabled = false;
	panel->horizontalScrollbarBG.x = panel->background.x;
	panel->horizontalScrollbarBG.h = panel->titleHeight - 10;
	panel->horizontalScrollbarBG.w = panel->background.w - panel->horizontalScrollbarBG.h;
	panel->horizontalScrollbarBG.y = panel->background.y + panel->background.h - panel->horizontalScrollbarBG.h;
	panel->horizontalScrollbar.x = panel->horizontalScrollbarBG.x;
	panel->horizontalScrollbar.y = panel->horizontalScrollbarBG.y;
	panel->horizontalScrollbar.w = panel->horizontalScrollbarBG.w;
	panel->horizontalScrollbar.h = panel->horizontalScrollbarBG.h;
	
	panel->isScrolling_vertical = false;
	panel->scroll_dy = 0;
	panel->verticalScrollbar_move_dy = 0;
	panel->y_scroll_offset = 0;

	panel->verticalScrollbarEnabled = false;
	panel->verticalScrollbarBG.y = panel->background.y;
	panel->verticalScrollbarBG.w = panel->titleHeight - 10;
	panel->verticalScrollbarBG.h = panel->background.h - panel->verticalScrollbarBG.w;
	panel->verticalScrollbarBG.x = panel->background.x + panel->background.w - panel->verticalScrollbarBG.w;
	panel->verticalScrollbar.x = panel->verticalScrollbarBG.x;
	panel->verticalScrollbar.y = panel->verticalScrollbarBG.y;
	panel->verticalScrollbar.w = panel->verticalScrollbarBG.w;
	panel->verticalScrollbar.h = panel->verticalScrollbarBG.h;
	
	panel->onResize = onResizeFallback;
	panel->onResize_data = NULL;
	panel->onMove = onMoveFallback;
	panel->onMove_data = NULL;
	panel->onMinimize = onMinimizeFallback;
	panel->onMinimize_data = NULL;
	panel->onMaximize = onMaximizeFallback;
	panel->onMaximize_data = NULL;
	
	panel->controlCount = 0;
	panel->buttonCount = 0;
	panel->checkBoxCount = 0;
	panel->textLabelCount = 0;
	panel->sliderCount = 0;
	panel->textInputBoxCount = 0;
	panel->listBoxCount = 0;
	
	return panel;
}

void SGE_DestroyWindowPanel(SGE_WindowPanel *panel)
{
	if(panel != NULL)
	{
		SGE_DestroyMinimizeButton(panel->minimizeButton);
		SGE_FreeTexture(panel->titleTextImg);
		free(panel);
	}
}

void SGE_WindowPanelHandleEvents(SGE_WindowPanel *panel)
{
	int i = 0;

	SGE_WindowPanel **panels = currentStateControls->panels;
	int panelCount = currentStateControls->panelCount;
	
	if(panel->isMinimizable)
	{
		SGE_MinimizeButtonHandleEvents(panel->minimizeButton);
	}
	
	/* Handle left mouse button click for panel*/
	if(SGE_GetSDLEvent()->type == SDL_MOUSEBUTTONDOWN)
	{
		if(SGE_GetSDLEvent()->button.button == 1)
		{
			/* Set panel as active when clicked */
			if(SGE_MouseInRect(&panel->border))
			{
				SGE_WindowPanel *active = NULL;
				for(i = 0; i < panelCount; i++)
				{
					if(SGE_MouseInRect(&panels[i]->border))
						active = panels[i];
				}
				SGE_SetActiveWindowPanel(active);
			}
			else
			{
				panel->isActive = false;
				printPanelsStr();
			}
			
			/* Move the panel when titleRect is clicked */
			if(SGE_MouseInRect(&panel->titleRect))
			{
				if(panel->isMovable)
				{
					panel->isMoving = true;
					for(i = panel->index + 1; i < panelCount; i++)
					{
						if(SGE_MouseInRect(&panels[i]->border))
							panel->isMoving = false;
					}
					
					panel->move_dx = SGE_GetMouseX() - panel->border.x;
					panel->move_dy = SGE_GetMouseY() - panel->border.y;

					if(panel->isMinimizable)
					{
						if(SGE_MouseInRect(&panel->minimizeButton->boundBox))
						{
							panel->isMoving = false;
						}
					}
				}
			}
			
			/* Resize the panel vertically */
			if(SGE_MouseInRect(&panel->resizeBar_vertical) && !SGE_MouseInRect(&panel->background))
			{
				if(panel->isResizable && !panel->isMinimized)
				{
					panel->isResizing_vertical = true;
					for(i = panel->index + 1; i < panelCount; i++)
					{
						if(SGE_MouseInRect(&panels[i]->border))
							panel->isResizing_vertical = false;
					}
					
					panel->resize_origin_y = SGE_GetMouseY();
					panel->resize_origin_h = panel->border.h;
				}
			}
			
			/* Resize the panel horizontally */
			if(SGE_MouseInRect(&panel->resizeBar_horizontal) && !SGE_MouseInRect(&panel->background))
			{
				if(panel->isResizable && !panel->isMinimized)
				{
					panel->isResizing_horizontal = true;
					for(i = panel->index + 1; i < panelCount; i++)
					{
						if(SGE_MouseInRect(&panels[i]->border))
							panel->isResizing_horizontal = false;
					}
					
					panel->resize_origin_x = SGE_GetMouseX();
					panel->resize_origin_w = panel->border.w;
				}
			}
			
			/* Scroll Horizontally */
			if(panel->horizontalScrollbarEnabled)
			{
				if(SGE_MouseInRect(&panel->horizontalScrollbar))
				{
					panel->isScrolling_horizontal = true;
					for(i = panel->index + 1; i < panelCount; i++)
					{
						if(SGE_MouseInRect(&panels[i]->border))
							panel->isScrolling_horizontal = false;
					}
					panel->horizontalScrollbar_move_dx = SGE_GetMouseX() - panel->horizontalScrollbar.x;
				}
				
				if(SGE_MouseInRect(&panel->horizontalScrollbarBG) && !SGE_MouseInRect(&panel->horizontalScrollbar))
				{
					panel->isScrolling_horizontal = true;
					for(i = panel->index + 1; i < panelCount; i++)
					{
						if(SGE_MouseInRect(&panels[i]->border))
							panel->isScrolling_horizontal = false;
					}
					if(panel->isScrolling_horizontal)
					{
						panel->horizontalScrollbar.x = SGE_GetMouseX() - (panel->horizontalScrollbar.w / 2);
						panel->horizontalScrollbar_move_dx = SGE_GetMouseX() - panel->horizontalScrollbar.x;
					}
				}
			}
			
			/* Scroll Vertically */
			if(panel->verticalScrollbarEnabled)
			{
				if(SGE_MouseInRect(&panel->verticalScrollbar))
				{
					panel->isScrolling_vertical = true;
					for(i = panel->index + 1; i < panelCount; i++)
					{
						if(SGE_MouseInRect(&panels[i]->border))
							panel->isScrolling_vertical = false;
					}
					panel->verticalScrollbar_move_dy = SGE_GetMouseY() - panel->verticalScrollbar.y;
				}
				
				if(SGE_MouseInRect(&panel->verticalScrollbarBG) && !SGE_MouseInRect(&panel->verticalScrollbar))
				{
					panel->isScrolling_vertical = true;
					for(i = panel->index + 1; i < panelCount; i++)
					{
						if(SGE_MouseInRect(&panels[i]->border))
							panel->isScrolling_vertical = false;
					}
					if(panel->isScrolling_vertical)
					{
						panel->verticalScrollbar.y = SGE_GetMouseY() - (panel->verticalScrollbar.h / 2);
						panel->verticalScrollbar_move_dy = SGE_GetMouseY() - panel->verticalScrollbar.y;
					}
				}
			}
		}
	}
	
	else if(SGE_GetSDLEvent()->type == SDL_MOUSEBUTTONUP)
	{
		if(panel->isMoving)
		{
			panel->isMoving = false;
		}
		
		if(panel->isResizing_vertical)
		{
			panel->isResizing_vertical = false;
		}
		
		if(panel->isResizing_horizontal)
		{
			panel->isResizing_horizontal = false;
		}
		
		if(panel->isScrolling_horizontal)
		{
			panel->isScrolling_horizontal = false;
		}
		
		if(panel->isScrolling_vertical)
		{
			panel->isScrolling_vertical = false;
		}
	}
	
	else if(SGE_GetSDLEvent()->type == SDL_MOUSEMOTION)
	{
		if(panel->isMoving)
			panel->onMove(panel->onMove_data);
		if(panel->isResizing_vertical || panel->isResizing_horizontal)
			panel->onResize(panel->onResize_data);
	}
}

void SGE_WindowPanelUpdate(SGE_WindowPanel *panel)
{
	/* Recalculate the window position when moved */
	if(panel->isMoving)
	{
		SGE_WindowPanelSetPosition(panel, SGE_GetMouseX() - panel->move_dx, SGE_GetMouseY() - panel->move_dy);
	}
	
	/* Recalculate the window width when resized horizontally */
	if(panel->isResizing_horizontal)
	{
		panel->isMoving = false;
		panel->border.w = panel->resize_origin_w - (panel->resize_origin_x - SGE_GetMouseX());
		if(panel->border.w < panel->titleTextImg->w + panel->minimizeButton->boundBox.w + 50)
		{
			panel->border.w = panel->titleTextImg->w + panel->minimizeButton->boundBox.w + 50;
		}
		panel->boundBox.w = panel->border.w;
		
		panel->resizeBar_horizontal.x = panel->border.x + panel->border.w - panel->resizeBar_horizontal.w;
		panel->resizeBar_vertical.x = panel->border.x + panel->border.w - panel->resizeBar_vertical.w;
		
		panel->background.w = panel->border.w - (2 * panel->borderThickness);
		panel->titleRect.w = panel->border.w;
		panel->titleTextImg->x = panel->titleRect.x + (panel->titleRect.w / 2) - (panel->titleTextImg->w / 2);
		
		panel->bgLocalCenter.x = panel->background.w / 2;
		panel->bgGlobalCenter.x = panel->bgLocalCenter.x + panel->background.x;
		
		panel->verticalScrollbarBG.x = panel->background.x + panel->background.w - panel->verticalScrollbarBG.w;
		panel->verticalScrollbar.x = panel->verticalScrollbarBG.x;
		
		SGE_WindowPanelShouldEnableHorizontalScroll(panel);
		SGE_WindowPanelShouldEnableVerticalScroll(panel);
	}
	
	/* Recalculate the window height when resized vertically */
	if(panel->isResizing_vertical)
	{
		panel->isMoving = false;
		panel->border.h = panel->resize_origin_h - (panel->resize_origin_y - SGE_GetMouseY());
		if(panel->border.h < panel->titleRect.h + panel->horizontalScrollbarBG.h)
		{
			panel->border.h = panel->titleRect.h + panel->horizontalScrollbarBG.h;
		}
		
		panel->boundBox.h = panel->border.h;
		panel->background.h = panel->border.h - (2 * panel->borderThickness) - panel->titleHeight + panel->borderThickness;
		panel->resizeBar_horizontal.y = panel->border.y + panel->border.h - panel->resizeBar_horizontal.h;
		panel->resizeBar_vertical.y = panel->border.y + panel->border.h - panel->resizeBar_vertical.h;
		
		panel->titleRect.h = panel->titleHeight + panel->borderThickness / 2;
		panel->titleTextImg->y = panel->titleRect.y + (panel->titleRect.h / 2) - (panel->titleTextImg->h / 2);
		
		panel->bgLocalCenter.y = panel->background.h / 2;
		panel->bgGlobalCenter.y = panel->bgLocalCenter.y + panel->background.y;
		
		panel->horizontalScrollbarBG.y = panel->background.y + panel->background.h - panel->horizontalScrollbarBG.h;
		panel->horizontalScrollbar.y = panel->horizontalScrollbarBG.y;
		
		SGE_WindowPanelShouldEnableHorizontalScroll(panel);
		SGE_WindowPanelShouldEnableVerticalScroll(panel);
	}
	
	if(panel->isScrolling_horizontal)
	{
		panel->horizontalScrollbar.x = SGE_GetMouseX() - panel->horizontalScrollbar_move_dx;
		if(panel->horizontalScrollbar.x < panel->horizontalScrollbarBG.x)
			panel->horizontalScrollbar.x = panel->horizontalScrollbarBG.x;
		if(panel->horizontalScrollbar.x + panel->horizontalScrollbar.w > panel->horizontalScrollbarBG.x + panel->horizontalScrollbarBG.w)
			panel->horizontalScrollbar.x = panel->horizontalScrollbarBG.x + panel->horizontalScrollbarBG.w - panel->horizontalScrollbar.w;
		
		int temp_w = panel->masterControlRect.w + (panel->masterControlRect.x - panel->background.x);
		panel->x_scroll_offset = ((panel->horizontalScrollbarBG.x - panel->horizontalScrollbar.x) / (double)panel->horizontalScrollbarBG.w) * temp_w;
	}
	
	if(panel->isScrolling_vertical)
	{
		panel->verticalScrollbar.y = SGE_GetMouseY() - panel->verticalScrollbar_move_dy;
		if(panel->verticalScrollbar.y < panel->verticalScrollbarBG.y)
			panel->verticalScrollbar.y = panel->verticalScrollbarBG.y;
		if(panel->verticalScrollbar.y + panel->verticalScrollbar.h > panel->verticalScrollbarBG.y + panel->verticalScrollbarBG.h)
			panel->verticalScrollbar.y = panel->verticalScrollbarBG.y + panel->verticalScrollbarBG.h - panel->verticalScrollbar.h;
		
		int temp_h = panel->masterControlRect.h + (panel->masterControlRect.y - panel->background.y);
		panel->y_scroll_offset = ((panel->verticalScrollbarBG.y - panel->verticalScrollbar.y) / (double)panel->verticalScrollbarBG.h) * temp_h;
	}
	
	if(panel->isMinimizable)
	{
		SGE_MinimizeButtonUpdate(panel->minimizeButton);
	}
}

void SGE_WindowPanelRender(SGE_WindowPanel *panel)
{
	int i = 0;
	
	/* Draw a rect that acts as a border and title bar */
	SGE_SetDrawBlendMode(SDL_BLENDMODE_BLEND);
	SGE_SetDrawColorRGBA(panel->borderColor.r, panel->borderColor.g, panel->borderColor.b, panel->alpha);
	SGE_DrawFillRect(&panel->border);
	
	/* Draw a white or black border around the panel */
	if(panel->isActive)
	{
		SGE_SetDrawColorRGBA(255, 255, 255, panel->alpha);
	}
	else
	{
		SGE_SetDrawColorRGBA(0, 0, 0, panel->alpha);
	}
	SGE_DrawRect(&panel->border);
	
	/* Draw the actual background of the panel */
	SGE_SetDrawColorRGBA(panel->backgroundColor.r, panel->backgroundColor.g, panel->backgroundColor.b, panel->alpha);
	SGE_DrawFillRect(&panel->background);
	
	/* Draw the panel title text */
	SGE_SetTextureAlpha(panel->titleTextImg, panel->alpha);
	SGE_RenderTexture(panel->titleTextImg);
	
	if(panel->isMinimizable)
	{
		SGE_MinimizeButtonRender(panel->minimizeButton);
	}
	
	/* Draw all the child controls */
	SGE_SetDrawClipRect(&panel->background);
	
	for(i = 0; i < panel->buttonCount; i++)
	{
		SGE_ButtonRender(panel->buttons[i]);
	}
	
	for(i = 0; i < panel->checkBoxCount; i++)
	{
		SGE_CheckBoxRender(panel->checkBoxes[i]);
	}
	
	for(i = 0; i < panel->textLabelCount; i++)
	{
		SGE_TextLabelRender(panel->textLabels[i]);
	}

	for(i = 0; i < panel->sliderCount; i++)
	{
		SGE_SliderRender(panel->sliders[i]);
	}
	
	for(i = 0; i < panel->textInputBoxCount; i++)
	{
		SGE_TextInputBoxRender(panel->textInputBoxes[i]);
	}
	
	for(i = 0; i < panel->listBoxCount; i++)
	{
		SGE_ListBoxRender(panel->listBoxes[i]);
	}
	
	SGE_SetDrawClipRect(NULL);
	SGE_SetDrawColorRGBA(255, 255, 255, 0);
	SGE_DrawPoint(0, 0);
	
	/* Draw Horizontal Scrollbar */
	if(panel->horizontalScrollbarEnabled)
	{
		SGE_SetDrawColorRGBA(255, 255, 255, panel->alpha);
		SGE_DrawFillRect(&panel->horizontalScrollbarBG);
		SGE_SetDrawColorRGBA(0, 0, 0, panel->alpha);
		SGE_DrawRect(&panel->horizontalScrollbarBG);
		
		SGE_SetDrawColorRGBA(panel->borderColor.r, panel->borderColor.g, panel->borderColor.b, panel->alpha);
		SGE_DrawFillRect(&panel->horizontalScrollbar);
		
		SGE_SetDrawColorRGBA(0, 0, 0, panel->alpha);
		if(SGE_MouseInRect(&panel->horizontalScrollbar))
		{
			SGE_SetDrawColorRGBA(225, 225, 225, panel->alpha);
			
			for(i = panel->index + 1; i < currentStateControls->panelCount; i++)
			{
				if(SGE_MouseInRect(&currentStateControls->panels[i]->border))
					SGE_SetDrawColorRGBA(0, 0, 0, panel->alpha);
			}
		}
		SGE_DrawRect(&panel->horizontalScrollbar);
	}
	
	/* Draw Vertical Scrollbar */
	if(panel->verticalScrollbarEnabled)
	{
		SGE_SetDrawColorRGBA(255, 255, 255, panel->alpha);
		SGE_DrawFillRect(&panel->verticalScrollbarBG);
		SGE_SetDrawColorRGBA(0, 0, 0, panel->alpha);
		SGE_DrawRect(&panel->verticalScrollbarBG);
		
		SGE_SetDrawColorRGBA(panel->borderColor.r, panel->borderColor.g, panel->borderColor.b, panel->alpha);
		SGE_DrawFillRect(&panel->verticalScrollbar);
		
		SGE_SetDrawColorRGBA(0, 0, 0, panel->alpha);
		if(SGE_MouseInRect(&panel->verticalScrollbar))
		{
			SGE_SetDrawColorRGBA(225, 225, 225, panel->alpha);
			
			for(i = panel->index + 1; i < currentStateControls->panelCount; i++)
			{
				if(SGE_MouseInRect(&currentStateControls->panels[i]->border))
					SGE_SetDrawColorRGBA(0, 0, 0, panel->alpha);
			}
		}
		SGE_DrawRect(&panel->verticalScrollbar);
	}
	
	if(showControlBounds)
	{
		/* Draw Resize Control Bars */
		SGE_SetDrawColorRGBA(255, 0, 255, panel->alpha);
		SGE_DrawRect(&panel->resizeBar_horizontal);
		SGE_SetDrawColorRGBA(0, 255, 0, panel->alpha);
		SGE_DrawRect(&panel->resizeBar_vertical);
		
		/* Draw the panel center point */
		SDL_Rect centerRect = {panel->bgGlobalCenter.x - 2, panel->bgGlobalCenter.y - 2, 4, 4};
		SGE_SetDrawColorRGBA(255, 255, 255, panel->alpha);
		SGE_DrawFillRect(&centerRect);
		SGE_SetDrawColorRGBA(0, 0, 0, panel->alpha);
		SGE_DrawRect(&centerRect);
		
		/* Draw panel MCR */
		SGE_SetDrawColorRGBA(0, 255, 0, panel->alpha);
		SGE_DrawRect(&panel->masterControlRect);
		
		/* Draw panel boundbox */
		SGE_SetDrawColorRGBA(255, 0, 255, panel->alpha);
		SGE_DrawRect(&panel->boundBox);
	}
}

void SGE_WindowPanelSetPosition(SGE_WindowPanel *panel, int x, int y)
{
	/* Store the difference between new and old positions */
	int dx = x - panel->boundBox.x;
	int dy = y - panel->boundBox.y;
	
	panel->boundBox.x = x;
	panel->boundBox.y = y;
	panel->border.x = panel->boundBox.x;
	panel->border.y = panel->boundBox.y;
	panel->background.x = panel->border.x + panel->borderThickness;
	panel->background.y = panel->border.y + panel->borderThickness + panel->titleHeight - panel->borderThickness;
	panel->titleRect.x = panel->border.x;
	panel->titleRect.y = panel->border.y;
	panel->titleTextImg->x = panel->titleRect.x + (panel->titleRect.w / 2) - (panel->titleTextImg->w / 2);
	panel->titleTextImg->y = panel->titleRect.y + (panel->titleRect.h / 2) - (panel->titleTextImg->h / 2);
	
	panel->resizeBar_horizontal.x = panel->border.x + panel->border.w - panel->resizeBar_horizontal.w;
	panel->resizeBar_horizontal.y = panel->border.y + panel->border.h - panel->resizeBar_horizontal.h;
	panel->resizeBar_vertical.x = panel->border.x + panel->border.w - panel->resizeBar_vertical.w;
	panel->resizeBar_vertical.y = panel->border.y + panel->border.h - panel->resizeBar_vertical.h;
	
	panel->bgLocalCenter.x = panel->background.w / 2;
	panel->bgLocalCenter.y = panel->background.h / 2;
	panel->bgGlobalCenter.x = panel->bgLocalCenter.x + panel->background.x;
	panel->bgGlobalCenter.y = panel->bgLocalCenter.y + panel->background.y;
	
	panel->minimizeButton->boundBox.x = panel->background.x;
	panel->minimizeButton->boundBox.y = panel->titleRect.y + (panel->titleRect.h / 2) - (panel->minimizeButton->boundBox.h / 2);
	panel->minimizeButton->buttonImg->x = panel->minimizeButton->boundBox.x + (panel->minimizeButton->boundBox.w / 2) - (panel->minimizeButton->buttonImg->w / 2);
	panel->minimizeButton->buttonImg->y = panel->minimizeButton->boundBox.y + (panel->minimizeButton->boundBox.h / 2) - (panel->minimizeButton->buttonImg->h / 2);
	
	panel->masterControlRect.x += dx;
	panel->masterControlRect.y += dy;
	
	panel->horizontalScrollbarBG.x = panel->background.x;
	panel->horizontalScrollbarBG.y = panel->background.y + panel->background.h - panel->horizontalScrollbarBG.h;
	panel->horizontalScrollbar.y = panel->horizontalScrollbarBG.y;

	/* Calculate horizontal scrollbar position based on scroll offset */
	int temp_w = panel->masterControlRect.w + (panel->masterControlRect.x - panel->background.x);
	double scrlBarPos = panel->horizontalScrollbarBG.x - ((panel->x_scroll_offset / temp_w) * panel->horizontalScrollbarBG.w);
	/* Must round result and cast to int */
	panel->horizontalScrollbar.x = SDL_ceil(scrlBarPos);

	panel->verticalScrollbarBG.y = panel->background.y;
	panel->verticalScrollbarBG.x = panel->background.x + panel->background.w - panel->verticalScrollbarBG.w;
	panel->verticalScrollbar.x = panel->verticalScrollbarBG.x;

	/* Calculate vertical scrollbar posiotion based on scroll offset */
	int temp_h = panel->masterControlRect.h + (panel->masterControlRect.y - panel->background.y);
	scrlBarPos = panel->verticalScrollbarBG.y - ((panel->y_scroll_offset / (double)temp_h) * panel->verticalScrollbarBG.h);	
	panel->verticalScrollbar.y = SDL_ceil(scrlBarPos);
}

void SGE_WindowPanelSetSize(SGE_WindowPanel *panel, int w, int h)
{
	panel->background.w = w;
	panel->background.h = h; 
	
	panel->border.w = panel->background.w + (2 * panel->borderThickness);
	panel->border.h = panel->background.h + (2 * panel->borderThickness)  + panel->titleHeight - panel->borderThickness;
	panel->boundBox.w = panel->border.w;
	panel->boundBox.h = panel->border.h;
	
	if(panel->border.w < panel->titleTextImg->w + panel->minimizeButton->boundBox.w + 50)
	{
		panel->border.w = panel->titleTextImg->w + panel->minimizeButton->boundBox.w + 50;
	}
	
	if(panel->border.h < panel->titleRect.h + panel->horizontalScrollbarBG.h)
	{
		panel->border.h = panel->titleRect.h + panel->horizontalScrollbarBG.h;
	}
	
	panel->titleRect.w = panel->border.w;
	panel->titleRect.h = panel->titleHeight + panel->borderThickness / 2;
	panel->titleTextImg->x = panel->titleRect.x + (panel->titleRect.w / 2) - (panel->titleTextImg->w / 2);
	panel->titleTextImg->y = panel->titleRect.y + (panel->titleRect.h / 2) - (panel->titleTextImg->h / 2);
	
	panel->bgLocalCenter.x = panel->background.w / 2;
	panel->bgLocalCenter.y = panel->background.h / 2;
	panel->bgGlobalCenter.x = panel->bgLocalCenter.x + panel->background.x;
	panel->bgGlobalCenter.y = panel->bgLocalCenter.y + panel->background.y;
	
	panel->resizeBar_horizontal.x = panel->border.x + panel->border.w - panel->resizeBar_horizontal.w;
	panel->resizeBar_horizontal.y = panel->border.y + panel->border.h - panel->resizeBar_horizontal.h;
	panel->resizeBar_vertical.x = panel->border.x + panel->border.w - panel->resizeBar_vertical.w;
	panel->resizeBar_vertical.y = panel->border.y + panel->border.h - panel->resizeBar_vertical.h;	
	
	panel->horizontalScrollbarBG.x = panel->background.x;
	panel->horizontalScrollbarBG.y = panel->background.y + panel->background.h;
	panel->horizontalScrollbarBG.w = panel->background.w;
	panel->horizontalScrollbar.x = panel->horizontalScrollbarBG.x - (panel->x_scroll_offset * (panel->background.w / (double)panel->masterControlRect.w));
	panel->horizontalScrollbar.y = panel->horizontalScrollbarBG.y;
	
	panel->verticalScrollbarBG.x = panel->background.x + panel->background.w - panel->verticalScrollbarBG.w;
	panel->verticalScrollbarBG.y = panel->background.y;
	panel->verticalScrollbarBG.h = panel->background.h - panel->verticalScrollbarBG.w;
	panel->verticalScrollbar.x = panel->verticalScrollbarBG.x;
	panel->verticalScrollbar.y = panel->verticalScrollbarBG.y;
}

void SGE_SetActiveWindowPanel(SGE_WindowPanel *panel)
{
	int i = 0;
	
	/* Sets all but the given panel as active */
	for(i = 0; i < currentStateControls->panelCount; i++)
	{
		currentStateControls->panels[i]->isActive = false;
	}
	panel->isActive = true;
	SGE_SendActivePanelToTop();
}

SGE_WindowPanel *SGE_GetActiveWindowPanel()
{
	return currentStateControls->panels[currentStateControls->panelCount - 1];
}

void SGE_SendActivePanelToTop()
{
	int i = 0;
	SGE_WindowPanel *tempPanel = NULL;

	SGE_WindowPanel **panels = currentStateControls->panels;
	int panelCount = currentStateControls->panelCount;
	
	/* Finds the active panel and sends it to the top of the panels stack */
	for(i = 0; i < panelCount - 1; i++)
	{
		if(panels[i]->isActive)
		{
			//SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Changing active Panel to %s", panels[i]->titleStr);
			
			/* Store the active panel in a temp location */
			tempPanel = panels[i];
			
			/* Send everything after the active panel index down one index to make space at the top of the stack */
			int j = 0;
			for(j = i; j < panelCount; j++)
			{
				//printPanels();
				panels[j] = panels[j + 1];
			}
			
			/* Send the active panel to the top */
			panels[panelCount - 1] = tempPanel;
			tempPanel = NULL;
			
			//printPanels();
			SGE_GUI_LogPrintLine(SGE_LOG_DEBUG, "Active Panel changed to %s", panels[panelCount - 1]->titleStr);
			
			/* Stop searching since the active panel has been found */
			break;
		}
	}
	
	/* Reindex panels */
	for(i = 0; i < panelCount; i++)
	{
		panels[i]->index = i;
	}
	
	printPanelsStr();
}

void SGE_WindowPanelToggleMinimized(SGE_WindowPanel *panel)
{
	if(panel->isMinimized)
	{
		panel->isMinimized = false;
		panel->minimizeButton->buttonImg->rotation = 0;
		
		panel->border.w = panel->temp_border_w;
		panel->border.h = panel->temp_border_h;
		
		panel->background.w = panel->temp_background_w;
		panel->background.h = panel->temp_background_h;
		
		panel->resizeBar_horizontal.x = panel->border.x + panel->border.w - panel->resizeBar_horizontal.w;
		panel->resizeBar_horizontal.y = panel->border.y + panel->border.h - panel->resizeBar_horizontal.h;
		panel->resizeBar_vertical.x = panel->border.x + panel->border.w - panel->resizeBar_vertical.w;
		panel->resizeBar_vertical.y = panel->border.y + panel->border.h - panel->resizeBar_vertical.h;
		
		panel->bgLocalCenter.x = panel->background.w / 2;
		panel->bgLocalCenter.y = panel->background.h / 2;
		panel->bgGlobalCenter.x = panel->bgLocalCenter.x + panel->background.x;
		panel->bgGlobalCenter.y = panel->bgLocalCenter.y + panel->background.y;
		
		panel->horizontalScrollbarEnabled = panel->temp_horizontalScrollbarEnabled;
		panel->horizontalScrollbarBG.x = panel->background.x;
		panel->horizontalScrollbarBG.y = panel->background.y + panel->background.h - panel->horizontalScrollbarBG.h;
		panel->horizontalScrollbar.y = panel->horizontalScrollbarBG.y;
		
		panel->verticalScrollbarEnabled = panel->temp_verticalScrollbarEnabled;
		panel->verticalScrollbarBG.y = panel->background.y;
		panel->verticalScrollbarBG.x = panel->background.x + panel->background.w - panel->verticalScrollbarBG.w;
		panel->verticalScrollbar.x = panel->verticalScrollbarBG.x;
		
		panel->onMaximize(panel->onMaximize_data);
	}
	else
	{
		panel->isMinimized = true;
		panel->minimizeButton->buttonImg->rotation = -90;
		
		panel->temp_border_w = panel->border.w;
		panel->temp_border_h = panel->border.h;
		panel->temp_background_w = panel->background.w;
		panel->temp_background_h = panel->background.h;
		panel->temp_horizontalScrollbarEnabled = panel->horizontalScrollbarEnabled;
		panel->temp_verticalScrollbarEnabled = panel->verticalScrollbarEnabled;
		
		panel->border.w = panel->titleRect.w;
		panel->border.h = panel->titleRect.h;
		
		panel->background.h = 0;
		
		panel->resizeBar_horizontal.x = panel->border.x + panel->border.w - panel->resizeBar_horizontal.w;
		panel->resizeBar_horizontal.y = panel->border.y + panel->border.h - panel->resizeBar_horizontal.h;
		panel->resizeBar_vertical.x = panel->border.x + panel->border.w - panel->resizeBar_vertical.w;
		panel->resizeBar_vertical.y = panel->border.y + panel->border.h - panel->resizeBar_vertical.h;
		
		panel->bgLocalCenter.x = panel->background.w / 2;
		panel->bgLocalCenter.y = panel->background.h / 2;
		panel->bgGlobalCenter.x = panel->bgLocalCenter.x + panel->background.x;
		panel->bgGlobalCenter.y = panel->bgLocalCenter.y + panel->background.y;
		
		panel->horizontalScrollbarEnabled = false;
		panel->horizontalScrollbarBG.x = panel->background.x;
		panel->horizontalScrollbarBG.y = panel->background.y + panel->background.h - panel->horizontalScrollbarBG.h;
		panel->horizontalScrollbar.y = panel->horizontalScrollbarBG.y;
		
		panel->verticalScrollbarEnabled = false;
		panel->verticalScrollbarBG.y = panel->background.y;
		panel->verticalScrollbarBG.x = panel->background.x + panel->background.w - panel->verticalScrollbarBG.w;
		panel->verticalScrollbar.x = panel->verticalScrollbarBG.x;
		
		panel->onMinimize(panel->onMinimize_data);
	}
}

/* Called whenever a control is added to a panel or when a control changes position */
void SGE_WindowPanelCalculateMCR(SGE_WindowPanel *panel, SDL_Rect boundBox)
{
	if(panel->controlCount == 1)
	{
		panel->masterControlRect = boundBox;
	}
	else
	{
		if(boundBox.x < panel->masterControlRect.x)
		{
			panel->masterControlRect.w += panel->masterControlRect.x - boundBox.x;
			panel->masterControlRect.x = boundBox.x;
		}
		
		if(boundBox.x + boundBox.w > panel->masterControlRect.x + panel->masterControlRect.w)
			panel->masterControlRect.w += (boundBox.x + boundBox.w) - (panel->masterControlRect.x + panel->masterControlRect.w);
		
		if(boundBox.y < panel->masterControlRect.y)
		{
			panel->masterControlRect.h += panel->masterControlRect.y - boundBox.y;
			panel->masterControlRect.y = boundBox.y;
		}
		
		if(boundBox.y + boundBox.h > panel->masterControlRect.y + panel->masterControlRect.h)
			panel->masterControlRect.h += (boundBox.y + boundBox.h) - (panel->masterControlRect.y + panel->masterControlRect.h);
	}
	
	SGE_WindowPanelShouldEnableHorizontalScroll(panel);
	SGE_WindowPanelShouldEnableVerticalScroll(panel);
}

void SGE_WindowPanelShouldEnableHorizontalScroll(SGE_WindowPanel *panel)
{
	/* Check if horizontal scrollbar should appear */
	if(panel->masterControlRect.x + panel->masterControlRect.w > panel->background.x + panel->background.w)
	{
		panel->scroll_dx = (panel->masterControlRect.x + panel->masterControlRect.w) - (panel->background.x + panel->background.w);
		panel->horizontalScrollbarEnabled = true;

		if(panel->verticalScrollbarEnabled)
			panel->horizontalScrollbarBG.w = panel->background.w - panel->horizontalScrollbarBG.h;
		else
			panel->horizontalScrollbarBG.w = panel->background.w;
		
		/* Calculate scrollbar width */
		int temp_w = panel->masterControlRect.w + (panel->masterControlRect.x - panel->background.x);
		double scrBarWidth = panel->horizontalScrollbarBG.w - ((panel->scroll_dx / (double)temp_w) * panel->horizontalScrollbarBG.w);
		panel->horizontalScrollbar.w = SDL_ceil(scrBarWidth);

		/* Calculate scrollbar x position */
		double scrlBarPos = panel->horizontalScrollbarBG.x - ((panel->x_scroll_offset / temp_w) * panel->horizontalScrollbarBG.w);
		panel->horizontalScrollbar.x = SDL_ceil(scrlBarPos);

		if((panel->horizontalScrollbar.x + panel->horizontalScrollbar.w) > (panel->horizontalScrollbarBG.x + panel->horizontalScrollbarBG.w))
		{
			panel->isScrolling_horizontal = true;
		}
	}
	else
	{
		panel->x_scroll_offset = 0;
		panel->horizontalScrollbar.x = panel->horizontalScrollbarBG.x;
		panel->horizontalScrollbarEnabled = false;
	}
}

void SGE_WindowPanelShouldEnableVerticalScroll(SGE_WindowPanel *panel)
{
	/* Check if vertical scrollbar should appear */
	if(panel->masterControlRect.y + panel->masterControlRect.h > panel->background.y + panel->background.h)
	{
		panel->scroll_dy = (panel->masterControlRect.y + panel->masterControlRect.h) - (panel->background.y + panel->background.h);
		panel->verticalScrollbarEnabled = true;

		if(panel->horizontalScrollbarEnabled)
			panel->verticalScrollbarBG.h = panel->background.h - panel->verticalScrollbarBG.w;
		else
			panel->verticalScrollbarBG.h = panel->background.h;
		
		/* Calculate vertical scrollbar width */
		int temp_h = panel->masterControlRect.h + (panel->masterControlRect.y - panel->background.y);
		double scrBarWidth = panel->verticalScrollbarBG.h - ((panel->scroll_dy / (double)temp_h) * panel->verticalScrollbarBG.h);
		panel->verticalScrollbar.h = SDL_ceil(scrBarWidth);

		/* Calculate vertical scrollbar y position */
		double scrlBarPos = panel->verticalScrollbarBG.y - ((panel->y_scroll_offset / temp_h) * panel->verticalScrollbarBG.h);
		panel->verticalScrollbar.y = SDL_ceil(scrlBarPos);

		if((panel->verticalScrollbar.y + panel->verticalScrollbar.h) > (panel->verticalScrollbarBG.y + panel->verticalScrollbarBG.h))
		{
			panel->isScrolling_vertical = true;
		}
	}
	else
	{
		panel->y_scroll_offset = 0;
		panel->verticalScrollbar.y = panel->verticalScrollbarBG.y;
		panel->verticalScrollbarEnabled = false;
	}
}

/* Layouting functions for easy GUI Layouting */

/**
 * \brief Calculates a position for a given control based on another control.
 * 
 * \param controlBoundBox The bounds of the control for which to calculate position.
 * \param targetBoundBox  The bounds of the control used as a target for the new position.
 * \param direction       The direction to place the new position in relative to the target.
 * \param spacing_x       The horizontal offset for the new position.
 * \param spacing_y       The vertical offset for the new position.
 * \return SDL_Point 
 */
SDL_Point SGE_ControlGetPositionNextTo(SDL_Rect controlBoundBox, SDL_Rect targetBoundBox, SGE_ControlDirection direction, int spacing_x, int spacing_y)
{
	SDL_Point position = {0, 0};
	
	switch(direction)
	{
		case SGE_CONTROL_DIRECTION_UP:
		position.x = targetBoundBox.x + spacing_x;
		position.y = targetBoundBox.y - controlBoundBox.h - spacing_y;
		break;
		
		case SGE_CONTROL_DIRECTION_UP_CENTERED:
		position.x = targetBoundBox.x + (targetBoundBox.w / 2) - (controlBoundBox.w / 2) + spacing_x;
		position.y = targetBoundBox.y - controlBoundBox.h - spacing_y;
		break;
		
		case SGE_CONTROL_DIRECTION_DOWN:
		position.x = targetBoundBox.x + spacing_x;
		position.y = targetBoundBox.y + targetBoundBox.h + spacing_y;
		break;
		
		case SGE_CONTROL_DIRECTION_DOWN_CENTERED:
		position.x = targetBoundBox.x + (targetBoundBox.w / 2) - (controlBoundBox.w / 2) + spacing_x;
		position.y = targetBoundBox.y + targetBoundBox.h + spacing_y;
		break;
		
		case SGE_CONTROL_DIRECTION_LEFT:
		position.x = targetBoundBox.x - controlBoundBox.w - spacing_x;
		position.y = targetBoundBox.y + spacing_y;
		break;
		
		case SGE_CONTROL_DIRECTION_LEFT_CENTERED:
		position.x = targetBoundBox.x - controlBoundBox.w - spacing_x;
		position.y = targetBoundBox.y + (targetBoundBox.h / 2) - (controlBoundBox.h / 2) + spacing_y;
		break;
		
		case SGE_CONTROL_DIRECTION_RIGHT:
		position.x = targetBoundBox.x + targetBoundBox.w + spacing_x;
		position.y = targetBoundBox.y + spacing_y;
		break;
		
		case SGE_CONTROL_DIRECTION_RIGHT_CENTERED:
		position.x = targetBoundBox.x + targetBoundBox.w + spacing_x;
		position.y = targetBoundBox.y + (targetBoundBox.h / 2) - (controlBoundBox.h / 2) + spacing_y;
		break;
	}
	
	return position;
}

void SGE_ButtonSetPositionNextTo(SGE_Button *button, SDL_Rect targetBoundBox, SGE_ControlDirection direction, int spacing_x, int spacing_y)
{
	SDL_Point position = SGE_ControlGetPositionNextTo(button->boundBox, targetBoundBox, direction, spacing_x, spacing_y);
	if(button->parentPanel != NULL)
	{
		position.x -= button->parentPanel->background.x;
		position.y -= button->parentPanel->background.y;
	}
	SGE_ButtonSetPosition(button, position.x, position.y);
}

void SGE_CheckBoxSetPositionNextTo(SGE_CheckBox *checkBox, SDL_Rect targetBoundBox, SGE_ControlDirection direction, int spacing_x, int spacing_y)
{
	SDL_Point position = SGE_ControlGetPositionNextTo(checkBox->boundBox, targetBoundBox, direction, spacing_x, spacing_y);
	if(checkBox->parentPanel != NULL)
	{
		position.x -= checkBox->parentPanel->background.x;
		position.y -= checkBox->parentPanel->background.y;
	}
	SGE_CheckBoxSetPosition(checkBox, position.x, position.y);
}

void SGE_TextLabelSetPositionNextTo(SGE_TextLabel *label, SDL_Rect targetBoundBox, SGE_ControlDirection direction, int spacing_x, int spacing_y)
{
	SDL_Point position = SGE_ControlGetPositionNextTo(label->boundBox, targetBoundBox, direction, spacing_x, spacing_y);
	if(label->parentPanel != NULL)
	{
		position.x -= label->parentPanel->background.x;
		position.y -= label->parentPanel->background.y;
	}
	SGE_TextLabelSetPosition(label, position.x, position.y);
}

void SGE_SliderSetPositionNextTo(SGE_Slider *slider, SDL_Rect targetBoundBox, SGE_ControlDirection direction, int spacing_x, int spacing_y)
{
	SDL_Point position = SGE_ControlGetPositionNextTo(slider->boundBox, targetBoundBox, direction, spacing_x, spacing_y);
	if(slider->parentPanel != NULL)
	{
		position.x -= slider->parentPanel->background.x;
		position.y -= slider->parentPanel->background.y;
	}
	SGE_SliderSetPosition(slider, position.x, position.y);
}

void SGE_TextInputBoxSetPositionNextTo(SGE_TextInputBox *textInputBox, SDL_Rect targetBoundBox, SGE_ControlDirection direction, int spacing_x, int spacing_y)
{
	SDL_Point position = SGE_ControlGetPositionNextTo(textInputBox->boundBox, targetBoundBox, direction, spacing_x, spacing_y);
	if(textInputBox->parentPanel != NULL)
	{
		position.x -= textInputBox->parentPanel->background.x;
		position.y -= textInputBox->parentPanel->background.y;
	}
	SGE_TextInputBoxSetPosition(textInputBox, position.x, position.y);
}

void SGE_ListBoxSetPositionNextTo(SGE_ListBox *listBox, SDL_Rect targetBoundBox, SGE_ControlDirection direction, int spacing_x, int spacing_y)
{
	SDL_Point position = SGE_ControlGetPositionNextTo(listBox->boundBox, targetBoundBox, direction, spacing_x, spacing_y);
	if(listBox->parentPanel != NULL)
	{
		position.x -= listBox->parentPanel->background.x;
		position.y -= listBox->parentPanel->background.y;
	}
	SGE_ListBoxSetPosition(listBox, position.x, position.y);
}

void SGE_WindowPanelSetPositionNextTo(SGE_WindowPanel *panel, SDL_Rect targetBoundBox, SGE_ControlDirection direction, int spacing_x, int spacing_y)
{
	SDL_Point position = SGE_ControlGetPositionNextTo(panel->border, targetBoundBox, direction, spacing_x, spacing_y);
	SGE_WindowPanelSetPosition(panel, position.x, position.y);
}
