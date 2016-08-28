#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <assert.h>
#include <stdint.h>
#include <errno.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

/*uint64_t i;*/
/*printf("%"PRIu64"\n", i);*/


/*#define GO_FAST 1*/
/* test */

uint64_t factorial(uint64_t num) 
{
        uint64_t answer = 1;
  
        while (num > 1) {
                answer *= num;
                num--;
        }
        
        return answer;
}

int binom(int n, int k)
{
        return (n==0 | k==0) ? 1 : (n*binom(n-1,k-1))/k;
}

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
        /*(perm & ~((perm_t)LETTERFACE << (index * PERM_BLOCK_SIZE))) | ((perm_t)value << (index * PERM_BLOCK_SIZE)); // clear digit and then rewrite its value*/
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
inline perm_t perm_from_string(char *str) 
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
                printf("%d", perm_get_block(perm, i));
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


/******************************************************************************
 * COUNTING PATTERNS IN THEM 
 ******************************************************************************/

perm_t downarrow(perm_t perm, int i)
{
        int pos;
        int j;

        int length = perm_length(perm);

        /* 
         * In paper written as length-i+1, but we index from 0 in
         * this implementation, so it requires another -1.  
         */
        int seek_digit = (length-i+1)-1;

        for (j=0; j<length; j++) {
                if (perm_get_block(perm, j) == seek_digit) {
                        pos = j;
                        break;
                }
        }

        perm = perm_remove_entry(perm, pos);

        for (j=0; j<length-1; j++) {
                int digit = perm_get_block(perm, j);
                if (digit > seek_digit) {
                        perm = perm_set_block(perm, j, digit-1);
                }
        }

        return perm;
}


int p(int i, perm_t perm, perm_t pattern) 
{
        int perm_len    = perm_length(perm);
        int pattern_len = perm_length(pattern);

        if (perm_len == pattern_len) {
                if (perm == pattern) {
                        return 1;
                } else {
                        return 0;
                }
        }

        if (i == pattern_len+1) {
                return 0;
        }

        return p(i, downarrow(perm, i+1), pattern) + p(i+1, perm, pattern);
}


perm_t downarrowfast(perm_t perm, int i, int length)
{
        int pos;
        int j;

        /* 
         * In paper written as length-i+1, but we index from 0 in
         * this implementation, so it requires another -1.  
         */
        int seek_digit = (length-i+1)-1;

        for (j=0; j<length; j++) {
                if (perm_get_block(perm, j) == seek_digit) {
                        pos = j;
                        break;
                }
        }

        perm = perm_remove_entry(perm, pos);

        for (j=0; j<length-1; j++) {
                int digit = perm_get_block(perm, j);
                if (digit > seek_digit) {
                        perm = perm_set_block(perm, j, digit-1);
                }
        }

        return perm;
}

int pfast(int i, perm_t perm, perm_t pattern, int perm_len, int pattern_len) 
{
        if (perm_len == pattern_len) {
                if (perm == pattern) {
                        return 1;
                } else {
                        return 0;
                }
        }

        if (i == pattern_len+1) {
                return 0;
        }

        return pfast(i, downarrow(perm, i+1), pattern, perm_len-1, pattern_len) + pfast(i+1, perm, pattern, perm_len, pattern_len);
}


/******************************************************************************
 * OTHER COUNTING STUFF
 ******************************************************************************/
int count(perm_t perm, perm_t pattern) 
{
        return p(0, perm, pattern);
}

int countfast(perm_t perm, perm_t pattern, int perm_len, int pattern_len) 
{
        return pfast(0, perm, pattern, perm_len, pattern_len);
}


float occ(int n, int k, float density)
{
        return (float)binom(n, k) * density;
}

void densities(int n, int k)
{
	int max = binom(n, k);
	int i;

	for (i=0; i<max; i++) {
		printf("[%d] => %g\n", i, (float)i/max);
	}
}



/* Equal to nchoosek */
#define TALLY_SIZE 1820

uint64_t Tally[TALLY_SIZE] = {0};

FILE *Log;

void write_tally(FILE *f)
{
        int i;

        /* Rewind file */
        fseek(f, 0L, SEEK_SET);

        for (i=0; i<TALLY_SIZE; i++) {
                fprintf(f, "%d %d\n", i, Tally[i]); 
        }
}


uint64_t Track = 0;
uint64_t Total = 0;
uint64_t Count = 0;


/**
 * perm_swap()
 * ```````````
 * Swap the values at two indices
 *
 * @perm   : Permutation
 * @index_a: First index
 * @index_b: Second index
 * Return  : Resulting permutation
 *
 * TODO
 * Make this faster!
 */
perm_t perm_swap(perm_t perm, int index_a, int index_b)
{
        int a = perm_get_block(perm, index_a);
        int b = perm_get_block(perm, index_b);

        perm = perm_set_block(perm, index_a, b);
        perm = perm_set_block(perm, index_b, a);
                
        return perm;
}


void do_the_thing(perm_t p, perm_t pattern)
{
        int c;

        c = count(p, pattern);

        Tally[c]++;

        Count++;

        #ifndef GO_FAST
        Track++;
        if (Track % 100) {
                Track=0;
                write_tally(Log);
        }

        printf("%d\t", c);
        perm_print(p);
        printf("\n");

        fprintf(stderr, "\r(%f%%) %d/%d", ((float)Count/(float)Total)*100, Count, Total);
        #else
        if (!(Track % 10000)) {
                fprintf(stderr, "\r(%f%%) %d/%d", ((float)Count/(float)Total)*100, Count, Total);
                Track=0;
        }
        Track++;
        #endif
}


void generate_permutations(perm_t p, perm_t pattern) 
{
        int idx[4096];
        int i;
        int t;
        int M;
        int N;
        int c;

        N = perm_length(p);

        for (i=1; i<=N; i++) {
                perm_set_block(p, i-1, i-1);
                idx[i] = 1;
        }

        /* EXECUTE FUNCTION */
        do_the_thing(p, pattern);

        for (i=1; i<=N;) {
                if (idx[i] < i) {
                        if (i % 2) {
                                p = perm_swap(p, 0, i-1);
                        } else {
                                p = perm_swap(p, idx[i]-1, i-1);
                        }
                        idx[i]++;
                        i = 1;

                        /* EXECUTE FUNCTION */
                        do_the_thing(p, pattern);
                } else {
                        idx[i++] = 1;
                }
        }
}

void generate_permutations_fast(perm_t p, perm_t pattern) 
{
        int idx[4096];
        int i;
        int t;
        int M;
        int N;
        int c;

        N = perm_length(p);
        M = perm_length(pattern);

        for (i=1; i<=N; i++) {
                perm_set_block(p, i-1, i-1);
                idx[i] = 1;
        }

        /* DO THE THING */
        c = countfast(p, pattern, N, M);
        Tally[c]++;
        Count++;

        if (Track++ == 10000) {
                fprintf(stderr, "\r(%f%%) %d/%d", ((float)Count/(float)Total)*100, Count, Total);
                Track=0;
        }
        /* DONE DOING THE THING */


        for (i=1; i<=N;) {
                if (idx[i] < i) {
                        if (i % 2) {
                                p = perm_swap(p, 0, i-1);
                        } else {
                                p = perm_swap(p, idx[i]-1, i-1);
                        }
                        idx[i]++;
                        i = 1;

                        /* DO THE THING */
                        c = countfast(p, pattern, N, M);
                        Tally[c]++;
                        Count++;

                        if (Track++ == 10000) {
                                fprintf(stderr, "\r(%f%%) %"PRIu64"/%"PRIu64"", ((float)Count/(float)Total)*100, Count, Total);
                                Track=0;
                        }
                        /* DONE DOING THE THING */

                } else {
                        idx[i++] = 1;
                }
        }
}

        
/**
 * permute()
 * `````````
 * Get all permutations using Heap's algorithm 
 *
 * @perm   : Permutation
 * @pattern: Pattern to find
 * @l      : Index to start permuting from (0 for whole string)
 * @r      : Index to stop permuting on 
 * Return  : Nothing
 */
void permute(perm_t p, perm_t pattern, int l, int r)
{
        int i;
        int c;
                    
        if (l == r) {
                c = count(p, pattern);

                Tally[c]++;

                #ifndef GO_FAST
                Track++;
                if (Track % 100) {
                        Track=0;
                        write_tally(Log);
                }


                printf("%d\t", c);
                perm_print(p);
                printf("\n");

                fprintf(stderr, "\r(%f%%) %d/%d", ((float)Count/(float)Total)*100, Count++, Total);
                #else
                if (Track++ % 100000) {
                        fprintf(stderr, "\r(%f%%) %d/%d", ((float)Count/(float)Total)*100, Count++, Total);
                        Track=0;
                }
                #endif
        } else {
                for (i=l; i<=r; i++) {
                        p = perm_swap(p, l, i);
                        permute(p, pattern, l+1, r);
                        p = perm_swap(p, l, i); //backtrack
                }
        }
}


/*void yup(perm_t perm, perm_t pattern) */
/*{*/
        /*permute(perm, pattern, 0, perm_length(perm)-1); */
/*}*/


/*void explore_output(perm_t perm, perm_t pattern) */
/*{*/
        /*int i;*/
        /*int c;*/
        /*perm_t p = perm;*/
        /*int len = perm_length(p);*/

        /*if (len > 15) {*/
                /*return;*/
        /*}*/

        /*for (i=0; i<len; i++) {*/
                /*p = perm_insert_block(p, i, len);*/

                /*explore_output(p, pattern);*/

                /*c = count(p, pattern);*/

                /*Tally[c]++;*/

                /*Track++;*/
                /*if (Track % 100) {*/
                        /*Track=0;*/
                        /*write_tally(Log);*/
                /*}*/

                /*printf("%d\t", c);*/
                /*perm_print(p);*/
                /*printf("\n");*/

                /*p = perm_remove_entry(p, i);*/
        /*}*/
/*}*/



int main(int argc, char** argv) 
{
        if (argc != 4) {
                printf("%s <pattern> <n> <tally file>\n", argv[0]);
                return 0;
        }

        char *pattern_string = argv[1];
        char *tally_file     = argv[3];
        int n                = atoi(argv[2]);

        Log = fopen(tally_file, "w+");

        perm_t pattern = perm_from_string(pattern_string);
        perm_t perm    = perm_of_length(n);

        Total = factorial((uint64_t)n);

        generate_permutations_fast(perm, pattern);
        /*permute(perm, pattern, 0, perm_length(perm)-1); */

        /*#ifdef GO_FAST*/
        write_tally(Log);
        /*#endif*/

        return 1;
}

