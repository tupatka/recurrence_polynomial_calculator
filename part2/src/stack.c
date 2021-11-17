/** @file Kalkulator operacji na wielomianach rzadkich wielu zmiennych
 *  @author Patrycja Stępień
*/

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stack.h"
#include "poly.h"

#define INIT_ARRAY_SIZE 4

struct Stack StackCreate() {
    Stack stack;
    stack.arr = malloc(INIT_ARRAY_SIZE * sizeof(Poly));
    if (stack.arr == NULL) {
        exit(1);
    }
    stack.curr_size = INIT_ARRAY_SIZE;
    stack.pointer = 0;

    return stack;
}

/**
 * Zwiększa dwukrotnie rozmiar tablicy na której zaimplementowany jest stos.
 * @param[in] stack : stos
 */
static void GrowStack(Stack* stack) {
    stack->curr_size = stack->curr_size * 2;
    stack->arr = realloc(stack->arr, stack->curr_size * sizeof(Poly));
    if (stack->arr == NULL) {
        exit(1);
    }
}

/**
 * Zmniejsza dwukrotnie rozmiar tablicy na której zaimplementowany jest stos.
 * @param[in] stack : stos
 */
static void DecreaseStack(Stack* stack) {
    if (stack->curr_size <= INIT_ARRAY_SIZE) {
        return;
    }
    stack->curr_size = stack->curr_size / 2;
    stack->arr = realloc(stack->arr, stack->curr_size * sizeof(Poly));
    if (stack->arr == NULL) {
        exit(1);
    }
}

void StackDestroy(Stack* stack) {
    for (size_t i = 0; i < stack->pointer; i++) {
        PolyDestroy(&(stack->arr[i]));
    }
    free(stack->arr);
}

/**
 * Sprawdza czy stos jest wypełniony.
 * @param[in] stack : stos
 * @return czy stos jest wypełniony
 */
static bool IsStackFull(const Stack* stack) {
    return stack->curr_size == stack->pointer;
}

bool IsStackEmpty(const Stack* stack) {
    return stack->pointer == 0;
}

bool IsStackHalfEmpty(const Stack* stack) {
    return stack->pointer <= (stack->curr_size) / 2;
}

bool IsStackSingle(const Stack* stack) {
    return stack->pointer == 1;
}

void Pop(Stack* stack, int lineNumber) {
    if (IsStackEmpty(stack)) {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", lineNumber);
        return;
    }
    (stack->pointer)--;
    PolyDestroy(&(stack->arr[stack->pointer]));

    if (IsStackHalfEmpty(stack)) {
        DecreaseStack(stack);
    }
}

Poly* Top(const Stack* stack) {
    return &(stack->arr[stack->pointer - 1]);
}

void Push(Stack* stack, Poly newPoly) {
    if (IsStackFull(stack)) {
        GrowStack(stack);
    }
    stack->arr[stack->pointer] = PolyClone(&newPoly);
    PolyDestroy(&newPoly);
    (stack->pointer)++;
}