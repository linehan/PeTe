#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "ptable.h"

inline bool not_zero_perm(perm_t key) {
#if (numbits == 256) // In this case, because of how the boost library works, being zero as an integer is not the same as having all zero bytes.
  for (int i = 0; i < sizeof(key) / 8; i++) {
    if (*(((uint64_t *)(&key)) + i) != 0L) {
	return true;
    }
  }
  return false;
#endif
  return key != (perm_t)0;
}

inline void assert_key_nonzero (perm_t key) {
  assert(not_zero_perm(key));
}

/******************************************************************************
 * Ptable utility functions 
 ******************************************************************************/

/**
 * hash_perm()
 * -----------
 * Hashes a permutation
 *
 * @key_in : Permutation to be hashed (should be 64-bit integer)
 * @maxsize: Maximum value of hashed output 
 * Return  : Unsigned integer between 0 and (@maxsize-1)
 *
 * NOTE
 * Hash function from http://www.concentric.net/~ttwang/tech/inthash.htm
 *
 * TODO
 * Fix the stupid polymorphism going on here with the #bits
 */
unsigned long long hash_perm(perm_t key_in, uint64_t maxsize)
{
        /* 
         * Do stupid polymorphism 
         */

        #if (numbits <= 64)
        uint64_t key = key_in;
        #endif
        #if (numbits == 128)
        perm_t bits = ((perm_t) 1 << 64) - 1;
        uint64_t key = (uint64_t)(bits & (key_in + (key_in >> 61))); // for 128 bit case
        #endif
        #if (numbits == 256)
        perm_t bits = ((perm_t) 1 << 64) - 1;
        uint64_t key = (uint64_t)(bits & (key_in + (key_in >> 61) + (key_in >> 126) + (key_in >> 189)));
        #endif

        /* 
         * Do the hash
         */

        key = (~key) + (key << 21);            // (key << 21) - (key - 1)
        key = key ^ (key >> 24);
        key = (key + (key << 3)) + (key << 8); // key * 265
        key = key ^ (key >> 14);
        key = (key + (key << 2)) + (key << 4); // key * 21
        key = key ^ (key >> 28);
        key = key + (key << 31);

        /* Truncate to (@maxsize - 1) assuming @maxsize is power of 2 */
        return ((uint64_t)key) & (maxsize-1); 
}

static inline perm_t make_key_nonzero(perm_t key) 
{
        /* 
         * For all permutations we consider, adding 1 
         * will result in a non-zero integer.
         */
        return key + 1; 
}

static inline perm_t revert_stored_key(perm_t key) 
{
        return key - 1;
}

static unsigned long long next_power_of_two(unsigned long long v) 
{
        unsigned long long x = 1;
        while (x < v) x*=2;
        return x;
}

/*static inline int not_zero_perm(perm_t perm)*/
/*{*/
        /*return perm != (perm_t)1;*/
/*}*/


/******************************************************************************
 * DB structure and functions 
 ******************************************************************************/

void ptable_init(struct ptable_t *table, unsigned long long startsize)
{
        table->maxsize = next_power_of_two(startsize);

        table->array = calloc(1, table->maxsize*sizeof(perm_t));

        table->size = 0;
}

void ptable_insert(struct ptable_t *db, perm_t perm)
{
        perm_t element = make_key_nonzero(perm);

        assert_key_nonzero(element);

        if (db->size >= db->maxsize/2) { //up to 50% full before we resize
                printf("OVER MAX SIZE... NEED TO HANDLE THIS DIFFERENTLY\n");
                /*hashdb temp = hashdb(maxsize*2);*/
                /*perm_t *oldarray = array;*/
                
                /*for (unsigned long long x=0; x<maxsize; x++){*/
                        /*if (not_zero_perm(array[x])) { */
                                /*temp.add(revert_stored_key(array[x])); //add all of the original elements*/
                        /*}*/
                /*}*/
                /**this = temp;*/
                /*temp.array = oldarray; // so that when the destructor runs on temp, it frees the oldarray, not the new array.  It's ugly, but...*/
        }
        
        unsigned long long place = hash_perm(element, db->maxsize);
  
        while (not_zero_perm(db->array[place]) && db->array[place] != element) {
                place=(place+1) & (db->maxsize - 1);
        }
        
        if (db->array[place] != element) {
                db->size++;
                db->array[place] = element; // we insert element + 1 into the array
        }
}

bool ptable_contains(struct ptable_t *db, perm_t perm) 
{
        perm_t element = make_key_nonzero(perm);
  
        assert_key_nonzero(element);
  
        unsigned long long place = hash_perm(element, db->maxsize);
  
        while (not_zero_perm(db->array[place])) {
                if (db->array[place] == element) { 
                        return true; //look for element+1 in the array
                }
    
                place = (place + 1) & (db->maxsize - 1);
        }
        return false;
}

