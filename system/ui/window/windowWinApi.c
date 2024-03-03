#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <windows.h>

#include "graphicsIncludes.h"
#include "system/debug/debug.h"
#include "window.h"

HGLRC hRC = NULL;
HDC hDC = NULL;
HWND hWnd = NULL;
HINSTANCE hInstance;

char keys[256];
char winApiWindowActive = 1;

LRESULT CALLBACK WndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam) {
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
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
            keys[wParam] = 1;
            return 0;
        case WM_KEYUP:
            keys[wParam] = 0;
            return 0;
    }
    return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

GLvoid FreeGL(GLvoid) {
    if(windowIsFullscreen()) {
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

int CreateGLWindow(char *title,int width,int height,int bits,int full) {
    GLuint pixelformat;
    WNDCLASS wc;
    DWORD dwExStyle;
    DWORD dwStyle;
    RECT windowrect;
    windowrect.left = (long) 0;
    windowrect.right = (long) width;
    windowrect.top = (long) 0;
    windowrect.bottom = (long) height;
    hInstance = GetModuleHandle(NULL);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC) WndProc;
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
    if(windowIsFullscreen()) {
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
    if(windowIsFullscreen()) {
        dwExStyle = WS_EX_APPWINDOW;
        dwStyle = WS_POPUP;
        ShowCursor(0);
    } else {
        dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        dwStyle = WS_EX_OVERLAPPEDWINDOW;
    }
    AdjustWindowRectEx(&windowrect,dwStyle,0,dwExStyle);
    if(!(hWnd = CreateWindowEx(dwExStyle,"c",title,WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwStyle,0,0,windowrect.right - windowrect.left,windowrect.bottom - windowrect.top,NULL,NULL,hInstance,NULL))) {
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
    glViewport(0,0,getWindowWidth(),getWindowHeight());
    return 1;
}

void windowDeinit(void)
{
	debugPrintf("Deinitializing window\n");
}

int windowShow(void)
{
	return CreateGLWindow(title,getWindowWidth(),getWindowHeight(),32,windowIsFullscreen());
}

#define TITLE_LENGTH 256

void windowSetTitle(const char *newTitle)
{
	strncpy(title, newTitle, TITLE_LENGTH);
}

void windowSetTitleTimer(const char *titleTimer)
{
	const int WHOLE_TITLE_STRING_LENGTH = TITLE_LENGTH + 64;
	char titleExtraInfo[WHOLE_TITLE_STRING_LENGTH];
	snprintf(titleExtraInfo, WHOLE_TITLE_STRING_LENGTH, "%s %s", title, titleTimer);
}
