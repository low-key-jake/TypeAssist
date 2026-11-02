/**
 * TypeAssist — Stack (Undo Functionality)
 *
 * A dynamic array-based stack for implementing undo/redo.
 * Each stack entry stores a snapshot of text state.
 *
 * Time Complexity:
 *   Push: O(1) amortised
 *   Pop:  O(1)
 *   Peek: O(1)
 *
 * Space Complexity: O(n) where n = number of entries
 */

#ifndef TYPEASSIST_STACK_H
#define TYPEASSIST_STACK_H

#include <stdbool.h>

#define STACK_INITIAL_CAPACITY 32
#define STACK_MAX_TEXT_LEN 4096

/**
 * A stack entry storing a text state and metadata.
 */
typedef struct {
    char *text;         /* Dynamically allocated text snapshot */
    int text_len;
    int timestamp;      /* Relative timestamp for ordering */
} StackEntry;

/**
 * Stack container.
 */
typedef struct {
    StackEntry *data;
    int size;
    int capacity;
} Stack;

/* Lifecycle */
Stack *stack_create(void);
void stack_destroy(Stack *stack);

/* Core operations */
bool stack_push(Stack *stack, const char *text, int timestamp);
bool stack_pop(Stack *stack, StackEntry *out);
bool stack_peek(const Stack *stack, StackEntry *out);

/* Utility */
int  stack_size(const Stack *stack);
bool stack_is_empty(const Stack *stack);
void stack_clear(Stack *stack);

#endif /* TYPEASSIST_STACK_H */
