#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "obj.h"

#include "system/debug/debug.h"
#include "system/io/io.h"
#include "system/graphics/texture.h"
#include "system/graphics/image/image.h"

/*
 * Format reference: https://en.wikipedia.org/wiki/Wavefront_.obj_file
 */

/*
Object:
mtllib dabomb.mtl
o Sphere_Sphere.002
v 0.742819 0.300897 -0.132256
v 0.514703 -2.736936 -0.293016
...
vt 0.3241 0.3117
vt 0.3509 0.3116
...
vn 0.9986 0.0525 0.0000
vn 0.9800 0.0519 -0.1921
...
usemtl Material.001
s 1
f 313/1/1 5/2/2 6/3/3 312/4/4
f 312/4/4 6/3/3 7/5/5 311/6/6
...
*/

/*
Material:
newmtl Material.001
Ns 96.078431
Ka 1.000000 1.000000 1.000000
Kd 0.800000 0.800000 0.800000
Ks 0.500000 0.500000 0.500000
Ke 0.000000 0.000000 0.000000
Ni 1.000000
d 1.000000
illum 0
map_Kd h:\dev\demot\666\dabombtex.png
*/

//texture_t *texture = imageLoadImage("");

/*
typedef struct obj_texture_t {
	obj_color_t color;
	texture_t *texture;
} obj_texture_t;

typedef struct obj_material_t {
	char *filename;
	char *name;
	obj_texture_t ambient; //Ka %f %f %f && map_Ka %s
	obj_texture_t diffuse; //Kd %f %f %f && map_Kd %s
	obj_texture_t specular; //Ks %f %f %f && map_Ks %s
	double specular_exponent; //Ns %f
	double alpha; //d %f || Tr %f --- d 1.0 = 0% transparent; Tr = 1.0 = 100% transparent
	unsigned char illumination_model; //illum %d
} obj_material_t;
*/

#define TYPE_NEW_MATERIAL              "newmtl"
#define TYPE_MAP_DIFFUSE               "map_Kd"
#define TYPE_MAP_REFLECTION            "map_refl"

#define ALLOC_STEP_SIZE 128

//dynamically add pointer to array of pointers. array will be extended if there's not enough size
#define add_to_array(type, array, element) \
	assert(element); \
	array##_size++; \
	if (array##_size%ALLOC_STEP_SIZE == 1) \
	{ \
		array = (type**)realloc(array, sizeof(type*)*(array##_size+ALLOC_STEP_SIZE)); \
		assert(array); \
	} \
	array[array##_size-1] = element

//reserve memory only up to array##_size variable, i.e. free the ALLOC_STEP_SIZE's leftovers
#define realloc_to_actual_size(type, array) \
	assert(array); \
	array = (type**)realloc(array, sizeof(type*)*(array##_size)); \
	assert(array)

static void initialize_obj_texture(obj_texture_t *obj_texture)
{
	assert(obj_texture);

	obj_texture->color.r = 0;
	obj_texture->color.g = 0;
	obj_texture->color.b = 0;

	obj_texture->texture = NULL;
}

static void initialize_obj_material(obj_material_t *material)
{
	assert(material);

	material->filename = NULL;
	material->name = NULL;

	initialize_obj_texture(&material->ambient);
	initialize_obj_texture(&material->diffuse);
	initialize_obj_texture(&material->specular);
	initialize_obj_texture(&material->reflection);

	material->specular_exponent = 0.0;
	material->alpha = 0.0;
	material->illumination_model = 0;
}

void obj_material_free(obj_material_t *material)
{
	assert(material);

	free(material->filename);
	free(material->name);

	/*char *filename;
	char *name;
	obj_texture_t ambient; //Ka %f %f %f && map_Ka %s
	obj_texture_t diffuse; //Kd %f %f %f && map_Kd %s
	obj_texture_t specular; //Ks %f %f %f && map_Ks %s*/

	free(material);
}

obj_material_t* obj_get_material(obj_container_t* obj_container, const char *material_name)
{
	assert(obj_container);

	unsigned int i;
	for(i = 0; i < obj_container->materials_size; i++)
	{
		obj_material_t *material = obj_container->materials[i];
		assert(material);

		if (!strcmp(material->name, material_name))
		{
			return material;
		}
	}

	return NULL;
}

void obj_material_file_load(obj_container_t *obj_container, const char *filename)
{
	assert(obj_container);

	#define LINE_SIZE 2048
	char *line = (char*)malloc(sizeof(char)*LINE_SIZE);
	assert(line);
	char *type = (char*)malloc(sizeof(char)*LINE_SIZE);
	assert(type);

	obj_material_t *current_material = NULL;

	char *full_filename = strdup(getFilePath(filename));
	assert(full_filename);

	FILE *f = fopen(full_filename, "rb");
	assert(f);

	while (fgets(line, LINE_SIZE, f) != NULL)
	{
		int matches = sscanf(line, "%s", type);
		if (matches == 0 || line[0] == '#')
		{
			continue;
		}

		//trim white spaces (LF or CRLF mainly) from the tail
		do
		{
			line[strlen(line)-1] = '\0';
		} while(strlen(line) != 0 && (int)line[strlen(line)-1] <= 32);

		if (!strcmp(type, TYPE_MAP_DIFFUSE))
		{
			if (current_material == NULL)
			{
				debugErrorPrintf("Parse error. Current material's name not defined.");
				assert(current_material);
			}

			char *original_map_line = strdup(line);
			assert(original_map_line);

			char *map_line = original_map_line;

			const char* DELIMETER = " ";
			char *token;
			while((token = strtok_reentrant(map_line, DELIMETER, &map_line)))
			{
				if (strlen(token) > 4 && token[strlen(token)-4] == '.')
				{
					current_material->diffuse.texture = imageLoadImage(token);
					assert(current_material->diffuse.texture);
					break;
				}
			}

			free(original_map_line);
		}
		else if (!strcmp(type, TYPE_MAP_REFLECTION))
		{
			if (current_material == NULL)
			{
				debugErrorPrintf("Parse error. Current material's name not defined.");
				assert(current_material);
			}

			char *original_map_line = strdup(line);
			assert(original_map_line);

			char *map_line = original_map_line;

			const char* DELIMETER = " ";
			char *token;
			while((token = strtok_reentrant(map_line, DELIMETER, &map_line)))
			{
				if (strlen(token) > 4 && token[strlen(token)-4] == '.')
				{
					current_material->reflection.texture = imageLoadImage(token);
					assert(current_material->reflection.texture);
					break;
				}
			}

			free(original_map_line);
		}
		else if (!strcmp(type, TYPE_NEW_MATERIAL))
		{
			if (current_material != NULL)
			{
				realloc_to_actual_size(obj_material_t, obj_container->materials);
			}

			current_material = (obj_material_t*)malloc(sizeof(obj_material_t));
			initialize_obj_material(current_material);
			add_to_array(obj_material_t, obj_container->materials, current_material);

			current_material->filename = strdup(full_filename);
			assert(current_material->filename);
			current_material->name = strdup(&line[strlen(TYPE_NEW_MATERIAL)+1]);
			assert(current_material->name);
		}
	}

	if (current_material == NULL)
	{
		debugErrorPrintf("No materials found in the material library?!");
	}
	assert(current_material);
	realloc_to_actual_size(obj_material_t, obj_container->materials);

	free(line);
	free(type);

	fclose(f);

	debugPrintf("Loaded material library '%s'", full_filename);
	free(full_filename);
}
