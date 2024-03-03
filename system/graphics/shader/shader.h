#ifndef SYSTEM_GRAPHICS_SHADER_SHADER_H_
#define SYSTEM_GRAPHICS_SHADER_SHADER_H_

typedef struct {
	char *name;
	char *filename;
	unsigned int id;
	int type;
	int ok;
	time_t fileLastModifiedTime;
} shader_t;

typedef struct {
	char *name;
	unsigned int id;
	shader_t **attachedShaders;
	unsigned int attachedShadersCount;
	int ok;
} shaderProgram_t;

extern void shaderProgramAddShaderByName(const char *shaderProgramName, const char *shaderName);
extern void shaderProgramAttachShader(shaderProgram_t *shaderProgram, shader_t *shader);
extern void shaderProgramLink(shaderProgram_t *shaderProgram);
extern void shaderProgramUse(shaderProgram_t *shaderProgram);

extern void shaderProgramAttachAndLink(shaderProgram_t *shaderProgram);

extern void shaderProgramCreateId(shaderProgram_t *shaderProgram);
extern shaderProgram_t *shaderProgramLoad(const char *name);
extern void shaderProgramDeinit(shaderProgram_t* shaderProgram);
extern shader_t *shaderLoad(const char *name, const char *_filename);
extern void shaderDeinit(shader_t* shader);

extern unsigned int getUniformLocation(const char *variable);

extern void disableShaderProgram();
extern void activateShaderProgram(const char *name);

#endif /* SYSTEM_GRAPHICS_SHADER_SHADER_H_ */
