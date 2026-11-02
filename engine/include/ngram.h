/**
 * TypeAssist — N-Gram Language Model
 *
 * Implements unigram, bigram, and trigram frequency models for
 * next-word prediction. Uses hash tables internally for O(1) lookups.
 *
 * Prediction strategy:
 *   1. Try trigram (two preceding words) → highest confidence
 *   2. Fallback to bigram (one preceding word)
 *   3. Fallback to unigram (most frequent words overall)
 *
 * Time Complexity:
 *   Lookup:   O(1) via hash table
 *   Predict:  O(V) where V = vocabulary size (scan candidates)
 *
 * Space Complexity: O(V + B + T)
 *   V = unigram entries, B = bigram entries, T = trigram entries
 */

#ifndef TYPEASSIST_NGRAM_H
#define TYPEASSIST_NGRAM_H

#include "hashtable.h"

#define NGRAM_MAX_WORD_LEN 64
#define NGRAM_MAX_PREDICTIONS 10
#define NGRAM_KEY_SEPARATOR '|'

/**
 * A predicted word with its probability score.
 */
typedef struct {
    char word[NGRAM_MAX_WORD_LEN];
    double score;   /* Conditional probability or frequency ratio */
} NGramPrediction;

/**
 * N-gram language model container.
 */
typedef struct {
    HashTable *unigrams;    /* word → frequency */
    HashTable *bigrams;     /* "w1|w2" → frequency */
    HashTable *trigrams;    /* "w1|w2|w3" → frequency */
    int total_unigrams;     /* Sum of all unigram counts */
    int vocabulary_size;    /* Number of unique words */
} NGramModel;

/* Lifecycle */
NGramModel *ngram_create(void);
void ngram_destroy(NGramModel *model);

/* Training / loading */
void ngram_add_unigram(NGramModel *model, const char *word, int count);
void ngram_add_bigram(NGramModel *model, const char *w1, const char *w2, int count);
void ngram_add_trigram(NGramModel *model, const char *w1, const char *w2, const char *w3, int count);

/* Bulk loading from processed data files */
int ngram_load_unigrams(NGramModel *model, const char *data, int data_len);
int ngram_load_bigrams(NGramModel *model, const char *data, int data_len);
int ngram_load_trigrams(NGramModel *model, const char *data, int data_len);

/* Prediction */
int ngram_predict(NGramModel *model, const char *context,
                  NGramPrediction *results, int max_results);

/* Query */
int ngram_get_unigram_freq(const NGramModel *model, const char *word);
int ngram_get_bigram_freq(const NGramModel *model, const char *w1, const char *w2);
int ngram_get_trigram_freq(const NGramModel *model, const char *w1, const char *w2, const char *w3);

#endif /* TYPEASSIST_NGRAM_H */
