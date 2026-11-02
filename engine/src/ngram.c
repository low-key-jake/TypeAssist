/**
 * TypeAssist — N-Gram Language Model Implementation
 *
 * Implements unigram, bigram, and trigram frequency models for
 * next-word prediction using hash tables for O(1) lookups.
 */

#include "ngram.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ── Internal helpers ──────────────────────────────────────────────────── */

/**
 * Build a bigram key in the format "w1|w2".
 * Returns 0 on success, -1 if the key would exceed buf_size.
 */
static int build_bigram_key(const char *w1, const char *w2,
                            char *buf, int buf_size)
{
    int needed = (int)(strlen(w1) + 1 + strlen(w2) + 1); /* w1 + '|' + w2 + '\0' */
    if (needed > buf_size) return -1;
    snprintf(buf, buf_size, "%s%c%s", w1, NGRAM_KEY_SEPARATOR, w2);
    return 0;
}

/**
 * Build a trigram key in the format "w1|w2|w3".
 */
static int build_trigram_key(const char *w1, const char *w2, const char *w3,
                             char *buf, int buf_size)
{
    int needed = (int)(strlen(w1) + 1 + strlen(w2) + 1 + strlen(w3) + 1);
    if (needed > buf_size) return -1;
    snprintf(buf, buf_size, "%s%c%s%c%s",
             w1, NGRAM_KEY_SEPARATOR, w2, NGRAM_KEY_SEPARATOR, w3);
    return 0;
}

/* ── Lifecycle ─────────────────────────────────────────────────────────── */

NGramModel *ngram_create(void)
{
    NGramModel *model = (NGramModel *)malloc(sizeof(NGramModel));
    if (!model) return NULL;

    model->unigrams = ht_create();
    model->bigrams  = ht_create();
    model->trigrams  = ht_create();

    if (!model->unigrams || !model->bigrams || !model->trigrams) {
        /* Clean up any that were created */
        if (model->unigrams) ht_destroy(model->unigrams);
        if (model->bigrams)  ht_destroy(model->bigrams);
        if (model->trigrams) ht_destroy(model->trigrams);
        free(model);
        return NULL;
    }

    model->total_unigrams  = 0;
    model->vocabulary_size = 0;
    return model;
}

void ngram_destroy(NGramModel *model)
{
    if (!model) return;
    if (model->unigrams) ht_destroy(model->unigrams);
    if (model->bigrams)  ht_destroy(model->bigrams);
    if (model->trigrams) ht_destroy(model->trigrams);
    free(model);
}

/* ── Training / adding entries ─────────────────────────────────────────── */

void ngram_add_unigram(NGramModel *model, const char *word, int count)
{
    if (!model || !word) return;

    /* Track whether this is a new word vs. an update */
    bool existed = ht_contains(model->unigrams, word);

    if (existed) {
        /* Add to the existing count */
        int old = ht_get(model->unigrams, word, 0);
        ht_update(model->unigrams, word, old + count);
    } else {
        ht_insert(model->unigrams, word, count);
        model->vocabulary_size++;
    }
    model->total_unigrams += count;
}

void ngram_add_bigram(NGramModel *model, const char *w1, const char *w2, int count)
{
    if (!model || !w1 || !w2) return;

    char key[NGRAM_MAX_WORD_LEN * 2 + 2];
    if (build_bigram_key(w1, w2, key, sizeof(key)) != 0) return;

    bool existed = ht_contains(model->bigrams, key);
    if (existed) {
        int old = ht_get(model->bigrams, key, 0);
        ht_update(model->bigrams, key, old + count);
    } else {
        ht_insert(model->bigrams, key, count);
    }
}

void ngram_add_trigram(NGramModel *model, const char *w1, const char *w2,
                       const char *w3, int count)
{
    if (!model || !w1 || !w2 || !w3) return;

    char key[NGRAM_MAX_WORD_LEN * 3 + 3];
    if (build_trigram_key(w1, w2, w3, key, sizeof(key)) != 0) return;

    bool existed = ht_contains(model->trigrams, key);
    if (existed) {
        int old = ht_get(model->trigrams, key, 0);
        ht_update(model->trigrams, key, old + count);
    } else {
        ht_insert(model->trigrams, key, count);
    }
}

/* ── Bulk loading ──────────────────────────────────────────────────────── */

/**
 * Parse lines of "word\tfrequency\n". Call ngram_add_unigram for each.
 * Returns number of entries loaded, or -1 on error.
 */
int ngram_load_unigrams(NGramModel *model, const char *data, int data_len)
{
    if (!model || !data || data_len <= 0) return -1;

    int count = 0;
    const char *ptr = data;
    const char *end = data + data_len;

    while (ptr < end) {
        /* Find the tab separator */
        const char *tab = ptr;
        while (tab < end && *tab != '\t') tab++;
        if (tab >= end) break;

        int word_len = (int)(tab - ptr);
        if (word_len <= 0 || word_len >= NGRAM_MAX_WORD_LEN) {
            /* Skip malformed line — advance to next newline */
            while (ptr < end && *ptr != '\n') ptr++;
            if (ptr < end) ptr++; /* skip '\n' */
            continue;
        }

        /* Extract word */
        char word[NGRAM_MAX_WORD_LEN];
        memcpy(word, ptr, word_len);
        word[word_len] = '\0';

        /* Parse frequency */
        const char *freq_start = tab + 1;
        int freq = 0;
        const char *fp = freq_start;
        while (fp < end && *fp >= '0' && *fp <= '9') {
            freq = freq * 10 + (*fp - '0');
            fp++;
        }

        ngram_add_unigram(model, word, freq);
        count++;

        /* Advance past newline */
        while (ptr < end && *ptr != '\n') ptr++;
        if (ptr < end) ptr++;
    }

    return count;
}

/**
 * Parse lines of "w1|w2\tfrequency\n". Split on '|' to get w1, w2.
 * Returns number of entries loaded, or -1 on error.
 */
int ngram_load_bigrams(NGramModel *model, const char *data, int data_len)
{
    if (!model || !data || data_len <= 0) return -1;

    int count = 0;
    const char *ptr = data;
    const char *end = data + data_len;

    while (ptr < end) {
        /* Find tab */
        const char *tab = ptr;
        while (tab < end && *tab != '\t') tab++;
        if (tab >= end) break;

        /* The key portion is ptr..tab. Find the pipe within it. */
        const char *pipe = ptr;
        while (pipe < tab && *pipe != NGRAM_KEY_SEPARATOR) pipe++;
        if (pipe >= tab) {
            /* No pipe found — skip line */
            while (ptr < end && *ptr != '\n') ptr++;
            if (ptr < end) ptr++;
            continue;
        }

        int w1_len = (int)(pipe - ptr);
        int w2_len = (int)(tab - pipe - 1);

        if (w1_len <= 0 || w1_len >= NGRAM_MAX_WORD_LEN ||
            w2_len <= 0 || w2_len >= NGRAM_MAX_WORD_LEN) {
            while (ptr < end && *ptr != '\n') ptr++;
            if (ptr < end) ptr++;
            continue;
        }

        char w1[NGRAM_MAX_WORD_LEN], w2[NGRAM_MAX_WORD_LEN];
        memcpy(w1, ptr, w1_len);   w1[w1_len] = '\0';
        memcpy(w2, pipe + 1, w2_len); w2[w2_len] = '\0';

        /* Parse frequency */
        int freq = 0;
        const char *fp = tab + 1;
        while (fp < end && *fp >= '0' && *fp <= '9') {
            freq = freq * 10 + (*fp - '0');
            fp++;
        }

        ngram_add_bigram(model, w1, w2, freq);
        count++;

        while (ptr < end && *ptr != '\n') ptr++;
        if (ptr < end) ptr++;
    }

    return count;
}

/**
 * Parse lines of "w1|w2|w3\tfrequency\n". Split on '|'.
 * Returns number of entries loaded, or -1 on error.
 */
int ngram_load_trigrams(NGramModel *model, const char *data, int data_len)
{
    if (!model || !data || data_len <= 0) return -1;

    int count = 0;
    const char *ptr = data;
    const char *end = data + data_len;

    while (ptr < end) {
        /* Find tab */
        const char *tab = ptr;
        while (tab < end && *tab != '\t') tab++;
        if (tab >= end) break;

        /* Find first pipe */
        const char *pipe1 = ptr;
        while (pipe1 < tab && *pipe1 != NGRAM_KEY_SEPARATOR) pipe1++;
        if (pipe1 >= tab) {
            while (ptr < end && *ptr != '\n') ptr++;
            if (ptr < end) ptr++;
            continue;
        }

        /* Find second pipe */
        const char *pipe2 = pipe1 + 1;
        while (pipe2 < tab && *pipe2 != NGRAM_KEY_SEPARATOR) pipe2++;
        if (pipe2 >= tab) {
            while (ptr < end && *ptr != '\n') ptr++;
            if (ptr < end) ptr++;
            continue;
        }

        int w1_len = (int)(pipe1 - ptr);
        int w2_len = (int)(pipe2 - pipe1 - 1);
        int w3_len = (int)(tab - pipe2 - 1);

        if (w1_len <= 0 || w1_len >= NGRAM_MAX_WORD_LEN ||
            w2_len <= 0 || w2_len >= NGRAM_MAX_WORD_LEN ||
            w3_len <= 0 || w3_len >= NGRAM_MAX_WORD_LEN) {
            while (ptr < end && *ptr != '\n') ptr++;
            if (ptr < end) ptr++;
            continue;
        }

        char w1[NGRAM_MAX_WORD_LEN], w2[NGRAM_MAX_WORD_LEN], w3[NGRAM_MAX_WORD_LEN];
        memcpy(w1, ptr, w1_len);       w1[w1_len] = '\0';
        memcpy(w2, pipe1 + 1, w2_len); w2[w2_len] = '\0';
        memcpy(w3, pipe2 + 1, w3_len); w3[w3_len] = '\0';

        /* Parse frequency */
        int freq = 0;
        const char *fp = tab + 1;
        while (fp < end && *fp >= '0' && *fp <= '9') {
            freq = freq * 10 + (*fp - '0');
            fp++;
        }

        ngram_add_trigram(model, w1, w2, w3, freq);
        count++;

        while (ptr < end && *ptr != '\n') ptr++;
        if (ptr < end) ptr++;
    }

    return count;
}

/* ── Query ─────────────────────────────────────────────────────────────── */

int ngram_get_unigram_freq(const NGramModel *model, const char *word)
{
    if (!model || !word) return 0;
    return ht_get(model->unigrams, word, 0);
}

int ngram_get_bigram_freq(const NGramModel *model, const char *w1, const char *w2)
{
    if (!model || !w1 || !w2) return 0;
    char key[NGRAM_MAX_WORD_LEN * 2 + 2];
    if (build_bigram_key(w1, w2, key, sizeof(key)) != 0) return 0;
    return ht_get(model->bigrams, key, 0);
}

int ngram_get_trigram_freq(const NGramModel *model, const char *w1,
                           const char *w2, const char *w3)
{
    if (!model || !w1 || !w2 || !w3) return 0;
    char key[NGRAM_MAX_WORD_LEN * 3 + 3];
    if (build_trigram_key(w1, w2, w3, key, sizeof(key)) != 0) return 0;
    return ht_get(model->trigrams, key, 0);
}

/* ── Prediction helpers ────────────────────────────────────────────────── */

/**
 * Context data passed to ht_foreach callback for trigram prediction.
 */
typedef struct {
    const char *ctx_w1;         /* First context word */
    const char *ctx_w2;         /* Second context word */
    const NGramModel *model;
    NGramPrediction *candidates;
    int candidate_count;
    int max_candidates;
} TrigramPredictCtx;

/**
 * Callback for iterating trigrams. Matches entries where
 * the first two words equal the context, collects the third word.
 */
static void trigram_predict_cb(const char *key, int value, void *user_data)
{
    TrigramPredictCtx *ctx = (TrigramPredictCtx *)user_data;
    if (ctx->candidate_count >= ctx->max_candidates) return;

    /* Parse the trigram key: "w1|w2|w3" */
    char buf[NGRAM_MAX_WORD_LEN * 3 + 3];
    int key_len = (int)strlen(key);
    if (key_len >= (int)sizeof(buf)) return;
    memcpy(buf, key, key_len + 1);

    /* Split on pipes */
    char *p1 = strchr(buf, NGRAM_KEY_SEPARATOR);
    if (!p1) return;
    *p1 = '\0';
    char *w2_start = p1 + 1;

    char *p2 = strchr(w2_start, NGRAM_KEY_SEPARATOR);
    if (!p2) return;
    *p2 = '\0';
    char *w3_start = p2 + 1;

    /* Check if first two words match the context */
    if (strcmp(buf, ctx->ctx_w1) != 0 || strcmp(w2_start, ctx->ctx_w2) != 0)
        return;

    /* Compute score: trigram_freq / bigram_freq(w1,w2) */
    int bigram_freq = ngram_get_bigram_freq(ctx->model, ctx->ctx_w1, ctx->ctx_w2);
    double score = (bigram_freq > 0) ? (double)value / bigram_freq : (double)value;

    NGramPrediction *pred = &ctx->candidates[ctx->candidate_count];
    strncpy(pred->word, w3_start, NGRAM_MAX_WORD_LEN - 1);
    pred->word[NGRAM_MAX_WORD_LEN - 1] = '\0';
    pred->score = score;
    ctx->candidate_count++;
}

/**
 * Context data for bigram prediction callback.
 */
typedef struct {
    const char *ctx_w1;
    const NGramModel *model;
    NGramPrediction *candidates;
    int candidate_count;
    int max_candidates;
} BigramPredictCtx;

/**
 * Callback for iterating bigrams. Matches entries where
 * the first word equals the context.
 */
static void bigram_predict_cb(const char *key, int value, void *user_data)
{
    BigramPredictCtx *ctx = (BigramPredictCtx *)user_data;
    if (ctx->candidate_count >= ctx->max_candidates) return;

    /* Parse bigram key: "w1|w2" */
    char buf[NGRAM_MAX_WORD_LEN * 2 + 2];
    int key_len = (int)strlen(key);
    if (key_len >= (int)sizeof(buf)) return;
    memcpy(buf, key, key_len + 1);

    char *pipe = strchr(buf, NGRAM_KEY_SEPARATOR);
    if (!pipe) return;
    *pipe = '\0';
    char *w2_start = pipe + 1;

    if (strcmp(buf, ctx->ctx_w1) != 0) return;

    /* Score: bigram_freq / unigram_freq(w1) */
    int uni_freq = ngram_get_unigram_freq(ctx->model, ctx->ctx_w1);
    double score = (uni_freq > 0) ? (double)value / uni_freq : (double)value;

    NGramPrediction *pred = &ctx->candidates[ctx->candidate_count];
    strncpy(pred->word, w2_start, NGRAM_MAX_WORD_LEN - 1);
    pred->word[NGRAM_MAX_WORD_LEN - 1] = '\0';
    pred->score = score;
    ctx->candidate_count++;
}

/**
 * Context data for unigram fallback callback.
 */
typedef struct {
    NGramPrediction *candidates;
    int candidate_count;
    int max_candidates;
    int total_unigrams;
} UnigramPredictCtx;

/**
 * Callback for iterating unigrams. Collects all words with their frequency.
 */
static void unigram_predict_cb(const char *key, int value, void *user_data)
{
    UnigramPredictCtx *ctx = (UnigramPredictCtx *)user_data;
    if (ctx->candidate_count >= ctx->max_candidates) return;

    NGramPrediction *pred = &ctx->candidates[ctx->candidate_count];
    strncpy(pred->word, key, NGRAM_MAX_WORD_LEN - 1);
    pred->word[NGRAM_MAX_WORD_LEN - 1] = '\0';
    /* Score is relative frequency */
    pred->score = (ctx->total_unigrams > 0) ?
                  (double)value / ctx->total_unigrams : (double)value;
    ctx->candidate_count++;
}

/**
 * Comparator for sorting predictions by score (descending).
 */
static int prediction_cmp_desc(const void *a, const void *b)
{
    const NGramPrediction *pa = (const NGramPrediction *)a;
    const NGramPrediction *pb = (const NGramPrediction *)b;
    if (pb->score > pa->score) return 1;
    if (pb->score < pa->score) return -1;
    return 0;
}

/**
 * Extract the last N words from a context string.
 * words[0] = second-to-last word (if available), words[1] = last word.
 * Returns the number of words extracted (0, 1, or 2).
 */
static int extract_context_words(const char *context, char words[2][NGRAM_MAX_WORD_LEN])
{
    if (!context || *context == '\0') return 0;

    words[0][0] = '\0';
    words[1][0] = '\0';

    /* Walk backwards to find the last two words */
    int len = (int)strlen(context);
    const char *end = context + len;

    /* Skip trailing spaces */
    const char *p = end - 1;
    while (p >= context && (*p == ' ' || *p == '\t')) p--;
    if (p < context) return 0;

    /* Last word: [word_start .. p] */
    const char *w2_end = p + 1;
    while (p >= context && *p != ' ' && *p != '\t') p--;
    const char *w2_start = p + 1;

    int w2_len = (int)(w2_end - w2_start);
    if (w2_len <= 0) return 0;
    if (w2_len >= NGRAM_MAX_WORD_LEN) w2_len = NGRAM_MAX_WORD_LEN - 1;

    /* Skip spaces between words */
    while (p >= context && (*p == ' ' || *p == '\t')) p--;
    if (p < context) {
        /* Only one word */
        memcpy(words[0], w2_start, w2_len);
        words[0][w2_len] = '\0';
        return 1;
    }

    /* Second-to-last word */
    const char *w1_end = p + 1;
    while (p >= context && *p != ' ' && *p != '\t') p--;
    const char *w1_start = p + 1;

    int w1_len = (int)(w1_end - w1_start);
    if (w1_len <= 0) {
        memcpy(words[0], w2_start, w2_len);
        words[0][w2_len] = '\0';
        return 1;
    }
    if (w1_len >= NGRAM_MAX_WORD_LEN) w1_len = NGRAM_MAX_WORD_LEN - 1;

    memcpy(words[0], w1_start, w1_len);
    words[0][w1_len] = '\0';
    memcpy(words[1], w2_start, w2_len);
    words[1][w2_len] = '\0';
    return 2;
}

/* ── Prediction ────────────────────────────────────────────────────────── */

int ngram_predict(NGramModel *model, const char *context,
                  NGramPrediction *results, int max_results)
{
    if (!model || !results || max_results <= 0) return 0;

    /* Cap the internal candidate buffer size */
    int max_candidates = 1024;  /* Enough for most vocabularies */

    char words[2][NGRAM_MAX_WORD_LEN];
    int nwords = extract_context_words(context, words);

    NGramPrediction *candidates = (NGramPrediction *)malloc(
        sizeof(NGramPrediction) * max_candidates);
    if (!candidates) return 0;

    int candidate_count = 0;

    /* Strategy 1: Trigram prediction (if two context words available) */
    if (nwords == 2) {
        TrigramPredictCtx tri_ctx;
        tri_ctx.ctx_w1 = words[0];
        tri_ctx.ctx_w2 = words[1];
        tri_ctx.model = model;
        tri_ctx.candidates = candidates;
        tri_ctx.candidate_count = 0;
        tri_ctx.max_candidates = max_candidates;

        ht_foreach(model->trigrams, trigram_predict_cb, &tri_ctx);
        candidate_count = tri_ctx.candidate_count;
    }

    /* Strategy 2: Bigram fallback (if trigram yielded nothing, or only 1 context word) */
    if (candidate_count == 0 && nwords >= 1) {
        /* Use the last word as context */
        const char *last_word = (nwords == 2) ? words[1] : words[0];

        BigramPredictCtx bi_ctx;
        bi_ctx.ctx_w1 = last_word;
        bi_ctx.model = model;
        bi_ctx.candidates = candidates;
        bi_ctx.candidate_count = 0;
        bi_ctx.max_candidates = max_candidates;

        ht_foreach(model->bigrams, bigram_predict_cb, &bi_ctx);
        candidate_count = bi_ctx.candidate_count;
    }

    /* Strategy 3: Unigram fallback */
    if (candidate_count == 0) {
        UnigramPredictCtx uni_ctx;
        uni_ctx.candidates = candidates;
        uni_ctx.candidate_count = 0;
        uni_ctx.max_candidates = max_candidates;
        uni_ctx.total_unigrams = model->total_unigrams;

        ht_foreach(model->unigrams, unigram_predict_cb, &uni_ctx);
        candidate_count = uni_ctx.candidate_count;
    }

    /* Sort candidates by score descending */
    if (candidate_count > 0) {
        qsort(candidates, candidate_count, sizeof(NGramPrediction),
              prediction_cmp_desc);
    }

    /* Copy top max_results to the output */
    int result_count = (candidate_count < max_results) ?
                       candidate_count : max_results;
    memcpy(results, candidates, sizeof(NGramPrediction) * result_count);

    free(candidates);
    return result_count;
}
