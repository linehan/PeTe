#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#include "perm.h"

/*#define perm_swap(p, a, b) \*/
        /*perm_block(p,a)*/

        /*(uint64_t)((perm >> (index*PERM_BLOCK_SIZE)) & PERM_BLOCK_MASK); */

        /*PERM_WRITE_BLOCK(PERM_CLEAR_BLOCK(perm, index), index, value);*/



/**
 * perm_get_block() 
 * ````````````````
 * Retrieve the block value stored at an index.
 *
 * @perm : Permutation
 * @index: Block index
 * Return: Value stored in block @index.
 */
uint64_t perm_get_block(perm_t perm, int index) 
{
        return (uint64_t)((perm >> (index*PERM_BLOCK_SIZE)) & PERM_BLOCK_MASK); 
}

/**
 * perm_set_block()
 * ````````````````
 * Clear a block and then re-write its value.
 *
 * @perm : Permutation
 * @index: Index of block to rewrite
 * @value: Value to write at block
 * Return: Resulting permutation 
 */
perm_t perm_set_block(perm_t perm, int index, uint64_t value) 
{
        return PERM_WRITE_BLOCK(PERM_CLEAR_BLOCK(perm, index), index, value);
        /*return (perm & ~((perm_t)PERM_BLOCK_MASK << (index * PERM_BLOCK_SIZE))) | ((perm_t)value << (index * PERM_BLOCK_SIZE)); // clear digit and then rewrite its value*/
}

/** 
 * perm_insert_blank_block()
 * `````````````````````````
 * Insert a blank block at a position, shifting blocks to the right.
 *
 * @perm : Permutation
 * @index: Block index where blank will be inserted.
 * Return: Resulting permutation
 *
 * NOTE:
 * The values in block @index and higher will be shifted 1 to the right. 
 */
perm_t perm_insert_blank(perm_t perm, int index) 
{
        perm_t bottom = perm & (((perm_t)1 << (PERM_BLOCK_SIZE * index)) - 1);
        perm_t top    = perm & ((~ (perm_t)0) - (((perm_t)1 << (PERM_BLOCK_SIZE * index)) - 1));
        
        return bottom + (top << PERM_BLOCK_SIZE);
}

/**
 * perm_insert_block()
 * ```````````````````
 * Insert a block and write a value to it. 
 *
 * @perm : Permutation
 * @index: Block index where value will be inserted.
 * @value: Value to write at @index
 * Return: Resulting permutation
 *
 * NOTE:
 * The values in block @index and higher will be shifted 1 to the right.
 */
perm_t perm_insert_block(perm_t perm, int index, uint64_t value) 
{
        return perm_set_block(perm_insert_blank(perm, index), index, value);
}

/**
 * perm_remove_block()
 * ```````````````````
 * Remove a block from the permutation
 *
 * @perm: Permutation
 * @index: Index of block to be removed 
 * Return: Resulting permutation
 *
 * NOTE:
 * Blocks at indices higher than @index will be shifted 1 to the left.
 */
perm_t perm_remove_entry(perm_t perm, int index) 
{
        perm_t bottom = perm & (((perm_t)1 << (PERM_BLOCK_SIZE * index)) - 1);
        perm_t top = perm & ((~ (perm_t)0) - (((perm_t)1 << (PERM_BLOCK_SIZE * index + PERM_BLOCK_SIZE)) - 1));

        if ((PERM_BLOCK_SIZE * index + PERM_BLOCK_SIZE) == PERM_SIZE) { 
                return bottom; // top is ill-defined in this case
        }

        return bottom + (top >> PERM_BLOCK_SIZE); 
}

/**
 * perm_length()
 * `````````````
 * Measure the length of a permutation
 *
 * @perm : Permutation
 * Return: Length of permutation
 *
 * TODO:
 * Eliminate loop, make this more efficient using either a header
 * block or some clever encoding to quickly tell the length.
 */
int perm_length(perm_t perm) 
{
        uint64_t len = 0;
        int i;
        
        for (i=0; i<PERM_SIZE/PERM_BLOCK_SIZE; i++) {
                len = MAX(len, perm_get_block(perm, i));
        }
  
        return (int)(len + 1);
}

/**
 * perm_from_string()
 * ``````````````````
 * Create a permutation (up to 9 digits long) from a string.
 *
 * @str  : String like "123456", "312", "4123", etc.
 * Return: Permutation
 */
perm_t perm_from_string(char *str) 
{
	perm_t perm = 0;
	int i;

  	for (i=0; i<strlen(str); i++) { 
		perm = perm_set_block(perm, i, (uint64_t)(str[i] - '1'));
	}

  	return perm;
}

/**
 * perm_length_n()
 * ```````````````
 * Make a permutation with length n.
 *
 * @n    : Length of permutation
 * Return: Permutation 1234...n.
 */
perm_t perm_of_length(int n)
{
        int i = 0;
        perm_t p = 0;

        for (i=0; i<n; i++) {
                p = perm_set_block(p, i, i);
        }
        return p;
}

/**
 * perm_print()
 * ````````````
 * Print a permutation to standard output.
 *
 * @perm : Permutation to print
 * Return: Nothing
 */
void perm_print(perm_t perm) 
{
        int i;

        for (i=0; i<PERM_SIZE/PERM_BLOCK_SIZE; i++) {
                if (i != (PERM_SIZE/PERM_BLOCK_SIZE)-1) {
                        printf("%d,", perm_get_block(perm, i));
                } else {
                        printf("%"PRIu64"", perm_get_block(perm, i));
                }
        }
        /* printf("\n"); */
}


void perm_print_bits(perm_t p)
{
        while (p) {
                if (p & 1) {
                        printf("1");
                } else {
                        printf("0");
                }
                p >>= 1;
        }
        printf("\n");
}


/*perm_t getinverse(perm_t perm, int length) */
/*{*/
        /*perm_t inverse = 0;*/
        /*int i;*/

        /*for (i=0; i<length; i++) {*/
		/*uint64_t digit = getdigit(perm, i);*/
		/*[>printf("get [%u]=%u set[%u]=%u\n", i, digit, digit, i);<]*/
                    /*inverse = setdigit(inverse, (int)digit, (uint64_t)i);*/
        /*}*/
        
        /*return inverse;*/
/*}*/

/*uint64_t getmaxdigit(perm_t perm) */
/*{*/
        /*uint64_t answer = 0;*/
        
        /*for (int i = 0; i < PERM_SIZE / PERM_BLOCK_SIZE; i++) {*/
                /*answer = MAX(answer, getdigit(perm, i));*/
        /*}*/
  
        /*return answer;*/
/*}*/


/*int prefix_matches(perm_t perm, perm_t pattern, int perm_len)*/
/*{*/
	/* 
	 * If the MSB of pattern ^ perm -- i.e. the first place
	 * that they differ, occurs prior to perm_len, then perm 
	 * is not a prefix of pattern.
	 */
	/*return (__builtin_clz(pattern ^ perm) < perm_len) ? 1 : 0;*/
/*}*/