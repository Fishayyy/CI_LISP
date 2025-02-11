%{
    #include "ciLisp.h"
%}

%union {
    double dval;
    char *sval;
    struct ast_node *astNode;
    struct symbol_table_node *symNode;
}

%token <sval> FUNC SYMBOL TYPE
%token <dval> INT DOUBLE
%token LPAREN RPAREN LET COND LAMBDA EOL QUIT

%type <astNode> number f_expr s_expr s_expr_list
%type <symNode> let_section let_list let_elem arg_list

%%

program:
	s_expr EOL {
		fprintf(stderr, "yacc: program ::= s_expr EOL\n");
		if ($1) {
		    printRetVal(eval($1));
		    freeNode($1);
		}
	};

s_expr:
	number {
		fprintf(stderr, "yacc: s_expr ::= number\n");
		$$ = $1;
	}
	|
	SYMBOL {
		fprintf(stderr, "yacc: s_expr ::= symbol\n");
		$$ = createSymbolASTNode($1);
       	}
	|
	f_expr {
		$$ = $1;
	}
	|
	LPAREN let_section s_expr RPAREN {
		fprintf(stderr, "yacc: s_expr ::= LPAREN let_section s_expr RPAREN\n");
                $$ = setSymbolScope($2, $3);
	}
	|
	LPAREN COND s_expr s_expr s_expr RPAREN {
		fprintf(stderr, "yacc: s_expr ::= LPAREN COND s_expr s_expr s_expr RPAREN\n");
		$$ = createConditionNode($3,$4,$5);
	}
	|
	QUIT {
		fprintf(stderr, "yacc: s_expr ::= QUIT\n");
		exit(EXIT_SUCCESS);
        }
        |
	error {
		fprintf(stderr, "yacc: s_expr ::= error\n");
		yyerror("unexpected token");
		$$ = NULL;
	};

number:
	INT {
		fprintf(stderr, "yacc: number ::= INT\n");
		$$ = createNumberNode($1, INT_TYPE);
	}
	|
	DOUBLE {
		fprintf(stderr, "yacc: number ::= DOUBLE\n");
		$$ = createNumberNode($1, DOUBLE_TYPE);
	}
	|
	TYPE INT {
		fprintf(stderr, "yacc: number ::= TYPE INT\n");
                $$ = createNumberNode($2, castType($1));
	}
	|
	TYPE DOUBLE {
		fprintf(stderr, "yacc: number ::= TYPE DOUBLE\n");
                $$ = createNumberNode($2, castType($1));
	};

f_expr:
	LPAREN FUNC s_expr_list RPAREN {
                fprintf(stderr, "yacc: f_expr ::= LPAREN FUNC s_expr_list RPAREN\n");
                $$ = createFunctionNode($2, $3);
	}
	|
	LPAREN SYMBOL s_expr_list RPAREN {
		fprintf(stderr, "yacc: f_expr ::= LPAREN SYMBOL expr RPAREN\n");
                $$ = createFunctionNode($2, $3);
	};

s_expr_list:
	s_expr s_expr_list {
	 	fprintf(stderr, "yacc: s_expr_list ::= s_expr s_expr_list\n");
                $$ = createExpressionList($1, $2);
	}
	|
	s_expr {
	 	fprintf(stderr, "yacc: s_expr_list ::= s_expr\n");
                $$ = $1;
	}
	| /*empty*/ {
		$$ = NULL;
	};

let_section:
	LPAREN LET let_list RPAREN {
		fprintf(stderr, "yacc: let_section ::= LPAREN LET let_list RPAREN\n");
		$$ = $3;
	}
	|
	/*empty*/{
        	$$ = NULL;
        };

let_list:
	let_elem {
		fprintf(stderr, "yacc: let_list ::= let_elem\n");
                $$ = $1;
	}
	|
	let_list let_elem {
		fprintf(stderr, "yacc: let_list ::= let_list let_elem\n");
                $$ = createLetList($1, $2);
	};

let_elem:
	LPAREN SYMBOL s_expr RPAREN {
		fprintf(stderr, "yacc: let_elem ::= LPAREN SYMBOL s_expr RPAREN\n");
		$$ = createSymbolTableNode("",$2,$3);
	}
	|
	LPAREN TYPE SYMBOL s_expr RPAREN {
		fprintf(stderr, "yacc: let_elem ::= LPAREN TYPE SYMBOL s_expr RPAREN\n");
		$$ = createSymbolTableNode($2, $3, $4);
	}
        |
        LPAREN SYMBOL LAMBDA LPAREN arg_list RPAREN s_expr RPAREN {
            	fprintf(stderr, "yacc: let_elem ::= LPAREN SYMBOL LAMBDA LPAREN arg_list RPAREN s_expr RPAREN\n");
                $$ = createLambdaFunc("", $2, $5, $7);
        }
        |
	LPAREN TYPE SYMBOL LAMBDA LPAREN arg_list RPAREN s_expr RPAREN {
		fprintf(stderr, "yacc: let_elem ::= LPAREN type SYMBOL LAMBDA LPAREN arg_list RPAREN s_expr RPAREN\n");
		$$ = createLambdaFunc($2, $3, $6, $8);
	};

arg_list:
	SYMBOL arg_list {
		fprintf(stderr, "yacc: arg_list ::= SYMBOL arg_list\n");
		$$ = createArgList($1, $2);
	}
	|
	SYMBOL {
		fprintf(stderr, "yacc: arg_list ::= SYMBOL\n");
		$$ = createArgList($1, NULL);
	};

%%
