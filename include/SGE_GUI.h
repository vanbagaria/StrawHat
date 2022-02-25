#ifndef __SGE_GUI_H__
#define __SGE_GUI_H__

#include "SGE_Graphics.h"
#include "SGE_Containers.h"
#include <stdbool.h>

typedef struct SGE_WindowPanel    SGE_WindowPanel;
typedef struct SGE_Button         SGE_Button;
typedef struct SGE_CheckBox       SGE_CheckBox;
typedef struct SGE_TextLabel      SGE_TextLabel;
typedef struct SGE_Slider         SGE_Slider;
typedef struct SGE_TextInputBox   SGE_TextInputBox;
typedef struct SGE_ListBox        SGE_ListBox;

#define STATE_MAX_BUTTONS          50
#define STATE_MAX_CHECKBOXES       50
#define STATE_MAX_LABELS           50
#define STATE_MAX_SLIDERS          50
#define STATE_MAX_TEXT_INPUT_BOXES 50
#define STATE_MAX_LISTBOXES        50
#define STATE_MAX_PANELS           50

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

typedef enum
{
	SGE_CONTROL_STATE_NORMAL,
	SGE_CONTROL_STATE_HOVER,
	SGE_CONTROL_STATE_CLICKED
} SGE_ControlState;

typedef enum
{
	SGE_CONTROL_DIRECTION_UP,
	SGE_CONTROL_DIRECTION_DOWN,
	SGE_CONTROL_DIRECTION_LEFT,
	SGE_CONTROL_DIRECTION_RIGHT,
	SGE_CONTROL_DIRECTION_UP_CENTERED,
	SGE_CONTROL_DIRECTION_DOWN_CENTERED,
	SGE_CONTROL_DIRECTION_LEFT_CENTERED,
	SGE_CONTROL_DIRECTION_RIGHT_CENTERED
} SGE_ControlDirection;

typedef struct SGE_Button
{
	int x, y;
	SDL_Rect boundBox;
	struct SGE_WindowPanel *parentPanel;
	Uint8 alpha;
	SGE_ControlState state;

	SDL_Color normalColor;
	SDL_Color hoverColor;
	SDL_Color clickedColor;
	SDL_Color currentColor;
	SGE_Texture *textImg;
	SDL_Rect background;
	
	void (*onMouseDown)(void *data);
	void *onMouseDown_data;
	void (*onMouseUp)(void *data);
	void *onMouseUp_data;
} SGE_Button;

typedef struct SGE_CheckBox
{
	int x, y;
	SDL_Rect boundBox;
	struct SGE_WindowPanel *parentPanel;
	Uint8 alpha;
	SGE_ControlState state;
	
	int size;
	SDL_Rect bg;
	SDL_Rect check;
	SDL_Color checkColor;
	bool isChecked;
	
	void (*onMouseDown)(void *data);
	void *onMouseDown_data;
	void (*onMouseUp)(void *data);
	void *onMouseUp_data;
} SGE_CheckBox;

typedef struct SGE_TextLabel
{
	int x, y;
	SDL_Rect boundBox;
	struct SGE_WindowPanel *parentPanel;
	Uint8 alpha;
	
	char text[200]; // TODO: Change to dynamic string
	TTF_Font *font;
	SDL_Color fgColor;
	SDL_Color bgColor;
	bool showBG;
	SGE_Texture *textImg;
	SGE_TextRenderMode mode;
	bool isVisible;
} SGE_TextLabel;

typedef struct SGE_Slider
{
	int x, y;
	SDL_Rect boundBox;
	struct SGE_WindowPanel *parentPanel;
	Uint8 alpha;
	SGE_ControlState state;
	
	SDL_Rect bar;
	SDL_Color barColor;
	SDL_Rect slider;
	SDL_Color sliderColor;
	double slider_xi;
	double value; // Between 0 and 1;
	double value_i; // Same as above, but used internally with offset.
	double x_offset; // Between 0 and 1;
	int move_dx;
	
	void (*onMouseDown)(void *data);
	void *onMouseDown_data;
	void (*onMouseUp)(void *data);
	void *onMouseUp_data;
	void (*onSlide)(void *data);
	void *onSlide_data;
} SGE_Slider;

typedef struct SGE_TextInputBox
{
	int x, y;
	SDL_Rect boundBox;
	struct SGE_WindowPanel *parentPanel;
	Uint8 alpha;
	
	char *text;
	int textLengthLimit;
	SDL_Rect inputBox;
	bool isEnabled;
	
	SDL_Rect cursor;
	int cursor_dx;
	int cursor_dy;
	int lastTime;
	bool showCursor;

	SGE_Texture *textImg;
	int lastTextWidth;
	int currentCharWidth;
	SGE_LinkedList *characterWidthStack;
	int lastSpacePosition;
	
	void (*onEnable)(void *data);
	void *onEnable_data;
	void (*onDisable)(void *data);
	void *onDisable_data;
	void (*onTextEnter)(void *data);
	void *onTextEnter_data;
	void (*onTextDelete)(void *data);
	void *onTextDelete_data;
} SGE_TextInputBox;

#define LIST_MAX_OPTIONS 10
#define LIST_OPTION_LENGTH 10
typedef struct SGE_ListBox
{
	int x, y;
	SDL_Rect boundBox;
	SGE_WindowPanel *parentPanel;
	Uint8 alpha;
	
	char optionList[LIST_MAX_OPTIONS][LIST_OPTION_LENGTH];
	SDL_Rect selectionBox;
	SGE_Texture *selectionImg;
	int selection;
	
	int optionCount;
	SGE_Texture *optionImages[LIST_MAX_OPTIONS];
	SDL_Rect optionBoxes[LIST_MAX_OPTIONS];
	bool isOpen;
	
	void (*onSelectionChange)(void *data);
	void *onSelectionChange_data;
} SGE_ListBox;

typedef struct SGE_MinimizeButton
{
	SDL_Rect boundBox;
	struct SGE_WindowPanel *parentPanel;
	SGE_ControlState state;
	
	SGE_Texture *buttonImg;
	SDL_Color normalColor;
	SDL_Color hoverColor;
	SDL_Color clickedColor;
	SDL_Color currentColor;
} SGE_MinimizeButton;

#define PANEL_MAX_BUTTONS          50
#define PANEL_MAX_CHECKBOXES       50
#define PANEL_MAX_LABELS           50
#define PANEL_MAX_SLIDERS          50
#define PANEL_MAX_TEXT_INPUT_BOXES 50
#define PANEL_MAX_LISTBOXES        50
typedef struct SGE_WindowPanel
{
	char titleStr[50];
	int index;
	Uint8 alpha;
	SDL_Rect boundBox;
	SGE_Texture *titleTextImg;
	SDL_Rect background;
	SDL_Color backgroundColor;
	SDL_Rect border;
	SDL_Color borderColor;
	SDL_Rect titleRect;
	int borderThickness;
	int titleHeight;
	SDL_Point bgLocalCenter;
	SDL_Point bgGlobalCenter;
	bool isActive;
	bool isVisible;
	
	bool isMovable;
	bool isMoving;
	int move_dx, move_dy;
	
	bool isResizable;
	SDL_Rect resizeBar_vertical;
	SDL_Rect resizeBar_horizontal;
	
	bool isResizing_vertical;
	bool isResizing_horizontal;
	int resize_origin_x, resize_origin_y;
	int resize_origin_w, resize_origin_h;
	
	bool isMinimizable;
	bool isMinimized;
	SGE_MinimizeButton *minimizeButton;
	int temp_border_w;
	int temp_border_h;
	int temp_background_w;
	int temp_background_h;
	bool temp_horizontalScrollbarEnabled;
	bool temp_verticalScrollbarEnabled;
	
	SDL_Rect masterControlRect;
	
	bool horizontalScrollbarEnabled;
	SDL_Rect horizontalScrollbarBG;
	SDL_Rect horizontalScrollbar;
	bool isScrolling_horizontal;
	int horizontalScrollbar_move_dx;
	int scroll_dx;
	double x_scroll_offset;
	
	bool verticalScrollbarEnabled;
	SDL_Rect verticalScrollbarBG;
	SDL_Rect verticalScrollbar;
	bool isScrolling_vertical;
	int verticalScrollbar_move_dy;
	int scroll_dy;
	double y_scroll_offset;
	
	void (*onMove)(void *data);
	void *onMove_data;
	void (*onResize)(void *data);
	void *onResize_data;
	void (*onMinimize)(void *data);
	void *onMinimize_data;
	void (*onMaximize)(void *data);
	void *onMaximize_data;
	
	int controlCount;
	SGE_Button *buttons[PANEL_MAX_BUTTONS];
	int buttonCount;
	SGE_CheckBox *checkBoxes[PANEL_MAX_CHECKBOXES];
	int checkBoxCount;
	SGE_TextLabel *textLabels[PANEL_MAX_LABELS];
	int textLabelCount;
	SGE_Slider *sliders[PANEL_MAX_SLIDERS];
	int sliderCount;
	SGE_TextInputBox *textInputBoxes[PANEL_MAX_TEXT_INPUT_BOXES];
	int textInputBoxCount;
	SGE_ListBox *listBoxes[PANEL_MAX_LISTBOXES];
	int listBoxCount;
} SGE_WindowPanel;

bool SGE_GUI_Init();
void SGE_GUI_Quit();
void SGE_GUI_HandleEvents();
void SGE_GUI_Update();
void SGE_GUI_Render();

SGE_Button *SGE_CreateButton(const char *text, int x, int y, struct SGE_WindowPanel *panel);
void SGE_ButtonSetPosition(SGE_Button *button, int x, int y);

SGE_CheckBox *SGE_CreateCheckBox(int x, int y, struct SGE_WindowPanel *panel);
void SGE_CheckBoxSetPosition(SGE_CheckBox *checkBox, int x, int y);

SGE_TextLabel *SGE_CreateTextLabel(const char *text, int x, int y, SDL_Color color, struct SGE_WindowPanel *panel);
SGE_TextLabel *SGE_CreateTextLabelf(int x, int y, SDL_Color color, struct SGE_WindowPanel *panel, const char *format, ...);
SGE_TextLabel *SGE_CreateTextLabelCustom(const char *text, int x, int y, SDL_Color color, TTF_Font *font, struct SGE_WindowPanel *panel);
void SGE_TextLabelSetPosition(SGE_TextLabel *label, int x, int y);
void SGE_TextLabelSetText(SGE_TextLabel *label, const char *text);
void SGE_TextLabelSetTextf(SGE_TextLabel *label, const char *format, ...);
void SGE_TextLabelSetFGColor(SGE_TextLabel *label, SDL_Color fg);
void SGE_TextLabelSetBGColor(SGE_TextLabel *label, SDL_Color bg);
void SGE_TextLabelSetMode(SGE_TextLabel *label, SGE_TextRenderMode mode);
void SGE_TextLabelSetAlpha(SGE_TextLabel *label, Uint8 alpha);
void SGE_TextLabelSetVisible(SGE_TextLabel *label, bool visible);

SGE_Slider *SGE_CreateSlider(int x, int y, struct SGE_WindowPanel *panel);
void SGE_SliderSetPosition(SGE_Slider *slider, int x, int y);
void SGE_SliderSetValue(SGE_Slider *slider, double value);
void SGE_SliderUpdateValue(SGE_Slider *slider);

SGE_TextInputBox *SGE_CreateTextInputBox(int maxTextLength, int x, int y, struct SGE_WindowPanel *panel);
void SGE_TextInputBoxSetPosition(SGE_TextInputBox *textInputBox, int x, int y);
void SGE_TextInputBoxSetPositionNextTo(SGE_TextInputBox *textInputBox, SDL_Rect targetBoundBox, SGE_ControlDirection direction, int spacing_x, int spacing_y);
void SGE_TextInputBoxClear(SGE_TextInputBox *textInputBox);

SGE_ListBox *SGE_CreateListBox(int listCount, char list[][LIST_OPTION_LENGTH], int x, int y, SGE_WindowPanel *panel);
void SGE_ListBoxSetPosition(SGE_ListBox *listBox, int x, int y);
void SGE_ListBoxSetPositionNextTo(SGE_ListBox *listBox, SDL_Rect targetBoundBox, SGE_ControlDirection direction, int spacing_x, int spacing_y);

SGE_WindowPanel *SGE_CreateWindowPanel(const char *title, int x, int y, int w, int h);
void SGE_WindowPanelSetPosition(SGE_WindowPanel *panel, int x, int y);
void SGE_WindowPanelSetSize(SGE_WindowPanel *panel, int w, int h);
void SGE_SetActiveWindowPanel(SGE_WindowPanel *panel);
void SGE_SendActivePanelToTop();
SGE_WindowPanel *SGE_GetActiveWindowPanel();
void SGE_WindowPanelToggleMinimized(SGE_WindowPanel *panel);

/**
 * \brief Get a list of panels in the current state as a string.
 * 
 * \return String containing names of panels in the current state.
 */
const char *SGE_GetPanelListAsStr();

void SGE_ButtonSetPositionNextTo(SGE_Button *button, SDL_Rect targetBoundBox, SGE_ControlDirection direction, int spacing_x, int spacing_y);
void SGE_CheckBoxSetPositionNextTo(SGE_CheckBox *checkBox, SDL_Rect targetBoundBox, SGE_ControlDirection direction, int spacing_x, int spacing_y);
void SGE_TextLabelSetPositionNextTo(SGE_TextLabel *label, SDL_Rect targetBoundBox, SGE_ControlDirection direction, int spacing_x, int spacing_y);
void SGE_SliderSetPositionNextTo(SGE_Slider *slider, SDL_Rect targetBoundBox, SGE_ControlDirection direction, int spacing_x, int spacing_y);
void SGE_WindowPanelSetPositionNextTo(SGE_WindowPanel *panel, SDL_Rect targetBoundBox, SGE_ControlDirection direction, int spacing_x, int spacing_y);


/**
 * \brief Updates the SDL_Renderer to be used by SGE_GUI.c
 * 
 */
void SGE_GUI_UpdateSDLRenderer();

/**
 * \brief Updates the control list to be used by SGE GUI functions.
 * 
 * This function is automatically called by SGE_SwitchStates() after a state switch is triggered
 * so the new state's SGE_GUI_ControlList is used by GUI functions.
 * It also resets the state of all the controls in the new state since a state switch might have
 * left the controls in an altered state. (e.g if a button is used to trigger a state switch)
 * 
 * \note This should not be called by the user, as it could lead to another
 *       state's GUI being rendered over the current state.
 * 
 * \param nextState The name of the state that is being switched to.
 */
void SGE_GUI_UpdateCurrentState(const char *nextState);

/**
 * \brief Free's a game state's GUI control list.
 * 
 * This function will delete all the GUI controls that were created by the specified state.
 * It is called automatically when a state is freed by the SGE_QuitState() function.
 * 
 * \note This should not be called by the user, as the state might try to use a control
 *       that has been freed.
 * 
 * \param name The name of the state whose GUI controls should be freed.
 */
void SGE_GUI_FreeState(const char *state);

#endif