#include <stdio.h>
#include "../lib/triples.h"

/* lines in output start at 1 */
#define LINE_OFFSET 1

const char *ARG_PREFIX[] = { "", "t", "", "", "s", "" };

void fprintArg(FILE *f, struct Argument arg) {
    switch(arg.format) {
        case UNUSED: return;
        case LINENO: 
            fprintf(f, " t%u", arg.val.lineno); break;
        case ICONST:
            fprintf(f, " %d", arg.val.iconst); break;
        case FCONST:
            fprintf(f, (arg.val.fconst == (int) arg.val.fconst) ?
                        " %g.0" : " %g", arg.val.fconst); break;
        case SYMID:
            fprintf(f, " s%d", arg.val.symID); break;
        case JUMP:
            fprintf(f, " %u", arg.val.lineno + LINE_OFFSET); break;
    }
}

const char *QUAD_OPERATORS[] = { "ADD", "SUB", "MLT", "DIV", "EQL", "NQL", "LSS", "GRT", "unused", "unused", "TO", "ASN", "INP", "PRT", "JUMP", "JMPZ" };

void genQuadInstrs(FILE *output) {
    Triple t;
    unsigned i;

    for (i=0; i<nextinstr; i++) {
        t = getInstr(i);

        fprintf(output, "%s%s%s",
                (t.op == JMP || t.op == JMPFALSE)
                    ? "" : (t.opType == INT ? "I" : "R"),
                (0 <= t.op && t.op < sizeof(QUAD_OPERATORS))
                    ? QUAD_OPERATORS[t.op] : "ERROR",
                (t.op == CAST
                    ? (t.opType == INT ? "R" : "I") : ""));

        switch (t.op) {
            case JMP: case JMPFALSE: case INPUT: 
            case OUTPUT: case ASSIGN: break;
            default:
                fprintf(output, " t%d", i);
        }
        fprintArg(output, t.arg1);
        fprintArg(output, t.arg2);
        fprintf(output, "\n");
    }

    fprintf(output, "HALT\n");
}

int backend(FILE *output) {
    if (output == NULL) {
        instrdel();
        fclose(output);
        return 1;
    }
    genQuadInstrs(output);

    /* ok to free instruction list */
    instrdel();
    fprintf(output, "SIGNATURE - ILAI POLINER\n");
    fclose(output);
    return 0;
}

