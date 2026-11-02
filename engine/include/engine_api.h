/**
 * TypeAssist — Engine API (WebAssembly Interface)
 *
 * This header declares all functions exported to JavaScript via Emscripten.
 * These are the ONLY entry points the frontend uses.
 *
 * Return values are either status codes or pointers to JSON strings
 * allocated in WASM memory that the JS side reads via UTF8ToString().
 *
 * IMPORTANT: The caller must call engine_free_string() on any returned
 * string pointer to avoid memory leaks.
 */

#ifndef TYPEASSIST_ENGINE_API_H
#define TYPEASSIST_ENGINE_API_H

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define EXPORT
#endif

/* ── Lifecycle ────────────────────────────────────────────────────────── */

/**
 * Initialise the engine. Must be called before any other function.
 * @return 0 on success, -1 on failure
 */
EXPORT int engine_init(void);

/**
 * Free all engine resources.
 */
EXPORT void engine_cleanup(void);

/**
 * Free a string previously returned by the engine.
 * @param ptr  Pointer to the string to free
 */
EXPORT void engine_free_string(char *ptr);

/* ── Data Loading ─────────────────────────────────────────────────────── */

/**
 * Load dictionary words into the Trie and Hash Table.
 * Format: "word<TAB>frequency\n" per line.
 * @param data     Pointer to the data buffer
 * @param data_len Length of the data buffer
 * @return Number of words loaded, or -1 on error
 */
EXPORT int engine_load_dictionary(const char *data, int data_len);

/**
 * Load N-gram data (unigrams, bigrams, trigrams).
 * Format: "word1[|word2[|word3]]<TAB>frequency\n" per line.
 * @param data     Pointer to the data buffer
 * @param data_len Length of the data buffer
 * @param type     1 = unigram, 2 = bigram, 3 = trigram
 * @return Number of entries loaded, or -1 on error
 */
EXPORT int engine_load_ngrams(const char *data, int data_len, int type);

/* ── Core Features ────────────────────────────────────────────────────── */

/**
 * Get autocomplete suggestions for a prefix.
 * @param prefix       The prefix to complete
 * @param max_results  Maximum number of suggestions
 * @return JSON string: [{"word":"...", "frequency":N}, ...]
 *         Caller must free with engine_free_string().
 */
EXPORT const char *engine_autocomplete(const char *prefix, int max_results);

/**
 * Spellcheck a word and return correction candidates.
 * @param word         The word to check
 * @param max_results  Maximum number of candidates
 * @return JSON string: {"correct":true/false, "suggestions":[{"word":"...", "distance":N}, ...]}
 *         Caller must free with engine_free_string().
 */
EXPORT const char *engine_spellcheck(const char *word, int max_results);

/**
 * Predict the next word given preceding context.
 * Context is space-separated words (last 1-2 words used).
 * @param context      Space-separated preceding text
 * @param max_results  Maximum predictions
 * @return JSON string: [{"word":"...", "score":0.N}, ...]
 *         Caller must free with engine_free_string().
 */
EXPORT const char *engine_predict_next(const char *context, int max_results);

/**
 * Check if a word exists in the dictionary.
 * @param word  The word to look up
 * @return 1 if found, 0 if not found
 */
EXPORT int engine_word_exists(const char *word);

/* ── Personal Dictionary ──────────────────────────────────────────────── */

/**
 * Add a word to the personal dictionary.
 * @param word  The word to add
 * @return 0 on success, -1 on error
 */
EXPORT int engine_add_word(const char *word);

/**
 * Remove a word from the personal dictionary.
 * @param word  The word to remove
 * @return 0 on success, -1 if not found
 */
EXPORT int engine_remove_word(const char *word);

/* ── Undo / History ───────────────────────────────────────────────────── */

/**
 * Push a text state onto the undo stack.
 * @param text  The text state to save
 * @return 0 on success, -1 on error
 */
EXPORT int engine_push_undo(const char *text);

/**
 * Pop the last text state from the undo stack.
 * @return The previous text state (JSON), or NULL if stack is empty.
 *         Caller must free with engine_free_string().
 */
EXPORT const char *engine_undo(void);

/**
 * Add an entry to the correction history.
 * @param original  Original text
 * @param modified  Modified text
 * @param type      History type (0-5, see HistoryType enum)
 * @return 0 on success, -1 on error
 */
EXPORT int engine_add_history(const char *original, const char *modified, int type);

/**
 * Get the correction history as JSON.
 * @return JSON string of history entries.
 *         Caller must free with engine_free_string().
 */
EXPORT const char *engine_get_history(void);

/**
 * Clear all history entries.
 */
EXPORT void engine_clear_history(void);

/* ── Statistics ────────────────────────────────────────────────────────── */

/**
 * Get engine statistics as JSON.
 * @return JSON string: {"dict_words":N, "trie_nodes":N, "ngram_entries":N, ...}
 *         Caller must free with engine_free_string().
 */
EXPORT const char *engine_get_stats(void);

#endif /* TYPEASSIST_ENGINE_API_H */
