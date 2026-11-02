/**
 * TypeAssist — Linked List (Correction History) Implementation
 *
 * Singly linked list with head insertion.  Enforces max_entries by
 * removing the oldest (tail) node when the limit is reached.
 */

#include "linkedlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- Internal helpers ---------- */

/**
 * Free a single HistoryNode and its dynamically-allocated strings.
 */
static void free_node(HistoryNode *node)
{
    if (!node)
        return;
    free(node->original);
    free(node->modified);
    free(node);
}

/**
 * Remove the tail node from the list (the oldest entry).
 * Assumes the list has at least one node.
 */
static void remove_tail(HistoryList *list)
{
    if (!list || !list->head)
        return;

    /* Special case: only one node */
    if (list->head->next == NULL) {
        free_node(list->head);
        list->head = NULL;
        list->count = 0;
        return;
    }

    /* Walk to the second-to-last node */
    HistoryNode *prev = list->head;
    while (prev->next->next != NULL)
        prev = prev->next;

    free_node(prev->next);
    prev->next = NULL;
    list->count--;
}

/* ---------- Lifecycle ---------- */

HistoryList *history_create(int max_entries)
{
    if (max_entries <= 0)
        return NULL;

    HistoryList *list = malloc(sizeof(HistoryList));
    if (!list)
        return NULL;

    list->head        = NULL;
    list->count       = 0;
    list->max_entries = max_entries;
    return list;
}

void history_destroy(HistoryList *list)
{
    if (!list)
        return;
    history_clear(list);
    free(list);
}

/* ---------- Core operations ---------- */

bool history_add(HistoryList *list, const char *original,
                 const char *modified, HistoryType type, int timestamp)
{
    if (!list || !original || !modified)
        return false;

    /* If at capacity, evict the oldest (tail) node first */
    if (list->count >= list->max_entries)
        remove_tail(list);

    HistoryNode *node = malloc(sizeof(HistoryNode));
    if (!node)
        return false;

    node->original  = strdup(original);
    node->modified  = strdup(modified);
    if (!node->original || !node->modified) {
        free(node->original);
        free(node->modified);
        free(node);
        return false;
    }

    node->type      = type;
    node->timestamp = timestamp;

    /* Insert at head */
    node->next  = list->head;
    list->head  = node;
    list->count++;
    return true;
}

HistoryNode *history_get_latest(const HistoryList *list)
{
    if (!list)
        return NULL;
    return list->head;
}

HistoryNode *history_get_all(const HistoryList *list)
{
    if (!list)
        return NULL;
    return list->head;
}

/* ---------- Serialisation ---------- */

/**
 * Escape a string for JSON: handle \, ", and control chars.
 * Writes into dest (up to dest_size-1 chars), always null-terminates.
 * Returns the number of characters written (excluding NUL), or -1 if truncated.
 */
static int json_escape(char *dest, int dest_size, const char *src)
{
    int di = 0;
    for (int si = 0; src[si] != '\0'; si++) {
        char c = src[si];
        if (c == '"' || c == '\\') {
            if (di + 2 >= dest_size) return -1;
            dest[di++] = '\\';
            dest[di++] = c;
        } else if (c == '\n') {
            if (di + 2 >= dest_size) return -1;
            dest[di++] = '\\';
            dest[di++] = 'n';
        } else if (c == '\r') {
            if (di + 2 >= dest_size) return -1;
            dest[di++] = '\\';
            dest[di++] = 'r';
        } else if (c == '\t') {
            if (di + 2 >= dest_size) return -1;
            dest[di++] = '\\';
            dest[di++] = 't';
        } else {
            if (di + 1 >= dest_size) return -1;
            dest[di++] = c;
        }
    }
    dest[di] = '\0';
    return di;
}

int history_to_json(const HistoryList *list, char *buffer, int buffer_size)
{
    if (!list || !buffer || buffer_size <= 0)
        return -1;

    int pos = 0;

    /* Opening bracket */
    if (pos + 1 >= buffer_size) return -1;
    buffer[pos++] = '[';

    HistoryNode *node = list->head;
    int first = 1;

    while (node) {
        /* Escape original and modified strings */
        char esc_orig[LL_MAX_TEXT_LEN * 2];
        char esc_mod[LL_MAX_TEXT_LEN * 2];
        if (json_escape(esc_orig, sizeof(esc_orig), node->original) < 0)
            return -1;
        if (json_escape(esc_mod, sizeof(esc_mod), node->modified) < 0)
            return -1;

        /* Format one JSON object */
        char entry[LL_MAX_TEXT_LEN * 4 + 128];
        int elen = snprintf(entry, sizeof(entry),
            "%s{\"original\":\"%s\",\"modified\":\"%s\",\"type\":%d,\"timestamp\":%d}",
            first ? "" : ",",
            esc_orig, esc_mod, (int)node->type, node->timestamp);

        if (elen < 0 || pos + elen >= buffer_size)
            return -1;

        memcpy(buffer + pos, entry, (size_t)elen);
        pos += elen;
        first = 0;
        node = node->next;
    }

    /* Closing bracket */
    if (pos + 1 >= buffer_size) return -1;
    buffer[pos++] = ']';
    buffer[pos]   = '\0';

    return pos;
}

/* ---------- Utility ---------- */

int history_count(const HistoryList *list)
{
    if (!list)
        return 0;
    return list->count;
}

void history_clear(HistoryList *list)
{
    if (!list)
        return;

    HistoryNode *node = list->head;
    while (node) {
        HistoryNode *next = node->next;
        free_node(node);
        node = next;
    }
    list->head  = NULL;
    list->count = 0;
}
