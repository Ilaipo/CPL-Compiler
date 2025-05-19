%option noyywrap
%option yylineno
%option noinput
%option nounput
%option never-interactive

digit [0-9]
letter [a-zA-Z]
id {letter}({letter}|{digit})*
int {digit}+
float {digit}+\.{digit}*
ws [ \n\t\r]+
comment "/*".*"*/"
op =|"||"|&&|!
punc [\(\)\{\}\n,:;]
rel ==|!=|<|>|<=|>=
cast static_cast<int>|static_cast<float>

%{
#include "../lib/symbols.h"
#include "../lib/triples.h"

enum Operator relfun(const char *yytext);
char castfun(const char *yytext);
void yyerror(const char *s);

#define cplerror(...) do { char *data;\
                        if (asprintf(&data, __VA_ARGS__) != -1) {\
                            yyerror(data);\
                            free(data);\
                        }} while(0)


/* error flag, for disabling output */
char fronterror;

%}

%%

{ws}
{comment}
"if"        return IF;
"else"      return ELSE;
"while"     return WHILE;
"int"       return TOK_INT;
"float"     return TOK_FLOAT;
"input"     return TOK_INPUT;
"output"    return TOK_OUTPUT;
{op}|{punc} return yytext[0];
"+"|"-"     {   yylval.fn = (yytext[0] == '-' ? MIN : PLUS);
                return ADDOP; }
"*"|"/"     {   yylval.fn = (yytext[0] == '/' ? DIV : MUL);
                return MULOP; }
{rel}       { yylval.fn = relfun(yytext); return RELOP; }
{cast}      { yylval.fn = castfun(yytext); return TOK_CAST; }
{int}       { yylval.ival = atoi(yytext); return TOK_ICONST; } 
{float}     { yylval.fval = atof(yytext); return TOK_FCONST; }
{id}        { yylval.idval = strdup(yytext); return ID; }
.           { int isprint(int c);
                cplerror(isprint(yytext[0]) ?
                    "Unrecognized character '%c'" :
                    "Unrecognized character ascii %d", yytext[0]); }

%%

enum Operator relfun(const char *yytext) {
    if (strcmp(yytext, "==") == 0)
        return EQ;
    if (strcmp(yytext, "!=") == 0)
        return NEQ;
    if (strcmp(yytext, "<") == 0)
        return LT;
    if (strcmp(yytext, ">") == 0)
        return GT;
    if (strcmp(yytext, ">=") == 0)
        return GE;
    if (strcmp(yytext, "<=") == 0)
        return LE;
    return EQ;
}

char castfun(const char *yytext) {
    if (strcmp(yytext, "static_cast<int>") == 0)
        return 0;
    if (strcmp(yytext, "static_cast<float>") == 0)
        return 1;
    return 0;
}

void yyerror(const char *s) {
    fprintf(stderr, "Error on line %d: %s\n", yylineno, s);
    fronterror = 1;
}

