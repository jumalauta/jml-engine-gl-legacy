#include <math.h>

#include "graphicsIncludes.h"
#include "system/graphics/graphics.h"
#include "system/extensions/gl/gl.h"
#include "system/timer/timer.h"
#include "system/graphics/image/image.h"
#include "system/player/player.h"
#include "system/debug/debug.h"
#include "system/math/general/general.h"
#include "system/datatypes/datatypes.h"
#include "system/javascript/javascript.h"
#include "system/graphics/font/font.h"

#define s(s) convertTimeToSeconds((s))

/**
 * Player global setup (like shader) initialization.
 * @ingroup player
 */
void sceneGlobalsInit(playerScene * UNUSED(scene))
{
	fontInit();

#ifdef JAVASCRIPT
	jsEvalString("Shader.compileAndLinkShaders();");
#endif
}

void sceneGlobalsRun(playerScene * UNUSED(scene))
{
}

/**
 * Player global setup deinitialization.
 * @ingroup player
 */
void sceneGlobalsDeinit(playerScene * UNUSED(scene))
{
	fontDeinit();
}

//custom demo global functions
