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

#include "hashtable.h"
#include "perm.h"
#include "pset.h"
#include "io.h"

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
                        /*printf("gotta change block\n");*/
                        /*fflush(stdout);*/
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

        return pfast(i, downarrowfast(perm, i+1, perm_len), pattern, perm_len-1, pattern_len) + pfast(i+1, perm, pattern, perm_len, pattern_len);
}

/*int BM_pffast_cache_misses = 0;*/

int pffast(int i, perm_t perm, perm_t pattern, int perm_len, int pattern_len, struct hashtable *ht) 
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

        perm_t p = 0;

        hash_get(ht, perm+(i+1), &p);

        if (p == 0) {
                /*BM_pffast_cache_misses++; */
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

void generate_permutations_fast_classes(perm_t p, perm_t pattern, struct pset_t *classes) 
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
        pset_add(&classes[c], p);

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
                        pset_add(&classes[c], p);

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


struct pargs_t {
        perm_t start;
        perm_t pattern;
        perm_t stop;       /* Only used when resuming */
        int    index[16];  /* Only used when resuming */
        int    i0;         /* Only used when resuming */
        int    thread_id;
        uint64_t blocksize; /*  How many perms this thread must do */
        FILE  *logfile;
        FILE  *tallyfile;
        uint64_t *tally;
        struct pset_t *class;
        char *path;
        uint64_t n;
        uint64_t k;
        uint64_t nchoosek;
        uint64_t nfactorial;
};


void run_p(struct pargs_t *P)
{
        int n;
        int k;
        int c;
        int i;
        int i0;
        uint64_t *tptr;
        struct pset_t *cptr;
        int *iptr;
        perm_t p;
        perm_t stop;
        perm_t pattern;
        uint64_t track = 0;
        uint64_t count = 0;

        p       = P->start;
        stop    = P->stop;
        pattern = P->pattern;

        n = P->n;
        k = P->k;

        tptr = P->tally;
        iptr = P->index;
        cptr = P->class;

        i0 = P->i0;

        /******************************
         * Initialize if necessary 
         ******************************/
        if (stop == 0) {
                for (i=1; i<=n; i++) {
                        perm_set_block(p, i-1, i-1);
                        iptr[i] = 1;
                }
        }

        /******************************
         * Take first count
         ******************************/
        c = countfast(p, pattern, n, k);
        tptr[c]++;
        pset_add(&cptr[c], p);

        /******************************
         * Iterative Heap's algorithm
         ******************************/
        for (i=i0; i<=n;) {
                if (iptr[i] < i) {
                        if (i % 2) {
                                p = perm_swap(p, 0, i-1);
                        } else {
                                p = perm_swap(p, iptr[i]-1, i-1);
                        }

                        iptr[i]++;
                        i = 1;

                        if (p == stop) {
                                /* EXIT THE LOOP -- DONE */
                                /* NOTE
                                 * If stop == 0, that indicates
                                 * we will do the whole thing, since
                                 * 0 is not a valid permutation, hence
                                 * p will never be equal to it, and the 
                                 * loop will terminate due to i being == n 
                                 * (in the for condition).
                                 */
                                break;
                        }

                        c = countfast(p, pattern, n, k);
                        tptr[c]++;
                        count++;
                        pset_add(&cptr[c], p);

                        if (track++ == 10000) {
                                /* Write progress every 10,000 perms */
                                write_progress_file(P->logfile, count, P->blocksize);
                                track=0;
                        }
                } else {
                        iptr[i++] = 1;
                }
        }

        write_tally_file(P->tallyfile, tptr, P->nchoosek);
}


/*void tally_with_classes(perm_t perm, perm_t pattern) */
/*{*/
        /*uint64_t n = perm_length(perm);*/
        /*uint64_t k = perm_length(pattern);*/
        /*uint64_t nfactorial = factorial((uint64_t)n);*/
        /*uint64_t nchoosek = nchoosek(n, k);*/

        /*****************************
         * Make the project directory 
         *****************************/

        /*char path[4096];*/

        /*[> Collect current datetime as a string <]*/
        /*char datestring[16];*/
        /*time_t now;*/
        /*struct tm *today;*/

        /*time(&now);*/
        /*today = localtime(&now);*/

        /*strftime(date, 15, "%Y%m%d", today);*/

        /*[> Make the pattern into a digit string <]*/
        /*char *pstr = perm_get_string(pattern);*/

        /*[> Print the project path <]*/
        /*sprintf(path, "%d-%s-%s", n, pstr, datestring);*/

        /*mkdir(DIR_PERMS, path); */

        /*[> Open the project logfile and tallyfile <]*/
        /*FILE *logfile   = fopenf("w+", "%s/%d-%s.log", path, n, pstr);*/
        /*FILE *tallyfile = fopenf("w+", "%s/%d-%s.tally", path, n, pstr);*/

        /*************************
         * Create the class psets
         *************************/

        /*int nclasses = nchoosek(n, perm_length(pattern));*/

        /*struct pset_t *classes = calloc(1, nclasses*sizeof(struct pset_t));*/

        /*int i;*/
        /*for (i=0; i<nchoosek; i++) {*/
                /*pset_init(&classes[i], n);*/
        /*}*/

        /*************************
         * Build the pargs object 
         *************************/
        /*struct pargs_t A;*/

        /*A.start = perm;*/
        /*A.stop  = 0;*/
        /*A.pattern = pattern;*/
        /*A.logfile   = logfile;*/
        /*A.tallyfile = tallyfile;*/
        /*A.n        = n;*/
        /*A.k        = k;*/
        /*A.nchoosek = nchoosek;*/
        /*A.nfactorial = nfactorial;*/
        /*A.tally = calloc(1, nchoosek * sizeof(uint64_t));*/
        /*A.class = calloc(1, nchoosek * sizeof(struct pset_t));*/

        /*for (i=0; i<nchoosek; i++) {*/
                /*pset_init(&A.class[i], n);*/
        /*}*/

        /*run_p(&A);*/

        /*
         * Print class files and tally file
         */
        /*for (i=0; i<nclasses; i++) {*/
                /*if (A->class[i].size != 0) {*/
                        /*FILE *f = fopenf("w+", "%d.%s-%s/class.%d.trace", n, pstr, datestring, i);*/
                        /*pset_write(&A.class[i], f);*/
                /*}*/
        /*}*/
/*}*/


void *run_tally_with_classes(void *A) 
{
        struct pargs_t *P = (struct pargs_t *)A;

        fprintf(stderr, "Thread %d is active! My blocksize is %"PRIu64"\n", P->thread_id, P->blocksize);

        int i;

        char *pstr = perm_get_string(P->pattern);

        /* Open the project logfile and tallyfile */
        P->logfile   = fopenf("w+", "%s/%d-%s.t%d.log", P->path, P->n, pstr, P->thread_id);
        P->tallyfile = fopenf("w+", "%s/%d-%s.t%d.tally", P->path, P->n, pstr, P->thread_id);

        free(pstr);

        run_p(P);

        /*
         * Print class files and tally file
         */
        for (i=0; i<P->nchoosek; i++) {
                if (P->class[i].size != 0) {
                        FILE *f = fopenf("w+", "%s/class.%d.t%d.trace", P->path, i, P->thread_id);
                        pset_write(&(P->class[i]), f);
                }
        }

        return NULL;
}


void tally_with_classes(perm_t perm, perm_t pattern, int nthreads)
{
        if (nthreads > 64) {
                printf("too many threads!\n");
                exit(1);
        }

        printf("preparing to thread...\n");

        uint64_t n = perm_length(perm);
        uint64_t k = perm_length(pattern);
        uint64_t nfactorial = factorial(n);
        uint64_t _nchoosek = nchoosek(n, k);

        /*****************************
         * Make the project directory 
         *****************************/

        char path[4096];

        /* Collect current datetime as a string */
        char datestring[16];
        time_t now;
        struct tm *today;

        time(&now);
        today = localtime(&now);

        strftime(datestring, 15, "%Y%m%d", today);

        /* Make the pattern into a digit string */
        char *pstr = perm_get_string(pattern);

        /* Print the project path */
        sprintf(path, "%d-%s-%s", n, pstr, datestring);

        free(pstr);

        mkdir(path, DIR_PERMS); 

        struct pargs_t *args = calloc(1, nthreads*sizeof(struct pargs_t));

        int i;
        int j;

        /* Set up entries common to all threads */
        for (i=0; i<nthreads; i++) {
                args[i].pattern   = pattern;
                args[i].thread_id = i;
                args[i].n = n;
                args[i].k = k;
                args[i].nchoosek = _nchoosek;
                args[i].nfactorial = nfactorial;
                args[i].path = strdup(path);
                args[i].tally = calloc(1, _nchoosek*sizeof(uint64_t));
                args[i].class = calloc(1, _nchoosek*sizeof(struct pset_t));
                for (j=0; j<_nchoosek; j++) {
                        pset_init(&(args[i].class[j]), n);
                }
        }


        /*************************************************
         * Run through the permutation list once to init 
         *************************************************/
        int index[100];
        uint64_t blocksize = (uint64_t)nfactorial / (uint64_t)nthreads;
        uint64_t count = 0;
        int block = 0;

        fprintf(stderr, "Making %d blocks of size %"PRIu64"/%"PRIu64"\n", nthreads, blocksize, nfactorial);

        for (i=1; i<=n; i++) {
                perm_set_block(perm, i-1, i-1);
                index[i] = 1;
        }

        args[0].start = perm;
        args[0].i0 = 1;
        args[0].blocksize = blocksize;
        for (j=0; j<16; j++) {
                args[0].index[j] = index[j];
        }

        for (i=1; i<=n;) {
                if (index[i] < i) {
                        if (i % 2) {
                                perm = perm_swap(perm, 0, i-1);
                        } else {
                                perm = perm_swap(perm, index[i]-1, i-1);
                        }
                        index[i]++;
                        i = 1;

                        if (count++ == blocksize) {
                                block += 1;
                                args[block].blocksize = blocksize;
                                args[block-1].stop = perm;
                                args[block].start = perm;
                                args[block].i0 = i;
                                for (j=0; j<16; j++) {
                                        args[block].index[j] = index[j];
                                }

                                fprintf(stderr, "Block %d/%d complete after %"PRIu64"\n", block, nthreads, count-1);

                                if (block > nthreads) {
                                        fprintf(stderr, "too many blocks!\n");
                                        exit(1);
                                }

                                count = 0;
                        }
                } else {
                        index[i++] = 1;
                }
        }

        fprintf(stderr, "Block %d/%d complete after %"PRIu64"\n", block, nthreads, count-1);

        args[block].stop = UINT64_MAX;

        /*************************************************
         * Run each of the args in its own thread 
         *************************************************/
        pthread_t threads[64];
        int rc;
       
        for (i=0; i<nthreads; i++) { 
                rc = pthread_create(&threads[i], NULL, run_tally_with_classes, (void *)&args[i]);
        }

        /* Wait for threads to finish */
        for (i=0; i<nthreads; i++) {
                rc = pthread_join(threads[i], NULL);
        }

        /* Combine all the tallies and class counts into a single one */
        uint64_t *tally = calloc(1, _nchoosek * sizeof(uint64_t));
        struct pset_t *class = calloc(1, _nchoosek *sizeof(struct pset_t));;

        for (i=0; i<_nchoosek; i++) {
                pset_init(&(class[i]), n);
        }

        for (i=0; i<nthreads; i++) {
                for (j=0; j<_nchoosek; j++) {
                        tally[j] += args[i].tally[j];
                        pset_sum(&class[j], &(args[i].class[j]));
                }
        }


        
        /***************** 
         * write it all
         *****************/
        pstr = perm_get_string(pattern);

        /* Open the project logfile and tallyfile */
        FILE *tallyfile = fopenf("w+", "%s/%d-%s.tally", path, n, pstr);

        free(pstr);

        for (i=0; i<_nchoosek; i++) {
                if (class[i].size != 0) {
                        FILE *f = fopenf("w+", "%s/class.%d.trace", path, i);
                        pset_write(&(class[i]), f);
                }
        }

        write_tally_file(tallyfile, tally, _nchoosek);
}


/*void tally_with_classes_threaded(perm_t p, perm_t pattern, int nthreads) */
/*{*/
        /*int index[32];*/
        /*uint64_t block_size;*/
        /*int current_block = 0;*/
        /*int count = 0;*/
        /*int i;*/
        /*int j;*/
        /*perm_t p;*/

        /*block_size = (uint64_t)factorial(perm_len) / (uint64_t)nthreads;*/
        /*p          = perm_of_length(perm_len);*/

        /*for (i=1; i<=perm_len; i++) {*/
                /*perm_set_block(p, i-1, i-1);*/
                /*index[i] = 1;*/
        /*}*/

        /*args[0].start     = p;*/
        /*args[0].pattern   = pattern;*/
        /*args[0].i0        = 1;*/
        /*args[0].thread_id = 0;*/
        /*args[0].total     = block_size;*/
        /*for (j=0; j<16; j++) {*/
                /*args[0].index[j] = index[j];*/
        /*}*/

        /*current_block = 1;*/

        /*for (i=1; i<=perm_len;) {*/
                /*if (index[i] < i) {*/
                        /*if (i % 2) {*/
                                /*p = perm_swap(p, 0, i-1);*/
                        /*} else {*/
                                /*p = perm_swap(p, index[i]-1, i-1);*/
                        /*}*/
                        /*index[i]++;*/
                        /*i = 1;*/

                        /*if (count++ == block_size) {*/
                                /*args[current_block-1].stop = p;*/
                                /*args[current_block].start = p;*/
                                /*args[current_block].pattern = pattern;*/
                                /*args[current_block].i0 = i;*/
                                /*args[current_block].thread_id = current_block;*/
                                /*args[current_block].total     = block_size;*/
                                /*for (j=0; j<16; j++) {*/
                                        /*args[current_block].index[j] = index[j];*/
                                /*}*/

                                /*current_block++;*/

                                /*if (current_block > nthreads) {*/
                                        /*fprintf(stderr, "too many blocks!\n");*/
                                        /*exit(1);*/
                                /*}*/

                                /*count = 0;*/
                        /*}*/
                /*} else {*/
                        /*index[i++] = 1;*/
                /*}*/
        /*}*/

        /*args[current_block].stop = 0;*/
/*}*/



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

        } else if (!strcmp(argv[1], "--pset")) {

                struct pset_t set; 

                perm_t a = perm_from_csv("12,6,4,0,3,2,5,7,8,11,1,10,9");
                perm_t b = perm_from_csv("12,6,4,8,3,2,5,7,0,11,1,10,9");
                perm_t c = perm_from_csv("1,6,4,8,3,2,5,7,0,11,12,10,9");

                pset_init(&set, 13);

                pset_add(&set, a);
                pset_add(&set, b);
                pset_add(&set, c);

                FILE *f = fopen("pset.test", "w+");

                pset_write(&set, f);

        } else if (!strcmp(argv[1], "--benchmark")) {

                perm_t perm = perm_from_csv("12,6,4,0,3,2,5,7,8,11,1,10,9");
                perm_t patt = perm_from_csv("3,1,0,2");

                uint64_t t0;
                uint64_t t1;
                int   count;
               
                t0    = rdtsc(); 
                count = countfast(perm, patt, 13, 4); 
                t1    = rdtsc(); 

                printf("countfast() computed for n=13 k=4 in %"PRIu64" cycles\n", t1-t0);
                /*printf("pfast() called %d times\n", BM_pfast_call_count);*/


                t0    = rdtsc(); 
                count = countffast(perm, patt, 13, 4); 
                t1    = rdtsc(); 

                printf("countffast() computed for n=13 k=4 in %"PRIu64" cycles\n", t1-t0);
                /*printf("cache missed %d times\n", BM_pffast_cache_misses);*/

        } else if (!strcmp(argv[1], "--tally-with-classes-multithread")) {

                char *pattern_string = argv[2];
                int n                = atoi(argv[3]);
                int threadcount      = atoi(argv[4]);

                perm_t pattern = perm_from_csv(pattern_string);
                perm_t perm    = perm_of_length(n);

                tally_with_classes(perm, pattern, threadcount);

                /*[> Make project path and files <]*/
                /*char path[4096];*/
                /*char *pstr = perm_get_string(pattern);*/

                /*sprintf(path, "%d-%s-%s", n, pstr, date);*/

                /*mkdir(DIR_PERMS, path); */

                /*FILE *logfile   = fopenf("w+", "%s/%d-%s.log", path, n, pstr);*/
                /*FILE *tallyfile = fopenf("w+", "%s/%d-%s.tally", path, n, pstr);*/

                /*[>***********************<]*/
                /*int nclasses = nchoosek(n, perm_length(pattern));*/

                /*struct pset_t *classes = calloc(1, nclasses*sizeof(struct pset_t));*/

                /*int i;*/
                /*for (i=0; i<nclasses; i++) {*/
                        /*pset_init(&classes[i], n);*/
                /*}*/
                /*[>***********************<]*/

                /*Total = factorial((uint64_t)n);*/

                /*generate_permutations_fast_classes(perm, pattern, classes);*/
                

                /*
                 * Print class files and tally file
                 */

                /*struct tm *today;  */
                /*char date[16];*/

                /*//get current date  */
                /*today = localtime(&now);*/

                /*strftime(date, 15, "%Y%m%d", today);*/

                /*char *pattstr = perm_get_string(pattern);*/

                /*fmkdir(DIR_PERMS, "%d.%s-%s", n, pattstr, date); */

                /*for (i=0; i<nclasses; i++) {*/
                        /*if (classes[i].size != 0) {*/
                                /*FILE *f = fopenf("w+", "%d.%s-%s/class.%d.trace", n, pattstr, date, i);*/
                                /*pset_write(&classes[i], f);*/
                        /*}*/
                /*}*/

                /*write_tally(Log);*/

        } else if (!strcmp(argv[1], "--tally")) {
                char *pattern_string = argv[1];
                char *tally_file     = argv[3];
                int n                = atoi(argv[2]);

                Log = fopen(tally_file, "w+");

                perm_t pattern = perm_from_string(pattern_string);
                perm_t perm    = perm_of_length(n);

                /*************************/
                int nclasses = nchoosek(perm_length(perm), perm_length(pattern));

                struct pset_t *classes = calloc(1, nclasses*sizeof(struct pset_t));

                int i;
                for (i=0; i<nclasses; i++) {
                        pset_init(&classes[i], n);
                }
                /*************************/

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

