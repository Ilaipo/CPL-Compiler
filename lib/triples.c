#include <stdio.h>
#include <stdlib.h>

#include "triples.h"

static Triple *instrList = NULL;
static unsigned capacity = 0;
unsigned nextinstr = 0;

const struct Argument EMPTY = {{0}, UNUSED};

/* private procedure to grow the arraylist */
static void grow() {
    /* allocate instrList with INITIAL_SIZE or
     * multiply by FACTOR, updating capacity */
    if ((instrList = realloc(instrList, sizeof(Triple) *
            (capacity = (capacity == 0 ? 
                         INITIAL_SIZE : capacity * FACTOR))))) 
        return;

    fprintf(stderr, "Could not allocate memory\n");
    exit(1);
}

Triple makeTriple(enum Operator op, enum Type opType, struct Argument arg1, struct Argument arg2) {
    Triple t;
    t.op = op;
    t.opType = opType;
    t.arg1 = arg1;
    t.arg2 = arg2;
    return t;
}

struct Argument makeJUMP(unsigned lineno) {
    struct Argument t;
    t.val.lineno = lineno;
    t.format = JUMP;
    return t;
}

struct Argument makeLINENO(unsigned lineno) {
    struct Argument t;
    t.val.lineno = lineno;
    t.format = LINENO;
    return t;
}

struct Argument makeSYMID(unsigned symID) {
    struct Argument t;
    t.val.symID = symID;
    t.format = SYMID;
    return t;
}

void emit(Triple t) {
    if (nextinstr >= capacity)
        grow();
    instrList[nextinstr++] = t;
}

void instrdel() {
    /* wrapper for free of static list */
    free(instrList);
}

void backpatch(PatchList p, unsigned i) {
    struct PatchNode *n;

    for (n = p.head; n; n = n->next) {
        instrList[n->patchinstr].arg1.val.lineno = i;
        instrList[n->patchinstr].arg1.format = JUMP;
    }

    deletelist(p);
}

Triple getInstr(unsigned ind) {
    if (ind < nextinstr)
        return instrList[ind];

    fprintf(stderr, "Instruction out of range\n");
    exit(1);
}

