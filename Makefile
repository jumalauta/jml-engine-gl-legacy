# Engine Makefile
#
# Supported operating systems: Windows, OS X, MorphOS and Linux
# Supported CPU architectures: 32bit, 64bit, little and big endian
#
# Requirements: GNU Make, GCC (or OS X clang) and usual unix tools
#
# Available commands:
#  make clean         - Clean everything
#  make               - Compile source code (excluding javascript)
#  make js            - Compile JavaScript source code
#  make documentation - Created PDF documentation
#

USEMINIMALIST = TRUE
#setup parameters
DEBUG = FALSE
USESDL = TRUE
USESDL2 = FALSE
USEPNG = STBPNG
CUNIT = FALSE
USEJS = TRUE
USETINYGL = FALSE
USEANTTWEAKBAR = FALSE
USEVIDEO = TRUE
CPUOPTIMIZATION = NONE
#SDL_TTF/BUILTIN
FONT_ENGINE = BUILTIN
#DEMO/INTRO
PRODUCTION_TYPE = DEMO
#FMOD/MINIFMOD/SDL_MIXER/BASS
SOUND = SDL_MIXER

#program defines
MAKE = make
CC = gcc
CXX = g++
RM = rm -f
CP = cp -R
MKDIR = mkdir
UNAME = uname
JAVA = java
PERL = perl
DOXYGEN = doxygen
MARKDOWN_PDF = markdown-pdf
GJSLINT = gjslint

#try to determine the platform
ARCHITECTURE = 
ENDIAN = -DLITTLE_ENDIAN

CHECK_OS = $(shell $(UNAME))
ifeq ($(CHECK_OS),Linux)
	PLATFORM = LINUX
else ifeq ($(CHECK_OS),Darwin)
	PLATFORM = OS_X
else ifeq ($(CHECK_OS),MorphOS)
	PLATFORM = MORPHOS
	ENDIAN = -DENGINE_BIG_ENDIAN
	USEMINIMALIST = TRUE
	#Assuming MorphOS build is for Altivec capable PPC machine
	#CPUOPTIMIZATION = ALTIVEC
else
	# default to Windows... uname MINGW32_NT-6.x etc...
	PLATFORM = WINDOWS
	#force Windows to build 32bit executables
	ARCHITECTURE = -m32
endif

ifeq ($(USEMINIMALIST), TRUE)
	USETINYGL = TRUE
	USEVIDEO = FALSE
	FONT_ENGINE = NO_FONT
endif

#Source paths
PATH_EFFECTS = effects/
PATH_EFFECTS_FILTERS = effects/filters/
PATH_SYSTEM = system/
PATH_AUDIO = system/audio/
PATH_AUDIO_MINIFMOD = system/audio/minifmod/src/
PATH_DATATYPES = system/datatypes/
PATH_DEBUG = system/debug/
PATH_EXTENSIONS_GL = system/extensions/gl/
PATH_GRAPHICS = system/graphics/
PATH_GRAPHICS_FONT = system/graphics/font/
PATH_GRAPHICS_IMAGE = system/graphics/image/
PATH_GRAPHICS_IMAGE_SVG = system/graphics/image/svg/
PATH_GRAPHICS_OBJECT = system/graphics/object/
PATH_GRAPHICS_OBJECT_3DS = system/graphics/object/3ds/
PATH_GRAPHICS_OBJECT_LIB3DS = system/graphics/object/lib3ds/
PATH_GRAPHICS_OBJECT_OBJ = system/graphics/object/obj/
PATH_GRAPHICS_PARTICLE = system/graphics/particle/
PATH_GRAPHICS_SHADER = system/graphics/shader/
PATH_GRAPHICS_VIDEO_THEORAPLAY = system/graphics/video/theoraplay/
PATH_GRAPHICS_VIDEO = system/graphics/video/
PATH_IO = system/io/
PATH_JAVASCRIPT = system/javascript/
PATH_JAVASCRIPT_BINDINGS = system/javascript/bindings/
PATH_MATH_GENERAL = system/math/general/
PATH_MATH_SORT = system/math/sort/
PATH_MATH_SPLINES = system/math/splines/
PATH_MATH_SPLINES_CUBIC = system/math/splines/cubic/
PATH_PLAYER = system/player/
PATH_ROCKET = system/rocket/
PATH_THREAD = system/thread/
PATH_TIMER = system/timer/
PATH_XML = system/xml/
PATH_UI_ANTTWEAKBAR_INCLUDE = system/ui/AntTweakBar/include/
PATH_UI_ANTTWEAKBAR = system/ui/AntTweakBar/src/
PATH_UI_EDITORS = system/ui/editors/
PATH_UI_INPUT = system/ui/input/
PATH_UI_WINDOW = system/ui/window/

PATH_TEST = test/

TARGET = release/engine
LDFLAGS = -lm -L$(PATH_AUDIO)
#-lsmpeg
#-lvfw_avi32 -lvfw_ms32
INCLUDES = -Iinclude -I.
CFLAGS = -Wall $(ARCHITECTURE) $(ENDIAN) $(INCLUDES)
CXXFLAGS = -Wall -Wextra $(ARCHITECTURE) $(ENDIAN) -fno-exceptions -fno-rtti 
RELEASES = 

ifeq ($(USEMINIMALIST), FALSE)
	#TODO: FIXME
	#CFLAGS += -Werror
	#CXXLAGS += -Werror -pedantic
endif

ifeq ($(CPUOPTIMIZATION), ALTIVEC)
	CFLAGS += -maltivec -mabi=altivec -DALTIVEC
	CXXFLAGS += -maltivec -mabi=altivec	-DALTIVEC
else ifeq ($(CPUOPTIMIZATION), SSE2)
	CFLAGS += -msse2
	CXXFLAGS += -msse2

	#auto vectorization
	# https://www.it.uu.se/edu/course/homepage/hpb/vt12/lab4.pdf
	#JAVASCRIPTFLAGS += -ftree-vectorize  -ftree-slp-vectorize -fopt-info-vec-missed -ftree-vectorizer-verbose=2
endif

#-Wno-unknown-warning-option -Wextra

#Platform standard configurations
ifeq ($(PLATFORM), LINUX)
	CFLAGS += -D__X11__ -D_UNIX
	LDFLAGS += -L/usr/lib/ -L/usr/X11R6/lib/ -lGL -lGLU -lX11

else ifeq ($(PLATFORM), OS_X)
	TARGET = release/engine.app/Contents/MacOS/engine
	CFLAGS += -isysroot /Developer/SDKs/MacOSX10.6.sdk -D_MACOSX 
	LDFLAGS += -Llib/osx -Wl,-syslibroot,/Developer/SDKs/MacOSX10.6.sdk -framework Cocoa -framework OpenGL  -framework AppKit -mmacosx-version-min=10.6

	CFLAGS += -DOS_X -I/System/Library/Frameworks/AGL.framework/Headers
	#for PowerPC - Intel crosscompilation use -arch switch
	#lipo tool can merge the binaries easily
	#CC = gcc-3.3
	#CXX = g++-3.3

	#AntTweakBar not supported in OS X
	USEANTTWEAKBAR = FALSE	

else ifeq ($(PLATFORM), WINDOWS)
	TARGET = release/engine.exe
	INCLUDES += -Iinclude/windows
	LDFLAGS += -Llib -Llib\mingw472
	#_WIN32_WINNT definition needed to get getaddrinfo compiling with MinGW GCC & Win 7
	CFLAGS += -DWINDOWS -D_WIN32_WINNT=0x0501 
	LDFLAGS += -mwindows -lmingw32 -lglu32 -lopengl32 -lstdc++ -lws2_32 -static-libgcc -static-libstdc++

else ifeq ($(PLATFORM), MORPHOS)
	CFLAGS += -DMORPHOS
	LDFLAGS += -lGLU
	CC = ppc-morphos-gcc-4
	CXX = ppc-morphos-g++-4
endif

#sourcefiles in use
JS_SRC = $(PATH_MATH_GENERAL)Matrix.js $(PATH_MATH_GENERAL)Vector.js $(PATH_MATH_SPLINES)CatmullRomSpline.js $(PATH_UI_INPUT)Input.js $(PATH_PLAYER)Utils.js $(PATH_PLAYER)Shader.js $(PATH_PLAYER)Sync.js $(PATH_PLAYER)Settings.js $(PATH_PLAYER)Effect.js $(PATH_PLAYER)Loader.js $(PATH_PLAYER)Player.js

OBJ = $(PATH_SYSTEM)main.o $(PATH_AUDIO)sound.o $(PATH_TIMER)timer.o $(PATH_UI_WINDOW)window.o $(PATH_UI_WINDOW)menu.o $(PATH_PLAYER)player.o $(PATH_GRAPHICS)graphics.o $(PATH_GRAPHICS)camera.o $(PATH_GRAPHICS)texture.o $(PATH_MATH_SPLINES)spline.o $(PATH_MATH_SPLINES_CUBIC)cubicSpline.o $(PATH_GRAPHICS_FONT)font.o $(PATH_GRAPHICS_IMAGE)image.o $(PATH_DATATYPES)datatypes.o $(PATH_DATATYPES)string.o $(PATH_DATATYPES)memory.o $(PATH_MATH_GENERAL)general.o $(PATH_MATH_GENERAL)expr.o $(PATH_EXTENSIONS_GL)gl.o $(PATH_IO)io.o $(PATH_GRAPHICS_SHADER)shader.o $(PATH_GRAPHICS)fbo.o $(PATH_GRAPHICS_OBJECT)vbo.o $(PATH_GRAPHICS_OBJECT)basic3dshapes.o $(PATH_GRAPHICS_OBJECT)lighting.o $(PATH_GRAPHICS_PARTICLE)particle.o $(PATH_THREAD)thread.o

OBJ += $(PATH_ROCKET)synceditor.o  $(PATH_ROCKET)device.o $(PATH_ROCKET)track.o
OBJ += $(PATH_DEBUG)debugPrint.o $(PATH_DEBUG)debugOpenGl.o 
OBJ += $(PATH_EFFECTS)playlist.o $(PATH_EFFECTS)scene_globals.o

ifeq ($(USEJS), TRUE)
	OBJ += $(PATH_JAVASCRIPT)duktape.o $(PATH_JAVASCRIPT)javascript.o
	OBJ += $(PATH_JAVASCRIPT_BINDINGS)opengl.o $(PATH_JAVASCRIPT_BINDINGS)anttweakbar.o $(PATH_JAVASCRIPT_BINDINGS)audio.o $(PATH_JAVASCRIPT_BINDINGS)graphics.o $(PATH_JAVASCRIPT_BINDINGS)ui.o $(PATH_JAVASCRIPT_BINDINGS)player.o $(PATH_JAVASCRIPT_BINDINGS)timer.o $(PATH_JAVASCRIPT_BINDINGS)synceditor.o $(PATH_JAVASCRIPT_BINDINGS)miscellaneous.o $(PATH_JAVASCRIPT_BINDINGS)custom.o
	CFLAGS += -DJAVASCRIPT
	#JAVASCRIPTFLAGS = -I$(PATH_JAVASCRIPT) -DDUK_OPT_DEBUG -DDUK_OPT_DPRINT -DAMIGA -std=c99 -Wno-clobbered -Wno-unused-parameter -Wno-unused-function
	JAVASCRIPTFLAGS = -I$(PATH_JAVASCRIPT) -std=c99 -Wno-clobbered -Wno-unused-parameter -Wno-unused-function

	#Duktape perf optimization flags? Seems to be a bit faster with PPC
	JAVASCRIPTFLAGS += -DDUK_OPT_NO_VOLUNTARY_GC -DDUK_USE_FASTINT -DDUK_USE_VALSTACK_UNSAFE

	ifeq ($(PLATFORM), MORPHOS)
		JAVASCRIPTFLAGS += -DAMIGA
	endif

	ifeq ($(USETINYGL), TRUE)
		OBJ += $(PATH_JAVASCRIPT_BINDINGS)duktape_tinygl.o
	else
		OBJ += $(PATH_JAVASCRIPT_BINDINGS)duktape_opengl.o 
	endif
endif

ifeq ($(USETINYGL), TRUE)
	CFLAGS += -DTINYGL
endif

ifeq ($(PRODUCTION_TYPE), DEMO)
	OBJ += $(PATH_GRAPHICS_OBJECT)object3d.o $(PATH_GRAPHICS_OBJECT_3DS)3dsplay.o
	OBJ += $(PATH_GRAPHICS_OBJECT_LIB3DS)io.o $(PATH_GRAPHICS_OBJECT_LIB3DS)vector.o $(PATH_GRAPHICS_OBJECT_LIB3DS)matrix.o $(PATH_GRAPHICS_OBJECT_LIB3DS)quat.o $(PATH_GRAPHICS_OBJECT_LIB3DS)tcb.o $(PATH_GRAPHICS_OBJECT_LIB3DS)ease.o $(PATH_GRAPHICS_OBJECT_LIB3DS)chunk.o $(PATH_GRAPHICS_OBJECT_LIB3DS)file.o $(PATH_GRAPHICS_OBJECT_LIB3DS)background.o $(PATH_GRAPHICS_OBJECT_LIB3DS)atmosphere.o $(PATH_GRAPHICS_OBJECT_LIB3DS)shadow.o $(PATH_GRAPHICS_OBJECT_LIB3DS)viewport.o $(PATH_GRAPHICS_OBJECT_LIB3DS)material.o $(PATH_GRAPHICS_OBJECT_LIB3DS)mesh.o $(PATH_GRAPHICS_OBJECT_LIB3DS)camera.o $(PATH_GRAPHICS_OBJECT_LIB3DS)light.o $(PATH_GRAPHICS_OBJECT_LIB3DS)tracks.o $(PATH_GRAPHICS_OBJECT_LIB3DS)node.o
	OBJ += $(PATH_GRAPHICS_OBJECT_OBJ)obj.o $(PATH_GRAPHICS_OBJECT_OBJ)mtl.o
	OBJ += $(PATH_XML)yxml.o $(PATH_XML)xml.o $(PATH_GRAPHICS_IMAGE_SVG)svg.o


	ifeq ($(USEVIDEO), TRUE)
		CFLAGS += -DSUPPORT_VIDEO
		OBJ += $(PATH_GRAPHICS_VIDEO)video.o $(PATH_GRAPHICS_VIDEO_THEORAPLAY)theoraplay.o
		ifeq ($(PLATFORM), OS_X)
			LDFLAGS += lib/osx/libogg.a lib/osx/libvorbis.a lib/osx/libtheora.a
		else
			LDFLAGS += -l:libogg.a -lvorbis -l:libtheora.a
		endif
	endif

	ifeq ($(PLATFORM), LINUX)
		LDFLAGS += -lpthread
	endif

	LIB3DSFLAGS = -Wno-unused-parameter -I$(PATH_GRAPHICS_OBJECT)

	CFLAGS += -I$(PATH_GRAPHICS_OBJECT)
	
	ifeq ($(USESDL), TRUE)
		#editor stuff
		OBJ += $(PATH_UI_EDITORS)splineEditor.o
	endif
	
	ifeq ($(CUNIT), TRUE)
		OBJ += $(PATH_TEST)test_main.o
		CFLAGS += -DCUNIT
		LDFLAGS += -l:libcunit.a
	endif

else ifneq ($(DEBUG),TRUE)
	#INTRO without debug
	CFLAGS += -DNDEBUG
endif

#Debug setup
ifeq ($(DEBUG),TRUE)
	CFLAGS += -g -O0
else
	CFLAGS += -O2 -ffast-math
endif

#AntTweakBar setup
ifeq ($(USEANTTWEAKBAR), TRUE)
	OBJ += $(PATH_UI_ANTTWEAKBAR)TwColors.o $(PATH_UI_ANTTWEAKBAR)TwFonts.o $(PATH_UI_ANTTWEAKBAR)TwOpenGL.o $(PATH_UI_ANTTWEAKBAR)TwOpenGLCore.o $(PATH_UI_ANTTWEAKBAR)TwBar.o $(PATH_UI_ANTTWEAKBAR)TwMgr.o $(PATH_UI_ANTTWEAKBAR)TwPrecomp.o $(PATH_UI_ANTTWEAKBAR)LoadOGL.o $(PATH_UI_ANTTWEAKBAR)LoadOGLCore.o

	ifeq ($(USESDL), TRUE)
		OBJ += $(PATH_UI_ANTTWEAKBAR)TwEventSDL.o $(PATH_UI_ANTTWEAKBAR)TwEventSDL12.o $(PATH_UI_ANTTWEAKBAR)TwEventSDL13.o
	endif

	CFLAGS += -DANTTWEAKBAR
	ANTTWEAKFLAGS = -I$(PATH_UI_ANTTWEAKBAR_INCLUDE) -D__PLACEMENT_NEW_INLINE -DTW_STATIC 
	CXXANTTWEAKFLAGS = -fpermissive

	ifeq ($(PLATFORM), OS_X)
		CXXANTTWEAKFLAGS += -ObjC++
	endif
endif

ifeq ($(FONT_ENGINE), NO_FONT)
	CFLAGS += -DNO_FONT
endif

#SDL setup
ifeq ($(USESDL), TRUE)
	OBJ += $(PATH_UI_WINDOW)windowSdl.o $(PATH_UI_INPUT)inputSdl.o
	CFLAGS += -DSDL
	INCLUDES += -Iinclude/SDL

	ifeq ($(PLATFORM), WINDOWS)
		CFLAGS += -Dmain=SDL_main
		LDFLAGS += -lSDLmain -lSDL 
	else ifeq ($(PLATFORM), LINUX)
		CFLAGS += `sdl-config --cflags` 
		LDFLAGS += `sdl-config --libs`
	else ifeq ($(PLATFORM), OS_X)
		OBJ += $(PATH_SYSTEM)SDLMain.o
		LDFLAGS += -framework SDL
	else ifeq ($(PLATFORM), MORPHOS)
		CFLAGS += -noixemul -Ilib/morphos/powersdl_sdk/Developer/usr/local/include
		LDFLAGS += -noixemul -Llib/morphos/powersdl_sdk/Developer/usr/local/lib -lSDL
	endif

	ifeq ($(FONT_ENGINE), SDL_TTF)
		CFLAGS += -DSDL_TTF
		ifneq ($(PLATFORM), OS_X)
			LDFLAGS += -lSDL_ttf
		endif
	endif

#SDL2 setup
else ifeq ($(USESDL2), TRUE)
	OBJ += $(PATH_UI_WINDOW)windowSdl.o $(PATH_UI_INPUT)inputSdl.o
	CFLAGS += -DSDL2
	INCLUDES += -Iinclude/SDL2
	LDFLAGS += -lSDL2main -lSDL2

#WinApi setup (intended for intros)
else ifeq ($(PLATFORM), WINDOWS)
	OBJ += $(PATH_UI_WINDOW)windowWinApi.o $(PATH_UI_WINDOW)inputWinApi.o
endif

#Audio setup
ifeq ($(SOUND), SDL_MIXER)
	CFLAGS += -DSDL_MIXER
	
	ifeq ($(PLATFORM), WINDOWS)
		LDFLAGS += -lSDL_mixer
	else ifeq ($(PLATFORM), LINUX)
		LDFLAGS += -lSDL_mixer
	else ifeq ($(PLATFORM), MORPHOS)
	LDFLAGS += -lSDL_mixer
	else ifeq ($(PLATFORM), OS_X)
		LDFLAGS += -framework SDL_mixer
	endif

else ifeq ($(SOUND), FMOD)
	CFLAGS += -DFMOD

	ifeq ($(PLATFORM), WINDOWS)
		LDFLAGS += -lfmodex
	else ifeq ($(PLATFORM), LINUX)
		LDFLAGS += ./libfmodex.so.4.00.37
	else ifeq ($(PLATFORM), OS_X)
		LDFLAGS += -lfmodex
	endif

else ifeq ($(SOUND), BASS)
	CFLAGS += -DBASS

else ifeq ($(SOUND), MINIFMOD)
	CFLAGS += -DMINIFMOD

	ifeq ($(PLATFORM), WINDOWS)
		OBJ += $(PATH_AUDIO_MINIFMOD)fmusic.o $(PATH_AUDIO_MINIFMOD)fsound.o $(PATH_AUDIO_MINIFMOD)mixer_clipcopy.o $(PATH_AUDIO_MINIFMOD)mixer_fpu_ramp.o $(PATH_AUDIO_MINIFMOD)music_formatxm.o $(PATH_AUDIO_MINIFMOD)system_file.o
		LDFLAGS += $(PATH_AUDIO_MINIFMOD)mixer_fpu_ramp_asm.o
	else ifeq ($(PLATFORM), LINUX)
		LDFLAGS += -lminifmod.linux
	else ifeq ($(PLATFORM), OS_X)
		LDFLAGS += -lminifmod.osx
	endif
endif

#PNG setup
ifeq ($(USEPNG), LIBPNG)
	CFLAGS += -DPNG -DLIBPNG

	ifeq ($(PLATFORM), OS_X)
		LDFLAGS += -lz -framework libpng15
	else ifeq ($(PLATFORM), MORPHOS)
		LDFLAGS += -lpng
	else ifeq ($(PLATFORM), WINDOWS)
		LDFLAGS += -lz -lpng
	else ifeq ($(PLATFORM), LINUX)
		LDFLAGS += -lz -lpng
	endif
else ifeq ($(USEPNG), STBPNG)
	CFLAGS += -DPNG
endif

#Executable linking
ifeq ($(PLATFORM), OS_X)
$(TARGET): $(OBJ)
	$(CXX) $(ARCHITECTURE) -o $@ $(OBJ) $(LDFLAGS) 
	install_name_tool -change @rpath/SDL.framework/Versions/A/SDL @executable_path/../Frameworks/SDL.framework/Versions/A/SDL $(TARGET)
	install_name_tool -change @rpath/SDL_mixer.framework/Versions/A/SDL_mixer @executable_path/../Frameworks/SDL_mixer.framework/Versions/A/SDL_mixer $(TARGET)
else
$(TARGET): $(OBJ)
	$(CXX) $(ARCHITECTURE) -o $@ $(OBJ) $(LDFLAGS) 
endif

#Object file compilation
#NOTE! Order object file compilation from longest to shortest path so that %.o doesn't accidentally recursively search path and compile other objects with wrong flags
$(PATH_JAVASCRIPT_BINDINGS)%.o: $(PATH_JAVASCRIPT_BINDINGS)%.c
	$(CC) $(CFLAGS) $(JAVASCRIPTFLAGS) -o $@ -c $<
	
$(PATH_JAVASCRIPT)%.o: $(PATH_JAVASCRIPT)%.c
	$(CC) $(CFLAGS) $(JAVASCRIPTFLAGS) -o $@ -c $<

$(PATH_XML)%.o: $(PATH_XML)%.c
	$(CC) $(CFLAGS) -I$(PATH_XML) -Wno-unused-parameter -o $@ -c $<

$(PATH_EFFECTS_FILTERS)%.o: $(PATH_EFFECTS_FILTERS)%.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) -o $@ -c $<

$(PATH_EFFECTS_FILTERS)%.o: $(PATH_EFFECTS_FILTERS)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_EFFECTS)%.o: $(PATH_EFFECTS)%.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) -o $@ -c $<

$(PATH_EFFECTS)%.o: $(PATH_EFFECTS)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_AUDIO_MINIFMOD)%.o: $(PATH_AUDIO_MINIFMOD)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_AUDIO)%.o: $(PATH_AUDIO)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_DEBUG)%.o: $(PATH_DEBUG)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_DATATYPES)%.o: $(PATH_DATATYPES)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_EXTENSIONS_GL)%.o: $(PATH_EXTENSIONS_GL)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_GRAPHICS_FONT)%.o: $(PATH_GRAPHICS_FONT)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_GRAPHICS_IMAGE_SVG)%.o: $(PATH_GRAPHICS_IMAGE_SVG)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_GRAPHICS_IMAGE)%.o: $(PATH_GRAPHICS_IMAGE)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_GRAPHICS_OBJECT_3DS)%.o: $(PATH_GRAPHICS_OBJECT_3DS)%.c
	$(CC) $(CFLAGS) $(LIB3DSFLAGS) -o $@ -c $<

$(PATH_GRAPHICS_OBJECT_LIB3DS)%.o: $(PATH_GRAPHICS_OBJECT_LIB3DS)%.c
	$(CC) $(CFLAGS) $(LIB3DSFLAGS) -o $@ -c $<

$(PATH_GRAPHICS_OBJECT_LIBOBJ)%.o: $(PATH_GRAPHICS_OBJECT_LIBOBJ)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_GRAPHICS_OBJECT)%.o: $(PATH_GRAPHICS_OBJECT)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_GRAPHICS_PARTICLE)%.o: $(PATH_GRAPHICS_PARTICLE)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_GRAPHICS_SHADER)%.o: $(PATH_GRAPHICS_SHADER)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_GRAPHICS_VIDEO_THEORAPLAY)%.o: $(PATH_GRAPHICS_VIDEO_THEORAPLAY)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_GRAPHICS_VIDEO)%.o: $(PATH_GRAPHICS_VIDEO)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_GRAPHICS)%.o: $(PATH_GRAPHICS)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_IO)%.o: $(PATH_IO)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_MATH_GENERAL)%.o: $(PATH_MATH_GENERAL)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_MATH_SORT)%.o: $(PATH_MATH_SORT)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_MATH_SPLINES_CUBIC)%.o: $(PATH_MATH_SPLINES_CUBIC)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_MATH_SPLINES)%.o: $(PATH_MATH_SPLINES)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_PLAYER)%.o: $(PATH_PLAYER)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_ROCKET)%.o: $(PATH_ROCKET)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_THREAD)%.o: $(PATH_THREAD)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_TIMER)%.o: $(PATH_TIMER)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_UI_ANTTWEAKBAR)%.o: $(PATH_UI_ANTTWEAKBAR)%.cpp
	$(CXX) $(ANTTWEAKFLAGS) $(CXXANTTWEAKFLAGS) -o $@ -c $<

$(PATH_UI_ANTTWEAKBAR)%.o: $(PATH_UI_ANTTWEAKBAR)%.c
	$(CC) $(ANTTWEAKFLAGS) $(CFLAGS) -o $@ -c $<

$(PATH_UI_EDITORS)%.o: $(PATH_UI_EDITORS)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_UI_INPUT)%.o: $(PATH_UI_INPUT)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_UI_WINDOW)%.o: $(PATH_UI_WINDOW)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_SYSTEM)%.o: $(PATH_SYSTEM)%.m
	$(CC) $(CFLAGS) -Wno-objc-protocol-method-implementation -Wno-incompatible-pointer-types-discards-qualifiers -Wno-deprecated-declarations -Wno-unused-parameter -Wno-int-conversion -o $@ -c $<

$(PATH_SYSTEM)%.o: $(PATH_SYSTEM)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(PATH_TEST)%.o: $(PATH_TEST)%.c
	$(CC) $(CFLAGS) -o $@ -c $<

#other commands
.PHONY: clean documentation js

release: clean js $(TARGET) documentation

clean:
	 $(RM) $(OBJ) $(TARGET)

js:
	$(GJSLINT) --nojsdoc --max_line_length=150 $(JS_SRC)
	$(JAVA) -jar utils/closure/compiler.jar --jscomp_off=missingProperties --jscomp_off=undefinedVars --warning_level=VERBOSE --compilation_level SIMPLE_OPTIMIZATIONS --language_in=ECMASCRIPT5 --language_out=ECMASCRIPT5 --js_output_file=release/engine.js $(JS_SRC)

# npm install -g markdown-pdf
# http://www.doxygen.org/
# http://miktex.org/download
documentation:
	$(MARKDOWN_PDF) --paper-orientation portrait --paper-format A4 --out documentation/README.pdf ../README.md
	$(DOXYGEN) doxygen.config
	$(PERL) doxygen_to_pdf.pl
