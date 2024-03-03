#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "obj.h"

#include "system/debug/debug.h"
#include "system/io/io.h"
#include "system/graphics/texture.h"

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

#define TYPE_VERTEX                    "v"
#define TYPE_VERTEX_NORMAL             "vn"
#define TYPE_VERTEX_TEXTURE_COORDINATE "vt"
#define TYPE_FACE                      "f"
#define TYPE_OBJECT_NAME               "o"
#define TYPE_SMOOTH_SHADING            "s"
#define TYPE_MATERIAL_LIBRARY          "mtllib"
#define TYPE_USE_MATERIAL              "usemtl"

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

static void initialize_obj_face_parameters(obj_face_parameters_t *face_parameters)
{
	assert(face_parameters);
	face_parameters->material = NULL;
	face_parameters->smooth_shading = 0;
}

static void initialize_obj_face(obj_face_t *face)
{
	assert(face);

	face->vertices = NULL;
	face->normals = NULL;
	face->texture_coordinates = NULL;
	face->size = 0;
	initialize_obj_face_parameters(&face->face_parameters);
}

static void realloc_obj_face(obj_face_t *face, unsigned int size)
{
	assert(face);

	face->vertices = (obj_vertex_t**)realloc(face->vertices, sizeof(obj_vertex_t*)*size);
	assert(face->vertices);
	face->normals = (obj_vertex_normal_t**)realloc(face->normals, sizeof(obj_vertex_normal_t*)*size);
	assert(face->normals);
	face->texture_coordinates = (obj_vertex_texture_coordinate_t**)realloc(face->texture_coordinates, sizeof(obj_vertex_texture_coordinate_t*)*size);
	assert(face->texture_coordinates);

	unsigned int i;
	for(i = face->size; i < size; i++)
	{
		face->vertices[i] = NULL;
		face->normals[i]  = NULL;
		face->texture_coordinates[i] = NULL;
	}
	face->size = size;
}

static void initialize_obj_object(obj_object_t *object)
{
	assert(object);

	object->faces = NULL;
	object->vertices = NULL;
	object->vertex_normals = NULL;
	object->vertex_texture_coordinates = NULL;
	object->faces_size = 0;
	object->vertices_size = 0;
	object->vertex_normals_size = 0;
	object->vertex_texture_coordinates_size = 0;
	object->name = NULL;
}

static void initialize_obj_container(obj_container_t *file)
{
	assert(file);

	file->objects = NULL;
	file->objects_size = 0;
	file->materials = NULL;
	file->materials_size = 0;
	file->filename = NULL;
}

static void obj_face_free(obj_face_t *face)
{
	assert(face);
	free(face->vertices);
	free(face->normals);
	free(face->texture_coordinates);
	free(face);
}

static void obj_object_free(obj_object_t *object)
{
	assert(object);

	unsigned int i;
	for(i = 0; i < object->faces_size; i++)
	{
		obj_face_free(object->faces[i]);
	}
	free(object->faces);

	for(i = 0; i < object->vertices_size; i++)
	{
		free(object->vertices[i]);
	}
	free(object->vertices);

	if (object->vertex_normals)
	{
		for(i = 0; i < object->vertex_normals_size; i++)
		{
			free(object->vertex_normals[i]);
		}
		free(object->vertex_normals);
	}

	if (object->vertex_texture_coordinates)
	{
		for(i = 0; i < object->vertex_texture_coordinates_size; i++)
		{
			free(object->vertex_texture_coordinates[i]);
		}
		free(object->vertex_texture_coordinates);
	}

	free(object->name);

	free(object);
}

void obj_container_free(obj_container_t *container)
{
	assert(container);
	unsigned int i;

	if (container->objects)
	{
		for(i = 0; i < container->objects_size; i++)
		{
			obj_object_free(container->objects[i]);
		}
		free(container->objects);
	}

	if (container->materials)
	{
		for(i = 0; i < container->materials_size; i++)
		{
			obj_material_free(container->materials[i]);
		}
		free(container->materials);
	}

	free(container->filename);

	free(container);
}

obj_container_t* obj_file_load(const char *filename)
{
	#define LINE_SIZE 2048
	char *line = (char*)malloc(sizeof(char)*LINE_SIZE);
	assert(line);
	char *type = (char*)malloc(sizeof(char)*LINE_SIZE);
	assert(type);

	obj_container_t *file = (obj_container_t*)malloc(sizeof(obj_container_t));
	initialize_obj_container(file);
	file->filename = strdup(getFilePath(filename));
	assert(file->filename);

	FILE *f = fopen(file->filename, "rb");
	assert(f);

	obj_object_t *current_object = NULL;

	obj_face_parameters_t current_face_parameters;
	initialize_obj_face_parameters(&current_face_parameters);

	int object_count = 0;
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

		if (!strcmp(type, TYPE_VERTEX))
		{
			if (current_object == NULL)
			{
				debugErrorPrintf("Parse error. Current object's name not defined.");
				assert(current_object);
			}
			obj_vertex_t *vertex = (obj_vertex_t*)malloc(sizeof(obj_vertex_t));
			add_to_array(obj_vertex_t, current_object->vertices, vertex);

			matches = sscanf(line, "%s %f %f %f %f", type, &vertex->xyz.x, &vertex->xyz.y, &vertex->xyz.z, &vertex->w);
			if (matches == 4)
			{
				vertex->w = 0.0;
			}
			assert(matches >= 4 && matches <= 5);
		}
		else if (!strcmp(type, TYPE_VERTEX_TEXTURE_COORDINATE))
		{
			obj_vertex_texture_coordinate_t *vertex_texture_coordinate = (obj_vertex_texture_coordinate_t*)malloc(sizeof(obj_vertex_texture_coordinate_t));
			add_to_array(obj_vertex_texture_coordinate_t, current_object->vertex_texture_coordinates, vertex_texture_coordinate);

			matches = sscanf(line, "%s %f %f %f", type, &vertex_texture_coordinate->uv.u, &vertex_texture_coordinate->uv.v, &vertex_texture_coordinate->w);
			if (matches == 3)
			{
				vertex_texture_coordinate->w = 0.0;
			}
			assert(matches >= 3 && matches <= 4);
		}
		else if (!strcmp(type, TYPE_VERTEX_NORMAL))
		{
			obj_vertex_normal_t *vertex_normal = (obj_vertex_normal_t*)malloc(sizeof(obj_vertex_normal_t));
			add_to_array(obj_vertex_normal_t, current_object->vertex_normals, vertex_normal);

			matches = sscanf(line, "%s %f %f %f", type, &vertex_normal->xyz.x, &vertex_normal->xyz.y, &vertex_normal->xyz.z);
			assert(matches == 4);
		}
		else if (!strcmp(type, TYPE_FACE))
		{
			if (current_object->faces_size == 0)
			{
				realloc_to_actual_size(obj_vertex_t, current_object->vertices);
				if (current_object->vertex_texture_coordinates)
				{
					realloc_to_actual_size(obj_vertex_texture_coordinate_t, current_object->vertex_texture_coordinates);
				}
				if (current_object->vertex_normals)
				{
					realloc_to_actual_size(obj_vertex_normal_t, current_object->vertex_normals);
				}
			}

			obj_face_t *face = (obj_face_t*)malloc(sizeof(obj_face_t));
			initialize_obj_face(face);
			realloc_obj_face(face, 3);

			face->face_parameters.material = current_face_parameters.material;
			face->face_parameters.smooth_shading = current_face_parameters.smooth_shading;

			char *original_face_line = strdup(line);
			assert(original_face_line);

			char *face_line = original_face_line;
			//debugPrintf("face_line: '%s'", face_line);

			const char* DELIMETER = " ";
			const char* PARAMETER_DELIMETER = "/";
			char *token;
			unsigned int face_size = 0;
			while((token = strtok_reentrant(face_line, DELIMETER, &face_line)))
			{
				if (token[0] == 'f')
				{
					continue;
				}

				face_size++;
				if (face_size > 3)
				{
					realloc_obj_face(face, face_size);
				}

				//Parse index values of face. For example: "123/456/789" => v[124], vt[457], vn[790]
				int parameter_type = 0;
				char *parameter_token = NULL;
				while((parameter_token = strtok_reentrant(token, PARAMETER_DELIMETER, &token)))
				{
					assert(parameter_type < 3);

					if (strlen(parameter_token) > 0)
					{
						unsigned int index = atoi(parameter_token) - 1; //OBJ index starts from 1. We want it to start natively from 0.

						if (parameter_type == 0) {
							assert(index < current_object->vertices_size);
							face->vertices[face_size-1] = current_object->vertices[index];
						}
						else if (parameter_type == 1) {
							assert(index < current_object->vertex_texture_coordinates_size);
							face->texture_coordinates[face_size-1] = current_object->vertex_texture_coordinates[index];
						}
						else if (parameter_type == 2) {
							assert(index < current_object->vertex_normals_size);
							face->normals[face_size-1] = current_object->vertex_normals[index];
						}
						else {
							debugErrorPrintf("Could not parse OBJ properly");
						}
					}

					parameter_type++;
					if (token[0] == '/') {
						parameter_type++;
					}
				}
			}

			free(original_face_line);

			add_to_array(obj_face_t, current_object->faces, face);

			if (face_size < 3 || face_size > 4)
			{
				debugErrorPrintf("Only TRIANGLE or QUAD faces are supported. Face vertices: %d, line:%s", face_size, line);
				assert(face_size >= 3 && face_size <= 4);
			}
		}
		else if (!strcmp(type, TYPE_OBJECT_NAME))
		{
			object_count++;

			if (current_object != NULL)
			{
				realloc_to_actual_size(obj_face_t, current_object->faces);
			}

			current_object = (obj_object_t*)malloc(sizeof(obj_object_t));
			initialize_obj_object(current_object);
			add_to_array(obj_object_t, file->objects, current_object);

			current_object->name = strdup(&line[strlen(TYPE_OBJECT_NAME)+1]);
			assert(current_object->name);
		}
		else if (!strcmp(type, TYPE_MATERIAL_LIBRARY))
		{
			obj_material_file_load(file, &line[strlen(TYPE_MATERIAL_LIBRARY)+1]);
		}
		else if (!strcmp(type, TYPE_USE_MATERIAL))
		{
			current_face_parameters.material = obj_get_material(file, &line[strlen(TYPE_USE_MATERIAL)+1]);
		}
		else if (!strcmp(type, TYPE_SMOOTH_SHADING))
		{
			matches = sscanf(line, "%s %u", type, &current_face_parameters.smooth_shading);
			if (matches == 1)
			{
				current_face_parameters.smooth_shading = 0; //"s off" allowed so %u would not match
			}
		}
	}

	if (current_object == NULL)
	{
		debugErrorPrintf("No objects found in the object?!");
	}
	assert(current_object);

	if (current_object->faces_size == 0)
	{
		debugErrorPrintf("Object has no faces, parse error?");
	}
	assert(current_object->faces_size > 0);

	realloc_to_actual_size(obj_face_t, current_object->faces);
	if (file->objects)
	{
		realloc_to_actual_size(obj_object_t, file->objects);
	}
	if (file->materials)
	{
		realloc_to_actual_size(obj_material_t, file->materials);
	}

	free(line);
	free(type);

	fclose(f);

	debugPrintf("Loaded object '%s'. Faces:%d (vertices:%d, normals:%d, texture_coordinates:%d)", file->filename, current_object->faces_size, current_object->vertices_size, current_object->vertex_normals_size, current_object->vertex_texture_coordinates_size);

	return file;
}
