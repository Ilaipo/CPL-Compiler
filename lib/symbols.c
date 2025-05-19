#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "symbols.h"

static Env *top = NULL;

/* hash: form hash value for symbol named 's' */
unsigned hash(const char *s) {
    unsigned hashval;
    
    for (hashval = 0; *s; s++)
        hashval = *s + 31 * hashval;
    return hashval % HASHSIZE;
}

struct Entry *add_in(struct Entry *list, char *name, enum Type type, unsigned ID) {
    struct Entry *e;

    if ((e = malloc(sizeof(struct Entry))) == NULL) {
        fprintf(stderr, "Could not allocate memory\n");
        exit(1);
    }
    e->s.name = name;
    e->s.type = type; 
    e->s.ID = ID;
    e->next = list;
    return e;
}

struct Entry *add(struct Entry *list, char *name) {
    return add_in(list, name, INT, 0); /* temporary type and id */
}

Symbol *lookup(const char *name) {
    struct Entry *e;
    Env *curr;
    
    if (top == NULL) {
        fprintf(stderr, "Can't lookup, no environment!\n");
        return NULL;
    }
    for (curr = top; curr != NULL; curr = curr->prev) {
        for (e = top->tab[hash(name)]; e; e = e->next)
            if (strcmp(e->s.name, name) == 0)
                return &e->s;
    }
    return NULL; /* reached end of environment stack */
}

int insert(char *name, enum Type type) {
    static unsigned ID = 0;
    unsigned hashval;
    struct Entry *e;

    if (top == NULL) {
        fprintf(stderr, "Can't insert, no environment!\n");
        return 1;
    }
    for (e = top->tab[hashval = hash(name)]; e; e = e->next)
        if (strcmp(e->s.name, name) == 0)
            return 1; /* name is already in current table */

    top->tab[hashval] = add_in(top->tab[hashval], name, type, ID++);
    return 0;
}

void delSymList(struct Entry *list) {
    struct Entry *t;

    while ((t = list)) {
        list = t->next; /* prepare to delete entry 't' */
        free(t->s.name); /* free symbol name string */
        free(t);
    }
}

void popEnv() {
    Env *delete;
    int i;

    if (top == NULL) {
        fprintf(stderr, "Can't pop, no environment!\n");
        return;
    }
    top = (delete = top)->prev; /* update 'top' pointer */
    for (i=0; i<HASHSIZE; i++) /* iterate over hashvalues */
        delSymList(delete->tab[i]); 
    free(delete);
}

void deleteAllEnv() {
    Env *delete;
    int i;

    for (; (delete = top) != NULL; free(delete) ) {
        for (i=0; i<HASHSIZE; i++) /* iterate over hashvalues */
            delSymList(delete->tab[i]); 
        top = top->prev; 
    }
}

void pushEnv() {
    Env *t;

    t = top;
    if ((top = calloc(1, sizeof(Env))) == NULL) { /* empty env */
        fprintf(stderr, "Could not allocate memory\n");
        exit(1);
    }
    top->prev = t;
}


char *strdup(const char *s) {
    char *p;

    if ((p = malloc(strlen(s)+1)) != NULL)
        strcpy(p, s);
    else {
        fprintf(stderr, "Could not allocate memory\n");
        exit(1);
    }
    return p;
}

