/**
 * TypeAssist — Hash Table Implementation
 *
 * Separate-chaining hash table with djb2 hashing and automatic resizing.
 */

#include "hashtable.h"

#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER) && !defined(strdup)
#define strdup _strdup
#endif

/* ------------------------------------------------------------------ */
/*  Internal helpers                                                    */
/* ------------------------------------------------------------------ */

/**
 * djb2 hash function (Dan Bernstein).
 * Produces a well-distributed 32-bit hash for ASCII strings.
 */
static unsigned long djb2_hash(const char *str)
{
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++) != 0) {
        hash = ((hash << 5) + hash) + (unsigned long)c;  /* hash * 33 + c */
    }
    return hash;
}

/**
 * Compute the bucket index for a given key.
 */
static int bucket_index(const char *key, int capacity)
{
    return (int)(djb2_hash(key) % (unsigned long)capacity);
}

/**
 * Create a new HTEntry node.  The key is duplicated with strdup.
 */
static HTEntry *entry_create(const char *key, int value)
{
    HTEntry *entry = (HTEntry *)malloc(sizeof(HTEntry));
    if (!entry) return NULL;

    entry->key = strdup(key);
    if (!entry->key) {
        free(entry);
        return NULL;
    }
    entry->value = value;
    entry->next  = NULL;
    return entry;
}

/**
 * Free a single entry (key + node).
 */
static void entry_destroy(HTEntry *entry)
{
    if (!entry) return;
    free(entry->key);
    free(entry);
}

/**
 * Resize the bucket array to `new_capacity`, rehashing all entries.
 * Returns true on success.
 */
static bool ht_resize(HashTable *ht, int new_capacity)
{
    HTEntry **new_buckets = (HTEntry **)calloc((size_t)new_capacity,
                                               sizeof(HTEntry *));
    if (!new_buckets) return false;

    /* Rehash every entry into the new array. */
    for (int i = 0; i < ht->capacity; i++) {
        HTEntry *cur = ht->buckets[i];
        while (cur) {
            HTEntry *next = cur->next;
            int idx       = bucket_index(cur->key, new_capacity);
            cur->next        = new_buckets[idx];
            new_buckets[idx] = cur;
            cur = next;
        }
    }

    free(ht->buckets);
    ht->buckets  = new_buckets;
    ht->capacity = new_capacity;
    return true;
}

/* ------------------------------------------------------------------ */
/*  Public API — Lifecycle                                              */
/* ------------------------------------------------------------------ */

HashTable *ht_create(void)
{
    return ht_create_with_capacity(HT_INITIAL_CAPACITY);
}

HashTable *ht_create_with_capacity(int capacity)
{
    if (capacity <= 0) capacity = HT_INITIAL_CAPACITY;

    HashTable *ht = (HashTable *)malloc(sizeof(HashTable));
    if (!ht) return NULL;

    ht->buckets = (HTEntry **)calloc((size_t)capacity, sizeof(HTEntry *));
    if (!ht->buckets) {
        free(ht);
        return NULL;
    }
    ht->capacity = capacity;
    ht->size     = 0;
    return ht;
}

void ht_destroy(HashTable *ht)
{
    if (!ht) return;
    ht_clear(ht);
    free(ht->buckets);
    free(ht);
}

/* ------------------------------------------------------------------ */
/*  Public API — Core operations                                        */
/* ------------------------------------------------------------------ */

bool ht_insert(HashTable *ht, const char *key, int value)
{
    if (!ht || !key) return false;

    /* If the key already exists, update its value. */
    int idx = bucket_index(key, ht->capacity);
    for (HTEntry *cur = ht->buckets[idx]; cur; cur = cur->next) {
        if (strcmp(cur->key, key) == 0) {
            cur->value = value;
            return true;
        }
    }

    /* Check load factor and resize if necessary (before inserting). */
    if ((double)(ht->size + 1) / (double)ht->capacity > HT_LOAD_FACTOR_THRESHOLD) {
        ht_resize(ht, ht->capacity * 2);
        /* Recompute index after potential resize. */
        idx = bucket_index(key, ht->capacity);
    }

    /* Create a new entry and prepend to the chain. */
    HTEntry *entry = entry_create(key, value);
    if (!entry) return false;

    entry->next      = ht->buckets[idx];
    ht->buckets[idx] = entry;
    ht->size++;
    return true;
}

bool ht_contains(const HashTable *ht, const char *key)
{
    if (!ht || !key) return false;

    int idx = bucket_index(key, ht->capacity);
    for (HTEntry *cur = ht->buckets[idx]; cur; cur = cur->next) {
        if (strcmp(cur->key, key) == 0) return true;
    }
    return false;
}

int ht_get(const HashTable *ht, const char *key, int default_value)
{
    if (!ht || !key) return default_value;

    int idx = bucket_index(key, ht->capacity);
    for (HTEntry *cur = ht->buckets[idx]; cur; cur = cur->next) {
        if (strcmp(cur->key, key) == 0) return cur->value;
    }
    return default_value;
}

bool ht_remove(HashTable *ht, const char *key)
{
    if (!ht || !key) return false;

    int idx = bucket_index(key, ht->capacity);
    HTEntry *prev = NULL;
    HTEntry *cur  = ht->buckets[idx];

    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            if (prev)
                prev->next = cur->next;
            else
                ht->buckets[idx] = cur->next;
            entry_destroy(cur);
            ht->size--;
            return true;
        }
        prev = cur;
        cur  = cur->next;
    }
    return false;
}

bool ht_update(HashTable *ht, const char *key, int value)
{
    if (!ht || !key) return false;

    int idx = bucket_index(key, ht->capacity);
    for (HTEntry *cur = ht->buckets[idx]; cur; cur = cur->next) {
        if (strcmp(cur->key, key) == 0) {
            cur->value = value;
            return true;
        }
    }
    return false;  /* key not found */
}

/* ------------------------------------------------------------------ */
/*  Public API — Utility                                                */
/* ------------------------------------------------------------------ */

int ht_size(const HashTable *ht)
{
    return ht ? ht->size : 0;
}

void ht_clear(HashTable *ht)
{
    if (!ht) return;
    for (int i = 0; i < ht->capacity; i++) {
        HTEntry *cur = ht->buckets[i];
        while (cur) {
            HTEntry *next = cur->next;
            entry_destroy(cur);
            cur = next;
        }
        ht->buckets[i] = NULL;
    }
    ht->size = 0;
}

/* ------------------------------------------------------------------ */
/*  Public API — Bulk loading                                           */
/* ------------------------------------------------------------------ */

/**
 * Parse a buffer of "key\tvalue\n" lines and insert each pair.
 * Returns the number of entries successfully inserted.
 */
int ht_load_from_buffer(HashTable *ht, const char *data, int data_len)
{
    if (!ht || !data || data_len <= 0) return 0;

    int loaded = 0;
    int pos    = 0;

    while (pos < data_len) {
        int line_start = pos;
        while (pos < data_len && data[pos] != '\n') pos++;

        int line_len = pos - line_start;
        if (pos < data_len) pos++;  /* skip newline */

        if (line_len == 0) continue;

        /* Find the tab or space separator. */
        int tab_pos = -1;
        for (int i = 0; i < line_len; i++) {
            if (data[line_start + i] == '\t' || data[line_start + i] == ' ') {
                tab_pos = i;
                break;
            }
        }

        if (tab_pos <= 0) continue;  /* no tab or empty key — skip line */

        /* Extract key (up to HT_MAX_KEY_LEN-1 chars). */
        char key[HT_MAX_KEY_LEN];
        int key_len = tab_pos < HT_MAX_KEY_LEN - 1
                        ? tab_pos
                        : HT_MAX_KEY_LEN - 1;
        memcpy(key, data + line_start, (size_t)key_len);
        key[key_len] = '\0';

        /* Parse value. */
        int value = atoi(data + line_start + tab_pos + 1);

        if (ht_insert(ht, key, value)) {
            loaded++;
        }
    }
    return loaded;
}

/* ------------------------------------------------------------------ */
/*  Public API — Iteration                                              */
/* ------------------------------------------------------------------ */

void ht_foreach(const HashTable *ht, ht_iter_fn fn, void *user_data)
{
    if (!ht || !fn) return;

    for (int i = 0; i < ht->capacity; i++) {
        for (HTEntry *cur = ht->buckets[i]; cur; cur = cur->next) {
            fn(cur->key, cur->value, user_data);
        }
    }
}
