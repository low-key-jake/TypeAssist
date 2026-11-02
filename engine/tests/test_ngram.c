/**
 * TypeAssist — N-Gram Model Tests
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/ngram.h"

static void test_create_destroy(void)
{
    NGramModel *model = ngram_create();
    assert(model != NULL);
    assert(model->total_unigrams == 0);
    assert(model->vocabulary_size == 0);
    ngram_destroy(model);
    printf("  PASS  test_create_destroy\n");
}

static void test_add_and_query(void)
{
    NGramModel *model = ngram_create();

    // Add unigrams
    ngram_add_unigram(model, "the", 500);
    ngram_add_unigram(model, "quick", 100);
    ngram_add_unigram(model, "brown", 80);
    ngram_add_unigram(model, "fox", 50);

    assert(ngram_get_unigram_freq(model, "the") == 500);
    assert(ngram_get_unigram_freq(model, "quick") == 100);
    assert(ngram_get_unigram_freq(model, "unknown") == 0);
    assert(model->vocabulary_size == 4);
    assert(model->total_unigrams == 730);

    // Add bigrams
    ngram_add_bigram(model, "the", "quick", 60);
    ngram_add_bigram(model, "quick", "brown", 40);
    assert(ngram_get_bigram_freq(model, "the", "quick") == 60);
    assert(ngram_get_bigram_freq(model, "the", "brown") == 0);

    // Add trigrams
    ngram_add_trigram(model, "the", "quick", "brown", 35);
    assert(ngram_get_trigram_freq(model, "the", "quick", "brown") == 35);
    assert(ngram_get_trigram_freq(model, "the", "quick", "fox") == 0);

    ngram_destroy(model);
    printf("  PASS  test_add_and_query\n");
}

static void test_predictions(void)
{
    NGramModel *model = ngram_create();

    // Train unigrams
    ngram_add_unigram(model, "i", 1000);
    ngram_add_unigram(model, "want", 500);
    ngram_add_unigram(model, "to", 800);
    ngram_add_unigram(model, "go", 300);
    ngram_add_unigram(model, "eat", 200);

    // Bigrams: "i want", "want to", "to go", "to eat"
    ngram_add_bigram(model, "i", "want", 400);
    ngram_add_bigram(model, "want", "to", 350);
    ngram_add_bigram(model, "to", "go", 200);
    ngram_add_bigram(model, "to", "eat", 150);

    // Trigram: "i want to"
    ngram_add_trigram(model, "i", "want", "to", 300);

    NGramPrediction preds[10];

    // Predict with 2-word context (triggers trigram or bigram)
    int n = ngram_predict(model, "i want", preds, 5);
    assert(n > 0);
    assert(strcmp(preds[0].word, "to") == 0);

    // Predict with 1-word context (triggers bigram)
    n = ngram_predict(model, "to", preds, 5);
    assert(n >= 2);
    // "go" (freq 200) should be ahead of "eat" (freq 150)
    assert(strcmp(preds[0].word, "go") == 0);
    assert(strcmp(preds[1].word, "eat") == 0);

    ngram_destroy(model);
    printf("  PASS  test_predictions\n");
}

static void test_load_buffer(void)
{
    NGramModel *model = ngram_create();

    const char *unigram_data = "apple\t50\nbanana\t30\ncherry\t20\n";
    int loaded = ngram_load_unigrams(model, unigram_data, strlen(unigram_data));
    assert(loaded == 3);
    assert(ngram_get_unigram_freq(model, "apple") == 50);
    assert(ngram_get_unigram_freq(model, "banana") == 30);

    const char *bigram_data = "eat|apple\t25\neat|banana\t15\n";
    loaded = ngram_load_bigrams(model, bigram_data, strlen(bigram_data));
    assert(loaded == 2);
    assert(ngram_get_bigram_freq(model, "eat", "apple") == 25);

    const char *trigram_data = "i|eat|apple\t12\n";
    loaded = ngram_load_trigrams(model, trigram_data, strlen(trigram_data));
    assert(loaded == 1);
    assert(ngram_get_trigram_freq(model, "i", "eat", "apple") == 12);

    ngram_destroy(model);
    printf("  PASS  test_load_buffer\n");
}

int main(void)
{
    printf("=== Running N-Gram Tests ===\n");
    test_create_destroy();
    test_add_and_query();
    test_predictions();
    test_load_buffer();
    printf("All N-Gram tests PASSED!\n\n");
    return 0;
}

