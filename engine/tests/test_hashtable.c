/**
 * TypeAssist — Hash Table Tests
 *
 * Comprehensive test suite for the Hash Table data structure.
 * Uses simple assert-based testing with descriptive function names.
 */

#include "hashtable.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/*  Helper macros                                                       */
/* ------------------------------------------------------------------ */

#define RUN_TEST(fn)                          \
    do {                                      \
        printf("  %-50s", #fn);               \
        fn();                                 \
        printf("PASS\n");                     \
        pass_count++;                         \
    } while (0)

static int pass_count = 0;

/* ------------------------------------------------------------------ */
/*  Test functions                                                      */
/* ------------------------------------------------------------------ */

static void test_create_and_destroy(void)
{
    HashTable *ht = ht_create();
    assert(ht != NULL);
    assert(ht_size(ht) == 0);
    ht_destroy(ht);
}

static void test_create_with_capacity(void)
{
    HashTable *ht = ht_create_with_capacity(16);
    assert(ht != NULL);
    assert(ht_size(ht) == 0);
    ht_destroy(ht);
}

static void test_insert_and_get(void)
{
    HashTable *ht = ht_create();
    assert(ht_insert(ht, "hello", 42));
    assert(ht_get(ht, "hello", -1) == 42);
    assert(ht_size(ht) == 1);
    ht_destroy(ht);
}

static void test_contains(void)
{
    HashTable *ht = ht_create();
    ht_insert(ht, "key1", 1);
    assert(ht_contains(ht, "key1"));
    assert(!ht_contains(ht, "key2"));
    ht_destroy(ht);
}

static void test_get_default_value(void)
{
    HashTable *ht = ht_create();
    assert(ht_get(ht, "missing", -999) == -999);
    ht_destroy(ht);
}

static void test_remove(void)
{
    HashTable *ht = ht_create();
    ht_insert(ht, "a", 1);
    ht_insert(ht, "b", 2);
    assert(ht_size(ht) == 2);

    assert(ht_remove(ht, "a"));
    assert(!ht_contains(ht, "a"));
    assert(ht_size(ht) == 1);

    /* Removing non-existent key returns false. */
    assert(!ht_remove(ht, "zzz"));
    assert(ht_size(ht) == 1);

    ht_destroy(ht);
}

static void test_update_existing_key(void)
{
    HashTable *ht = ht_create();
    ht_insert(ht, "counter", 0);
    assert(ht_update(ht, "counter", 10));
    assert(ht_get(ht, "counter", -1) == 10);
    assert(ht_size(ht) == 1);  /* size unchanged */
    ht_destroy(ht);
}

static void test_update_nonexistent_key(void)
{
    HashTable *ht = ht_create();
    assert(!ht_update(ht, "ghost", 5));
    assert(ht_size(ht) == 0);
    ht_destroy(ht);
}

static void test_insert_overwrites_value(void)
{
    HashTable *ht = ht_create();
    ht_insert(ht, "x", 1);
    ht_insert(ht, "x", 2);
    assert(ht_get(ht, "x", -1) == 2);
    assert(ht_size(ht) == 1);  /* still one entry */
    ht_destroy(ht);
}

static void test_collision_handling(void)
{
    /* Use a small capacity to force collisions. */
    HashTable *ht = ht_create_with_capacity(4);

    for (int i = 0; i < 20; i++) {
        char key[16];
        snprintf(key, sizeof(key), "key%d", i);
        assert(ht_insert(ht, key, i));
    }

    assert(ht_size(ht) == 20);

    for (int i = 0; i < 20; i++) {
        char key[16];
        snprintf(key, sizeof(key), "key%d", i);
        assert(ht_contains(ht, key));
        assert(ht_get(ht, key, -1) == i);
    }

    ht_destroy(ht);
}

static void test_resize(void)
{
    /* Start with tiny capacity to trigger multiple resizes. */
    HashTable *ht = ht_create_with_capacity(4);

    /* Insert enough entries to exceed load factor multiple times:
     * capacity 4 -> threshold at 3 entries -> resize to 8
     * capacity 8 -> threshold at 6 entries -> resize to 16
     * ...
     */
    for (int i = 0; i < 50; i++) {
        char key[16];
        snprintf(key, sizeof(key), "item%d", i);
        ht_insert(ht, key, i * 10);
    }

    assert(ht_size(ht) == 50);
    /* Capacity should have grown well beyond initial 4. */
    assert(ht->capacity > 4);

    /* Verify all entries survived rehashing. */
    for (int i = 0; i < 50; i++) {
        char key[16];
        snprintf(key, sizeof(key), "item%d", i);
        assert(ht_get(ht, key, -1) == i * 10);
    }

    ht_destroy(ht);
}

static void test_clear(void)
{
    HashTable *ht = ht_create();
    ht_insert(ht, "a", 1);
    ht_insert(ht, "b", 2);
    assert(ht_size(ht) == 2);

    ht_clear(ht);
    assert(ht_size(ht) == 0);
    assert(!ht_contains(ht, "a"));
    assert(!ht_contains(ht, "b"));

    /* Should still be usable after clearing. */
    ht_insert(ht, "c", 3);
    assert(ht_size(ht) == 1);
    assert(ht_get(ht, "c", -1) == 3);

    ht_destroy(ht);
}

static void test_load_from_buffer(void)
{
    HashTable *ht = ht_create();
    const char *buf = "apple\t10\nbanana\t20\ncherry\t30\n";
    int loaded = ht_load_from_buffer(ht, buf, (int)strlen(buf));

    assert(loaded == 3);
    assert(ht_get(ht, "apple", -1) == 10);
    assert(ht_get(ht, "banana", -1) == 20);
    assert(ht_get(ht, "cherry", -1) == 30);
    ht_destroy(ht);
}

static void test_load_from_buffer_empty_lines(void)
{
    HashTable *ht = ht_create();
    const char *buf = "\nfoo\t1\n\nbar\t2\n\n";
    int loaded = ht_load_from_buffer(ht, buf, (int)strlen(buf));

    assert(loaded == 2);
    assert(ht_get(ht, "foo", -1) == 1);
    assert(ht_get(ht, "bar", -1) == 2);
    ht_destroy(ht);
}

static void test_load_from_buffer_null(void)
{
    HashTable *ht = ht_create();
    assert(ht_load_from_buffer(NULL, "a\t1\n", 4) == 0);
    assert(ht_load_from_buffer(ht, NULL, 10) == 0);
    assert(ht_load_from_buffer(ht, "a\t1\n", 0) == 0);
    ht_destroy(ht);
}

/* Callback state for foreach test. */
typedef struct {
    int sum;
    int count;
} ForeachState;

static void sum_callback(const char *key, int value, void *user_data)
{
    (void)key;  /* unused */
    ForeachState *state = (ForeachState *)user_data;
    state->sum += value;
    state->count++;
}

static void test_foreach(void)
{
    HashTable *ht = ht_create();
    ht_insert(ht, "a", 10);
    ht_insert(ht, "b", 20);
    ht_insert(ht, "c", 30);

    ForeachState state = { 0, 0 };
    ht_foreach(ht, sum_callback, &state);

    assert(state.count == 3);
    assert(state.sum == 60);
    ht_destroy(ht);
}

static void test_null_inputs(void)
{
    HashTable *ht = ht_create();

    assert(!ht_insert(NULL, "k", 1));
    assert(!ht_insert(ht, NULL, 1));
    assert(!ht_contains(NULL, "k"));
    assert(!ht_contains(ht, NULL));
    assert(ht_get(NULL, "k", -1) == -1);
    assert(ht_get(ht, NULL, -1) == -1);
    assert(!ht_remove(NULL, "k"));
    assert(!ht_remove(ht, NULL));
    assert(!ht_update(NULL, "k", 1));
    assert(!ht_update(ht, NULL, 1));
    assert(ht_size(NULL) == 0);

    ht_foreach(NULL, sum_callback, NULL);  /* should not crash */
    ht_foreach(ht, NULL, NULL);            /* should not crash */

    ht_destroy(ht);
    ht_destroy(NULL);  /* should not crash */
}

static void test_empty_string_key(void)
{
    HashTable *ht = ht_create();
    assert(ht_insert(ht, "", 42));
    assert(ht_contains(ht, ""));
    assert(ht_get(ht, "", -1) == 42);
    assert(ht_remove(ht, ""));
    assert(!ht_contains(ht, ""));
    ht_destroy(ht);
}

static void test_remove_and_reinsert(void)
{
    HashTable *ht = ht_create();
    ht_insert(ht, "temp", 1);
    assert(ht_remove(ht, "temp"));
    assert(!ht_contains(ht, "temp"));

    ht_insert(ht, "temp", 999);
    assert(ht_get(ht, "temp", -1) == 999);
    ht_destroy(ht);
}

static void test_many_entries(void)
{
    HashTable *ht = ht_create();
    char key[32];

    for (int i = 0; i < 10000; i++) {
        snprintf(key, sizeof(key), "entry_%d", i);
        assert(ht_insert(ht, key, i));
    }
    assert(ht_size(ht) == 10000);

    /* Verify all entries. */
    for (int i = 0; i < 10000; i++) {
        snprintf(key, sizeof(key), "entry_%d", i);
        assert(ht_get(ht, key, -1) == i);
    }

    /* Remove half. */
    for (int i = 0; i < 5000; i++) {
        snprintf(key, sizeof(key), "entry_%d", i);
        assert(ht_remove(ht, key));
    }
    assert(ht_size(ht) == 5000);

    /* Remaining half should still be intact. */
    for (int i = 5000; i < 10000; i++) {
        snprintf(key, sizeof(key), "entry_%d", i);
        assert(ht_get(ht, key, -1) == i);
    }

    ht_destroy(ht);
}

/* ------------------------------------------------------------------ */
/*  Main                                                                */
/* ------------------------------------------------------------------ */

int main(void)
{
    printf("===== Hash Table Test Suite =====\n");

    RUN_TEST(test_create_and_destroy);
    RUN_TEST(test_create_with_capacity);
    RUN_TEST(test_insert_and_get);
    RUN_TEST(test_contains);
    RUN_TEST(test_get_default_value);
    RUN_TEST(test_remove);
    RUN_TEST(test_update_existing_key);
    RUN_TEST(test_update_nonexistent_key);
    RUN_TEST(test_insert_overwrites_value);
    RUN_TEST(test_collision_handling);
    RUN_TEST(test_resize);
    RUN_TEST(test_clear);
    RUN_TEST(test_load_from_buffer);
    RUN_TEST(test_load_from_buffer_empty_lines);
    RUN_TEST(test_load_from_buffer_null);
    RUN_TEST(test_foreach);
    RUN_TEST(test_null_inputs);
    RUN_TEST(test_empty_string_key);
    RUN_TEST(test_remove_and_reinsert);
    RUN_TEST(test_many_entries);

    printf("\nAll %d tests PASSED.\n", pass_count);
    return 0;
}
