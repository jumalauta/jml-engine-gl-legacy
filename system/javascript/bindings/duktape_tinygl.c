/*
 *  Duktape OpenGL 0.5 - OpenGL bindings for Duktape embeddable Javascript engine.
 *  See AUTHORS.rst and LICENSE.txt for copyright and licensing information.
 *  
 *  Duktape OpenGL: https://github.com/mrautio/duktape-opengl/
 *  Duktape: http://duktape.org/
 *  OpenGL API reference: https://www.opengl.org/registry/
 */

#include <duktape.h>

#include <GL/gl.h>

typedef unsigned int GLbitfield;
typedef float GLclampf;
typedef double GLclampd;

#define DUK_GL_TINYGL

/*
 *  Macro for binding OpenGL wrapper C function as Duktape JavaScript function
 */
#define duk_gl_bind_opengl_wrapper(ctx, c_function_name, argument_count) \
	duk_push_c_function((ctx), duk_gl_##c_function_name, (argument_count)); \
	duk_put_prop_string((ctx), -2, #c_function_name)

/*
 *  Macro for setting OpenGL constants
 */
#define duk_gl_push_opengl_constant_property(ctx, opengl_constant) \
	duk_push_uint((ctx), (opengl_constant)); \
	duk_put_prop_string((ctx), -2, #opengl_constant)

/*
 *  Macro for handling of arrays
 */
DUK_LOCAL size_t duk_gl_determine_array_length(duk_context *ctx, duk_idx_t obj_index, duk_size_t sz, size_t num)
{
	size_t array_length = sz;
	if (sz < 1)
	{
		/* use <arrayVariable>.length is array size not explicitly defined */
		duk_get_prop_string(ctx, obj_index, "length");
		array_length = (unsigned int)duk_get_uint(ctx, -1);
		duk_pop(ctx);
	}
	/* prevent buffer overflow by clamping the value */
	if (array_length > num)
	{
		array_length = num;
	}
	return array_length;
}

#define DUK_GL_ARRAY_GET_FUNCTION(argtypedef1, arg1) \
DUK_LOCAL argtypedef1 *duk_gl_get_##argtypedef1##_array(duk_context *ctx, duk_idx_t obj_index, duk_size_t sz, argtypedef1 *array, size_t num) \
{ \
	if (duk_is_array(ctx, obj_index)) \
	{ \
		size_t array_length = duk_gl_determine_array_length(ctx, obj_index, sz, num); \
		unsigned int i = 0; \
		for(i=0; i<array_length; i++) \
		{ \
			duk_get_prop_index(ctx, obj_index, (duk_uarridx_t)i); \
			array[i] = (argtypedef1)duk_get_##arg1(ctx, -1); \
			duk_pop(ctx); \
		} \
		return array; \
	} \
	return NULL; \
}

#define DUK_GL_ARRAY_PUT_FUNCTION(argtypedef1, arg1) \
DUK_LOCAL duk_bool_t duk_gl_put_##argtypedef1##_array(duk_context *ctx, duk_idx_t obj_index, duk_size_t sz, argtypedef1 *array, size_t num) \
{ \
	if (duk_is_array(ctx, obj_index)) \
	{ \
		duk_get_prop(ctx, obj_index); \
		size_t array_length = duk_gl_determine_array_length(ctx, obj_index, sz, num); \
		unsigned int i = 0; \
		for(i=0; i<array_length; i++) \
		{ \
			duk_push_##arg1(ctx, (argtypedef1)array[i]); \
			duk_put_prop_index(ctx, obj_index, (duk_uarridx_t)i); \
		} \
		duk_pop(ctx); \
		return 1; \
	} \
	return 0; \
}

DUK_GL_ARRAY_GET_FUNCTION(GLfloat, number)

/*
DUK_GL_ARRAY_GET_FUNCTION(GLboolean, number)
DUK_GL_ARRAY_PUT_FUNCTION(GLboolean, number)
DUK_GL_ARRAY_GET_FUNCTION(GLbyte, number)
DUK_GL_ARRAY_GET_FUNCTION(GLubyte, number)
DUK_GL_ARRAY_PUT_FUNCTION(GLubyte, number)
DUK_GL_ARRAY_GET_FUNCTION(GLdouble, number)
DUK_GL_ARRAY_PUT_FUNCTION(GLdouble, number)
DUK_GL_ARRAY_GET_FUNCTION(GLfloat, number)
DUK_GL_ARRAY_PUT_FUNCTION(GLfloat, number)
DUK_GL_ARRAY_GET_FUNCTION(GLclampf, number)
DUK_GL_ARRAY_GET_FUNCTION(GLint, number)
DUK_GL_ARRAY_PUT_FUNCTION(GLint, number)
DUK_GL_ARRAY_GET_FUNCTION(GLuint, number)
DUK_GL_ARRAY_PUT_FUNCTION(GLuint, number)
DUK_GL_ARRAY_GET_FUNCTION(GLshort, number)
DUK_GL_ARRAY_GET_FUNCTION(GLushort, number)
DUK_GL_ARRAY_PUT_FUNCTION(GLushort, number)
*/

/*
 *  Wrapper macros for OpenGL C functions.
 *  Macro name defines the amount of return types and function arguments it supports.
 *  Macros take arguments in following manner (<Function name>, <OpenGL variable type 1>, <Duktape push type 1>, ...N)
 *  - Function name = OpenGL C function name
 *  - OpenGL variable type N = OpenGL's C variable type definition, for example, "GLenum"
 *  - Duktape push type 1 = Duktape API's duk_get_... function's type name, for example, duk_get_number
 */
#define DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG0(PREPROCESS, POSTPROCESS, c_function_name) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	c_function_name(); \
	POSTPROCESS; \
	return 0; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET1_ARG0(PREPROCESS, POSTPROCESS, c_function_name, rettypedef1) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	duk_push_##rettypedef1(ctx, c_function_name()); \
	POSTPROCESS; \
	return 1; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG1(PREPROCESS, POSTPROCESS, c_function_name, argtypedef1, arg1) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	c_function_name( \
		(argtypedef1)arg1 \
	); \
	POSTPROCESS; \
	return 0; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET1_ARG1(PREPROCESS, POSTPROCESS, c_function_name, rettypedef1, argtypedef1, arg1) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	duk_push_##rettypedef1(ctx, c_function_name( \
		(argtypedef1)arg1 \
	)); \
	POSTPROCESS; \
	return 1; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG2(PREPROCESS, POSTPROCESS, c_function_name, argtypedef1, arg1, argtypedef2, arg2) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2 \
	); \
	POSTPROCESS; \
	return 0; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET1_ARG2(PREPROCESS, POSTPROCESS, c_function_name, rettypedef1, argtypedef1, arg1, argtypedef2, arg2) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	duk_push_##rettypedef1(ctx, c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2 \
	)); \
	POSTPROCESS; \
	return 1; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG3(PREPROCESS, POSTPROCESS, c_function_name, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3 \
	); \
	POSTPROCESS; \
	return 0; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET1_ARG3(PREPROCESS, POSTPROCESS, c_function_name, rettypedef1, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	duk_push_##rettypedef1(ctx, c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3 \
	)); \
	POSTPROCESS; \
	return 1; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG4(PREPROCESS, POSTPROCESS, c_function_name, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4 \
	); \
	POSTPROCESS; \
	return 0; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET1_ARG4(PREPROCESS, POSTPROCESS, c_function_name, rettypedef1, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	duk_push_##rettypedef1(ctx, c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4 \
	)); \
	POSTPROCESS; \
	return 1; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG5(PREPROCESS, POSTPROCESS, c_function_name, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4, argtypedef5, arg5) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4,  \
		(argtypedef5)arg5 \
	); \
	POSTPROCESS; \
	return 0; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET1_ARG5(PREPROCESS, POSTPROCESS, c_function_name, rettypedef1, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4, argtypedef5, arg5) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	duk_push_##rettypedef1(ctx, c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4,  \
		(argtypedef5)arg5 \
	)); \
	POSTPROCESS; \
	return 1; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG6(PREPROCESS, POSTPROCESS, c_function_name, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4, argtypedef5, arg5, argtypedef6, arg6) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4,  \
		(argtypedef5)arg5,  \
		(argtypedef6)arg6 \
	); \
	POSTPROCESS; \
	return 0; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET1_ARG6(PREPROCESS, POSTPROCESS, c_function_name, rettypedef1, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4, argtypedef5, arg5, argtypedef6, arg6) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	duk_push_##rettypedef1(ctx, c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4,  \
		(argtypedef5)arg5,  \
		(argtypedef6)arg6 \
	)); \
	POSTPROCESS; \
	return 1; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG7(PREPROCESS, POSTPROCESS, c_function_name, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4, argtypedef5, arg5, argtypedef6, arg6, argtypedef7, arg7) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4,  \
		(argtypedef5)arg5,  \
		(argtypedef6)arg6,  \
		(argtypedef7)arg7 \
	); \
	POSTPROCESS; \
	return 0; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET1_ARG7(PREPROCESS, POSTPROCESS, c_function_name, rettypedef1, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4, argtypedef5, arg5, argtypedef6, arg6, argtypedef7, arg7) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	duk_push_##rettypedef1(ctx, c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4,  \
		(argtypedef5)arg5,  \
		(argtypedef6)arg6,  \
		(argtypedef7)arg7 \
	)); \
	POSTPROCESS; \
	return 1; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG8(PREPROCESS, POSTPROCESS, c_function_name, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4, argtypedef5, arg5, argtypedef6, arg6, argtypedef7, arg7, argtypedef8, arg8) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4,  \
		(argtypedef5)arg5,  \
		(argtypedef6)arg6,  \
		(argtypedef7)arg7,  \
		(argtypedef8)arg8 \
	); \
	POSTPROCESS; \
	return 0; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET1_ARG8(PREPROCESS, POSTPROCESS, c_function_name, rettypedef1, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4, argtypedef5, arg5, argtypedef6, arg6, argtypedef7, arg7, argtypedef8, arg8) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	duk_push_##rettypedef1(ctx, c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4,  \
		(argtypedef5)arg5,  \
		(argtypedef6)arg6,  \
		(argtypedef7)arg7,  \
		(argtypedef8)arg8 \
	)); \
	POSTPROCESS; \
	return 1; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG9(PREPROCESS, POSTPROCESS, c_function_name, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4, argtypedef5, arg5, argtypedef6, arg6, argtypedef7, arg7, argtypedef8, arg8, argtypedef9, arg9) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4,  \
		(argtypedef5)arg5,  \
		(argtypedef6)arg6,  \
		(argtypedef7)arg7,  \
		(argtypedef8)arg8,  \
		(argtypedef9)arg9 \
	); \
	POSTPROCESS; \
	return 0; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET1_ARG9(PREPROCESS, POSTPROCESS, c_function_name, rettypedef1, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4, argtypedef5, arg5, argtypedef6, arg6, argtypedef7, arg7, argtypedef8, arg8, argtypedef9, arg9) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	duk_push_##rettypedef1(ctx, c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4,  \
		(argtypedef5)arg5,  \
		(argtypedef6)arg6,  \
		(argtypedef7)arg7,  \
		(argtypedef8)arg8,  \
		(argtypedef9)arg9 \
	)); \
	POSTPROCESS; \
	return 1; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG10(PREPROCESS, POSTPROCESS, c_function_name, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4, argtypedef5, arg5, argtypedef6, arg6, argtypedef7, arg7, argtypedef8, arg8, argtypedef9, arg9, argtypedef10, arg10) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4,  \
		(argtypedef5)arg5,  \
		(argtypedef6)arg6,  \
		(argtypedef7)arg7,  \
		(argtypedef8)arg8,  \
		(argtypedef9)arg9,  \
		(argtypedef10)arg10 \
	); \
	POSTPROCESS; \
	return 0; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET1_ARG10(PREPROCESS, POSTPROCESS, c_function_name, rettypedef1, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4, argtypedef5, arg5, argtypedef6, arg6, argtypedef7, arg7, argtypedef8, arg8, argtypedef9, arg9, argtypedef10, arg10) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	duk_push_##rettypedef1(ctx, c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4,  \
		(argtypedef5)arg5,  \
		(argtypedef6)arg6,  \
		(argtypedef7)arg7,  \
		(argtypedef8)arg8,  \
		(argtypedef9)arg9,  \
		(argtypedef10)arg10 \
	)); \
	POSTPROCESS; \
	return 1; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG11(PREPROCESS, POSTPROCESS, c_function_name, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4, argtypedef5, arg5, argtypedef6, arg6, argtypedef7, arg7, argtypedef8, arg8, argtypedef9, arg9, argtypedef10, arg10, argtypedef11, arg11) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4,  \
		(argtypedef5)arg5,  \
		(argtypedef6)arg6,  \
		(argtypedef7)arg7,  \
		(argtypedef8)arg8,  \
		(argtypedef9)arg9,  \
		(argtypedef10)arg10,  \
		(argtypedef11)arg11 \
	); \
	POSTPROCESS; \
	return 0; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET1_ARG11(PREPROCESS, POSTPROCESS, c_function_name, rettypedef1, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4, argtypedef5, arg5, argtypedef6, arg6, argtypedef7, arg7, argtypedef8, arg8, argtypedef9, arg9, argtypedef10, arg10, argtypedef11, arg11) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	duk_push_##rettypedef1(ctx, c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4,  \
		(argtypedef5)arg5,  \
		(argtypedef6)arg6,  \
		(argtypedef7)arg7,  \
		(argtypedef8)arg8,  \
		(argtypedef9)arg9,  \
		(argtypedef10)arg10,  \
		(argtypedef11)arg11 \
	)); \
	POSTPROCESS; \
	return 1; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG12(PREPROCESS, POSTPROCESS, c_function_name, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4, argtypedef5, arg5, argtypedef6, arg6, argtypedef7, arg7, argtypedef8, arg8, argtypedef9, arg9, argtypedef10, arg10, argtypedef11, arg11, argtypedef12, arg12) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4,  \
		(argtypedef5)arg5,  \
		(argtypedef6)arg6,  \
		(argtypedef7)arg7,  \
		(argtypedef8)arg8,  \
		(argtypedef9)arg9,  \
		(argtypedef10)arg10,  \
		(argtypedef11)arg11,  \
		(argtypedef12)arg12 \
	); \
	POSTPROCESS; \
	return 0; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET1_ARG12(PREPROCESS, POSTPROCESS, c_function_name, rettypedef1, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4, argtypedef5, arg5, argtypedef6, arg6, argtypedef7, arg7, argtypedef8, arg8, argtypedef9, arg9, argtypedef10, arg10, argtypedef11, arg11, argtypedef12, arg12) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	duk_push_##rettypedef1(ctx, c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4,  \
		(argtypedef5)arg5,  \
		(argtypedef6)arg6,  \
		(argtypedef7)arg7,  \
		(argtypedef8)arg8,  \
		(argtypedef9)arg9,  \
		(argtypedef10)arg10,  \
		(argtypedef11)arg11,  \
		(argtypedef12)arg12 \
	)); \
	POSTPROCESS; \
	return 1; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG13(PREPROCESS, POSTPROCESS, c_function_name, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4, argtypedef5, arg5, argtypedef6, arg6, argtypedef7, arg7, argtypedef8, arg8, argtypedef9, arg9, argtypedef10, arg10, argtypedef11, arg11, argtypedef12, arg12, argtypedef13, arg13) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4,  \
		(argtypedef5)arg5,  \
		(argtypedef6)arg6,  \
		(argtypedef7)arg7,  \
		(argtypedef8)arg8,  \
		(argtypedef9)arg9,  \
		(argtypedef10)arg10,  \
		(argtypedef11)arg11,  \
		(argtypedef12)arg12,  \
		(argtypedef13)arg13 \
	); \
	POSTPROCESS; \
	return 0; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET1_ARG13(PREPROCESS, POSTPROCESS, c_function_name, rettypedef1, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4, argtypedef5, arg5, argtypedef6, arg6, argtypedef7, arg7, argtypedef8, arg8, argtypedef9, arg9, argtypedef10, arg10, argtypedef11, arg11, argtypedef12, arg12, argtypedef13, arg13) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	duk_push_##rettypedef1(ctx, c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4,  \
		(argtypedef5)arg5,  \
		(argtypedef6)arg6,  \
		(argtypedef7)arg7,  \
		(argtypedef8)arg8,  \
		(argtypedef9)arg9,  \
		(argtypedef10)arg10,  \
		(argtypedef11)arg11,  \
		(argtypedef12)arg12,  \
		(argtypedef13)arg13 \
	)); \
	POSTPROCESS; \
	return 1; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG14(PREPROCESS, POSTPROCESS, c_function_name, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4, argtypedef5, arg5, argtypedef6, arg6, argtypedef7, arg7, argtypedef8, arg8, argtypedef9, arg9, argtypedef10, arg10, argtypedef11, arg11, argtypedef12, arg12, argtypedef13, arg13, argtypedef14, arg14) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4,  \
		(argtypedef5)arg5,  \
		(argtypedef6)arg6,  \
		(argtypedef7)arg7,  \
		(argtypedef8)arg8,  \
		(argtypedef9)arg9,  \
		(argtypedef10)arg10,  \
		(argtypedef11)arg11,  \
		(argtypedef12)arg12,  \
		(argtypedef13)arg13,  \
		(argtypedef14)arg14 \
	); \
	POSTPROCESS; \
	return 0; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET1_ARG14(PREPROCESS, POSTPROCESS, c_function_name, rettypedef1, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4, argtypedef5, arg5, argtypedef6, arg6, argtypedef7, arg7, argtypedef8, arg8, argtypedef9, arg9, argtypedef10, arg10, argtypedef11, arg11, argtypedef12, arg12, argtypedef13, arg13, argtypedef14, arg14) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	duk_push_##rettypedef1(ctx, c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4,  \
		(argtypedef5)arg5,  \
		(argtypedef6)arg6,  \
		(argtypedef7)arg7,  \
		(argtypedef8)arg8,  \
		(argtypedef9)arg9,  \
		(argtypedef10)arg10,  \
		(argtypedef11)arg11,  \
		(argtypedef12)arg12,  \
		(argtypedef13)arg13,  \
		(argtypedef14)arg14 \
	)); \
	POSTPROCESS; \
	return 1; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG15(PREPROCESS, POSTPROCESS, c_function_name, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4, argtypedef5, arg5, argtypedef6, arg6, argtypedef7, arg7, argtypedef8, arg8, argtypedef9, arg9, argtypedef10, arg10, argtypedef11, arg11, argtypedef12, arg12, argtypedef13, arg13, argtypedef14, arg14, argtypedef15, arg15) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4,  \
		(argtypedef5)arg5,  \
		(argtypedef6)arg6,  \
		(argtypedef7)arg7,  \
		(argtypedef8)arg8,  \
		(argtypedef9)arg9,  \
		(argtypedef10)arg10,  \
		(argtypedef11)arg11,  \
		(argtypedef12)arg12,  \
		(argtypedef13)arg13,  \
		(argtypedef14)arg14,  \
		(argtypedef15)arg15 \
	); \
	POSTPROCESS; \
	return 0; \
}

#define DUK_GL_C_WRAPPER_FUNCTION_RET1_ARG15(PREPROCESS, POSTPROCESS, c_function_name, rettypedef1, argtypedef1, arg1, argtypedef2, arg2, argtypedef3, arg3, argtypedef4, arg4, argtypedef5, arg5, argtypedef6, arg6, argtypedef7, arg7, argtypedef8, arg8, argtypedef9, arg9, argtypedef10, arg10, argtypedef11, arg11, argtypedef12, arg12, argtypedef13, arg13, argtypedef14, arg14, argtypedef15, arg15) \
DUK_LOCAL duk_ret_t duk_gl_##c_function_name(duk_context *ctx) \
{ \
	PREPROCESS; \
	duk_push_##rettypedef1(ctx, c_function_name( \
		(argtypedef1)arg1,  \
		(argtypedef2)arg2,  \
		(argtypedef3)arg3,  \
		(argtypedef4)arg4,  \
		(argtypedef5)arg5,  \
		(argtypedef6)arg6,  \
		(argtypedef7)arg7,  \
		(argtypedef8)arg8,  \
		(argtypedef9)arg9,  \
		(argtypedef10)arg10,  \
		(argtypedef11)arg11,  \
		(argtypedef12)arg12,  \
		(argtypedef13)arg13,  \
		(argtypedef14)arg14,  \
		(argtypedef15)arg15 \
	)); \
	POSTPROCESS; \
	return 1; \
}

/*
 *  OpenGL wrapper function definitions
 */
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG3(/*EMPTY*/,/*EMPTY*/,glTexGeni, GLenum, duk_get_uint(ctx, 0), GLenum, duk_get_uint(ctx, 1), GLint, duk_get_int(ctx, 2))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG2(/*EMPTY*/,/*EMPTY*/,glLineStipple, GLint, duk_get_int(ctx, 0), GLushort, duk_get_uint(ctx, 1))

DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG1(/*EMPTY*/,/*EMPTY*/,glEnable, GLenum, duk_get_uint(ctx, 0))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG1(/*EMPTY*/,/*EMPTY*/,glDisable, GLenum, duk_get_uint(ctx, 0))

DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG1(/*EMPTY*/,/*EMPTY*/,glShadeModel, GLenum, duk_get_uint(ctx, 0))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG1(/*EMPTY*/,/*EMPTY*/,glCullFace, GLenum, duk_get_uint(ctx, 0))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG2(/*EMPTY*/,/*EMPTY*/,glPolygonMode, GLenum, duk_get_uint(ctx, 0), GLenum, duk_get_uint(ctx, 1))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG1(/*EMPTY*/,/*EMPTY*/,glBegin, GLenum, duk_get_uint(ctx, 0))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG0(/*EMPTY*/,/*EMPTY*/,glEnd)

DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG2(/*EMPTY*/,/*EMPTY*/,glVertex2i, GLint, duk_get_int(ctx, 0), GLint, duk_get_int(ctx, 1))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG2(/*EMPTY*/,/*EMPTY*/,glVertex2d, GLdouble, duk_get_number(ctx, 0), GLdouble, duk_get_number(ctx, 1))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG2(/*EMPTY*/,/*EMPTY*/,glVertex2f, GLfloat, duk_get_number(ctx, 0), GLfloat, duk_get_number(ctx, 1))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG3(/*EMPTY*/,/*EMPTY*/,glVertex3i, GLint, duk_get_int(ctx, 0), GLint, duk_get_int(ctx, 1), GLint, duk_get_int(ctx, 2))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG3(/*EMPTY*/,/*EMPTY*/,glVertex3d, GLdouble, duk_get_number(ctx, 0), GLdouble, duk_get_number(ctx, 1), GLdouble, duk_get_number(ctx, 2))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG3(/*EMPTY*/,/*EMPTY*/,glVertex3f, GLfloat, duk_get_number(ctx, 0), GLfloat, duk_get_number(ctx, 1), GLfloat, duk_get_number(ctx, 2))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG4(/*EMPTY*/,/*EMPTY*/,glVertex4d, GLdouble, duk_get_number(ctx, 0), GLdouble, duk_get_number(ctx, 1), GLdouble, duk_get_number(ctx, 2), GLdouble, duk_get_number(ctx, 3))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG4(/*EMPTY*/,/*EMPTY*/,glVertex4f, GLfloat, duk_get_number(ctx, 0), GLfloat, duk_get_number(ctx, 1), GLfloat, duk_get_number(ctx, 2), GLfloat, duk_get_number(ctx, 3))

DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG3(/*EMPTY*/,/*EMPTY*/,glColor3d, GLdouble, duk_get_number(ctx, 0), GLdouble, duk_get_number(ctx, 1), GLdouble, duk_get_number(ctx, 2))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG3(/*EMPTY*/,/*EMPTY*/,glColor3f, GLfloat, duk_get_number(ctx, 0), GLfloat, duk_get_number(ctx, 1), GLfloat, duk_get_number(ctx, 2))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG4(/*EMPTY*/,/*EMPTY*/,glColor4d, GLdouble, duk_get_number(ctx, 0), GLdouble, duk_get_number(ctx, 1), GLdouble, duk_get_number(ctx, 2), GLdouble, duk_get_number(ctx, 3))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG4(/*EMPTY*/,/*EMPTY*/,glColor4f, GLfloat, duk_get_number(ctx, 0), GLfloat, duk_get_number(ctx, 1), GLfloat, duk_get_number(ctx, 2), GLfloat, duk_get_number(ctx, 3))

DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG3(/*EMPTY*/,/*EMPTY*/,glNormal3d, GLdouble, duk_get_number(ctx, 0), GLdouble, duk_get_number(ctx, 1), GLdouble, duk_get_number(ctx, 2))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG3(/*EMPTY*/,/*EMPTY*/,glNormal3f, GLfloat, duk_get_number(ctx, 0), GLfloat, duk_get_number(ctx, 1), GLfloat, duk_get_number(ctx, 2))

DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG1(/*EMPTY*/,/*EMPTY*/,glTexCoord1d, GLdouble, duk_get_number(ctx, 0))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG1(/*EMPTY*/,/*EMPTY*/,glTexCoord1f, GLfloat, duk_get_number(ctx, 0))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG2(/*EMPTY*/,/*EMPTY*/,glTexCoord2d, GLdouble, duk_get_number(ctx, 0), GLdouble, duk_get_number(ctx, 1))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG2(/*EMPTY*/,/*EMPTY*/,glTexCoord2f, GLfloat, duk_get_number(ctx, 0), GLfloat, duk_get_number(ctx, 1))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG3(/*EMPTY*/,/*EMPTY*/,glTexCoord3f, GLfloat, duk_get_number(ctx, 0), GLfloat, duk_get_number(ctx, 1), GLfloat, duk_get_number(ctx, 2))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG4(/*EMPTY*/,/*EMPTY*/,glTexCoord4f, GLfloat, duk_get_number(ctx, 0), GLfloat, duk_get_number(ctx, 1), GLfloat, duk_get_number(ctx, 2), GLfloat, duk_get_number(ctx, 3))


DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG1(/*EMPTY*/,/*EMPTY*/,glEdgeFlag, GLboolean, duk_get_boolean(ctx, 0))

DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG1(/*EMPTY*/,/*EMPTY*/,glMatrixMode, GLenum, duk_get_uint(ctx, 0))
/*
void glLoadMatrixf(const float *m);
*/
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG0(/*EMPTY*/,/*EMPTY*/,glLoadIdentity)
/*
void glMultMatrixf(const float *m);
*/
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG0(/*EMPTY*/,/*EMPTY*/,glPushMatrix)
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG0(/*EMPTY*/,/*EMPTY*/,glPopMatrix)
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG4(/*EMPTY*/,/*EMPTY*/,glRotatef, GLfloat, duk_get_number(ctx, 0), GLfloat, duk_get_number(ctx, 1), GLfloat, duk_get_number(ctx, 2), GLfloat, duk_get_number(ctx, 3))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG3(/*EMPTY*/,/*EMPTY*/,glTranslatef, GLfloat, duk_get_number(ctx, 0), GLfloat, duk_get_number(ctx, 1), GLfloat, duk_get_number(ctx, 2))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG3(/*EMPTY*/,/*EMPTY*/,glScalef, GLfloat, duk_get_number(ctx, 0), GLfloat, duk_get_number(ctx, 1), GLfloat, duk_get_number(ctx, 2))

DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG4(GLsizei var2 = duk_get_int(ctx, 2); GLsizei var3 = duk_get_int(ctx, 3); ,/*EMPTY*/,glViewport, GLint, duk_get_int(ctx, 0), GLint, duk_get_int(ctx, 1), GLsizei, var2, GLsizei, var3)
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG6(/*EMPTY*/,/*EMPTY*/,glFrustum, GLdouble, duk_get_number(ctx, 0), GLdouble, duk_get_number(ctx, 1), GLdouble, duk_get_number(ctx, 2), GLdouble, duk_get_number(ctx, 3), GLdouble, duk_get_number(ctx, 4), GLdouble, duk_get_number(ctx, 5))

/* lists */
DUK_GL_C_WRAPPER_FUNCTION_RET1_ARG1(GLsizei var0 = duk_get_int(ctx, 0); ,/*EMPTY*/,glGenLists, uint, GLsizei, var0)
DUK_GL_C_WRAPPER_FUNCTION_RET1_ARG1(/*EMPTY*/,/*EMPTY*/,glIsList, boolean, GLuint, duk_get_uint(ctx, 0))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG2(/*EMPTY*/,/*EMPTY*/,glNewList, GLuint, duk_get_uint(ctx, 0), GLenum, duk_get_uint(ctx, 1))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG0(/*EMPTY*/,/*EMPTY*/,glEndList)
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG1(/*EMPTY*/,/*EMPTY*/,glCallList, GLuint, duk_get_uint(ctx, 0))


/* clear */
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG1(/*EMPTY*/,/*EMPTY*/,glClear, GLbitfield, duk_get_uint(ctx, 0))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG4(/*EMPTY*/,/*EMPTY*/,glClearColor, GLclampf, duk_get_number(ctx, 0), GLclampf, duk_get_number(ctx, 1), GLclampf, duk_get_number(ctx, 2), GLclampf, duk_get_number(ctx, 3))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG1(/*EMPTY*/,/*EMPTY*/,glClearDepth, GLclampd, duk_get_number(ctx, 0))

/* selection */
DUK_GL_C_WRAPPER_FUNCTION_RET1_ARG1(/*EMPTY*/,/*EMPTY*/,glRenderMode, int, GLenum, duk_get_uint(ctx, 0))

/*
void glSelectBuffer(int size,unsigned int *buf);
*/

DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG0(/*EMPTY*/,/*EMPTY*/,glInitNames)
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG1(/*EMPTY*/,/*EMPTY*/,glPushName, GLuint, duk_get_uint(ctx, 0))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG0(/*EMPTY*/,/*EMPTY*/,glPopName)

DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG1(/*EMPTY*/,/*EMPTY*/,glLoadName, GLuint, duk_get_uint(ctx, 0))

/*
void glGenTextures(int n, unsigned int *textures);
void glDeleteTextures(int n, const unsigned int *textures);
*/
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG2(/*EMPTY*/,/*EMPTY*/,glBindTexture, GLenum, duk_get_uint(ctx, 0), GLuint, duk_get_uint(ctx, 1))
/*
void glTexImage2D( int target, int level, int components,
		    int width, int height, int border,
                    int format, int type, void *pixels);
*/
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG3(/*EMPTY*/,/*EMPTY*/,glTexEnvi, GLenum, duk_get_uint(ctx, 0), GLenum, duk_get_uint(ctx, 1), GLint, duk_get_int(ctx, 2))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG3(/*EMPTY*/,/*EMPTY*/,glTexParameteri, GLenum, duk_get_uint(ctx, 0), GLenum, duk_get_uint(ctx, 1), GLint, duk_get_int(ctx, 2))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG2(/*EMPTY*/,/*EMPTY*/,glPixelStorei, GLenum, duk_get_uint(ctx, 0), GLint, duk_get_int(ctx, 1))

/* lighting */

DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG3( GLfloat var2 [1]; ,/*EMPTY*/,glMaterialfv, GLenum, duk_get_uint(ctx, 0), GLenum, duk_get_uint(ctx, 1), const GLfloat *, duk_gl_get_GLfloat_array(ctx, 2, 1, var2, 1))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG3(/*EMPTY*/,/*EMPTY*/,glMaterialf, GLenum, duk_get_uint(ctx, 0), GLenum, duk_get_uint(ctx, 1), GLfloat, duk_get_number(ctx, 2))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG2(/*EMPTY*/,/*EMPTY*/,glColorMaterial, GLenum, duk_get_uint(ctx, 0), GLenum, duk_get_uint(ctx, 1))

DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG3( GLfloat var2 [1]; ,/*EMPTY*/,glLightfv, GLenum, duk_get_uint(ctx, 0), GLenum, duk_get_uint(ctx, 1), const GLfloat *, duk_gl_get_GLfloat_array(ctx, 2, 1, var2, 1))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG3(/*EMPTY*/,/*EMPTY*/,glLightf, GLenum, duk_get_uint(ctx, 0), GLenum, duk_get_uint(ctx, 1), GLfloat, duk_get_number(ctx, 2))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG2(/*EMPTY*/,/*EMPTY*/,glLightModeli, GLenum, duk_get_uint(ctx, 0), GLint, duk_get_int(ctx, 1))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG2( GLfloat var1 [1]; ,/*EMPTY*/,glLightModelfv, GLenum, duk_get_uint(ctx, 0), const GLfloat *, duk_gl_get_GLfloat_array(ctx, 1, 1, var1, 1))

DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG0(/*EMPTY*/,/*EMPTY*/,glFlush)
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG2(/*EMPTY*/,/*EMPTY*/,glHint, GLenum, duk_get_uint(ctx, 0), GLenum, duk_get_uint(ctx, 1))
/*
void glGetIntegerv(int pname,int *params);
void glGetFloatv(int pname, float *v);
*/
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG1(/*EMPTY*/,/*EMPTY*/,glFrontFace, GLenum, duk_get_uint(ctx, 0))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG1(/*EMPTY*/,/*EMPTY*/,glEnableClientState, GLenum, duk_get_uint(ctx, 0))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG1(/*EMPTY*/,/*EMPTY*/,glDisableClientState, GLenum, duk_get_uint(ctx, 0))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG1(/*EMPTY*/,/*EMPTY*/,glArrayElement, GLint, duk_get_int(ctx, 0))
/*
void glVertexPointer(GLint size, GLenum type, GLsizei stride,
                     const GLvoid *pointer);
void glColorPointer(GLint size, GLenum type, GLsizei stride,
                     const GLvoid *pointer);
void glNormalPointer(GLenum type, GLsizei stride,
                      const GLvoid *pointer);
void glTexCoordPointer(GLint size, GLenum type, GLsizei stride,
                       const GLvoid *pointer);
*/
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG2(/*EMPTY*/,/*EMPTY*/,glPolygonOffset, GLfloat, duk_get_number(ctx, 0), GLfloat, duk_get_number(ctx, 1))

DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG3(/*EMPTY*/,/*EMPTY*/,glColor3ub, GLubyte, duk_get_uint(ctx, 0), GLubyte, duk_get_uint(ctx, 1), GLubyte, duk_get_uint(ctx, 2))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG4(/*EMPTY*/,/*EMPTY*/,glColor4ub, GLubyte, duk_get_uint(ctx, 0), GLubyte, duk_get_uint(ctx, 1), GLubyte, duk_get_uint(ctx, 2), GLubyte, duk_get_uint(ctx, 3))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG1(/*EMPTY*/,/*EMPTY*/,glPushAttrib, GLbitfield, duk_get_uint(ctx, 0))
DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG0(/*EMPTY*/,/*EMPTY*/,glPopAttrib)

DUK_GL_C_WRAPPER_FUNCTION_RET0_ARG2(/*EMPTY*/,/*EMPTY*/,glBlendFunc, GLenum, duk_get_uint(ctx, 0), GLenum, duk_get_uint(ctx, 1))

/*
 *  OpenGL function bindings to JavaScript
 */
void duk_gl_bind_opengl_functions(duk_context *ctx)
{
	duk_gl_bind_opengl_wrapper(ctx, glTexGeni, 3);
	duk_gl_bind_opengl_wrapper(ctx, glLineStipple, 2);
	duk_gl_bind_opengl_wrapper(ctx, glColor4ub, 4);
	duk_gl_bind_opengl_wrapper(ctx, glColor3ub, 3);
	duk_gl_bind_opengl_wrapper(ctx, glPushAttrib, 1);
	duk_gl_bind_opengl_wrapper(ctx, glPopAttrib, 0);
	duk_gl_bind_opengl_wrapper(ctx, glBlendFunc, 2);

	duk_gl_bind_opengl_wrapper(ctx, glEnable, 1);
	duk_gl_bind_opengl_wrapper(ctx, glDisable, 1);
	duk_gl_bind_opengl_wrapper(ctx, glShadeModel, 1);
	duk_gl_bind_opengl_wrapper(ctx, glCullFace, 1);
	duk_gl_bind_opengl_wrapper(ctx, glPolygonMode, 2);
	duk_gl_bind_opengl_wrapper(ctx, glBegin, 1);
	duk_gl_bind_opengl_wrapper(ctx, glEnd, 0);
	duk_gl_bind_opengl_wrapper(ctx, glVertex2i, 2);
	duk_gl_bind_opengl_wrapper(ctx, glVertex2d, 2);
	duk_gl_bind_opengl_wrapper(ctx, glVertex2f, 2);
	duk_gl_bind_opengl_wrapper(ctx, glVertex3d, 3);
	duk_gl_bind_opengl_wrapper(ctx, glVertex3i, 3);
	duk_gl_bind_opengl_wrapper(ctx, glVertex3f, 3);
	duk_gl_bind_opengl_wrapper(ctx, glVertex4d, 4);
	duk_gl_bind_opengl_wrapper(ctx, glVertex4f, 4);
	duk_gl_bind_opengl_wrapper(ctx, glColor3d, 3);
	duk_gl_bind_opengl_wrapper(ctx, glColor3f, 3);
	duk_gl_bind_opengl_wrapper(ctx, glColor4d, 4);
	duk_gl_bind_opengl_wrapper(ctx, glColor4f, 4);
	duk_gl_bind_opengl_wrapper(ctx, glNormal3d, 3);
	duk_gl_bind_opengl_wrapper(ctx, glNormal3f, 3);
	duk_gl_bind_opengl_wrapper(ctx, glTexCoord1d, 1);
	duk_gl_bind_opengl_wrapper(ctx, glTexCoord1f, 1);
	duk_gl_bind_opengl_wrapper(ctx, glTexCoord2d, 2);
	duk_gl_bind_opengl_wrapper(ctx, glTexCoord2f, 2);
	duk_gl_bind_opengl_wrapper(ctx, glTexCoord3f, 3);
	duk_gl_bind_opengl_wrapper(ctx, glTexCoord4f, 4);
	duk_gl_bind_opengl_wrapper(ctx, glEdgeFlag, 1);
	duk_gl_bind_opengl_wrapper(ctx, glMatrixMode, 1);
	duk_gl_bind_opengl_wrapper(ctx, glLoadIdentity, 0);
	duk_gl_bind_opengl_wrapper(ctx, glPushMatrix, 0);
	duk_gl_bind_opengl_wrapper(ctx, glPopMatrix, 0);
	duk_gl_bind_opengl_wrapper(ctx, glRotatef, 4);
	duk_gl_bind_opengl_wrapper(ctx, glTranslatef, 3);
	duk_gl_bind_opengl_wrapper(ctx, glScalef, 3);
	duk_gl_bind_opengl_wrapper(ctx, glViewport, 4);
	duk_gl_bind_opengl_wrapper(ctx, glFrustum, 6);
	duk_gl_bind_opengl_wrapper(ctx, glGenLists, 1);
	duk_gl_bind_opengl_wrapper(ctx, glIsList, 1);
	duk_gl_bind_opengl_wrapper(ctx, glNewList, 2);
	duk_gl_bind_opengl_wrapper(ctx, glEndList, 0);
	duk_gl_bind_opengl_wrapper(ctx, glCallList, 1);
	duk_gl_bind_opengl_wrapper(ctx, glClear, 1);
	duk_gl_bind_opengl_wrapper(ctx, glClearColor, 4);
	duk_gl_bind_opengl_wrapper(ctx, glClearDepth, 1);
	duk_gl_bind_opengl_wrapper(ctx, glRenderMode, 1);
	duk_gl_bind_opengl_wrapper(ctx, glInitNames, 0);
	duk_gl_bind_opengl_wrapper(ctx, glPushName, 1);
	duk_gl_bind_opengl_wrapper(ctx, glPopName, 0);
	duk_gl_bind_opengl_wrapper(ctx, glLoadName, 1);
	duk_gl_bind_opengl_wrapper(ctx, glBindTexture, 2);
	duk_gl_bind_opengl_wrapper(ctx, glTexEnvi, 3);
	duk_gl_bind_opengl_wrapper(ctx, glTexParameteri, 3);
	duk_gl_bind_opengl_wrapper(ctx, glPixelStorei, 2);
	duk_gl_bind_opengl_wrapper(ctx, glMaterialfv, 3);
	duk_gl_bind_opengl_wrapper(ctx, glMaterialf, 3);
	duk_gl_bind_opengl_wrapper(ctx, glColorMaterial, 2);
	duk_gl_bind_opengl_wrapper(ctx, glLightfv, 3);
	duk_gl_bind_opengl_wrapper(ctx, glLightf, 3);
	duk_gl_bind_opengl_wrapper(ctx, glLightModeli, 2);
	duk_gl_bind_opengl_wrapper(ctx, glLightModelfv, 2);
	duk_gl_bind_opengl_wrapper(ctx, glFlush, 0);
	duk_gl_bind_opengl_wrapper(ctx, glHint, 2);
	duk_gl_bind_opengl_wrapper(ctx, glFrontFace, 1);
	duk_gl_bind_opengl_wrapper(ctx, glEnableClientState, 1);
	duk_gl_bind_opengl_wrapper(ctx, glDisableClientState, 1);
	duk_gl_bind_opengl_wrapper(ctx, glArrayElement, 1);
	duk_gl_bind_opengl_wrapper(ctx, glPolygonOffset, 2);
}

/*
 *  OpenGL constants to JavaScript
 */
void duk_gl_set_constants(duk_context *ctx)
{
	duk_gl_push_opengl_constant_property(ctx, GL_FALSE);
	duk_gl_push_opengl_constant_property(ctx, GL_TRUE);
	duk_gl_push_opengl_constant_property(ctx, GL_BYTE);
	duk_gl_push_opengl_constant_property(ctx, GL_UNSIGNED_BYTE);
	duk_gl_push_opengl_constant_property(ctx, GL_SHORT);
	duk_gl_push_opengl_constant_property(ctx, GL_UNSIGNED_SHORT);
	duk_gl_push_opengl_constant_property(ctx, GL_INT);
	duk_gl_push_opengl_constant_property(ctx, GL_UNSIGNED_INT);
	duk_gl_push_opengl_constant_property(ctx, GL_FLOAT);
	duk_gl_push_opengl_constant_property(ctx, GL_DOUBLE);
	duk_gl_push_opengl_constant_property(ctx, GL_2_BYTES);
	duk_gl_push_opengl_constant_property(ctx, GL_3_BYTES);
	duk_gl_push_opengl_constant_property(ctx, GL_4_BYTES);
	duk_gl_push_opengl_constant_property(ctx, GL_LINES);
	duk_gl_push_opengl_constant_property(ctx, GL_POINTS);
	duk_gl_push_opengl_constant_property(ctx, GL_LINE_STRIP);
	duk_gl_push_opengl_constant_property(ctx, GL_LINE_LOOP);
	duk_gl_push_opengl_constant_property(ctx, GL_TRIANGLES);
	duk_gl_push_opengl_constant_property(ctx, GL_TRIANGLE_STRIP);
	duk_gl_push_opengl_constant_property(ctx, GL_TRIANGLE_FAN);
	duk_gl_push_opengl_constant_property(ctx, GL_QUADS);
	duk_gl_push_opengl_constant_property(ctx, GL_QUAD_STRIP);
	duk_gl_push_opengl_constant_property(ctx, GL_POLYGON);
	duk_gl_push_opengl_constant_property(ctx, GL_EDGE_FLAG);
	duk_gl_push_opengl_constant_property(ctx, GL_VERTEX_ARRAY);
	duk_gl_push_opengl_constant_property(ctx, GL_NORMAL_ARRAY);
	duk_gl_push_opengl_constant_property(ctx, GL_COLOR_ARRAY);
	duk_gl_push_opengl_constant_property(ctx, GL_INDEX_ARRAY);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_COORD_ARRAY);
	duk_gl_push_opengl_constant_property(ctx, GL_EDGE_FLAG_ARRAY);
	duk_gl_push_opengl_constant_property(ctx, GL_VERTEX_ARRAY_SIZE);
	duk_gl_push_opengl_constant_property(ctx, GL_VERTEX_ARRAY_TYPE);
	duk_gl_push_opengl_constant_property(ctx, GL_VERTEX_ARRAY_STRIDE);
	//duk_gl_push_opengl_constant_property(ctx, GL_VERTEX_ARRAY_COUNT);
	duk_gl_push_opengl_constant_property(ctx, GL_NORMAL_ARRAY_TYPE);
	duk_gl_push_opengl_constant_property(ctx, GL_NORMAL_ARRAY_STRIDE);
	//duk_gl_push_opengl_constant_property(ctx, GL_NORMAL_ARRAY_COUNT);
	duk_gl_push_opengl_constant_property(ctx, GL_COLOR_ARRAY_SIZE);
	duk_gl_push_opengl_constant_property(ctx, GL_COLOR_ARRAY_TYPE);
	duk_gl_push_opengl_constant_property(ctx, GL_COLOR_ARRAY_STRIDE);
	//duk_gl_push_opengl_constant_property(ctx, GL_COLOR_ARRAY_COUNT);
	duk_gl_push_opengl_constant_property(ctx, GL_INDEX_ARRAY_TYPE);
	duk_gl_push_opengl_constant_property(ctx, GL_INDEX_ARRAY_STRIDE);
	//duk_gl_push_opengl_constant_property(ctx, GL_INDEX_ARRAY_COUNT);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_COORD_ARRAY_SIZE);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_COORD_ARRAY_TYPE);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_COORD_ARRAY_STRIDE);
	//duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_COORD_ARRAY_COUNT);
	duk_gl_push_opengl_constant_property(ctx, GL_EDGE_FLAG_ARRAY_STRIDE);
	//duk_gl_push_opengl_constant_property(ctx, GL_EDGE_FLAG_ARRAY_COUNT);
	duk_gl_push_opengl_constant_property(ctx, GL_VERTEX_ARRAY_POINTER);
	duk_gl_push_opengl_constant_property(ctx, GL_NORMAL_ARRAY_POINTER);
	duk_gl_push_opengl_constant_property(ctx, GL_COLOR_ARRAY_POINTER);
	duk_gl_push_opengl_constant_property(ctx, GL_INDEX_ARRAY_POINTER);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_COORD_ARRAY_POINTER);
	duk_gl_push_opengl_constant_property(ctx, GL_EDGE_FLAG_ARRAY_POINTER);
	duk_gl_push_opengl_constant_property(ctx, GL_V2F);
	duk_gl_push_opengl_constant_property(ctx, GL_V3F);
	duk_gl_push_opengl_constant_property(ctx, GL_C4UB_V2F);
	duk_gl_push_opengl_constant_property(ctx, GL_C4UB_V3F);
	duk_gl_push_opengl_constant_property(ctx, GL_C3F_V3F);
	duk_gl_push_opengl_constant_property(ctx, GL_N3F_V3F);
	duk_gl_push_opengl_constant_property(ctx, GL_C4F_N3F_V3F);
	duk_gl_push_opengl_constant_property(ctx, GL_T2F_V3F);
	duk_gl_push_opengl_constant_property(ctx, GL_T4F_V4F);
	duk_gl_push_opengl_constant_property(ctx, GL_T2F_C4UB_V3F);
	duk_gl_push_opengl_constant_property(ctx, GL_T2F_C3F_V3F);
	duk_gl_push_opengl_constant_property(ctx, GL_T2F_N3F_V3F);
	duk_gl_push_opengl_constant_property(ctx, GL_T2F_C4F_N3F_V3F);
	duk_gl_push_opengl_constant_property(ctx, GL_T4F_C4F_N3F_V4F);
	duk_gl_push_opengl_constant_property(ctx, GL_MATRIX_MODE);
	duk_gl_push_opengl_constant_property(ctx, GL_MODELVIEW);
	duk_gl_push_opengl_constant_property(ctx, GL_PROJECTION);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE);
	duk_gl_push_opengl_constant_property(ctx, GL_POINT_SMOOTH);
	duk_gl_push_opengl_constant_property(ctx, GL_POINT_SIZE);
	duk_gl_push_opengl_constant_property(ctx, GL_POINT_SIZE_GRANULARITY);
	duk_gl_push_opengl_constant_property(ctx, GL_POINT_SIZE_RANGE);
	duk_gl_push_opengl_constant_property(ctx, GL_LINE_SMOOTH);
	duk_gl_push_opengl_constant_property(ctx, GL_LINE_STIPPLE);
	duk_gl_push_opengl_constant_property(ctx, GL_LINE_STIPPLE_PATTERN);
	duk_gl_push_opengl_constant_property(ctx, GL_LINE_STIPPLE_REPEAT);
	duk_gl_push_opengl_constant_property(ctx, GL_LINE_WIDTH);
	duk_gl_push_opengl_constant_property(ctx, GL_LINE_WIDTH_GRANULARITY);
	duk_gl_push_opengl_constant_property(ctx, GL_LINE_WIDTH_RANGE);
	duk_gl_push_opengl_constant_property(ctx, GL_POINT);
	duk_gl_push_opengl_constant_property(ctx, GL_LINE);
	duk_gl_push_opengl_constant_property(ctx, GL_FILL);
	duk_gl_push_opengl_constant_property(ctx, GL_CCW);
	duk_gl_push_opengl_constant_property(ctx, GL_CW);
	duk_gl_push_opengl_constant_property(ctx, GL_FRONT);
	duk_gl_push_opengl_constant_property(ctx, GL_BACK);
	duk_gl_push_opengl_constant_property(ctx, GL_CULL_FACE);
	duk_gl_push_opengl_constant_property(ctx, GL_CULL_FACE_MODE);
	duk_gl_push_opengl_constant_property(ctx, GL_POLYGON_SMOOTH);
	duk_gl_push_opengl_constant_property(ctx, GL_POLYGON_STIPPLE);
	duk_gl_push_opengl_constant_property(ctx, GL_FRONT_FACE);
	duk_gl_push_opengl_constant_property(ctx, GL_POLYGON_MODE);
	duk_gl_push_opengl_constant_property(ctx, GL_POLYGON_OFFSET_FACTOR);
	duk_gl_push_opengl_constant_property(ctx, GL_POLYGON_OFFSET_UNITS);
	duk_gl_push_opengl_constant_property(ctx, GL_POLYGON_OFFSET_POINT);
	duk_gl_push_opengl_constant_property(ctx, GL_POLYGON_OFFSET_LINE);
	duk_gl_push_opengl_constant_property(ctx, GL_POLYGON_OFFSET_FILL);
	duk_gl_push_opengl_constant_property(ctx, GL_COMPILE);
	duk_gl_push_opengl_constant_property(ctx, GL_COMPILE_AND_EXECUTE);
	duk_gl_push_opengl_constant_property(ctx, GL_LIST_BASE);
	duk_gl_push_opengl_constant_property(ctx, GL_LIST_INDEX);
	duk_gl_push_opengl_constant_property(ctx, GL_LIST_MODE);
	duk_gl_push_opengl_constant_property(ctx, GL_NEVER);
	duk_gl_push_opengl_constant_property(ctx, GL_LESS);
	duk_gl_push_opengl_constant_property(ctx, GL_GEQUAL);
	duk_gl_push_opengl_constant_property(ctx, GL_LEQUAL);
	duk_gl_push_opengl_constant_property(ctx, GL_GREATER);
	duk_gl_push_opengl_constant_property(ctx, GL_NOTEQUAL);
	duk_gl_push_opengl_constant_property(ctx, GL_EQUAL);
	duk_gl_push_opengl_constant_property(ctx, GL_ALWAYS);
	duk_gl_push_opengl_constant_property(ctx, GL_DEPTH_TEST);
	duk_gl_push_opengl_constant_property(ctx, GL_DEPTH_BITS);
	duk_gl_push_opengl_constant_property(ctx, GL_DEPTH_CLEAR_VALUE);
	duk_gl_push_opengl_constant_property(ctx, GL_DEPTH_FUNC);
	duk_gl_push_opengl_constant_property(ctx, GL_DEPTH_RANGE);
	duk_gl_push_opengl_constant_property(ctx, GL_DEPTH_WRITEMASK);
	duk_gl_push_opengl_constant_property(ctx, GL_DEPTH_COMPONENT);
	duk_gl_push_opengl_constant_property(ctx, GL_LIGHTING);
	duk_gl_push_opengl_constant_property(ctx, GL_LIGHT0);
	duk_gl_push_opengl_constant_property(ctx, GL_LIGHT1);
	duk_gl_push_opengl_constant_property(ctx, GL_LIGHT2);
	duk_gl_push_opengl_constant_property(ctx, GL_LIGHT3);
	duk_gl_push_opengl_constant_property(ctx, GL_LIGHT4);
	duk_gl_push_opengl_constant_property(ctx, GL_LIGHT5);
	duk_gl_push_opengl_constant_property(ctx, GL_LIGHT6);
	duk_gl_push_opengl_constant_property(ctx, GL_LIGHT7);
	duk_gl_push_opengl_constant_property(ctx, GL_SPOT_EXPONENT);
	duk_gl_push_opengl_constant_property(ctx, GL_SPOT_CUTOFF);
	duk_gl_push_opengl_constant_property(ctx, GL_CONSTANT_ATTENUATION);
	duk_gl_push_opengl_constant_property(ctx, GL_LINEAR_ATTENUATION);
	duk_gl_push_opengl_constant_property(ctx, GL_QUADRATIC_ATTENUATION);
	duk_gl_push_opengl_constant_property(ctx, GL_AMBIENT);
	duk_gl_push_opengl_constant_property(ctx, GL_DIFFUSE);
	duk_gl_push_opengl_constant_property(ctx, GL_SPECULAR);
	duk_gl_push_opengl_constant_property(ctx, GL_SHININESS);
	duk_gl_push_opengl_constant_property(ctx, GL_EMISSION);
	duk_gl_push_opengl_constant_property(ctx, GL_POSITION);
	duk_gl_push_opengl_constant_property(ctx, GL_SPOT_DIRECTION);
	duk_gl_push_opengl_constant_property(ctx, GL_AMBIENT_AND_DIFFUSE);
	duk_gl_push_opengl_constant_property(ctx, GL_COLOR_INDEXES);
	duk_gl_push_opengl_constant_property(ctx, GL_LIGHT_MODEL_TWO_SIDE);
	duk_gl_push_opengl_constant_property(ctx, GL_LIGHT_MODEL_LOCAL_VIEWER);
	duk_gl_push_opengl_constant_property(ctx, GL_LIGHT_MODEL_AMBIENT);
	duk_gl_push_opengl_constant_property(ctx, GL_FRONT_AND_BACK);
	duk_gl_push_opengl_constant_property(ctx, GL_SHADE_MODEL);
	duk_gl_push_opengl_constant_property(ctx, GL_FLAT);
	duk_gl_push_opengl_constant_property(ctx, GL_SMOOTH);
	duk_gl_push_opengl_constant_property(ctx, GL_COLOR_MATERIAL);
	duk_gl_push_opengl_constant_property(ctx, GL_COLOR_MATERIAL_FACE);
	duk_gl_push_opengl_constant_property(ctx, GL_COLOR_MATERIAL_PARAMETER);
	duk_gl_push_opengl_constant_property(ctx, GL_NORMALIZE);
	duk_gl_push_opengl_constant_property(ctx, GL_CLIP_PLANE0);
	duk_gl_push_opengl_constant_property(ctx, GL_CLIP_PLANE1);
	duk_gl_push_opengl_constant_property(ctx, GL_CLIP_PLANE2);
	duk_gl_push_opengl_constant_property(ctx, GL_CLIP_PLANE3);
	duk_gl_push_opengl_constant_property(ctx, GL_CLIP_PLANE4);
	duk_gl_push_opengl_constant_property(ctx, GL_CLIP_PLANE5);
	duk_gl_push_opengl_constant_property(ctx, GL_ACCUM_RED_BITS);
	duk_gl_push_opengl_constant_property(ctx, GL_ACCUM_GREEN_BITS);
	duk_gl_push_opengl_constant_property(ctx, GL_ACCUM_BLUE_BITS);
	duk_gl_push_opengl_constant_property(ctx, GL_ACCUM_ALPHA_BITS);
	duk_gl_push_opengl_constant_property(ctx, GL_ACCUM_CLEAR_VALUE);
	duk_gl_push_opengl_constant_property(ctx, GL_ACCUM);
	duk_gl_push_opengl_constant_property(ctx, GL_ADD);
	duk_gl_push_opengl_constant_property(ctx, GL_LOAD);
	duk_gl_push_opengl_constant_property(ctx, GL_MULT);
	duk_gl_push_opengl_constant_property(ctx, GL_RETURN);
	duk_gl_push_opengl_constant_property(ctx, GL_ALPHA_TEST);
	duk_gl_push_opengl_constant_property(ctx, GL_ALPHA_TEST_REF);
	duk_gl_push_opengl_constant_property(ctx, GL_ALPHA_TEST_FUNC);
	duk_gl_push_opengl_constant_property(ctx, GL_BLEND);
	duk_gl_push_opengl_constant_property(ctx, GL_BLEND_SRC);
	duk_gl_push_opengl_constant_property(ctx, GL_BLEND_DST);
	duk_gl_push_opengl_constant_property(ctx, GL_ZERO);
	duk_gl_push_opengl_constant_property(ctx, GL_ONE);
	duk_gl_push_opengl_constant_property(ctx, GL_SRC_COLOR);
	duk_gl_push_opengl_constant_property(ctx, GL_ONE_MINUS_SRC_COLOR);
	duk_gl_push_opengl_constant_property(ctx, GL_DST_COLOR);
	duk_gl_push_opengl_constant_property(ctx, GL_ONE_MINUS_DST_COLOR);
	duk_gl_push_opengl_constant_property(ctx, GL_SRC_ALPHA);
	duk_gl_push_opengl_constant_property(ctx, GL_ONE_MINUS_SRC_ALPHA);
	duk_gl_push_opengl_constant_property(ctx, GL_DST_ALPHA);
	duk_gl_push_opengl_constant_property(ctx, GL_ONE_MINUS_DST_ALPHA);
	duk_gl_push_opengl_constant_property(ctx, GL_SRC_ALPHA_SATURATE);
	duk_gl_push_opengl_constant_property(ctx, GL_CONSTANT_COLOR);
	duk_gl_push_opengl_constant_property(ctx, GL_ONE_MINUS_CONSTANT_COLOR);
	duk_gl_push_opengl_constant_property(ctx, GL_CONSTANT_ALPHA);
	duk_gl_push_opengl_constant_property(ctx, GL_ONE_MINUS_CONSTANT_ALPHA);
	duk_gl_push_opengl_constant_property(ctx, GL_FEEDBACK);
	duk_gl_push_opengl_constant_property(ctx, GL_RENDER);
	duk_gl_push_opengl_constant_property(ctx, GL_SELECT);
	duk_gl_push_opengl_constant_property(ctx, GL_2D);
	duk_gl_push_opengl_constant_property(ctx, GL_3D);
	duk_gl_push_opengl_constant_property(ctx, GL_3D_COLOR);
	duk_gl_push_opengl_constant_property(ctx, GL_3D_COLOR_TEXTURE);
	duk_gl_push_opengl_constant_property(ctx, GL_4D_COLOR_TEXTURE);
	duk_gl_push_opengl_constant_property(ctx, GL_POINT_TOKEN);
	duk_gl_push_opengl_constant_property(ctx, GL_LINE_TOKEN);
	duk_gl_push_opengl_constant_property(ctx, GL_LINE_RESET_TOKEN);
	duk_gl_push_opengl_constant_property(ctx, GL_POLYGON_TOKEN);
	duk_gl_push_opengl_constant_property(ctx, GL_BITMAP_TOKEN);
	duk_gl_push_opengl_constant_property(ctx, GL_DRAW_PIXEL_TOKEN);
	duk_gl_push_opengl_constant_property(ctx, GL_COPY_PIXEL_TOKEN);
	duk_gl_push_opengl_constant_property(ctx, GL_PASS_THROUGH_TOKEN);
	duk_gl_push_opengl_constant_property(ctx, GL_FOG);
	duk_gl_push_opengl_constant_property(ctx, GL_FOG_MODE);
	duk_gl_push_opengl_constant_property(ctx, GL_FOG_DENSITY);
	duk_gl_push_opengl_constant_property(ctx, GL_FOG_COLOR);
	duk_gl_push_opengl_constant_property(ctx, GL_FOG_INDEX);
	duk_gl_push_opengl_constant_property(ctx, GL_FOG_START);
	duk_gl_push_opengl_constant_property(ctx, GL_FOG_END);
	duk_gl_push_opengl_constant_property(ctx, GL_LINEAR);
	duk_gl_push_opengl_constant_property(ctx, GL_EXP);
	duk_gl_push_opengl_constant_property(ctx, GL_EXP2);
	duk_gl_push_opengl_constant_property(ctx, GL_LOGIC_OP);
	duk_gl_push_opengl_constant_property(ctx, GL_LOGIC_OP_MODE);
	duk_gl_push_opengl_constant_property(ctx, GL_CLEAR);
	duk_gl_push_opengl_constant_property(ctx, GL_SET);
	duk_gl_push_opengl_constant_property(ctx, GL_COPY);
	duk_gl_push_opengl_constant_property(ctx, GL_COPY_INVERTED);
	duk_gl_push_opengl_constant_property(ctx, GL_NOOP);
	duk_gl_push_opengl_constant_property(ctx, GL_INVERT);
	duk_gl_push_opengl_constant_property(ctx, GL_AND);
	duk_gl_push_opengl_constant_property(ctx, GL_NAND);
	duk_gl_push_opengl_constant_property(ctx, GL_OR);
	duk_gl_push_opengl_constant_property(ctx, GL_NOR);
	duk_gl_push_opengl_constant_property(ctx, GL_XOR);
	duk_gl_push_opengl_constant_property(ctx, GL_EQUIV);
	duk_gl_push_opengl_constant_property(ctx, GL_AND_REVERSE);
	duk_gl_push_opengl_constant_property(ctx, GL_AND_INVERTED);
	duk_gl_push_opengl_constant_property(ctx, GL_OR_REVERSE);
	duk_gl_push_opengl_constant_property(ctx, GL_OR_INVERTED);
	duk_gl_push_opengl_constant_property(ctx, GL_STENCIL_TEST);
	duk_gl_push_opengl_constant_property(ctx, GL_STENCIL_WRITEMASK);
	duk_gl_push_opengl_constant_property(ctx, GL_STENCIL_BITS);
	duk_gl_push_opengl_constant_property(ctx, GL_STENCIL_FUNC);
	duk_gl_push_opengl_constant_property(ctx, GL_STENCIL_VALUE_MASK);
	duk_gl_push_opengl_constant_property(ctx, GL_STENCIL_REF);
	duk_gl_push_opengl_constant_property(ctx, GL_STENCIL_FAIL);
	duk_gl_push_opengl_constant_property(ctx, GL_STENCIL_PASS_DEPTH_PASS);
	duk_gl_push_opengl_constant_property(ctx, GL_STENCIL_PASS_DEPTH_FAIL);
	duk_gl_push_opengl_constant_property(ctx, GL_STENCIL_CLEAR_VALUE);
	duk_gl_push_opengl_constant_property(ctx, GL_STENCIL_INDEX);
	duk_gl_push_opengl_constant_property(ctx, GL_KEEP);
	duk_gl_push_opengl_constant_property(ctx, GL_REPLACE);
	duk_gl_push_opengl_constant_property(ctx, GL_INCR);
	duk_gl_push_opengl_constant_property(ctx, GL_DECR);
	duk_gl_push_opengl_constant_property(ctx, GL_NONE);
	duk_gl_push_opengl_constant_property(ctx, GL_LEFT);
	duk_gl_push_opengl_constant_property(ctx, GL_RIGHT);
	duk_gl_push_opengl_constant_property(ctx, GL_FRONT_LEFT);
	duk_gl_push_opengl_constant_property(ctx, GL_FRONT_RIGHT);
	duk_gl_push_opengl_constant_property(ctx, GL_BACK_LEFT);
	duk_gl_push_opengl_constant_property(ctx, GL_BACK_RIGHT);
	duk_gl_push_opengl_constant_property(ctx, GL_AUX0);
	duk_gl_push_opengl_constant_property(ctx, GL_AUX1);
	duk_gl_push_opengl_constant_property(ctx, GL_AUX2);
	duk_gl_push_opengl_constant_property(ctx, GL_AUX3);
	duk_gl_push_opengl_constant_property(ctx, GL_COLOR_INDEX);
	duk_gl_push_opengl_constant_property(ctx, GL_RED);
	duk_gl_push_opengl_constant_property(ctx, GL_GREEN);
	duk_gl_push_opengl_constant_property(ctx, GL_BLUE);
	duk_gl_push_opengl_constant_property(ctx, GL_ALPHA);
	duk_gl_push_opengl_constant_property(ctx, GL_LUMINANCE);
	duk_gl_push_opengl_constant_property(ctx, GL_LUMINANCE_ALPHA);
	duk_gl_push_opengl_constant_property(ctx, GL_ALPHA_BITS);
	duk_gl_push_opengl_constant_property(ctx, GL_RED_BITS);
	duk_gl_push_opengl_constant_property(ctx, GL_GREEN_BITS);
	duk_gl_push_opengl_constant_property(ctx, GL_BLUE_BITS);
	duk_gl_push_opengl_constant_property(ctx, GL_INDEX_BITS);
	duk_gl_push_opengl_constant_property(ctx, GL_SUBPIXEL_BITS);
	duk_gl_push_opengl_constant_property(ctx, GL_AUX_BUFFERS);
	duk_gl_push_opengl_constant_property(ctx, GL_READ_BUFFER);
	duk_gl_push_opengl_constant_property(ctx, GL_DRAW_BUFFER);
	duk_gl_push_opengl_constant_property(ctx, GL_DOUBLEBUFFER);
	duk_gl_push_opengl_constant_property(ctx, GL_STEREO);
	duk_gl_push_opengl_constant_property(ctx, GL_BITMAP);
	duk_gl_push_opengl_constant_property(ctx, GL_COLOR);
	duk_gl_push_opengl_constant_property(ctx, GL_DEPTH);
	duk_gl_push_opengl_constant_property(ctx, GL_STENCIL);
	duk_gl_push_opengl_constant_property(ctx, GL_DITHER);
	duk_gl_push_opengl_constant_property(ctx, GL_RGB);
	duk_gl_push_opengl_constant_property(ctx, GL_RGBA);
	duk_gl_push_opengl_constant_property(ctx, GL_MAX_LIST_NESTING);
	duk_gl_push_opengl_constant_property(ctx, GL_MAX_ATTRIB_STACK_DEPTH);
	duk_gl_push_opengl_constant_property(ctx, GL_MAX_MODELVIEW_STACK_DEPTH);
	duk_gl_push_opengl_constant_property(ctx, GL_MAX_NAME_STACK_DEPTH);
	duk_gl_push_opengl_constant_property(ctx, GL_MAX_PROJECTION_STACK_DEPTH);
	duk_gl_push_opengl_constant_property(ctx, GL_MAX_TEXTURE_STACK_DEPTH);
	duk_gl_push_opengl_constant_property(ctx, GL_MAX_EVAL_ORDER);
	duk_gl_push_opengl_constant_property(ctx, GL_MAX_LIGHTS);
	duk_gl_push_opengl_constant_property(ctx, GL_MAX_CLIP_PLANES);
	duk_gl_push_opengl_constant_property(ctx, GL_MAX_TEXTURE_SIZE);
	duk_gl_push_opengl_constant_property(ctx, GL_MAX_PIXEL_MAP_TABLE);
	duk_gl_push_opengl_constant_property(ctx, GL_MAX_VIEWPORT_DIMS);
	duk_gl_push_opengl_constant_property(ctx, GL_MAX_CLIENT_ATTRIB_STACK_DEPTH);
	duk_gl_push_opengl_constant_property(ctx, GL_ATTRIB_STACK_DEPTH);
	duk_gl_push_opengl_constant_property(ctx, GL_COLOR_CLEAR_VALUE);
	duk_gl_push_opengl_constant_property(ctx, GL_COLOR_WRITEMASK);
	duk_gl_push_opengl_constant_property(ctx, GL_CURRENT_INDEX);
	duk_gl_push_opengl_constant_property(ctx, GL_CURRENT_COLOR);
	duk_gl_push_opengl_constant_property(ctx, GL_CURRENT_NORMAL);
	duk_gl_push_opengl_constant_property(ctx, GL_CURRENT_RASTER_COLOR);
	duk_gl_push_opengl_constant_property(ctx, GL_CURRENT_RASTER_DISTANCE);
	duk_gl_push_opengl_constant_property(ctx, GL_CURRENT_RASTER_INDEX);
	duk_gl_push_opengl_constant_property(ctx, GL_CURRENT_RASTER_POSITION);
	duk_gl_push_opengl_constant_property(ctx, GL_CURRENT_RASTER_TEXTURE_COORDS);
	duk_gl_push_opengl_constant_property(ctx, GL_CURRENT_RASTER_POSITION_VALID);
	duk_gl_push_opengl_constant_property(ctx, GL_CURRENT_TEXTURE_COORDS);
	duk_gl_push_opengl_constant_property(ctx, GL_INDEX_CLEAR_VALUE);
	duk_gl_push_opengl_constant_property(ctx, GL_INDEX_MODE);
	duk_gl_push_opengl_constant_property(ctx, GL_INDEX_WRITEMASK);
	duk_gl_push_opengl_constant_property(ctx, GL_MODELVIEW_MATRIX);
	duk_gl_push_opengl_constant_property(ctx, GL_MODELVIEW_STACK_DEPTH);
	duk_gl_push_opengl_constant_property(ctx, GL_NAME_STACK_DEPTH);
	duk_gl_push_opengl_constant_property(ctx, GL_PROJECTION_MATRIX);
	duk_gl_push_opengl_constant_property(ctx, GL_PROJECTION_STACK_DEPTH);
	duk_gl_push_opengl_constant_property(ctx, GL_RENDER_MODE);
	duk_gl_push_opengl_constant_property(ctx, GL_RGBA_MODE);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_MATRIX);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_STACK_DEPTH);
	duk_gl_push_opengl_constant_property(ctx, GL_VIEWPORT);
	duk_gl_push_opengl_constant_property(ctx, GL_AUTO_NORMAL);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP1_COLOR_4);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP1_GRID_DOMAIN);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP1_GRID_SEGMENTS);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP1_INDEX);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP1_NORMAL);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP1_TEXTURE_COORD_1);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP1_TEXTURE_COORD_2);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP1_TEXTURE_COORD_3);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP1_TEXTURE_COORD_4);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP1_VERTEX_3);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP1_VERTEX_4);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP2_COLOR_4);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP2_GRID_DOMAIN);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP2_GRID_SEGMENTS);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP2_INDEX);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP2_NORMAL);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP2_TEXTURE_COORD_1);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP2_TEXTURE_COORD_2);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP2_TEXTURE_COORD_3);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP2_TEXTURE_COORD_4);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP2_VERTEX_3);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP2_VERTEX_4);
	duk_gl_push_opengl_constant_property(ctx, GL_COEFF);
	duk_gl_push_opengl_constant_property(ctx, GL_DOMAIN);
	duk_gl_push_opengl_constant_property(ctx, GL_ORDER);
	duk_gl_push_opengl_constant_property(ctx, GL_FOG_HINT);
	duk_gl_push_opengl_constant_property(ctx, GL_LINE_SMOOTH_HINT);
	duk_gl_push_opengl_constant_property(ctx, GL_PERSPECTIVE_CORRECTION_HINT);
	duk_gl_push_opengl_constant_property(ctx, GL_POINT_SMOOTH_HINT);
	duk_gl_push_opengl_constant_property(ctx, GL_POLYGON_SMOOTH_HINT);
	duk_gl_push_opengl_constant_property(ctx, GL_DONT_CARE);
	duk_gl_push_opengl_constant_property(ctx, GL_FASTEST);
	duk_gl_push_opengl_constant_property(ctx, GL_NICEST);
	duk_gl_push_opengl_constant_property(ctx, GL_SCISSOR_TEST);
	duk_gl_push_opengl_constant_property(ctx, GL_SCISSOR_BOX);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP_COLOR);
	duk_gl_push_opengl_constant_property(ctx, GL_MAP_STENCIL);
	duk_gl_push_opengl_constant_property(ctx, GL_INDEX_SHIFT);
	duk_gl_push_opengl_constant_property(ctx, GL_INDEX_OFFSET);
	duk_gl_push_opengl_constant_property(ctx, GL_RED_SCALE);
	duk_gl_push_opengl_constant_property(ctx, GL_RED_BIAS);
	duk_gl_push_opengl_constant_property(ctx, GL_GREEN_SCALE);
	duk_gl_push_opengl_constant_property(ctx, GL_GREEN_BIAS);
	duk_gl_push_opengl_constant_property(ctx, GL_BLUE_SCALE);
	duk_gl_push_opengl_constant_property(ctx, GL_BLUE_BIAS);
	duk_gl_push_opengl_constant_property(ctx, GL_ALPHA_SCALE);
	duk_gl_push_opengl_constant_property(ctx, GL_ALPHA_BIAS);
	duk_gl_push_opengl_constant_property(ctx, GL_DEPTH_SCALE);
	duk_gl_push_opengl_constant_property(ctx, GL_DEPTH_BIAS);
	duk_gl_push_opengl_constant_property(ctx, GL_PIXEL_MAP_S_TO_S_SIZE);
	duk_gl_push_opengl_constant_property(ctx, GL_PIXEL_MAP_I_TO_I_SIZE);
	duk_gl_push_opengl_constant_property(ctx, GL_PIXEL_MAP_I_TO_R_SIZE);
	duk_gl_push_opengl_constant_property(ctx, GL_PIXEL_MAP_I_TO_G_SIZE);
	duk_gl_push_opengl_constant_property(ctx, GL_PIXEL_MAP_I_TO_B_SIZE);
	duk_gl_push_opengl_constant_property(ctx, GL_PIXEL_MAP_I_TO_A_SIZE);
	duk_gl_push_opengl_constant_property(ctx, GL_PIXEL_MAP_R_TO_R_SIZE);
	duk_gl_push_opengl_constant_property(ctx, GL_PIXEL_MAP_G_TO_G_SIZE);
	duk_gl_push_opengl_constant_property(ctx, GL_PIXEL_MAP_B_TO_B_SIZE);
	duk_gl_push_opengl_constant_property(ctx, GL_PIXEL_MAP_A_TO_A_SIZE);
	duk_gl_push_opengl_constant_property(ctx, GL_PIXEL_MAP_S_TO_S);
	duk_gl_push_opengl_constant_property(ctx, GL_PIXEL_MAP_I_TO_I);
	duk_gl_push_opengl_constant_property(ctx, GL_PIXEL_MAP_I_TO_R);
	duk_gl_push_opengl_constant_property(ctx, GL_PIXEL_MAP_I_TO_G);
	duk_gl_push_opengl_constant_property(ctx, GL_PIXEL_MAP_I_TO_B);
	duk_gl_push_opengl_constant_property(ctx, GL_PIXEL_MAP_I_TO_A);
	duk_gl_push_opengl_constant_property(ctx, GL_PIXEL_MAP_R_TO_R);
	duk_gl_push_opengl_constant_property(ctx, GL_PIXEL_MAP_G_TO_G);
	duk_gl_push_opengl_constant_property(ctx, GL_PIXEL_MAP_B_TO_B);
	duk_gl_push_opengl_constant_property(ctx, GL_PIXEL_MAP_A_TO_A);
	duk_gl_push_opengl_constant_property(ctx, GL_PACK_ALIGNMENT);
	duk_gl_push_opengl_constant_property(ctx, GL_PACK_LSB_FIRST);
	duk_gl_push_opengl_constant_property(ctx, GL_PACK_ROW_LENGTH);
	duk_gl_push_opengl_constant_property(ctx, GL_PACK_SKIP_PIXELS);
	duk_gl_push_opengl_constant_property(ctx, GL_PACK_SKIP_ROWS);
	duk_gl_push_opengl_constant_property(ctx, GL_PACK_SWAP_BYTES);
	duk_gl_push_opengl_constant_property(ctx, GL_UNPACK_ALIGNMENT);
	duk_gl_push_opengl_constant_property(ctx, GL_UNPACK_LSB_FIRST);
	duk_gl_push_opengl_constant_property(ctx, GL_UNPACK_ROW_LENGTH);
	duk_gl_push_opengl_constant_property(ctx, GL_UNPACK_SKIP_PIXELS);
	duk_gl_push_opengl_constant_property(ctx, GL_UNPACK_SKIP_ROWS);
	duk_gl_push_opengl_constant_property(ctx, GL_UNPACK_SWAP_BYTES);
	duk_gl_push_opengl_constant_property(ctx, GL_ZOOM_X);
	duk_gl_push_opengl_constant_property(ctx, GL_ZOOM_Y);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_ENV);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_ENV_MODE);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_1D);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_2D);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_WRAP_S);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_WRAP_T);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_MAG_FILTER);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_MIN_FILTER);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_ENV_COLOR);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_GEN_S);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_GEN_T);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_GEN_MODE);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_BORDER_COLOR);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_WIDTH);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_HEIGHT);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_BORDER);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_COMPONENTS);
	duk_gl_push_opengl_constant_property(ctx, GL_NEAREST_MIPMAP_NEAREST);
	duk_gl_push_opengl_constant_property(ctx, GL_NEAREST_MIPMAP_LINEAR);
	duk_gl_push_opengl_constant_property(ctx, GL_LINEAR_MIPMAP_NEAREST);
	duk_gl_push_opengl_constant_property(ctx, GL_LINEAR_MIPMAP_LINEAR);
	duk_gl_push_opengl_constant_property(ctx, GL_OBJECT_LINEAR);
	duk_gl_push_opengl_constant_property(ctx, GL_OBJECT_PLANE);
	duk_gl_push_opengl_constant_property(ctx, GL_EYE_LINEAR);
	duk_gl_push_opengl_constant_property(ctx, GL_EYE_PLANE);
	duk_gl_push_opengl_constant_property(ctx, GL_SPHERE_MAP);
	duk_gl_push_opengl_constant_property(ctx, GL_DECAL);
	duk_gl_push_opengl_constant_property(ctx, GL_MODULATE);
	duk_gl_push_opengl_constant_property(ctx, GL_NEAREST);
	duk_gl_push_opengl_constant_property(ctx, GL_REPEAT);
	duk_gl_push_opengl_constant_property(ctx, GL_CLAMP);
	duk_gl_push_opengl_constant_property(ctx, GL_S);
	duk_gl_push_opengl_constant_property(ctx, GL_T);
	duk_gl_push_opengl_constant_property(ctx, GL_R);
	duk_gl_push_opengl_constant_property(ctx, GL_Q);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_GEN_R);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_GEN_Q);
	duk_gl_push_opengl_constant_property(ctx, GL_PROXY_TEXTURE_1D);
	duk_gl_push_opengl_constant_property(ctx, GL_PROXY_TEXTURE_2D);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_PRIORITY);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_RESIDENT);
	//duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_1D_BINDING);
	//duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_2D_BINDING);
	duk_gl_push_opengl_constant_property(ctx, GL_ALPHA4);
	duk_gl_push_opengl_constant_property(ctx, GL_ALPHA8);
	duk_gl_push_opengl_constant_property(ctx, GL_ALPHA12);
	duk_gl_push_opengl_constant_property(ctx, GL_ALPHA16);
	duk_gl_push_opengl_constant_property(ctx, GL_LUMINANCE4);
	duk_gl_push_opengl_constant_property(ctx, GL_LUMINANCE8);
	duk_gl_push_opengl_constant_property(ctx, GL_LUMINANCE12);
	duk_gl_push_opengl_constant_property(ctx, GL_LUMINANCE16);
	duk_gl_push_opengl_constant_property(ctx, GL_LUMINANCE4_ALPHA4);
	duk_gl_push_opengl_constant_property(ctx, GL_LUMINANCE6_ALPHA2);
	duk_gl_push_opengl_constant_property(ctx, GL_LUMINANCE8_ALPHA8);
	duk_gl_push_opengl_constant_property(ctx, GL_LUMINANCE12_ALPHA4);
	duk_gl_push_opengl_constant_property(ctx, GL_LUMINANCE12_ALPHA12);
	duk_gl_push_opengl_constant_property(ctx, GL_LUMINANCE16_ALPHA16);
	duk_gl_push_opengl_constant_property(ctx, GL_INTENSITY);
	duk_gl_push_opengl_constant_property(ctx, GL_INTENSITY4);
	duk_gl_push_opengl_constant_property(ctx, GL_INTENSITY8);
	duk_gl_push_opengl_constant_property(ctx, GL_INTENSITY12);
	duk_gl_push_opengl_constant_property(ctx, GL_INTENSITY16);
	duk_gl_push_opengl_constant_property(ctx, GL_R3_G3_B2);
	duk_gl_push_opengl_constant_property(ctx, GL_RGB4);
	duk_gl_push_opengl_constant_property(ctx, GL_RGB5);
	duk_gl_push_opengl_constant_property(ctx, GL_RGB8);
	duk_gl_push_opengl_constant_property(ctx, GL_RGB10);
	duk_gl_push_opengl_constant_property(ctx, GL_RGB12);
	duk_gl_push_opengl_constant_property(ctx, GL_RGB16);
	duk_gl_push_opengl_constant_property(ctx, GL_RGBA2);
	duk_gl_push_opengl_constant_property(ctx, GL_RGBA4);
	duk_gl_push_opengl_constant_property(ctx, GL_RGB5_A1);
	duk_gl_push_opengl_constant_property(ctx, GL_RGBA8);
	duk_gl_push_opengl_constant_property(ctx, GL_RGB10_A2);
	duk_gl_push_opengl_constant_property(ctx, GL_RGBA12);
	duk_gl_push_opengl_constant_property(ctx, GL_RGBA16);
	duk_gl_push_opengl_constant_property(ctx, GL_VENDOR);
	duk_gl_push_opengl_constant_property(ctx, GL_RENDERER);
	duk_gl_push_opengl_constant_property(ctx, GL_VERSION);
	duk_gl_push_opengl_constant_property(ctx, GL_EXTENSIONS);
	duk_gl_push_opengl_constant_property(ctx, GL_INVALID_VALUE);
	duk_gl_push_opengl_constant_property(ctx, GL_INVALID_ENUM);
	duk_gl_push_opengl_constant_property(ctx, GL_INVALID_OPERATION);
	duk_gl_push_opengl_constant_property(ctx, GL_STACK_OVERFLOW);
	duk_gl_push_opengl_constant_property(ctx, GL_STACK_UNDERFLOW);
	duk_gl_push_opengl_constant_property(ctx, GL_OUT_OF_MEMORY);
	duk_gl_push_opengl_constant_property(ctx, GL_CONSTANT_COLOR_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_ONE_MINUS_CONSTANT_COLOR_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_CONSTANT_ALPHA_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_ONE_MINUS_CONSTANT_ALPHA_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_BLEND_EQUATION_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_MIN_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_MAX_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_FUNC_ADD_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_FUNC_SUBTRACT_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_FUNC_REVERSE_SUBTRACT_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_BLEND_COLOR_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_POLYGON_OFFSET_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_POLYGON_OFFSET_FACTOR_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_POLYGON_OFFSET_BIAS_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_VERTEX_ARRAY_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_NORMAL_ARRAY_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_COLOR_ARRAY_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_INDEX_ARRAY_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_COORD_ARRAY_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_EDGE_FLAG_ARRAY_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_VERTEX_ARRAY_SIZE_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_VERTEX_ARRAY_TYPE_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_VERTEX_ARRAY_STRIDE_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_VERTEX_ARRAY_COUNT_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_NORMAL_ARRAY_TYPE_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_NORMAL_ARRAY_STRIDE_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_NORMAL_ARRAY_COUNT_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_COLOR_ARRAY_SIZE_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_COLOR_ARRAY_TYPE_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_COLOR_ARRAY_STRIDE_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_COLOR_ARRAY_COUNT_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_INDEX_ARRAY_TYPE_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_INDEX_ARRAY_STRIDE_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_INDEX_ARRAY_COUNT_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_COORD_ARRAY_SIZE_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_COORD_ARRAY_TYPE_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_COORD_ARRAY_STRIDE_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_COORD_ARRAY_COUNT_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_EDGE_FLAG_ARRAY_STRIDE_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_EDGE_FLAG_ARRAY_COUNT_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_VERTEX_ARRAY_POINTER_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_NORMAL_ARRAY_POINTER_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_COLOR_ARRAY_POINTER_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_INDEX_ARRAY_POINTER_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_COORD_ARRAY_POINTER_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_EDGE_FLAG_ARRAY_POINTER_EXT);
	duk_gl_push_opengl_constant_property(ctx, GL_CURRENT_BIT);
	duk_gl_push_opengl_constant_property(ctx, GL_POINT_BIT);
	duk_gl_push_opengl_constant_property(ctx, GL_LINE_BIT);
	duk_gl_push_opengl_constant_property(ctx, GL_POLYGON_BIT);
	duk_gl_push_opengl_constant_property(ctx, GL_POLYGON_STIPPLE_BIT);
	duk_gl_push_opengl_constant_property(ctx, GL_PIXEL_MODE_BIT);
	duk_gl_push_opengl_constant_property(ctx, GL_LIGHTING_BIT);
	duk_gl_push_opengl_constant_property(ctx, GL_FOG_BIT);
	duk_gl_push_opengl_constant_property(ctx, GL_DEPTH_BUFFER_BIT);
	duk_gl_push_opengl_constant_property(ctx, GL_ACCUM_BUFFER_BIT);
	duk_gl_push_opengl_constant_property(ctx, GL_STENCIL_BUFFER_BIT);
	duk_gl_push_opengl_constant_property(ctx, GL_VIEWPORT_BIT);
	duk_gl_push_opengl_constant_property(ctx, GL_TRANSFORM_BIT);
	duk_gl_push_opengl_constant_property(ctx, GL_ENABLE_BIT);
	duk_gl_push_opengl_constant_property(ctx, GL_COLOR_BUFFER_BIT);
	duk_gl_push_opengl_constant_property(ctx, GL_HINT_BIT);
	duk_gl_push_opengl_constant_property(ctx, GL_EVAL_BIT);
	duk_gl_push_opengl_constant_property(ctx, GL_LIST_BIT);
	duk_gl_push_opengl_constant_property(ctx, GL_TEXTURE_BIT);
	duk_gl_push_opengl_constant_property(ctx, GL_SCISSOR_BIT);
	duk_gl_push_opengl_constant_property(ctx, GL_ALL_ATTRIB_BITS);
}

/*
 *  Push OpenGL bindings to JavaScript global object
 */
void duk_gl_push_opengl_bindings(duk_context *ctx)
{
	duk_push_global_object(ctx);
	duk_gl_bind_opengl_functions(ctx);
	duk_gl_set_constants(ctx);
	duk_pop(ctx);
}
