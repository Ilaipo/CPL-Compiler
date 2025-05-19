%{
#define _GNU_SOURCE
#include "../lib/triples.h"
#include "../lib/patches.h"
#include "../lib/symbols.h"
%}

%define parse.error verbose

%union {
    /* terminal values */
    int ival;
    float fval;
    char fn;
    char *idval;
    /* nonterminal values */
    enum Type type;
    PatchList nextlist;
    unsigned instr;
    PatchList bl[2];
    struct Entry *symbolList;
    struct expData {
        enum Type type;
        struct Argument data;
    } expData;
}

/* token names, to be returned by yylex */
%token IF ELSE WHILE TOK_INT TOK_FLOAT TOK_INPUT TOK_OUTPUT
%token <ival> TOK_ICONST
%token <fval> TOK_FCONST
%token <fn> RELOP TOK_CAST ADDOP MULOP
%token <idval> ID

%type <bl> boolexpr boolterm boolfactor
%type <expData> expression term factor
%type <instr> M
%type <nextlist> stmt stmtlist N
%type <type> type
%type <symbolList> idlist

%destructor { free($$); } <idval>
%destructor { deletelist($$); } <nextlist>
%destructor { deletelist($$[0]); deletelist($$[1]); } <bl>
%destructor { delSymList($$); } <symbolList>

%{
#include "cpl.yy.c"

void typeConversion(struct expData *e1, struct expData *e2);
%}

%%

program: { pushEnv(); } declarations '{' stmtlist '}'
    {   popEnv();
        /* backpatch(L.nextlist, nextinstr); */
        backpatch($4, nextinstr); } 
    ;
declarations: declarations declaration
    | /* empty */
    | error
    ;
declaration: idlist ':' type ';'
    {   /* foreach in idlist insert with type */
        struct Entry *node;
        for (node = $1 ; node != NULL; node = node->next)
            if (insert(strdup(node->s.name), $3))
                cplerror("Symbol '%s' redeclared", node->s.name);
        delSymList($1); }
    ;
type: TOK_INT { $$ = INT; }
    | TOK_FLOAT { $$ = FLOAT; }
    ;
idlist: idlist ',' ID
    {   /* idlist1.idlist = merge(idlist2.idlist, ID.name) */
        $$ = add($1, $3); }
    | ID
    {   /* idlist.idlist = {ID.name} */
        $$ = add(NULL, $1); }
    ;
stmt: assignment_stmt   { $$ = emptyList; /* S.nextlist = {} */ }
    | input_stmt        { $$ = emptyList; }
    | output_stmt       { $$ = emptyList; }
    | IF '(' boolexpr ')' M stmt N ELSE M stmt
    {   /* backpatch(B.truelist, M1.instr) */
        backpatch($3[0], $5);
        /* backpatch(B.falselist, M2.instr) */
        backpatch($3[1], $9);
        /* temp = merge(S1.nextlist, N.nextlist) */
        PatchList temp = merge($6, $7);
        /* S.nextlist = merge(temp, S2.nextlist) */
        $$ = merge(temp, $10); }
    | WHILE M '(' boolexpr ')' M stmt
    {   /* backpatch(S1.nextlist, M1.instr) */
        backpatch($7, $2);
        /* backpatch(B.truelist, M2.instr) */
        backpatch($4[0], $6);
        /* S.nextlist = B.falselist */
        $$ = $4[1];
        /* gen('goto' M1.instr) */
        emit(makeTriple(JMP, INT, makeJUMP($2), EMPTY)); }
    | '{' stmtlist '}'
    {   /* S.nextlist = L.nextlist */
        $$ = $2; }
    | error ';'     { yyerrok; $$ = emptyList; }
    ;
assignment_stmt: ID '=' expression ';'
    {   Symbol *s;
        struct expData idExp;
        if ((s = lookup($1)) == NULL)
            cplerror("Identifier '%s' not declared", $1);
        else if (s->type == INT && $3.type == FLOAT)
            cplerror("Invalid assignment of float expression to integer identifier '%s'", s->name);
        else {
            idExp.type = s->type;
            /* will convert to 'idExp's type */
            typeConversion(&idExp, &$3);
            emit(makeTriple(ASSIGN, $3.type, 
            makeSYMID(s->ID), $3.data));
        }
        free($1);
    }
    ;
input_stmt: TOK_INPUT '(' ID ')' ';'
    {   Symbol *s;
        if ((s = lookup($3)) == NULL)
            cplerror("Identifier '%s' not declared", $3);
        else
            emit(makeTriple(INPUT,s->type,makeSYMID(s->ID),EMPTY));
        free($3); }
    ;
output_stmt: TOK_OUTPUT '(' expression ')' ';'
    {   emit(makeTriple(OUTPUT, $3.type, $3.data, EMPTY)); }
    ;
stmtlist: stmtlist M stmt
    {   /* backpatch(L1.nextlist, M.instr) */
        backpatch($1, $2);
        /* L.nextlist = S.nextlist */
        $$ = $3; }
    |   /* empty */
    {   $$ = emptyList;  }
    ;

M: /* empty */ 
    {   /* M.instr = nextinstr */
        $$ = nextinstr; }
 ;
N: /* empty */ 
    {   /* N.nextlist = makelist(nextinstr) */
        $$ = makelist(nextinstr);
        /* gen('goto _') */
        emit(makeTriple(JMP, INT, EMPTY, EMPTY)); }
 ;
boolexpr: boolexpr '|' M boolterm
    {   /* backpatch(B1.falselist, M.instr) */
        backpatch($1[1], $3);
        /* B.truelist = merge(B1.truelist, B2.truelist) */
        $$[0] = merge($1[0], $4[0]);
        /* B.falselist = B2.falselist */
        $$[1] = $4[1]; }
    | boolterm
    ;
boolterm: boolterm '&' M boolfactor
    {   /* backpatch(B1.truelist, M.instr) */
        backpatch($1[0], $3);
        /* B.truelist = B2.truelist */
        $$[0] = $4[0];
        /* B.falselist = merge(B1.falselist, B2.falselist) */
        $$[1] = merge($1[1], $4[1]); }
    | boolfactor
    ;
boolfactor: '!' '(' boolexpr ')'
    {   /* B.truelist = B1.falselist */
        $$[0] = $3[1];
        /* B.falselist = B1.truelist */
        $$[1] = $3[0]; }
    | expression RELOP expression
    {   typeConversion(&$1, &$3);
        /* emit (E1 rel E2) */
        emit(makeTriple(($2 == GE ? LT : ($2 == LE ? GT : $2)), 
                        $1.type, $1.data, $3.data));

        /* GE and LE are achieved by using algebraic rules */
        $$[($2 == GE || $2 == LE)] = makelist(nextinstr + 1);
        $$[!($2 == GE || $2 == LE)] = makelist(nextinstr);

        /* emit (jmpfalse _) refer to comparison instruction */
        emit(makeTriple(JMPFALSE, INT, 
                        EMPTY, makeLINENO(nextinstr-1)));
        /* emit (jmp _) */
        emit(makeTriple(JMP, INT, EMPTY, EMPTY)); }
    ;
expression: expression ADDOP term
    {   typeConversion(&$1, &$3);
        $$.type = $1.type; /* result type is type of both operands */
        $$.data = makeLINENO(nextinstr);
        emit(makeTriple($2, $1.type, $1.data, $3.data)); }
    | term
    ;
term: term MULOP factor
    {   typeConversion(&$1, &$3);
        $$.type = $1.type; /* result type is type of both operands */
        $$.data = makeLINENO(nextinstr);
        emit(makeTriple($2, $1.type, $1.data, $3.data)); }
    | factor
    ;
factor: '(' expression ')' { $$ = $2; }
    | TOK_CAST '(' expression ')'
    {   $$.type = ($1 == 0 ? INT : FLOAT);
        if ($$.type != $3.type) {
            $$.data = makeLINENO(nextinstr);
            emit(makeTriple(CAST, $3.type, $3.data, EMPTY));
        }
        else
            $$.data = $3.data;
    }
    | ID
    {   Symbol *s;
        if ((s = lookup($1)) == NULL)
            cplerror("Identifier '%s' not declared", $1);
        else {
            /* factor.type = lookup(ID)->type */
            $$.type = s->type;
            $$.data = makeSYMID(s->ID);
        }
        free($1);
    }
    | TOK_ICONST
    {   $$.type = INT; /* expression type */
        $$.data.val.iconst = $1; /* data value is the ICONST */
        $$.data.format = ICONST; /* data type (in union) */ }
    | TOK_FCONST
    {   $$.type = FLOAT;
        $$.data.val.fconst = $1;
        $$.data.format = FCONST; }
    ;

%%

/* Emit necessary casts to e1 or e2. Modifies e1 or e2 to
 * be an expression referring to the cast instruction */
void typeConversion(struct expData *e1, struct expData *e2) {
    struct expData *intExp;

    if (e1->type == e2->type)
        return; /* no conversion needed */

    /* otherwise, one is float and other is int */
    intExp = (e1->type == INT ? e1 : e2);
    /* generate instruction to convert int expression to float */
    emit(makeTriple(CAST, INT, intExp->data, EMPTY));
    /* uptdate the expData */
    intExp->data = makeLINENO(nextinstr-1); /* refers to cast */
    intExp->type = FLOAT;
}

/* returns whether there was an error */
int frontend(FILE *input) {
    if (input == NULL) /* error in previous stage, skip */
        return 1;

    fronterror = 0;
    yyin = input;
    yyparse();
 
    /* frees memory allocated by flex preventing memory leak
    https://stackoverflow.com/questions/43671389/minimal-bison-flex-generated-code-has-memory-leak */
    yylex_destroy();

    /* in case of parsing error, make sure everything was freed */
    if (fronterror) {
        deleteAllEnv();
        instrdel(); 
    }

    fclose(input);
    return fronterror;
}

