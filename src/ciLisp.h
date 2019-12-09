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

int yyparse(void);

int yylex(void);

void yyerror(char *s);

void printWarning(char *s);

typedef enum oper {
	NEG_OPER,
	ABS_OPER,
	ADD_OPER,
	SUB_OPER,
	MULT_OPER,
	DIV_OPER,
	REMAINDER_OPER,
	EXP_OPER,
	EXP2_OPER,
	POW_OPER,
	LOG_OPER,
	SQRT_OPER,
	CBRT_OPER,
	HYPOT_OPER,
	MAX_OPER,
	MIN_OPER,
	PRINT_OPER,
	READ_OPER,
	RAND_OPER,
	EQUAL_OPER,
	LESS_OPER,
	GREATER_OPER,
	CUSTOM_OPER
} OPER_TYPE;

OPER_TYPE resolveFunc(char *);

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

typedef struct symbol_table_node {
	NUM_TYPE val_type;
	char *ident;
	struct ast_node *value;
	struct symbol_table_node *next;
} SYMBOL_TABLE_NODE;

typedef struct {
	struct ast_node *cond;
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

NUM_TYPE castType(char *type);
AST_NODE *createASTNode();
AST_NODE *createNumberNode(double value, NUM_TYPE type);
AST_NODE *createFunctionNode(char *funcName, AST_NODE *op_list);
AST_NODE *createExpressionList(AST_NODE *s_expr, AST_NODE *s_expr_list);
AST_NODE *createSymbolASTNode(char *ident);
AST_NODE *createConditionNode(AST_NODE *condition, AST_NODE *trueExpr, AST_NODE *falseExpr);

SYMBOL_TABLE_NODE *createSymbolTableNode(char *type, char *ident, AST_NODE *value);
SYMBOL_TABLE_NODE *createLetList(SYMBOL_TABLE_NODE *let_list, SYMBOL_TABLE_NODE *let_elem);
AST_NODE *setSymbolScope(SYMBOL_TABLE_NODE *let_list, AST_NODE *s_expr);

void freeNode(AST_NODE *node);

RET_VAL eval(AST_NODE *node);
RET_VAL evalNumNode(AST_NODE *node);
RET_VAL evalFuncNode(AST_NODE *node);
RET_VAL evalSymbolNode(AST_NODE *node);
RET_VAL evalConditionNode(AST_NODE *node);
SYMBOL_TABLE_NODE *findSymbolTableNode(char *ident, AST_NODE *node);

void printRetVal(RET_VAL val);

#endif
