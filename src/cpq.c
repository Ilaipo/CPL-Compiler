#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define IN_SUFFIX ".ou"
#define OUT_SUFFIX ".qud"

/* opens the input validating file ending. return file pointer */
FILE *openin(char const *inname, char const **suffix) {
    FILE *in;
    if (!(*suffix = strrchr(inname, '.')) ||
            strcmp(*suffix, IN_SUFFIX)) {
        fprintf(stderr, "File must end with '%s'\n", IN_SUFFIX);
        return NULL;
    }
    if (!(in = fopen(inname, "r"))) {
        fprintf(stderr, "Unable to open file '%s'\n", inname);
        return NULL;
    }

    return in;
}

/* opens the output file, given a pointer to input file name, 
 * and pointer to that file name's suffix section. returns file */
FILE *openout(char const *inname, char const *suffix) {
    FILE *out;
    char *outname;

    if (!(outname = malloc(suffix - inname + sizeof OUT_SUFFIX))) {
        fprintf(stderr, "Memory allocation error\n");
        return NULL;
    }
    strncpy(outname, inname, suffix - inname);
    strcpy(outname + (suffix - inname), OUT_SUFFIX);
    if (!(out = fopen(outname, "w"))) {
        fprintf(stderr, "Unable to open file '%s'\n", outname);
        free(outname);
        return NULL;
    }
    free(outname);
    return out;
}

int frontend(FILE *input); /* defined in "cpl.y" */
int backend(FILE *output); /* defined in "quad.c" */

int main(int argc, char *argv[]) {
    char const *suffix;

    if (argc != 2) {
        fprintf(stderr, "Expected input file as argument.\n");
        return 1;
    }
    
    if (frontend(openin(argv[1], &suffix)))
        return 1;
    if (backend(openout(argv[1], suffix)))
        return 1;

    fprintf(stderr, "SIGNATURE - ILAI POLINER\n");

    return 0;
}

