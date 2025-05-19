#include <stdlib.h>
#include <stdio.h>

#include "patches.h"

const PatchList emptyList = {NULL, NULL};

PatchList makelist(int patchinstr) {
    PatchList p;
    struct PatchNode *h;

    if ((h = malloc(sizeof(struct PatchNode))) == NULL) {
        fprintf(stderr, "Could not allocate memory\n");
        exit(1);
    }

    h->patchinstr = patchinstr;
    h->next = NULL;
    p.head = p.tail = h;
    return p;
}

PatchList merge(PatchList l1, PatchList l2) {
   PatchList p;

   if (l1.head == NULL)
       return l2;
   if (l2.head == NULL)
       return l1;

   p.head = l1.head;
   l1.tail->next = l2.head;
   p.tail = l2.tail;
   return p;
}

void swapLists(PatchList *l1, PatchList *l2) {
    struct PatchNode *swap;

    swap = l1->head;
    l1->head = l2->head;
    l2->head = swap;
    swap = l1->tail;
    l1->tail = l2->tail;
    l2->tail = swap;
}

void deletelist(PatchList l) {
    struct PatchNode *d, *t;

    for (t = l.head; (d = t); free(d))
        t = d->next;
}

