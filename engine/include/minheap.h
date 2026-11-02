/**
 * TypeAssist — Min-Heap (Priority Queue)
 *
 * A binary min-heap for ranking spelling correction candidates.
 * The element with the smallest score (best match) is extracted first.
 *
 * Time Complexity:
 *   Insert:       O(log n)
 *   Extract-min:  O(log n)
 *   Peek:         O(1)
 *
 * Space Complexity: O(n)
 */

#ifndef TYPEASSIST_MINHEAP_H
#define TYPEASSIST_MINHEAP_H

#include <stdbool.h>

#define HEAP_MAX_WORD_LEN 64
#define HEAP_INITIAL_CAPACITY 64

/**
 * A heap element: a word with a score (edit distance).
 * Lower score = better match.
 */
typedef struct {
    char word[HEAP_MAX_WORD_LEN];
    int score;       /* Edit distance (lower = better) */
    int frequency;   /* Word frequency (for tie-breaking) */
} HeapElement;

/**
 * Min-heap container using a dynamic array.
 */
typedef struct {
    HeapElement *data;
    int size;
    int capacity;
} MinHeap;

/* Lifecycle */
MinHeap *heap_create(void);
MinHeap *heap_create_with_capacity(int capacity);
void heap_destroy(MinHeap *heap);

/* Core operations */
bool heap_insert(MinHeap *heap, const char *word, int score, int frequency);
bool heap_extract_min(MinHeap *heap, HeapElement *out);
bool heap_peek(const MinHeap *heap, HeapElement *out);

/* Utility */
int  heap_size(const MinHeap *heap);
bool heap_is_empty(const MinHeap *heap);
void heap_clear(MinHeap *heap);

#endif /* TYPEASSIST_MINHEAP_H */
