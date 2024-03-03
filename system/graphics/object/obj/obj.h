#ifndef EXH_SYSTEM_GRAPHICS_OBJECT_OBJ_OBJ_H_
#define EXH_SYSTEM_GRAPHICS_OBJECT_OBJ_OBJ_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "system/graphics/texture.h"

typedef struct obj_color_t {
	float r;
	float g;
	float b;
} obj_color_t;

typedef struct obj_xyz_t {
	float x;
	float y;
	float z;
} obj_xyz_t;

typedef struct obj_uv_t {
	float u;
	float v;
} obj_uv_t;

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
	obj_texture_t reflection; //map_refl %s
	float specular_exponent; //Ns %f
	float alpha; //d %f || Tr %f --- d 1.0 = 0% transparent; Tr = 1.0 = 100% transparent
	unsigned char illumination_model; //illum %d
} obj_material_t;

typedef struct obj_vertex_t {
	obj_xyz_t xyz;
	float w;
} obj_vertex_t;

typedef struct obj_vertex_normal_t {
	obj_xyz_t xyz;
} obj_vertex_normal_t;

typedef struct obj_vertex_texture_coordinate_t {
	obj_uv_t uv;
	float w;
} obj_vertex_texture_coordinate_t;

typedef struct obj_face_parameters_t {
	obj_material_t *material;
	unsigned int smooth_shading;
} obj_face_parameters_t;

typedef struct obj_face_t {
	obj_vertex_t **vertices;
	obj_vertex_normal_t **normals;
	obj_vertex_texture_coordinate_t **texture_coordinates;
	unsigned char size;
	obj_face_parameters_t face_parameters;
} obj_face_t;

typedef struct obj_object_t {
	obj_face_t **faces;
	obj_vertex_t **vertices;
	obj_vertex_normal_t **vertex_normals;
	obj_vertex_texture_coordinate_t **vertex_texture_coordinates;
	unsigned int faces_size;
	unsigned int vertices_size;
	unsigned int vertex_normals_size;
	unsigned int vertex_texture_coordinates_size;
	char *name;
} obj_object_t;

typedef struct obj_container_t {
	obj_object_t **objects;
	unsigned int objects_size;
	obj_material_t **materials;
	unsigned int materials_size;
	char *filename;
} obj_container_t;

extern obj_container_t* obj_file_load(const char *filename);
extern void obj_container_free(obj_container_t *container);

extern void obj_material_free(obj_material_t *material);
extern obj_material_t* obj_get_material(obj_container_t* obj_container, const char *material_name);
extern void obj_material_file_load(obj_container_t *obj_container, const char *filename);

#ifdef __cplusplus
/* end 'extern "C"' wrapper */
}
#endif

#endif /*EXH_SYSTEM_GRAPHICS_OBJECT_OBJ_OBJ_H_*/
