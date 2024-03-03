/* Mathematical expression parser
 Usage: expr [--function-list] [--constant-list] [--trace] [--help] [--usage] <statement>
 f.e. expr "-1+2+3*-10+-sqrt(32*-M_PI*-1*sin(M_PI*fmod(M_PI,1.3)))"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "general.h"


//2012/02/19: GCC MinGW doesn't support some C99 functions out-of-box
//#define EXPR_C99_SUPPORT

#define printf debugPrintf

static int isTokenTrace = 0;
static int isCalcTrace  = 0;
#define tokenParseDebugPrintf(...) do { if (isTokenTrace) { debugPrintf(__VA_ARGS__); } } while(0)
#define calcParseDebugPrintf(...)  do { if (isCalcTrace)  { debugPrintf(__VA_ARGS__); } } while(0)

static const char *skipCharacters           = " \t\n\r";
static const char *digitCharacters          = "0123456789.";
static const char *exprCharacters           = "+-/*(),";
//static const char *bracketCharacters        = "()";
static const char *signValueValidCharacters = "+-/*(,";

#define FUNCTION_BASE 100
enum Operator
{
	UNKNOWN = 0,
	VALUE,
	VARIABLE,
	BRACKET_OPEN = 10,
	BRACKET_CLOSE,
	COMMA,
	SUM,
	SUB,
	DIVIDE,
	MULTIPLY,
	REMAINDER,
	//start function operators from value FUNCTION_BASE
	STD_FUNC_SQRT = FUNCTION_BASE,
	STD_FUNC_SINF,
	STD_FUNC_SIN,
	STD_FUNC_COSF,
	STD_FUNC_COS,
	STD_FUNC_TANF,
	STD_FUNC_TAN,
	STD_FUNC_POWF,
	STD_FUNC_POW,
	STD_FUNC_ACOS,
	STD_FUNC_ASIN,
	STD_FUNC_ATAN,
	STD_FUNC_ATAN2,
	STD_FUNC_COSH,
	STD_FUNC_SINH,
	STD_FUNC_TANH,
	STD_FUNC_LOG,
	STD_FUNC_LOG10,
	STD_FUNC_HYPOT,
	STD_FUNC_FMOD,
	STD_FUNC_FABS,
	STD_FUNC_FLOOR,
	STD_FUNC_CEIL,
	STD_FUNC_EXP,
	STD_FUNC_EXPL,
	STD_FUNC_EXPF,
	STD_FUNC_EXP2,
	STD_FUNC_EXP2L,
	STD_FUNC_EXP2F
#ifdef EXPR_C99_SUPPORT
	,STD_FUNC_EXPM1
	,STD_FUNC_EXPM1L
	,STD_FUNC_EXPM1F
#endif
};

#ifdef EXPR_C99_SUPPORT
#define MAX_FUNCTIONS 32
#else
#define MAX_FUNCTIONS 32-3
#endif

static exprFunction_t functionList[MAX_FUNCTIONS] = {
	{STD_FUNC_SQRT   , "sqrt"   , 1, "square root"},
	{STD_FUNC_SINF   , "sinf"   , 1, "sine"},
	{STD_FUNC_SIN    , "sin"    , 1, "sine"},
	{STD_FUNC_COSF   , "cosf"   , 1, "cosine"},
	{STD_FUNC_COS    , "cos"    , 1, "cosine"},
	{STD_FUNC_TANF   , "tanf"   , 1, "tangent"},
	{STD_FUNC_TAN    , "tan"    , 1, "tangent"},
	{STD_FUNC_POWF   , "powf"   , 2, "raise x to the power of y, x^y"},
	{STD_FUNC_POW    , "pow"    , 2, "raise x to the power of y, x^y"},
	{STD_FUNC_ACOS   , "acos"   , 1, "arc cosine"},
	{STD_FUNC_ASIN   , "asin"   , 1, "arc sine"},
	{STD_FUNC_ATAN   , "atan"   , 1, "one-parameter arc tangent"},
	{STD_FUNC_ATAN2  , "atan2"  , 2, "two-parameter arc tangent"},
	{STD_FUNC_COSH   , "cosh"   , 1, "hyperbolic cosine"},
	{STD_FUNC_SINH   , "sinh"   , 1, "hyperbolic sine"},
	{STD_FUNC_TANH   , "tanh"   , 1, "hyperbolic tangent"},
	{STD_FUNC_LOG    , "log"    , 1, "natural logarithm"},
	{STD_FUNC_LOG10  , "log10"  , 1, "base-10 logarithm"},
	{STD_FUNC_HYPOT  , "hypot"  , 2, "hypotenuse, sqrt(x^2 + y^2)"},
	{STD_FUNC_FMOD   , "fmod"   , 2, "floating-point remainder (modulus)"},
	{STD_FUNC_FABS   , "fabs"   , 1, "absolute value of a floating-point number"},
	{STD_FUNC_FLOOR  , "floor"  , 1, "the largest integer not greater than parameter"},
	{STD_FUNC_CEIL   , "ceil"   , 1, "the smallest integer not less than parameter"},
	{STD_FUNC_EXP    , "exp"    , 1, "exponential function"},
	{STD_FUNC_EXPL   , "expl"   , 1, "exponential function"},
	{STD_FUNC_EXPF   , "expf"   , 1, "exponential function"},
	{STD_FUNC_EXP2   , "exp2"   , 1, "raise 2 to the power of x, 2^x"},
	{STD_FUNC_EXP2L  , "exp2l"  , 1, "raise 2 to the power of x, 2^x"},
	{STD_FUNC_EXP2F  , "exp2f"  , 1, "raise 2 to the power of x, 2^x"}
#ifdef EXPR_C99_SUPPORT
	,{STD_FUNC_EXPM1  , "expm1"  , 1, "one less than the exponential of x, e^x âˆ’ 1"}
	,{STD_FUNC_EXPM1L , "expm1l" , 1, "one less than the exponential of x, e^x âˆ’ 1"}
	,{STD_FUNC_EXPM1F , "expm1f" , 1, "one less than the exponential of x, e^x âˆ’ 1"}
#endif
};

static double getOperatorDoubleValue(exprCalculation_t *calculation, exprOperator_t* op);
static int isFunction(int type);
static void deleteExprOperator(exprCalculation_t *calculation, exprOperator_t *op);
static void setOperator(exprOperator_t *op, int type, double value);

static exprFunction_t *getFunctionInfo(int type)
{
	int i;
	for (i=0; i<MAX_FUNCTIONS; i++)
	{
		if (type == functionList[i].type)
		{
			return &functionList[i];
		}
	}
		
	return NULL;
}

static int isValueTypeValid(int type)
{
	
	if (VALUE == type || VARIABLE == type || isFunction(type))
	{
		return 1;
	}

	return 0;
}

static int getOperatorCharacterType(char character)
{
	switch (character)
	{
		case '+':
			return SUM;
		case '-':
			return SUB;
		case '*':
			return MULTIPLY;
		case '/':
			return DIVIDE;
		case '(':
			return BRACKET_OPEN;
		case ')':
			return BRACKET_CLOSE;
		case '%':
			return REMAINDER;
		case ',':
			return COMMA;
		default:
			return UNKNOWN;
	}
}

//public
exprVariable_t* exprAddVariable(exprCalculation_t *calculation, const char *name, void* valuePointer)
{
	exprVariable_t *var;
	var = (exprVariable_t*)malloc(sizeof(exprVariable_t));
	
	var->prev = NULL;
	var->next = NULL;

	var->valuePointer = valuePointer;
	var->name = strdup(name);
	
	if (calculation->variableHead == NULL)
	{
		calculation->variableHead = var;
		calculation->variableTail = var;
	}
	else
	{
		var->prev                       = (struct exprVariable_t*)calculation->variableTail;
		calculation->variableTail->next = (struct exprVariable_t*)var;
		calculation->variableTail       = var;
	}
	
	return var;
}

static exprVariable_t* getVariable(exprCalculation_t *calculation, const char *name)
{
	if ('\0' == name[0])
	{
		return NULL;
	}
	
	exprVariable_t* variableCurrent    = calculation->variableHead;
	while(variableCurrent != NULL)
	{
		if (variableCurrent->name && !strcmp(variableCurrent->name, name))
		{
			return variableCurrent;
		}

		variableCurrent    = (exprVariable_t*)variableCurrent->next;
	}
	
	return NULL;
}

static exprOperator_t* addOperator(exprCalculation_t *calculation)
{
	exprOperator_t *op;
	op = (exprOperator_t*)malloc(sizeof(exprOperator_t));
	
	op->prev       = NULL;
	op->next       = NULL;
	op->type       = UNKNOWN;
	op->value      = 0.0;
	op->actual     = NULL;
	op->variable   = NULL;
	op->isLocked   = 0;
	op->isNegative = 0;
	
	if (calculation->operatorHead == NULL)
	{
		calculation->operatorHead = op;
		calculation->operatorTail = op;
	}
	else
	{
		op->prev                        = (struct exprOperator_t*)calculation->operatorTail;
		calculation->operatorTail->next = (struct exprOperator_t*)op;
		calculation->operatorTail       = op;
	}
	
	return op;
}

static double getFunctionDoubleValue(exprCalculation_t *calculation, exprOperator_t* op)
{
	int functionType = op->type;
	exprFunction_t* function = getFunctionInfo(functionType);

	double doubleParamValues[3];
	
	exprOperator_t *functionBracketOpen   = NULL;
	exprOperator_t *functionBracketClosed = NULL;
	
	calcParseDebugPrintf("%s: ",op->actual);

	exprOperator_t *paramOperator = ((exprOperator_t*)op->next);
	if (paramOperator->type != BRACKET_OPEN)
	{
		calcParseDebugPrintf("\n");
		printf("Parse error! Function '%s' missing opening bracket from function start!\n", op->actual);
		return 0.0f;
	}
	else
	{
		functionBracketOpen = paramOperator;
	}

	int i;
	for(i=0; i < function->params; i++)
	{
		paramOperator = ((exprOperator_t*)paramOperator->next);
		if (paramOperator->type == COMMA)
		{
			paramOperator = ((exprOperator_t*)paramOperator->next);
			calcParseDebugPrintf(", ");
		}
		
		if (paramOperator->type == BRACKET_CLOSE)
		{
			calcParseDebugPrintf("\n");
			printf("Error parsing function '%s', not enough params given!\n",op->actual);
			return 0.0f;
		}

		calcParseDebugPrintf("%s",paramOperator->actual);
		doubleParamValues[i] = getOperatorDoubleValue(calculation, paramOperator);
	}

	paramOperator = ((exprOperator_t*)paramOperator->next);
	if (paramOperator->type != BRACKET_CLOSE)
	{
		calcParseDebugPrintf("\n");

		if (paramOperator->type == COMMA)
		{
			printf("Parse error! Function '%s' has too many parameters. Function should have %d parameters!\n", op->actual, function->params);
		}
		else
		{
			printf("Parse error! Function '%s' requires closing bracket to end of the function!\n", op->actual);
		}
		
		return 0.0f;
	}
	else
	{
		functionBracketClosed = paramOperator;
	}
	
	double resultValue = 0.0;
	switch(functionType)
	{
		case STD_FUNC_SQRT:
			resultValue = sqrt(doubleParamValues[0]);
			break;
		case STD_FUNC_SINF:
		case STD_FUNC_SIN:
			resultValue = sin(doubleParamValues[0]);
			break;
		case STD_FUNC_COSF:
		case STD_FUNC_COS:
			resultValue = cos(doubleParamValues[0]);
			break;
		case STD_FUNC_TANF:
		case STD_FUNC_TAN:
			resultValue = tan(doubleParamValues[0]);
			break;
		case STD_FUNC_POWF:
		case STD_FUNC_POW:
			resultValue = pow(doubleParamValues[0],doubleParamValues[1]);
			break;
		case STD_FUNC_ACOS:
			resultValue = acos(doubleParamValues[0]);
			break;
		case STD_FUNC_ASIN:
			resultValue = asin(doubleParamValues[0]);
			break;
		case STD_FUNC_ATAN:
			resultValue = atan(doubleParamValues[0]);
			break;
		case STD_FUNC_ATAN2:
			resultValue = atan2(doubleParamValues[0],doubleParamValues[1]);
			break;
		case STD_FUNC_COSH:
			resultValue = cosh(doubleParamValues[0]);
			break;
		case STD_FUNC_SINH:
			resultValue = sinh(doubleParamValues[0]);
			break;
		case STD_FUNC_TANH:
			resultValue = tanh(doubleParamValues[0]);
			break;
		case STD_FUNC_LOG:
			resultValue = log(doubleParamValues[0]);
			break;
		case STD_FUNC_LOG10:
			resultValue = log10(doubleParamValues[0]);
			break;
		case STD_FUNC_HYPOT:
			resultValue = hypot(doubleParamValues[0],doubleParamValues[1]);
			break;			
		case STD_FUNC_FMOD:
			resultValue = fmod(doubleParamValues[0],doubleParamValues[1]);
			break;
		case STD_FUNC_FABS:
			resultValue = fabs(doubleParamValues[0]);
			break;
		case STD_FUNC_FLOOR:
			resultValue = floor(doubleParamValues[0]);
			break;
		case STD_FUNC_CEIL:
			resultValue = ceil(doubleParamValues[0]);
			break;
		case STD_FUNC_EXP:
			resultValue = exp(doubleParamValues[0]);
			break;
		case STD_FUNC_EXPL:
			resultValue = expl(doubleParamValues[0]);
			break;
		case STD_FUNC_EXPF:
			resultValue = expf(doubleParamValues[0]);
			break;
		case STD_FUNC_EXP2:
			resultValue = exp2(doubleParamValues[0]);
			break;
		case STD_FUNC_EXP2L:
			resultValue = exp2l(doubleParamValues[0]);
			break;
		case STD_FUNC_EXP2F:
			resultValue = exp2f(doubleParamValues[0]);
			break;
#ifdef EXPR_C99_SUPPORT
		case STD_FUNC_EXPM1:
			resultValue = expm1(doubleParamValues[0]);
			break;
		case STD_FUNC_EXPM1L:
			resultValue = expm1l(doubleParamValues[0]);
			break;
		case STD_FUNC_EXPM1F:
			resultValue = expm1f(doubleParamValues[0]);
			break;
#endif
		default:
			printf("getFunctionDoubleValue: function not supported sorry...\n");
			break;
	}
	
	//remove function bracket from parse list
	exprOperator_t *currentOperator = functionBracketOpen;
	while(currentOperator != functionBracketClosed)
	{
		exprOperator_t *next = (exprOperator_t*)currentOperator->next;
		deleteExprOperator(calculation, currentOperator);
		currentOperator = next;
	}
	deleteExprOperator(calculation, functionBracketClosed);
	
	if (op->isNegative)
	{
		resultValue = -resultValue;
	}
		
	setOperator(op, VALUE, resultValue);
	
	calcParseDebugPrintf(" = %s\n", op->actual);

	return resultValue;
}

static double getOperatorDoubleValue(exprCalculation_t *calculation, exprOperator_t* op)
{
	assert(op);

	if (VALUE == op->type)
	{
		return op->value;
	}
	else if (VARIABLE == op->type)
	{
		if (op->variable == NULL)
		{
			int subStringIndex = 0;
			if (op->isNegative)
			{
				subStringIndex = 1;
			}

			op->variable = getVariable(calculation, op->actual+subStringIndex);
		}
		
		if (op->variable == NULL)
		{
			printf("getOperatorDoubleValue: could not find variable '%s'!\n",op->actual);
			return 0.0f;
		}
		
		double *doubleValue = op->variable->valuePointer;
		if (op->isNegative)
		{
			return -*doubleValue;
		}
		return *doubleValue;
	}
	else if (isFunction(op->type))
	{
		return getFunctionDoubleValue(calculation, op);
	}
	
	printf("getOperatorDoubleValue: invalid operator type: %d!\n", op->type);
	return 0.0f;
}

static void setOperatorType(exprOperator_t* op, int type)
{
	op->type = type;
}

static void setOperatorValue(exprOperator_t* op, double value)
{
	op->value = value;
}

static void setActualString(exprOperator_t* op, const char *str)
{
	op->actual = strdup(str);
}

#define MAX_VARIABLE_SIZE 32
#define MAX_OPERATOR_SIZE 32

static char variable[MAX_VARIABLE_SIZE];
static int variableLength = 0;

static int isCharacter(char c, const char *characterList)
{
	unsigned int i;
	for (i=0; i < strlen(characterList); i++)
	{
		//check if character is in character list
		if (c == characterList[i])
		{
			return 1;
		}
	}
	
	return 0;
}

static void clearString(char *string)
{
	int stringLength = strlen(string);

	int i;
	for(i=0; i < stringLength; i++)
	{
		string[i] = '\0';
	}
	variableLength = 0;
}

static void fillString(char *string, int stringSize, char character)
{
	int i;
	for(i=0; i<stringSize; i++)
	{
		string[i] = character;
	}
}

static void addToVariable(char character)
{
	variable[variableLength] = character;
	variableLength++;
}

static int isCodeVariableCharacter(char c)
{
	//Code variable can only include numbers and a-z or A-Z characters and underscore
	if ((c >= '0' && c <= '9')
		|| (c >= 'a' && c <= 'z')
		|| (c >= 'A' && c <= 'Z')
		|| c == '_')
	{
		return 1;
	}
	
	return 0;
}

/*static int isCodeVariable()
{
	//number can't be the first character of a variable
	if (variable[0] >= '0' && variable[0] <= '9')
	{
		return 0;
	}

	unsigned int i;
	for (i=0; i < strlen(variable); i++)
	{
		if (isCodeVariableCharacter(variable[i]) == 0)
		{
			return 0;
		}
	}
	
	return 1;
}*/

static int isFunctionName(const char *str)
{
	int i;
	for(i=0; i<MAX_FUNCTIONS; i++)
	{
		if (!strcmp(functionList[i].name, str))
		{
			return functionList[i].type;
		}
	}

	return 0;
}

static int isFunction(int type)
{
	if (type >= FUNCTION_BASE)
	{
		return 1;
	}

	return 0;
}

static int parseExpression(exprCalculation_t *calculation)
{
	/*
	 Math expression rules:
	 ( )
	 potenssit ja neliÃ¶juuret
	 * /
	 + -
	 */
	
	char *expression = calculation->expression;
	
	fillString(variable, MAX_VARIABLE_SIZE, '\0');
	
	int isDigit    = 0;
	int isExpr     = 0;
	int isVariable = 0;
	variableLength = 0;
	

	unsigned int i;
	for (i=0; i <= strlen(expression); i++)
	{
		char character = 0;
		
		if (i == strlen(expression))
		{
			character = expression[i-1];

			if (1 == isVariable)
			{
				tokenParseDebugPrintf("%s\n", variable);
				
				exprOperator_t *op = addOperator(calculation);
				
				int subStringIndex = 0;
				if (variable[0] == '-' || variable[0] == '+')
				{
					if (variable[0] == '-')
					{
						op->isNegative = 1;
					}
					subStringIndex = 1;
				}

				setOperatorType(op, VARIABLE);
				setActualString(op, variable);
				
				int functionType = isFunctionName(variable+subStringIndex);
				if (functionType != 0)
				{
					setOperatorType(op, functionType);
				}
				
				clearString(variable);
				isVariable = 0;
			}
			if (1 == isDigit)
			{
				tokenParseDebugPrintf("%s\n", variable);
				
				exprOperator_t *op = addOperator(calculation);
				setOperatorType( op, VALUE);
				setOperatorValue(op, atof(variable));
				setActualString(op, variable);
				
				clearString(variable);
				isDigit = 0;
			}
		}
		else
		{
			character = expression[i];
		}

		if (isCharacter(character, skipCharacters))
		{
			if (1 == isDigit)
			{
				tokenParseDebugPrintf("%s\n", variable);
				
				exprOperator_t *op = addOperator(calculation);
				setOperatorType( op, VALUE);
				setOperatorValue(op, atof(variable));
				setActualString(op, variable);
				
				clearString(variable);
				isDigit = 0;
			}
			if (1 == isExpr)
			{
				tokenParseDebugPrintf("%s\n", variable);
				exprOperator_t *op = addOperator(calculation);
				setOperatorType(op, getOperatorCharacterType(variable[0]));
				setActualString(op, variable);

				clearString(variable);
				isExpr = 0;
			}
			if (1 == isVariable)
			{
				tokenParseDebugPrintf("%s\n", variable);
				
				exprOperator_t *op = addOperator(calculation);
				
				int subStringIndex = 0;
				if (variable[0] == '-' || variable[0] == '+')
				{
					if (variable[0] == '-')
					{
						op->isNegative = 1;
					}
					subStringIndex = 1;
				}

				setOperatorType(op, VARIABLE);
				setActualString(op, variable);
				
				int functionType = isFunctionName(variable+subStringIndex);
				if (functionType != 0)
				{
					setOperatorType(op, functionType);
				}
				
				clearString(variable);
				isVariable = 0;
			}
			
			continue;
		}

		if (isCharacter(character, digitCharacters))
		{
			if (1 == isExpr)
			{
				if (strlen(variable) == 1
					&& (variable[0] == '-' || variable[0] == '+')
					&& (calculation->operatorTail == NULL
						||
						(isCharacter(calculation->operatorTail->actual[0], signValueValidCharacters)
						 && strlen(calculation->operatorTail->actual) == 1)
						)
					)
				{
					isExpr = 0;
					isDigit = 1;					
					addToVariable(character);
					continue;
				}
				
				tokenParseDebugPrintf("%s\n", variable);
				
				exprOperator_t *op = addOperator(calculation);
				setOperatorType(op, getOperatorCharacterType(variable[0]));
				setActualString(op, variable);
				
				clearString(variable);
				isExpr = 0;
				
			}
			if (1 == isVariable)
			{
				addToVariable(character);
				continue;
			}
			
			isDigit = 1;
			addToVariable(character);
			
			continue;
		}
		
		if (1 == isCodeVariableCharacter(character))
		{
			if (1 == isDigit)
			{
				tokenParseDebugPrintf("%s\n", variable);
				
				exprOperator_t *op = addOperator(calculation);
				setOperatorType( op, VALUE);
				setOperatorValue(op, atof(variable));
				setActualString(op, variable);
				
				clearString(variable);
				isDigit = 0;
			}
			if (1 == isExpr)
			{
				if (strlen(variable) == 1
					&& (variable[0] == '-' || variable[0] == '+')
					&& (calculation->operatorTail == NULL
						||
						(isCharacter(calculation->operatorTail->actual[0], signValueValidCharacters)
						 && strlen(calculation->operatorTail->actual) == 1)
						)
					)
				{
					isExpr     = 0;
					isVariable = 1;					
					addToVariable(character);
					continue;
				}
				
				tokenParseDebugPrintf("%s\n", variable);
				
				exprOperator_t *op = addOperator(calculation);
				setOperatorType(op, getOperatorCharacterType(variable[0]));
				setActualString(op, variable);
				
				clearString(variable);
				isExpr = 0;
			}
			
			isVariable = 1;
			addToVariable(character);
			
			continue;
		}

		if (isCharacter(character, exprCharacters))
		{
			if (1 == isDigit)
			{
				tokenParseDebugPrintf("%s\n", variable);
				
				exprOperator_t *op = addOperator(calculation);
				setOperatorType( op, VALUE);
				setOperatorValue(op, atof(variable));
				setActualString(op, variable);
				
				clearString(variable);
				isDigit = 0;
			}
			if (1 == isExpr)
			{
				tokenParseDebugPrintf("%s\n", variable);

				exprOperator_t *op = addOperator(calculation);
				setOperatorType(op, getOperatorCharacterType(variable[0]));
				setActualString(op, variable);

				clearString(variable);
				isExpr = 0;
			}
			if (1 == isVariable)
			{
				tokenParseDebugPrintf("%s\n", variable);
				
				exprOperator_t *op = addOperator(calculation);
				
				tokenParseDebugPrintf("1!", variable);

				int subStringIndex = 0;
				if (variable[0] == '-' || variable[0] == '+')
				{
					if (variable[0] == '-')
					{
						op->isNegative = 1;
					}
					subStringIndex = 1;
				}

				setOperatorType(op, VARIABLE);
				setActualString(op, variable);
				
				int functionType = isFunctionName(variable+subStringIndex);
				if (functionType != 0)
				{
					setOperatorType(op, functionType);
				}

				clearString(variable);
				isVariable = 0;
			}
			
			isExpr = 1;
			addToVariable(character);
			
			continue;
		}
		
		//error parsing
		addToVariable(character);
		printf("%s\n", expression);
		unsigned int error_i;
		for(error_i = 0; error_i < i; error_i++)
		{
			printf(" ");
		}
		printf("^\n");
		printf("Could not parse '%s' in position %d\n", variable, i);
		
		return 0;
	}
	
	return 1;
}

static void freeExprVariableList(exprVariable_t *variableHead)
{
	exprVariable_t* variableCurrent    = variableHead;
	while(variableCurrent != NULL)
	{
		exprVariable_t* var  = variableCurrent;
		exprVariable_t* next = NULL;

		if (variableCurrent->next)
		{
			next = (exprVariable_t*)variableCurrent->next;
		}
		if (var)
		{
			if (var->name)
			{
				free(var->name);
				var->name = NULL;
			}

			free(var);
			var = NULL;
		}
		
		variableCurrent    = next;
	}
}

static void freeExprOperatorList(exprOperator_t *opHead)
{
	exprOperator_t* operatorCurrent    = opHead;
	while(operatorCurrent != NULL)
	{
		exprOperator_t* op = operatorCurrent;
		if (op)
		{
			if (op->actual)
			{
				free(op->actual);
			}
			
			free(op);
			
			op = NULL;
		}
		
		operatorCurrent    = (exprOperator_t*)operatorCurrent->next;
	}
}

static void checkTailAndHead(exprCalculation_t *calculation, exprOperator_t *op)
{
	if (calculation->operatorTail == op)
	{
		if (op->prev != NULL)
		{
			calculation->operatorTail = (exprOperator_t*)op->prev;
		}
		else
		{
			calculation->operatorTail = NULL;
		}
	}

	if (calculation->operatorHead == op)
	{
		if (op->next != NULL)
		{
			calculation->operatorHead = (exprOperator_t*)op->next;
		}
		else
		{
			calculation->operatorHead = NULL;
		}
	}	
}

static void deleteExprOperator(exprCalculation_t *calculation, exprOperator_t *op)
{
	if (op)
	{
		if (op->next != NULL)
		{
			((exprOperator_t*)op->next)->prev = op->prev;
		}
		else
		{
			checkTailAndHead(calculation, op);
		}

		if (op->prev != NULL)
		{
			((exprOperator_t*)op->prev)->next = op->next;
		}
		else
		{
			checkTailAndHead(calculation, op);
		}
		
		if (op->actual)
		{
			free(op->actual);
		}
		
		free(op);
	}
}

static void setOperator(exprOperator_t *op, int type, double value)
{
	assert(op);

	op->type   = type;
	op->value  = value;
	
	if (op->actual)
	{
		free(op->actual);
	}
	
	char str[MAX_OPERATOR_SIZE];
	snprintf(str, MAX_OPERATOR_SIZE, "%f", value);
	op->actual = strdup(str);
	
	//cut zero values
	unsigned int i, decimalPointOccurred = 0;
	for (i = 0; i < strlen(op->actual); i++)
	{
		if (op->actual[i] == '.')
		{
			decimalPointOccurred = 1;
		}
		
		if (decimalPointOccurred && op->actual[i] == '0')
		{
			op->actual[i] = '\0';
			if (op->actual[i-1] == '.')
			{
				op->actual[i-1] = '\0';
			}
		}
	}
}

/*static void printOperatorInfo(exprOperator_t *op)
{
	if (op == NULL) { printf("setOperator: op is NULL!\n"); }

	printf("%d: '%s' (%f) \n",op->type,op->actual,op->value);
}*/

static void calculateSubExpression(exprCalculation_t *calculation, exprOperator_t *start, exprOperator_t *end)
{
	exprOperator_t* operatorCurrent = start;
	exprOperator_t* prev = NULL;
	exprOperator_t* next = NULL;
	//printf("start: %d'%s'\n",operatorCurrent->type,operatorCurrent->actual);
	while (operatorCurrent != NULL && operatorCurrent->type != BRACKET_CLOSE)
	{
		prev = (exprOperator_t*)operatorCurrent->prev;
		next = (exprOperator_t*)operatorCurrent->next;
		
		//printf("%p -> %d:'%s', value = %.2f\n",operatorCurrent,operatorCurrent->type, operatorCurrent->actual, operatorCurrent->value);
		if (MULTIPLY == operatorCurrent->type)
		{
			if (!isValueTypeValid(prev->type) || !isValueTypeValid(next->type))
			{
				printf("Could not parse expression '%s%s%s'\n", prev->actual, operatorCurrent->actual, next->actual);
			}
			
			double value = getOperatorDoubleValue(calculation, prev) * getOperatorDoubleValue(calculation, next);

			calcParseDebugPrintf ("MUL: %s*%s=%f\n",((exprOperator_t*)operatorCurrent->prev)->actual, ((exprOperator_t*)operatorCurrent->next)->actual, value);
			setOperator(prev, VALUE, value);

			exprOperator_t *newOperator = prev;
			deleteExprOperator(calculation, next);
			deleteExprOperator(calculation, operatorCurrent);
			operatorCurrent = newOperator;
		}
		else if (DIVIDE == operatorCurrent->type)
		{
			if (!isValueTypeValid(prev->type) || !isValueTypeValid(next->type))
			{
				printf("Could not parse expression '%s%s%s'\n", prev->actual, operatorCurrent->actual, next->actual);
			}

			double value = getOperatorDoubleValue(calculation, prev) / getOperatorDoubleValue(calculation, next);
			
			calcParseDebugPrintf ("DIV: %s/%s=%f\n",((exprOperator_t*)operatorCurrent->prev)->actual, ((exprOperator_t*)operatorCurrent->next)->actual, value);
			setOperator(prev, VALUE, value);
			
			exprOperator_t *newOperator = prev;
			deleteExprOperator(calculation, next);
			deleteExprOperator(calculation, operatorCurrent);
			operatorCurrent = newOperator;
		}
		else if (isFunction(operatorCurrent->type))
		{
			getFunctionDoubleValue(calculation, operatorCurrent);
		}
		
		operatorCurrent = (exprOperator_t*)operatorCurrent->next;
	}
	operatorCurrent = start;
	//printf("start: %d'%s'\n",operatorCurrent->type,operatorCurrent->actual);
	while (operatorCurrent != NULL && operatorCurrent->type != BRACKET_CLOSE)
	{
		prev = (exprOperator_t*)operatorCurrent->prev;
		next = (exprOperator_t*)operatorCurrent->next;

		if (SUM == operatorCurrent->type)
		{
			if (!isValueTypeValid(prev->type) || !isValueTypeValid(next->type))
			{
				printf("Could not parse expression '%s%s%s'\n", prev->actual, operatorCurrent->actual, next->actual);
			}
			
			double value = getOperatorDoubleValue(calculation, prev) + getOperatorDoubleValue(calculation, next);
			
			calcParseDebugPrintf ("SUM: %s+%s=%f\n",((exprOperator_t*)operatorCurrent->prev)->actual, ((exprOperator_t*)operatorCurrent->next)->actual, value);
			setOperator(prev, VALUE, value);
			
			exprOperator_t *newOperator = prev;
			deleteExprOperator(calculation, next);
			deleteExprOperator(calculation, operatorCurrent);
			operatorCurrent = newOperator;
		}
		else if (SUB == operatorCurrent->type)
		{
			if (!isValueTypeValid(prev->type) || !isValueTypeValid(next->type))
			{
				printf("Could not parse expression '%s%s%s'\n", prev->actual, operatorCurrent->actual, next->actual);
			}

			double value = getOperatorDoubleValue(calculation, prev) - getOperatorDoubleValue(calculation, next);
			
			calcParseDebugPrintf ("SUB: %s-%s=%f\n",((exprOperator_t*)operatorCurrent->prev)->actual, ((exprOperator_t*)operatorCurrent->next)->actual, value);
			setOperator(prev, VALUE, value);

			exprOperator_t *newOperator = prev;
			deleteExprOperator(calculation, next);
			deleteExprOperator(calculation, operatorCurrent);
			operatorCurrent = newOperator;
		}
		operatorCurrent = (exprOperator_t*)operatorCurrent->next;
	}

	if (start->prev && isFunction(((exprOperator_t*)(start->prev))->type))
	{
		start->isLocked = 1;
		end->isLocked   = 1;
	}
	else
	{
		if (start->type == BRACKET_OPEN)
		{
			deleteExprOperator(calculation, start);

			//for comma parsing we'll check ending bracket only if first parse operator is opening bracket
			if (end->type == BRACKET_CLOSE)
			{
				deleteExprOperator(calculation, end);
			}
		}
	}
}

//public
void exprFreeExpression(exprCalculation_t *calculation)
{
	if (calculation)
	{
		if (calculation->expression)
		{
			free(calculation->expression);
			calculation->expression = NULL;
		}
		
		if (calculation->variableHead)
		{
			freeExprVariableList(calculation->variableHead);
			calculation->variableHead = NULL;
			calculation->variableTail = NULL;
		}
		
		if (calculation->operatorHead)
		{
			freeExprOperatorList(calculation->operatorHead);
			calculation->operatorHead = NULL;
			calculation->operatorTail = NULL;
		}
				
		free(calculation);
	}
	else
	{
		printf("exprFreeExpression: expression is null!\n");
	}
}

//some constant variables here, bitte
static double pi = M_PI;

static void setExpression(exprCalculation_t *calculation, const char *expression)
{
	if (calculation->variableHead)
	{
		freeExprVariableList(calculation->variableHead);
		calculation->variableHead = NULL;
		calculation->variableTail = NULL;
	}
	
	if (calculation->operatorHead)
	{
		freeExprOperatorList(calculation->operatorHead);
		calculation->operatorHead = NULL;
		calculation->operatorTail = NULL;
	}
	
	if (calculation->expression)
	{
		free(calculation->expression);
		calculation->expression = NULL;
	}
	calculation->expression   = (char*)malloc(sizeof(char)*(strlen(expression)+1));
	strcpy(calculation->expression, expression);
	calculation->isError      = 0;
	calculation->result       = 0.0;

	//constant declaration here
	exprAddVariable(calculation, "M_PI", &pi);
}

//public
exprCalculation_t *exprNewExpression(const char *expression)
{
	exprCalculation_t *calculation = (exprCalculation_t*)malloc(sizeof(exprCalculation_t));
	calculation->expression = NULL;
	
	setExpression(calculation, expression);

	return calculation;
}

//public
double exprCalculateExpression(exprCalculation_t *calculation)
{
	if (isCalcTrace)
	{
		printf("user      expression = '%s'\n", calculation->expression);
	}

	if (1 == parseExpression(calculation))
	{
		exprOperator_t* operatorCurrent  = NULL;
		exprOperator_t* bracketOpen      = NULL;
		exprOperator_t* bracketClose     = NULL;
		
		if (isCalcTrace)
		{
			printf("user      expression = '%s'\n", calculation->expression);
			printf("tokenized expression = '");
			operatorCurrent = calculation->operatorHead;
			while(operatorCurrent != NULL)
			{			
				printf("%s",operatorCurrent->actual);			
				operatorCurrent = (exprOperator_t*)operatorCurrent->next;
			}
			printf("'\n");
		}
		
		operatorCurrent = calculation->operatorHead;
		while(operatorCurrent != NULL)
		{			
			if (BRACKET_OPEN == operatorCurrent->type && !operatorCurrent->isLocked)
			{
				bracketOpen  = operatorCurrent;
				bracketClose = NULL;
			}
			else if (bracketOpen && BRACKET_CLOSE == operatorCurrent->type && !operatorCurrent->isLocked)
			{
				bracketClose = operatorCurrent;
				
				//operatorCurrent = (exprOperator_t*)bracketOpen->prev;
				
				calculateSubExpression(calculation, bracketOpen, bracketClose);
				
				bracketOpen  = NULL;
				bracketClose = NULL;
				
				operatorCurrent = calculation->operatorHead;
			}
			else if (!bracketOpen && BRACKET_CLOSE == operatorCurrent->type && !operatorCurrent->isLocked)
			{
				printf("Parse error! Excessive closing bracket in expression.\n");
				//return EXIT_FAILURE;
			}
			
			//printf("%d:'%s', value = %.2f\n",operatorCurrent->type, operatorCurrent->actual, operatorCurrent->value);
			
			operatorCurrent = (exprOperator_t*)operatorCurrent->next;
		}
		if (bracketOpen)
		{
			printf("Parse error! Excessive open bracket in expression.\n");
			//return EXIT_FAILURE;
		}
		if (bracketClose)
		{
			printf("Parse error! Excessive closed bracket in expression.\n");
			//return EXIT_FAILURE;
		}
		calculateSubExpression(calculation, calculation->operatorHead, calculation->operatorTail);
		
		if (isCalcTrace)
		{
			//print expression
			operatorCurrent = calculation->operatorHead;
			printf("unrolled expression = '");
			while(operatorCurrent != NULL)
			{			
				printf("%s",operatorCurrent->actual);			
				operatorCurrent = (exprOperator_t*)operatorCurrent->next;
			}
			printf("'\n");
		}
	}
	
	if (calculation->operatorHead == calculation->operatorTail)
	{
		//case that user wanted to return just a variable
		if (calculation->operatorHead->type == VARIABLE)
		{
			calculation->operatorHead->value = getOperatorDoubleValue(calculation, calculation->operatorHead);
		}
	}
	else
	{
		calculation->isError = 1;
		printf("Calculation failed due to a parse error!\n");
	}

	if (isCalcTrace)
	{
		printf("%s = %f\n",calculation->expression,calculation->operatorHead->value);
	}
	/*else
	{
		printf("%f\n",operatorHead->value);
	}*/
	
	calculation->isError = 0;
	calculation->result  = calculation->operatorHead->value;
	return calculation->operatorHead->value;
}

#ifdef EXPR_STANDALONE
//Quick test for bidirectional linked list handling
static void testLinkedList(exprCalculation_t *calculation)
{
	puts("START linked list test");
#define ADD_OP(STR) op = addOperator(calculation); setActualString(op, STR)
	
	exprOperator_t *op = NULL;
	
	ADD_OP("A");
	ADD_OP("B");
	ADD_OP("C");
	ADD_OP("D");
	ADD_OP("E");
	ADD_OP("F");
	ADD_OP("G");
	
	puts ("First run");
	exprOperator_t* operatorCurrent    = calculation->operatorHead;
	exprOperator_t* operatorCurrentRwd = calculation->operatorTail;
	while(operatorCurrent != NULL)
	{			
		printf("%s\t%s\n",operatorCurrent->actual,operatorCurrentRwd->actual);
		operatorCurrent    = (exprOperator_t*)operatorCurrent->next;
		operatorCurrentRwd = (exprOperator_t*)operatorCurrentRwd->prev;
	}

	exprOperator_t* opDeleteHead = NULL; 
	
	operatorCurrent    = calculation->operatorHead;
	operatorCurrentRwd = calculation->operatorTail;
	int i = 0;
	while(operatorCurrent != NULL)
	{		
		if (i == 1)
		{
			opDeleteHead = (exprOperator_t*)operatorCurrent->next;
			deleteExprOperator(calculation, operatorCurrent);
		}
		
		operatorCurrent    = (exprOperator_t*)operatorCurrent->next;
		i++;
	}
	
	operatorCurrent    = calculation->operatorHead;
	operatorCurrentRwd = calculation->operatorTail;
	puts ("Second run");
	while(operatorCurrent != NULL)
	{			
		printf("%s\t%s\n",operatorCurrent->actual,operatorCurrentRwd->actual);
		operatorCurrent    = (exprOperator_t*)operatorCurrent->next;
		operatorCurrentRwd = (exprOperator_t*)operatorCurrentRwd->prev;
	}
	
	freeExprOperatorList(opDeleteHead);
	operatorCurrent    = calculation->operatorHead;
	operatorCurrentRwd = calculation->operatorTail;
	puts ("Third run");
	while(operatorCurrent != NULL)
	{			
		printf("%s\t%s\n",operatorCurrent->actual,operatorCurrentRwd->actual);
		operatorCurrent    = (exprOperator_t*)operatorCurrent->next;
		operatorCurrentRwd = (exprOperator_t*)operatorCurrentRwd->prev;
	}
	
	freeExprOperatorList(calculation->operatorHead);
	calculation->operatorHead = NULL;
	calculation->operatorTail = NULL;

	operatorCurrent    = calculation->operatorHead;
	operatorCurrentRwd = calculation->operatorTail;
	puts ("Fourth run");
	while(operatorCurrent != NULL)
	{			
		printf("%s\t%s\n",operatorCurrent->actual,operatorCurrentRwd->actual);
		operatorCurrent    = (exprOperator_t*)operatorCurrent->next;
		operatorCurrentRwd = (exprOperator_t*)operatorCurrentRwd->prev;
	}
	
	puts("END linked list test");
}

static void printConstantList(void)
{
	printf("Defined constants:\n");
	printf("M_PI = %f\n",M_PI);
}

static void printFunctionList(void)
{
	printf("Defined functions:\n");
	const char *parameters = "xyz";
	int i;
	for(i=0; i < MAX_FUNCTIONS; i++)
	{
		printf("%s (",functionList[i].name);
		int j;
		for(j=0; j < functionList[i].params; j++)
		{
			printf("%c",parameters[j]);
			
			if(j+1 != functionList[i].params)
			{
				printf(", ");
			}
		}
		printf(") - %s\n",functionList[i].description);
	}
}

static void printHelp(char *argv)
{
	printf("%s [--function-list] [--constant-list] [--trace] [--help] [--usage] <statement>\n",argv);
}

static void printUsage(char *argv)
{
	printf("Usage: %s \"123 + sin(4.5)*2\"\n",argv);
}


static int expr_main(int argc, char **argv)
{
	char *expression = NULL;

	if (argc < 2)
	{
		printf("Not enough arguments!\n\n");
		printHelp(argv[0]);
		printUsage(argv[0]);
		return EXIT_SUCCESS;
	}
	
	int argi;
	for(argi=1; argi < argc; argi++)
	{
		char *argument = argv[argi];
		
		if (!strcmp(argument, "--function-list"))
		{
			printFunctionList();
		}
		else if (!strcmp(argument, "--constant-list"))
		{
			printConstantList();
		}
		else if (!strcmp(argument, "--help"))
		{
			printHelp(argv[0]);
		}
		else if (!strcmp(argument, "--trace"))
		{
			isCalcTrace = 1;
		}
		else if (!strcmp(argument, "--usage"))
		{
			printUsage(argv[0]);
		}
		else
		{
			expression = argv[argi];
		}
	}

	if (expression == NULL)
	{
		return EXIT_SUCCESS;
	}

	exprCalculation_t *calculation = exprNewExpression(expression);
	exprCalculateExpression(calculation);
	printf("%f\n",calculation->result);
	exprFreeExpression(calculation);

	/*exprCalculation_t *calculation;
	
	calculation = exprNewExpression(expression);
	exprCalculateExpression(calculation);
	printf("=> %f\n",calculation->result);

	setExpression(calculation, "123*5435/2+sqrt(34.34)+1");
	exprCalculateExpression(calculation);
	printf("=> %f\n",calculation->result);
	
	setExpression(calculation, "M_PI");
	exprCalculateExpression(calculation);
	printf("=> %f\n",calculation->result);

	setExpression(calculation,"34785+12*543/454+3/(2+1*(5+3/1))*sqrt(((1+3*(1+1)/2)*5+3)/34)");
	printf("! => '%s'\n",((exprVariable_t*)getVariable(calculation, "M_PI"))->name);
	exprCalculateExpression(calculation);
	printf("=> %f\n",calculation->result);
	
	exprFreeExpression(calculation);*/
	
	//testLinkedList();
	//return EXIT_SUCCESS;

	//const char *expression      = "34785+12*543/454+3/(2+1*(5+3/1))*sqrt(((1+3*(1+1)/2)*5+3)/34)"; //= 34800.35242290749

//calculatio here
	
	return EXIT_SUCCESS;
}
#endif
