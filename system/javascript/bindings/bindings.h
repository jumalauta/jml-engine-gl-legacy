#ifndef EXH_SYSTEM_JAVASCRIPT_BINDINGS_BINDINGS_H_
#define EXH_SYSTEM_JAVASCRIPT_BINDINGS_BINDINGS_H_

#define bindCFunctionToJs(cFunction, argumentCount) \
  duk_push_c_function(ctx, duk_##cFunction, argumentCount); \
  duk_put_prop_string(ctx, -2, #cFunction)

#define duk_gl_bind_opengl_wrapper(ctx, c_function_name, argument_count) \
	duk_push_c_function((ctx), duk_gl_##c_function_name, (argument_count)); \
	duk_put_prop_string((ctx), -2, #c_function_name)

#define duk_gl_push_opengl_constant_property(ctx, opengl_constant) \
	duk_push_uint((ctx), (opengl_constant)); \
	duk_put_prop_string((ctx), -2, #opengl_constant)

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

extern void bindJsMiscellaneousFunctions(duk_context *ctx);
extern void bindJsOpenGlFunctions(duk_context *ctx);
extern void bindJsAntTweakBarFunctions(duk_context *ctx);
extern void bindJsAudioFunctions(duk_context *ctx);
extern void bindJsGraphicsFunctions(duk_context *ctx);
extern void bindJsUiFunctions(duk_context *ctx);
extern void bindJsPlayerFunctions(duk_context *ctx);
extern void bindJsTimerFunctions(duk_context *ctx);
extern void bindJsSyncEditorFunctions(duk_context *ctx);
extern void bindJsCustomFunctions(duk_context *ctx);

#endif /*EXH_SYSTEM_JAVASCRIPT_BINDINGS_BINDINGS_H_*/
