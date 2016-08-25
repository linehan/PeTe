#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <assert.h>
#include <stdint.h>
#include <errno.h>

unsigned long long factorial(long long num) 
{
        int answer = 1;
  
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


/******************************************************************************
 * PERMUTATIONS 
 ******************************************************************************/

typedef uint64_t perm_t;

#define MAXPERMSIZE 16
#define LETTERSIZE 4
#define numbits 64

#define LETTERFACE ((((perm_t) 1) << LETTERSIZE) - (perm_t)1)

#define MAX(a,b) \
        ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a > _b ? _a : _b; })


#define MIN(a,b) \
        ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a < _b ? _a : _b; })


uint64_t perm_get_entry(perm_t perm, int index) 
{
        return (uint64_t)((perm >> (index * LETTERSIZE)) & LETTERFACE); 
}

// indexing starts at 0 for both position and value in permutation
perm_t perm_set_entry(perm_t perm, int index, uint64_t value) 
{
        return (perm & ~((perm_t)LETTERFACE << (index * LETTERSIZE))) | ((perm_t)value << (index * LETTERSIZE)); // clear digit and then rewrite its value
}

// inserts a blank in position index
perm_t perm_insert_blank(perm_t perm, int index) 
{
        perm_t bottom = perm & (((perm_t)1 << (LETTERSIZE * index)) - 1);
        perm_t top    = perm & ((~ (perm_t)0) - (((perm_t)1 << (LETTERSIZE * index)) - 1));
        
        return bottom + (top << LETTERSIZE);
}


perm_t perm_insert_entry(perm_t perm, int index, uint64_t value) 
{
        return perm_set_entry(perm_insert_blank(perm, index), index, value);
}

perm_t perm_remove_entry(perm_t perm, int index) 
{
        perm_t bottom = perm & (((perm_t)1 << (LETTERSIZE * index)) - 1);
        perm_t top = perm & ((~ (perm_t)0) - (((perm_t)1 << (LETTERSIZE * index + LETTERSIZE)) - 1));

        if ((LETTERSIZE * index + LETTERSIZE) == numbits) { 
                return bottom; // top is ill-defined in this case
        }

        return bottom + (top >> LETTERSIZE); 
}


int perm_length(perm_t perm) 
{
        uint64_t len = 0;
        int i;
        
        for (i=0; i<numbits/LETTERSIZE; i++) {
                len = MAX(len, perm_get_entry(perm, i));
        }
  
        return (int)(len + 1);
}

// gives permuation corresponding with a string
inline perm_t stringtoperm(char *str) 
{
	perm_t perm = 0;
	int i;

  	for (i=0; i<strlen(str); i++) { 
		perm = perm_set_entry(perm, i, (uint64_t)(str[i] - '1'));
	}

  	return perm;
}


void print_bits(perm_t p)
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


void displayperm(perm_t perm) 
{
        int i;

        for (i=0; i<numbits/LETTERSIZE; i++) {
                printf("%d", perm_get_entry(perm, i));
        }
        /* printf("\n"); */
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
        
        /*for (int i = 0; i < numbits / LETTERSIZE; i++) {*/
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


// Recursively checks whether perm contains a pattern from patternset.
// Requires that all patterns in patternset are length currentpatternlength + numlettersleft
// currentpatterncomplement stores complement of normalization of permutation subsequence already examined
// largestletterused tells us the value of the largest letter icnluded so far in the permutation-subsequence being examined
// seenpos is a bitmap used to efficiently update currentpatterncomplement as we make the subsequence-being looked at one longer
// prefixmap contains the complements of each prefix of each pattern in \Pi.
// Note that prefixmap contains complements of normalized prefixes of patterns
// If USEADDFACTOR, computes P_1(perm) instead of P(perm). If USESECONDADDFACTOR, computes P_2(perm) instead
/*void checkpatterns(perm_t perm, perm_t inverse, perm_t cur_compl, int cur_compl_len, int largestletterused, int numlettersleft, uint32_t seenpos, perm_t pat_compl, int *count) */
/*{*/
        /*printf("cur_compl_len:%d\nlargestletterused:%d\nnumlettersleft:%d\n", cur_compl_len, largestletterused, numlettersleft);*/
        /*print_bits(pat_compl);*/
        /*print_bits(cur_compl);*/

        /*if (cur_compl != 0 && !prefix_matches(cur_compl, pat_compl, cur_compl_len)) {*/
		/*printf("done 1 (no match)\n");*/
                /*return;*/
        /*}*/
  
	/*// Assumes all patterns are same size --> this is only case where */
	/*// prefix is a pattern*/
        /*if (numlettersleft == 0) { */
		/*printf("done 2\n");*/
                /*(*count)++;*/
                /*return; // also assumes all patterns same size*/
        /*}*/
  
	/*// Similarly to as in extendnormalizetop (defined in perm.cpp), */
	/*// we will build the complement of the normalization of the new */
	/*// permutation-subsequence (with the new letter added to it)*/
	/*int oldpos = getdigit(inverse, largestletterused-1);*/
	/*int newpos = 0;*/

	/*// Note: shifting by 32 is ill-defined, which is why we explicitly eliminate digit = 0 case.*/
	/*if (oldpos != 0) {*/
		/*uint32_t temp = seenpos << (32 - oldpos); */
		/*newpos = __builtin_popcount(temp);*/
	/*}*/

        /*printf("oldpos:%d\n", oldpos);*/
        /*printf("newpos:%d\n", newpos);*/

	/*perm_t new_cur_compl = setdigit(addpos(cur_compl, newpos), newpos, cur_compl_len);*/

	/*// Recurse to make sequence longer until we eventually either do or don't get a pattern:*/
	/*checkpatterns(perm, inverse, new_cur_compl, cur_compl_len+1, largestletterused-1, numlettersleft-1, seenpos | (1 << oldpos), pat_compl, count);*/
        
	/*printf("done 3\n");*/
        /*return;*/
/*}*/

/*perm_t compute_downarrow2(perm_t perm, int i, int length)*/
/*{*/
        /*int j;*/
        /*int pos;*/

        /*[>printf("removing %d\n", length-i+1);<]*/

        /*for (j=0; j<length; j++) {*/
                /*if (getdigit(perm, j) == length-i+1) {*/
                        /*pos = j;*/
                        /*break;*/
                /*}*/
        /*}*/

        /*printf("removing %d at position %d\n", length-i+1, pos);*/

        /*perm = killpos(perm, pos);*/

        /*for (j=0; j<length-1; j++) {*/
                /*int digit = getdigit(perm, j);*/
                /*if (digit > length-i+1) {*/
                        /*printf("setting %d=>%d\n", digit, digit-1);*/
                        /*perm = setdigit(perm, j, digit-1);*/
                /*}*/
        /*}*/

        /*return perm;*/
/*}*/


/*perm_t compute_downarrow(perm_t perm, perm_t prev, int i, int length)*/
/*{*/
        /*perm_t inverse = getinverse(prev, length);*/

        /*int pos_to_set  = getdigit(inverse, length-i+1); */
        /*int pos_to_kill = getdigit(inverse, length-i);*/

        /*perm_t almost_result = setdigit(prev, pos_to_set, length-i);*/

        /*perm_t result = killpos(almost_result, pos_to_kill);*/

        /*return result;*/
/*}*/


/******************************************************************************
 * COUNTING PATTERNS IN THEM 
 ******************************************************************************/

perm_t downarrow(perm_t perm, int i)
{
        int pos;
        int j;

        int length = perm_length(perm);

        /*printf("len:%d i:%d\n", length, i);*/

        /* 
         * In paper written as length-i+1, but we index from 0 in
         * this implementation, so it requires another -1.  
         */
        int seek_digit = (length-i+1)-1;

        for (j=0; j<length; j++) {
                if (perm_get_entry(perm, j) == seek_digit) {
                        pos = j;
                        break;
                }
        }

        perm = perm_remove_entry(perm, pos);

        for (j=0; j<length-1; j++) {
                int digit = perm_get_entry(perm, j);
                if (digit > seek_digit) {
                        perm = perm_set_entry(perm, j, digit-1);
                }
        }

        return perm;
}


int p(int i, perm_t perm, perm_t pattern) 
{
        int perm_len    = perm_length(perm);
        int pattern_len = perm_length(pattern);

        /*printf("permlen[%d] pattlen[%d]\n", perm_len, pattern_len);*/

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


/*void counttt(perm_t perm, perm_t pattern, int perm_len, int pattern_len, int *count) */
/*{*/
        /*[>printf("cur_compl_len:%d\nlargestletterused:%d\nnumlettersleft:%d\n", cur_compl_len, largestletterused, numlettersleft);<]*/
        /*[>print_bits(pat_compl);<]*/
        /*[>print_bits(cur_compl);<]*/

        /*[>if (perm != 0 && !prefix_matches(perm, pattern, perm_len)) {<]*/
		/*[>printf("done 1 (no match)\n");<]*/
                /*[>return;<]*/
        /*[>}<]*/

        /*[>if (!prefix_matches<]*/
  
	/*// Assumes all patterns are same size --> this is only case where */
	/*// prefix is a pattern*/
        /*if (perm_len == pattern_len) { */
		/*[>printf("test matchxxx\n");<]*/
                /*[>if (prefix_matches(perm, pattern, perm_len)) {<]*/
                /*if (perm == pattern) {*/
			/*printf("[!!]\n");*/
                        /*(*count)++;*/
                        /*return; // also assumes all patterns same size*/
                /*} else {*/
                        /*printf("[no match]\n");*/
                        /*return;*/
                /*}*/
        /*}*/

        /*int i;*/

        /*for (i=pattern_len; i>0; i--) {*/
                /*printf("[%d:%d]\n", i, perm_len);*/
                /*perm_t minus = compute_downarrow2(perm, i+1, perm_len); */
                /*[>printf("\n");<]*/
                /*[>displayperm(perm);<]*/
                /*[>printf("\n");<]*/
                /*[>displayperm(minus);<]*/

                /*counttt(minus, pattern, perm_len-1, pattern_len, count);*/
        /*}*/
  
	/*printf("done 3\n");*/
        /*return;*/
/*}*/

/*void count_brute_force(perm_t perm, perm_t pattern, int perm_digit, int pattern_digit, int *count)*/
/*{*/
        /*int perm_len    = perm_length(perm);*/
        /*int pattern_len = perm_length(pattern);*/

        /*int i;*/
        /*int j;*/

        /*int di, dj, pi, pj;*/

        /*for (i=perm_digit; i<perm_len; i++) {*/
                /*di = getdigit(perm, i);*/

                /*for (j=i; j<perm_len; j++) {*/

                        /*dj = getdigit(perm, j);*/

                        /*pi = getdigit(pattern, pattern_digit);*/
                        /*pj = getdigit(pattern, pattern_digit+1);*/

                        /*[>printf("%d - %d >= %d - %d ?\n", di, dj, pi, pj);<]*/

                        /*if (((di>dj) && (pi<pj)) || ((di<dj) && (pi>pj))) {*/
                                /*[>printf("no match\n");<]*/
                        /*} else if (((di>dj) && (di-dj) >= (pi-pj)) */
                               /*||  ((di<dj) && (dj-di) >= (pj-pi))) {*/
                                /*[>printf("gonna do something\n");<]*/
                                /*if (pattern_digit == pattern_len - 2) {*/
                                        /*[>printf("%d,%d matches %d,%d [!!]\n", di, dj, pi, pj);<]*/
                                        /*printf("%d,%d[!]\n", di+1, dj+1);*/
                                        /*[>printf("yes\n");<]*/
                                        /*(*count)++;*/
                                        /*return;*/
                                /*} else {*/
                                        /*printf("%d,", di+1);*/
                                        /*[>printf("%d,%d matches %d,%d\n", di, dj, pi, pj);<]*/
                                        /*count_brute_force(perm, pattern, j, pattern_digit + 1, count);*/
                                /*}*/
                        /*}*/
                /*}*/

                /*[> failure... this gets reset <]*/
                /*printf("\33[2K\r");*/
                /*pattern_digit = 0;*/
        /*}*/
/*}*/




int countem(perm_t perm, perm_t pattern) 
{
	int perm_len = perm_length(perm);//getmaxdigit(perm);
	int pat_len  = perm_length(pattern);//getmaxdigit(pattern);

	/*perm_t perm_inverse = getinverse(perm, perm_len);*/

	int count = 0;

        /*count_brute_force(perm, pattern, 0, 0, &count);*/

	/*counttt(perm, (perm_t)0, pattern, 0, pat_len, pat_len+1, 0, &count);*/
        /*counttt(perm, pattern, perm_len, pat_len, &count);*/

        count = p(0, perm, pattern);

	/*printf("got count:%d\n", count);*/

	return count;
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



int Tally[560] = {0};

FILE *Log;

void write_tally(FILE *f)
{
        int i;

        /* Rewind file */
        fseek(f, 0L, SEEK_SET);

        for (i=0; i<560; i++) {
                fprintf(f, "%d %d\n", i, Tally[i]); 
        }
}




int Track = 0;


/*  Function to swap values at two pointers */
perm_t perm_swap(perm_t perm, int index_a, int index_b)
{
        int a = perm_get_entry(perm, index_a);
        int b = perm_get_entry(perm, index_b);

        perm = perm_set_entry(perm, index_a, b);
        perm = perm_set_entry(perm, index_b, a);
                
        return perm;
}
        
/*  Function to print permutations of string
*     This function takes three parameters:
*        1. String
*           2. Starting index of the string
*       3. Ending index of the string. */
void permute(perm_t p, perm_t pattern, int l, int r)
{
        int i;
        int c;
                    
        if (l == r) {

                c = countem(p, pattern);

                Tally[c]++;

                Track++;
                if (Track % 100) {
                        Track=0;
                        write_tally(Log);
                }

                printf("%d\t", c);
                displayperm(p);
                printf("\n");
        } else {
                for (i=l; i<=r; i++) {
                        p = perm_swap(p, l, i);
                        permute(p, pattern, l+1, r);
                        p = perm_swap(p, l, i); //backtrack
                }
        }
}


void yup(perm_t perm, perm_t pattern) 
{
        int len = perm_length(perm);
        
        permute(perm, pattern, 0, len-1); 
}


void explore_output(perm_t perm, perm_t pattern) 
{
        int i;
        int c;
        perm_t p = perm;
        int len = perm_length(p);

        if (len > 15) {
                return;
        }

        for (i=0; i<len; i++) {
                p = perm_insert_entry(p, i, len);

                explore_output(p, pattern);

                c = countem(p, pattern);

                Tally[c]++;

                Track++;
                if (Track % 100) {
                        Track=0;
                        write_tally(Log);
                }

                printf("%d\t", c);
                displayperm(p);
                printf("\n");

                p = perm_remove_entry(p, i);
        }
}


perm_t maken(int n)
{
        int i = 0;
        perm_t p = 0;

        for (i=0; i<n; i++) {
                p = perm_set_entry(p, i, i);
        }
        return p;
}

int main(int argc, char** argv) 
{
        Log = fopen("tally.log", "w+");

        perm_t c = stringtoperm("312");

        /*perm_t ff = stringtoperm("123456789");*/

        perm_t ff = maken(10);
        /*displayperm(ff);*/
        /*return 1;*/

        yup(ff, c);

        return 1;

        /*perm_t c = stringtoperm("312");*/


        printf("pattern:");
        displayperm(c);
        printf("\n");

        explore_output(c, c);

        return 0;

        perm_t a = stringtoperm("463152");
        perm_t b = stringtoperm("312");

        countem(a, b);

        /*if (argc == 4) {*/
                /*int   n = atoi(argv[1]);*/
                /*int   k = atoi(argv[2]);*/
                /*float x = (float)atof(argv[3]);*/

                /*float o = occ(n, k, x);*/

                /*printf("occurrences: %g\n", o);*/

		/*densities(n, k);*/

        /*}*/

        return 0;
}

