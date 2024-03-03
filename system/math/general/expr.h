#ifndef EXH_SYSTEM_MATH_GENERAL_EXPR_H_
#define EXH_SYSTEM_MATH_GENERAL_EXPR_H_

typedef struct
{
	int type;
	const char *name;
	int params;
	const char *description;
} exprFunction_t;

typedef struct exprVariable_t exprVariable_t;

struct exprVariable_t {
	union
	{
		void   *valuePointer;	

		float  *floatPointer;
		double *doublePointer;

		short  *shortPointer;
		int    *intPointer;
		long   *longPointer;
	};
	char *name;
	struct exprVariable_t *prev, *next;
};

typedef struct exprOperator_t exprOperator_t;

struct exprOperator_t {
	double value;
	int type;
	int isLocked;
	int isNegative;
	char *actual;
	exprVariable_t *variable;
	struct exprOperator_t *prev, *next;
};

typedef struct {
	char *expression;
	exprVariable_t* variableHead;
	exprVariable_t* variableTail;
	exprOperator_t* operatorHead;
	exprOperator_t* operatorTail;
	int isError;
	double result;
} exprCalculation_t;

extern exprVariable_t* exprAddVariable(exprCalculation_t *calculation, const char *name, void* valuePointer);
extern void exprFreeExpression(exprCalculation_t *calculation);
extern exprCalculation_t *exprNewExpression(const char *expression);
extern double exprCalculateExpression(exprCalculation_t *calculation);

#endif /*EXH_SYSTEM_MATH_SORT_EXPR_H_*/
