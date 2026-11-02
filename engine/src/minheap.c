/**
 * TypeAssist — Min-Heap Implementation
 *
 * Binary min-heap using a dynamic array. Elements are sorted by score
 * (ascending), with ties broken by frequency (descending — higher frequency wins).
 */

#include "minheap.h"
#include <stdlib.h>
#include <string.h>

/* ---------- Static helpers ---------- */

/**
 * Compare two heap elements.
 * Returns true if 'a' should be closer to the root than 'b'.
 * Primary: lower score wins.  Tie-break: higher frequency wins.
 */
static bool has_higher_priority(const HeapElement *a, const HeapElement *b)
{
    if (a->score != b->score)
        return a->score < b->score;
    return a->frequency > b->frequency;
}

/**
 * Swap two heap elements in-place.
 */
static void swap(HeapElement *a, HeapElement *b)
{
    HeapElement tmp = *a;
    *a = *b;
    *b = tmp;
}

/**
 * Restore heap property upward from index i.
 */
static void sift_up(MinHeap *heap, int i)
{
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (has_higher_priority(&heap->data[i], &heap->data[parent])) {
            swap(&heap->data[i], &heap->data[parent]);
            i = parent;
        } else {
            break;
        }
    }
}

/**
 * Restore heap property downward from index i.
 */
static void sift_down(MinHeap *heap, int i)
{
    while (1) {
        int smallest = i;
        int left  = 2 * i + 1;
        int right = 2 * i + 2;

        if (left < heap->size &&
            has_higher_priority(&heap->data[left], &heap->data[smallest]))
            smallest = left;

        if (right < heap->size &&
            has_higher_priority(&heap->data[right], &heap->data[smallest]))
            smallest = right;

        if (smallest != i) {
            swap(&heap->data[i], &heap->data[smallest]);
            i = smallest;
        } else {
            break;
        }
    }
}

/* ---------- Lifecycle ---------- */

MinHeap *heap_create(void)
{
    return heap_create_with_capacity(HEAP_INITIAL_CAPACITY);
}

MinHeap *heap_create_with_capacity(int capacity)
{
    if (capacity <= 0)
        capacity = HEAP_INITIAL_CAPACITY;

    MinHeap *heap = malloc(sizeof(MinHeap));
    if (!heap)
        return NULL;

    heap->data = malloc(sizeof(HeapElement) * (size_t)capacity);
    if (!heap->data) {
        free(heap);
        return NULL;
    }

    heap->size     = 0;
    heap->capacity = capacity;
    return heap;
}

void heap_destroy(MinHeap *heap)
{
    if (!heap)
        return;
    free(heap->data);
    free(heap);
}

/* ---------- Core operations ---------- */

bool heap_insert(MinHeap *heap, const char *word, int score, int frequency)
{
    if (!heap || !word)
        return false;

    /* Grow the array if full (double capacity) */
    if (heap->size >= heap->capacity) {
        int new_cap = heap->capacity * 2;
        HeapElement *new_data = realloc(heap->data,
                                        sizeof(HeapElement) * (size_t)new_cap);
        if (!new_data)
            return false;
        heap->data     = new_data;
        heap->capacity = new_cap;
    }

    /* Copy word, always null-terminate */
    HeapElement *elem = &heap->data[heap->size];
    strncpy(elem->word, word, HEAP_MAX_WORD_LEN - 1);
    elem->word[HEAP_MAX_WORD_LEN - 1] = '\0';
    elem->score     = score;
    elem->frequency = frequency;

    heap->size++;
    sift_up(heap, heap->size - 1);
    return true;
}

bool heap_extract_min(MinHeap *heap, HeapElement *out)
{
    if (!heap || heap->size == 0)
        return false;

    if (out)
        *out = heap->data[0];

    /* Move last element to root and sift down */
    heap->size--;
    if (heap->size > 0) {
        heap->data[0] = heap->data[heap->size];
        sift_down(heap, 0);
    }

    return true;
}

bool heap_peek(const MinHeap *heap, HeapElement *out)
{
    if (!heap || heap->size == 0 || !out)
        return false;

    *out = heap->data[0];
    return true;
}

/* ---------- Utility ---------- */

int heap_size(const MinHeap *heap)
{
    if (!heap)
        return 0;
    return heap->size;
}

bool heap_is_empty(const MinHeap *heap)
{
    if (!heap)
        return true;
    return heap->size == 0;
}

void heap_clear(MinHeap *heap)
{
    if (!heap)
        return;
    heap->size = 0;
}
