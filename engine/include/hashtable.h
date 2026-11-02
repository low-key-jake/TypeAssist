/**
 * TypeAssist — Hash Table
 *
 * A hash table with separate chaining for collision resolution.
 * Used for O(1) dictionary lookups and word frequency/metadata storage.
 *
 * Time Complexity:
 *   Insert:  O(1) amortised
 *   Lookup:  O(1) amortised
 *   Delete:  O(1) amortised
 *
 * Space Complexity: O(N) where N = number of entries
 */

#ifndef TYPEASSIST_HASHTABLE_H
#define TYPEASSIST_HASHTABLE_H

#include <stdbool.h>

#define HT_INITIAL_CAPACITY 4096
#define HT_LOAD_FACTOR_THRESHOLD 0.75
#define HT_MAX_KEY_LEN 64

/**
 * A single entry in a hash table chain.
 */
typedef struct HTEntry {
    char *key;
    int value;              /* Primary value (e.g., frequency) */
    struct HTEntry *next;   /* Next entry in chain (collision handling) */
} HTEntry;

/**
 * Hash table container.
 */
typedef struct {
    HTEntry **buckets;
    int capacity;
    int size;
} HashTable;

/* Lifecycle */
HashTable *ht_create(void);
HashTable *ht_create_with_capacity(int capacity);
void ht_destroy(HashTable *ht);

/* Core operations */
bool ht_insert(HashTable *ht, const char *key, int value);
bool ht_contains(const HashTable *ht, const char *key);
int  ht_get(const HashTable *ht, const char *key, int default_value);
bool ht_remove(HashTable *ht, const char *key);
bool ht_update(HashTable *ht, const char *key, int value);

/* Utility */
int  ht_size(const HashTable *ht);
void ht_clear(HashTable *ht);

/* Bulk loading */
int ht_load_from_buffer(HashTable *ht, const char *data, int data_len);

/* Iteration (for N-gram lookups) */
typedef void (*ht_iter_fn)(const char *key, int value, void *user_data);
void ht_foreach(const HashTable *ht, ht_iter_fn fn, void *user_data);

#endif /* TYPEASSIST_HASHTABLE_H */
