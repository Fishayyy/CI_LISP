/**
* Name: Austin Fisher
* Lab: CILisp
* Date: 12/11/19
**/
#include "ciLisp.h"

void yyerror(char *s) {
	fprintf(stdout, "ERROR: %s\n", s);
}

void freeNode(AST_NODE *node) {
	if (NULL == node)
		return;
	
	switch (node->type) {
		case FUNC_NODE_TYPE:
			freeNode(node->data.function.op_list->next);
			if(!node->data.function.ident)
				free(node->data.function.ident);
			freeNode(node->data.function.op_list);
			break;
		case COND_NODE_TYPE:
			freeNode(node->data.condition.condition);
			freeNode(node->data.condition.trueExpr);
			freeNode(node->data.condition.falseExpr);
			break;
		case SYMBOL_NODE_TYPE:
			free(node->data.symbol.ident);
			break;
		default:
			break;
	}
	
	free(node);
}


void printWarning(char *s) {
	fprintf(stdout, "WARNING: %s\n", s);
}

char *funcNames[] = {
		//UNARY
		"neg",
		"abs",
		"exp",
		"exp2",
		"log",
		"sqrt",
		"cbrt",
		//BINARY
		"sub",
		"div",
		"remainder",
		"pow",
		"equal",
		"less",
		"greater",
		//N-ARY
		"add",
		"mult",
		"hypot",
		"max",
		"min",
		//NO-OPERANDS
		"print",
		"read",
		"rand",
		//UNKNOWN
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

AST_NODE *initASTNode()
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
	AST_NODE *node = initASTNode();
	
	node->type = NUM_NODE_TYPE;
	node->symbolTable = NULL;
	node->parent = NULL;
	node->data.number.type = type;
	node->data.number.value = (type == INT_TYPE) ? floor(value) : value;
	node->next = NULL;
	
	return node;
}

AST_NODE *createFunctionNode(char *funcName, AST_NODE *op_list)
{
	AST_NODE *node = initASTNode();
	
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
	AST_NODE *node = initASTNode();
	
	node->type = SYMBOL_NODE_TYPE;
	node->symbolTable = NULL;
	node->parent = NULL;
	node->data.symbol.ident = ident;
	node->next = NULL;
	
	return node;
}

AST_NODE *createConditionNode(AST_NODE *condition, AST_NODE *trueExpr, AST_NODE *falseExpr)
{
	AST_NODE *node = initASTNode();
	
	node->type = COND_NODE_TYPE;
	node->symbolTable = NULL;
	node->parent = NULL;
	condition->parent = node;
	node->data.condition.condition = condition;
	trueExpr->parent = node;
	node->data.condition.trueExpr = trueExpr;
	falseExpr->parent = node;
	node->data.condition.falseExpr = falseExpr;
	node->next = NULL;
	return node;
}

SYMBOL_TABLE_NODE *initSymbolTableNode()
{
	SYMBOL_TABLE_NODE *node;
	size_t nodeSize;
	
	nodeSize = sizeof(SYMBOL_TABLE_NODE);
	if ((node = calloc(nodeSize, 1)) == NULL)
		yyerror("Memory allocation failed!");
	
	return node;
}

SYMBOL_TABLE_NODE *createSymbolTableNode(char *type, char *ident, AST_NODE *value)
{
	SYMBOL_TABLE_NODE *node = initSymbolTableNode();
	
	node->type = VARIABLE_TYPE;
	node->val_type = castType(type);
	node->ident = ident;
	node->value = value;
	node->stack = NULL;
	node->next = NULL;
	
	return node;
}

SYMBOL_TABLE_NODE *createArgList(char *argument, SYMBOL_TABLE_NODE *arg_list)
{
	SYMBOL_TABLE_NODE *node = initSymbolTableNode();
	
	node->type = ARG_TYPE;
	node->val_type = UNSPECIFIED_TYPE;
	node->ident = argument;
	node->value = NULL;
	node->stack = NULL;
	node->next = arg_list;
	
	return node;
}

SYMBOL_TABLE_NODE *createLambdaFunc(char *type, char *ident, SYMBOL_TABLE_NODE *argList, AST_NODE *function)
{
	SYMBOL_TABLE_NODE *node = initSymbolTableNode();
	
	node->type = LAMBDA_TYPE;
	node->val_type = castType(type);
	node->ident = ident;
	node->value = function;
	node->value->symbolTable = argList;
	node->stack = NULL;
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

RET_VAL eval(AST_NODE *node)
{
	RET_VAL result = DEFAULT_RET_VAL;
	
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
			break;
		case COND_NODE_TYPE:
			result = evalConditionNode(node);
			break;
		default:
			yyerror("Invalid AST_NODE_TYPE, probably invalid writes somewhere!");
	}
	return result;
}

RET_VAL evalNumNode(AST_NODE *node)
{
	RET_VAL result = DEFAULT_RET_VAL;
	
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
	

	while (temp != NULL)
	{
		operand[numOps] = eval(temp);
		
		if (operand[numOps++].type == DOUBLE_TYPE)
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
							printf("Print ::= Integer: %ld\n", (long) operand[i].value);
							break;
						case DOUBLE_TYPE:
							printf("Print ::= Double: %lf\n", operand[i].value);
							break;
						default:
							break;
					}
				}
				result.type = operand[numOps -1].type;
				result.value = operand[numOps - 1].value;
				break;
			case READ_OPER:
				result.value = NAN;
				yyerror("read called with operands");
				break;
			case RAND_OPER:
				result.value = NAN;
				yyerror("rand called with operands");
				break;
			case EQUAL_OPER:
				result.type = INT_TYPE;
				if(numOps == 1)
				{
					result.value = NAN;
					yyerror("equal called with only one arg!");
				}
				else
				{
					if(numOps > 2)
						printWarning("equal called with extra (ignored) operands!");
					
					result.value = (operand[0].value == operand[1].value) ? 1 : 0;
				}
				break;
			case LESS_OPER:
				result.type = INT_TYPE;
				if(numOps == 1)
				{
					result.value = NAN;
					yyerror("less called with only one arg!");
				}
				else
				{
					if(numOps > 2)
						printWarning("less called with extra (ignored) operands!");
					
					result.value = (operand[0].value < operand[1].value) ? 1 : 0;
				}
				break;
			case GREATER_OPER:
				result.type = INT_TYPE;
				if(numOps == 1)
				{
					result.value = NAN;
					yyerror("greater called with only one arg!");
				}
				else
				{
					if(numOps > 2)
						printWarning("greater called with extra (ignored) operands!");
					
					result.value = (operand[0].value > operand[1].value) ? 1 : 0;
				}
				break;
			case CUSTOM_OPER:
				result = evalCustomFunction(node);
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
				result.value =  (double) rand() / RAND_MAX;
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
			case CUSTOM_OPER:
				result = evalCustomFunction(node);
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
	
	RET_VAL result = DEFAULT_RET_VAL;
	
	if (symbolTableNode == NULL)
	{
		char line_buff[BUFF_SIZE] = "Use of undefined let_elem ";
		strcat(line_buff, node->data.symbol.ident);
		yyerror(line_buff);
		return result;
	}
			
	if (symbolTableNode->value->type == FUNC_NODE_TYPE)
		result = evalFuncNode(symbolTableNode->value);
	else if (symbolTableNode->value->type == NUM_NODE_TYPE)
		result = evalNumNode(symbolTableNode->value);
	else if (symbolTableNode->value->type == COND_NODE_TYPE)
		result = evalConditionNode(symbolTableNode->value);
	
	if (symbolTableNode->val_type == INT_TYPE && result.type == DOUBLE_TYPE)
	{
		char line_buff[BUFF_SIZE] = "precision loss in the assignment for variable ";
		strcat(line_buff, symbolTableNode->ident);
		printWarning(line_buff);
		symbolTableNode->value->data.number.value = floor(symbolTableNode->value->data.number.value);
		result.value = symbolTableNode->value->data.number.value;
		result.type = INT_TYPE;
	}
	if (symbolTableNode->val_type == DOUBLE_TYPE && result.type == INT_TYPE)
		result.type = DOUBLE_TYPE;
	
	return result;
}

RET_VAL evalConditionNode(AST_NODE *node)
{
	return (eval(node->data.condition.condition).value == 1) ? eval(node->data.condition.trueExpr) : eval(node->data.condition.falseExpr);
}

RET_VAL evalCustomFunction(AST_NODE *node)
{
	RET_VAL result = DEFAULT_RET_VAL;
	
	if (!node)
		return result;
	
	SYMBOL_TABLE_NODE *lambdaST;
	AST_NODE *lambdaFunc = node;
	
	while (lambdaFunc)
	{
		lambdaST = lambdaFunc->symbolTable;
		while (lambdaST)
		{
			if (strcmp(lambdaST->ident, node->data.function.ident) == 0 && (lambdaST->type == LAMBDA_TYPE))
			{
				lambdaFunc = lambdaST->value;
				STACK_NODE *argValues = createStackNodes(lambdaFunc, node->data.function.op_list);
				
				if (!argValues)
					return result;
				
				linkStackNodes(lambdaFunc->symbolTable, argValues);
				
				RET_VAL result = eval(lambdaFunc);
				result.type = lambdaST->val_type;
				result.value = (result.type == INT_TYPE) ? floor(result.value) : result.value;
				return result;
			}
			lambdaST = lambdaST->next;
		}
		lambdaFunc = lambdaFunc->parent;
	}
	
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

void linkStackNodes(SYMBOL_TABLE_NODE *arguments, STACK_NODE *parameters)
{
	SYMBOL_TABLE_NODE *currArg = arguments;
	STACK_NODE *currStackNode = parameters;
	STACK_NODE *prevStackNode;
	
	while ((currArg != NULL))
	{
		prevStackNode = currStackNode;
		
		currArg->value = currStackNode->value;
		
		currArg = currArg->next;
		currStackNode = currStackNode->next;
		free(prevStackNode);
	}
}

STACK_NODE *createStackNodes(AST_NODE *lambdaFunc, AST_NODE *paramList)
{
	if(!paramList)
	{
		yyerror("Parameter List is empty.\n");
		return NULL;
	}
	
	if (!lambdaFunc)
	{
		yyerror("Invalid lambda function.\n");
		return NULL;
	}
	
	STACK_NODE *top;
	size_t nodeSize;
	
	nodeSize = sizeof(STACK_NODE);
	if ((top = calloc(nodeSize, 1)) == NULL)
		yyerror("Memory allocation failed!");
	
	RET_VAL retVal = eval(paramList);
	top->value =  createNumberNode(retVal.value, retVal.type);
	
	SYMBOL_TABLE_NODE *currArg = lambdaFunc->symbolTable->next;
	AST_NODE *currOp = paramList->next;
	
	STACK_NODE *currStackFrame = top;
	
	while ((currArg != NULL) && (currOp != NULL))
	{
		
		if ((currStackFrame->next = calloc(nodeSize, 1)) == NULL)
			yyerror("Memory allocation failed!");
		
		currStackFrame = currStackFrame->next;
		currStackFrame->next = NULL;
		retVal = eval(currOp);
		currStackFrame->value = createNumberNode(retVal.value, retVal.type);
		
		currArg = currArg->next;
		currOp = currOp->next;
	}
	
	if ((currArg == NULL) && (currOp != NULL))
		printWarning("lambda function called with extra (ignored) operands!");
	else if (currArg != NULL)
	{
		yyerror("Too few parameters for lambda function.");
		while (currArg != NULL)
		{
			if ((currStackFrame->next = calloc(nodeSize, 1)) == NULL)
				yyerror("Memory allocation failed!");
			
			currStackFrame = currStackFrame->next;
			currStackFrame->next = NULL;
			currStackFrame->value = createNumberNode(NAN,INT_TYPE);
			
			currArg = currArg->next;
		}
	}
	return top;
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