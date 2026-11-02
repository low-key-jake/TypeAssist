/**
 * TypeAssist — Linked List Tests
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/linkedlist.h"

static void test_create_destroy(void)
{
    HistoryList *list = history_create(10);
    assert(list != NULL);
    assert(history_count(list) == 0);
    assert(history_get_latest(list) == NULL);
    assert(history_get_all(list) == NULL);
    history_destroy(list);
    printf("  PASS  test_create_destroy\n");
}

static void test_add_and_order(void)
{
    HistoryList *list = history_create(10);
    assert(history_add(list, "recieve", "receive", HISTORY_CORRECTION, 100));
    assert(history_count(list) == 1);

    HistoryNode *latest = history_get_latest(list);
    assert(latest != NULL);
    assert(strcmp(latest->original, "recieve") == 0);
    assert(strcmp(latest->modified, "receive") == 0);
    assert(latest->type == HISTORY_CORRECTION);
    assert(latest->timestamp == 100);

    // Add second entry - should be head (newest first)
    assert(history_add(list, "teh", "the", HISTORY_CORRECTION, 200));
    assert(history_count(list) == 2);

    latest = history_get_latest(list);
    assert(latest != NULL);
    assert(strcmp(latest->original, "teh") == 0);
    assert(latest->next != NULL);
    assert(strcmp(latest->next->original, "recieve") == 0);

    history_destroy(list);
    printf("  PASS  test_add_and_order\n");
}

static void test_max_entries(void)
{
    HistoryList *list = history_create(3);
    assert(history_add(list, "w1", "c1", HISTORY_CORRECTION, 1));
    assert(history_add(list, "w2", "c2", HISTORY_CORRECTION, 2));
    assert(history_add(list, "w3", "c3", HISTORY_CORRECTION, 3));
    assert(history_count(list) == 3);

    // Adding 4th should evict the oldest ("w1")
    assert(history_add(list, "w4", "c4", HISTORY_CORRECTION, 4));
    assert(history_count(list) == 3);

    HistoryNode *curr = history_get_all(list);
    assert(curr != NULL && strcmp(curr->original, "w4") == 0);
    curr = curr->next;
    assert(curr != NULL && strcmp(curr->original, "w3") == 0);
    curr = curr->next;
    assert(curr != NULL && strcmp(curr->original, "w2") == 0);
    assert(curr->next == NULL); // w1 was evicted

    history_destroy(list);
    printf("  PASS  test_max_entries\n");
}

static void test_to_json(void)
{
    HistoryList *list = history_create(5);
    history_add(list, "definately", "definitely", HISTORY_CORRECTION, 10);
    history_add(list, "hel", "hello", HISTORY_AUTOCOMPLETE, 20);

    char buf[1024];
    int len = history_to_json(list, buf, sizeof(buf));
    assert(len > 0);
    assert(buf[0] == '[');
    assert(strstr(buf, "\"original\":\"hel\"") != NULL);
    assert(strstr(buf, "\"modified\":\"hello\"") != NULL);
    assert(strstr(buf, "\"original\":\"definately\"") != NULL);

    history_destroy(list);
    printf("  PASS  test_to_json\n");
}

static void test_clear(void)
{
    HistoryList *list = history_create(5);
    history_add(list, "a", "b", HISTORY_TONE_SHIFT, 1);
    history_add(list, "c", "d", HISTORY_TRANSLATION, 2);
    assert(history_count(list) == 2);

    history_clear(list);
    assert(history_count(list) == 0);
    assert(history_get_latest(list) == NULL);

    // Re-adding works after clear
    assert(history_add(list, "e", "f", HISTORY_GRAMMAR_FIX, 3));
    assert(history_count(list) == 1);

    history_destroy(list);
    printf("  PASS  test_clear\n");
}

static void test_edge_cases(void)
{
    assert(history_create(0) == NULL);
    assert(history_create(-5) == NULL);
    assert(!history_add(NULL, "a", "b", HISTORY_CORRECTION, 1));

    HistoryList *list = history_create(5);
    assert(!history_add(list, NULL, "b", HISTORY_CORRECTION, 1));
    assert(!history_add(list, "a", NULL, HISTORY_CORRECTION, 1));

    char buf[10];
    int res = history_to_json(list, buf, sizeof(buf)); // empty list
    assert(res >= 2);
    assert(strcmp(buf, "[]") == 0);

    history_destroy(list);
    printf("  PASS  test_edge_cases\n");
}

int main(void)
{
    printf("=== Running Linked List Tests ===\n");
    test_create_destroy();
    test_add_and_order();
    test_max_entries();
    test_to_json();
    test_clear();
    test_edge_cases();
    printf("All Linked List tests PASSED!\n\n");
    return 0;
}

