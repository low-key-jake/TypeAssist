/**
 * TypeAssist — Stack Tests
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/stack.h"

/* ---- Test: basic create / destroy ---- */
static void test_create_destroy(void)
{
    Stack *s = stack_create();
    assert(s != NULL);
    assert(stack_size(s) == 0);
    assert(stack_is_empty(s));
    stack_destroy(s);
    printf("  PASS  test_create_destroy\n");
}

/* ---- Test: push and pop ---- */
static void test_push_pop(void)
{
    Stack *s = stack_create();
    assert(stack_push(s, "hello", 1));
    assert(stack_size(s) == 1);

    StackEntry e;
    assert(stack_pop(s, &e));
    assert(strcmp(e.text, "hello") == 0);
    assert(e.timestamp == 1);
    free(e.text);

    assert(stack_is_empty(s));
    stack_destroy(s);
    printf("  PASS  test_push_pop\n");
}

/* ---- Test: LIFO ordering ---- */
static void test_lifo_order(void)
{
    Stack *s = stack_create();
    stack_push(s, "first",  10);
    stack_push(s, "second", 20);
    stack_push(s, "third",  30);

    StackEntry e;
    assert(stack_pop(s, &e));
    assert(strcmp(e.text, "third") == 0);
    assert(e.timestamp == 30);
    free(e.text);

    assert(stack_pop(s, &e));
    assert(strcmp(e.text, "second") == 0);
    free(e.text);

    assert(stack_pop(s, &e));
    assert(strcmp(e.text, "first") == 0);
    free(e.text);

    assert(stack_is_empty(s));
    stack_destroy(s);
    printf("  PASS  test_lifo_order\n");
}

/* ---- Test: peek without removal ---- */
static void test_peek(void)
{
    Stack *s = stack_create();
    stack_push(s, "alpha", 1);
    stack_push(s, "beta", 2);

    StackEntry e;
    assert(stack_peek(s, &e));
    assert(strcmp(e.text, "beta") == 0);
    assert(e.timestamp == 2);
    free(e.text);

    assert(stack_size(s) == 2);   /* unchanged */

    /* Peek again */
    assert(stack_peek(s, &e));
    assert(strcmp(e.text, "beta") == 0);
    free(e.text);

    stack_destroy(s);
    printf("  PASS  test_peek\n");
}

/* ---- Test: empty stack operations ---- */
static void test_empty_stack(void)
{
    Stack *s = stack_create();
    StackEntry e;

    assert(!stack_pop(s, &e));
    assert(!stack_peek(s, &e));
    assert(stack_is_empty(s));

    stack_destroy(s);
    printf("  PASS  test_empty_stack\n");
}

/* ---- Test: push many, pop all ---- */
static void test_many(void)
{
    Stack *s = stack_create();
    for (int i = 0; i < 200; i++) {
        char buf[32];
        snprintf(buf, sizeof(buf), "entry_%d", i);
        assert(stack_push(s, buf, i));
    }
    assert(stack_size(s) == 200);

    StackEntry e;
    for (int i = 199; i >= 0; i--) {
        assert(stack_pop(s, &e));
        char expected[32];
        snprintf(expected, sizeof(expected), "entry_%d", i);
        assert(strcmp(e.text, expected) == 0);
        assert(e.timestamp == i);
        free(e.text);
    }

    assert(stack_is_empty(s));
    stack_destroy(s);
    printf("  PASS  test_many\n");
}

/* ---- Test: clear and reuse ---- */
static void test_clear_reuse(void)
{
    Stack *s = stack_create();
    stack_push(s, "one", 1);
    stack_push(s, "two", 2);
    stack_push(s, "three", 3);

    stack_clear(s);
    assert(stack_is_empty(s));
    assert(stack_size(s) == 0);

    /* Reuse after clear */
    stack_push(s, "four", 4);
    StackEntry e;
    assert(stack_pop(s, &e));
    assert(strcmp(e.text, "four") == 0);
    free(e.text);

    stack_destroy(s);
    printf("  PASS  test_clear_reuse\n");
}

/* ---- Test: NULL inputs ---- */
static void test_null_inputs(void)
{
    assert(!stack_push(NULL, "hello", 1));
    assert(!stack_pop(NULL, NULL));
    assert(!stack_peek(NULL, NULL));
    assert(stack_is_empty(NULL));
    assert(stack_size(NULL) == 0);

    Stack *s = stack_create();
    assert(!stack_push(s, NULL, 1));
    stack_destroy(s);

    stack_clear(NULL);    /* should not crash */
    stack_destroy(NULL);  /* should not crash */
    printf("  PASS  test_null_inputs\n");
}

/* ---- Test: text_len field ---- */
static void test_text_len(void)
{
    Stack *s = stack_create();
    stack_push(s, "hello world", 1);

    StackEntry e;
    assert(stack_peek(s, &e));
    assert(e.text_len == 11);
    free(e.text);

    assert(stack_pop(s, &e));
    assert(e.text_len == 11);
    free(e.text);

    stack_destroy(s);
    printf("  PASS  test_text_len\n");
}

int main(void)
{
    printf("=== Stack Tests ===\n");
    test_create_destroy();
    test_push_pop();
    test_lifo_order();
    test_peek();
    test_empty_stack();
    test_many();
    test_clear_reuse();
    test_null_inputs();
    test_text_len();
    printf("All stack tests passed!\n");
    return 0;
}
