/**
* Name: Austin Fisher
* Lab: CILisp
* Date: 12/11/19
**/
#ifndef __cilisp_h_
#define __cilisp_h_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "ciLispParser.h"

#define BUFF_SIZE 1024
#define DEFAULT_RET_VAL (RET_VAL){INT_TYPE, NAN}

typedef enum oper {
	//UNARY
	NEG_OPER,
	ABS_OPER,
	EXP_OPER,
	EXP2_OPER,
	LOG_OPER,
	SQRT_OPER,
	CBRT_OPER,
	//BINARY
	SUB_OPER,
	DIV_OPER,
	REMAINDER_OPER,
	POW_OPER,
	EQUAL_OPER,
	LESS_OPER,
	GREATER_OPER,
	//N-ARY
	ADD_OPER,
	MULT_OPER,
	HYPOT_OPER,
	MAX_OPER,
	MIN_OPER,
	//NO-OPERANDS
	PRINT_OPER,
	READ_OPER,
	RAND_OPER,
	//UNKNOWN
	CUSTOM_OPER
} OPER_TYPE;

typedef enum {
    NUM_NODE_TYPE,
    FUNC_NODE_TYPE,
    SYMBOL_NODE_TYPE,
    COND_NODE_TYPE
} AST_NODE_TYPE;

typedef enum {
    INT_TYPE = 0,
    DOUBLE_TYPE,
    UNSPECIFIED_TYPE
} NUM_TYPE;

typedef enum {
	VARIABLE_TYPE,
	LAMBDA_TYPE,
	ARG_TYPE
} SYMBOL_TYPE;

typedef struct {
    NUM_TYPE type;
    double value;
} NUM_AST_NODE;

typedef NUM_AST_NODE RET_VAL;

typedef struct {
    OPER_TYPE oper;
    char* ident;
	struct ast_node *op_list;
} FUNC_AST_NODE;

typedef struct symbol_ast_node {
	char *ident;
} SYMBOL_AST_NODE;

typedef struct stack_node {
	struct ast_node *value;
	struct stack_node *next;
} STACK_NODE;

typedef struct symbol_table_node {
	SYMBOL_TYPE type;
	NUM_TYPE val_type;
	char *ident;
	struct ast_node *value;
	STACK_NODE *stack;
	struct symbol_table_node *next;
} SYMBOL_TABLE_NODE;

typedef struct {
	struct ast_node *condition;
	struct ast_node *trueExpr;
	struct ast_node *falseExpr;
} COND_AST_NODE;

typedef struct ast_node {
    AST_NODE_TYPE type;
	SYMBOL_TABLE_NODE *symbolTable;
	struct ast_node *parent;
    union {
        NUM_AST_NODE number;
        FUNC_AST_NODE function;
        COND_AST_NODE condition;
        SYMBOL_AST_NODE symbol;
    } data;
    struct ast_node *next;
} AST_NODE;

int yyparse(void);
int yylex(void);
void yyerror(char *s);
void printWarning(char *s);

void freeNode(AST_NODE *node);

OPER_TYPE resolveFunc(char *);

NUM_TYPE castType(char *type);
AST_NODE *initASTNode();
AST_NODE *createNumberNode(double value, NUM_TYPE type);
AST_NODE *createFunctionNode(char *funcName, AST_NODE *op_list);
AST_NODE *createExpressionList(AST_NODE *s_expr, AST_NODE *s_expr_list);
AST_NODE *createSymbolASTNode(char *ident);
AST_NODE *createConditionNode(AST_NODE *condition, AST_NODE *trueExpr, AST_NODE *falseExpr);

SYMBOL_TABLE_NODE *initSymbolTableNode();
SYMBOL_TABLE_NODE *createSymbolTableNode(char *val_type, char *ident, AST_NODE *value);
SYMBOL_TABLE_NODE *createArgList(char *argument, SYMBOL_TABLE_NODE *arg_list);
SYMBOL_TABLE_NODE *createLambdaFunc(char *type, char *ident, SYMBOL_TABLE_NODE *arg_list, AST_NODE *function);
SYMBOL_TABLE_NODE *createLetList(SYMBOL_TABLE_NODE *let_list, SYMBOL_TABLE_NODE *let_elem);
AST_NODE *setSymbolScope(SYMBOL_TABLE_NODE *let_list, AST_NODE *s_expr);

RET_VAL eval(AST_NODE *node);
RET_VAL evalNumNode(AST_NODE *node);
RET_VAL evalFuncNode(AST_NODE *node);
RET_VAL evalSymbolNode(AST_NODE *node);
RET_VAL evalConditionNode(AST_NODE *node);
RET_VAL evalCustomFunction(AST_NODE *node);
SYMBOL_TABLE_NODE *findSymbolTableNode(char *ident, AST_NODE *node);
void linkStackNodes(SYMBOL_TABLE_NODE *arguments, STACK_NODE *parameters);
STACK_NODE *createStackNodes(AST_NODE *lambdaFunc, AST_NODE *parameter_list);

void printRetVal(RET_VAL val);

#endif
