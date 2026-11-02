/**
 * TypeAssist — Trie Data Structure
 *
 * A prefix tree for dictionary storage and autocomplete.
 * Each node represents a character; paths from root to leaf form words.
 * Supports frequency-ranked autocomplete and prefix searching.
 *
 * Time Complexity:
 *   Insert:     O(L) where L = word length
 *   Search:     O(L)
 *   Prefix:     O(L + k) where k = number of matches
 *   Autocomplete: O(L + subtree traversal)
 *
 * Space Complexity: O(ALPHABET_SIZE * N) where N = total nodes
 */

#ifndef TYPEASSIST_TRIE_H
#define TYPEASSIST_TRIE_H

#include <stdbool.h>

#define TRIE_ALPHABET_SIZE 27  /* a-z + apostrophe */
#define TRIE_MAX_SUGGESTIONS 10
#define TRIE_MAX_WORD_LEN 64

/**
 * Autocomplete suggestion with word text and frequency.
 */
typedef struct {
    char word[TRIE_MAX_WORD_LEN];
    int frequency;
} TrieSuggestion;

/**
 * Single node in the Trie.
 */
typedef struct TrieNode {
    struct TrieNode *children[TRIE_ALPHABET_SIZE];
    bool is_end_of_word;
    int frequency;  /* Higher = more common word */
} TrieNode;

/**
 * Trie container with root node and statistics.
 */
typedef struct {
    TrieNode *root;
    int word_count;
    int node_count;
} Trie;

/* Lifecycle */
Trie *trie_create(void);
void trie_destroy(Trie *trie);

/* Core operations */
bool trie_insert(Trie *trie, const char *word, int frequency);
bool trie_search(Trie *trie, const char *word);
bool trie_starts_with(Trie *trie, const char *prefix);
int  trie_get_frequency(Trie *trie, const char *word);

/* Autocomplete */
int trie_autocomplete(Trie *trie, const char *prefix,
                      TrieSuggestion *results, int max_results);

/* Bulk loading */
int trie_load_from_buffer(Trie *trie, const char *data, int data_len);

/* Statistics */
int trie_word_count(const Trie *trie);
int trie_node_count(const Trie *trie);

#endif /* TYPEASSIST_TRIE_H */
