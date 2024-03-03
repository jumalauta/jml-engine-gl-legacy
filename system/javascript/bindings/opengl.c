/**
 * @page JSAPI JavaScript Public API
 * JavaScript Public API refers that a function can be called from the JavaScript as well.
 * Please refer to the JavaScript engine documentation for more information and examples.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <duktape.h>

#include "system/debug/debug.h"

#include "graphicsIncludes.h"
#include "system/graphics/shader/shader.h"
 #include "system/datatypes/memory.h"

#include "bindings.h"


static int duk_getUniformLocation(duk_context *ctx)
{
	const char* variable = (const char*)duk_get_string(ctx, 0);

	duk_push_uint(ctx, getUniformLocation(variable));

	return 1;
}

static int duk_glUniformf(duk_context *ctx)
{
	int argc = duk_get_top(ctx);
	if(argc<2 || argc>5)
	{
		debugErrorPrintf("Argument count invalid! count:'%d'", argc);
		return 0;
	}

	unsigned int uniformLocation = (unsigned int)duk_get_uint(ctx, 0);
	float value1 = 0.0f;
	float value2 = 0.0f;
	float value3 = 0.0f;
	float value4 = 0.0f;

	switch(argc)
	{
		case 5:
			value4 = (float)duk_get_number(ctx, 4);
		case 4:
			value3 = (float)duk_get_number(ctx, 3);
		case 3:
			value2 = (float)duk_get_number(ctx, 2);
		case 2:
			value1 = (float)duk_get_number(ctx, 1);
		default:
			break;
	}

	switch(argc)
	{
		case 5:
			glUniform4f(uniformLocation, value1, value2, value3, value4);
			break;
		case 4:
			glUniform3f(uniformLocation, value1, value2, value3);
			break;
		case 3:
			glUniform2f(uniformLocation, value1, value2);
			break;
		case 2:
			glUniform1f(uniformLocation, value1);
			break;
		default:
			break;
	}

	return 0;
}

static int duk_glUniformi(duk_context *ctx)
{
	int argc = duk_get_top(ctx);
	if(argc<2 || argc>5)
	{
		debugErrorPrintf("Argument count invalid! count:'%d'", argc);
		return 0;
	}

	unsigned int uniformLocation = (unsigned int)duk_get_uint(ctx, 0);
	int value1 = 0;
	int value2 = 0;
	int value3 = 0;
	int value4 = 0;

	switch(argc)
	{
		case 5:
			value4 = (int)duk_get_int(ctx, 4);
		case 4:
			value3 = (int)duk_get_int(ctx, 3);
		case 3:
			value2 = (int)duk_get_int(ctx, 2);
		case 2:
			value1 = (int)duk_get_int(ctx, 1);
		default:
			break;
	}

	switch(argc)
	{
		case 5:
			glUniform4i(uniformLocation, value1, value2, value3, value4);
			break;
		case 4:
			glUniform3i(uniformLocation, value1, value2, value3);
			break;
		case 3:
			glUniform2i(uniformLocation, value1, value2);
			break;
		case 2:
			glUniform1i(uniformLocation, value1);
			break;
		default:
			break;
	}

	return 0;
}

static int duk_disableShaderProgram(duk_context *ctx)
{
	disableShaderProgram();

	return 0;
}

static int duk_activateShaderProgram(duk_context *ctx)
{
	const char* name = (const char*)duk_get_string(ctx, 0);

	activateShaderProgram(name);

	return 0;
}

static int duk_shaderProgramUse(duk_context *ctx)
{
	shaderProgram_t *shaderProgram = (shaderProgram_t*)duk_get_pointer(ctx, 0);

	shaderProgramUse(shaderProgram);

	return 0;
}

static duk_idx_t duk_push_shader_program_object(duk_context *ctx, shaderProgram_t *shaderProgram)
{
	assert(ctx != NULL);

	duk_idx_t shaderProgram_obj = duk_push_object(ctx);
	
	if (shaderProgram == NULL)
	{
		return shaderProgram_obj;
	}

	duk_push_pointer(ctx, (void*)shaderProgram);
	duk_put_prop_string(ctx, shaderProgram_obj, "ptr");
	duk_push_string(ctx, (const char*)shaderProgram->name);
	duk_put_prop_string(ctx, shaderProgram_obj, "name");
	duk_push_uint(ctx, shaderProgram->id);
	duk_put_prop_string(ctx, shaderProgram_obj, "id");
	duk_push_int(ctx, shaderProgram->ok);
	duk_put_prop_string(ctx, shaderProgram_obj, "ok");

	return shaderProgram_obj;
}

static duk_idx_t duk_push_shader_object(duk_context *ctx, shader_t *shader)
{
	assert(ctx != NULL);

	duk_idx_t shader_obj = duk_push_object(ctx);
	
	if (shader == NULL)
	{
		return shader_obj;
	}

	duk_push_pointer(ctx, (void*)shader);
	duk_put_prop_string(ctx, shader_obj, "ptr");
	duk_push_string(ctx, (const char*)shader->name);
	duk_put_prop_string(ctx, shader_obj, "name");
	duk_push_string(ctx, (const char*)shader->filename);
	duk_put_prop_string(ctx, shader_obj, "filename");
	duk_push_uint(ctx, shader->id);
	duk_put_prop_string(ctx, shader_obj, "id");
	duk_push_int(ctx, shader->ok);
	duk_put_prop_string(ctx, shader_obj, "ok");

	return shader_obj;
}

static int duk_shaderProgramLoad(duk_context *ctx)
{
	const char* name = (const char*)duk_get_string(ctx, 0);

	//debugPrintf("shaderProgramLoad(%s)",name);
	duk_push_shader_program_object(ctx, shaderProgramLoad(name));

	return 1;
}


static int duk_getShaderProgramFromMemory(duk_context *ctx)
{
	const char* name = (const char*)duk_get_string(ctx, 0);

	duk_push_shader_program_object(ctx, getShaderProgramFromMemory(name));

	return 1;
}


static int duk_shaderLoad(duk_context *ctx)
{
	const char* shaderName = (const char*)duk_get_string(ctx, 0);
	const char* shaderFilename = (const char*)duk_get_string(ctx, 1);

	duk_push_shader_object(ctx, shaderLoad(shaderName, shaderFilename));

	return 1;
}

static int duk_shaderProgramAddShaderByName(duk_context *ctx)
{
	const char* shaderProgramName = (const char*)duk_get_string(ctx, 0);
	const char* shaderName = (const char*)duk_get_string(ctx, 1);

	//debugPrintf("shaderProgramAddShaderByName(%s,%s)",shaderProgramName, shaderName);
	shaderProgramAddShaderByName(shaderProgramName, shaderName);

	return 0;
}

static int duk_shaderProgramAttachAndLink(duk_context *ctx)
{
	shaderProgram_t *shaderProgram = (shaderProgram_t*)duk_get_pointer(ctx, 0);

	shaderProgramAttachAndLink(shaderProgram);

	return 0;
}

DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG1(/*EMPTY*/,/*EMPTY*/,glActiveTexture, GLenum, duk_get_uint(ctx, 0))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG2(/*EMPTY*/,/*EMPTY*/,glUniform1f, GLint, duk_get_int(ctx, 0), GLfloat, duk_get_number(ctx, 1))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG2(/*EMPTY*/,/*EMPTY*/,glUniform1i, GLint, duk_get_int(ctx, 0), GLint, duk_get_int(ctx, 1))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG3(/*EMPTY*/,/*EMPTY*/,glUniform2f, GLint, duk_get_int(ctx, 0), GLfloat, duk_get_number(ctx, 1), GLfloat, duk_get_number(ctx, 2))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG3(/*EMPTY*/,/*EMPTY*/,glUniform2i, GLint, duk_get_int(ctx, 0), GLint, duk_get_int(ctx, 1), GLint, duk_get_int(ctx, 2))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG4(/*EMPTY*/,/*EMPTY*/,glUniform3f, GLint, duk_get_int(ctx, 0), GLfloat, duk_get_number(ctx, 1), GLfloat, duk_get_number(ctx, 2), GLfloat, duk_get_number(ctx, 3))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG4(/*EMPTY*/,/*EMPTY*/,glUniform3i, GLint, duk_get_int(ctx, 0), GLint, duk_get_int(ctx, 1), GLint, duk_get_int(ctx, 2), GLint, duk_get_int(ctx, 3))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG5(/*EMPTY*/,/*EMPTY*/,glUniform4f, GLint, duk_get_int(ctx, 0), GLfloat, duk_get_number(ctx, 1), GLfloat, duk_get_number(ctx, 2), GLfloat, duk_get_number(ctx, 3), GLfloat, duk_get_number(ctx, 4))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG5(/*EMPTY*/,/*EMPTY*/,glUniform4i, GLint, duk_get_int(ctx, 0), GLint, duk_get_int(ctx, 1), GLint, duk_get_int(ctx, 2), GLint, duk_get_int(ctx, 3), GLint, duk_get_int(ctx, 4))

void bindJsOpenGlFunctions(duk_context *ctx)
{
	//engine function binding
	bindCFunctionToJs(shaderProgramLoad, 1);
	bindCFunctionToJs(getShaderProgramFromMemory, 1);
	bindCFunctionToJs(shaderLoad, 2);
	bindCFunctionToJs(shaderProgramAddShaderByName, 2);
	bindCFunctionToJs(shaderProgramAttachAndLink, 1);

	//OpenGL external function binding
	bindCFunctionToJs(getUniformLocation, 1);
	bindCFunctionToJs(glUniformf, DUK_VARARGS);
	bindCFunctionToJs(glUniformi, DUK_VARARGS);
	bindCFunctionToJs(disableShaderProgram, 0);
	bindCFunctionToJs(activateShaderProgram, 1);
	bindCFunctionToJs(shaderProgramUse, 1);
	
	//Some stolen bindings from Duktape OpenGL
	duk_gl_bind_opengl_wrapper(ctx, glActiveTexture, 1);
	duk_gl_bind_opengl_wrapper(ctx, glUniform1f, 2);
	duk_gl_bind_opengl_wrapper(ctx, glUniform1i, 2);
	duk_gl_bind_opengl_wrapper(ctx, glUniform2f, 3);
	duk_gl_bind_opengl_wrapper(ctx, glUniform2i, 3);
	duk_gl_bind_opengl_wrapper(ctx, glUniform3f, 4);
	duk_gl_bind_opengl_wrapper(ctx, glUniform3i, 4);
	duk_gl_bind_opengl_wrapper(ctx, glUniform4f, 5);
	duk_gl_bind_opengl_wrapper(ctx, glUniform4i, 5);

	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE0);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE1);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE2);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE3);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE4);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE5);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE6);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE7);
#ifndef TINYGL
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE8);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE9);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE10);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE11);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE12);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE13);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE14);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE15);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE16);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE17);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE18);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE19);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE20);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE21);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE22);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE23);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE24);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE25);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE26);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE27);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE28);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE29);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE30);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE31);
#endif
	duk_gl_push_opengl_constant_property(ctx, GL_REFLECTION_MAP);
}
