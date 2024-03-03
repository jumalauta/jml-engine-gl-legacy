/**
 * The main entry point of the engine / program.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graphicsIncludes.h"
#include "version.h"
#include "system/graphics/graphics.h"
#include "system/ui/window/window.h"
#include "system/ui/window/menu.h"
#include "system/ui/input/input.h"
#include "system/ui/editors/splineEditor.h"
#include "system/audio/sound.h"
#include "system/player/player.h"
#include "system/thread/thread.h"
#include "system/timer/timer.h"
#include "system/debug/debug.h"
#include "system/javascript/javascript.h"
#include "system/datatypes/memory.h"
#include "system/rocket/synceditor.h"
#include "system/io/io.h"
#include "effects/scene_globals.h"
#include "effects/playlist.h"

//#include "test/test_main.h"

static float timerPosition      = 0.0f;
static int showMenu             = 1;
static int splineEditor         = 0;
static unsigned int threadCount = 0;

/**
 * Process all command line arguments.
 * @param argc number of commandline arguments
 * @param argv commandline arguments
 * @return 1 if arguments were handled successfully, 0 if there were errors
 */
static int handleCommandLineArguments(int argc, char **argv)
{
	const int MUTE_SOUND      = 0;
	const int CHANGE_POSITION = 1;
	const int RESOLUTION      = 2;
	const int FULLSCREEN      = 3;
	const int NO_MENU         = 4;
	const int VERBOSE         = 5;
	const int TOOL            = 6;
	const int SPLINE_EDITOR   = 7;
	const int FILE            = 8;
	const int THREAD_COUNT    = 9;
	const int VERSION         = 10;
	const int DEMO_PATH       = 11;
	const char commandSwitches[12][256] =
	{
		"--muteSound\0",
		"--changePosition\0",
		"--resolution\0",
		"--fullscreen\0",
		"--noMenu\0",
		"--verbose\0",
		"--tool\0",
		"--splineEditor\0",
		"--file\0",
		"--threadCount\0",
		"--version\0",
		"--demoPath\0"
	};

	int i;
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], commandSwitches[MUTE_SOUND]))
		{
			debugPrintf("User requested sound muting.");
			soundMute(1);
		}
		else if (!strcmp(argv[i], commandSwitches[CHANGE_POSITION]))
		{
			timerPosition  = convertTimeToSeconds(argv[++i]); //rewind player
		}
		else if (!strcmp(argv[i], commandSwitches[RESOLUTION]))
		{
			int windowWidth  = 0;
			int windowHeight = 0;

			sscanf(argv[++i],"%4dx%4d", &windowWidth, &windowHeight);
			debugPrintf("User requested resolution: %dx%d", windowWidth, windowHeight);
			setWindowDimensions(windowWidth, windowHeight);
		}
		else if (!strcmp(argv[i], commandSwitches[FULLSCREEN]))
		{
			int fullscreen = 0;
			sscanf(argv[++i],"%1d", &fullscreen);
			debugPrintf("Fullscreen option: %d", fullscreen);
			setWindowFullscreen(fullscreen);
		}
		else if (!strcmp(argv[i], commandSwitches[NO_MENU]))
		{
			showMenu = 0;
		}
		else if (!strcmp(argv[i], commandSwitches[TOOL]))
		{
			setPlayerEditor(1);
		}
		else if (!strcmp(argv[i], commandSwitches[VERBOSE]))
		{
			setDebug(1);
		}
		else if (!strcmp(argv[i], commandSwitches[SPLINE_EDITOR]))
		{
			splineEditor = 1;
		}
		else if (!strcmp(argv[i], commandSwitches[THREAD_COUNT]))
		{
			sscanf(argv[++i],"%u", &threadCount);
			debugPrintf("Requested threads: %u", threadCount);
		}
		else if (!strcmp(argv[i], commandSwitches[VERSION]))
		{
			printf("Version %s (Build %s %s)", DEMO_ENGINE_VERSION_STRING, __DATE__, __TIME__);
			return 0;
		}
		else if (!strcmp(argv[i], commandSwitches[DEMO_PATH]))
		{
			setStartPath(argv[++i]);
		}
		else if (!strcmp(argv[i], commandSwitches[FILE]) && ++i < argc)
		{
			splineEditorLoad(argv[i]);
		}
		else
		{
			printf("Unknown command line switch '%s'\n", argv[i]);
			printf("Available command line switches are:\n");
			printf("%s - Mutes audio playing\n", commandSwitches[MUTE_SOUND]);
			printf("%s <SECONDS> - Jumps to following position in the demo\n", commandSwitches[CHANGE_POSITION]);
			printf("%s <WIDTH>x<HEIGHT> - Sets width and height of the window\n", commandSwitches[RESOLUTION]);
			printf("%s <0|1> - 1=fullscreen, 0=windowed\n", commandSwitches[FULLSCREEN]);

			//exit the engine
			return 0;
		}
	}
	
	return 1;
}

/**
 * Makes all preprocessing and setups that are needed to initialize engine
 * @param argc number of commandline arguments
 * @param argv commandline arguments
 */
static void systemPreinit(int argc, char **argv)
{
	if (!handleCommandLineArguments(argc, argv))
	{
		exit(EXIT_FAILURE);
	}

	if (strlen(getStartPath()) == 0)
	{
		setStartPath(argv[0]);
	}

	setClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	memoryInit();

	menuInit();

#ifdef JAVASCRIPT
	if (jsInit() == -1)
	{
		exit(EXIT_FAILURE);
	}
#endif

	//HACK... handle second time so that init.js doesn't override arguments
	handleCommandLineArguments(argc, argv);
}

/**
 * Futher initialize the engine and player.
 */
static void systemInit()
{	
	windowInit();

	if (windowShow() == -1)
	{
		exit(EXIT_FAILURE);
	}

	loadOpenGlMainContext();
	threadInit(threadCount);
	threadQueueInit();

	windowSetTitle("Amphibian shit");

	debugPrintf("Extension and remaining initializations started.");
	openGlExtensionsInit();

	if (isOpenGlError())
	{
		debugErrorPrintf("OpenGL initialization problems in extensions initialization");
		printOpenGlErrors();
	}

	if (syncEditorInit() == -1)
	{
		exit(EXIT_FAILURE);
	}

	playerInit();

	threadQueueDeinit();

	if (splineEditor)
	{
		splineEditorInit();
		/*if (argc > 1)
		{
			splineEditorLoad(argv[argc-1]);
		}*/
	}
	
	//comment this out to disable/mute the sound
	if (!soundIsMute())
	{
		soundInit();
//		void soundLoadPlaylist(const char *filename);

		//normal demo init
		soundClearPlaylist();
		soundAddSongToPlaylist(getPlaylistMusic(), "", 0);
		soundPlaySong(0);
	}
	
	timerInit(getPlaylistLength());
	timerAddTime(timerPosition);

	timerUpdate();
}

/**
 * The main demo playing loop.
 */
static void systemRun()
{
	srand(1); //fix pseudo-random to initial value

	if (splineEditor)
	{
		debugPrintf("Editor started.");
		while(!isUserExit())
		{
			timerUpdate();
			
			splineEditorRun();
			
			timerAdjustFramerate();
		}
		debugPrintf("Demo ended, deinitialization started.");
	}
	else
	{
		debugPrintf("Demo started.");
		while((timerIsPause() || !timerIsEnd()) && !isUserExit())
		{
			timerUpdate();

			syncEditorRun();

			playerDraw();
			
			timerAdjustFramerate();
		}
		debugPrintf("Demo ended, deinitialization started.");
	}
}

/**
 * Deinitialize the engine and cleanup.
 */
static void systemDeinit()
{
	
	//splineEditorDeinit();
	if (!soundIsMute())
	{
		soundDeinit();
	}

	if (splineEditor)
	{
		splineEditorDeinit();
	}

	playerDeinit();

	syncEditorDeinit();

	memoryDeinit();

	threadDeinit();

	windowDeinit();

#ifdef JAVASCRIPT
	jsDeinit();
#endif

	debugPrintf("System deinitialized successfully");
}

/**
 * Program entry point.
 * @fn main
 * @return EXIT_SUCCESS if program completes successfully. EXIT_FAILURE in case of any issues.
 */
#ifdef SDL
int main(int argc, char **argv)
#elif WINDOWS
static int argc = 0;
static int argv[1][1] = {{'\0'}};
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,INT NcMDShow)
#endif
{
	//testMain();

	systemPreinit(argc, argv);
	
	debugPrintf("Demo engine version %s (Build %s %s)", DEMO_ENGINE_VERSION_STRING, __DATE__, __TIME__);

#ifdef MORPHOS
	showMenu = 0;
#endif

	if (showMenu)
	{		
		menuDraw();
	}

	systemInit();

	systemRun();

	systemDeinit();

	return EXIT_SUCCESS;
}
