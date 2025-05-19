#ifndef PATCHES
#define PATCHES

typedef struct PatchList {
    struct PatchNode {
        int patchinstr;
        struct PatchNode *next;
    } *head, *tail;
} PatchList;

extern const PatchList emptyList;

/* return a list of one element - paramater */
PatchList makelist(int patchinstr);

/* free memory allocated by list 'l' */
void deletelist(PatchList l);

/* return a concatenated list. There will be no need to free
 * 'l1' or 'l2' afterwards, only the returned list.*/
PatchList merge(PatchList l1, PatchList l2);

/* swaps the internal lists in l1 and l2 */
void swapLists(PatchList *l1, PatchList *l2);

#endif
