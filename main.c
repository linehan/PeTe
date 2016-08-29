#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <assert.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>

/* For PRu64 format macro to printf() */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "hashtable.h"

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

int nchoosek(int n, int k)
{
        return (n==0 | k==0) ? 1 : (n*nchoosek(n-1,k-1))/k;
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


/******************************************************************************
 * FASTER 
 ******************************************************************************/
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

int BM_pfast_call_count = 0; 

int pfast(int i, perm_t perm, perm_t pattern, int perm_len, int pattern_len) 
{
        BM_pfast_call_count++;

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

int BM_pffast_cache_misses = 0;

int pffast(int i, perm_t perm, perm_t pattern, int perm_len, int pattern_len, struct hashtable *ht) 
{
        BM_pfast_call_count++;

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

        perm_t p = 0;

        hash_get(ht, perm+(i+1), &p);

        if (p == 0) {
                BM_pffast_cache_misses++; 
                p = downarrow(perm, i+1);
                hash_put(ht, perm+(i+1), p);
        }

        return pffast(i, p, pattern, perm_len-1, pattern_len, ht) + pffast(i+1, perm, pattern, perm_len, pattern_len, ht);
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

int countffast(perm_t perm, perm_t pattern, int perm_len, int pattern_len) 
{
        struct hashtable ht;
        hash_init_alloc(&ht, 65536);

        return pffast(0, perm, pattern, perm_len, pattern_len, &ht);
}


float occ(int n, int k, float density)
{
        return (float)nchoosek(n, k) * density;
}

void densities(int n, int k)
{
	int max = nchoosek(n, k);
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

void idx_print(int *idx, int n)
{
        int i;

        for (i=0; i<n; i++) {
                if (i != n-1) {
                        printf("%d,", idx[i]);
                } else {
                        printf("%d", idx[i]);
                }
        }
}

void get_permutation_start_states(int N, int num_blocks) 
{
        int idx[4096];
        int i;
        int t;
        int M;
        int c;

        printf("computing...factorial?\n");
        int block_size = factorial(N)/num_blocks;
        int block_count = 0;

        printf("BLOCK SIZE: %d\n", block_size);

        perm_t p = perm_of_length(N);

        for (i=1; i<=N; i++) {
                perm_set_block(p, i-1, i-1);
                idx[i] = 1;
        }

        printf("P[0]: ");
        perm_print(p);
        printf("\n");
        printf("I[0]: ");
        idx_print(idx, N);
        printf("\n");
        printf("i[0]: 0\n", i);
        fflush(stdout);

        Total = 1;
        block_count = 1;

        for (i=1; i<=N;) {
                if (idx[i] < i) {
                        if (i % 2) {
                                p = perm_swap(p, 0, i-1);
                        } else {
                                p = perm_swap(p, idx[i]-1, i-1);
                        }
                        idx[i]++;
                        i = 1;

                        if ((Total++ % 1000) == 0) {
                                fprintf(stderr, "%"PRIu64"\n", Count);
                        }

                        if (Count++ == block_size) {
                                printf("P[%d]: ", block_count);
                                perm_print(p);
                                printf("\n");
                                printf("I[%d]: ", block_count);
                                idx_print(idx, N);
                                printf("\n");
                                printf("i[%d]: %d\n", block_count++, i);
                                fflush(stdout);
                                Count = 0;
                        }
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


/*struct runner_t {*/
        /*perm_t start;*/
        /*perm_t stop;*/
        /*int    idx[4096];*/
        /*int    i;*/
/*};*/

/*struct runner_t Runner[64] = {0};*/
/*int Runner_count = 0;*/
/*int Runner_perm_length = 0;*/



perm_t perm_from_csv(char *csv_string)
{
        /* 
         * strtok_r() mutates its argument string, so we must
         * copy the (possibly immutable e.g. a string literal)
         * argument @csv to this mutable buffer before proceeding.
         */
        char csv[4096];

        char *tok; /* token */
        char *ptr; /* pointer */

        perm_t perm = 0;
        int i = 0;
        int value;

        /* Copy csv to buffer, making it mutable. */
        strncpy(csv, csv_string, 4096); 

        /* Make a pointer to the buffer for strtok_r() to use. */
        ptr = csv;

        for (tok=strtok_r(csv,",",&ptr);tok!=NULL;tok=strtok_r(NULL,",",&ptr)) {
                sscanf(tok, "%d", &value);
                /* 
                 * Assume the permutation digits start from 0 
                 * Otherwise we would use (value - 1).
                 */
                perm = perm_set_block(perm, i++, (uint64_t)value);
        }

        return perm;
}

void array_from_csv(int *array, char *csv)
{
        char *t; /* token */
        char *p; /* pointer */
        int value;

        int i = 0;

        for (t=strtok_r(csv, ",", &p); t!=NULL; t=strtok_r(NULL, ",", &p)) {
                sscanf(t, "%d", &value);
                array[i++] = value;
        }
}

/*void prepare_run(FILE *config_file)*/
/*{*/
        /*int i;*/
        /*int sections = 0;*/
        /*int perm_len = 0;*/

        /*char buffer[4096];*/

        /*perm_t last_perm = 0;*/


        /*[> Read the first line, like a header. <]*/
        /*fgets(buffer, 4096, config_file);*/
        /*sscanf(buffer, "%d %d\n", &sections, &perm_len);*/

        /*Runner_count = sections;*/
        /*Runner_perm_length = perm_len;*/

        /*for (i=0; i<sections; i++) {*/
                /*fgets(buffer, 4096, config_file);*/

                /*Runner[i].start = perm_from_csv(buffer);*/

                /*if (i>0) {*/
                        /*Runner[i-1].stop = Runner[i].start;*/
                /*}*/

                /*fgets(buffer, 4096, config_file);*/

                /*array_from_csv(Runner[i].idx, buffer); */

                /*fgets(buffer, 4096, config_file);*/

                /*sscanf(buffer, "%d\n", &Runner[i].i);*/
        /*}*/
/*}*/


void run_from_state(perm_t start, perm_t stop, perm_t pattern, int *index, int i_0)
{
        int M;
        int N;
        int c;
        int i;
        int idx[4096];

        perm_t p = start;

        N = perm_length(p);
        M = perm_length(pattern);

        Total = factorial(N)/64;

        for (i=1; i<=N; i++) {
                idx[i] = index[i];
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


        for (i=i_0; i<=N;) {
                if (idx[i] < i) {
                        if (i % 2) {
                                p = perm_swap(p, 0, i-1);
                        } else {
                                p = perm_swap(p, idx[i]-1, i-1);
                        }
                        idx[i]++;
                        i = 1;

                        if (p == stop) {
                                exit(0);
                        }

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


void write_tally_file(FILE *f, uint64_t *tally, int n)
{
        int i;

        /* Rewind file */
        fseek(f, 0L, SEEK_SET);

        for (i=0; i<n; i++) {
                fprintf(f, "%d %d\n", i, tally[i]); 
        }
}

void write_progress_file(FILE *f, uint64_t count, uint64_t total)
{
        int i;

        /* Rewind file */
        fseek(f, 0L, SEEK_SET);
        fprintf(f, "\r(%f%%) %"PRIu64"/%"PRIu64"", ((float)count/(float)total)*100, count, total);
}


struct thread_args_t {
        perm_t start;
        perm_t stop;
        perm_t pattern;
        int    index[16]; 
        int    i0; 
        int    thread_id;
        uint64_t total;
};


void prepare_to_thread(int perm_len, perm_t pattern, int nthreads, struct thread_args_t *args) 
{
        int index[32];
        uint64_t block_size;
        int current_block = 0;
        int count = 0;
        int i;
        int j;
        perm_t p;

        block_size = (uint64_t)factorial(perm_len) / (uint64_t)nthreads;
        p          = perm_of_length(perm_len);

        for (i=1; i<=perm_len; i++) {
                perm_set_block(p, i-1, i-1);
                index[i] = 1;
        }

        args[0].start     = p;
        args[0].pattern   = pattern;
        args[0].i0        = 1;
        args[0].thread_id = 0;
        args[0].total     = block_size;
        for (j=0; j<16; j++) {
                args[0].index[j] = index[j];
        }

        current_block = 1;

        for (i=1; i<=perm_len;) {
                if (index[i] < i) {
                        if (i % 2) {
                                p = perm_swap(p, 0, i-1);
                        } else {
                                p = perm_swap(p, index[i]-1, i-1);
                        }
                        index[i]++;
                        i = 1;

                        if (count++ == block_size) {
                                args[current_block-1].stop = p;
                                args[current_block].start = p;
                                args[current_block].pattern = pattern;
                                args[current_block].i0 = i;
                                args[current_block].thread_id = current_block;
                                args[current_block].total     = block_size;
                                for (j=0; j<16; j++) {
                                        args[current_block].index[j] = index[j];
                                }

                                current_block++;

                                if (current_block > nthreads) {
                                        fprintf(stderr, "too many blocks!\n");
                                        exit(1);
                                }

                                count = 0;
                        }
                } else {
                        index[i++] = 1;
                }
        }

        args[current_block].stop = 0;
}

void do_block(perm_t start, perm_t stop, perm_t pattern, int *index, int i0, uint64_t *tally, FILE *logfile, FILE *tallyfile, int tally_n, uint64_t total)
{
        int M;
        int N;
        int c;
        int i;
        int idx[4096];
        uint64_t track = 0;
        uint64_t count = 0;

        perm_t p = start;

        N = perm_length(p);
        M = perm_length(pattern);

        for (i=1; i<=N; i++) {
                idx[i] = index[i];
        }

        /* DO THE THING */
        c = countfast(p, pattern, N, M);
        tally[c]++;
        /* DONE DOING THE THING */


        for (i=i0; i<=N;) {
                if (idx[i] < i) {
                        if (i % 2) {
                                p = perm_swap(p, 0, i-1);
                        } else {
                                p = perm_swap(p, idx[i]-1, i-1);
                        }
                        idx[i]++;
                        i = 1;

                        if (p == stop) {
                                write_tally_file(tallyfile, tally, tally_n);
                                return;
                        }

                        /* DO THE THING */
                        c = countfast(p, pattern, N, M);
                        tally[c]++;
                        count++;

                        if (track++ == 10000) {
                                write_progress_file(logfile, count, total);
                                track=0;
                        }
                        /* DONE DOING THE THING */

                } else {
                        idx[i++] = 1;
                }
        }
}




void *count_in_thread(void *args)
{
        struct thread_args_t *A = (struct thread_args_t *)args;

        printf("Thread [%d] online!\n", A->thread_id);

        int perm_len = perm_length(A->start);
        int patt_len = perm_length(A->pattern);

        int max_tally = nchoosek(perm_len, patt_len);

        uint64_t *tally = calloc(1, max_tally*sizeof(uint64_t));

        char filename[4096];

        sprintf(filename, "n%d-k%d-t%d.tally", perm_len, patt_len, A->thread_id);

        FILE *tallyfile = fopen(filename, "w+");

        sprintf(filename, "n%d-k%d-t%d.log", perm_len, patt_len, A->thread_id);

        FILE *logfile = fopen(filename, "w+");

        /* NOW LETS DO THE THING */
        do_block(
                A->start,
                A->stop,
                A->pattern,
                A->index,
                A->i0,
                tally,
                logfile,
                tallyfile,
                max_tally,
                A->total
        );

        return NULL;
}



void count_threaded(perm_t pattern, int perm_len, int nthreads)
{
        pthread_t threads[64];
        struct thread_args_t thread_args[64];
        int rc;
        int i;
       
        if (nthreads > 32) {
                printf("too many threads!\n");
                exit(1);
        }

        printf("preparing to thread...\n");
        prepare_to_thread(perm_len, pattern, nthreads, thread_args);
        printf("preparing to thread...done\n");


        for (i=0; i<nthreads; i++) { 
                printf("spawning thread %d\n", i);

                rc = pthread_create(&threads[i], NULL, count_in_thread, (void *)&thread_args[i]);
        }

        /* Wait for threads to finish */
        for (i=0; i<nthreads; i++) {
                rc = pthread_join(threads[i], NULL);
        }
}



/*perm_t Start[64];*/
/*perm_t Stop[64];*/
/*perm_t Index[64][16];*/
/*perm_t I_0[64];*/

/*int NUM_SECTIONS = 0;*/
/*int PERM_LENGTH  = 0;*/


inline uint64_t rdtsc() 
{
        uint32_t lo;
        uint32_t hi;

        __asm__ __volatile__ (
                /* Flush the CPU instruction pipeline first */
                "xorl %%eax, %%eax\n"
                "cpuid\n"
                /* Get RDTSC counter in ebx:ecx */
                "rdtsc\n"
                : "=a" (lo), "=d" (hi)
                :
                : "%ebx", "%ecx"
        );
        return (uint64_t)hi << 32 | lo;
}

int main(int argc, char *argv[]) 
{
        if (argc == 1) {
                printf("USAGE:\n");
                printf("%s --benchmark\n", argv[0]);
                printf("%s --tally                          <pattern> <n> <tally file>\n", argv[0]);
                printf("%s --tally-multithread-config-write <perm_len> <num_blocks>\n", argv[0]);
                printf("%s --tally-multithread-config-read  <pattern> <config_path> \n", argv[0]);
                printf("%s --tally-from-start-state         <start> <stop> <idx> <i> <pattern> \n", argv[0]);

        } else if (!strcmp(argv[1], "--benchmark")) {

                /*struct hashtable ht;*/
                /*hash_init_alloc(&ht, 65536); */

                perm_t perm = perm_from_csv("12,6,4,0,3,2,5,7,8,11,1,10,9");
                perm_t patt = perm_from_csv("3,1,0,2");

                /*uint64_t val;*/
                /*hash_put(&ht, perm, 13); */
                /*hash_get(&ht, perm, &val);*/
                /*printf("got %"PRIu64"\n", val);*/

                uint64_t t0;
                uint64_t t1;
                int   count;
               
                t0    = rdtsc(); 
                count = countfast(perm, patt, 13, 4); 
                t1    = rdtsc(); 

                printf("countfast() computed for n=13 k=4 in %"PRIu64" cycles\n", t1-t0);
                printf("pfast() called %d times\n", BM_pfast_call_count);


                t0    = rdtsc(); 
                count = countffast(perm, patt, 13, 4); 
                t1    = rdtsc(); 

                printf("countffast() computed for n=13 k=4 in %"PRIu64" cycles\n", t1-t0);
                printf("cache missed %d times\n", BM_pffast_cache_misses);

        } else if (!strcmp(argv[1], "--tally")) {
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
        } else if (!strcmp(argv[1], "--tally-multithread")) {

                perm_t pattern = perm_from_csv(argv[2]);
                int n = atoi(argv[3]); 
                int threadcount = atoi(argv[4]);

                count_threaded(pattern, n, threadcount);

        } else if (!strcmp(argv[1], "--tally-multithread-config-read")) {

                /*char buffer[4096];*/
                /*int i;*/

                /*perm_t pattern = perm_from_csv(argv[2]);*/

                /*FILE *config = fopen(argv[3], "w+");*/

                /*[> Read the first line, like a header. <]*/
                /*fgets(buffer, 4096, config);*/
                /*sscanf(buffer, "%d %d\n", &NUM_SECTIONS, &PERM_LENGTH);*/

                /*for (i=0; i<NUM_SECTIONS; i++) {*/
                        /*fgets(buffer, 4096, config);*/

                        /*Start[i] = perm_from_csv(buffer);*/

                        /*if (i>0) {*/
                                /*Stop[i-1] = Start[i];*/
                        /*}*/

                        /*fgets(buffer, 4096, config);*/
                        /*array_from_csv(Index[i], buffer); */
                        /*fgets(buffer, 4096, config);*/
                        /*sscanf(buffer, "%d\n", &I_0[i]);*/
                /*}*/

                /*for (i=0; i<NUM_SECTIONS; i++) {*/
                        /*[> ... <]*/
                /*}*/
        } else if (!strcmp(argv[1], "--tally-multithread-config-write")) {
                /*int N = atoi(argv[1]);*/
                /*int B = atoi(argv[2]);*/

                /*get_permutation_start_states(N, B);*/

        } else if (!strcmp(argv[1], "--tally-from-start-state")) {
                perm_t start   = perm_from_csv(argv[1]);
                perm_t stop    = perm_from_csv(argv[2]);
                perm_t pattern = perm_from_csv(argv[5]);
                int idx[100];
                array_from_csv(idx, argv[4]);
                int i = atoi(argv[5]);

                run_from_state(start, stop, pattern, idx, i);
        } else {
                printf("I don't understand.\n\nUSAGE:\n");
                printf("%s --benchmark\n", argv[0]);
                printf("%s --tally                          <pattern> <n> <tally file>\n", argv[0]);
                printf("%s --tally-multithread-config-write <perm_len> <num_blocks>\n", argv[0]);
                printf("%s --tally-multithread-config-read  <pattern> <config_path> \n", argv[0]);
                printf("%s --tally-from-start-state         <start> <stop> <idx> <i> <pattern> \n", argv[0]);
        }

        return 1;
}

