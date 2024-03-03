/*
* The 3D Studio File Format Library
* Copyright (C) 1996-2007 by Jan Eric Kyprianidis <www.lib3ds.org>
* All rights reserved.
*
* This program is  free  software;  you can redistribute it and/or modify it
* under the terms of the  GNU Lesser General Public License  as published by 
* the  Free Software Foundation;  either version 2.1 of the License,  or (at 
* your option) any later version.
*
* This  program  is  distributed in  the  hope that it will  be useful,  but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or  FITNESS FOR A  PARTICULAR PURPOSE.  See the  GNU Lesser General Public  
* License for more details.
*
* You should  have received  a copy of the GNU Lesser General Public License
* along with  this program;  if not, write to the  Free Software Foundation,
* Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
* $Id: 3dsplay.c,v 1.14 2007/06/18 06:51:53 jeh Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <lib3ds/types.h>
#include <lib3ds/file.h>
#include <lib3ds/camera.h>
#include <lib3ds/mesh.h>
#include <lib3ds/node.h>
#include <lib3ds/material.h>
#include <lib3ds/matrix.h>
#include <lib3ds/vector.h>
#include <lib3ds/light.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>

#include <graphicsIncludes.h>
#include "system/ui/window/window.h"
#include "system/graphics/graphics.h"
#include "system/graphics/image/image.h"
#include "system/extensions/gl/gl.h"
#include "system/datatypes/memory.h"
#include "system/graphics/object/object3d.h"


/*// OS X has a different path than everyone else
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
*/

//#define USE_SDL
#ifdef	USE_SDL
#include <SDL_image.h>
#endif

#define	MOUSE_SCALE	.1	/* degrees/pixel movement */

/*!
\example player.c

Previews a <i>3DS</i> file using OpenGL.

\code
Syntax: player filename
\endcode

\warning To compile this program you must have OpenGL and glut installed.
*/


typedef	enum {ROTATING, WALKING} RunMode;

//static	RunMode runMode = ROTATING;

//static const char* filepath=NULL;
//static char datapath[256];
//static char filename[256];
//static int dbuf=1;
//static int halt=0;
//static int flush=0;
static int anti_alias=1;

//static const char* camera=0;
//static Lib3dsFile *file=0;
//static float current_frame=0.0;
//static int gl_width;
//static int gl_height;
//static int menu_id=0;

static int show_object=1;
//static int show_bounds=0;
//static int rotating = 0;
//static int show_cameras = 0;
//static int show_lights = 0;

//static int cameraList, lightList;	/* Icon display lists */

static Lib3dsVector bmin, bmax;
static float	sx, sy, sz, size;	/* bounding box dimensions */
static float	cx, cy, cz;		/* bounding box center */

static	float	view_rotx = 0., view_roty = 0., view_rotz = 0.;
//static	float	anim_rotz = 0.;

//static	int	mx, my;
/*
static const GLfloat white[4] = {1.,1.,1.,1.};
static const GLfloat dgrey[4] = {.25,.25,.25,1.};
static const GLfloat grey[4] = {.5,.5,.5,1.};
static const GLfloat lgrey[4] = {.75,.75,.75,1.};
static const GLfloat black[] = {0.,0.,0.,1.};
static const GLfloat red[4] = {1.,0.,0.,1.};
static const GLfloat green[4] = {0.,1.,0.,1.};
static const GLfloat blue[4] = {0.,0.,1.,1.};
*/

// texture size: by now minimum standard
#define	TEX_XSIZE	1024
#define	TEX_YSIZE	1024

struct _player_texture
{
  int valid; // was the loading attempt successful ? 
#ifdef	USE_SDL
  SDL_Surface *bitmap;
#else
  void *bitmap;
#endif

  float scale_x, scale_y; // scale the texcoords, as OpenGL thinks in TEX_XSIZE and TEX_YSIZE
  texture_t* texture;
    texture_t* texture1_map;
    texture_t*  texture1_mask;
     texture_t* texture2_map;
     texture_t* texture2_mask;
     texture_t* opacity_map;
     texture_t* opacity_mask;
     texture_t* bump_map;
     texture_t* bump_mask;
     texture_t* specular_map;
     texture_t* specular_mask;
     texture_t* shininess_map;
     texture_t* shininess_mask;
     texture_t* self_illum_map;
     texture_t* self_illum_mask;
     texture_t* reflection_map;
     texture_t* reflection_mask;
};

typedef struct _player_texture Player_texture; 

typedef struct mesh_extension mesh_extension;
struct mesh_extension
{
	vbo_t *vbo;
	Lib3dsMaterial *material;
	GLuint list;
};

#define	NA(a)	(sizeof(a)/sizeof(a[0]))

#ifndef	MIN
#define	MIN(a,b) ((a)<(b)?(a):(b))
#define	MAX(a,b) ((a)>(b)?(a):(b))
#endif

static int check_replace_material_texture(object3d_t *object, Lib3dsMaterial *mat, const char *mapType, Lib3dsTextureMap* map, const char *findTextureName, texture_t *findTexture, texture_t *replaceTexture)
{
  if ((findTexture != NULL && map->user.p == findTexture) || (findTexture == NULL && !strcmp(findTextureName, map->name))) {
	map->user.p = replaceTexture;
	debugPrintf("Object '%s': material '%s' %s '%s' replaced by '%s'!", object->filename, mat->name, mapType, findTextureName, replaceTexture->name);
	return 1;
  }

  return 0;
}

int replace_model_3ds_material_texture(object3d_t *object, const char *findTextureName, const char *replaceTextureName)
{
  assert(object);
  
  int replacedTextureCount = 0;
  
  texture_t *findTexture = imageLoadImage(findTextureName);
  texture_t *replaceTexture = imageLoadImage(replaceTextureName);
  if (!replaceTexture)
  {
	debugWarningPrintf("replaceTexture '%s' not found!", replaceTextureName);
	return replacedTextureCount;
  }

  Lib3dsFile *file = object->data.file;
  if (file) {
    Lib3dsMaterial *mat = NULL;
    for (mat=file->materials; mat; mat=mat->next) {
	  replacedTextureCount += check_replace_material_texture(object, mat, "texture1", &mat->texture1_map, findTextureName, findTexture, replaceTexture);
	  replacedTextureCount += check_replace_material_texture(object, mat, "texture2", &mat->texture2_map, findTextureName, findTexture, replaceTexture);
	  replacedTextureCount += check_replace_material_texture(object, mat, "reflection texture", &mat->reflection_map, findTextureName, findTexture, replaceTexture);
    }
  }
  
  if (replacedTextureCount == 0)
  {
    debugWarningPrintf("Object '%s': Material with texture '%s' not found! Replaced by '%s' ignored.", object->filename, findTextureName, replaceTextureName);
  }
 
  return replacedTextureCount;
}

static void loadModelTextures(object3d_t *object);

void free_model_3ds(Lib3dsFile *file)
{
	assert(file);
	
	Lib3dsMesh *mesh = NULL;

	for (mesh=file->meshes; mesh; mesh=mesh->next)
	{
		if (mesh->user.p)
		{
			free(mesh->user.p);
		}
	}

	lib3ds_file_free(file);
}

static void light_update(Lib3dsFile *file, Lib3dsLight *l);

/*!
* Load the model from .3ds file.
*/
object3d_t *
load_model_3ds(const char *filepath)
{
  object3d_t *object = getObjectFromMemory(filepath);
  if (object != NULL) {
	return object;
  }
  debugPrintf("Loading object '%s'", filepath);
  object = memoryAllocateObject(NULL);
  objectInit(object);
  assert(object != NULL);

  //object->data.file = (Lib3dsFile*)malloc(sizeof(Lib3dsFile));
  //assert(object->data.file != NULL);

  object->data.file=lib3ds_file_load(filepath);
  if (!object->data.file) {
    debugErrorPrintf("Loading of 3DS file '%s' failed.", filepath);
    return NULL;
  }

  object->filename = strdup(filepath);

  /* No nodes?  Fabricate nodes to display all the meshes. */
  if( !object->data.file->nodes )
  {
	debugPrintf("No nodes in '%s'", object->filename);
    Lib3dsMesh *mesh;
    Lib3dsNode *node;

    for(mesh = object->data.file->meshes; mesh != NULL; mesh = mesh->next)
    {
      node = lib3ds_node_new_object();
      strcpy(node->name, mesh->name);
      node->parent_id = LIB3DS_NO_PARENT;
      lib3ds_file_insert_node(object->data.file, node);
	  debugPrintf("Creating dummy node '%s'", node->name);
    }
  }

  lib3ds_file_eval(object->data.file, 1.0f);
  lib3ds_file_bounding_box_of_nodes(object->data.file, LIB3DS_TRUE, LIB3DS_FALSE, LIB3DS_FALSE, bmin, bmax);
  debugPrintf("%s bounding box - min x:%.2f, y:%.2f, z:%.2f max x:%.2f, y:%.2f, z:%.2f", object->filename, bmin[0], bmin[1], bmin[2], bmax[0], bmax[1], bmax[2]);
  sx = bmax[0] - bmin[0];
  sy = bmax[1] - bmin[1];
  sz = bmax[2] - bmin[2];
  size = MAX(sx, sy); size = MAX(size, sz);
  cx = (bmin[0] + bmax[0])/2;
  cy = (bmin[1] + bmax[1])/2;
  cz = (bmin[2] + bmax[2])/2;


  /* No cameras in the file?  Add four */

  if( !object->data.file->cameras ) {
	debugPrintf("No cameras in '%s'", object->filename);
	object->useObjectCamera = 0;
    /* Add some cameras that encompass the bounding box */

    Lib3dsCamera *camera = lib3ds_camera_new("Camera_X");
    camera->target[0] = cx;
    camera->target[1] = cy;
    camera->target[2] = cz;
    memcpy(camera->position, camera->target, sizeof(camera->position));
    camera->position[0] = bmax[0] + 1.5 * MAX(sy,sz);
    camera->near_range = ( camera->position[0] - bmax[0] ) * .5;
    camera->far_range = ( camera->position[0] - bmin[0] ) * 2;
    lib3ds_file_insert_camera(object->data.file, camera);

    /* Since lib3ds considers +Y to be into the screen, we'll put
    * this camera on the -Y axis, looking in the +Y direction.
    */
    camera = lib3ds_camera_new("Camera_Y");
    camera->target[0] = cx;
    camera->target[1] = cy;
    camera->target[2] = cz;
    memcpy(camera->position, camera->target, sizeof(camera->position));
    camera->position[1] = bmin[1] - 1.5 * MAX(sx,sz);
    camera->near_range = ( bmin[1] - camera->position[1] ) * .5;
    camera->far_range = ( bmax[1] - camera->position[1] ) * 2;
    lib3ds_file_insert_camera(object->data.file, camera);

    camera = lib3ds_camera_new("Camera_Z");
    camera->target[0] = cx;
    camera->target[1] = cy;
    camera->target[2] = cz;
    memcpy(camera->position, camera->target, sizeof(camera->position));
    camera->position[2] = bmax[2] + 1.5 * MAX(sx,sy);
    camera->near_range = ( camera->position[2] - bmax[2] ) * .5;
    camera->far_range = ( camera->position[2] - bmin[2] ) * 2;
    lib3ds_file_insert_camera(object->data.file, camera);

    camera = lib3ds_camera_new("Camera01");
    camera->target[0] = cx;
    camera->target[1] = cy;
    camera->target[2] = cz;
    memcpy(camera->position, camera->target, sizeof(camera->position));
    camera->position[2] = bmax[2] + 1.5 * MAX(sx,sy);
    camera->near_range = ( camera->position[2] - bmax[2] ) * .5;
    camera->far_range = ( camera->position[2] - bmin[2] ) * 2;
    lib3ds_file_insert_camera(object->data.file, camera);

    camera = lib3ds_camera_new("Camera_ISO");
    camera->target[0] = cx;
    camera->target[1] = cy;
    camera->target[2] = cz;
    memcpy(camera->position, camera->target, sizeof(camera->position));
    camera->position[0] = bmax[0] + .75 * size;
    camera->position[1] = bmin[1] - .75 * size;
    camera->position[2] = bmax[2] + .75 * size;
    camera->near_range = ( camera->position[0] - bmax[0] ) * .5;
    camera->far_range = ( camera->position[0] - bmin[0] ) * 3;
    lib3ds_file_insert_camera(object->data.file, camera);
  }


  /* No lights in the file?  Add some. */

  if (object->data.file->lights == NULL)
  {
	debugPrintf("No lights in '%s'", object->filename);
	object->useObjectLighting = 0;
    Lib3dsLight *light;

    light = lib3ds_light_new("light0");
    light->spot_light = 0;
    light->see_cone = 0;
    light->color[0] = light->color[1] = light->color[2] = .6;
    light->position[0] = cx + size * .75;
    light->position[1] = cy - size * 1.;
    light->position[2] = cz + size * 1.5;
    //light->position[3] = 0.;
    light->outer_range = 100;
    light->inner_range = 10;
    light->multiplier = 1;
    lib3ds_file_insert_light(object->data.file, light);

    light = lib3ds_light_new("light1");
    light->spot_light = 0;
    light->see_cone = 0;
    light->color[0] = light->color[1] = light->color[2] = .3;
    light->position[0] = cx - size;
    light->position[1] = cy - size;
    light->position[2] = cz + size * .75;
    //light->position[3] = 0.;
    light->outer_range = 100;
    light->inner_range = 10;
    light->multiplier = 1;
    lib3ds_file_insert_light(object->data.file, light);

    light = lib3ds_light_new("light2");
    light->spot_light = 0;
    light->see_cone = 0;
    light->color[0] = light->color[1] = light->color[2] = .3;
    light->position[0] = cx;
    light->position[1] = cy + size;
    light->position[2] = cz + size;
    //light->position[3] = 0.;
    light->outer_range = 100;
    light->inner_range = 10;
    light->multiplier = 1;
    lib3ds_file_insert_light(object->data.file, light);

  }

  if (!object->data.file->cameras) {
    debugErrorPrintf("No Camera found!");
    lib3ds_file_free(object->data.file);
    object->data.file=0;
    return NULL;
  }
object->camera=object->data.file->cameras->name;

  lib3ds_file_eval(object->data.file,0.);
 
 loadModelTextures(object);

 if (object->useObjectLighting)
 {
    static GLfloat c[] = {1.0f, 1.0f, 1.0f, 1.0f};
    static GLfloat p[] = {0.0f, 0.0f, 0.0f, 1.0f};
     //static const GLfloat a[] = {0.0f, 0.0f, 0.0f, 1.0f};
    Lib3dsLight *l;
    unsigned int li=0;
    for (l=object->data.file->lights; l; l=l->next) {
      light_update(object->data.file, l);

      c[0] = l->color[0];
      c[1] = l->color[1];
      c[2] = l->color[2];
      p[0] = l->position[0];
      p[1] = l->position[1];
      p[2] = l->position[2];
      debugPrintf("light %d: r: %.2f, g: %.2f, b: %.2f", li, c[0], c[1], c[2]);
      debugPrintf("light %d position: x: %.2f, y: %.2f, z: %.2f", li, p[0], p[1], p[2]);
                 
//glColor3f(c[0], c[1], c[2]);

      if (l->spot_light) {
        p[0] = l->spot[0] - l->position[0];
        p[1] = l->spot[1] - l->position[1];
        p[2] = l->spot[2] - l->position[2];
      debugPrintf("light %d spot direction: x: %.2f, y: %.2f, z: %.2f", li, p[0], p[1], p[2]);
      }
      ++li;
    }
 }
//viewReset(); 
  //display3ds(object, 1);
  
  return object;
}



#ifdef  USE_SDL
/**
* Convert an SDL bitmap for use with OpenGL.
*
* Written by Gernot < gz@lysator.liu.se >
*/
void *convert_to_RGB_Surface(SDL_Surface *bitmap)
{
  unsigned char *pixel = (unsigned char *)malloc(sizeof(char) * 4 * bitmap->h * bitmap->w); 
  int soff = 0;   
  int doff = 0;   
  int x, y;
  unsigned char *spixels = (unsigned char *)bitmap->pixels;
  SDL_Palette *pal = bitmap->format->palette; 

  for (y = 0; y < bitmap->h; y++)
    for (x = 0; x < bitmap->w; x++)
    {
      SDL_Color* col = &pal->colors[spixels[soff]];

      pixel[doff] = col->r; 
      pixel[doff+1] = col->g; 
      pixel[doff+2] = col->b; 
      pixel[doff+3] = 255; 
      doff += 4; 
      soff++;
    }

    return (void *)pixel; 
}
#endif

static void handleTexture(object3d_t *object, Lib3dsMaterial *mat, Lib3dsTextureMap* map, Lib3dsTextureMap* mask, const char *type, int supported)
{
	assert (mat && map && mask);
	
	if (map->name[0])
	{
		if (!supported) {
			debugWarningPrintf(
				"Object '%s': material '%s': %s texture not supported! name:'%s', mask:'%s'",
				object->filename, mat->name, type, map->name, mask->name);
			return;
		}
	
		if (mask->name[0]) 
		{
			debugWarningPrintf(
				"Object '%s': material '%s': %s texture mask not supported! name:'%s', mask:'%s'",
				object->filename, mat->name, type, map->name, mask->name);
		}

		//if (map->name[0])
		{
		  object->tex_mode = 1;
			if (map->user.p) {
			  return;
			}

		debugPrintf(
			"Object '%s': material '%s' - loading %s texture name:'%s'",
			object->filename, mat->name, type, map->name);

			texture_t *pt = imageLoadImage((const char*)map->name);
		  map->user.p = pt;

		}
	}
}

static void loadMaterialTextures(object3d_t *object, Lib3dsMaterial *mat)
{
	assert (object && mat);

	if (mat->use_blur)
	{
		debugWarningPrintf("Object '%s': material '%s' - Blur is not supported! value:'%.2f'", object->filename, mat->name, mat->blur);
	}
	if (mat->use_falloff)
	{
		debugWarningPrintf("Object '%s': material '%s' - Falloff is not supported! value:'%.2f'", object->filename, mat->name, mat->falloff);
	}
	if (mat->self_illum)
	{
		debugWarningPrintf("Object '%s': material '%s' - Self illumination is not supported! value:'%.2f'", object->filename, mat->name, mat->self_ilpct);
		handleTexture(object, mat, &mat->self_illum_map, &mat->self_illum_mask, "self_illum", 0);
	}
	if (mat->soften)
	{
		debugWarningPrintf("Object '%s': material '%s' - Soften is not supported!", object->filename, mat->name);
	}
	if (mat->face_map)
	{
		debugWarningPrintf("Object '%s': material '%s' - Face map is not supported!", object->filename, mat->name);
	}
	if (mat->map_decal)
	{
		debugWarningPrintf("Object '%s': material '%s' - Map decal is not supported!", object->filename, mat->name);
	}
	if (mat->use_wire)
	{
		debugWarningPrintf("Object '%s': material '%s' - Wire is not supported! value:'%.2f'", object->filename, mat->name, mat->wire_size);
	}
	if (mat->use_wire_abs)
	{
		debugWarningPrintf("Object '%s': material '%s' - Wire abs is not supported! value:'%.2f'", object->filename, mat->name, mat->wire_size);
	}

	handleTexture(object, mat, &mat->texture1_map, &mat->texture1_mask, "texture1", 1);
	handleTexture(object, mat, &mat->texture2_map, &mat->texture2_mask, "texture2", 0);
	handleTexture(object, mat, &mat->opacity_map, &mat->opacity_mask, "opacity", 0);
	handleTexture(object, mat, &mat->bump_map, &mat->bump_mask, "bump", 0);
	handleTexture(object, mat, &mat->specular_map, &mat->specular_mask, "specular", 0);
	handleTexture(object, mat, &mat->shininess_map, &mat->shininess_mask, "shininess", 0);
	handleTexture(object, mat, &mat->reflection_map, &mat->reflection_mask, "reflection", 1);
}


static void loadModelTextures(object3d_t *object)
{
	assert (object);
	
  Lib3dsFile *file = object->data.file;
  if (file) {
    Lib3dsMaterial *mat = NULL;
    for (mat=file->materials; mat; mat=mat->next) {
		debugPrintf(
			"Object '%s': material '%s':\n" \
			"\tambient: r:%.2f, g:%.2f, b:%.2f, a:%.2f\n" \
			"\tdiffuse: r:%.2f, g:%.2f, b:%.2f, a:%.2f\n" \
			"\tspecular: r:%.2f, g:%.2f, b:%.2f, a:%.2f\n" \
			"\tshininess:%.2f, shin_strength:%.2f, transparency:%.2f, shading:%d",
			object->filename, mat->name,
			mat->ambient[0], mat->ambient[1], mat->ambient[2], mat->ambient[3],
			mat->diffuse[0], mat->diffuse[1], mat->diffuse[2], mat->diffuse[3],
			mat->specular[0], mat->specular[1], mat->specular[2], mat->specular[3],
			mat->shininess, mat->shin_strength, mat->transparency, mat->shading
		);

		loadMaterialTextures(object, mat);
	}
  }
}



static void enableMaterials(object3d_t *object, Lib3dsMaterial *mat)
{
if (!mat || mat == (Lib3dsMaterial *)-1)
{
	return;
}
if (!object) {return;}
//debugPrintf("Object '%s' ('%s'), material '%s' - enable!",node->name, object->filename, mat->name);
//TODO: multitexturing support here please
  texture_t *texture1 = (texture_t*)mat->texture1_map.user.p;
  texture_t *texture2 = (texture_t*)mat->texture2_map.user.p;
  texture_t *reflection = (texture_t*)mat->reflection_map.user.p;
  if (!texture1 && !texture2 && !reflection)
  {
	object->tex_mode = 0;
  }
  else
  {
    object->tex_mode = 1;
  }
//glColor4f(1,1,1,1);
  			  //PENIS THINGS
  glDisable(GL_BLEND);
        if (mat->transparency > 0 || object->tex_mode)
        {
          glEnable(GL_BLEND);
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        if (mat->additive)
        {
          if (mat->additive)
          {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            //glBlendFunc(GL_ONE, GL_ONE);
          }
          else
          {
            //glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
          }
          
          //glBlendFunc(GL_ONE, GL_ONE);
          //glEnable(GL_BLEND);
          //glColor4f(1,1,1,1-mat->transparency);
        }
        const float SIMPLE_COLOR_THRESHOLD = 0.05;
        if (object->tex_mode == 0)
        {
          //glDisable(GL_BLEND);
          float col[3] = {mat->ambient[0],mat->ambient[1],mat->ambient[2]};
          if (object->useSimpleColors)
          {
            //mitae vittua kanttu?!
            if (col[0] > SIMPLE_COLOR_THRESHOLD) {col[0] = 1;}
            if (col[1] > SIMPLE_COLOR_THRESHOLD) {col[1] = 1;}
            if (col[2] > SIMPLE_COLOR_THRESHOLD) {col[2] = 1;}
          }

          glColor4f(col[0],col[1],col[2],1-mat->transparency);          
        }
        else
        {
          //glEnable(GL_BLEND);
          //glColor4f(1,1,1,1-mat->transparency);
          //glColor4f(mat->specular[0],mat->specular[1],mat->specular[2],1-mat->transparency);
          //glColor4f(mat->ambient[0],mat->ambient[1],mat->ambient[2],1-mat->transparency);          
          float col[3] = {mat->ambient[0],mat->ambient[1],mat->ambient[2]};
          if (object->useSimpleColors)
          {
            //mitae vittua kanttu?!
            if (col[0] > SIMPLE_COLOR_THRESHOLD) {col[0] = 1;}
            if (col[1] > SIMPLE_COLOR_THRESHOLD) {col[1] = 1;}
            if (col[2] > SIMPLE_COLOR_THRESHOLD) {col[2] = 1;}
          }

          glColor4f(col[0],col[1],col[2],1-mat->transparency);          
        }
        //glColor4f(mat->specular[0],mat->specular[1],mat->specular[2],1-mat->transparency);
        //glColor4f(mat->diffuse[0],mat->diffuse[1],mat->diffuse[2],1-mat->transparency);
			  //glColor3fv(mat->diffuse);

	//glColor3f(1,1,1);
	if (object->tex_mode) {
	  //printf("Binding texture %d\n", pt->tex_id);
	  //glColor4f(1,1,1,0.5);
	  

	  //else
	  {
	  if (texture1)
	  {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1->id);
		glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
	  }
	  if (texture2)
	  {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2->id);
		glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
	  }
	  }
    if (reflection)
    {
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, reflection->id);
    glEnable(GL_TEXTURE_2D);
glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
      glEnable(GL_TEXTURE_GEN_T);
      glEnable(GL_TEXTURE_GEN_S);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
    //glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 0.5f);
    //glTexEnvf(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 0.1f);

    }
	}
	glBegin(GL_TRIANGLES);
}

static void disableMaterials(object3d_t *object, Lib3dsMaterial *mat)
{
if (!mat || mat == (Lib3dsMaterial *)-1)
{
	return;
}
if (!object) {return;}
glEnd();

//debugPrintf("Object '%s' ('%s'), material '%s' - disable!",node->name, object->filename, mat->name);
  texture_t *texture1 = (texture_t*)mat->texture1_map.user.p;
  texture_t *texture2 = (texture_t*)mat->texture2_map.user.p;
  texture_t *reflection = (texture_t*)mat->reflection_map.user.p;
  if (!texture1 && !texture2 && !reflection)
  {
	object->tex_mode = 0;
  }

	if (object->tex_mode)
	{
	  if (reflection)
	  {
		glActiveTexture(GL_TEXTURE2);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
		  glDisable(GL_TEXTURE_GEN_T);
		  glDisable(GL_TEXTURE_GEN_S);
		  //glActiveTexture(GL_TEXTURE0);
	  }
	  //else
	  {
	  if (texture2)
	  {
		glActiveTexture(GL_TEXTURE1);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	  }
	  if (texture1)
	  {
		glActiveTexture(GL_TEXTURE0);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	  }
	  }
	}
	if (mat->additive || mat->transparency > 0)
	{
		glDisable(GL_BLEND);
		glColor4f(1,1,1,1);
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}

/*!
* Render node recursively, first children, then parent.
* Each node receives its own OpenGL display list.
*/
static void
render_node(object3d_t *object, Lib3dsNode *node)
{
  ASSERT(object);
  ASSERT(object->data.file);
  ASSERT(node);
  
  //debugPrintf("Rendering '%s' '%s'", object->filename, node->name);

  {
    Lib3dsNode *p;
    /*int nodeSize = 0;
    Lib3dsNode *node[512];

    for (p=node->childs; p!=0; p=p->next) {
      node[nodeSize++] = p;
    //render_node(object, p);
    }
    int node_i = 0;
    for(node_i = nodeSize-1; node_i >= 0; node_i--)
    {
      render_node(object, node[node_i]);      
    }*/


    for (p=node->childs; p!=0; p=p->next) {
      render_node(object, p);
    }
  }
  if (node->type==LIB3DS_OBJECT_NODE) {
    Lib3dsMesh *mesh;

    if (strcmp(node->name,"$$$DUMMY")==0) {
      return;
    }

    mesh = lib3ds_file_mesh_by_name(object->data.file, node->data.object.morph);
    if( mesh == NULL )
      mesh = lib3ds_file_mesh_by_name(object->data.file, node->name);

    ASSERT(mesh);
    if (!mesh) {
      return;
    }

	  //assert(mesh->user.p);
	  mesh_extension *meshext = NULL;
    if (!mesh->user.p) {
	


	  debugPrintf("Generating vertex data. file:'%s', node:'%s', faces:'%d', color_index:'%d'", object->filename, node->name, mesh->faces, mesh->color);

      //mesh->user.d=glGenLists(1);
      //glNewList(mesh->user.d, GL_COMPILE);
		meshext = (mesh_extension*)malloc(sizeof(mesh_extension));
		mesh->user.p = meshext;
		meshext->list=0;
		meshext->list=glGenLists(1);
		glNewList(meshext->list, GL_COMPILE);

      {

        unsigned p;
        Lib3dsVector *normalL=malloc(3*sizeof(Lib3dsVector)*mesh->faces);
        Lib3dsMaterial *oldmat = (Lib3dsMaterial *)-1;
        {
          Lib3dsMatrix M;
          lib3ds_matrix_copy(M, mesh->matrix);
          lib3ds_matrix_inv(M);
          glMultMatrixf(&M[0][0]);
        }
        lib3ds_mesh_calculate_normals(mesh, normalL);


		  
		unsigned int vertices_max = mesh->faces*3*3;
		unsigned int texels_max = mesh->faces*3*2;
		unsigned int texels_i = 0;
		unsigned int vertices_i = 0;
		vbo_t *vbo = NULL;
		float* vertices = NULL;
		float* texels = NULL;
		if (meshext->list == 0)
		{
			meshext->vbo = vboInit(NULL);
			  vbo = meshext->vbo;
			  assert(vbo->id > 0);
			  //debugPrintf("%s/%s mesh->user.p: %p", object->filename, node->name, mesh->user.p);
			  //debugPrintf("%s/%s mesh->user.p: %p (%p), id:%d, count:%d", object->filename, node->name, mesh->user.p, vbo, vbo->id, vbo->count);
			vertices = (float*)malloc(vertices_max*sizeof(float)); //faces * vertexes per face * xyz
			texels = (float*)malloc(texels_max*sizeof(float)); //faces * vertexes per face * uv
			//debugPrintf("vertices: %d, texels: %d", vertices_max, texels_max);
			//float* normals = (float*)malloc(mesh->faces*3*sizeof(float));
		}

        for (p=0; p<mesh->faces; ++p) {
          Lib3dsFace *f=&mesh->faceL[p];
          Lib3dsMaterial *mat=(Lib3dsMaterial *)-1;
          //Player_texture *pt = NULL;
#ifdef	USE_SDL
          int tex_mode = 0;
#endif
          if (f->material[0]) {
            mat=lib3ds_file_material_by_name(object->data.file, f->material);
          }

          if( mat != oldmat ) {
		  if (meshext->list > 0 && oldmat != (Lib3dsMaterial *)-1) disableMaterials(object, oldmat);
            if (mat) {
			
              if( mat->two_sided )
                glDisable(GL_CULL_FACE);
              else
                glEnable(GL_CULL_FACE);

              //glDisable(GL_CULL_FACE);
              //glEnable(GL_CULL_FACE);
              //glCullFace(GL_FRONT);

              /* Texturing added by Gernot < gz@lysator.liu.se > */
              loadMaterialTextures(object, mat);

			  if (object->useObjectLighting)
			  {
				  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat->ambient);
				  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat->diffuse);
				  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat->specular);
				  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, pow(2, 10.0*mat->shininess));
			  }
			  
			  
			  if (meshext->list > 0) enableMaterials(object, mat);

            }
            else if (object->useObjectLighting) {
              static const Lib3dsRgba a={0.7, 0.7, 0.7, 1.0};
              static const Lib3dsRgba d={0.7, 0.7, 0.7, 1.0};
              static const Lib3dsRgba s={1.0, 1.0, 1.0, 1.0};
              glMaterialfv(GL_FRONT, GL_AMBIENT, a);
              glMaterialfv(GL_FRONT, GL_DIFFUSE, d);
              glMaterialfv(GL_FRONT, GL_SPECULAR, s);
              glMaterialf(GL_FRONT, GL_SHININESS, 32.0);
              //pow crashes gcc in morphos "get_z.c:46: MPFR assertion failed"
              //glMaterialf(GL_FRONT, GL_SHININESS, pow(2, 10.0*0.5));
            }
			
			
            oldmat = mat;
          }

          /*else if (mat != NULL && mat->texture1_map.name[0]) {
            Lib3dsTextureMap *tex = &mat->texture1_map;
            if (tex != NULL && tex->user.p != NULL) {
              pt = (Player_texture *)tex->user.p;
              object->tex_mode = pt->valid;
            }
          }*/


          {
            int i;
			
			meshext->material = mat;
			
			if (meshext->list == 0) {
				if (vertices_i+3*3 >= vertices_max)
				{
					//debugPrintf("!!! %d %d: faces:%d, face_i:%d",vertices_i, vertices_max, mesh->faces, p);
				}

				assert(vertices_i+3*3 <= vertices_max);
				if (object->tex_mode) { assert(texels_i+3*2 <= texels_max); }

				//glBegin(GL_TRIANGLES);
				//glNormal3fv(f->normal);
				for (i=0; i<3; ++i) {
				  //glNormal3fv(normalL[3*p+i]);

				  if (object->tex_mode) {
				  float u = mesh->texelL[f->points[i]][0];
				  float v = mesh->texelL[f->points[i]][1];
					//if (texture1) { glMultiTexCoord2f(GL_TEXTURE0, u, v); }
					//if (texture2) { glMultiTexCoord2f(GL_TEXTURE1, u, v); }
					texels[texels_i++] = u;
					texels[texels_i++] = v;
				  }

				  vertices[vertices_i++] = mesh->pointL[f->points[i]].pos[0];
				  vertices[vertices_i++] = mesh->pointL[f->points[i]].pos[1];
				  vertices[vertices_i++] = mesh->pointL[f->points[i]].pos[2];
				  
				  //glVertex3fv(mesh->pointL[f->points[i]].pos);
				}
				//glEnd();
			}
			else
			{
				//enableMaterials(object, mat);

				/*texture_t *texture1 = NULL;
				texture_t *texture2 = NULL;
				if (mat && mat > 0 && mat < UINTPTR_MAX && mat != (Lib3dsMaterial *)-1)
				{
					texture1 = (texture_t*)mat->texture1_map.user.p;
					texture2 = (texture_t*)mat->texture2_map.user.p;
				}*/
				/*if (mat->diffuse[0]+mat->diffuse[1]+mat->diffuse[2] == 0)
				{
				continue;
				}*/

				if (oldmat == (Lib3dsMaterial *)-1)
				{
					glBegin(GL_TRIANGLES);
				}
				//glBegin(GL_TRIANGLES);

				if (object->useObjectNormals)
        {
            glNormal3fv(f->normal);
        }
				for (i=0; i<3; ++i) {
          if (object->useObjectNormals)
          {
            glNormal3fv(normalL[3*p+i]);
          }

				  if (object->tex_mode && object->useObjectTextureCoordinates) {
          texture_t *texture1 = (texture_t*)mat->texture1_map.user.p;
          texture_t *texture2 = (texture_t*)mat->texture2_map.user.p;
          texture_t *reflection = (texture_t*)mat->reflection_map.user.p;
				  float u = mesh->texelL[f->points[i]][0];
				  float v = mesh->texelL[f->points[i]][1];
					if (texture1) { glMultiTexCoord2f(GL_TEXTURE0, u, v); }
					if (texture2) { glMultiTexCoord2f(GL_TEXTURE1, u, v); }
          if (reflection) { glMultiTexCoord2f(GL_TEXTURE2, u, v); }
					//glTexCoord2f(u, v);
				  }

				  glVertex3fv(mesh->pointL[f->points[i]].pos);
				}
				if (oldmat == (Lib3dsMaterial *)-1)
				{
					glEnd();
				}
				//glEnd();
				
				//disableMaterials(object, mat);
			}


          }
        }

		if (meshext->list == 0)
		{
			if (object->tex_mode) assert(texels_max == texels_i);
			assert(vertices_max == vertices_i);
			vboLoad(vbo, mesh->faces*3, vertices, texels, NULL);
			
			//debugPrintf("%s/%s: vertices_max:%d, faces:%d, faceCount:%d", object->filename, node->name, vertices_max, mesh->faces, vbo->count);
			
			//vboSetFaceCount(vbo, mesh->faces);
			//assert(vbo->count == mesh->faces);

			free(texels);
			free(vertices);
		}
		
        free(normalL);
		if (meshext->list > 0 && oldmat != (Lib3dsMaterial *)-1) disableMaterials(object, oldmat);
      }

	  
	  if (meshext->list > 0) { glEndList(); }
    }

    if (mesh->user.p) {
      Lib3dsObjectData *d;
	  mesh_extension *meshext = (mesh_extension*)mesh->user.p;
	  

      glPushMatrix();
      d=&node->data.object;

//printf("pivot: ");
//lib3ds_vector_dump(d->pivot);
if (object->clearZBuffer)
{
	//takaveto fix... glClear(GL_DEPTH_BUFFER_BIT);
	//glClear(GL_DEPTH_BUFFER_BIT);
}

      glMultMatrixf(&node->matrix[0][0]);
      glTranslatef(-d->pivot[0], -d->pivot[1], -d->pivot[2]);
	  if (meshext->list > 0)
	  {
		  glCallList(meshext->list);
	  }
	  else
	  {
	  if (meshext->list == 0)
	  {
          Lib3dsMatrix M;
          lib3ds_matrix_copy(M, mesh->matrix);
          lib3ds_matrix_inv(M);
          glMultMatrixf(&M[0][0]);
	  }

		  vbo_t *vbo = meshext->vbo;
		//  debugPrintf("%s/%s mesh->user.p: %p, id:%d, count:%d", object->filename, node->name, mesh->user.p, vbo->id, vbo->count);
		  assert(vbo->id > 0);
		  Lib3dsMaterial *mat=meshext->material;
		  assert(mat);

		  texture_t *texture1 = (texture_t*)mat->texture1_map.user.p;
		  texture_t *texture2 = (texture_t*)mat->texture2_map.user.p;
		  texture_t *reflection = (texture_t*)mat->reflection_map.user.p;
		  if (!texture1 && !texture2 && !reflection)
		  {
			object->tex_mode = 0;
		  }
		  else
		  {
		  object->tex_mode = 1;
		  }

			if (mat->additive||mat->transparency)
			{
				if (mat->additive)
				{
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				}
				else
				{
					glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
				}
				
				//glBlendFunc(GL_ONE, GL_ONE);
				glEnable(GL_BLEND);
				glColor4f(1,1,1,1-mat->transparency);
			}
			//glColor3fv(mat->diffuse);
			//glColor3f(1,1,1);
		vboEnablePointer(vbo, GL_VERTEX_ARRAY);
		vboEnablePointer(vbo, GL_NORMAL_ARRAY);

            if (object->tex_mode) {
              //printf("Binding texture %d\n", pt->tex_id);
			  //glColor4f(1,1,1,0.5);
			  
			  if (reflection)
			  {
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, reflection->id);
				glEnable(GL_TEXTURE_2D);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
				  glEnable(GL_TEXTURE_GEN_T);
				  glEnable(GL_TEXTURE_GEN_S);
				  vboEnablePointer(vbo, GL_TEXTURE_COORD_ARRAY);
			  }
			  else
			  {
			  if (texture1)
			  {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture1->id);
				glEnable(GL_TEXTURE_2D);
				vboEnablePointer(vbo, GL_TEXTURE_COORD_ARRAY);
			  }
			  if (texture2)
			  {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, texture2->id);
				glEnable(GL_TEXTURE_2D);
				vboEnablePointer(vbo, GL_TEXTURE_COORD_ARRAY);
			  }
			  }
            }


		vboDrawArrays(vbo);
		
            if (object->tex_mode)
			{
			  if (reflection)
			  {
				glActiveTexture(GL_TEXTURE2);
				vboDisablePointer(vbo, GL_TEXTURE_COORD_ARRAY);
				glBindTexture(GL_TEXTURE_2D, 0);
				glDisable(GL_TEXTURE_2D);
				  glDisable(GL_TEXTURE_GEN_T);
				  glDisable(GL_TEXTURE_GEN_S);
				  glActiveTexture(GL_TEXTURE0);
			  }
			  //else
			  {
			  if (texture2)
			  {
				glActiveTexture(GL_TEXTURE1);
				vboDisablePointer(vbo, GL_TEXTURE_COORD_ARRAY);
				glBindTexture(GL_TEXTURE_2D, 0);
				glDisable(GL_TEXTURE_2D);
			  }
			  if (texture1)
			  {
				glActiveTexture(GL_TEXTURE0);
				vboDisablePointer(vbo, GL_TEXTURE_COORD_ARRAY);
				glBindTexture(GL_TEXTURE_2D, 0);
				glDisable(GL_TEXTURE_2D);
			  }
			  }
			  glActiveTexture(GL_TEXTURE0);
			}

		vboDisablePointer(vbo, GL_VERTEX_ARRAY);
		vboDisablePointer(vbo, GL_NORMAL_ARRAY);
			if (mat->additive||mat->transparency)
			{
				glDisable(GL_BLEND);
				glColor4f(1,1,1,1);
			}
	  }
	  
      /* glutSolidSphere(50.0, 20,20); */
      glPopMatrix();
      //if( flush )
        //glFlush();
    }
  }
}




/*!
* Update information about a light.  Try to find corresponding nodes
* if possible, and copy values from nodes into light struct.
*/

static void
light_update(Lib3dsFile *file, Lib3dsLight *l)
{
  Lib3dsNode *ln, *sn;

  ln = lib3ds_file_node_by_name(file, l->name, LIB3DS_LIGHT_NODE);
  sn = lib3ds_file_node_by_name(file, l->name, LIB3DS_SPOT_NODE);

  if( ln != NULL ) {
    memcpy(l->color, ln->data.light.col, sizeof(Lib3dsRgb));
    memcpy(l->position, ln->data.light.pos, sizeof(Lib3dsVector));
  }

  if( sn != NULL )
    memcpy(l->spot, sn->data.spot.pos, sizeof(Lib3dsVector));
}

/*!
 * Compute a camera matrix based on position, target and roll.
 *
 * Generates a translate/rotate matrix that maps world coordinates
 * to camera coordinates.  Resulting matrix does not include perspective
 * transform.
 *
 * \param matrix Destination matrix.
 * \param pos Camera position
 * \param tgt Camera target
 * \param roll Roll angle
 *
 * \ingroup matrix
 */

#define getMin(A, B) ((A)<(B)?(A):(B))
#define getMax(A, B) ((A)>(B)?(A):(B))
#define compare_with_deviation(dev, a, b) ((a)<(b) && (a)+(dev) >= (b) && (a)-(dev) <= (b))
static void
lib3ds_matrix_camera2(Lib3dsMatrix matrix, Lib3dsVector pos,
  Lib3dsVector tgt, Lib3dsFloat roll)
{
/*debugPrintf("=> '%s'", object->filename);
printf("roll: %f\n", roll);
printf("matrix:\n");
lib3ds_matrix_dump(matrix);
printf("pos: ");
lib3ds_vector_dump(pos);
printf("tgt: ");
lib3ds_vector_dump(tgt);
*/

roll = 0.0f; //remove roll => not supported by penis
//if (((int)pos[0] == (int)tgt[0] || (int)(pos[0]-1) == (int)tgt[0] || (int)(pos[0]+1) == (int)tgt[0])
//	&& ((int)pos[1] == (int)tgt[1] || (int)(pos[1]-1) == (int)tgt[1] || (int)(pos[1]+1) == (int)tgt[1]))
/*
[1:58.565] system/graphics/object/3ds/3dsplay.c:lib3ds_matrix_camera2():1255: 
1=> 'data/4.3ds'
roll: 3.141593
matrix:
0.332914 0.000000 0.000000 1.875000 
0.000000 0.000000 0.000000 0.000000 
0.000000 0.000000 0.000000 0.000000 
0.000000 0.000000 0.000000 0.000000 
pos: -60.319813 -0.000003 77.110451
tgt: -60.556686 0.000000 -6.002588
[1:58.565] system/graphics/object/3ds/3dsplay.c:lib3ds_matrix_camera2():1255: 
1=> 'data/4.3ds'
roll: 3.141593
matrix:
0.332914 0.000000 0.000000 1.875000 
0.000000 0.000000 0.000000 0.000000 
0.000000 0.000000 0.000000 0.000000 
0.000000 0.000000 0.000000 0.000000 
pos: -60.319813 -0.000003 77.110451
tgt: -60.556686 0.000000 -6.002588
[1:58.565] system/graphics/object/3ds/3dsplay.c:lib3ds_matrix_camera2():1255: 
1=> 'data/4.3ds'

*/
const double deviation = 0.6;
if (compare_with_deviation(deviation, pos[0], tgt[0]) && compare_with_deviation(deviation, pos[1], tgt[1]))
{
	roll = M_PI;
}

 
  Lib3dsMatrix M;
  Lib3dsVector x, y, z;

  lib3ds_vector_sub(y, tgt, pos);
  lib3ds_vector_normalize(y);

  if (y[0] != 0. || y[1] != 0) {
    z[0] = 0;
    z[1] = 0;
    z[2] = 1.0;
  }
  else {	// Special case:  looking straight up or down z axis 
    z[0] = -1.0;
    z[1] = 0;
    z[2] = 0;
  }

  lib3ds_vector_cross(x, y, z);
  lib3ds_vector_cross(z, x, y);
  lib3ds_vector_normalize(x);
  lib3ds_vector_normalize(z);


  /*
  pos: 700.373291 -0.000002 0.000000
tgt: -60.225983 -0.000000 0.000000

  x: 0.000000 1.000000 -0.000000
y: -1.000000 0.000000 0.000000
z: 0.000000 0.000000 1.000000
*/
  /*x[0] = 1;
  x[1] = 0;
  x[2] = 0;
  y[0] = 0;
  y[1] = 1;
  y[2] = 0;
  z[0] = 0;
  z[1] = 0;
  z[2] = 1;*/
  /*
printf("x: ");
lib3ds_vector_dump(x);
printf("y: ");
lib3ds_vector_dump(y);
printf("z: ");
lib3ds_vector_dump(z);
*/
  lib3ds_matrix_identity(M);
  /*M[0][0] = x[0];
  M[1][0] = x[1];
  M[2][0] = x[2];
  M[0][1] = y[0];
  M[1][1] = y[1];
  M[2][1] = y[2];
  M[0][2] = z[0];
  M[1][2] = z[1];
  M[2][2] = z[2];*/
  /*M[0][0] = x[0];
  M[1][0] = x[1]*-1;
  M[2][0] = x[2];
  M[0][1] = y[0];
  M[1][1] = y[1]*-1;
  M[2][1] = y[2];
  M[0][2] = z[0];
  M[1][2] = z[1]*-1;
  M[2][2] = z[2];*/
  M[0][0] = x[0];
  M[1][0] = x[1];
  M[2][0] = x[2];
  M[0][1] = y[0];
  M[1][1] = y[1];
  M[2][1] = y[2];
  M[0][2] = z[0];
  M[1][2] = z[1];
  M[2][2] = z[2];
  
  lib3ds_matrix_identity(matrix);
  lib3ds_matrix_rotate_y(matrix, roll);
  lib3ds_matrix_mult(matrix, M);
  lib3ds_matrix_translate_xyz(matrix, -pos[0],-pos[1],-pos[2]);
  
/*printf("matrix2:\n");
lib3ds_matrix_dump(matrix);*/

}


/*!
* Main display function; called whenever the scene needs to be redrawn.
*/
void display3ds(object3d_t *object)
{
  assert(object != NULL);
  assert(object->data.file != NULL);

	if (object->useObjectLighting)
	{
		glEnable(GL_LIGHTING);
	}

  glDisable(GL_BLEND);
 
  Lib3dsFloat fov = 45.0f, roll;
  float _far, _near, dist;
  float *campos = NULL;
  float *tgt = NULL;
  Lib3dsNode *c=NULL,*t=NULL;
  Lib3dsMatrix M;
  Lib3dsCamera *cam;
  Lib3dsVector v;
  Lib3dsNode *p;
  const char *camera = object->camera;

  if( object->data.file != NULL && object->data.file->background.solid.use )
    glClearColor(object->data.file->background.solid.col[0],
    object->data.file->background.solid.col[1],
    object->data.file->background.solid.col[2], 1.);

  /* TODO: fog */

  //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if( anti_alias )
    glEnable(GL_POLYGON_SMOOTH);
  else
    glDisable(GL_POLYGON_SMOOTH);

    //glDisable(GL_POLYGON_SMOOTH);


  if (!object->data.file) {
    return;
  }

  if (object->useObjectLighting)
  {
	  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, object->data.file->ambient);
  }

  c=lib3ds_file_node_by_name(object->data.file, camera, LIB3DS_CAMERA_NODE);
  t=lib3ds_file_node_by_name(object->data.file, camera, LIB3DS_TARGET_NODE);

  if( t != NULL )
    tgt = t->data.target.pos;

  if( c != NULL ) {
    fov = c->data.camera.fov;
    roll = c->data.camera.roll;
    campos = c->data.camera.pos;
  }

  /*Lib3dsMatrix M2;
  int cam_i = 0;
  for( cam = object->data.file->cameras; cam != NULL; cam = cam->next )
  {
	debugPrintf("Camera%02d!",++cam_i);
	//lib3ds_matrix_camera2(object, M2, cam->position, cam->target, cam->roll);
  }*/
  if ((cam = lib3ds_file_camera_by_name(object->data.file, camera)) == NULL)
    return;

//  _near = cam->near_range;
//  _far = cam->far_range;
  _near = 1.0f;
  _far = 10000.0f;

  if (c == NULL || t == NULL) {
    if( c == NULL ) {
      fov = cam->fov;
      roll = cam->roll;
      campos = cam->position;
    }
    if( t == NULL )
      tgt = cam->target;
  }

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  /* KLUDGE alert:  OpenGL can't handle a near clip plane of zero,
  * so if the camera's near plane is zero, we give it a small number.
  * In addition, many .3ds files I've seen have a far plane that's
  * much too close and the model gets clipped away.  I haven't found
  * a way to tell OpenGL not to clip the far plane, so we move it
  * further away.  A factor of 10 seems to make all the models I've
  * seen visible.
  */
  if( _near <= 0. ) _near = _far * .001;

  float _aspect = getWindowScreenAreaAspectRatio();

  if (!object->useObjectCamera)
  {
	camera_t *camera = getCamera();
	campos[0] = camera->position.x;
	campos[1] = camera->position.y;
	campos[2] = camera->position.z;
	tgt[0] = camera->lookAt.x;
	tgt[1] = camera->lookAt.y;
	tgt[2] = camera->lookAt.z;
	
	fov = camera->fovy;
	_aspect = camera->aspect;
	_near = camera->zNear;
	_far = camera->zFar;

	roll = 0;
  }

  gluPerspective( fov, _aspect, _near, _far);

  glMatrixMode(GL_MODELVIEW);
  //glLoadIdentity();
  glRotatef(-90, 1.0,0,0);

  /* User rotates the view about the target point */

  lib3ds_vector_sub(v, tgt, campos);
  dist = lib3ds_vector_length(v);

  glTranslatef(0.,dist, 0.);
  glRotatef(view_rotx, 1., 0., 0.);
  glRotatef(view_roty, 0., 1., 0.);
  glRotatef(view_rotz, 0., 0., 1.);
  glTranslatef(0.,-dist, 0.);
 
  lib3ds_matrix_camera2(M, campos, tgt, roll);//-M_PI*0.82);
  //lib3ds_matrix_inv(M);
  //change_matrix(M);
  /*M[0][0] *= -1;
  M[0][1] *= -1;
  M[0][2] *= -1;
  //M[1][4] *= -1;*/
  glMultMatrixf(&M[0][0]);

  /* Lights.  Set them from light nodes if possible.  If not, use the
  * light objects directly.
  */
  if (object->useObjectLighting)
  {
    //static const GLfloat a[] = {0.0f, 0.0f, 0.0f, 1.0f};
    static GLfloat c[] = {1.0f, 1.0f, 1.0f, 1.0f};
    static GLfloat p[] = {0.0f, 0.0f, 0.0f, 1.0f};
    Lib3dsLight *l;

    unsigned int li=GL_LIGHT0;
    for (l=object->data.file->lights; l; l=l->next) {
      glEnable(li);

      light_update(object->data.file, l);

      c[0] = l->color[0];
      c[1] = l->color[1];
      c[2] = l->color[2];
	  //printf("light %d: r: %.2f, g: %.2f, b: %.2f\n", li, c[0], c[1], c[2]);
      glLightfv(li, GL_AMBIENT, c);
      glLightfv(li, GL_DIFFUSE, c);
      glLightfv(li, GL_SPECULAR, c);
	  
	           
//glColor3f(c[0], c[1], c[2]);
      p[0] = l->position[0];
      p[1] = l->position[1];
      p[2] = l->position[2];
      glLightfv(li, GL_POSITION, p);

      if (l->spot_light) {
        p[0] = l->spot[0] - l->position[0];
        p[1] = l->spot[1] - l->position[1];
        p[2] = l->spot[2] - l->position[2];
        glLightfv(li, GL_SPOT_DIRECTION, p);
      }
      ++li;
    }
  }

  if( show_object ) 
  {
    int nodeSize = 0;
    Lib3dsNode *node[512];

    for (p=object->data.file->nodes; p!=0; p=p->next) {
      node[nodeSize++] = p;
	  //render_node(object, p);
    }
    int node_i = 0;
    for(node_i = nodeSize-1; node_i >= 0; node_i--)
    {
      //float *col = node[node_i]->data.ambient.col;
      //debugPrintf("'%s' Color! r:%.2f, g:%.2f, b:%.2f", node[node_i]->name, col[0], col[1], col[2]);
      render_node(object, node[node_i]);      
    }
    /*for (p=object->data.file->nodes; p!=0; p=p->next) {
      render_node(object, p);
    }*/
  }

  /*if( show_bounds )
    draw_bounds(tgt);

  if( show_cameras )
  {
    for( cam = object->data.file->cameras; cam != NULL; cam = cam->next )
    {
      lib3ds_matrix_camera2(object, M, cam->position, cam->target, cam->roll);
      lib3ds_matrix_inv(M);
      glPushMatrix();
      glMultMatrixf(&M[0][0]);
      glScalef(size/20, size/20, size/20);
      glCallList(cameraList);
      glPopMatrix();
    }
  }

  if( show_lights )
  {
    Lib3dsLight *light;
    for( light = object->data.file->lights; light != NULL; light = light->next )
      draw_light(light->position, light->color);
    glMaterialfv(GL_FRONT, GL_EMISSION, black);
  }*/

	
	if (object->useObjectLighting)
	{
	  Lib3dsLight *l;
		unsigned int li=GL_LIGHT0;
		for (l=object->data.file->lights; l; l=l->next) {
		  glDisable(li);
		  ++li;
		}
		//if (isLightingEnabled())
		{
			glDisable(GL_LIGHTING);
		}
	}

  //glutSwapBuffers();
}
