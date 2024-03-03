#ifndef EXH_SYSTEM_GRAPHICS_OBJECT_OBJECT3D_H_
#define EXH_SYSTEM_GRAPHICS_OBJECT_OBJECT3D_H_

#include "system/graphics/object/lib3ds/types.h"
#include "system/graphics/object/obj/obj.h"
#include "system/datatypes/datatypes.h"

#define BASIC_3D_SHAPE_COMPLEX 0
#define BASIC_3D_SHAPE_CYLINDER 1
#define BASIC_3D_SHAPE_DISK 2
#define BASIC_3D_SHAPE_SPHERE 3
#define BASIC_3D_SHAPE_CUBE 4
#define BASIC_3D_MATRIX 5
#define BASIC_3D_SHAPE_COMPLEX_3DS 6
#define BASIC_3D_SHAPE_COMPLEX_OBJ 7

typedef struct object_shape_disk_t {
	double inner;
	double outer;
	int slices;
	int loops;
} object_shape_disk_t;

typedef struct object_shape_cylinder_t {
	double base;
	double top;
	double height;
	int slices;
	int stacks;
} object_shape_cylinder_t;

typedef struct object_shape_sphere_t {
	double radius;
	int lats;
	int longs;
} object_shape_sphere_t;

typedef struct object3d_t object3d_t;

struct object3d_t
{
	char *filename;
	int tex_mode; // Texturing active ? 
	int clearZBuffer;
	int objectType;
	int useObjectCamera;
	int useObjectLighting;
	int useObjectNormals;
	int useObjectTextureCoordinates;
	int useSimpleColors;
	const char *camera;

	point3d_t position;
	point3d_t scale;
	point3d_t pivot;
	point3d_t degrees;
	point3d_t angle;
	color_t color;

	void (*vertexTransform)(float*, float*, float*);
	
	union data {
		Lib3dsFile *file;
		obj_container_t *obj;
		GLUquadric *quadric;
	} data;

	union shape {
		object_shape_disk_t disk;
		object_shape_cylinder_t cylinder;
		object_shape_sphere_t sphere;
	} shape;
};

extern void objectInit(object3d_t* object);

extern void useObjectLighting(object3d_t* object, int useObjectLighting);
extern void useObjectCamera(object3d_t* object, int useObjectCamera);
extern void useObjectNormals(object3d_t* object, int useObjectNormals);
extern void useObjectTextureCoordinates(object3d_t* object, int useObjectTextureCoordinates);
extern void useSimpleColors(object3d_t* object, int useSimpleColors);
extern void setObjectScale(object3d_t* object, float x, float y, float z);
extern void setObjectPosition(object3d_t* object, float x, float y, float z);
extern void setObjectPivot(object3d_t* object, float x, float y, float z);
extern void setObjectRotation(object3d_t* object, float degreesX, float degreesY, float degreesZ, float x, float y, float z);
extern void setObjectColor(object3d_t* object, float r, float g, float b, float a);

extern void drawObject(void* object_ptr, const char* displayCamera, double displayFrame, int clear);
extern void* loadObjectBasicShape(const char * name, int objectType);
extern void *loadObject(const char * filename);
extern int replaceObjectTexture(object3d_t *object, const char *findTextureName, const char *replaceTextureName);

extern void objectDeinit(object3d_t* object);

#endif /*EXH_SYSTEM_GRAPHICS_OBJECT_OBJECT3D_H_*/
