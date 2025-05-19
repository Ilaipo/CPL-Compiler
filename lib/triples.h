#ifndef TRIPLES
#define TRIPLES

#include "patches.h"
#include "symbols.h"

#define INITIAL_SIZE 200 
#define FACTOR 2

typedef struct Triple {
    enum Operator {
        PLUS, MIN, MUL, DIV, EQ, NEQ, LT, GT, GE, LE,  
        CAST, ASSIGN, INPUT, OUTPUT, JMP, JMPFALSE } op;
    enum Type opType;
    struct Argument {
        union {
            unsigned lineno;
            int iconst;
            float fconst;
            unsigned symID;
        } val;
        enum ArgFormat {UNUSED, LINENO, ICONST, FCONST, SYMID, JUMP} format;
    } arg1, arg2;
} Triple;


/* next free index (doubles as size of array) */
extern unsigned nextinstr;

/* shorthand for when no argument is needed */
extern const struct Argument EMPTY;

/* constructing function for making most types of Triples
 * easier to initialize */
Triple makeTriple(enum Operator op, enum Type opType, struct Argument arg1, struct Argument arg2);

/* constructing function for a JUMP type argument */
struct Argument makeJUMP(unsigned lineno);

/* constructing function for a LINENO type argument */
struct Argument makeLINENO(unsigned lineno);

/* constructing function for a SYMID type argument */
struct Argument makeSYMID(unsigned symID);

/* add a new Triple to the instructions list */
void emit(Triple t);

/* clean up instruction list */
void instrdel();

/* inserts i as target instruction for each instruction in list 
 * (changes arg1 of every instruction to 'i')
 * after list is patchede, deletes the list. */
void backpatch(PatchList p, unsigned i);

/* get instruction at index 'ind'. exits program if out of range */
Triple getInstr(unsigned ind);

void instrout();
#endif
