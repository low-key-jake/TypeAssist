/**
 * TypeAssist — Trie Tests
 *
 * Comprehensive test suite for the Trie data structure.
 * Uses simple assert-based testing with descriptive function names.
 */

#include "trie.h"

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
    Trie *t = trie_create();
    assert(t != NULL);
    assert(trie_word_count(t) == 0);
    assert(trie_node_count(t) == 1);  /* root node */
    trie_destroy(t);
}

static void test_insert_and_search_single_word(void)
{
    Trie *t = trie_create();
    assert(trie_insert(t, "hello", 5));
    assert(trie_search(t, "hello"));
    assert(!trie_search(t, "hell"));
    assert(!trie_search(t, "helloo"));
    trie_destroy(t);
}

static void test_insert_duplicate_updates_frequency(void)
{
    Trie *t = trie_create();
    trie_insert(t, "cat", 3);
    assert(trie_get_frequency(t, "cat") == 3);

    /* Insert again with different frequency — should update, not add. */
    trie_insert(t, "cat", 10);
    assert(trie_get_frequency(t, "cat") == 10);
    assert(trie_word_count(t) == 1);
    trie_destroy(t);
}

static void test_search_nonexistent_word(void)
{
    Trie *t = trie_create();
    trie_insert(t, "apple", 1);
    assert(!trie_search(t, "banana"));
    assert(!trie_search(t, "app"));
    assert(!trie_search(t, "apples"));
    trie_destroy(t);
}

static void test_starts_with(void)
{
    Trie *t = trie_create();
    trie_insert(t, "apple", 1);
    trie_insert(t, "application", 2);
    assert(trie_starts_with(t, "app"));
    assert(trie_starts_with(t, "apple"));
    assert(trie_starts_with(t, "applic"));
    assert(!trie_starts_with(t, "ban"));
    assert(trie_starts_with(t, ""));  /* empty prefix matches root */
    trie_destroy(t);
}

static void test_autocomplete_frequency_ordering(void)
{
    Trie *t = trie_create();
    trie_insert(t, "apple",       50);
    trie_insert(t, "app",         10);
    trie_insert(t, "application", 100);
    trie_insert(t, "apply",       80);
    trie_insert(t, "banana",      200);

    TrieSuggestion results[TRIE_MAX_SUGGESTIONS];
    int n = trie_autocomplete(t, "app", results, TRIE_MAX_SUGGESTIONS);

    assert(n == 4);
    /* Expect descending frequency: application(100), apply(80), apple(50), app(10) */
    assert(results[0].frequency == 100);
    assert(strcmp(results[0].word, "application") == 0);
    assert(results[1].frequency == 80);
    assert(strcmp(results[1].word, "apply") == 0);
    assert(results[2].frequency == 50);
    assert(strcmp(results[2].word, "apple") == 0);
    assert(results[3].frequency == 10);
    assert(strcmp(results[3].word, "app") == 0);

    trie_destroy(t);
}

static void test_autocomplete_max_results(void)
{
    Trie *t = trie_create();
    trie_insert(t, "aaa", 1);
    trie_insert(t, "aab", 2);
    trie_insert(t, "aac", 3);
    trie_insert(t, "aad", 4);
    trie_insert(t, "aae", 5);

    TrieSuggestion results[2];
    int n = trie_autocomplete(t, "aa", results, 2);
    assert(n == 2);
    assert(results[0].frequency == 5);
    assert(results[1].frequency == 4);
    trie_destroy(t);
}

static void test_autocomplete_no_match(void)
{
    Trie *t = trie_create();
    trie_insert(t, "hello", 1);

    TrieSuggestion results[5];
    int n = trie_autocomplete(t, "xyz", results, 5);
    assert(n == 0);
    trie_destroy(t);
}

static void test_case_insensitivity(void)
{
    Trie *t = trie_create();
    trie_insert(t, "Hello", 5);
    assert(trie_search(t, "hello"));
    assert(trie_search(t, "HELLO"));
    assert(trie_search(t, "HeLLo"));
    assert(trie_word_count(t) == 1);
    trie_destroy(t);
}

static void test_apostrophe_handling(void)
{
    Trie *t = trie_create();
    trie_insert(t, "don't", 10);
    assert(trie_search(t, "don't"));
    assert(trie_get_frequency(t, "don't") == 10);
    assert(trie_starts_with(t, "don'"));
    trie_destroy(t);
}

static void test_empty_string(void)
{
    Trie *t = trie_create();
    /* Inserting an empty string marks the root as end-of-word. */
    trie_insert(t, "", 1);
    assert(trie_search(t, ""));
    assert(trie_word_count(t) == 1);
    trie_destroy(t);
}

static void test_null_inputs(void)
{
    Trie *t = trie_create();

    assert(!trie_insert(NULL, "word", 1));
    assert(!trie_insert(t, NULL, 1));
    assert(!trie_search(NULL, "word"));
    assert(!trie_search(t, NULL));
    assert(!trie_starts_with(NULL, "w"));
    assert(!trie_starts_with(t, NULL));
    assert(trie_get_frequency(NULL, "w") == 0);
    assert(trie_get_frequency(t, NULL) == 0);

    TrieSuggestion res[5];
    assert(trie_autocomplete(NULL, "w", res, 5) == 0);
    assert(trie_autocomplete(t, NULL, res, 5) == 0);
    assert(trie_autocomplete(t, "w", NULL, 5) == 0);
    assert(trie_autocomplete(t, "w", res, 0) == 0);

    assert(trie_word_count(NULL) == 0);
    assert(trie_node_count(NULL) == 0);

    trie_destroy(t);
    trie_destroy(NULL);  /* should not crash */
}

static void test_load_from_buffer(void)
{
    Trie *t = trie_create();
    const char *buf = "hello\t100\nworld\t50\napple\t75\n";
    int loaded = trie_load_from_buffer(t, buf, (int)strlen(buf));

    assert(loaded == 3);
    assert(trie_search(t, "hello"));
    assert(trie_get_frequency(t, "hello") == 100);
    assert(trie_search(t, "world"));
    assert(trie_get_frequency(t, "world") == 50);
    assert(trie_search(t, "apple"));
    assert(trie_get_frequency(t, "apple") == 75);
    trie_destroy(t);
}

static void test_load_from_buffer_no_tab(void)
{
    Trie *t = trie_create();
    const char *buf = "alpha\nbeta\n";
    int loaded = trie_load_from_buffer(t, buf, (int)strlen(buf));

    assert(loaded == 2);
    assert(trie_get_frequency(t, "alpha") == 1);
    assert(trie_get_frequency(t, "beta") == 1);
    trie_destroy(t);
}

static void test_load_from_buffer_null(void)
{
    Trie *t = trie_create();
    assert(trie_load_from_buffer(NULL, "a\t1\n", 4) == 0);
    assert(trie_load_from_buffer(t, NULL, 10) == 0);
    assert(trie_load_from_buffer(t, "a\t1\n", 0) == 0);
    trie_destroy(t);
}

static void test_word_and_node_count(void)
{
    Trie *t = trie_create();
    assert(trie_word_count(t) == 0);
    assert(trie_node_count(t) == 1);  /* root */

    trie_insert(t, "a", 1);
    assert(trie_word_count(t) == 1);
    assert(trie_node_count(t) == 2);  /* root + 'a' */

    trie_insert(t, "ab", 1);
    assert(trie_word_count(t) == 2);
    assert(trie_node_count(t) == 3);  /* root + 'a' + 'b' */

    /* Duplicate should not increase counts. */
    trie_insert(t, "ab", 5);
    assert(trie_word_count(t) == 2);
    assert(trie_node_count(t) == 3);

    trie_destroy(t);
}

static void test_many_words(void)
{
    Trie *t = trie_create();
    char word[TRIE_MAX_WORD_LEN];

    /* Insert 100 words: w0, w1, ... w99 */
    for (int i = 0; i < 100; i++) {
        snprintf(word, sizeof(word), "word%d", i);
        trie_insert(t, word, i);
    }
    assert(trie_word_count(t) == 100);

    for (int i = 0; i < 100; i++) {
        snprintf(word, sizeof(word), "word%d", i);
        assert(trie_search(t, word));
        assert(trie_get_frequency(t, word) == i);
    }
    trie_destroy(t);
}

static void test_skip_invalid_chars(void)
{
    Trie *t = trie_create();
    /* Digits and special chars should be silently skipped. */
    trie_insert(t, "he1lo", 1);
    assert(trie_search(t, "helo"));  /* '1' was skipped */
    trie_destroy(t);
}

/* ------------------------------------------------------------------ */
/*  Main                                                                */
/* ------------------------------------------------------------------ */

int main(void)
{
    printf("===== Trie Test Suite =====\n");

    RUN_TEST(test_create_and_destroy);
    RUN_TEST(test_insert_and_search_single_word);
    RUN_TEST(test_insert_duplicate_updates_frequency);
    RUN_TEST(test_search_nonexistent_word);
    RUN_TEST(test_starts_with);
    RUN_TEST(test_autocomplete_frequency_ordering);
    RUN_TEST(test_autocomplete_max_results);
    RUN_TEST(test_autocomplete_no_match);
    RUN_TEST(test_case_insensitivity);
    RUN_TEST(test_apostrophe_handling);
    RUN_TEST(test_empty_string);
    RUN_TEST(test_null_inputs);
    RUN_TEST(test_load_from_buffer);
    RUN_TEST(test_load_from_buffer_no_tab);
    RUN_TEST(test_load_from_buffer_null);
    RUN_TEST(test_word_and_node_count);
    RUN_TEST(test_many_words);
    RUN_TEST(test_skip_invalid_chars);

    printf("\nAll %d tests PASSED.\n", pass_count);
    return 0;
}
