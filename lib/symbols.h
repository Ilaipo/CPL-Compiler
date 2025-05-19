#ifndef SYMBOLS
#define SYMBOLS

#define HASHSIZE 101

enum Type {INT, FLOAT};

typedef struct Symbol {
    char *name;
    enum Type type;
    unsigned ID;
} Symbol;

typedef struct Env {
    struct Entry {
        Symbol s;
        struct Entry *next;
    } *tab[HASHSIZE];
    struct Env *prev;
} Env;

/* creates a symbol named 'name' and adds it to the list pointed
 * to by 'list' (may be NULL) the new list head will be returned */
struct Entry *add(struct Entry *list, char *name);

/* adds symbol to the symbol table in environment top.
 * If no errors returns 0. If duplicate symbol returns 1. */
int insert(char *name, enum Type type);

/* free memory of symbol list pointed to by 'list' (may be NULL) */
void delSymList(struct Entry *list);

/* returns pointer to Symbol entry or NULL if not found.
 * This function searches recursivly, so that if name is not
 * found in current top environment, continue searching in
 * previous environments. */
Symbol *lookup(const char *name);

/* deallocates top environment, including the whole symbol table */
void popEnv();

/* deallocates all enviornments in stack */
void deleteAllEnv();

/* allocates a new top enviornment, linked to previous top */
void pushEnv();

/* allocate memory for a copy of 's', returns pointer to copy. */
char *strdup(const char *s);

#endif
