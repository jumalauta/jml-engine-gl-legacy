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
#include "system/player/player.h"

#include "bindings.h"

#ifdef ANTTWEAKBAR
#include <AntTweakBar/AntTweakBar.h>

typedef struct {
	char* sceneName[512];
	char* variableName[512];
	char* definition[512];
	int type;
} jsTwVariable_t;

static void TW_CALL jsTwSetCallback(const void *value, void *clientData)
{
	jsTwVariable_t *var = (jsTwVariable_t*)clientData;
	char jsEvalString[512];

	switch(var->type)
	{
		case TW_TYPE_FLOAT:
			sprintf(jsEvalString, "sceneVariables['%s'].%s.value = %f", (const char*)var->sceneName, (const char*)var->variableName, *(float*)value);
			duk_eval_string_noresult(ctx, jsEvalString);
			break;
		case TW_TYPE_DOUBLE:
			sprintf(jsEvalString, "sceneVariables['%s'].%s.value = %f", (const char*)var->sceneName, (const char*)var->variableName, *(double*)value);
			duk_eval_string_noresult(ctx, jsEvalString);
			break;
		case TW_TYPE_BOOL32:
			sprintf(jsEvalString, "sceneVariables['%s'].%s.value = %s", (const char*)var->sceneName, (const char*)var->variableName, *(int*)value==1?"true":"false");
			duk_eval_string_noresult(ctx, jsEvalString);
			break;
		case TW_TYPE_INT32:
			sprintf(jsEvalString, "sceneVariables['%s'].%s.value = %d", (const char*)var->sceneName, (const char*)var->variableName, *(int*)value);
			duk_eval_string_noresult(ctx, jsEvalString);
			break;
		case TW_TYPE_UINT32:
			sprintf(jsEvalString, "sceneVariables['%s'].%s.value = %d", (const char*)var->sceneName, (const char*)var->variableName, *(unsigned int*)value);
			duk_eval_string_noresult(ctx, jsEvalString);
			break;
		case TW_TYPE_COLOR4F:
			sprintf(jsEvalString, "sceneVariables['%s'].%s.a = %f", (const char*)var->sceneName, (const char*)var->variableName, ((float*)value)[3]);
			duk_eval_string_noresult(ctx, jsEvalString);
			//fall through to TW_TYPE_COLOR3F
		case TW_TYPE_COLOR3F:
			sprintf(jsEvalString, "sceneVariables['%s'].%s.r = %f", (const char*)var->sceneName, (const char*)var->variableName, ((float*)value)[0]);
			duk_eval_string_noresult(ctx, jsEvalString);
			sprintf(jsEvalString, "sceneVariables['%s'].%s.g = %f", (const char*)var->sceneName, (const char*)var->variableName, ((float*)value)[1]);
			duk_eval_string_noresult(ctx, jsEvalString);
			sprintf(jsEvalString, "sceneVariables['%s'].%s.b = %f", (const char*)var->sceneName, (const char*)var->variableName, ((float*)value)[2]);
			duk_eval_string_noresult(ctx, jsEvalString);
			break;
		default:
			debugPrintf("Variable '%s' type '%d' not supported!", (const char*)var->variableName, var->type);
	}
}


// Callback function called by the main tweak bar to get the 'Subdiv' value
static void TW_CALL jsTwGetCallback(void *value, void *clientData)
{
	jsTwVariable_t *var = (jsTwVariable_t*)clientData;
	char jsEvalString[512];

	switch(var->type)
	{
		case TW_TYPE_FLOAT:
			sprintf(jsEvalString, "sceneVariables['%s'].%s.value", (const char*)var->sceneName, (const char*)var->variableName);
			duk_eval_string(ctx, jsEvalString);
			*(float*)value = (float)duk_get_uint(ctx, -1);
			duk_pop(ctx);
			break;
		case TW_TYPE_DOUBLE:
			sprintf(jsEvalString, "sceneVariables['%s'].%s.value", (const char*)var->sceneName, (const char*)var->variableName);
			duk_eval_string(ctx, jsEvalString);
			*(double*)value = (double)duk_get_uint(ctx, -1);
			duk_pop(ctx);
			break;
		case TW_TYPE_INT32:
			sprintf(jsEvalString, "sceneVariables['%s'].%s.value", (const char*)var->sceneName, (const char*)var->variableName);
			duk_eval_string(ctx, jsEvalString);
			*(int*)value = (int)duk_get_uint(ctx, -1);
			duk_pop(ctx);
			break;
		case TW_TYPE_UINT32:
			sprintf(jsEvalString, "sceneVariables['%s'].%s.value", (const char*)var->sceneName, (const char*)var->variableName);
			duk_eval_string(ctx, jsEvalString);
			*(unsigned int*)value = (unsigned int)duk_get_uint(ctx, -1);
			duk_pop(ctx);
			break;
		case TW_TYPE_COLOR4F:
			sprintf(jsEvalString, "sceneVariables['%s'].%s.a", (const char*)var->sceneName, (const char*)var->variableName);
			duk_eval_string(ctx, jsEvalString);
			((float*)value)[3] = (float)duk_get_number(ctx, -1);
			duk_pop(ctx);
			//fall through to TW_TYPE_COLOR3F
		case TW_TYPE_COLOR3F:
			sprintf(jsEvalString, "sceneVariables['%s'].%s.r", (const char*)var->sceneName, (const char*)var->variableName);
			duk_eval_string(ctx, jsEvalString);
			((float*)value)[0] = (float)duk_get_number(ctx, -1);
			duk_pop(ctx);
			sprintf(jsEvalString, "sceneVariables['%s'].%s.g", (const char*)var->sceneName, (const char*)var->variableName);
			duk_eval_string(ctx, jsEvalString);
			((float*)value)[1] = (float)duk_get_number(ctx, -1);
			duk_pop(ctx);
			sprintf(jsEvalString, "sceneVariables['%s'].%s.b", (const char*)var->sceneName, (const char*)var->variableName);
			duk_eval_string(ctx, jsEvalString);
			((float*)value)[2] = (float)duk_get_number(ctx, -1);
			duk_pop(ctx);
			break;
		default:
			debugPrintf("Variable '%s' type '%d' not supported!", (const char*)var->variableName, var->type);
	}
}
#endif

//addTwVariable(scene.name, variableName, variable.type, variable.definition);
static int duk_addTwVariable(duk_context *ctx)
{
	const char* sceneName = (const char*)duk_get_string(ctx, 0);
	const char* variableName = (const char*)duk_get_string(ctx, 1);
	const char* variableType = (const char*)duk_get_string(ctx, 2);
	const char* variableDefinition = (const char*)duk_get_string(ctx, 3);

	if (!isPlayerEditor())
	{
		return 0;
	}

#ifdef ANTTWEAKBAR
	//duk_push_uint(ctx, getUniformLocation(variable));
	char definition[512];
	sprintf(definition, " group='%s' %s", sceneName, variableDefinition);
	
	//http://anttweakbar.sourceforge.net/doc/tools:anttweakbar:twtype
	int type = TW_TYPE_FLOAT;
	if (!strcmp(variableType,"float"))
	{
		type = TW_TYPE_FLOAT;
	}
	else if (!strcmp(variableType,"double"))
	{
		type = TW_TYPE_DOUBLE;
	}
	else if (!strcmp(variableType,"int"))
	{
		type = TW_TYPE_INT32;
	}
	else if (!strcmp(variableType,"uint"))
	{
		type = TW_TYPE_UINT32;
	}
	else if (!strcmp(variableType,"bool"))
	{
		type = TW_TYPE_BOOL32;
	}
	else if (!strcmp(variableType,"color3f"))
	{
		type = TW_TYPE_COLOR3F;
	}
	else if (!strcmp(variableType,"color4f"))
	{
		type = TW_TYPE_COLOR4F;
	}

	jsTwVariable_t *var = (jsTwVariable_t*)memoryAddGeneral((void*)var, sizeof(jsTwVariable_t), NULL);
	sprintf((char*)var->sceneName,"%s",sceneName);
	sprintf((char*)var->variableName,"%s",variableName);
	sprintf((char*)var->definition,"%s",definition);
	var->type = type;
	
	char uniqueVariableName[512];
	sprintf((char*)uniqueVariableName, "%s (%s)", (const char*)var->variableName, (const char*)var->sceneName);

	TwAddVarCB(TwGetBarByName("Settings"), uniqueVariableName, type, jsTwSetCallback, jsTwGetCallback, (void*)var, definition);
#else
	debugPrintf("Tweak variable adding not supported! scene:'%s', variable:'%s', type:'%s', definition:'%s'",sceneName,variableName,variableType,variableDefinition);
#endif

	return 0;
}

void bindJsAntTweakBarFunctions(duk_context *ctx)
{
	bindCFunctionToJs(addTwVariable, 4);
}
