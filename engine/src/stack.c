/**
 * TypeAssist — Stack Implementation
 *
 * Dynamic array-based stack for undo/redo text snapshots.
 * Grows by doubling when full.
 */

#include "stack.h"
#include <stdlib.h>
#include <string.h>

/* ---------- Lifecycle ---------- */

Stack *stack_create(void)
{
    Stack *stack = malloc(sizeof(Stack));
    if (!stack)
        return NULL;

    stack->data = malloc(sizeof(StackEntry) * STACK_INITIAL_CAPACITY);
    if (!stack->data) {
        free(stack);
        return NULL;
    }

    stack->size     = 0;
    stack->capacity = STACK_INITIAL_CAPACITY;
    return stack;
}

void stack_destroy(Stack *stack)
{
    if (!stack)
        return;
    stack_clear(stack);
    free(stack->data);
    free(stack);
}

/* ---------- Core operations ---------- */

bool stack_push(Stack *stack, const char *text, int timestamp)
{
    if (!stack || !text)
        return false;

    /* Grow if full */
    if (stack->size >= stack->capacity) {
        int new_cap = stack->capacity * 2;
        StackEntry *new_data = realloc(stack->data,
                                       sizeof(StackEntry) * (size_t)new_cap);
        if (!new_data)
            return false;
        stack->data     = new_data;
        stack->capacity = new_cap;
    }

    StackEntry *entry = &stack->data[stack->size];
    entry->text = strdup(text);
    if (!entry->text)
        return false;

    entry->text_len  = (int)strlen(text);
    entry->timestamp = timestamp;
    stack->size++;
    return true;
}

bool stack_pop(Stack *stack, StackEntry *out)
{
    if (!stack || stack->size == 0)
        return false;

    stack->size--;
    StackEntry *top = &stack->data[stack->size];

    if (out) {
        /* Transfer ownership of the text pointer to caller */
        out->text      = top->text;
        out->text_len  = top->text_len;
        out->timestamp = top->timestamp;
    } else {
        /* No receiver — free the text ourselves */
        free(top->text);
    }

    /* Clear the slot (the text pointer has been moved or freed) */
    top->text = NULL;
    return true;
}

bool stack_peek(const Stack *stack, StackEntry *out)
{
    if (!stack || stack->size == 0 || !out)
        return false;

    const StackEntry *top = &stack->data[stack->size - 1];

    /* Give the caller their own copy of the text so they can free it */
    out->text = strdup(top->text);
    if (!out->text)
        return false;

    out->text_len  = top->text_len;
    out->timestamp = top->timestamp;
    return true;
}

/* ---------- Utility ---------- */

int stack_size(const Stack *stack)
{
    if (!stack)
        return 0;
    return stack->size;
}

bool stack_is_empty(const Stack *stack)
{
    if (!stack)
        return true;
    return stack->size == 0;
}

void stack_clear(Stack *stack)
{
    if (!stack)
        return;

    for (int i = 0; i < stack->size; i++) {
        free(stack->data[i].text);
        stack->data[i].text = NULL;
    }
    stack->size = 0;
}
