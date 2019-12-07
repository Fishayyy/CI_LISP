#include "ciLisp.h"

void yyerror(char *s) {
    fprintf(stderr, "\nERROR: %s\n", s);
}

char *funcNames[] = {
		"neg",
		"abs",
		"exp",
		"sqrt",
		"add",
		"sub",
		"mult",
		"div",
		"remainder",
		"log",
		"pow",
		"max",
		"min",
		"exp2",
		"cbrt",
		"hypot",
		"read",
		"rand",
		"print",
		"equal",
		"less",
		"greater",
		""
};

OPER_TYPE resolveFunc(char *funcName)
{
    int i = 0;
    while (funcNames[i][0] != '\0')
    {
        if (strcmp(funcNames[i], funcName) == 0)
            return i;
        i++;
    }
    return CUSTOM_OPER;
}

AST_NODE *createNumberNode(double value, NUM_TYPE type)
{
    AST_NODE *node;
    size_t nodeSize;

    nodeSize = sizeof(AST_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
        yyerror("Memory allocation failed!");

	node->type = NUM_NODE_TYPE;
	node->symbolTable = NULL;
	node->parent = NULL;
	node->data.number.type = type;
	node->data.number.value = value;
    
    return node;
}

AST_NODE *createFunctionNode(char *funcName, AST_NODE *op1, AST_NODE *op2)
{
    AST_NODE *node;
    size_t nodeSize;

    nodeSize = sizeof(AST_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
        yyerror("Memory allocation failed!");
    
	node->type = FUNC_NODE_TYPE;
	node->symbolTable = NULL;
	node->parent = NULL;
	node->data.function.oper = resolveFunc(funcName);
	
	if(node->data.function.oper == CUSTOM_OPER)
		node->data.function.ident = funcName;
	else
		free(funcName);
	
	if(op1 != NULL)
	{
		op1->parent = node;
		node->data.function.op1 = op1;
	}
	if(op2 != NULL)
	{
		op2->parent = node;
		node->data.function.op2 = op2;
	}

    return node;
}

AST_NODE *createSymbolASTNode(char *ident)
{
	AST_NODE *node;
	size_t nodeSize;
	
	nodeSize = sizeof(AST_NODE);
	if ((node = calloc(nodeSize, 1)) == NULL)
		yyerror("Memory allocation failed!");
	
	node->type = SYMBOL_NODE_TYPE;
	node->symbolTable = NULL;
	node->parent = NULL;
	node->data.symbol.ident = ident;
	
	return node;
}

SYMBOL_TABLE_NODE *createSymbolTableNode(char *ident, AST_NODE *value)
{
	SYMBOL_TABLE_NODE *node;
	size_t nodeSize;
	
	nodeSize = sizeof(SYMBOL_TABLE_NODE);
	if ((node = calloc(nodeSize, 1)) == NULL)
		yyerror("Memory allocation failed!");
	
	node->value = value;
	node->ident = ident;
	node->next = NULL;
	
	return node;
}

SYMBOL_TABLE_NODE *createLetList(SYMBOL_TABLE_NODE *let_list, SYMBOL_TABLE_NODE *let_elem) {
	let_elem->next = let_list;
	return let_elem;
}

AST_NODE *setSymbolScope(SYMBOL_TABLE_NODE *let_list, AST_NODE *s_expr)
{
	s_expr->symbolTable = let_list;
	SYMBOL_TABLE_NODE *temp = let_list;
	
	while(temp != NULL)
	{
		temp->value->parent = s_expr;
		temp = temp->next;
	}
	return s_expr;
}

void freeNode(AST_NODE *node)
{
    if (!node)
        return;

    if (node->type == FUNC_NODE_TYPE)
    {
        freeNode(node->data.function.op1);
        freeNode(node->data.function.op2);

        if (node->data.function.oper == CUSTOM_OPER)
        {
            free(node->data.function.ident);
        }
    }

    free(node);
}

RET_VAL eval(AST_NODE *node)
{
    RET_VAL result = (RET_VAL){INT_TYPE, NAN};
    
    if (!node)
        return result;

    switch (node->type)
    {
	    case NUM_NODE_TYPE:
		    result = evalNumNode(node);
		    break;
	    case FUNC_NODE_TYPE:
		    result = evalFuncNode(node);
		    break;
    	case SYMBOL_NODE_TYPE:
    		result = evalSymbolNode(node);
        default:
            yyerror("Invalid AST_NODE_TYPE, probably invalid writes somewhere!");
    }
    return result;
}  

RET_VAL evalNumNode(AST_NODE *node)
{
    RET_VAL result = (RET_VAL){INT_TYPE, NAN};

	result.type = node->data.number.type;
	result.value = node->data.number.value;

    return result;
}

RET_VAL evalFuncNode(AST_NODE *node)
{
    RET_VAL result = (RET_VAL){INT_TYPE, NAN};

	RET_VAL op1 = eval(node->data.function.op1);
	RET_VAL op2 = eval(node->data.function.op2);
	
	if ((op1.type == DOUBLE_TYPE) || (DOUBLE_TYPE == op2.type))
		result.type = DOUBLE_TYPE;
	
	switch(node->data.function.oper)
	{
		case NEG_OPER:
			result.value = -op1.value;
			break;
		case ABS_OPER:
			result.value = fabs(op1.value);
			break;
		case EXP_OPER:
			result.type = DOUBLE_TYPE;
			result.value = exp(op1.value);
			break;
		case SQRT_OPER:
			result.type = DOUBLE_TYPE;
			result.value = sqrt(op1.value);
			break;
		case ADD_OPER:
			result.value = op1.value + op2.value;
			break;
		case SUB_OPER:
			result.value = op1.value - op2.value;
			break;
		case MULT_OPER:
			result.value = op1.value * op2.value;
			break;
		case DIV_OPER:
			result.value = op1.value / op2.value;
			break;
		case REMAINDER_OPER:
			result.type = DOUBLE_TYPE;
			result.value = fmod(op1.value, op2.value);
			break;
		case LOG_OPER:
			result.type = DOUBLE_TYPE;
			result.value = log(op1.value);
			break;
		case POW_OPER:
			result.value = pow(op1.value, op2.value);
			break;
		case MAX_OPER:
			result.value = op1.value > op2.value ? op1.value : op2.value;
			break;
		case MIN_OPER:
			result.value = op1.value < op2.value ? op1.value : op2.value;
			break;
		case EXP2_OPER:
			result.value = exp2(op1.value);
			break;
		case CBRT_OPER:
			result.type = DOUBLE_TYPE;
			result.value = cbrt(op1.value);
			break;
		case HYPOT_OPER:
			result.type = DOUBLE_TYPE;
			result.value = hypot(op1.value, op2.value);
			break;
		case READ_OPER:
			break;
		case RAND_OPER:
			break;
		case PRINT_OPER:
			break;
		case EQUAL_OPER:
			break;
		case LESS_OPER:
			break;
		case GREATER_OPER:
			break;
		case CUSTOM_OPER:
			break;
	}
	
    return result;
}

RET_VAL evalSymbolNode(AST_NODE *node)
{
	SYMBOL_TABLE_NODE *symbolTableNode = findSymbolTableNode(node->data.symbol.ident, node);

	RET_VAL result = (RET_VAL){INT_TYPE, NAN};

	if(symbolTableNode->value->type == FUNC_NODE_TYPE)
		result = evalFuncNode(symbolTableNode->value);
	if(symbolTableNode->value->type == NUM_NODE_TYPE)
		result = evalNumNode(symbolTableNode->value);

	return result;
}

SYMBOL_TABLE_NODE *findSymbolTableNode(char *ident, AST_NODE *node)
{
	if (node == NULL)
		return NULL;

	if(node->symbolTable == NULL)
		return findSymbolTableNode(ident, node->parent);

	SYMBOL_TABLE_NODE *temp = node->symbolTable;

	while(temp != NULL)
	{
		if(strcmp(ident, temp->ident) == 0)
			return temp;

		temp = temp->next;
	}

	return findSymbolTableNode(ident, node->parent);
}

void printRetVal(RET_VAL val)
{
	switch (val.type)
	{
		case INT_TYPE:
			printf("RET_VAL:\n\tType: INT_TYPE\n\tValue: %ld\n", (long) val.value);
			break;
		case DOUBLE_TYPE:
			printf("RET_VAL:\n\tType: DOUBLE_TYPE\n\tValue: %.4lf\n", val.value);
			break;
	}
}
