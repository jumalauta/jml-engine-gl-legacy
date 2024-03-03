#include "input.h"
#include "graphicsIncludes.h"
#include <windows.h>

extern char winApiWindowActive;
extern char keys[256];

static MSG msg;


int isUserExit(void)
{
	if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
	{
		if(msg.message == WM_QUIT)
		{
			return 1;
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
				return 1;
			}
		}
	}

	return 0;
}
