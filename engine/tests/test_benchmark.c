/**
 * TypeAssist — Engine Benchmarks
 *
 * Measures performance of Trie, Hash Table, Edit Distance, and N-Gram operations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/trie.h"
#include "../include/hashtable.h"
#include "../include/edit_distance.h"
#include "../include/minheap.h"
#include "../include/ngram.h"

#define NUM_BENCH_WORDS 10000

static double get_time_sec(clock_t start, clock_t end)
{
    return (double)(end - start) / CLOCKS_PER_SEC;
}

static void generate_synthetic_word(int id, char *buf, int max_len)
{
    // Generate pseudo-random deterministic words
    int len = 4 + (id % 8);
    for (int i = 0; i < len && i < max_len - 1; i++) {
        buf[i] = 'a' + ((id * 31 + i * 17) % 26);
    }
    buf[len] = '\0';
}

int main(void)
{
    printf("==================================================\n");
    printf("       TYPEASSIST C ENGINE BENCHMARK SUITE       \n");
    printf("==================================================\n\n");

    // 1. Benchmark Hash Table Insertion & Lookup
    printf("[1] Benchmarking Hash Table with %d words...\n", NUM_BENCH_WORDS);
    HashTable *ht = ht_create_with_capacity(16384);
    char word[64];

    clock_t start = clock();
    for (int i = 0; i < NUM_BENCH_WORDS; i++) {
        generate_synthetic_word(i, word, sizeof(word));
        ht_insert(ht, word, i + 1);
    }
    clock_t end = clock();
    double ht_insert_time = get_time_sec(start, end);
    printf("    Insert %d items: %.4f sec (%.0f ops/sec)\n",
           NUM_BENCH_WORDS, ht_insert_time, NUM_BENCH_WORDS / ht_insert_time);

    start = clock();
    int hits = 0;
    for (int i = 0; i < NUM_BENCH_WORDS; i++) {
        generate_synthetic_word(i, word, sizeof(word));
        if (ht_contains(ht, word)) hits++;
    }
    end = clock();
    double ht_lookup_time = get_time_sec(start, end);
    printf("    Lookup %d items: %.4f sec (%.0f ops/sec, hits: %d)\n\n",
           NUM_BENCH_WORDS, ht_lookup_time, NUM_BENCH_WORDS / ht_lookup_time, hits);
    ht_destroy(ht);

    // 2. Benchmark Trie Insertion & Autocomplete
    printf("[2] Benchmarking Trie with %d words...\n", NUM_BENCH_WORDS);
    Trie *trie = trie_create();

    start = clock();
    for (int i = 0; i < NUM_BENCH_WORDS; i++) {
        generate_synthetic_word(i, word, sizeof(word));
        trie_insert(trie, word, i + 1);
    }
    end = clock();
    double trie_insert_time = get_time_sec(start, end);
    printf("    Insert %d items: %.4f sec (%.0f ops/sec)\n",
           NUM_BENCH_WORDS, trie_insert_time, NUM_BENCH_WORDS / trie_insert_time);

    TrieSuggestion suggestions[10];
    start = clock();
    int query_count = 5000;
    for (int i = 0; i < query_count; i++) {
        char prefix[4];
        prefix[0] = 'a' + (i % 26);
        prefix[1] = 'a' + ((i * 3) % 26);
        prefix[2] = '\0';
        trie_autocomplete(trie, prefix, suggestions, 5);
    }
    end = clock();
    double trie_auto_time = get_time_sec(start, end);
    printf("    Autocomplete %d queries: %.4f sec (%.0f queries/sec)\n\n",
           query_count, trie_auto_time, query_count / trie_auto_time);
    trie_destroy(trie);

    // 3. Benchmark Damerau-Levenshtein Edit Distance
    printf("[3] Benchmarking Damerau-Levenshtein Edit Distance...\n");
    int ed_pairs = 20000;
    start = clock();
    int total_dist = 0;
    for (int i = 0; i < ed_pairs; i++) {
        char w1[32], w2[32];
        generate_synthetic_word(i, w1, sizeof(w1));
        generate_synthetic_word(i + 1, w2, sizeof(w2));
        total_dist += damerau_levenshtein(w1, w2);
    }
    end = clock();
    double ed_time = get_time_sec(start, end);
    printf("    Calculated %d distances: %.4f sec (%.0f pairs/sec, avg dist: %.1f)\n\n",
           ed_pairs, ed_time, ed_pairs / ed_time, (double)total_dist / ed_pairs);

    // 4. Benchmark Min-Heap
    printf("[4] Benchmarking Min-Heap (10,000 pushes + pops)...\n");
    MinHeap *heap = heap_create_with_capacity(10000);
    start = clock();
    for (int i = 0; i < 10000; i++) {
        generate_synthetic_word(i, word, sizeof(word));
        heap_insert(heap, word, (i * 37) % 50, i);
    }
    HeapElement elem;
    while (!heap_is_empty(heap)) {
        heap_extract_min(heap, &elem);
    }
    end = clock();
    double heap_time = get_time_sec(start, end);
    printf("    10K inserts + extracts: %.4f sec (%.0f ops/sec)\n\n",
           heap_time, 20000.0 / heap_time);
    heap_destroy(heap);

    printf("==================================================\n");
    printf("              BENCHMARK COMPLETED                 \n");
    printf("==================================================\n");
    return 0;
}

