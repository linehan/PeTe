#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "pset.h"

void pset_init(struct pset_t *set, int n)
{
        int i;

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
        int i;
        int v;

        /* We are NOT checking length of perm, so warning. */
        for (i=0; i<set->n; i++) {
                v = perm_get_block(perm, i);

                set->trace[i][v] += 1;
        }
        set->size++;
}

void pset_sum(struct pset_t *a, struct pset_t *b)
{
        int i;
        int j;

        a->size += b->size;

        /* We are NOT checking length of perm, so warning. */
        for (i=0; i<a->n; i++) {
                for (j=0; j<a->n; j++) {
                        a->trace[i][j] += b->trace[i][j];
                }
        }
}

void pset_write(struct pset_t *set, FILE *file)
{
        int i;
        int j;

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

