#include <stdio.h>
#include <assert.h>

#include "system/graphics/object/lib3ds/types.h"
#include "system/graphics/object/lib3ds/viewport.h"
#include "system/graphics/object/lib3ds/shadow.h"
#include "system/graphics/object/lib3ds/atmosphere.h"
#include "system/graphics/object/lib3ds/background.h"
#include "system/graphics/object/lib3ds/file.h"
#include "system/graphics/object/lib3ds/camera.h"
#include "system/graphics/object/lib3ds/mesh.h"
#include "system/graphics/object/lib3ds/tcb.h"
#include "system/graphics/object/lib3ds/tracks.h"
#include "system/graphics/object/lib3ds/node.h"
#include "system/graphics/object/lib3ds/material.h"
#include "system/graphics/object/lib3ds/matrix.h"
#include "system/graphics/object/lib3ds/vector.h"
#include "system/graphics/object/lib3ds/light.h"

#include "system/io/io.h"
#include "system/graphics/graphics.h"
#include "system/debug/debug.h"
#include "system/graphics/object/3ds/3dsplay.h"
#include "system/graphics/object/basic3dshapes.h"
#include "system/datatypes/memory.h"

#include "object3d.h"

int replaceObjectTexture(object3d_t *object, const char *findTextureName, const char *replaceTextureName)
{
	return replace_model_3ds_material_texture(object, findTextureName, replaceTextureName);
}

void objectInit(object3d_t* object)
{
	object->data.file = NULL;
	object->data.quadric = NULL;
	object->filename = NULL;
	object->tex_mode = 0;
	object->clearZBuffer = 0;
	object->camera = NULL;
	object->objectType = BASIC_3D_SHAPE_COMPLEX_3DS;
	object->useObjectCamera = 1;
	object->useObjectLighting = 1;
	object->useObjectNormals = 1;
	object->useObjectTextureCoordinates = 1;
	object->useSimpleColors = 0;

	object->vertexTransform = NULL;
	
	object->scale.x = 1.0;
	object->scale.y = 1.0;
	object->scale.z = 1.0;

	object->position.x = 0;
	object->position.y = 0;
	object->position.z = 0;
	
	object->pivot.x = 0;
	object->pivot.y = 0;
	object->pivot.z = 0;

	object->degrees.x = 0;
	object->degrees.y = 0;
	object->degrees.z = 0;

	object->angle.x = 0;
	object->angle.y = 0;
	object->angle.z = 0;
	
	object->color.r = 1;
	object->color.g = 1;
	object->color.b = 1;
	object->color.a = 1;
}

void* loadObjectBasicShape(const char * name, int objectType)
{
	object3d_t *object = getObjectFromMemory(name);
	if (object != NULL)
	{
		return object;
	}

	object = memoryAllocateObject(NULL);
	objectInit(object);
	object->filename = strdup(name);

	debugPrintf("Loading basic 3D shape '%s'", object->filename);
	
	object->objectType = objectType;

	return object;
}

static int obj_object_gl_bind_face_texture(obj_face_t *face)
{
	assert(face);
	obj_material_t *obj_material = face->face_parameters.material;

	int enabledTextures = 0;

	if (obj_material)
	{
		texture_t *texture = obj_material->ambient.texture;
		if (texture == NULL)
		{
			texture = obj_material->diffuse.texture;
			if (texture == NULL)
			{
				texture = obj_material->specular.texture;
			}
		}

		if (texture)
		{
			enabledTextures++;
			glActiveTexture(GL_TEXTURE0);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, texture->id);
		}

		if (obj_material->reflection.texture != NULL)
		{
			enabledTextures++;
			glActiveTexture(GL_TEXTURE1);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, obj_material->reflection.texture->id);

			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
			glEnable(GL_TEXTURE_GEN_T);
			glEnable(GL_TEXTURE_GEN_S);
		}
	}

	return enabledTextures;
}

static void obj_object_gl_legacy_render(object3d_t *object_main, obj_object_t *object)
{
	assert(object_main);
	assert(object);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glEnable(GL_TEXTURE_2D);

	//WARNING: 3D models should be exported so that there's only one type of faces
	//rendering will go off if you mix triangles and quads etc... in single mesh
	obj_face_t *face = object->faces[0];
	//int enabledTextures = obj_object_gl_bind_face_texture(face);
	switch(face->size)
	{
		case 4:
			glBegin(GL_QUADS);
			break;
		case 3:
			glBegin(GL_TRIANGLES);
			break;
		default:
			debugErrorPrintf("Face size not supported!");
			assert(2==1);
			break;
	}

	unsigned int i = 0;
	for(i = 0; i < object->faces_size; i++)
	{
		face = object->faces[i];
		//enabledTextures = obj_object_gl_bind_face_texture(face);


		unsigned char vertex_i;
		for(vertex_i = 0; vertex_i < face->size; vertex_i++)
		{
			obj_vertex_normal_t *normal = face->normals[vertex_i];
			obj_vertex_texture_coordinate_t *texture = face->texture_coordinates[vertex_i];
			obj_vertex_t *vertex = face->vertices[vertex_i];

			if (normal != NULL && object_main->useObjectNormals)
			{
				glNormal3f(normal->xyz.x, normal->xyz.y, normal->xyz.z);
			}
			if (texture != NULL && object_main->useObjectTextureCoordinates)
			{
				//glMultiTexCoord2f(GL_TEXTURE0, texture->uv.u, texture->uv.v);
				//if (enabledTextures > 1)
				{
					//glMultiTexCoord2f(GL_TEXTURE1, texture->uv.u, texture->uv.v);
				}
			}

			float x = vertex->xyz.x;
			float y = vertex->xyz.y;
			float z = vertex->xyz.z;
			if (object_main->vertexTransform != NULL) {
				object_main->vertexTransform(&x,&y,&z);
			}
			glVertex3f(x,y,z);
		}
	}

	glEnd();

	/*if (enabledTextures > 1)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_S);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);*/
	//glEnd();

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glDisable(GL_TEXTURE_2D);
}

static void obj_container_gl_legacy_render(object3d_t *object)
{
	assert(object);
	assert(object->objectType == BASIC_3D_SHAPE_COMPLEX_OBJ);

	obj_container_t *container = object->data.obj;
	assert(container);
	unsigned int i;

	if (container->objects)
	{
		for(i = 0; i < container->objects_size; i++)
		{
			obj_object_gl_legacy_render(object, container->objects[i]);
		}
	}
}

static object3d_t *loadObjectObj(const char *filepath)
{
	object3d_t *object = getObjectFromMemory(filepath);
	if (object != NULL) {
		return object;
	}
	debugPrintf("Loading object '%s'", filepath);
	object = memoryAllocateObject(NULL);
	objectInit(object);
	assert(object != NULL);

	object->data.obj=obj_file_load(filepath);
	if (!object->data.obj) {
		debugErrorPrintf("Loading of OBJ file '%s' failed.", filepath);
		return NULL;
	}

	object->filename = strdup(filepath);
	object->objectType = BASIC_3D_SHAPE_COMPLEX_OBJ;

	return object;
}

void* loadObject(const char * filename)
{
	object3d_t *object = NULL;
	if (endsWithIgnoreCase(filename, ".3ds"))
	{
		object = load_model_3ds(getFilePath(filename));
		object->objectType = BASIC_3D_SHAPE_COMPLEX_3DS;
	}
	else if (endsWithIgnoreCase(filename, ".obj"))
	{
		object = loadObjectObj(getFilePath(filename));
	}
	else
	{
		debugErrorPrintf("3d object format not recognized, use 3ds or obj! filename:'%s'", filename);
	}
	assert(object != NULL);

	return (void*)object;
}

void useObjectLighting(object3d_t* object, int useObjectLighting)
{
	assert(object != NULL);
	
	object->useObjectLighting = useObjectLighting;
}

void useObjectCamera(object3d_t* object, int useObjectCamera)
{
	assert(object != NULL);
	
	object->useObjectCamera = useObjectCamera;
}

void useObjectNormals(object3d_t* object, int useObjectNormals)
{
	assert(object != NULL);
	
	object->useObjectNormals = useObjectNormals;
}

void useObjectTextureCoordinates(object3d_t* object, int useObjectTextureCoordinates)
{
	assert(object != NULL);
	
	object->useObjectTextureCoordinates = useObjectTextureCoordinates;
}

void useSimpleColors(object3d_t* object, int useSimpleColors)
{
	assert(object != NULL);
	
	object->useSimpleColors = useSimpleColors;
}

void setObjectScale(object3d_t* object, float x, float y, float z)
{
	assert(object != NULL);
	
	object->scale.x = x;
	object->scale.y = y;
	object->scale.z = z;
}

void setObjectPosition(object3d_t* object, float x, float y, float z)
{
	assert(object != NULL);
	
	object->position.x = x;
	object->position.y = y;
	object->position.z = z;
}

void setObjectPivot(object3d_t* object, float x, float y, float z)
{
	assert(object != NULL);
	
	object->pivot.x = x;
	object->pivot.y = y;
	object->pivot.z = z;
}

void setObjectRotation(object3d_t* object, float degreesX, float degreesY, float degreesZ, float x, float y, float z)
{
	assert(object != NULL);
	
	object->degrees.x = degreesX;
	object->degrees.y = degreesY;
	object->degrees.z = degreesZ;

	object->angle.x = x;
	object->angle.y = y;
	object->angle.z = z;
}

void setObjectColor(object3d_t* object, float r, float g, float b, float a)
{
	assert(object != NULL);
	
	object->color.r = r;
	object->color.g = g;
	object->color.b = b;
	object->color.a = a;
}

void drawObject(void* object_ptr, const char* displayCamera, double displayFrame, int clear)
{
	object3d_t *object = (object3d_t*)object_ptr;
	assert(object != NULL);

	object->camera = displayCamera;
	object->clearZBuffer = clear;

	if (object->clearZBuffer)
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	/*glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	glDisable(GL_NORMALIZE);
	glPolygonOffset(1.0, 2);*/

	if (object->objectType == BASIC_3D_SHAPE_COMPLEX_3DS)
	{
		assert(object->data.file);
		displayFrame = fmod(displayFrame, object->data.file->frames);
		lib3ds_file_eval(object->data.file, displayFrame);
	}
	else if (object->objectType != BASIC_3D_SHAPE_COMPLEX_OBJ)
	{
		assert(object->data.quadric);
	}


	glEnable(GL_DEPTH_TEST);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glEnable(GL_BLEND);

	if (object->objectType != BASIC_3D_MATRIX)
	{
		glPushMatrix();
	}

	glTranslatef(object->position.x, object->position.y, object->position.z);

	glScalef(object->scale.x, object->scale.y, object->scale.z);

	glTranslatef(object->pivot.x, object->pivot.y, object->pivot.z);

	glRotatef(object->degrees.x, -object->angle.x,                0,                0);
	glRotatef(object->degrees.y,                0, -object->angle.y,                0);
	glRotatef(object->degrees.z,                0,                0, -object->angle.z);

	glTranslatef(-object->pivot.x, -object->pivot.y, -object->pivot.z);

	//glColor4f(object->color.r,object->color.g,object->color.b,object->color.a);

	if (object->objectType != BASIC_3D_MATRIX)
	{
		switch(object->objectType)
		{
			case BASIC_3D_SHAPE_COMPLEX_3DS:
				display3ds(object);
				break;
			case BASIC_3D_SHAPE_COMPLEX_OBJ:
				obj_container_gl_legacy_render(object);
				break;
			case BASIC_3D_SHAPE_CUBE:
				drawObjectCube(object);
				break;
			default:
				debugErrorPrintf("3d object type incorrect! name:'%s', type:'%d'", object->filename, object->objectType);
				break;
		}
	
		glPopMatrix();
	}


	//glDisable(GL_BLEND);
}

void objectDeinit(object3d_t* object)
{
	assert(object);
	debugPrintf("Cleaning '%s'", object->filename);

    if (object->objectType == BASIC_3D_SHAPE_COMPLEX_3DS)
    {
        if (object->data.file)
        {
			free_model_3ds(object->data.file);
        }
    }
    else if (object->objectType == BASIC_3D_SHAPE_COMPLEX_OBJ)
    {
        if (object->data.obj)
        {
			obj_container_free(object->data.obj);
        }
    }
	
	free(object->filename);
}
