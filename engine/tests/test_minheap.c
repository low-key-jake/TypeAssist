/**
 * TypeAssist — Min-Heap Tests
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../include/minheap.h"

/* ---- Test: basic create / destroy ---- */
static void test_create_destroy(void)
{
    MinHeap *h = heap_create();
    assert(h != NULL);
    assert(heap_size(h) == 0);
    assert(heap_is_empty(h));
    heap_destroy(h);

    MinHeap *h2 = heap_create_with_capacity(4);
    assert(h2 != NULL);
    heap_destroy(h2);
    printf("  PASS  test_create_destroy\n");
}

/* ---- Test: insert and extract in min order ---- */
static void test_insert_extract_order(void)
{
    MinHeap *h = heap_create();
    assert(heap_insert(h, "cat",    3, 100));
    assert(heap_insert(h, "car",    1, 200));
    assert(heap_insert(h, "card",   2, 150));

    assert(heap_size(h) == 3);

    HeapElement e;
    assert(heap_extract_min(h, &e));
    assert(e.score == 1);
    assert(strcmp(e.word, "car") == 0);

    assert(heap_extract_min(h, &e));
    assert(e.score == 2);
    assert(strcmp(e.word, "card") == 0);

    assert(heap_extract_min(h, &e));
    assert(e.score == 3);
    assert(strcmp(e.word, "cat") == 0);

    assert(heap_is_empty(h));
    heap_destroy(h);
    printf("  PASS  test_insert_extract_order\n");
}

/* ---- Test: tie-breaking by frequency (higher freq wins) ---- */
static void test_tiebreak_frequency(void)
{
    MinHeap *h = heap_create();
    assert(heap_insert(h, "alpha",  2, 50));
    assert(heap_insert(h, "bravo",  2, 200));
    assert(heap_insert(h, "charlie", 2, 100));

    HeapElement e;
    assert(heap_extract_min(h, &e));
    assert(e.score == 2 && e.frequency == 200);
    assert(strcmp(e.word, "bravo") == 0);

    assert(heap_extract_min(h, &e));
    assert(e.score == 2 && e.frequency == 100);
    assert(strcmp(e.word, "charlie") == 0);

    assert(heap_extract_min(h, &e));
    assert(e.score == 2 && e.frequency == 50);
    assert(strcmp(e.word, "alpha") == 0);

    heap_destroy(h);
    printf("  PASS  test_tiebreak_frequency\n");
}

/* ---- Test: empty heap operations ---- */
static void test_empty_heap(void)
{
    MinHeap *h = heap_create();
    HeapElement e;

    assert(!heap_extract_min(h, &e));
    assert(!heap_peek(h, &e));
    assert(heap_is_empty(h));
    assert(heap_size(h) == 0);

    heap_destroy(h);
    printf("  PASS  test_empty_heap\n");
}

/* ---- Test: peek without removing ---- */
static void test_peek(void)
{
    MinHeap *h = heap_create();
    heap_insert(h, "dog", 5, 10);
    heap_insert(h, "dot", 1, 20);

    HeapElement e;
    assert(heap_peek(h, &e));
    assert(e.score == 1);
    assert(strcmp(e.word, "dot") == 0);
    assert(heap_size(h) == 2);   /* size unchanged */

    /* Peek again — still the same */
    assert(heap_peek(h, &e));
    assert(e.score == 1);

    heap_destroy(h);
    printf("  PASS  test_peek\n");
}

/* ---- Test: interleaved insert and extract ---- */
static void test_interleaved(void)
{
    MinHeap *h = heap_create();
    heap_insert(h, "x", 10, 1);
    heap_insert(h, "y", 5,  1);

    HeapElement e;
    assert(heap_extract_min(h, &e));
    assert(e.score == 5);

    heap_insert(h, "z", 3, 1);
    assert(heap_extract_min(h, &e));
    assert(e.score == 3);

    assert(heap_extract_min(h, &e));
    assert(e.score == 10);

    assert(heap_is_empty(h));
    heap_destroy(h);
    printf("  PASS  test_interleaved\n");
}

/* ---- Test: clear and reuse ---- */
static void test_clear_reuse(void)
{
    MinHeap *h = heap_create();
    heap_insert(h, "a", 1, 1);
    heap_insert(h, "b", 2, 1);
    heap_insert(h, "c", 3, 1);

    heap_clear(h);
    assert(heap_is_empty(h));
    assert(heap_size(h) == 0);

    /* Reuse after clear */
    heap_insert(h, "d", 7, 1);
    HeapElement e;
    assert(heap_extract_min(h, &e));
    assert(e.score == 7);
    assert(strcmp(e.word, "d") == 0);

    heap_destroy(h);
    printf("  PASS  test_clear_reuse\n");
}

/* ---- Test: growth beyond initial capacity ---- */
static void test_growth(void)
{
    MinHeap *h = heap_create_with_capacity(2);
    for (int i = 0; i < 100; i++) {
        char word[16];
        snprintf(word, sizeof(word), "w%d", i);
        assert(heap_insert(h, word, 100 - i, i));
    }
    assert(heap_size(h) == 100);

    HeapElement e;
    int prev_score = -1;
    for (int i = 0; i < 100; i++) {
        assert(heap_extract_min(h, &e));
        assert(e.score >= prev_score);
        prev_score = e.score;
    }

    heap_destroy(h);
    printf("  PASS  test_growth\n");
}

/* ---- Test: NULL inputs ---- */
static void test_null_inputs(void)
{
    assert(!heap_insert(NULL, "word", 1, 1));
    assert(!heap_extract_min(NULL, NULL));
    assert(!heap_peek(NULL, NULL));
    assert(heap_is_empty(NULL));
    assert(heap_size(NULL) == 0);

    MinHeap *h = heap_create();
    assert(!heap_insert(h, NULL, 1, 1));
    heap_destroy(h);

    heap_clear(NULL);   /* should not crash */
    heap_destroy(NULL); /* should not crash */
    printf("  PASS  test_null_inputs\n");
}

int main(void)
{
    printf("=== Min-Heap Tests ===\n");
    test_create_destroy();
    test_insert_extract_order();
    test_tiebreak_frequency();
    test_empty_heap();
    test_peek();
    test_interleaved();
    test_clear_reuse();
    test_growth();
    test_null_inputs();
    printf("All min-heap tests passed!\n");
    return 0;
}
