#ifndef _P_PSET_H_
#define _P_PSET_H_
#include <stdio.h>
#include "perm.h"

struct pset_t {
        int n;
        uint64_t size;
        uint64_t **trace; 
};

void pset_init(struct pset_t *set, int n);
void pset_add(struct pset_t *set, perm_t perm);
void pset_write(struct pset_t *set, FILE *file);
void pset_sum(struct pset_t *a, struct pset_t *b);

#endif
