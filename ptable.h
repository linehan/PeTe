/*
 *  ptable.h
 */
#ifndef _P_PTABLE_H_
#define _P_PTABLE_H_

#include <stdbool.h>
#include "perm.h"

/*
 * NOTE
 * Empty slots will be initialized to 0, and an
 * element x will be stored as x+1 to compensate.
 *
 * TODO
 * Rather than this ridiculous state of affairs,
 * perhaps we should require that the permutations
 * begin at 1, rather than 0.
 */
struct ptable_t {
        unsigned long long averageinsertiontime;
        perm_t *array;
        unsigned long long maxsize; 
        unsigned long long size; 
};


unsigned long long hash_perm(perm_t key_in, uint64_t maxsize);

void ptable_init    (struct ptable_t *table, unsigned long long startsize);
void ptable_insert  (struct ptable_t *ptable, perm_t perm);
bool ptable_contains(struct ptable_t *ptable, perm_t perm);

#endif
