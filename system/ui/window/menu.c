#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graphicsIncludes.h"
#include "system/graphics/graphics.h"
#include "system/graphics/font/font.h"
#include "system/ui/window/window.h"
#include "system/ui/input/input.h"
#include "system/audio/sound.h"
#include "system/player/player.h"
#include "system/timer/timer.h"
#include "system/debug/debug.h"
#include "effects/playlist.h"

#ifdef SDL
#elif WINDOWS
#include <windows.h>
#define LOWORD(l) ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l) ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

#include "menu.h"

static const int menu_window_width = 350;
static const int menu_window_height = 130;

extern unsigned char font_rasters[][13];

static GLuint font_offset;
static void make_raster_font(void)
{
    GLuint i;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    font_offset = glGenLists (128);
    for (i = 32; i < 127; i++) {
        glNewList(i+font_offset, GL_COMPILE);
            glBitmap(8, 13, 0.0, 2.0, 10.0, 0.0, font_rasters[i-32]);
        glEndList();
    }
}

static void print_string(const char * s)
{
    glPushAttrib (GL_LIST_BIT);
    glListBase(font_offset);
    glCallLists(strlen(s), GL_UNSIGNED_BYTE, (GLubyte *) s);
    glPopAttrib ();
}

typedef struct
{
	int middle, right, left;
	int x, y;
} gui_mouse_t;

static void init_gui_mouse(gui_mouse_t* gui_mouse_ptr)
{
	gui_mouse_ptr->middle = gui_mouse_ptr->right = gui_mouse_ptr->left = 0;
	gui_mouse_ptr->x = 0;
	gui_mouse_ptr->y = 0;
}

static gui_mouse_t gui_mouse_event;

enum component
{
	NO_COMPONENT, Button, Label, CheckBox, TextArea, ComboBox, RadioButton, DragButton
};

enum align
{
	Left, Center, Right
};

#define MENU_COMPONENT_SIZE 9
static gui_component_t menu_component[MENU_COMPONENT_SIZE];
gui_component_t* get_gui_component(int index)
{
	return &menu_component[index];
}

static void gui_component_init(gui_component_t* gui_component_ptr)
{
	gui_component_ptr->pressed = gui_component_ptr->selected = gui_component_ptr->resize = 0;
	gui_component_ptr->visible = gui_component_ptr->enabled = 1;
	gui_component_ptr->x = gui_component_ptr->y = gui_component_ptr->w = gui_component_ptr->h = gui_component_ptr->type = gui_component_ptr->radioGroup = gui_component_ptr->textalign = 0;
	gui_component_ptr->red = gui_component_ptr->green = gui_component_ptr->blue = 0.0f;
	gui_component_ptr->text = NULL;
}

static void gui_component_deinit(gui_component_t* gui_component_ptr)
{
	if (gui_component_ptr->text != NULL)
	{
		free(gui_component_ptr->text);
	}
}

void gui_component_set_selected(gui_component_t* gui_component_ptr)
{
	if ((gui_component_ptr->type == CheckBox) ||
	(gui_component_ptr->type == ComboBox))
	{
		gui_component_ptr->selected = !gui_component_ptr->selected;
	}
	else if (gui_component_ptr->type == RadioButton)
	{
		gui_component_ptr->selected = 1;
		int i;
		for(i=0;i<6;i++)
		{
			if ((menu_component[i].type == RadioButton) &&
			(&menu_component[i] != gui_component_ptr) &&
			(gui_component_ptr->radioGroup == menu_component[i].radioGroup))
			{
				menu_component[i].selected = 0;
			}
		}
		
	}
	else //TextArea, Button
	{
		gui_component_ptr->selected = 1;
	}
}
void gui_component_set_color(gui_component_t* gui_component_ptr, float r, float g, float b)
{
	gui_component_ptr->red = r;
	gui_component_ptr->green = g;
	gui_component_ptr->blue = b;
}

static const float BG_GRAY = 0.8;
static const float FG_GRAY = 0.6;
static const float TXT_GRAY = 0.0;
static const float RESIZE_GRAY = 0.4;

static void gui_set_background_color()
{
	glColor3f(BG_GRAY,BG_GRAY,BG_GRAY);
}

static void gui_set_foreground_color()
{
	glColor3f(FG_GRAY,FG_GRAY,FG_GRAY);
}

static void gui_set_text_color()
{
	glColor3f(TXT_GRAY,TXT_GRAY,TXT_GRAY);
}

static void gui_set_resize_color()
{
	glColor3f(RESIZE_GRAY,RESIZE_GRAY,RESIZE_GRAY);
}


void gui_component_set_type(gui_component_t* gui_component_ptr, int val)
{
	gui_component_ptr->type = val;

	if (gui_component_ptr->type == DragButton)
	{
		gui_component_set_color(gui_component_ptr, 0.8f, 0.0f, 0.0f);
	}
}

void gui_component_set_text(gui_component_t* gui_component_ptr, const char * val)
{
	if (gui_component_ptr->text != NULL)
	{
		free(gui_component_ptr->text);
	}

	gui_component_ptr->text = strdup(val);
}

int gui_component_mouse_entered(gui_component_t* gui_component_ptr, int mouse_x, int mouse_y)
{
	if (!gui_component_ptr->visible)
	{
		return 0;
	}

	if ((mouse_x >= gui_component_ptr->x) && (mouse_y >= gui_component_ptr->y) && (mouse_x <= gui_component_ptr->x+gui_component_ptr->w) && (mouse_y <= gui_component_ptr->y+gui_component_ptr->h))
	{
		return 1;
	}

	return 0;
}

void gui_component_set_dimension4i(gui_component_t* gui_component_ptr, int tx, int ty, int tw, int th)
{
	gui_component_ptr->x = tx;
	gui_component_ptr->y = ty;
	gui_component_ptr->w = tw;
	gui_component_ptr->h = th;
}

void gui_component_set_dimension2i(gui_component_t* gui_component_ptr, int tx, int ty)
{
	gui_component_ptr->x = tx;
	gui_component_ptr->y = ty;

	switch(gui_component_ptr->type)
	{
		case Button:
			gui_component_ptr->w = 50;
			gui_component_ptr->h = 20;
			break;
		case Label:
			gui_component_ptr->w = 100;
			gui_component_ptr->h = 15;
			break;
		case CheckBox:
			gui_component_ptr->w = 15;
			gui_component_ptr->h = 15;
			break;
		case TextArea:
			gui_component_ptr->w = 300;
			gui_component_ptr->h = 200;
			break;
		case ComboBox:
			gui_component_ptr->w = 100;
			gui_component_ptr->h = 15;
			break;
		case RadioButton:
			gui_component_ptr->w = 15;
			gui_component_ptr->h = 15;
			break;
		case DragButton:
			gui_component_ptr->w = 50;
			gui_component_ptr->h = 20;
			break;
	}
}

void gui_component_draw_label(gui_component_t* gui_component_ptr)
{
	unsigned int chars = gui_component_ptr->w/10;
	unsigned int textlen = (chars<strlen(gui_component_ptr->text)?chars:strlen(gui_component_ptr->text));
	int calcx;

	if (gui_component_ptr->textalign == Left)
	{
		calcx = gui_component_ptr->x+3;
	}
	else if (gui_component_ptr->textalign == Center)
	{
		calcx = gui_component_ptr->x+(((chars-textlen)*10)/2);
	}
	else
	{
		calcx = gui_component_ptr->x+((chars-textlen)*10);
	}

	gui_set_text_color();
	glRasterPos2i(calcx,gui_component_ptr->y+3+(gui_component_ptr->h/2-8));
	//print_string(text.substr(0,chars));
	print_string((const char*)gui_component_ptr->text);
}

void gui_component_draw_label3i(gui_component_t* gui_component_ptr, int addx, int addy, int nw)
{
	unsigned int chars = nw/10;
	unsigned int textlen = (chars<strlen(gui_component_ptr->text)?chars:strlen(gui_component_ptr->text));
	int calcx;

	if (gui_component_ptr->textalign == Left)
	{
		calcx = gui_component_ptr->x+3;
	}
	else if (gui_component_ptr->textalign == Center)
	{
		calcx = gui_component_ptr->x+(((chars-textlen)*10)/2);
	}
	else
	{
		calcx = gui_component_ptr->x+((chars-textlen)*10);
	}

	gui_set_text_color();
	glRasterPos2i(addx+calcx,addy+gui_component_ptr->y+3+(gui_component_ptr->h/2-8));
	//print_string(text.substr(0,chars));
	print_string((const char*)gui_component_ptr->text);
}

void gui_component_draw_label5i(gui_component_t* gui_component_ptr, int tx, int ty, int addx, int addy, int nw)
{
	unsigned int chars = nw/10;
	unsigned int textlen = (chars<strlen((const char*)gui_component_ptr->text)?chars:strlen((const char*)gui_component_ptr->text));
	int calcx;

	if (gui_component_ptr->textalign == Left)
	{
		calcx = tx+3;
	}
	else if (gui_component_ptr->textalign == Center)
	{
		calcx = tx+(((chars-textlen)*10)/2);
	}
	else
	{
		calcx = tx+((chars-textlen)*10);
	}

	gui_set_text_color();
	glRasterPos2i(addx+calcx,addy+ty+3+(gui_component_ptr->h/2-8));
	//print_string(text.substr(0,chars));
	print_string((const char*)gui_component_ptr->text);
}

void gui_component_draw_radio_button(gui_component_t* gui_component_ptr)
{
	glPushMatrix();
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	gui_set_background_color();
	glBegin(GL_QUADS);
	glVertex2f(gui_component_ptr->x,gui_component_ptr->y);
	glVertex2f(gui_component_ptr->x,gui_component_ptr->y+gui_component_ptr->h);
	glVertex2f(gui_component_ptr->x+gui_component_ptr->w,gui_component_ptr->y+gui_component_ptr->h);
	glVertex2f(gui_component_ptr->x+gui_component_ptr->w,gui_component_ptr->y);
	glEnd();

	if (gui_component_ptr->pressed) { glLineWidth(2.0f); }
	gui_set_foreground_color();
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
	glVertex2f(gui_component_ptr->x,gui_component_ptr->y);
	glVertex2f(gui_component_ptr->x,gui_component_ptr->y+gui_component_ptr->h);
	glVertex2f(gui_component_ptr->x+gui_component_ptr->w,gui_component_ptr->y+gui_component_ptr->h);
	glVertex2f(gui_component_ptr->x+gui_component_ptr->w,gui_component_ptr->y);
	glEnd();

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (gui_component_ptr->selected)
	{
		gui_set_text_color();
		glBegin(GL_QUADS);
		glVertex2f(gui_component_ptr->x+(gui_component_ptr->w*0.2f),gui_component_ptr->y+(gui_component_ptr->h*0.2f));
		glVertex2f(gui_component_ptr->x+(gui_component_ptr->w*0.2f),(gui_component_ptr->y+gui_component_ptr->h)-(gui_component_ptr->h*0.2f));
		glVertex2f(gui_component_ptr->x+gui_component_ptr->w-(gui_component_ptr->w*0.2f),(gui_component_ptr->y+gui_component_ptr->h)-(gui_component_ptr->h*0.2f));
		glVertex2f(gui_component_ptr->x+gui_component_ptr->w-(gui_component_ptr->w*0.2f),gui_component_ptr->y+(gui_component_ptr->h*0.2f));
		glEnd();
	}

	gui_component_draw_label3i(gui_component_ptr, gui_component_ptr->w, 0, 100);

	glLineWidth(1.0f);
	glPopAttrib();
	glPopMatrix();
}

void gui_component_draw_check_box(gui_component_t* gui_component_ptr)
{
	float x = gui_component_ptr->x;
	float y = gui_component_ptr->y;
	float h = gui_component_ptr->h;
	float w = gui_component_ptr->w;
	
	glPushMatrix();
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	gui_set_background_color();
	glBegin(GL_QUADS);
	glVertex2f(x,y);
	glVertex2f(x,y+h);
	glVertex2f(x+w,y+h);
	glVertex2f(x+w,y);
	glEnd();

	if (gui_component_ptr->pressed) { glLineWidth(2.0f); }
	gui_set_foreground_color();
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
	glVertex2f(x,y);
	glVertex2f(x,y+h);
	glVertex2f(x+w,y+h);
	glVertex2f(x+w,y);
	glEnd();
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	gui_set_text_color();
	if (gui_component_ptr->selected)
	{
		glLineWidth(w*0.2f);
		glBegin(GL_LINES);
			glVertex2f(x+(w*0.2f),y+(h*0.2f));
			glVertex2f(x+w-(w*0.2f),y+h-(h*0.2f));
			glVertex2f(x+w-(w*0.2f),y+(w*0.2f));
			glVertex2f(x+(w*0.2f),y+h-(w*0.2f));
		glEnd();
	}

	gui_component_draw_label3i(gui_component_ptr, w, 0, 100);

	glLineWidth(1.0f);
	glPopAttrib();
	glPopMatrix();
}

void gui_component_draw_button(gui_component_t* gui_component_ptr)
{
	float x = gui_component_ptr->x;
	float y = gui_component_ptr->y;
	float h = gui_component_ptr->h;
	float w = gui_component_ptr->w;

	glPushMatrix();
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	gui_set_background_color();
	glBegin(GL_QUADS);
	glVertex2f(x,y);
	glVertex2f(x,y+h);
	glVertex2f(x+w,y+h);
	glVertex2f(x+w,y);
	glEnd();

	if (gui_component_ptr->pressed) { glLineWidth(2.0f); }
	gui_set_foreground_color();
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
	glVertex2f(x,y);
	glVertex2f(x,y+h);
	glVertex2f(x+w,y+h);
	glVertex2f(x+w,y);
	glEnd();

	gui_component_draw_label3i(gui_component_ptr, 0,1,w);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLineWidth(1.0f);
	glPopAttrib();
	glPopMatrix();
}

void gui_component_draw_text_area(gui_component_t* gui_component_ptr)
{
	float x = gui_component_ptr->x;
	float y = gui_component_ptr->y;
	float h = gui_component_ptr->h;
	float w = gui_component_ptr->w;

	glPushMatrix();
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	gui_set_background_color();
	glBegin(GL_QUADS);
	glVertex2f(x,y);
	glVertex2f(x,y+h);
	glVertex2f(x+w,y+h);
	glVertex2f(x+w,y);
	glEnd();

	gui_set_foreground_color();
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
	glVertex2f(x,y);
	glVertex2f(x,y+h);
	glVertex2f(x+w,y+h);
	glVertex2f(x+w,y);
	glEnd();

	gui_component_draw_label(gui_component_ptr);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPopAttrib();
	glPopMatrix();
}

void gui_component_draw_combo_box(gui_component_t* gui_component_ptr)
{
	float x = gui_component_ptr->x;
	float y = gui_component_ptr->y;
	float h = gui_component_ptr->h;
	float w = gui_component_ptr->w;

	glPushMatrix();
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	gui_set_background_color();
	glBegin(GL_QUADS);
	glVertex2f(x,y);
	glVertex2f(x,y+h);
	glVertex2f(x+w,y+h);
	glVertex2f(x+w,y);
	glEnd();

	gui_set_foreground_color();
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
	glVertex2f(x,y);
	glVertex2f(x,y+h);
	glVertex2f(x+w,y+h);
	glVertex2f(x+w,y);
	glEnd();

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	float dropButtonX = w*0.8; //(w*0.8>20?x-20:w*0.8);
	float dropButtonW = (w-dropButtonX);
	glBegin(GL_LINES);
	glVertex2f(x+dropButtonX,y);
	glVertex2f(x+dropButtonX,y+h);
	glEnd();

	//if (gui_component_ptr->selected) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }

	glBegin(GL_TRIANGLES);
	glVertex2f(x+dropButtonX+(dropButtonW*0.2f),y+(h*0.8f));
	glVertex2f(x+dropButtonX+(dropButtonW*0.8f),y+(h*0.8f));
	glVertex2f(x+dropButtonX+(dropButtonW*0.5f),y+h-(h*0.8f));
	glEnd();

	gui_component_draw_label3i(gui_component_ptr, 0, 0, (int)(w-(w-dropButtonX)));

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPopAttrib();
	glPopMatrix();
}

void gui_component_draw_drag_button(gui_component_t* gui_component_ptr)
{
	float x = gui_component_ptr->x;
	float y = gui_component_ptr->y;
	float h = gui_component_ptr->h;
	float w = gui_component_ptr->w;
	float red = gui_component_ptr->red;
	float green = gui_component_ptr->green;
	float blue = gui_component_ptr->blue;

	glPushMatrix();
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	glColor3f(red, green, blue);
	glBegin(GL_QUADS);
	glVertex2f(x,y);
	glVertex2f(x,y+h);
	glVertex2f(x+w,y+h);
	glVertex2f(x+w,y);
	glEnd();

	gui_set_foreground_color();

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
	glVertex2f(x,y);
	glVertex2f(x,y+h);
	glVertex2f(x+w,y+h);
	glVertex2f(x+w,y);
	glEnd();

	gui_component_draw_label3i(gui_component_ptr, 0,0,w);

	if (gui_component_ptr->resize)
	{
		gui_set_resize_color();
		glBegin(GL_QUADS);
		glVertex2f(x,y);
		glVertex2f(x,y+h);
		glVertex2f(gui_mouse_event.x,y+h);
		glVertex2f(gui_mouse_event.x,y);
		glEnd();
	}
	else if (gui_component_ptr->selected)
	{
		int selectx = gui_component_ptr->mousePressedX-x;
		int selecty = gui_component_ptr->mousePressedY-y;

		gui_set_resize_color();
		glBegin(GL_QUADS);
		glVertex2f(gui_mouse_event.x-selectx,gui_mouse_event.y-selecty);
		glVertex2f(gui_mouse_event.x-selectx,gui_mouse_event.y-selecty+h);
		glVertex2f(gui_mouse_event.x-selectx+w,gui_mouse_event.y-selecty+h);
		glVertex2f(gui_mouse_event.x-selectx+w,gui_mouse_event.y-selecty);
		glEnd();
		gui_component_draw_label5i(gui_component_ptr, gui_mouse_event.x-selectx,gui_mouse_event.y-selecty,0,0,w);
	}

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLineWidth(1.0f);
	glPopAttrib();
	glPopMatrix();
}

void gui_component_draw(gui_component_t* gui_component_ptr)
{
	if (!gui_component_ptr->visible)
	{
		return;
	}
	
	switch(gui_component_ptr->type)
	{
		case Button:
			gui_component_draw_button(gui_component_ptr);
			break;
		case Label:
			gui_component_draw_label(gui_component_ptr);
			break;
		case CheckBox:
			gui_component_draw_check_box(gui_component_ptr);
			break;
		case TextArea:
			gui_component_draw_text_area(gui_component_ptr);
			break;
		case ComboBox:
			gui_component_draw_combo_box(gui_component_ptr);
			break;
		case RadioButton:
			gui_component_draw_radio_button(gui_component_ptr);
			break;
		case DragButton:
			gui_component_draw_drag_button(gui_component_ptr);
			break;
	}
}

static int menu_done = 0;
static int mouseSelectedcomponent = -1;
	
#define NONE 0
#define QUIT 1
#define KEYDOWN 2
#define MOUSEBUTTONDOWN 3
#define MOUSEBUTTONUP 4

#define KEY_ESC 100
#define KEY_RETURN 101

#define BUTTON_LEFT 200
		
static int event_type = NONE;
static int button_press = NONE;
static int key_press = NONE;

#if defined(WINDOWS) && !defined(SDL)
static HGLRC hRC = NULL;
static HDC hDC = NULL;
static HWND hWnd = NULL;
static HINSTANCE hInstance;
static MSG msg;

static char keys[256];
static char winApiWindowActive = 1;

static LRESULT CALLBACK WndProcMenu(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam) {
   switch(uMsg) {
        case WM_ACTIVATE:
            if(!HIWORD(wParam)) {
                winApiWindowActive = 1;
            } else {
                winApiWindowActive = 0;
            }
            return 0;
        case WM_SYSCOMMAND:
            switch(wParam) {
                case SC_SCREENSAVE:
                case SC_MONITORPOWER:
                    return 0;
            }
            break;
        case WM_CLOSE:
			event_type = QUIT;
            PostQuitMessage(0);
            return 0;
		case WM_LBUTTONDOWN:
            keys[wParam] = 1;
			event_type = MOUSEBUTTONDOWN;
			button_press = BUTTON_LEFT;
			return 0;
		case WM_LBUTTONUP:
            keys[wParam] = 0;
			event_type = MOUSEBUTTONUP;
			button_press = BUTTON_LEFT;
			return 0;
        case WM_KEYDOWN:
            keys[wParam] = 1;
			event_type = KEYDOWN;
            return 0;
        case WM_KEYUP:
            keys[wParam] = 0;
            return 0;
    }
    return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

static GLvoid freeMenuGL(int full) {
    if(full) {
        ChangeDisplaySettings(NULL,0);
        ShowCursor(1);
    }
    if(hRC) {
        if(!wglMakeCurrent(NULL,NULL)) //MessageBox(NULL,"Releace of DC and RC failed.","BAD!!",MB_OK | MB_ICONINFORMATION);
        if(!wglDeleteContext(hRC)) //MessageBox(NULL,"Releace of rendering context failed.","BAD!!",MB_OK | MB_ICONINFORMATION);
        hRC = NULL;
    }
    if(hDC && !ReleaseDC(hWnd,hDC)) {
        //MessageBox(NULL,"Releace of device context failed.","BAD!!",MB_OK | MB_ICONINFORMATION);
        hDC = NULL;
    }
    if(hWnd && !DestroyWindow(hWnd)) {
        //MessageBox(NULL,"Releace of window failed.","BAD!!",MB_OK | MB_ICONINFORMATION);
        hWnd = NULL;
    }
    if(!UnregisterClass("c",hInstance)) {
        //MessageBox(NULL,"Releace of class failed.","BAD!!",MB_OK | MB_ICONINFORMATION);
        hInstance = NULL;
    }
}

static int createGLMenuWindow(char *title,int width,int height,int bits,int full) {
    GLuint pixelformat;
    WNDCLASS wc;
    DWORD dwExStyle;
    DWORD dwStyle;
    RECT windowrect;
    windowrect.left = (long) 0;
    windowrect.right = (long) width;
    windowrect.top = (long) 0;
    windowrect.bottom = (long) height+30;
    hInstance = GetModuleHandle(NULL);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC) WndProcMenu;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL,IDI_WINLOGO);
    wc.hCursor = LoadCursor(NULL,IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "c";
    if(!RegisterClass(&wc)) {
        MessageBox(NULL,"Shit hit the fan (of YUP).","Error",MB_OK | MB_ICONEXCLAMATION);
        return -1;
    }
    if(full) {
        DEVMODE ss;
        memset(&ss,0,sizeof(ss));
        ss.dmSize = sizeof(ss);
        ss.dmPelsWidth = width;
        ss.dmPelsHeight = height;
        ss.dmBitsPerPel = bits;
        ss.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
        if(ChangeDisplaySettings(&ss,CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
            MessageBox(NULL,"The requested fullscreen mode is not supported by your hardware.","Error",MB_OK);
            return -1;
        }
    }
    if(full) {
        dwExStyle = WS_EX_APPWINDOW;
        dwStyle = WS_POPUP;
        ShowCursor(0);
    } else {
        dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        dwStyle = WS_EX_OVERLAPPEDWINDOW;
    }
    AdjustWindowRectEx(&windowrect,dwStyle,0,dwExStyle);
	
	const unsigned int windowrect_width = windowrect.right - windowrect.left;
	const unsigned int windowrect_height = windowrect.bottom - windowrect.top;
	const unsigned int window_position_x = (GetSystemMetrics(SM_CXSCREEN) - windowrect_width) / 2;
	const unsigned int window_position_y = (GetSystemMetrics(SM_CYSCREEN) - windowrect_height) / 2;
	
    if(!(hWnd = CreateWindowEx(dwExStyle,"c",title,WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwStyle,window_position_x,window_position_y,windowrect_width,windowrect_height,NULL,NULL,hInstance,NULL))) {
        FreeGL();
        MessageBox(NULL,"Creation of window failed.","Error",MB_OK | MB_ICONEXCLAMATION);
        return -1;
    }
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        bits,
        0,0,0,0,0,0,
        0, // No alpha buffer
        0,
        0, // No accumulation buffer
        0,0,0,0,
        16, // 16-bit z buffer
        0, // No stencil buffer <- Shadows? ;)
        0, // No auxilary buffer
        PFD_MAIN_PLANE,
        0,0,0,0
    };
    hDC = GetDC(hWnd);
    pixelformat = ChoosePixelFormat(hDC,&pfd);
    SetPixelFormat(hDC,pixelformat,&pfd);
    hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC,hRC);
    ShowWindow(hWnd,SW_SHOW);
    SetForegroundWindow(hWnd);
    SetFocus(hWnd);
    glViewport(0,0,width,height);
    return 1;
}
#endif

void menuInit()
{
	init_gui_mouse(&gui_mouse_event);
	
	//debugPrintf("Menu component init");
	//InitGL();
	//sound.init(mainDir);
	//make_raster_font();
	int i;
	for(i = 0; i < MENU_COMPONENT_SIZE; i++)
	{
		gui_component_init(&menu_component[i]);
	}


#ifndef MORPHOS
	const char *defaultResolutions[4] = {
		"800x600",
		"1024x768",
		"1280x720",
		"1920x1080"
	};
#else
	//MorphOS should have possibility for lower resolution of 640x480 as well so that performance would be adequate
	const char *defaultResolutions[4] = {
		"640x480",
		"800x600",
		"1024x768",
		"1280x720"
	};
#endif

	gui_component_set_dimension4i(&menu_component[0], 10,100,15,15);
	gui_component_set_type(&menu_component[0], RadioButton);
	gui_component_set_text(&menu_component[0], defaultResolutions[0]);

	gui_component_set_dimension4i(&menu_component[1], 10,80,15,15);
	gui_component_set_type(&menu_component[1], RadioButton);
	gui_component_set_text(&menu_component[1], defaultResolutions[1]);

	gui_component_set_dimension4i(&menu_component[2], 10, 60, 15, 15);
	gui_component_set_type(&menu_component[2], RadioButton);
	gui_component_set_text(&menu_component[2], defaultResolutions[2]);

	gui_component_set_dimension4i(&menu_component[3], 10, 40, 15, 15);
	gui_component_set_type(&menu_component[3], RadioButton);
	gui_component_set_text(&menu_component[3], defaultResolutions[3]);
	menu_component[3].selected = 1;

	gui_component_set_dimension4i(&menu_component[4], 10,10,60,25);
	gui_component_set_type(&menu_component[4], Button);
	gui_component_set_text(&menu_component[4], "Demo");
	menu_component[4].textalign = Center;

	gui_component_set_dimension4i(&menu_component[5], 80,10,60,25);
	gui_component_set_type(&menu_component[5], Button);
	gui_component_set_text(&menu_component[5], "Die");
	menu_component[5].textalign = Center;

	gui_component_set_dimension4i(&menu_component[6], 160,100,15,15);
	gui_component_set_type(&menu_component[6], CheckBox);
	gui_component_set_text(&menu_component[6], "fullscreen");
	menu_component[6].selected = 1;

	gui_component_set_dimension4i(&menu_component[7], 160,80,15,15);
	gui_component_set_type(&menu_component[7], CheckBox);
	gui_component_set_text(&menu_component[7], "music");
	menu_component[7].selected = 1;

}

static void handleEvents(int event_type, int button_press, int key_press)
{
	//Check for mouse actions in the GUI components
	int mouseActioncomponent = -1;
	int i;
	for(i=0;i<MENU_COMPONENT_SIZE;i++)
	{
		if (gui_component_mouse_entered(&menu_component[i], gui_mouse_event.x, gui_mouse_event.y))
		{
			mouseActioncomponent = i;
			break;
		}
	}

	switch(event_type)
	{
		case QUIT:
			debugPrintf("User requested quit in the menu");
			get_gui_component(5)->selected = 1; //hack for escape
			menu_done = 1;
			break;
		case KEYDOWN:
			switch (key_press)
			{
				case KEY_RETURN:
					menu_component[4].selected = 1;
					menu_done = 1;
					break;
				default:
					break;
			}
		case MOUSEBUTTONDOWN:
			if (mouseActioncomponent == -1)
			{
				mouseSelectedcomponent = -1;
				break;
			}
			menu_component[mouseActioncomponent].mousePressedX = gui_mouse_event.x;
			menu_component[mouseActioncomponent].mousePressedY = gui_mouse_event.y;

			switch(button_press)
			{
				case BUTTON_LEFT:
					if (!gui_mouse_event.left)
					{
//if (mouseActioncomponent == 3) break;
						gui_component_set_selected(&menu_component[mouseActioncomponent]);

						if ((menu_component[mouseActioncomponent].type != CheckBox)
						&& (menu_component[mouseActioncomponent].type != ComboBox)
						&& menu_component[mouseActioncomponent].type != RadioButton)
						{
							menu_done = 1;
						}

						gui_mouse_event.left = 1;
					}
					break;
			}

			mouseSelectedcomponent = mouseActioncomponent;
			break;
		case MOUSEBUTTONUP:
			//if (mouseActioncomponent == -1) { break; }
			switch(button_press)
			{
				case BUTTON_LEFT:
					if (gui_mouse_event.left)
					{
						if ((menu_component[mouseSelectedcomponent].type != RadioButton) &&
						(menu_component[mouseSelectedcomponent].type != CheckBox))
						{
							menu_component[mouseSelectedcomponent].selected = 0;
						}
						gui_mouse_event.left = 0;
					}
					break;
			}
			break;
		default:
			break;
	}
}

#ifdef SDL
#define RESOLUTION_COUNT 4
#define RESOLUTION_LENGTH 16
static char currentResolutions[RESOLUTION_COUNT][RESOLUTION_LENGTH];
static void setValidMenuScreenModes()
{
	SDL_Rect **modes = SDL_ListModes(NULL, SDL_OPENGL|SDL_FULLSCREEN);

	if(modes == (SDL_Rect **)0)
	{
		//fullscreen not supported => hide fullscreen option from the menu
		debugWarningPrintf("Fullscreen resolutions are not supported!");
		get_gui_component(6)->selected = get_gui_component(6)->visible = 0;

		modes = SDL_ListModes(NULL, SDL_OPENGL);
		if(modes == (SDL_Rect **)0)
		{
			debugErrorPrintf("No screen resolutions available!");
			return; //let's hope this is a joke and do nothing
		}
	}

	int maximumWidth  = 0;
	int maximumHeight = 0;

	int i;
	for(i=0; i<RESOLUTION_COUNT; i++)
	{
		sprintf(currentResolutions[i], "%s", get_gui_component(i)->text);
		
		int height = 0;
		int width = 0;
		sscanf(currentResolutions[i], "%4dx%4d", &width, &height);
		
		if (width > maximumWidth)
		{
			maximumWidth = width;
		}
		if (height > maximumHeight)
		{
			maximumHeight = height;
		}
	}
	
	debugPrintf("Maximum dimensions: %dx%d", maximumWidth, maximumHeight);
	
	if(modes == (SDL_Rect**)-1)
	{
		debugPrintf("All fullscreen resolutions supported");
	}
	else
	{
		int k = 0;
		//char confirmedResolutions[RESOLUTION_COUNT][RESOLUTION_LENGTH];
		int checkedResolution = 0;
		//go backwards the resolutions so that higher resolution is checked first
		for(i=RESOLUTION_COUNT-1, checkedResolution=0; i>=0; i--, checkedResolution++)
		{
			char resolution[RESOLUTION_LENGTH];
			int resolutionSupported = 0;
			int j;
			for(j=checkedResolution; modes[j] != NULL; j++)
			{
				if (modes[j]->w > maximumWidth || modes[j]->h > maximumHeight)
				{
					resolutionSupported = 1;
					break;
				}

				snprintf(resolution, RESOLUTION_LENGTH, "%dx%d", modes[j]->w, modes[j]->h);
				if (!strcmp(resolution, currentResolutions[i]))
				{
					resolutionSupported = 1;
				}

				for(k=RESOLUTION_COUNT-1; k>i; k--)
				{
					if (!strcmp(currentResolutions[i], currentResolutions[k]))
					{
						resolutionSupported = -1;
						break;
					}
				}

				if (resolutionSupported && j > checkedResolution)
				{
					break;
				}
			}

			if (resolutionSupported <= 0)
			{
				snprintf(resolution, RESOLUTION_LENGTH, "%dx%d", modes[checkedResolution]->w, modes[checkedResolution]->h);
				if (resolutionSupported == 0)
				{
					debugPrintf("Not supported resolution '%s'! Changing resolution to '%s'", currentResolutions[i], resolution);
				}
				else
				{
					debugPrintf("Changing resolution '%s' to '%s'", currentResolutions[i], resolution);
				}

				snprintf(currentResolutions[i], RESOLUTION_LENGTH, "%s", resolution); 
				gui_component_set_text(get_gui_component(i), (const char*)currentResolutions[i]);
			}
		}
	}
}
#endif

static void menuHandle()
{
	//Quit
	if (get_gui_component(5)->selected)
	{
		SDL_Quit();
		exit(EXIT_SUCCESS);
	}
	//Run demo
	else if (get_gui_component(4)->selected)
	{
		setWindowFullscreen(get_gui_component(6)->selected);
		soundMute(!get_gui_component(7)->selected);

		int windowWidth = 0;
		int windowHeight = 0;

		if (get_gui_component(0)->selected)
		{
			sscanf(get_gui_component(0)->text,"%4dx%4d", &windowWidth, &windowHeight);
		}
		if (get_gui_component(1)->selected)
		{
			sscanf(get_gui_component(1)->text,"%4dx%4d", &windowWidth, &windowHeight);
		}
		else if (get_gui_component(2)->selected)
		{
			sscanf(get_gui_component(2)->text,"%4dx%4d", &windowWidth, &windowHeight);
		}
		else if (get_gui_component(3)->selected)
		{
			sscanf(get_gui_component(3)->text,"%4dx%4d", &windowWidth, &windowHeight);
		}
		setWindowDimensions(windowWidth, windowHeight);
	}
}

void menuDraw()
{
	const char *MENU_TITLE = "";
#ifdef SDL
	debugPrintf("SDL menu initialization...");

#ifndef MORPHOS
	putenv("SDL_VIDEO_CENTERED=1");
#endif

	SDL_Init(SDL_INIT_VIDEO);
	setValidMenuScreenModes();
	
	SDL_Event event;
	SDL_WM_SetCaption(MENU_TITLE, 0);
	//SDL_WM_SetCaption("Amphibian shit",0);
	//SDL_WM_SetCaption(mainDir,0);
	SDL_SetVideoMode(menu_window_width, menu_window_height, 0, SDL_OPENGL);
#elif WINDOWS
	debugPrintf("WinAPI menu initialization...");
	createGLMenuWindow(MENU_TITLE,menu_window_width,menu_window_height,32,0);
#endif

	//debugPrintf("Menu raster font init");
	make_raster_font();

	//debugPrintf("Menu loop start menu_done:%d",menu_done);
	//component where mouse is on top
	
	//component that has been previously pressed
	while(!menu_done)
	{
		event_type = NONE;
		button_press = NONE;
		key_press = NONE;

		//debugPrintf("Menu loop iteration");
		timerSleep(1);

		glEnable(GL_LINE_SMOOTH);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

		glClearColor(BG_GRAY, BG_GRAY, BG_GRAY, BG_GRAY);
		glLineWidth(1.0f);
		//glDepthMask(1);

		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		//2D
		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST);
		glPushMatrix();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glPushMatrix();
		glOrtho(0, menu_window_width, 0, menu_window_height, 0, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//debugPrintf("Draw gui components");
		int i;
		for(i=0;i<MENU_COMPONENT_SIZE;i++)
		{
			//debugPrintf("Draw gui component:%d type:%d",i,menu_component[i].type);
			gui_component_draw(&menu_component[i]);
		}
		//debugPrintf("Draw gui components end");
		glPopMatrix();
		glPopMatrix();
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45.0f, 150.0/130.0, 0.01f, 1000.0f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

#ifdef SDL
SDL_PumpEvents();

		while (SDL_PollEvent(&event))
		{
			SDL_GetMouseState(&gui_mouse_event.x, &gui_mouse_event.y);
			gui_mouse_event.y = menu_window_height-gui_mouse_event.y;

			switch(event.type)
			{
				case SDL_QUIT:
					event_type = QUIT;
					break;

				case SDL_KEYDOWN:
					event_type = KEYDOWN;
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							key_press = KEY_ESC;
							event_type = QUIT;
							break;
						case SDLK_RETURN:
							key_press = KEY_RETURN;
							break;
						default:
							break;
					}
					break;

				case SDL_MOUSEBUTTONDOWN:
					event_type = MOUSEBUTTONDOWN;

					switch(event.button.button)
					{
						case SDL_BUTTON_LEFT:
							button_press = BUTTON_LEFT;
							break;
					}
					break;

				case SDL_MOUSEBUTTONUP:
					event_type = MOUSEBUTTONUP;
					//if (mouseActioncomponent == -1) { break; }
					switch(event.button.button)
					{
						case SDL_BUTTON_LEFT:
							button_press = BUTTON_LEFT;
							break;
					}
					break;
				default:
					break;
			}
			
			handleEvents(event_type, button_press, key_press);
		}
#endif

#if defined(WINDOWS) && !defined(SDL)
		POINT mouse_position;
		BOOL cursor_get_result = GetCursorPos(&mouse_position);
		if (cursor_get_result)
		{
			if (ScreenToClient(hWnd, &mouse_position))
			{
				gui_mouse_event.x = mouse_position.x;
				gui_mouse_event.y = mouse_position.y;
				gui_mouse_event.y = menu_window_height-gui_mouse_event.y;
			}
		}

		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{
				event_type = QUIT;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if(winApiWindowActive)
			{
				if(keys[VK_ESCAPE])
				{
					event_type = KEYDOWN;
					key_press = KEY_ESC;
				}
				else if (keys[VK_RETURN])
				{
					event_type = KEYDOWN;
					key_press = KEY_RETURN;
				}
				else if (keys[VK_LBUTTON])
				{
					event_type = MOUSEBUTTONDOWN;
					button_press = BUTTON_LEFT;
				}
			}
		}
		
		handleEvents(event_type, button_press, key_press);
#endif

		graphicsFlush();
	}
	
	menuHandle();
	
	int i;
	for(i = 0; i < MENU_COMPONENT_SIZE; i++)
	{
		gui_component_deinit(&menu_component[i]);
	}

	glColor3f(1,1,1);
		
	#if defined(WINDOWS) && !defined(SDL)
		freeMenuGL(0);
	#endif
}
