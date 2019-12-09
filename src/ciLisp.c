#include "ciLisp.h"

void yyerror(char *s) {
    fprintf(stderr, "ERROR: %s\n", s);
}

void printWarning(char *s) {
	fprintf(stdout, "WARNING: %s\n", s);
}

char *funcNames[] = {
		"neg",
		"abs",
		"add",
		"sub",
		"mult",
		"div",
		"remainder",
		"exp",
		"exp2",
		"pow",
		"log",
		"sqrt",
		"cbrt",
		"hypot",
		"max",
		"min",
		"print",
		"read",
		"rand",
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

NUM_TYPE castType(char *type)
{
	NUM_TYPE numType = UNSPECIFIED_TYPE;
	
	if(strcmp("int", type) == 0)
		numType = INT_TYPE;
	if(strcmp("double", type) == 0)
		numType = DOUBLE_TYPE;

	return numType;
}

AST_NODE *createASTNode()
{
	AST_NODE *node;
	size_t nodeSize;
	
	nodeSize = sizeof(AST_NODE);
	if ((node = calloc(nodeSize, 1)) == NULL)
		yyerror("Memory allocation failed!");
	
	return node;
}

AST_NODE *createNumberNode(double value, NUM_TYPE type)
{
    AST_NODE *node = createASTNode();

	node->type = NUM_NODE_TYPE;
	node->symbolTable = NULL;
	node->parent = NULL;
	node->data.number.type = type;
	node->data.number.value = value;
	node->next = NULL;
    
    return node;
}

AST_NODE *createFunctionNode(char *funcName, AST_NODE *op_list)
{
	AST_NODE *node = createASTNode();
    
	node->type = FUNC_NODE_TYPE;
	node->symbolTable = NULL;
	node->parent = NULL;
	node->data.function.oper = resolveFunc(funcName);
	node->data.function.op_list = op_list;
	node->next = NULL;
	
	if(node->data.function.oper == CUSTOM_OPER)
		node->data.function.ident = funcName;
	else
		free(funcName);
	
	AST_NODE *temp = op_list;
	
	while(temp != NULL)
	{
		temp->parent = node;
		temp = temp->next;
	}

    return node;
}

AST_NODE *createExpressionList(AST_NODE *s_expr, AST_NODE *s_expr_list)
{
	s_expr->next = s_expr_list;
	return s_expr;
}

AST_NODE *createSymbolASTNode(char *ident)
{
	AST_NODE *node = createASTNode();
	
	node->type = SYMBOL_NODE_TYPE;
	node->symbolTable = NULL;
	node->parent = NULL;
	node->data.symbol.ident = ident;
	node->next = NULL;
	
	return node;
}

AST_NODE *createConditionNode(AST_NODE *condition, AST_NODE *trueExpr, AST_NODE *falseExpr)
{
	AST_NODE *node = createASTNode();
	
	node->type = COND_NODE_TYPE;
	node->symbolTable = NULL;
	node->parent = NULL;
	condition->parent = node;
	node->data.condition.cond = condition;
	trueExpr->parent = node;
	node->data.condition.trueExpr = trueExpr;
	falseExpr->parent = node;
	node->data.condition.falseExpr = falseExpr;
	node->next = NULL;
	return node;
}

SYMBOL_TABLE_NODE *createSymbolTableNode(char *type, char *ident, AST_NODE *value)
{
	SYMBOL_TABLE_NODE *node;
	size_t nodeSize;
	
	nodeSize = sizeof(SYMBOL_TABLE_NODE);
	if ((node = calloc(nodeSize, 1)) == NULL)
		yyerror("Memory allocation failed!");
	
	node->val_type = castType(type);
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
	    freeNode(node->data.function.op_list);

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
    	case COND_NODE_TYPE:
    		result = evalConditionNode(node);
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
    RET_VAL result = (RET_VAL){INT_TYPE, 0};

	AST_NODE *temp = node->data.function.op_list;
	RET_VAL operand[BUFF_SIZE];
	int numOps = 0;
	
	while(temp != NULL)
	{
		operand[numOps] = eval(temp);
		
		if(operand[numOps++].type == DOUBLE_TYPE)
			result.type = DOUBLE_TYPE;
		
		temp = temp->next;
	}
	
	if(numOps > 0)
	{
		switch(node->data.function.oper)
		{
			case NEG_OPER:
				if(numOps > 1)
					printWarning("neg called with extra (ignored) operands!");
				result.type = operand[0].type;
				result.value = -operand[0].value;
				break;
			case ABS_OPER:
				if(numOps > 1)
					printWarning("abs called with extra (ignored) operands!");
				result.type = operand[0].type;
				result.value = fabs(operand[0].value);
				break;
			case EXP_OPER:
				if(numOps > 1)
					printWarning("exp called with extra (ignored) operands!");
				result.type = DOUBLE_TYPE;
				result.value = exp(operand[0].value);
				break;
			case SQRT_OPER:
				if(numOps > 1)
					printWarning("sqrt called with extra (ignored) operands!");
				result.type = DOUBLE_TYPE;
				result.value = sqrt(operand[0].value);
				break;
			case ADD_OPER:
				for(int i = 0; i < numOps; ++i)
					result.value += operand[i].value;
				break;
			case SUB_OPER:
				if(numOps == 1)
				{
					result.type = INT_TYPE;
					result.value = NAN;
					yyerror("sub called with only one arg!");
				}
				else
				{
					if(numOps > 2)
					{
						printWarning("sub called with extra (ignored) operands!");
						result.type = (operand[0].type == INT_TYPE && operand[1].type == INT_TYPE) ? INT_TYPE : DOUBLE_TYPE;
					}
					result.value = operand[0].value - operand[1].value;
				}
				break;
			case MULT_OPER:
				result.value = operand[0].value;
				for(int i = 1; i < numOps; ++i)
					result.value = result.value * operand[i].value;
				break;
			case DIV_OPER:
				if(numOps == 1)
				{
					result.type = INT_TYPE;
					result.value = NAN;
					yyerror("div called with only one arg!");
				}
				else
				{
					if (operand[1].value == 0)
					{
						result.type = INT_TYPE;
						result.value = NAN;
						yyerror("cannot divide by 0!");
					}
					else
					{
						if (numOps > 2)
						{
							printWarning("div called with extra (ignored) operands!");
							result.type = (operand[0].type == INT_TYPE && operand[1].type == INT_TYPE) ? INT_TYPE : DOUBLE_TYPE;
						}
						result.value = (result.type == INT_TYPE) ? floor(operand[0].value / operand[1].value) : operand[0].value / operand[1].value;
					}
				}
				break;
			case REMAINDER_OPER:
				if(numOps == 1)
				{
					result.type = INT_TYPE;
					result.value = NAN;
					yyerror("remainder called with only one arg!");
				}
				else
				{
					if (operand[1].value == 0)
					{
						result.type = INT_TYPE;
						result.value = NAN;
						yyerror("cannot divide by 0!");
					}
					else
					{
						if (numOps > 2)
						{
							printWarning("remainder called with extra (ignored) operands!");
							result.type = (operand[0].type == INT_TYPE && operand[1].type == INT_TYPE) ? INT_TYPE : DOUBLE_TYPE;
						}
						result.value = fabs(fmod(operand[0].value, operand[1].value));
					}
				}
				break;
			case LOG_OPER:
				if(numOps > 1)
					printWarning("log called with extra (ignored) operands!");
				result.type = DOUBLE_TYPE;
				result.value = log(operand[0].value);
				break;
			case POW_OPER:
				if(numOps == 1)
				{
					result.type = INT_TYPE;
					result.value = NAN;
					yyerror("pow called with only one arg!");
				}
				else
				{
					if(numOps > 2)
					{
						printWarning("pow called with extra (ignored) operands!");
						result.type = (operand[0].type == INT_TYPE && operand[1].type == INT_TYPE) ? INT_TYPE : DOUBLE_TYPE;
					}
					result.value = pow(operand[0].value, operand[1].value);
				}
				break;
			case MAX_OPER:
			{
				double max = operand[0].value;
				result.type = operand[0].type;
				for(int i = 1; i < numOps; ++i)
					if(max < operand[i].value)
					{
						max = operand[i].value;
						result.type = operand[i].type;
					}
				result.value = max;
				break;
			}
			case MIN_OPER:
			{
				double min = operand[0].value;
				result.type = operand[0].type;
				for(int i = 1; i < numOps; ++i)
					if(min > operand[i].value)
					{
						min = operand[i].value;
						result.type = operand[i].type;
					}
				result.value = min;
				break;
			}
			case EXP2_OPER:
				if(numOps > 1)
					printWarning("exp2 called with extra (ignored) operands!");
				result.type = operand[0].type;
				result.value = exp2(operand[0].value);
				break;
			case CBRT_OPER:
				if(numOps > 1)
					printWarning("cbrt called with extra (ignored) operands!");
				result.type = DOUBLE_TYPE;
				result.value = cbrt(operand[0].value);
				break;
			case HYPOT_OPER:
			{
				double sum = 0;
				for(int i = 0; i < numOps; ++i)
					sum += pow(operand[i].value, 2);
				
				result.type = DOUBLE_TYPE;
				result.value = sqrt(sum);
				break;
			}
			case PRINT_OPER:
				for(int i = 0; i < numOps; ++i)
				{
					switch(operand[i].type)
					{
						case INT_TYPE:
							printf("Print:\n\tInteger: %ld\n", (long) operand[i].value);
							break;
						case DOUBLE_TYPE:
							printf("Print:\n\tDouble: %lf\n", operand[i].value);
							break;
						default:
							break;
					}
				}
				result.type = operand[numOps -1].type;
				result.value = operand[numOps - 1].value;
				break;
			case EQUAL_OPER:
				if(numOps == 1)
				{
					result.type = INT_TYPE;
					result.value = NAN;
					yyerror("equal called with only one arg!");
				}
				else
				{
					if(numOps > 2)
					{
						printWarning("equal called with extra (ignored) operands!");
						result.type = (operand[0].type == INT_TYPE && operand[1].type == INT_TYPE) ? INT_TYPE : DOUBLE_TYPE;
					}
					result.value = (operand[0].value == operand[1].value) ? 1 : 0;
				}
				break;
			case LESS_OPER:
				if(numOps == 1)
				{
					result.type = INT_TYPE;
					result.value = NAN;
					yyerror("less called with only one arg!");
				}
				else
				{
					if(numOps > 2)
					{
						printWarning("less called with extra (ignored) operands!");
						result.type = (operand[0].type == INT_TYPE && operand[1].type == INT_TYPE) ? INT_TYPE : DOUBLE_TYPE;
					}
					result.value = (operand[0].value < operand[1].value) ? 1 : 0;
				}
				break;
			case GREATER_OPER:
				if(numOps == 1)
				{
					result.type = INT_TYPE;
					result.value = NAN;
					yyerror("greater called with only one arg!");
				}
				else
				{
					if(numOps > 2)
					{
						printWarning("greater called with extra (ignored) operands!");
						result.type = (operand[0].type == INT_TYPE && operand[1].type == INT_TYPE) ? INT_TYPE : DOUBLE_TYPE;
					}
					result.value = (operand[0].value > operand[1].value) ? 1 : 0;
				}
				break;
			case CUSTOM_OPER:
				
				break;
			default:
				break;
		}// end switch(...)
	}// end if(...)
	else
	{
		char line_buff[BUFF_SIZE] = {"too few parameters for the function "};
		switch(node->data.function.oper)
		{
			case READ_OPER:
			{
				printf("read := ");
				fgets(line_buff, sizeof(line_buff), stdin);
				
				if (strchr(line_buff, '.'))
					result.type = DOUBLE_TYPE;
	
				result.value = strtod(line_buff,NULL);
				node->type = NUM_NODE_TYPE;
				node->data.number = result;
				break;
			}
			case RAND_OPER:
				result.type = DOUBLE_TYPE;
				result.value =  (double) (random() / RAND_MAX);
				node->type = NUM_NODE_TYPE;
				node->data.number = result;
				break;
			case ADD_OPER:
				printWarning("add call with no operands, 0 returned!");
				break;
			case MULT_OPER:
				printWarning("mult call with no operands, 1 returned!");
				result.value = 1;
				break;
			case HYPOT_OPER:
				printWarning("hypot call with no operands, 0.0 returned!");
				result.type = DOUBLE_TYPE;
				result.value = 0.0;
				break;
			default:
				strcat(line_buff, funcNames[node->data.function.oper]);
				result.value = NAN;
				yyerror(line_buff);
		}
	}// end else

	
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
	
	if(symbolTableNode->val_type == INT_TYPE && result.type == DOUBLE_TYPE)
	{
		char line_buff[BUFF_SIZE] = "precision loss in the assignment for variable ";
		strcat(line_buff, symbolTableNode->ident);
		printWarning(line_buff);
		symbolTableNode->value->data.number.value = floor(symbolTableNode->value->data.number.value);
		result.value = symbolTableNode->value->data.number.value;
		result.type = INT_TYPE;
	}
	if(symbolTableNode->val_type == DOUBLE_TYPE && result.type == INT_TYPE)
		result.type = DOUBLE_TYPE;
	
	return result;
}

RET_VAL evalConditionNode(AST_NODE *node)
{
	return (RET_VAL){INT_TYPE,NAN};
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
			if(isnan(val.value))
				printf("Integer: NAN\n");
			else
				printf("Integer: %ld\n", (long) val.value);
			break;
		case DOUBLE_TYPE:
			printf("Double: %lf\n", val.value);
			break;
		default:
			yyerror("Invalid NUM_TYPE, probably invalid writes somewhere!");
	}
}
