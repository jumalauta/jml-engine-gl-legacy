#ifndef EXH_SYSTEM_GRAPHICS_OBJECT_BASIC3DSHAPES_H_
#define EXH_SYSTEM_GRAPHICS_OBJECT_BASIC3DSHAPES_H_

#include "system/graphics/object/object3d.h"

extern object3d_t* setObjectCubeData(const char *name, object3d_t *object);

extern void drawObjectCube(object3d_t *object);
extern void drawObjectPyramid(object3d_t *object);

#endif /*EXH_SYSTEM_GRAPHICS_OBJECT_BASIC3DSHAPES_H_*/
