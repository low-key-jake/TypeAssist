/**
 * TypeAssist — Trie Implementation
 *
 * Prefix tree supporting insertion, search, prefix matching,
 * frequency-ranked autocomplete, and bulk loading from a buffer.
 */

#include "trie.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/*  Internal helpers                                                    */
/* ------------------------------------------------------------------ */

/**
 * Map a character to a child index (0-25 for a-z, 26 for apostrophe).
 * Returns -1 for characters that are not part of the trie alphabet.
 */
static int char_to_index(char c)
{
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c == '\'')            return 26;
    return -1;
}

/**
 * Map an index back to a character.
 */
static char index_to_char(int idx)
{
    if (idx >= 0 && idx <= 25) return (char)('a' + idx);
    if (idx == 26)             return '\'';
    return '\0';
}

/**
 * Allocate and zero-initialise a new TrieNode.
 */
static TrieNode *trie_node_create(void)
{
    TrieNode *node = (TrieNode *)calloc(1, sizeof(TrieNode));
    return node;  /* NULL on allocation failure */
}

/**
 * Recursively free a subtree rooted at `node`.
 */
static void trie_node_destroy(TrieNode *node)
{
    if (!node) return;
    for (int i = 0; i < TRIE_ALPHABET_SIZE; i++) {
        trie_node_destroy(node->children[i]);
    }
    free(node);
}

/* ------------------------------------------------------------------ */
/*  DFS helper for autocomplete                                         */
/* ------------------------------------------------------------------ */

/**
 * State carried during the DFS collection phase.
 * We collect ALL matching words into a dynamic array, then sort later.
 */
typedef struct {
    TrieSuggestion *buf;   /* dynamically grown array of suggestions    */
    int count;             /* number of suggestions collected so far    */
    int capacity;          /* allocated capacity of buf                 */
} CollectState;

/**
 * DFS: traverse the subtree rooted at `node`, building the current word
 * in `prefix` (length `depth`).  Appends every end-of-word to `state`.
 */
static void trie_collect_words(TrieNode *node, char *prefix, int depth,
                               CollectState *state)
{
    if (!node) return;

    if (node->is_end_of_word) {
        /* Grow buffer if needed */
        if (state->count >= state->capacity) {
            int new_cap = state->capacity == 0 ? 64 : state->capacity * 2;
            TrieSuggestion *tmp = (TrieSuggestion *)realloc(
                state->buf, (size_t)new_cap * sizeof(TrieSuggestion));
            if (!tmp) return;  /* allocation failure — stop collecting */
            state->buf      = tmp;
            state->capacity = new_cap;
        }
        prefix[depth] = '\0';
        strncpy(state->buf[state->count].word, prefix, TRIE_MAX_WORD_LEN - 1);
        state->buf[state->count].word[TRIE_MAX_WORD_LEN - 1] = '\0';
        state->buf[state->count].frequency = node->frequency;
        state->count++;
    }

    /* Guard against exceeding our prefix buffer */
    if (depth >= TRIE_MAX_WORD_LEN - 1) return;

    for (int i = 0; i < TRIE_ALPHABET_SIZE; i++) {
        if (node->children[i]) {
            prefix[depth] = index_to_char(i);
            trie_collect_words(node->children[i], prefix, depth + 1, state);
        }
    }
}

/**
 * qsort comparator: sort by frequency descending, then lexicographically.
 */
static int suggestion_cmp(const void *a, const void *b)
{
    const TrieSuggestion *sa = (const TrieSuggestion *)a;
    const TrieSuggestion *sb = (const TrieSuggestion *)b;
    if (sb->frequency != sa->frequency)
        return sb->frequency - sa->frequency;          /* descending */
    return strcmp(sa->word, sb->word);                  /* ascending  */
}

/* ------------------------------------------------------------------ */
/*  Public API — Lifecycle                                              */
/* ------------------------------------------------------------------ */

Trie *trie_create(void)
{
    Trie *trie = (Trie *)calloc(1, sizeof(Trie));
    if (!trie) return NULL;

    trie->root = trie_node_create();
    if (!trie->root) {
        free(trie);
        return NULL;
    }
    trie->word_count = 0;
    trie->node_count = 1;  /* root counts as one node */
    return trie;
}

void trie_destroy(Trie *trie)
{
    if (!trie) return;
    trie_node_destroy(trie->root);
    free(trie);
}

/* ------------------------------------------------------------------ */
/*  Public API — Core operations                                        */
/* ------------------------------------------------------------------ */

bool trie_insert(Trie *trie, const char *word, int frequency)
{
    if (!trie || !word) return false;

    TrieNode *cur = trie->root;

    for (int i = 0; word[i] != '\0'; i++) {
        int idx = char_to_index(word[i]);
        if (idx < 0) continue;  /* skip characters outside the alphabet */

        if (!cur->children[idx]) {
            cur->children[idx] = trie_node_create();
            if (!cur->children[idx]) return false;
            trie->node_count++;
        }
        cur = cur->children[idx];
    }

    if (!cur->is_end_of_word) {
        cur->is_end_of_word = true;
        trie->word_count++;
    }
    /* Always update the frequency (allows caller to change it). */
    cur->frequency = frequency;
    return true;
}

bool trie_search(Trie *trie, const char *word)
{
    if (!trie || !word) return false;

    TrieNode *cur = trie->root;
    for (int i = 0; word[i] != '\0'; i++) {
        int idx = char_to_index(word[i]);
        if (idx < 0) continue;

        if (!cur->children[idx]) return false;
        cur = cur->children[idx];
    }
    return cur->is_end_of_word;
}

bool trie_starts_with(Trie *trie, const char *prefix)
{
    if (!trie || !prefix) return false;

    TrieNode *cur = trie->root;
    for (int i = 0; prefix[i] != '\0'; i++) {
        int idx = char_to_index(prefix[i]);
        if (idx < 0) continue;

        if (!cur->children[idx]) return false;
        cur = cur->children[idx];
    }
    return true;  /* prefix found — at least one path continues */
}

int trie_get_frequency(Trie *trie, const char *word)
{
    if (!trie || !word) return 0;

    TrieNode *cur = trie->root;
    for (int i = 0; word[i] != '\0'; i++) {
        int idx = char_to_index(word[i]);
        if (idx < 0) continue;

        if (!cur->children[idx]) return 0;
        cur = cur->children[idx];
    }
    return cur->is_end_of_word ? cur->frequency : 0;
}

/* ------------------------------------------------------------------ */
/*  Public API — Autocomplete                                           */
/* ------------------------------------------------------------------ */

int trie_autocomplete(Trie *trie, const char *prefix,
                      TrieSuggestion *results, int max_results)
{
    if (!trie || !prefix || !results || max_results <= 0) return 0;

    /* Walk to the node representing the end of the prefix. */
    TrieNode *cur = trie->root;
    char prefix_buf[TRIE_MAX_WORD_LEN];
    int  plen = 0;

    for (int i = 0; prefix[i] != '\0'; i++) {
        int idx = char_to_index(prefix[i]);
        if (idx < 0) continue;

        if (!cur->children[idx]) return 0;  /* prefix not in trie */
        if (plen < TRIE_MAX_WORD_LEN - 1) {
            prefix_buf[plen++] = (char)('a' + (idx < 26 ? idx : 0));
            if (idx == 26) prefix_buf[plen - 1] = '\'';
        }
        cur = cur->children[idx];
    }

    /* Collect all words under `cur` via DFS. */
    CollectState state = { NULL, 0, 0 };
    char word_buf[TRIE_MAX_WORD_LEN];
    memcpy(word_buf, prefix_buf, (size_t)plen);

    trie_collect_words(cur, word_buf, plen, &state);

    if (state.count == 0) {
        free(state.buf);
        return 0;
    }

    /* Sort by frequency descending. */
    qsort(state.buf, (size_t)state.count, sizeof(TrieSuggestion),
          suggestion_cmp);

    /* Copy the top results. */
    int n = state.count < max_results ? state.count : max_results;
    memcpy(results, state.buf, (size_t)n * sizeof(TrieSuggestion));

    free(state.buf);
    return n;
}

/* ------------------------------------------------------------------ */
/*  Public API — Bulk loading                                           */
/* ------------------------------------------------------------------ */

/**
 * Parse a buffer of "word\tfrequency\n" lines and insert each word.
 * If no tab is present on a line, the frequency defaults to 1.
 * Returns the number of words successfully inserted.
 */
int trie_load_from_buffer(Trie *trie, const char *data, int data_len)
{
    if (!trie || !data || data_len <= 0) return 0;

    int loaded = 0;
    int pos    = 0;

    while (pos < data_len) {
        /* Find end of current line. */
        int line_start = pos;
        while (pos < data_len && data[pos] != '\n') pos++;

        int line_len = pos - line_start;
        if (pos < data_len) pos++;  /* skip the newline */

        if (line_len == 0) continue;  /* blank line */

        /* Extract word and optional frequency. */
        char word[TRIE_MAX_WORD_LEN];
        int  freq = 1;
        int  tab_pos = -1;

        for (int i = 0; i < line_len; i++) {
            if (data[line_start + i] == '\t' || data[line_start + i] == ' ') {
                tab_pos = i;
                break;
            }
        }

        int word_len;
        if (tab_pos >= 0) {
            word_len = tab_pos < TRIE_MAX_WORD_LEN - 1
                         ? tab_pos
                         : TRIE_MAX_WORD_LEN - 1;
            memcpy(word, data + line_start, (size_t)word_len);
            word[word_len] = '\0';

            /* Parse frequency from the portion after the tab. */
            freq = atoi(data + line_start + tab_pos + 1);
            if (freq <= 0) freq = 1;
        } else {
            word_len = line_len < TRIE_MAX_WORD_LEN - 1
                         ? line_len
                         : TRIE_MAX_WORD_LEN - 1;
            memcpy(word, data + line_start, (size_t)word_len);
            word[word_len] = '\0';
        }

        if (word_len > 0 && trie_insert(trie, word, freq)) {
            loaded++;
        }
    }
    return loaded;
}

/* ------------------------------------------------------------------ */
/*  Public API — Statistics                                             */
/* ------------------------------------------------------------------ */

int trie_word_count(const Trie *trie)
{
    return trie ? trie->word_count : 0;
}

int trie_node_count(const Trie *trie)
{
    return trie ? trie->node_count : 0;
}
