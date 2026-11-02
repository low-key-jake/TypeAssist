/**
 * TypeAssist — Engine API Implementation
 *
 * Ties together all data structures (Trie, HashTable, NGram, Stack,
 * HistoryList, MinHeap, EditDistance) into a unified WebAssembly API.
 * All public functions return JSON strings or integer status codes.
 */

#include "engine_api.h"
#include "trie.h"
#include "hashtable.h"
#include "ngram.h"
#include "stack.h"
#include "linkedlist.h"
#include "minheap.h"
#include "edit_distance.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* ── Global engine state ───────────────────────────────────────────────── */

static Trie      *g_trie        = NULL;
static HashTable *g_dict_ht     = NULL;
static NGramModel *g_ngram      = NULL;
static Stack     *g_undo_stack  = NULL;
static HistoryList *g_history   = NULL;
static Trie      *g_personal_trie = NULL;  /* Personal dictionary trie */
static HashTable *g_personal_ht   = NULL;  /* Personal dictionary hash table */

/* Monotonic timestamp counter for undo entries */
static int g_undo_timestamp = 0;

/* Monotonic timestamp counter for history entries */
static int g_history_timestamp = 0;

/* ── Lifecycle ─────────────────────────────────────────────────────────── */

EXPORT int engine_init(void)
{
    /* Create all core data structures */
    g_trie = trie_create();
    if (!g_trie) goto fail;

    g_dict_ht = ht_create();
    if (!g_dict_ht) goto fail;

    g_ngram = ngram_create();
    if (!g_ngram) goto fail;

    g_undo_stack = stack_create();
    if (!g_undo_stack) goto fail;

    g_history = history_create(1000); /* Max 1000 history entries */
    if (!g_history) goto fail;

    g_personal_trie = trie_create();
    if (!g_personal_trie) goto fail;

    g_personal_ht = ht_create();
    if (!g_personal_ht) goto fail;

    g_undo_timestamp = 0;
    g_history_timestamp = 0;
    return 0;

fail:
    engine_cleanup();
    return -1;
}

EXPORT void engine_cleanup(void)
{
    if (g_trie)          { trie_destroy(g_trie);           g_trie = NULL; }
    if (g_dict_ht)       { ht_destroy(g_dict_ht);          g_dict_ht = NULL; }
    if (g_ngram)         { ngram_destroy(g_ngram);          g_ngram = NULL; }
    if (g_undo_stack)    { stack_destroy(g_undo_stack);     g_undo_stack = NULL; }
    if (g_history)       { history_destroy(g_history);      g_history = NULL; }
    if (g_personal_trie) { trie_destroy(g_personal_trie);  g_personal_trie = NULL; }
    if (g_personal_ht)   { ht_destroy(g_personal_ht);      g_personal_ht = NULL; }
}

EXPORT void engine_free_string(char *ptr)
{
    free(ptr);
}

/* ── Data Loading ──────────────────────────────────────────────────────── */

EXPORT int engine_load_dictionary(const char *data, int data_len)
{
    if (!g_trie || !g_dict_ht || !data || data_len <= 0) return -1;

    /* Load into both data structures from the same buffer */
    int trie_count = trie_load_from_buffer(g_trie, data, data_len);
    if (trie_count < 0) return -1;

    int ht_count = ht_load_from_buffer(g_dict_ht, data, data_len);
    if (ht_count < 0) return -1;

    /* Return the trie count (should match ht_count) */
    return trie_count;
}

EXPORT int engine_load_ngrams(const char *data, int data_len, int type)
{
    if (!g_ngram || !data || data_len <= 0) return -1;

    switch (type) {
        case 1: return ngram_load_unigrams(g_ngram, data, data_len);
        case 2: return ngram_load_bigrams(g_ngram, data, data_len);
        case 3: return ngram_load_trigrams(g_ngram, data, data_len);
        default: return -1;
    }
}

/* ── Core Features ─────────────────────────────────────────────────────── */

EXPORT const char *engine_autocomplete(const char *prefix, int max_results)
{
    if (!prefix || max_results <= 0) {
        char *empty = (char *)malloc(3);
        if (empty) { strcpy(empty, "[]"); }
        return empty;
    }

    /* Cap max_results to a reasonable limit */
    if (max_results > TRIE_MAX_SUGGESTIONS) max_results = TRIE_MAX_SUGGESTIONS;

    /* Allocate suggestion arrays for main + personal tries */
    TrieSuggestion *main_results = (TrieSuggestion *)calloc(
        max_results, sizeof(TrieSuggestion));
    TrieSuggestion *personal_results = (TrieSuggestion *)calloc(
        max_results, sizeof(TrieSuggestion));
    if (!main_results || !personal_results) {
        free(main_results);
        free(personal_results);
        return NULL;
    }

    int main_count = 0, personal_count = 0;

    if (g_trie) {
        main_count = trie_autocomplete(g_trie, prefix, main_results, max_results);
    }
    if (g_personal_trie) {
        personal_count = trie_autocomplete(g_personal_trie, prefix,
                                           personal_results, max_results);
    }

    /* Merge results. Personal results go first (higher priority),
     * then main dictionary results, up to max_results total. */
    int total_count = personal_count + main_count;
    if (total_count > max_results) total_count = max_results;

    /* Build JSON: [{"word":"...","frequency":N}, ...] */
    /* Estimate size: ~80 bytes per entry + brackets */
    int buf_size = total_count * 80 + 16;
    char *json = (char *)malloc(buf_size);
    if (!json) {
        free(main_results);
        free(personal_results);
        return NULL;
    }

    int offset = 0;
    json[offset++] = '[';

    int written = 0;

    /* Add personal results first */
    for (int i = 0; i < personal_count && written < total_count; i++, written++) {
        if (written > 0) { json[offset++] = ','; }
        offset += snprintf(json + offset, buf_size - offset,
                           "{\"word\":\"%s\",\"frequency\":%d}",
                           personal_results[i].word,
                           personal_results[i].frequency);
    }

    /* Add main dictionary results */
    for (int i = 0; i < main_count && written < total_count; i++) {
        /* Skip duplicates (words already in personal results) */
        bool duplicate = false;
        for (int j = 0; j < personal_count; j++) {
            if (strcmp(main_results[i].word, personal_results[j].word) == 0) {
                duplicate = true;
                break;
            }
        }
        if (duplicate) continue;

        if (written > 0) { json[offset++] = ','; }
        offset += snprintf(json + offset, buf_size - offset,
                           "{\"word\":\"%s\",\"frequency\":%d}",
                           main_results[i].word,
                           main_results[i].frequency);
        written++;
    }

    json[offset++] = ']';
    json[offset] = '\0';

    free(main_results);
    free(personal_results);
    return json;
}

/**
 * Generate candidate corrections for a misspelled word using edit operations.
 * Checks each generated variant against the dictionary hash table.
 * Inserts valid candidates into the min-heap with edit distance and frequency.
 */
static void generate_spell_candidates(const char *word, MinHeap *heap,
                                      int max_distance)
{
    int len = (int)strlen(word);
    if (len <= 0 || len >= TRIE_MAX_WORD_LEN) return;

    char candidate[TRIE_MAX_WORD_LEN];

    /* For distances 1 and 2, generate all single-edit variants.
     * For distance 2, we generate edits-of-edits (two-level). */
    
    /* Helper: check a candidate and insert if found in dictionary */
    #define CHECK_CANDIDATE(cand, dist) do {                            \
        if (ht_contains(g_dict_ht, (cand))) {                           \
            int freq = ht_get(g_dict_ht, (cand), 0);                   \
            heap_insert(heap, (cand), (dist), freq);                    \
        }                                                               \
        if (g_personal_ht && ht_contains(g_personal_ht, (cand))) {      \
            int freq = ht_get(g_personal_ht, (cand), 0);               \
            heap_insert(heap, (cand), (dist), freq);                    \
        }                                                               \
    } while(0)

    /* === Distance 1 edits === */

    /* 1. Deletions: remove one character */
    for (int i = 0; i < len; i++) {
        memcpy(candidate, word, i);
        memcpy(candidate + i, word + i + 1, len - i - 1);
        candidate[len - 1] = '\0';
        CHECK_CANDIDATE(candidate, 1);
    }

    /* 2. Transpositions: swap adjacent characters */
    for (int i = 0; i < len - 1; i++) {
        memcpy(candidate, word, len + 1);
        char tmp = candidate[i];
        candidate[i] = candidate[i + 1];
        candidate[i + 1] = tmp;
        CHECK_CANDIDATE(candidate, 1);
    }

    /* 3. Replacements: replace each character with a-z */
    for (int i = 0; i < len; i++) {
        memcpy(candidate, word, len + 1);
        for (char c = 'a'; c <= 'z'; c++) {
            if (c == word[i]) continue;
            candidate[i] = c;
            CHECK_CANDIDATE(candidate, 1);
        }
    }

    /* 4. Insertions: insert each character a-z at each position */
    if (len + 1 < TRIE_MAX_WORD_LEN) {
        for (int i = 0; i <= len; i++) {
            memcpy(candidate, word, i);
            memcpy(candidate + i + 1, word + i, len - i);
            candidate[len + 1] = '\0';
            for (char c = 'a'; c <= 'z'; c++) {
                candidate[i] = c;
                CHECK_CANDIDATE(candidate, 1);
            }
        }
    }

    /* === Distance 2 edits (edits of the distance-1 deletions) === */
    if (max_distance >= 2) {
        /* For efficiency, apply distance-1 edits to each distance-1 deletion.
         * This covers a large portion of distance-2 candidates. */
        char edit1[TRIE_MAX_WORD_LEN];
        for (int i = 0; i < len; i++) {
            /* Build distance-1 deletion */
            memcpy(edit1, word, i);
            memcpy(edit1 + i, word + i + 1, len - i - 1);
            edit1[len - 1] = '\0';
            int e1len = len - 1;
            if (e1len <= 0) continue;

            /* Further deletions */
            for (int j = 0; j < e1len; j++) {
                memcpy(candidate, edit1, j);
                memcpy(candidate + j, edit1 + j + 1, e1len - j - 1);
                candidate[e1len - 1] = '\0';
                CHECK_CANDIDATE(candidate, 2);
            }

            /* Further transpositions */
            for (int j = 0; j < e1len - 1; j++) {
                memcpy(candidate, edit1, e1len + 1);
                char tmp = candidate[j];
                candidate[j] = candidate[j + 1];
                candidate[j + 1] = tmp;
                CHECK_CANDIDATE(candidate, 2);
            }

            /* Further replacements */
            for (int j = 0; j < e1len; j++) {
                memcpy(candidate, edit1, e1len + 1);
                for (char c = 'a'; c <= 'z'; c++) {
                    if (c == edit1[j]) continue;
                    candidate[j] = c;
                    CHECK_CANDIDATE(candidate, 2);
                }
            }

            /* Further insertions */
            if (e1len + 1 < TRIE_MAX_WORD_LEN) {
                for (int j = 0; j <= e1len; j++) {
                    memcpy(candidate, edit1, j);
                    memcpy(candidate + j + 1, edit1 + j, e1len - j);
                    candidate[e1len + 1] = '\0';
                    for (char c = 'a'; c <= 'z'; c++) {
                        candidate[j] = c;
                        CHECK_CANDIDATE(candidate, 2);
                    }
                }
            }
        }
    }

    #undef CHECK_CANDIDATE
}

EXPORT const char *engine_spellcheck(const char *word, int max_results)
{
    if (!word) return NULL;
    if (max_results <= 0) max_results = 5;

    /* Check if word exists in main or personal dictionary */
    bool found = false;
    if (g_dict_ht && ht_contains(g_dict_ht, word)) found = true;
    if (!found && g_personal_ht && ht_contains(g_personal_ht, word)) found = true;

    if (found) {
        /* Word is correct */
        const char *correct_json = "{\"correct\":true,\"suggestions\":[]}";
        int len = (int)strlen(correct_json);
        char *result = (char *)malloc(len + 1);
        if (!result) return NULL;
        memcpy(result, correct_json, len + 1);
        return result;
    }

    /* Word not found — generate correction candidates */
    MinHeap *heap = heap_create();
    if (!heap) return NULL;

    generate_spell_candidates(word, heap, 2);

    /* Extract top candidates from min-heap (sorted by edit distance asc,
     * tie-broken by frequency desc via the heap's natural ordering) */
    int buf_size = max_results * 120 + 64;
    char *json = (char *)malloc(buf_size);
    if (!json) {
        heap_destroy(heap);
        return NULL;
    }

    int offset = 0;
    offset += snprintf(json + offset, buf_size - offset,
                       "{\"correct\":false,\"suggestions\":[");

    int count = 0;
    HeapElement elem;

    /* Track already-added words to avoid duplicates */
    char seen[20][TRIE_MAX_WORD_LEN]; /* Max 20 unique suggestions tracked */
    int seen_count = 0;

    while (count < max_results && heap_extract_min(heap, &elem)) {
        /* Check for duplicates */
        bool dup = false;
        for (int i = 0; i < seen_count; i++) {
            if (strcmp(seen[i], elem.word) == 0) {
                dup = true;
                break;
            }
        }
        if (dup) continue;

        /* Record as seen */
        if (seen_count < 20) {
            strncpy(seen[seen_count], elem.word, TRIE_MAX_WORD_LEN - 1);
            seen[seen_count][TRIE_MAX_WORD_LEN - 1] = '\0';
            seen_count++;
        }

        if (count > 0) { json[offset++] = ','; }
        offset += snprintf(json + offset, buf_size - offset,
                           "{\"word\":\"%s\",\"distance\":%d,\"frequency\":%d}",
                           elem.word, elem.score, elem.frequency);
        count++;
    }

    offset += snprintf(json + offset, buf_size - offset, "]}");

    heap_destroy(heap);
    return json;
}

EXPORT const char *engine_predict_next(const char *context, int max_results)
{
    if (!g_ngram || !context || max_results <= 0) {
        char *empty = (char *)malloc(3);
        if (empty) strcpy(empty, "[]");
        return empty;
    }

    if (max_results > NGRAM_MAX_PREDICTIONS)
        max_results = NGRAM_MAX_PREDICTIONS;

    NGramPrediction *predictions = (NGramPrediction *)calloc(
        max_results, sizeof(NGramPrediction));
    if (!predictions) return NULL;

    int count = ngram_predict(g_ngram, context, predictions, max_results);

    /* Build JSON: [{"word":"...","score":0.N}, ...] */
    int buf_size = count * 100 + 16;
    char *json = (char *)malloc(buf_size);
    if (!json) {
        free(predictions);
        return NULL;
    }

    int offset = 0;
    json[offset++] = '[';

    for (int i = 0; i < count; i++) {
        if (i > 0) json[offset++] = ',';
        offset += snprintf(json + offset, buf_size - offset,
                           "{\"word\":\"%s\",\"score\":%.6f}",
                           predictions[i].word, predictions[i].score);
    }

    json[offset++] = ']';
    json[offset] = '\0';

    free(predictions);
    return json;
}

EXPORT int engine_word_exists(const char *word)
{
    if (!word) return 0;
    if (g_dict_ht && ht_contains(g_dict_ht, word)) return 1;
    if (g_personal_ht && ht_contains(g_personal_ht, word)) return 1;
    return 0;
}

/* ── Personal Dictionary ───────────────────────────────────────────────── */

EXPORT int engine_add_word(const char *word)
{
    if (!word || !g_personal_trie || !g_personal_ht) return -1;

    trie_insert(g_personal_trie, word, 100);
    ht_insert(g_personal_ht, word, 100);
    return 0;
}

EXPORT int engine_remove_word(const char *word)
{
    if (!word || !g_personal_ht) return -1;

    if (!ht_contains(g_personal_ht, word)) return -1;
    ht_remove(g_personal_ht, word);
    /* Note: We don't remove from the trie since trie deletion is complex.
     * The word will remain in the trie but won't be found in dictionary
     * lookups which use the hash table. */
    return 0;
}

/* ── Undo / History ────────────────────────────────────────────────────── */

EXPORT int engine_push_undo(const char *text)
{
    if (!text || !g_undo_stack) return -1;

    g_undo_timestamp++;
    if (!stack_push(g_undo_stack, text, g_undo_timestamp)) return -1;
    return 0;
}

EXPORT const char *engine_undo(void)
{
    if (!g_undo_stack || stack_is_empty(g_undo_stack)) return NULL;

    StackEntry entry;
    if (!stack_pop(g_undo_stack, &entry)) return NULL;

    /* Build JSON: {"text":"...","timestamp":N} */
    int text_len = entry.text_len;
    /* Worst case: every char needs escaping (\" → 2 chars) + JSON overhead */
    int buf_size = text_len * 2 + 64;
    char *json = (char *)malloc(buf_size);
    if (!json) {
        free(entry.text);
        return NULL;
    }

    /* Escape the text for JSON (handle quotes and backslashes) */
    char *escaped = (char *)malloc(text_len * 2 + 1);
    if (!escaped) {
        free(json);
        free(entry.text);
        return NULL;
    }

    int ei = 0;
    for (int i = 0; i < text_len; i++) {
        if (entry.text[i] == '"' || entry.text[i] == '\\') {
            escaped[ei++] = '\\';
        }
        escaped[ei++] = entry.text[i];
    }
    escaped[ei] = '\0';

    snprintf(json, buf_size, "{\"text\":\"%s\",\"timestamp\":%d}",
             escaped, entry.timestamp);

    free(escaped);
    free(entry.text);
    return json;
}

EXPORT int engine_add_history(const char *original, const char *modified, int type)
{
    if (!original || !modified || !g_history) return -1;
    if (type < 0 || type > 5) return -1;

    g_history_timestamp++;
    if (!history_add(g_history, original, modified,
                     (HistoryType)type, g_history_timestamp))
        return -1;

    return 0;
}

EXPORT const char *engine_get_history(void)
{
    if (!g_history) return NULL;

    /* Allocate a buffer for JSON output */
    int buf_size = g_history->count * 512 + 64;
    if (buf_size < 256) buf_size = 256;

    char *json = (char *)malloc(buf_size);
    if (!json) return NULL;

    int written = history_to_json(g_history, json, buf_size);
    if (written < 0) {
        /* Buffer too small — retry with larger buffer */
        buf_size *= 2;
        char *bigger = (char *)realloc(json, buf_size);
        if (!bigger) {
            free(json);
            return NULL;
        }
        json = bigger;
        written = history_to_json(g_history, json, buf_size);
        if (written < 0) {
            free(json);
            return NULL;
        }
    }

    return json;
}

EXPORT void engine_clear_history(void)
{
    if (g_history) {
        history_clear(g_history);
    }
}

/* ── Statistics ────────────────────────────────────────────────────────── */

EXPORT const char *engine_get_stats(void)
{
    int dict_words = g_dict_ht ? ht_size(g_dict_ht) : 0;
    int trie_nodes = g_trie ? trie_node_count(g_trie) : 0;
    int trie_words = g_trie ? trie_word_count(g_trie) : 0;
    int personal_words = g_personal_ht ? ht_size(g_personal_ht) : 0;

    int unigram_count = 0, bigram_count = 0, trigram_count = 0;
    int total_unigrams = 0, vocab_size = 0;
    if (g_ngram) {
        unigram_count = ht_size(g_ngram->unigrams);
        bigram_count  = ht_size(g_ngram->bigrams);
        trigram_count = ht_size(g_ngram->trigrams);
        total_unigrams = g_ngram->total_unigrams;
        vocab_size = g_ngram->vocabulary_size;
    }

    int undo_depth = g_undo_stack ? stack_size(g_undo_stack) : 0;
    int history_entries = g_history ? history_count(g_history) : 0;

    int buf_size = 512;
    char *json = (char *)malloc(buf_size);
    if (!json) return NULL;

    snprintf(json, buf_size,
        "{\"dict_words\":%d,"
        "\"trie_nodes\":%d,"
        "\"trie_words\":%d,"
        "\"personal_words\":%d,"
        "\"unigram_entries\":%d,"
        "\"bigram_entries\":%d,"
        "\"trigram_entries\":%d,"
        "\"total_unigrams\":%d,"
        "\"vocabulary_size\":%d,"
        "\"undo_depth\":%d,"
        "\"history_entries\":%d}",
        dict_words, trie_nodes, trie_words, personal_words,
        unigram_count, bigram_count, trigram_count,
        total_unigrams, vocab_size,
        undo_depth, history_entries);

    return json;
}
