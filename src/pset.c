#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "pset.h"

void pset_init(struct pset_t *set, uint64_t n)
{
        uint64_t i;

        assert(n <= 16);

        set->trace = calloc(1, n*sizeof(uint64_t *)); 

        for (i=0; i<n; i++) {
                set->trace[i] = calloc(1, n*sizeof(uint64_t));
        }

        set->n    = n;
        set->size = 0;
}

void pset_add(struct pset_t *set, perm_t perm)
{
        uint64_t i;
        uint64_t v;

        /* 
         * We are NOT checking length of perm, so warning. 
         * We assume that set->n is equal to length of perm.
         */
        for (i=0; i<set->n; i++) {
                /* SIMD THIS */
                v = perm_get_block(perm, i);

                set->trace[i][v] += 1;
        }
        set->size++;
}

void pset_sum(struct pset_t *a, struct pset_t *b)
{
        uint64_t i;
        uint64_t j;

        a->size += b->size;

        /* We are NOT checking length of perm, so warning. */
        for (i=0; i<a->n; i++) {
                for (j=0; j<a->n; j++) {
                        a->trace[i][j] += b->trace[i][j];
                }
        }
}

/*
 * What orientation do you prefer?
 *
 * Standard for perms is 0,0 at top-left, and increasing position going down y-axis.
 *
 */
void pset_write(struct pset_t *set, FILE *file)
{
        uint64_t i;
        uint64_t j;

        for (i=0; i<set->n; i++) {
                for (j=0; j<set->n; j++) {
                        /*if (j == set->n-1) {*/
                                /*fprintf(file, "%"PRIu64"\n", set->trace[i][j]);*/
                        /*} else {*/
                                /*fprintf(file, "%"PRIu64" ", set->trace[i][j]);*/
                        /*}*/
                        if (j == set->n-1) {
                                fprintf(file, "%g\n", (double)set->trace[i][j]/(double)set->size);
                        } else {
                                fprintf(file, "%g ", (double)set->trace[i][j]/(double)set->size);
                        }
                }
        }
}

