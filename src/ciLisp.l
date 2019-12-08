%option noyywrap
%option nounput
%option noinput

%{
    #include "ciLisp.h"
%}

digit [0-9]
int [+-]?{digit}+
double [+-]?{digit}+\.{digit}*
func "neg"|"abs"|"exp"|"sqrt"|"add"|"sub"|"mult"|"div"|"remainder"|"log"|"pow"|"max"|"min"
type "int"|"double"
symbol [a-zA-Z]+

%%

"quit" {
    return QUIT;
}

"(" {
    fprintf(stderr, "lex: LPAREN\n");
    return LPAREN;
}

")" {
    fprintf(stderr, "lex: RPAREN\n");
    return RPAREN;
}

"let" {
    fprintf(stderr, "lex: LET\n");
    return LET;
}

{int} {
    yylval.dval = strtod(yytext, NULL);
    fprintf(stderr, "lex: INT dval = %lf\n", yylval.dval);
    return INT;
}

{double} {
    yylval.dval = strtod(yytext, NULL);
    fprintf(stderr, "lex: DOUBLE dval = %lf\n", yylval.dval);
    return DOUBLE;
}

{func} {
    yylval.sval = (char *) malloc(strlen(yytext)*sizeof(char));
    strcpy(yylval.sval, yytext);
    fprintf(stderr, "lex: FUNC sval = %s\n", yylval.sval);
    return FUNC;
}

{type} {
    yylval.sval = (char *) malloc(strlen(yytext)*sizeof(char));
    strcpy(yylval.sval, yytext);
    fprintf(stderr, "lex: TYPE sval = %s\n", yylval.sval);
    return TYPE;
}

{symbol} {
    yylval.sval = (char *) malloc(strlen(yytext)*sizeof(char));
    strcpy(yylval.sval, yytext);
    fprintf(stderr, "lex: SYMBOL sval = %s\n", yylval.sval);
    return SYMBOL;
}

[\n] {
    fprintf(stderr, "lex: EOL\n");
    YY_FLUSH_BUFFER;
    return EOL;
}

[ |\t] ; /* skip whitespace */

. { // anything else
    printf("ERROR: invalid character: >>%s<<\n", yytext);
}

%%

/*
 * The code below should NOT be edited except to comment or uncomment
 * the "freopen" call in the main. When this call is commented, the fprintf
 * calls to stderr will appear in red in the console. When it is uncommented,
 * they will not appear in the console.
 */

#include <stdio.h>
#include <stdlib.h>

size_t readline(char **lineptr, size_t *n, FILE *stream, size_t n_terminate) {
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;

    if (lineptr == NULL) {
        return -1;
    }
    if (stream == NULL) {
        return -1;
    }
    if (n == NULL) {
        return -1;
    }
    bufptr = *lineptr;
    size = *n;

    c = fgetc(stream);
    if (c == EOF) {
        return -1;
    }
    if (bufptr == NULL) {
        bufptr = malloc(128);
        if (bufptr == NULL) {
            return -1;
        }
        size = 128;
    }
    p = bufptr;
    while(c != EOF) {
        if ((p - bufptr + 1 + n_terminate) > (size)) {
            int offset = p - bufptr;
            size = size + 128;
            bufptr = realloc(bufptr, size);
            if (bufptr == NULL) {
                return -1;
            }
            p = bufptr + offset;
        }
        *p++ = c;
        if (c == '\n') {
            break;
        }
        c = fgetc(stream);
    }

    while( n_terminate > 0)
    {
        *p++ = '\0';
        n_terminate--;
    }

    *n = p - bufptr;
    bufptr = realloc(bufptr, *n);
    *lineptr = bufptr;

    return p - bufptr;
}


int main(void) {

    freopen("/dev/null", "w", stderr);

    char *s_expr_str;
    size_t s_expr_str_len;
    size_t s_expr_postfix_padding = 2;
    YY_BUFFER_STATE buffer;
    while (true) {
        printf("\n> ");
        s_expr_str = NULL;
        s_expr_str_len = 0;
        readline(&s_expr_str, &s_expr_str_len, stdin, s_expr_postfix_padding);
        buffer = yy_scan_buffer(s_expr_str, s_expr_str_len);
        yyparse();
        yy_delete_buffer(buffer);
    }

    free(s_expr_str);
}