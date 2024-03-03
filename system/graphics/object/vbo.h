#ifndef EXH_SYSTEM_GRAPHICS_OBJECT_VBO_H_
#define EXH_SYSTEM_GRAPHICS_OBJECT_VBO_H_

typedef struct {
	GLuint id, vertexId, normalId, texCoordId;
	unsigned int count;
} vbo_t;

extern void vboSetDefaults(vbo_t* vbo);
extern vbo_t* vboInit(vbo_t* vbo);
extern void vboDeinit(vbo_t* vbo);
extern void vboLoadArray(GLuint* bufferId, GLenum arrayType, unsigned int elementCount, float* buffer);
extern void vboLoad(vbo_t* vbo, unsigned int elementCount, float* vertexBuffer, float* texcoordBuffer, float* normalBuffer);
extern void vboSetPointer(vbo_t* vbo, GLenum arrayType, int status);
extern void vboEnablePointer(vbo_t* vbo, GLenum arrayType);
extern void vboDisablePointer(vbo_t* vbo, GLenum arrayType);
extern void vboSetFaceCount(vbo_t* vbo, unsigned int count);
extern void vboDrawArrays(vbo_t* vbo);
extern void vboDraw(vbo_t* vbo);

#endif /*EXH_SYSTEM_GRAPHICS_OBJECT_VBO_H_*/
