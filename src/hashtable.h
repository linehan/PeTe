#ifndef _P_HASHTABLE_H_
#define _P_HASHTABLE_H_
#include <stdint.h>

struct hashtable {
        struct hashentry *entry; 
        int capacity;
        int size;
};

struct hashentry {
        uint64_t key;
        uint64_t val;
};


int hash_put(struct hashtable *table, uint64_t key, uint64_t val);
int hash_get(struct hashtable *table, uint64_t key, uint64_t *val);
int hash_del(struct hashtable *table, uint64_t key);
void hash_init_static(struct hashtable *table, struct hashentry *back, int n);
void hash_init_alloc(struct hashtable *table, int n);

#endif
