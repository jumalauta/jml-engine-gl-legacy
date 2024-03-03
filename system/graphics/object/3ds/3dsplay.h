#ifndef EXH_SYSTEM_GRAPHICS_OBJECT_3DS_3DSPLAY_H_
#define EXH_SYSTEM_GRAPHICS_OBJECT_3DS_3DSPLAY_H_

#include "system/graphics/object/object3d.h"

extern int replace_model_3ds_material_texture(object3d_t *object, const char *findTextureName, const char *replaceTextureName);
extern void free_model_3ds(Lib3dsFile *file);
extern object3d_t* load_model_3ds(const char *filepath);
extern void display3ds(object3d_t *object);

#endif /*EXH_SYSTEM_GRAPHICS_OBJECT_3DS_3DSPLAY_H_*/
