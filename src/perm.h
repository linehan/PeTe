#ifndef _P_PERM_H_
#define _P_PERM_H_
#include <stdint.h>

/* For PRu64 format macro to printf() */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#define MAX(a,b) \
        ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a > _b ? _a : _b; })


#define MIN(a,b) \
        ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a < _b ? _a : _b; })


/******************************************************************************
 * PERMUTATION TYPES
 ******************************************************************************/
typedef uint64_t perm_t;

#define PERM_MAX_LENGTH 16
#define PERM_BLOCK_SIZE  4
#define PERM_SIZE       64

/* 
 * 0000 0001 0000 - 000 0000 0001 = 0000 0000 1111
 */
#define PERM_BLOCK_MASK \
        ((((perm_t) 1) << PERM_BLOCK_SIZE) - (perm_t)1)

#define PERM_CLEAR_BLOCK(p, i) \
        (p & ~((perm_t)PERM_BLOCK_MASK << (i * PERM_BLOCK_SIZE))) 

#define PERM_WRITE_BLOCK(p, i, v) \
        (p | ((perm_t)v << (i * PERM_BLOCK_SIZE))) 


#define perm_block(p, i) \
        ((uint64_t)((p >> (i*PERM_BLOCK_SIZE)) & PERM_BLOCK_MASK))

inline uint64_t perm_get_block(perm_t perm, int index) 
{
        return (uint64_t)((perm >> (index*PERM_BLOCK_SIZE)) & PERM_BLOCK_MASK); 
}

inline perm_t perm_set_block(perm_t perm, uint64_t index, uint64_t value) 
{
        return PERM_WRITE_BLOCK(PERM_CLEAR_BLOCK(perm, index), index, value);
        /*return (perm & ~((perm_t)PERM_BLOCK_MASK << (index * PERM_BLOCK_SIZE))) | ((perm_t)value << (index * PERM_BLOCK_SIZE)); // clear digit and then rewrite its value*/
}

inline perm_t perm_remove_block(perm_t perm, int index) 
{
        perm_t bottom = perm & (((perm_t)1 << (PERM_BLOCK_SIZE * index)) - 1);
        perm_t top = perm & ((~ (perm_t)0) - (((perm_t)1 << (PERM_BLOCK_SIZE * index + PERM_BLOCK_SIZE)) - 1));

        if ((PERM_BLOCK_SIZE * index + PERM_BLOCK_SIZE) == PERM_SIZE) { 
                return bottom; // top is ill-defined in this case
        }

        return bottom + (top >> PERM_BLOCK_SIZE); 
}

inline perm_t perm_swap(perm_t perm, int index_a, int index_b)
{
        int a = perm_get_block(perm, index_a);
        int b = perm_get_block(perm, index_b);

        perm = perm_set_block(perm, index_a, b);
        perm = perm_set_block(perm, index_b, a);
                
        return perm;
}

//uint64_t perm_get_block(perm_t perm, int index);
//perm_t perm_set_block(perm_t perm, int index, uint64_t value);
perm_t perm_inverse(perm_t perm, int length);
perm_t perm_insert_blank(perm_t perm, int index);
perm_t perm_insert_block(perm_t perm, int index, uint64_t value);
//perm_t perm_remove_entry(perm_t perm, int index);
int perm_length(perm_t perm);
perm_t perm_from_string(char *str);
perm_t perm_of_length(int n);
void perm_print(perm_t perm);
void perm_print_bits(perm_t p);
char *perm_get_string(perm_t perm);
perm_t perm_from_csv(char *csv_string);
char *perm_get_string_literal(perm_t perm, int length);

#endif
