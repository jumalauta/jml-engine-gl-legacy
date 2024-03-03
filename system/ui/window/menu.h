#ifndef EXH_SYSTEM_UI_WINDOW_MENU_H_
#define EXH_SYSTEM_UI_WINDOW_MENU_H_

typedef struct
{
	int resize, pressed, selected, visible, enabled;
	int x, y, w, h, type, radioGroup, textalign, mousePressedX, mousePressedY;
	float red,green,blue;
	char *text;
} gui_component_t;

extern void menuInit();
extern void menuDraw();
extern gui_component_t* get_gui_component(int index);
extern void gui_component_set_selected(gui_component_t* gui_component_ptr);
extern void gui_component_set_text(gui_component_t* gui_component_ptr, const char * val);

#endif /* EXH_SYSTEM_UI_WINDOW_MENU_H_ */
