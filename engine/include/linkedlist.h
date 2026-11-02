/**
 * TypeAssist — Linked List (Correction History)
 *
 * A singly linked list for maintaining a chronological history
 * of corrections and transformations during a session.
 * New entries are inserted at the head for O(1) insertion.
 *
 * Time Complexity:
 *   Insert (head): O(1)
 *   Traverse:      O(n)
 *   Delete:        O(n)
 *
 * Space Complexity: O(n)
 */

#ifndef TYPEASSIST_LINKEDLIST_H
#define TYPEASSIST_LINKEDLIST_H

#include <stdbool.h>

#define LL_MAX_TEXT_LEN 2048

/**
 * Types of history entries.
 */
typedef enum {
    HISTORY_CORRECTION,     /* Spelling correction */
    HISTORY_AUTOCOMPLETE,   /* Accepted autocomplete */
    HISTORY_PREDICTION,     /* Accepted prediction */
    HISTORY_TONE_SHIFT,     /* Tone transformation */
    HISTORY_TRANSLATION,    /* Translation */
    HISTORY_GRAMMAR_FIX     /* Grammar correction */
} HistoryType;

/**
 * A node in the correction history linked list.
 */
typedef struct HistoryNode {
    char *original;         /* Original text */
    char *modified;         /* Modified/corrected text */
    HistoryType type;
    int timestamp;
    struct HistoryNode *next;
} HistoryNode;

/**
 * Linked list container with head pointer and count.
 */
typedef struct {
    HistoryNode *head;
    int count;
    int max_entries;        /* Limit to prevent unbounded growth */
} HistoryList;

/* Lifecycle */
HistoryList *history_create(int max_entries);
void history_destroy(HistoryList *list);

/* Core operations */
bool history_add(HistoryList *list, const char *original,
                 const char *modified, HistoryType type, int timestamp);
HistoryNode *history_get_latest(const HistoryList *list);
HistoryNode *history_get_all(const HistoryList *list);  /* Returns head */

/* Serialisation for WASM export */
int history_to_json(const HistoryList *list, char *buffer, int buffer_size);

/* Utility */
int  history_count(const HistoryList *list);
void history_clear(HistoryList *list);

#endif /* TYPEASSIST_LINKEDLIST_H */
