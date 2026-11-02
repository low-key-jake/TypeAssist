/**
 * TypeAssist — Edit Distance Tests
 */

#include <assert.h>
#include <stdio.h>
#include "../include/edit_distance.h"

/* ---- Test: NULL inputs ---- */
static void test_null_inputs(void)
{
    assert(levenshtein(NULL, "hello") == -1);
    assert(levenshtein("hello", NULL) == -1);
    assert(levenshtein(NULL, NULL) == -1);

    assert(damerau_levenshtein(NULL, "hello") == -1);
    assert(damerau_levenshtein("hello", NULL) == -1);

    assert(optimal_string_alignment(NULL, "hello") == -1);
    assert(optimal_string_alignment("hello", NULL) == -1);
    printf("  PASS  test_null_inputs\n");
}

/* ---- Test: identical strings ---- */
static void test_identical(void)
{
    assert(levenshtein("hello", "hello") == 0);
    assert(damerau_levenshtein("hello", "hello") == 0);
    assert(optimal_string_alignment("hello", "hello") == 0);

    assert(levenshtein("", "") == 0);
    assert(damerau_levenshtein("", "") == 0);
    assert(optimal_string_alignment("", "") == 0);
    printf("  PASS  test_identical\n");
}

/* ---- Test: empty strings ---- */
static void test_empty_strings(void)
{
    assert(levenshtein("", "abc") == 3);
    assert(levenshtein("abc", "") == 3);

    assert(damerau_levenshtein("", "abc") == 3);
    assert(damerau_levenshtein("abc", "") == 3);

    assert(optimal_string_alignment("", "abc") == 3);
    assert(optimal_string_alignment("abc", "") == 3);
    printf("  PASS  test_empty_strings\n");
}

/* ---- Test: single character operations ---- */
static void test_single_ops(void)
{
    /* Insertion */
    assert(levenshtein("abc", "abcd") == 1);
    /* Deletion */
    assert(levenshtein("abcd", "abc") == 1);
    /* Substitution */
    assert(levenshtein("abc", "axc") == 1);
    /* Transposition (levenshtein counts as 2, damerau counts as 1) */
    assert(levenshtein("ab", "ba") == 2);
    assert(damerau_levenshtein("ab", "ba") == 1);
    assert(optimal_string_alignment("ab", "ba") == 1);
    printf("  PASS  test_single_ops\n");
}

/* ---- Test: "recieve" -> "receive" (transposition) ---- */
static void test_recieve(void)
{
    /* Transposition of 'ie' -> 'ei': distance 1 for DL and OSA */
    assert(damerau_levenshtein("recieve", "receive") == 1);
    assert(optimal_string_alignment("recieve", "receive") == 1);
    /* Levenshtein (no transposition): 2 */
    assert(levenshtein("recieve", "receive") == 2);
    printf("  PASS  test_recieve\n");
}

/* ---- Test: "kitten" -> "sitting" ---- */
static void test_kitten_sitting(void)
{
    /* Classic example: kitten -> sitting = 3 */
    assert(levenshtein("kitten", "sitting") == 3);
    assert(damerau_levenshtein("kitten", "sitting") == 3);
    assert(optimal_string_alignment("kitten", "sitting") == 3);
    printf("  PASS  test_kitten_sitting\n");
}

/* ---- Test: "saturday" -> "sunday" ---- */
static void test_saturday_sunday(void)
{
    /* saturday -> sunday = 3 */
    assert(levenshtein("saturday", "sunday") == 3);
    assert(damerau_levenshtein("saturday", "sunday") == 3);
    assert(optimal_string_alignment("saturday", "sunday") == 3);
    printf("  PASS  test_saturday_sunday\n");
}

/* ---- Test: DL vs OSA on known differing case ---- */
static void test_dl_vs_osa(void)
{
    /*
     * Classic case where DL and OSA differ:
     * "CA" -> "ABC"
     *   OSA = 3  (cannot re-edit a transposed substring)
     *   DL  = 2  (transpose CA->AC, then insert B: AC->ABC)
     */
    int dl  = damerau_levenshtein("CA", "ABC");
    int osa = optimal_string_alignment("CA", "ABC");
    assert(dl == 2);
    assert(osa == 3);
    printf("  PASS  test_dl_vs_osa\n");
}

/* ---- Test: longer strings ---- */
static void test_longer(void)
{
    assert(levenshtein("abcdef", "azced") == 3);
    assert(damerau_levenshtein("abcdef", "abcfed") == 1);
    printf("  PASS  test_longer\n");
}

/* ---- Test: single character strings ---- */
static void test_single_char(void)
{
    assert(levenshtein("a", "b") == 1);
    assert(levenshtein("a", "a") == 0);
    assert(levenshtein("a", "") == 1);
    assert(levenshtein("", "a") == 1);
    printf("  PASS  test_single_char\n");
}

int main(void)
{
    printf("=== Edit Distance Tests ===\n");
    test_null_inputs();
    test_identical();
    test_empty_strings();
    test_single_ops();
    test_recieve();
    test_kitten_sitting();
    test_saturday_sunday();
    test_dl_vs_osa();
    test_longer();
    test_single_char();
    printf("All edit distance tests passed!\n");
    return 0;
}
