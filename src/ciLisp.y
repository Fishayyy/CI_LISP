%{
    #include "ciLisp.h"
%}

%union {
    double dval;
    char *sval;
    struct ast_node *astNode;
    struct symbol_table_node *symNode;
}

%token <sval> FUNC SYMBOL
%token <dval> INT DOUBLE
%token LPAREN RPAREN LET EOL QUIT

%type <astNode> number f_expr s_expr symbol
%type <symNode> let_section let_list let_elem

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
	symbol {
		fprintf(stderr, "yacc: s_expr ::= symbol\n");
		$$ = $1;
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
	QUIT {
        		fprintf(stderr, "yacc: s_expr ::= QUIT\n");
        		exit(EXIT_SUCCESS);
        }
	error {
		fprintf(stderr, "yacc: s_expr ::= error\n");
		yyerror("unexpected token");
		$$ = NULL;
	};

f_expr:
	LPAREN FUNC s_expr RPAREN {
		fprintf(stderr, "yacc: s_expr ::= LPAREN FUNC expr RPAREN\n");
		$$ = createFunctionNode($2, $3, NULL);
	}
	|
	LPAREN FUNC s_expr s_expr RPAREN {
		fprintf(stderr, "yacc: s_expr ::= LPAREN FUNC expr expr RPAREN\n");
		$$ = createFunctionNode($2, $3, $4);
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
	}

let_elem:
	LPAREN SYMBOL s_expr RPAREN {
		fprintf(stderr, "yacc: let_elem ::= LPAREN SYMBOL s_expr RPAREN\n");
		$$ = createSymbolTableNode($2,$3);
	}

symbol:
	SYMBOL {
		fprintf(stderr, "yacc: s_expr ::= symbol\n");
                $$ = createSymbolASTNode($1);
	}

number:
	INT {
		fprintf(stderr, "yacc: number ::= INT\n");
		$$ = createNumberNode($1, INT_TYPE);
	}
	|
	DOUBLE {
		fprintf(stderr, "yacc: number ::= DOUBLE\n");
		$$ = createNumberNode($1, DOUBLE_TYPE);
	};
%%

