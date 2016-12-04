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
#include <limits.h>

#include "hashtable.h"
#include "perm.h"
#include "pset.h"
#include "ptable.h"
#include "io.h"




void runit(char *filename)
{
	FILE *file;
	char buffer[4096] = {0};

	file =fopen(filename, "r");
	if (!file) {
		return;
	}

	while (fgets(buffer, 4096, file) != NULL) {
		int lineno = 0;
		int value = 0;
		sscanf(buffer, "%d %d\n", &lineno, &value);

                if (value < 0) {
                        uint64_t a = abs(INT_MIN - value);
                        uint64_t b = INT_MAX;
                        printf("%d %"PRIu64"\n", lineno, a+b);
                } else {
                        printf("%d %"PRIu64"\n", lineno, value);
                }
	}

	fclose(file);
	return;

}




/******************************************************************************
 * MATH FUNCTIONS 
 ******************************************************************************/
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

/* WARNING: this will print LSB on left! This is backwards! */
void print_bits_reverse(uint64_t v)
{
        int i = 64;
        while (i-->0) {
                if (v & 1) {
                        printf("1");
                } else {
                        printf("0");
                }
                v >>= 1;
        }
        printf("\n");
}

/* This puts LSB on right as is appropriate */
void print_bits(uint64_t v)
{
        int i = 0;
        while (i++ < 64) {
                if (v & 0x8000000000000000) {
                        printf("1");
                } else {
                        printf("0");
                }
                v <<= 1;
        }
        printf("\n");
}



/******************************************************************************
 * Fasterrr
 ******************************************************************************/

// Input: perm, perm's inverse (which needs to be correct in position length - index), perm's length, index, answer = complement of normalization of (index)-prefix of perm, a bitmap named seenpos which should start off at zero for index = 0. 
// Output: bitmap is updated to keep track of the positions in perm of each letter from n - i to n. answer is updated to be complement of normalization of (index + 1)-prefix of perm
void extendnormalizetop(perm_t perm, perm_t inverse, int length, int index, perm_t *answer, uint64_t *seenpos) 
{
        int i = length - index - 1; 

        int oldpos = perm_get_block(inverse, i); // position of (length - index) in perm

        printf("perm:");
        perm_print(perm);
        printf("\ninverse:");
        perm_print(inverse);
        printf("\n");

        int newpos = 0; // will be position of (length - index) in normalization of (i+1)-prefix of perm
  
        if (oldpos != 0){
                uint64_t temp = *seenpos << (64 - oldpos); // Note: shifting by 32 is ill-defined, which is why we explicitly eliminate digit = 0 case.
                newpos = __builtin_popcount(temp);
        }

        printf("index:%d length:%d i:%d\n", index, length, i);
        printf("oldpos == %d\n", oldpos);
        printf("newpos := %d\n", newpos);
        printf("seenpos:");
        print_bits(*seenpos);
  
        *answer = perm_insert_block(*answer, newpos, index);
        /*perm_set_block(perm_insert_block(*answer, newpos), newpos, index);*/
        *seenpos = *seenpos | (1 << oldpos);
}

// Input: perm, perm's inverse (which needs to be correct in position length - index), perm's length, index, answer = complement of normalization of (index)-prefix of perm, a bitmap named seenpos which should start off at zero for index = 0. 
// Output: bitmap is updated to keep track of the positions in perm of each letter from n - i to n. answer is updated to be complement of normalization of (index + 1)-prefix of perm
/*void extendnormalizetop(perm_t perm, perm_t inverse, int length, int index, perm_t &answer, uint32_t & seenpos) {*/
  /*int i = length - index - 1; */
  /*int oldpos = getdigit(inverse, i); // position of (length - index) in perm*/
  /*int newpos = 0; // will be position of (length - index) in normalization of (i+1)-prefix of perm*/
  /*if (oldpos != 0){*/
    /*uint32_t temp = seenpos << (32 - oldpos); // Note: shifting by 32 is ill-defined, which is why we explicitly eliminate digit = 0 case.*/
    /*newpos = __builtin_popcount(temp);*/
  /*}*/
  /*answer = setdigit(addpos(answer, newpos), newpos, index);*/
  /*seenpos = seenpos | (1 << oldpos);*/
/*}*/




/*void extendnormalizetop(perm_t perm, perm_t inverse, int length, int index, perm_t &answer, uint32_t & seenpos) {*/
  /*int i = length - index - 1; */
  /*int oldpos = getdigit(inverse, i); // position of (length - index) in perm*/
  /*int newpos = 0; // will be position of (length - index) in normalization of (i+1)-prefix of perm*/
  /*if (oldpos != 0){*/
    /*uint32_t temp = seenpos << (32 - oldpos); // Note: shifting by 32 is ill-defined, which is why we explicitly eliminate digit = 0 case.*/
    /*newpos = __builtin_popcount(temp);*/
  /*}*/
  /*answer = setdigit(addpos(answer, newpos), newpos, index);*/
  /*seenpos = seenpos | (1 << oldpos);*/
/*}*/




//
// Input: 
//      perm, 
//      perm's inverse (which needs to be correct in position length - index), 
//      perm's length, 
//      index, 
//      answer = complement of normalization of (index)-prefix of perm, 
//      a bitmap named seenpos which should start off at zero for index = 0. 
// Output: 
//     -bitmap is updated to keep track of the positions in perm of each 
//      letter from n - i to n. 
//
//     -answer is updated to be complement of normalization of 
//      (index + 1)-prefix of perm
void __extendnormalizetop(perm_t perm, perm_t inverse, int length, int index, perm_t *answer, uint64_t *seenpos) 
{
        // If length == 4 and I ask for index 0, I want to remove the value 
        // 3 (the highest, or 0-th value) and normalize.
        int i = length - index - 1; 

        // here is the location of the value 3 in the permutation. 
        int oldpos = perm_get_block(inverse, i); 

        // here is the location of the value 3 in the complement of
        // the normalization of the 3-prefix of the permutation. (???)
        //
        // (i.e. when I normalize the 3-prefix, what digits will
        // I have to kick out/reassign?)
        // 
        // (I suppose this is perhaps recording where the "holes" are
        // left by the downarrow operation?)
        int newpos = 0; 
  
        if (oldpos != 0) {
                // Note: shifting by 64 is ill-defined, which is why we explicitly eliminate digit = 0 case.
                uint64_t temp = *seenpos << (64 - oldpos); 
                newpos = __builtin_popcount(temp);
        }

        *answer = perm_insert_block(*answer, newpos, index);

        // Blot out the positions where we have down-arrowed.
        *seenpos = *seenpos | (1 << oldpos);
}


/*// for patterns in S_{<10} can use like this:*/
/*//  string permlist = "3124 4123 3142 4132";*/
/*//  makepatterns(permlist, patternset, maxpatternsize);*/
/*void make_pattern_db(char *pattern, struct db_t *db) */
/*{*/
        /*uint64_t lettersused = 0; // Used to detect malformed patterns*/

        /*int pos = 0;*/
        /*int i;*/
	/*int length = strlen(pattern);*/
  
        /*perm_t perm = 0;*/
  
        /*for (i=0; i<length; i++) {*/
                /*uint64_t letter = (uint64_t)(pattern[i] - '0' - 1);*/
                /*if (letter > 9) {*/
                        /*printf("Malformed pattern detected in set\n");*/
                /*}*/
                /*lettersused += (1 << letter);*/
                /*perm = setdigit(perm, pos, (int)(pattern[i] - '0' - 1));*/
                /*pos++;*/
        /*}*/

        /*// We use a bit hack to confirm that the inputted permutation*/
        /*// was in fact a valid permutation on the letters 1, ..., pos.*/
        /*if (lettersused + 1 != ((uint64_t)1 << pos)) {*/
                /*printf("Malformed pattern detected in set\n");*/
        /*}*/


	/*
	 * Add the complements of normalizations of
	 * prefixes of the permutation to the db.
	 */
        /*perm_t entry = 0;*/
        /*perm_t inverse = getinverse(perm, length);*/

        /*uint32_t seenpos = 0; // bit map of which letters we've seen so far*/
  
        /*for (int i=0; i<length; i++) {*/
                /*extendnormalizetop(perm, inverse, length, i, &entry, &seenpos);*/

                /*if (!db_contains(db, entry)) {*/
                        /*db_add(db, entry);*/
                /*}*/
        /*}*/

	/*[>*user_perm    = perm;<]*/
	/*[>*user_inverse = inverse;<]*/
/*}*/

perm_t perm_complement(perm_t perm, int length)
{
        perm_t compl = 0;

        for (int i=0; i<length; i++) {
                compl = perm_set_block(compl, i, (length-1)-perm_get_block(perm, i));
        }

        return compl;
}


perm_t perm_reduce(perm_t perm, int length)
{
        int i;
        int j;

        uint64_t min_value    = UINT64_MAX;
        uint64_t min_index    = 0;
        uint64_t cutoff_index = 0;
        uint64_t cutoff_value = 0;
        uint64_t val          = 0;

        perm_t new = 0;

        for (i=0; i<length; i++) {
                for (j=0; j<length; j++) {
                        val = perm_get_block(perm, j); 

                        if ((i == 0 || val > cutoff_value) && val < min_value) {
                                /*printf("min candidate:%d\n", val);*/
                                min_value = val;
                                min_index = j;
                        }
                }
                /*printf("min value:%d\n", min_value);*/
                /*printf("min index:%d\n", min_index);*/
                new = perm_set_block(new, min_index, (uint64_t)i);
                /*perm_print(new);*/
                /*printf("\n");*/
                cutoff_value = min_value;
                min_value    = UINT64_MAX; /* big value so first loop hits */
        }

        return new;
}

perm_t perm_prefix(perm_t perm, int prefix_length)
{
        perm_t prefix = 0;
        perm_t norm = 0;
        uint64_t block = 0;

        uint64_t max_v = 0;
        uint64_t max_j = 0;
        uint64_t ceiling = 10000;

        for (int i=0; i<prefix_length; i++) {
                prefix = perm_set_block(prefix, i, perm_get_block(perm, i));
        }

        /*for (int i=prefix_length-1; i>=0; i--) {*/
                /*max_v = -1;*/
                /*max_j = -1;*/
                /*for (int j=0; j<prefix_length; j++) {*/
                        /*block = perm_get_block(prefix, j);*/
                        /*[>if (block > prefix_length) {<]*/
                                /*if (block <= ceiling) {*/
                                        /*[>printf("here\n");<]*/
                                        /*max_v = block;*/
                                        /*max_j = j;*/
                                /*}*/
                        /*[>}<]*/
                /*}*/
                /*perm_print(norm); */
                /*printf("\n");*/
                /*printf("len:%d max_v:%d\n", prefix_length, max_v);*/
                /*if (max_v != -1 && max_j != -1) {*/
                        /*ceiling = i;*/
                        /*norm = perm_set_block(norm, max_j, i);*/
                /*}*/
        /*}*/

        /*for (int i=0; i<prefix_length; i++) {*/
                /*prefix = perm_set_block(prefix, i, perm_get_block(perm, i));*/
        /*}*/

        /* Easier to just mask it out but... for now this is fine */
        /* ALSO NOTE: this is the literal digits, it is NOT normalized 
         * and hence NOT a real permutation */
        return prefix;
}

perm_t perm_suffix(perm_t perm, int suffix_length)
{
        perm_t suffix = 0;

        int perm_len = perm_length(perm);

        for (int i=0; i<suffix_length; i++) {
                suffix = perm_set_block(suffix, i, perm_get_block(perm, (perm_len-suffix_length)+i));
        }

        return suffix;
}

perm_t perm_reverse(perm_t perm, int perm_length)
{
        perm_t rev = 0;

        for (int i=0; i<perm_length; i++) {
                rev = perm_set_block(rev, i, perm_get_block(perm, perm_length-i));
        }

        return rev;
}


/*
 * This is true:
 *
 * compl( norml( p\upharpoonright{n}^{-1} ) ) = norml( p \upharpoonright{n} )
 *
 */
perm_t new_compl(perm_t perm, int length)
{
        int i = length - 1;
        int l = 0;
        uint64_t seenpos;
        perm_t compl = 0;

        perm_t inverse = perm_inverse(perm, length);

        while (i-->=0) {
                int oldpos = perm_get_block(inverse, i);
                int newpos = 0;
                if (oldpos != 0) {
                        uint64_t temp = seenpos << (64 - oldpos); //Note: shifting by 32 is ill-defined, which is why we explicitly eliminate digit = 0 case.
                        newpos = __builtin_popcountll(temp);
                }

                compl = perm_insert_block(compl, newpos, l);
                seenpos |= (1 << oldpos);
                l += 1;

                printf("compl:%s\n", perm_get_string(compl));
        }

        printf("compl:%s\n", perm_get_string(compl));

        return compl;
}



/* 
 * index: 0   
 * i    : |p| - 1   (really |p|-(0+1))
 * ans  : compl. of norm. of |p|-prefix of perm 
 *
 * index: 1   
 * i    : |p| - 2   (really |p|-(1+1))
 * ans  : compl. of norm. of (|p|-1)-prefix of perm 
 *
 * index: 2   
 * i    : |p| - 3   (really |p|-(2+1))
 * ans  : compl. of norm. of (|p|-2)-prefix of perm 
 *
 * ...and so on until...
 *
 * index: |p|+1 
 * i    : 0
 * ans  : compl. of norm. of 1-prefix of perm
 *
 * NOTE: undefined for index >= length
 */
/*void my_ent_inloop(perm_t perm, perm_t inverse, int length, int index, perm_t *answer, uint64_t *seenpos) */
/*{*/
        /*assert(index < length);*/
        /*assert(index >=0);*/

        /*int i = length - index - 1; */
        /*int oldpos = perm_get_block(inverse, i); // position of (length - index) in perm*/
        /*int newpos = 0;                          // will be position of (length - index) in normalization of (i+1)-prefix of perm*/
        
        /*if (oldpos != 0) {*/
                /*uint64_t temp = *(seenpos) << (64 - oldpos); // Note: shifting by 32 is ill-defined, which is why we explicitly eliminate digit = 0 case.*/
                /*newpos = __builtin_popcountll(temp);*/
        /*}*/
        
        /* This is where the complementing takes place: the value of @index
         * is equal to (@length - @i - 1).
         *
         * In order to ensure the complementing takes place with respect
         * to the length of the prefix we're considering, we must ensure
         *
         * that my_ent_inloop() is called from a small to a large index. 
         */
        
        /**answer = perm_insert_block(*answer, newpos, index);*/

        /**seenpos = *seenpos | (1 << oldpos);*/
/*}*/


/*
 * Computes the complement of the normalization of the (index)-prefix of @perm
 *
 * EX
 *      perm    index   prefix  norm    compl of norm
 *
 *      1302    4       1302    1302    2031
 *      1302    3       130     120     102
 *      1302    2       13      01      10
 *      1302    1       1       0       0
 */
perm_t my_ent(perm_t perm, perm_t inverse, int length, int index) 
{
        int i;
        int j;
        int oldpos;
        int newpos = 0;

        perm_t   answer  = 0;
        uint64_t seenpos = 0;

        assert(index <= (length-1)); /* Max index value is length-1 */
        assert(index >=0);           /* Min index value is 0        */

        for (i=0; i<=(length-index-1); i++) {
                /*my_ent_inloop(perm, inverse, length, i, &answer, &seenpos);*/

                /* 
                 * Here we compute which value of the inverse to
                 * grab, and once we grab it, we do the work of
                 * normalizing.
                 *
                 * The normalization is done by building the result
                 * up in stages, beginning with the largest digit
                 * and moving to the smallest digit.
                 *
                 * The size and location of these digits is tracked
                 * using the '@seenpos' bitmap.
                 *
                 */
                j = length - i - 1; 

                oldpos = perm_get_block(inverse, j); // position of (length - index) in perm
                newpos = 0;                          // will be position of (length - index) in normalization of (i+1)-prefix of perm
                
                if (oldpos != 0) {
                        uint64_t temp = seenpos << (64 - oldpos); // Note: shifting by 32 is ill-defined, which is why we explicitly eliminate digit = 0 case.
                        newpos = __builtin_popcountll(temp);
                }
                
                /* 
                 * Now the complementing takes place: the value 
                 * @i is equal to (@length - @j - 1).
                 *
                 * The action of the for() loop is very specific here: we must
                 * proceed from small to large index in order to ensure the 
                 * complementing takes place properly, with respect to the length
                 * of the prefix we're considering.
                 *
                 * For example if we take any 1-prefix of any permutation, the normalized
                 * form of the 1-prefix will be the permutation '0', since there is only 
                 * one digit. 
                 *
                 * However the complement of this permutation '0' depends on what we 
                 * believe its length to be. If we take the complement with respect
                 * to a permutation with max. digit 3, then the complement of 0 will 
                 * be 3. 
                 *
                 * We WANT the complement of the permutation '0' to be '0', the
                 * complement of '01' to be '10', and so on. The length of the
                 * prefix is the context of the complement, as it should be.
                 */
                answer = perm_insert_block(answer, newpos, i);

                /* 
                 * Mark the location just seen with a 1, so we can count it next time
                 * around in the loop.
                 */
                seenpos = seenpos | (1 << oldpos);
        }

        return answer;
}




/*
 * Used on the pattern.
 */
void build_complement_prefixes(struct ptable_t *table, perm_t perm)
{
        uint64_t lettersused = 0; // Used to detect malformed patterns

        int pos = 0;
        int i;
	int length = perm_length(perm);
  
        perm_t prefix  = 0;
        perm_t inverse = perm_inverse(perm, length);
        perm_t compl = perm_complement(perm, length);
        perm_t compl2 = new_compl(perm, length);

        perm_print(inverse);
        printf("\n");
        perm_print(compl);
        printf("\n");
        perm_print(compl2);
        printf("\n");

        /*printf("complement is: %s\n", perm_get_string(compl));*/

        uint64_t helperbitmap = 0; // bit map of which letters we've seen so far

        for (i=1; i<=length; i++) {
                /*prefix = perm_reduce(perm_suffix(compl, length-i), length-i);*/
                /*prefix = perm_complement(perm_reduce(perm_prefix(inverse, i+1), i+1), i+1);*/
                prefix = my_ent(perm, inverse, length, length-i);
                /*__extendnormalizetop(perm, inverse, length, i, &prefix, &helperbitmap);*/
                printf("extension is: %s\n",perm_get_string(prefix));
                /*print_bits(helperbitmap);*/

                if (!ptable_contains(table, prefix)) {
                        printf("inserting %s\n", perm_get_string(prefix));
                        ptable_insert(table, prefix);
                }
        }
}



void etc(void)
{
        /*perm_t pete= perm_from_csv("1,3,0,2");*/
        /*perm_t plen = perm_length(pete);*/
        /*perm_t pinv = perm_inverse(pete, plen);*/

        /*[>perm_t ans       = 0;<]*/
        /*[>uint64_t seenpos = 0;<]*/

        /*my_ent(pete, pinv, plen, plen-2);*/

        /*[>perm_print(pete);<]*/
        /*[>printf("\n");<]*/
        /*[>perm_print(pinv);<]*/
        /*[>printf("\n");<]*/
        /*[>perm_print(ans);<]*/
        /*[>printf("\n");<]*/
        /*[>print_bits(seenpos);<]*/
        /*[>printf("\n");<]*/

        /*return;*/





        /*perm_t perm = perm_from_csv("5,3,8,0");*/

        /*perm_t reduce = perm_reduce(perm, 4);*/

        /*perm_print(perm); */
        /*printf("\n");*/
        /*perm_print(reduce); */
        /*printf("\n");*/

        /*return;*/
        struct ptable_t tab;

        ptable_init(&tab, 600000);

        /*perm_t pattern = perm_from_csv("3,2,1,0");*/
        perm_t pattern = perm_from_csv("0,1");
        /*perm_t pattern = perm_from_csv("1,0,3,2");*/
        /*perm_t pattern = perm_from_csv("1,0,2,3");*/

        build_complement_prefixes(&tab, pattern);

        /*perm_t perm = perm_from_csv("1,0,2,3,5,4,6");*/
        /*perm_t perm = perm_from_csv("3,2,1,0");*/
        /*perm_t perm = perm_from_csv("1,0,3,2");*/
        perm_t perm = perm_from_csv("0,1,2,3");
        /*perm_t perm = perm_from_csv("0,1,2,3");*/

        int perm_len = perm_length(perm);
        int pattern_len = perm_length(pattern);

        perm_t inverse = perm_inverse(perm, perm_len);


        uint64_t count = 0;

        checkpatterns(
                perm,
                inverse,
                (perm_t)0,
                0,
                perm_len,         
                pattern_len,      
                (uint64_t)0,
                &tab,
                &count
        );

        printf("%"PRIu64"\n", count);
}


void prefix_test(void)
{
        struct ptable_t tab;

        ptable_init(&tab, 600000);

        perm_t pattern = perm_from_csv("3,0,1,2");

        build_complement_prefixes(&tab, pattern);

        if (ptable_contains(&tab, perm_from_csv("0,3,1,2"))) {
                printf("got 0,3,1,2\n");
        }
        if (ptable_contains(&tab, perm_from_csv("3,2,1"))) {
                printf("got 3,2,1\n");
        }
        if (ptable_contains(&tab, perm_from_csv("3,2"))) {
                printf("got 3,2\n");
        }
        if (ptable_contains(&tab, perm_from_csv("3"))) {
                printf("got 3\n");
        }

        printf("table size is: %d\n", tab.size);
}




/*checkpatterns(perm, inverse, 0, 0, currentsize, maxpatternsize, 0, prefixmap, count);*/
int checkpatterns(perm_t perm, perm_t inverse, perm_t currentpatterncomplement, int currentpatternlength, int largestletterused, int numlettersleft, uint64_t seenpos, struct ptable_t *prefixmap, uint64_t *count) 
{
        /*printf("perm:");*/
        /*perm_print(perm); */
        /*printf("\n");*/
        /*printf("compl:");*/
        /*perm_print(currentpatterncomplement); */
        /*printf("\n");*/

        /*printf("largest-used:%d, num-left:%d current-length:%d\n", largestletterused, numlettersleft, currentpatternlength);*/
        int val;

        printf("currentpatterncomplement:%s\n", perm_get_string(currentpatterncomplement));

        if (currentpatterncomplement != 0 && !ptable_contains(prefixmap, currentpatterncomplement)) {
                /*[> Early exit <]*/
                /*printf("compl:%s\n", perm_get_string(currentpatterncomplement));*/
                /*printf("done 1\n");*/
                /*printf("exit ... period\n");*/
                return;
        }

        if (numlettersleft == 0) { // Assumes all patterns are same size --> this is only cae where prefix is a pattern
                (*count)++;
                /*printf("count := %d\n", *count);*/
                /*if (*count > 1) {*/
                        /*return -1;*/
                /*}*/
                /*printf("count:=%d\n", *count);*/
                return; // also assumes all patterns same size
        }


        /*printf("largest-used:%d, num-left:%d current-length:%d\n", largestletterused, numlettersleft, currentpatternlength);*/
  
        /*
         * The inverse of a permutation is like a way of looking up
         * the location of each value in the permutation.
         *
         * If we let i be an index into the inverse, then inverse[i] 
         * is simply the position of the value i in the permutation. 
         *
         * So we loop through the permutation from the largest digit
         * to the smallest.
         *
         *      If p = 1302, order visited is p[1], p[3], p[0], p[2].
         */
        for (int i=(largestletterused-1); i>=(numlettersleft-1); i--) { 
                /*[>// for i >= numlettersleft requirement, assumes all patterns are same size<]*/

                // Similarly to as in extendnormalizetop (defined in perm.cpp), 
                // we will build the complement of the normalization of the new 
                // permutation-subsequence (with the new letter added to it)
                /*printf("value i:%d\n", i);*/
                /*
                 * 'seenpos' is a bitmap showing which positions have
                 * already been sampled during the loop.
                 *
                 *      If p = 1302, 
                 *
                 *      i = 0,  seenpos = ...(60 zeroes)...0000
                 *      i = 1,  seenpos = ...(60 zeroes)...0100
                 *      i = 2,  seenpos = ...(60 zeroes)...0101
                 *      i = 3,  seenpos = ...(60 zeroes)...1101
                 *      i = 4,  seenpos = ...(60 zeroes)...1111
                 */
                int oldpos = perm_get_block(inverse, i);
                int newpos = 0;
                if (oldpos != 0) {
                        /* 
                         * seenpos << (64 - oldpos) 
                         *
                         * Removes all 1's except for those that occur
                         * in a prefix of length 'oldpos'.
                         *
                         * In other words, those that occur prior to 
                         * (and not including) 'oldpos' in the permutation.
                         *
                         * Counting these amounts to counting the number
                         * of digits that occur positionally before the
                         * digit 'i', that have been visited already in
                         * the loop.
                         *
                         * This in turn means counting all the digits that
                         * have value strictly greater than 'i' which also
                         * have position strictly less than 'inverse[i]'. 
                         *
                         *      j < i && p[j] > p[i], in other words.
                         */
                        uint64_t temp = seenpos << (64 - oldpos); //Note: shifting by 32 is ill-defined, which is why we explicitly eliminate digit = 0 case.
                        /*printf("oldp:%d\n", oldpos);*/
                        /*print_bits(oldpos);*/
                        /*printf("seen:");*/
                        /*print_bits(seenpos);*/
                        /*printf("temp:");*/
                        /*print_bits(temp);*/
                        newpos = __builtin_popcountll(temp);
                        /*printf("newpos:%d\n", newpos);*/
                }

                /*perm_t newpattern = setdigit(addpos(currentpatterncomplement, newpos), newpos, currentpatternlength);*/

                /* ALERT: currentpatternlength is not making this the complement ... it's the un-complement. */
                /*perm_t newpattern = perm_insert_block(currentpatterncomplement, newpos, currentpatternlength);*/
                perm_t newpattern = perm_insert_block(currentpatterncomplement, newpos, currentpatternlength);


                /*perm_print(newpattern);*/
                /*printf("\n");*/

                /*printf("recurse\n");*/

                // Recurse to make sequence longer until we eventually either do or don't get a pattern:
                val = checkpatterns(perm, inverse, newpattern, currentpatternlength + 1, i, numlettersleft - 1, seenpos | (1 << oldpos), prefixmap, count);
                /*if (val == -1) {*/
                        /*return -1;*/
                /*}*/
        }
        
        /*printf("exhausted\n");*/
        return 0;
}




/******************************************************************************
 * COUNTING PATTERNS IN THEM 
 ******************************************************************************/

inline perm_t downarrow(perm_t perm, int i)
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

        perm = perm_remove_block(perm, pos);

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
inline perm_t downarrowfast(perm_t perm, int i, int length)
{
        int pos;
        int j;

        /* 
         * In paper written as length-i+1, but we index from 0 in
         * this implementation, so it requires another -1.  
         */
        int seek_digit = (length-i+1)-1;

        for (j=0; j<length; j++) {
                /* TODO: This could be done as a lookup with the inverse */
                if (perm_get_block(perm, j) == seek_digit) {
                        pos = j;
                        break;
                }
        }

        perm = perm_remove_block(perm, pos);

        for (j=0; j<length-1; j++) {
                /* TODO: This part fuckin sucks */
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

inline int countfast(perm_t perm, perm_t pattern, int perm_len, int pattern_len) 
{
        return pfast(0, perm, pattern, perm_len, pattern_len);
}

int countffast(perm_t perm, perm_t pattern, int perm_len, int pattern_len) 
{
        struct hashtable ht;
        hash_init_alloc(&ht, 65536);

        return pffast(0, perm, pattern, perm_len, pattern_len, &ht);
}


uint64_t countfast_enhanced(perm_t perm, perm_t inverse, perm_t pattern, struct ptable_t *pattern_prefixes, int perm_len, int pattern_len)
{
        uint64_t count = 0;

        checkpatterns(
                perm,
                inverse,
                (perm_t)0,
                0,
                perm_len,         
                pattern_len,      
                (uint64_t)0,
                pattern_prefixes,
                &count
        );

        /*printf("count:%d\n", count);*/

        return count;
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

struct pargs_t {
        perm_t start;
        perm_t stop;       /* Only used when resuming */
        int    index[16];  /* Only used when resuming */
        int    i0;         /* Only used when resuming */
        int    thread_id;
        uint64_t blocksize; /*  How many perms this thread must do */
        uint64_t *tally;
        struct pset_t *class;
        struct ptable_t *pattern_prefixes;

        /* THESE COULD BE MADE GLOBAL, EVEN IN THREADED CASE (ATOMIC) */
        perm_t pattern;
        uint64_t n;
        uint64_t k;
        uint64_t n_choose_k;
        uint64_t n_factorial;
        uint64_t n_threads;
};


/*
 * Used for the logging thread to track progress
 * of individual threads. 
 */
struct monitor_data_t { 
        uint64_t count;
        uint64_t total;
};

struct monitor_data_t MONITOR[64]; 


struct monitor_args_t {
        FILE *log;
        int   n_threads;
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

        /* Set the total so the monitor thread can print percentages */
        MONITOR[P->thread_id].total = P->blocksize;

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
        /*if (c == 1) {*/
                /*perm_print(p); */
                /*printf("\n");*/
        /*}*/
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
                        /*if (c == 0) {*/
                                /*perm_print(p); */
                                /*printf("\n");*/
                        /*}*/
                        tptr[c]++;
                        count++;
                        pset_add(&cptr[c], p);

                        if (track++ == 10000) {
                                /* 
                                 * Every 10,000 permutations, let the 
                                 * monitor thread know the updated
                                 * count. 
                                 */
                                MONITOR[P->thread_id].count = count;
                                track=0;
                        }
                } else {
                        iptr[i++] = 1;
                }
        }
}

void run_p_enhanced(struct pargs_t *P)
{
        uint64_t n;
        uint64_t k;
        uint64_t c;
        uint64_t i;
        uint64_t i0;
        uint64_t *tptr;
        struct pset_t *cptr;
        int *iptr;
        perm_t p;
        perm_t stop;
        perm_t pattern;
        uint64_t track = 0;
        uint64_t count = 0;
        perm_t inverse;
        struct ptable_t *pattern_prefixes;

        /* Set the total so the monitor thread can print percentages */
        MONITOR[P->thread_id].total = P->blocksize;

        p       = P->start;
        stop    = P->stop;
        pattern = P->pattern;

        n = P->n;
        k = P->k;

        tptr = P->tally;
        iptr = P->index;
        cptr = P->class;

        pattern_prefixes = P->pattern_prefixes;

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
        c = countfast_enhanced(p, perm_inverse(p, n), pattern, pattern_prefixes, n, k);
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

                        c = countfast_enhanced(p, perm_inverse(p, n), pattern, pattern_prefixes, n, k);
                        /*if (c == 0) {*/
                                /*perm_print(p); */
                                /*printf("\n");*/
                        /*}*/
                        tptr[c]++;
                        count++;
                        pset_add(&cptr[c], p);

                        if (track++ == 10000) {
                                /* 
                                 * Every 10,000 permutations, let the 
                                 * monitor thread know the updated
                                 * count. 
                                 */
                                MONITOR[P->thread_id].count = count;
                                track=0;
                        }
                } else {
                        iptr[i++] = 1;
                }
        }
}

void *run_tally_with_classes(void *A) 
{
        struct pargs_t *P = (struct pargs_t *)A;

        fprintf(stderr, "Thread %d is active! My blocksize is %"PRIu64"\n", P->thread_id, P->blocksize);

        /*char *pstr = perm_get_string(P->pattern);*/

        /* Open the project logfile and tallyfile */
        /*P->logfile   = fopenf("w+", "%s/%d-%s.t%d.log", P->path, P->n, pstr, P->thread_id);*/
        /*P->tallyfile = fopenf("w+", "%s/%d-%s.t%d.tally", P->path, P->n, pstr, P->thread_id);*/

        /*free(pstr);*/

        run_p(P);

        /*
         * Print class files and tally file
         */
        /*for (i=0; i<P->nchoosek; i++) {*/
                /*if (P->class[i].size != 0) {*/
                        /*FILE *f = fopenf("w+", "%s/class.%d.t%d.trace", P->path, i, P->thread_id);*/
                        /*pset_write(&(P->class[i]), f);*/
                /*}*/
        /*}*/

        return NULL;
}


void *run_tally_with_classes_enhanced(void *A) 
{
        struct pargs_t *P = (struct pargs_t *)A;

        fprintf(stderr, "Thread %d is active! My blocksize is %"PRIu64"\n", P->thread_id, P->blocksize);

        run_p_enhanced(P);

        return NULL;
}


int COMPUTE_THREADS_DONE = 0;

void *monitor_threads(void *args)
{
        struct monitor_args_t *mon = (struct monitor_args_t *)args;

        int i;
        float pct = 0;

        fprintf(stderr, "Monitor thread is active\n");

        /* 
         * Store variables to prevent having to use
         * a mutex... since integer r/w is atomic,
         * and total won't change, only count.
         */
        uint64_t count; 
        uint64_t total;

        uint64_t total_all_threads = 0;
        uint64_t count_all_threads = 0;
        double pct_all_threads;

        do { 
                /* Rewind the file */
                fseek(mon->log, 0L, SEEK_SET);

                for (i=0; i<mon->n_threads; i++) {
                        count = MONITOR[i].count;
                        total = MONITOR[i].total;

                        count_all_threads += count;
                        total_all_threads += total;

                        pct = ((float)count/(float)total)*100;

                        fprintf(mon->log, "thread %02d: (%f%%) %"PRIu64"/%"PRIu64"\n", i, pct, count, total); 
                }

                pct_all_threads = ((float)count_all_threads/(float)total_all_threads)*100;

                fprintf(mon->log, "==========\nALL TOTAL: (%f%%) %"PRIu64"/%"PRIu64"\n", pct_all_threads, count_all_threads, total_all_threads); 

                count_all_threads = 0;
                total_all_threads = 0;

                /* Sleep for 1 second (this is Linux only!) */
                sleep(1);
        } while (COMPUTE_THREADS_DONE == 0);
}


inline void print_to_blocklist(FILE *blocklist, int block, int *index, perm_t perm, int i)
{
        int j;

        fprintf(blocklist, "%d: ", block);
        for (j=0; j<16; j++) {
                if (j != 15) {
                        fprintf(blocklist, "%"PRIu64",", perm_get_block(perm, j));
                } else {
                        fprintf(blocklist, "%"PRIu64, perm_get_block(perm, j));
                }
        }
        fprintf(blocklist, " ");
        for (j=0; j<16; j++) {
                if (j != 15) {
                        fprintf(blocklist, "%d,", index[j]);
                } else {
                        fprintf(blocklist, "%d", index[j]);
                }
        }
        fprintf(blocklist, " %d\n", i);
}


void tally_with_classes(perm_t perm, perm_t pattern, int n_threads)
{
        if (n_threads > 64) {
                printf("too many threads!\n");
                exit(1);
        }

        printf("preparing to thread...\n");

        uint64_t n = perm_length(perm);
        uint64_t k = perm_length(pattern);
        uint64_t n_factorial = factorial(n);
        uint64_t n_choose_k  = nchoosek(n, k);
        /*
         * There are (n_choose_k + 1) possible classes, corresponding to
         * the potential counts for occurrences of a length-k pattern in a
         * length-n permutation. These range from 0 occurrences all the way
         * up to n_choose_k occurrences.
         */
        uint64_t number_of_classes = (n_choose_k + 1);

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
        sprintf(path, "%d-%s-%s", (int)n, pstr, datestring);

        free(pstr);

        mkdir(path, DIR_PERMS); 

        struct pargs_t *args = calloc(1, (n_threads+2)*sizeof(struct pargs_t));

        int i;
        int j;

        /* Set up entries common to all threads */
        for (i=0; i<n_threads; i++) {
                args[i].pattern   = pattern;
                args[i].thread_id = i;
                args[i].n = n;
                args[i].k = k;
                args[i].n_choose_k = n_choose_k;
                args[i].n_factorial = n_factorial;
                args[i].tally = calloc(1, number_of_classes*sizeof(uint64_t));
                args[i].class = calloc(1, number_of_classes*sizeof(struct pset_t));

                /* 
                 * Initialize each of the pset objects. These are more like
                 * permutons...  
                 */
                for (j=0; j<number_of_classes; j++) {
                        pset_init(&(args[i].class[j]), n);
                }
        }


        pthread_t threads[64];
        pthread_t monitor;
        int rc;

        /*************************************************
         * Create a monitor thread arguments
         *************************************************/
        struct monitor_args_t monitor_args; 

        monitor_args.n_threads = n_threads;
        monitor_args.log = fopenf("w+", "%s/monitor.log", path);

        for (i=0; i<n_threads; i++) {
                MONITOR[i].count = 0;
                MONITOR[i].total = args[i].blocksize;
        }

        if (monitor_args.log) {
                fprintf(stderr, "Monitor log is open\n");
        } else {
                fprintf(stderr, "Something wrong with monitor log\n");
        }

        /* Create the monitor thread */
        pthread_create(&monitor, NULL, monitor_threads, (void *)&monitor_args);

        
        FILE *blocklist = fopenf("w+", "%s/blocklist.log", path);



        /*************************************************
         * Run through the permutation list once to init 
         *************************************************/
        int index[100];
        uint64_t blocksize = (uint64_t)n_factorial / (uint64_t)n_threads;
        uint64_t count = 0;
        int block = 0;

        fprintf(stderr, "Making %d blocks of size %"PRIu64"/%"PRIu64"\n", n_threads, blocksize, n_factorial);

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

        print_to_blocklist(blocklist, 0, index, args[0].start, i);

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

                                print_to_blocklist(blocklist, block, index, perm, i);

                                fprintf(stderr, "Block %d/%d complete after %"PRIu64"\n", block, n_threads, count-1);

                                pthread_create(&threads[block-1], NULL, run_tally_with_classes, (void *)&args[block-1]);

                                if (block > n_threads) {
                                        fprintf(stderr, "too many blocks!\n");
                                        exit(1);
                                }

                                count = 0;
                        }
                } else {
                        index[i++] = 1;
                }
        }

        fprintf(stderr, "Block %d/%d complete after %"PRIu64"\n", block, n_threads, count-1);
        args[block].stop = UINT64_MAX;
        pthread_create(&threads[block], NULL, run_tally_with_classes, (void *)&args[block]);

        fprintf(stderr, "All blocks complete\n");



        /*fprintf(monitor_args.log, "Monitor thread log test\n");*/


        /*************************************************
         * Run each of the args in its own thread 
         *************************************************/
       
        /*for (i=0; i<n_threads; i++) { */
                /*rc = pthread_create(&threads[i], NULL, run_tally_with_classes, (void *)&args[i]);*/
        /*}*/


        /* Wait for compute threads to finish */
        for (i=0; i<n_threads; i++) {
                rc = pthread_join(threads[i], NULL);
                fprintf(stderr, "Compute thread [%d] complete\n", i);
        }

        /* Signal monitor that threads are done */
        COMPUTE_THREADS_DONE = 1;

        /* Wait for monitor to finish */
        rc = pthread_join(monitor, NULL);

        fprintf(stderr, "Monitor thread complete\n");

        /* Combine all the tallies and class counts into a single one */
        uint64_t *tally = calloc(1, n_choose_k * sizeof(uint64_t));
        struct pset_t *class = calloc(1, n_choose_k *sizeof(struct pset_t));;

        for (i=0; i<n_choose_k; i++) {
                pset_init(&(class[i]), n);
        }

        fprintf(stderr, "Computing occurrence class sizes...\n");
        fprintf(stderr, "Computing permutons...\n");

        for (i=0; i<n_threads; i++) {
                for (j=0; j<n_choose_k; j++) {
                        tally[j] += args[i].tally[j];
                        pset_sum(&class[j], &(args[i].class[j]));
                }
        }

        /***************** 
         * write it all
         *****************/
        fprintf(stderr, "Writing tally...\n");

        /* Write the master tally file */
        FILE *tallyfile = fopenf("w+", "%s/compute.tally", path, n, pstr);
        for (i=0; i<n_choose_k; i++) {
                fprintf(tallyfile, "%d %d\n", i, tally[i]); 
        }

        fprintf(stderr, "Writing permutons...\n");

        /* Write the permuton files for each of the classes */
        for (i=0; i<n_choose_k; i++) {
                if (class[i].size != 0) {
                        FILE *f = fopenf("w+", "%s/class.%d.permuton", path, i);
                        pset_write(&(class[i]), f);
                }
        }

        fprintf(stderr, "All results in ./%s\n", path);
        fprintf(stderr, "RUN COMPLETE\n");

}

void tally_with_classes_enhanced(perm_t perm, perm_t pattern, int n_threads)
{
        if (n_threads > 64) {
                printf("too many threads!\n");
                exit(1);
        }

        printf("preparing to thread...\n");

        uint64_t n = perm_length(perm);
        uint64_t k = perm_length(pattern);
        uint64_t n_factorial = factorial(n);
        uint64_t n_choose_k  = nchoosek(n, k);
        /*
         * There are (n_choose_k + 1) possible classes, corresponding to
         * the potential counts for occurrences of a length-k pattern in a
         * length-n permutation. These range from 0 occurrences all the way
         * up to n_choose_k occurrences.
         */
        uint64_t number_of_classes = (n_choose_k + 1);

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
        sprintf(path, "%d-%s-%s", (int)n, pstr, datestring);

        free(pstr);

        mkdir(path, DIR_PERMS); 

        struct pargs_t *args = calloc(1, (n_threads+2)*sizeof(struct pargs_t));

        uint64_t i;
        uint64_t j;

        /* Set up entries common to all threads */
        for (i=0; i<n_threads; i++) {
                args[i].pattern   = pattern;
                args[i].thread_id = i;
                args[i].n = n;
                args[i].k = k;
                args[i].n_choose_k = n_choose_k;
                args[i].n_factorial = n_factorial;
                args[i].tally = calloc(1, number_of_classes*sizeof(uint64_t));
                args[i].class = calloc(1, number_of_classes*sizeof(struct pset_t));

                /* Build a pattern prefix hash table */
                args[i].pattern_prefixes = calloc(1, sizeof(struct ptable_t));
                ptable_init(args[i].pattern_prefixes, 600000);
                build_complement_prefixes(args[i].pattern_prefixes, pattern);


                /* 
                 * Initialize each of the pset objects. These are more like
                 * permutons...  
                 */
                for (j=0; j<number_of_classes; j++) {
                        pset_init(&(args[i].class[j]), n);
                }
        }


        pthread_t threads[64];
        pthread_t monitor;
        uint64_t rc;

        /*************************************************
         * Create a monitor thread arguments
         *************************************************/
        struct monitor_args_t monitor_args; 

        monitor_args.n_threads = n_threads;
        monitor_args.log = fopenf("w+", "%s/monitor.log", path);

        for (i=0; i<n_threads; i++) {
                MONITOR[i].count = 0;
                MONITOR[i].total = args[i].blocksize;
        }

        if (monitor_args.log) {
                fprintf(stderr, "Monitor log is open\n");
        } else {
                fprintf(stderr, "Something wrong with monitor log\n");
        }

        /* Create the monitor thread */
        pthread_create(&monitor, NULL, monitor_threads, (void *)&monitor_args);

        
        FILE *blocklist = fopenf("w+", "%s/blocklist.log", path);



        /*************************************************
         * Run through the permutation list once to init 
         *************************************************/
        uint64_t index[100];
        uint64_t blocksize = (uint64_t)n_factorial / (uint64_t)n_threads;
        uint64_t count = 0;
        uint64_t block = 0;

        fprintf(stderr, "Making %d blocks of size %"PRIu64"/%"PRIu64"\n", n_threads, blocksize, n_factorial);

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

        print_to_blocklist(blocklist, 0, index, args[0].start, i);

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

                                print_to_blocklist(blocklist, block, index, perm, i);

                                fprintf(stderr, "Block %d/%d complete after %"PRIu64"\n", block, n_threads, count-1);

                                pthread_create(&threads[block-1], NULL, run_tally_with_classes_enhanced, (void *)&args[block-1]);

                                if (block > n_threads) {
                                        fprintf(stderr, "too many blocks!\n");
                                        /*exit(1);*/
                                }

                                count = 0;
                        }
                } else {
                        index[i++] = 1;
                }
        }

        fprintf(stderr, "Block %d/%d complete after %"PRIu64"\n", block, n_threads, count-1);
        args[block].stop = UINT64_MAX;
        pthread_create(&threads[block], NULL, run_tally_with_classes_enhanced, (void *)&args[block]);

        fprintf(stderr, "All blocks complete\n");



        /*fprintf(monitor_args.log, "Monitor thread log test\n");*/


        /*************************************************
         * Run each of the args in its own thread 
         *************************************************/
       
        /*for (i=0; i<n_threads; i++) { */
                /*rc = pthread_create(&threads[i], NULL, run_tally_with_classes, (void *)&args[i]);*/
        /*}*/


        /* Wait for compute threads to finish */
        for (i=0; i<n_threads; i++) {
                rc = pthread_join(threads[i], NULL);
                fprintf(stderr, "Compute thread [%d] complete\n", i);
        }

        /* Signal monitor that threads are done */
        COMPUTE_THREADS_DONE = 1;

        /* Wait for monitor to finish */
        rc = pthread_join(monitor, NULL);

        fprintf(stderr, "Monitor thread complete\n");

        /* Combine all the tallies and class counts into a single one */
        uint64_t *tally = calloc(1, n_choose_k * sizeof(uint64_t));
        struct pset_t *class = calloc(1, n_choose_k *sizeof(struct pset_t));;

        for (i=0; i<n_choose_k; i++) {
                pset_init(&(class[i]), n);
        }

        fprintf(stderr, "Computing occurrence class sizes...\n");
        fprintf(stderr, "Computing permutons...\n");

        for (i=0; i<n_threads; i++) {
                for (j=0; j<n_choose_k; j++) {
                        tally[j] += args[i].tally[j];
                        pset_sum(&class[j], &(args[i].class[j]));
                }
        }

        /***************** 
         * write it all
         *****************/
        fprintf(stderr, "Writing tally...\n");

        /* Write the master tally file */
        FILE *tallyfile = fopenf("w+", "%s/compute.tally", path, n, pstr);
        for (i=0; i<n_choose_k; i++) {
                fprintf(tallyfile, "%d %d\n", i, tally[i]); 
        }

        fprintf(stderr, "Writing permutons...\n");

        /* Write the permuton files for each of the classes */
        for (i=0; i<n_choose_k; i++) {
                if (class[i].size != 0) {
                        FILE *f = fopenf("w+", "%s/class.%d.permuton", path, i);
                        pset_write(&(class[i]), f);
                }
        }

        fprintf(stderr, "All results in ./%s\n", path);
        fprintf(stderr, "RUN COMPLETE\n");

}



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
                printf("%s --tally-with-classes-multithread <pattern> <n> <nthreads>\n", argv[0]);
                /*printf("%s --benchmark\n", argv[0]);*/
                /*printf("%s --tally                          <pattern> <n> <tally file>\n", argv[0]);*/
                /*printf("%s --tally-multithread-config-write <perm_len> <num_blocks>\n", argv[0]);*/
                /*printf("%s --tally-multithread-config-read  <pattern> <config_path> \n", argv[0]);*/
                /*printf("%s --tally-from-start-state         <start> <stop> <idx> <i> <pattern> \n", argv[0]);*/

        /*} else if (!strcmp(argv[1], "--pset")) {*/

                /*struct pset_t set; */

                /*perm_t a = perm_from_csv("12,6,4,0,3,2,5,7,8,11,1,10,9");*/
                /*perm_t b = perm_from_csv("12,6,4,8,3,2,5,7,0,11,1,10,9");*/
                /*perm_t c = perm_from_csv("1,6,4,8,3,2,5,7,0,11,12,10,9");*/

                /*pset_init(&set, 13);*/

                /*pset_add(&set, a);*/
                /*pset_add(&set, b);*/
                /*pset_add(&set, c);*/

                /*FILE *f = fopen("pset.test", "w+");*/

                /*pset_write(&set, f);*/

        /*} else if (!strcmp(argv[1], "--benchmark")) {*/

                /*perm_t perm = perm_from_csv("12,6,4,0,3,2,5,7,8,11,1,10,9");*/
                /*perm_t patt = perm_from_csv("3,1,0,2");*/

                /*uint64_t t0;*/
                /*uint64_t t1;*/
                /*int   count;*/
               
                /*t0    = rdtsc(); */
                /*count = countfast(perm, patt, 13, 4); */
                /*t1    = rdtsc(); */

                /*printf("countfast() computed for n=13 k=4 in %"PRIu64" cycles\n", t1-t0);*/
                /*[>printf("pfast() called %d times\n", BM_pfast_call_count);<]*/


                /*t0    = rdtsc(); */
                /*count = countffast(perm, patt, 13, 4); */
                /*t1    = rdtsc(); */

                /*printf("countffast() computed for n=13 k=4 in %"PRIu64" cycles\n", t1-t0);*/
                /*[>printf("cache missed %d times\n", BM_pffast_cache_misses);<]*/

        } else if (!strcmp(argv[1], "--test")) {

                etc();
                /*prefix_test();*/
                /*counting_test();*/
        } else if (!strcmp(argv[1], "--try-convert")) {

		runit(argv[2]);

                /*uint64_t a = abs(INT_MIN - atoi(argv[2]));*/
                /*uint64_t b = INT_MAX;*/

                /*[>printf("%d\n", INT_MAX + 1);<]*/

                /*[>int n = atoi(argv[2]);<]*/
                /*printf("%"PRIu64"\n", a+b);*/
                /*[>prefix_test();<]*/
                /*counting_test();*/

        } else if (!strcmp(argv[1], "--frequency")) {

                char *pattern_string = argv[2];
                int n                = atoi(argv[3]);
                int threadcount      = atoi(argv[4]);

                perm_t pattern = perm_from_csv(pattern_string);
                perm_t perm    = perm_of_length(n);

                tally_with_classes(perm, pattern, threadcount);

        } else if (!strcmp(argv[1], "--ffrequency")) {

                char *pattern_string = argv[2];
                int n                = atoi(argv[3]);
                int threadcount      = atoi(argv[4]);

                perm_t pattern = perm_from_csv(pattern_string);
                perm_t perm    = perm_of_length(n);

                tally_with_classes_enhanced(perm, pattern, threadcount);

        /*} else if (!strcmp(argv[1], "--tally")) {*/
                /*char *pattern_string = argv[1];*/
                /*char *tally_file     = argv[3];*/
                /*int n                = atoi(argv[2]);*/

                /*Log = fopen(tally_file, "w+");*/

                /*perm_t pattern = perm_from_string(pattern_string);*/
                /*perm_t perm    = perm_of_length(n);*/

                /*[>***********************<]*/
                /*int nclasses = nchoosek(perm_length(perm), perm_length(pattern));*/

                /*struct pset_t *classes = calloc(1, nclasses*sizeof(struct pset_t));*/

                /*int i;*/
                /*for (i=0; i<nclasses; i++) {*/
                        /*pset_init(&classes[i], n);*/
                /*}*/
                /*[>***********************<]*/

                /*Total = factorial((uint64_t)n);*/

                /*generate_permutations_fast(perm, pattern);*/
                /*[>permute(perm, pattern, 0, perm_length(perm)-1); <]*/

                /*[>#ifdef GO_FAST<]*/
                /*write_tally(Log);*/
                /*[>#endif<]*/
        /*} else if (!strcmp(argv[1], "--tally-multithread")) {*/

                /*perm_t pattern = perm_from_csv(argv[2]);*/
                /*int n = atoi(argv[3]); */
                /*int threadcount = atoi(argv[4]);*/

                /*count_threaded(pattern, n, threadcount);*/
        } else if (!strcmp(argv[1], "--etc")) {
                etc();
        } else {
                printf("I don't understand.\n\nUSAGE:\n");
                printf("%s --frequency <pattern> <n> <nthreads>\n", argv[0]);
                /*printf("%s --benchmark\n", argv[0]);*/
                /*printf("%s --tally                          <pattern> <n> <tally file>\n", argv[0]);*/
                /*printf("%s --tally-multithread-config-write <perm_len> <num_blocks>\n", argv[0]);*/
                /*printf("%s --tally-multithread-config-read  <pattern> <config_path> \n", argv[0]);*/
                /*printf("%s --tally-from-start-state         <start> <stop> <idx> <i> <pattern> \n", argv[0]);*/
        }

        return 1;
}

