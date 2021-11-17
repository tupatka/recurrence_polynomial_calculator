/** @file
 *  Kalkulator operacji na wielomianach rzadkich wielu zmiennych
 *  @author Patrycja Stępień
*/
#include <stdbool.h>
#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include "stack.h"
#include "poly.h"
#include "tools.h"
#include "instructions.h"

void POP(Stack* stack, int lineNumber) {
    Pop(stack, lineNumber);
}

/**
 * Funkcja pomocnicza do PRINT, wypisuje wielomian na standardowe wyjście
 * w przypadku, gdy nie jest on współczynnikiem.
 * @param[in] p : wypisywany wielomian
 */
static void PrintPoly(const Poly *p) {
    for (size_t i = 0; i < p->size; i++) {
        if (i == 0) {
            printf("(");
        } else {
            printf("+(");
        }
        if (p->arr[i].p.arr == NULL) {
            printf("%ld,%d)", p->arr[i].p.coeff, p->arr[i].exp);
        } else {
            PrintPoly(&(p->arr[i].p));
            printf(",%d)", p->arr[i].exp);
        }
    }
}

void PRINT(Stack* stack, int lineNumber) {
    if (IsStackEmpty(stack)) {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", lineNumber);
        return;
    }

    Poly* polyTop = Top(stack);
    if (PolyIsCoeff(polyTop)) {
        printf("%ld", polyTop->coeff);
    } else {
        PrintPoly(polyTop);
    }
    printf("\n");
}

/**
 * Wykonuje na stosie arytmetyczne operacje dwuargumentowe:
 * dodawanie, odejmowanie i mnożenie.
 * @param[in, out] stack : stos
 * @param[in] lineNumber : numer wiersza z poleceniem
 * @param[in] operation : wykonywane działanie
 */
static void TwoArgOperation (Stack* stack, int lineNumber, enum TwoArgOp operation) {
    if (IsStackSingle(stack) || IsStackEmpty(stack)) {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", lineNumber);
        return;
    }

    Poly* PolyA = Top(stack);
    (stack->pointer)--;
    Poly* PolyB = Top(stack);
    (stack->pointer)++;

    Poly PolyRes;
    switch (operation) {
        case add:
            PolyRes = PolyAdd(PolyA, PolyB);
            break;
        case sub:
            PolyRes = PolySub(PolyA, PolyB);
            break;
        case mul:
            PolyRes = PolyMul(PolyA, PolyB);
            break;
    }
    POP(stack, lineNumber);
    POP(stack, lineNumber);
    Push(stack, PolyRes);
}

void ADD(Stack* stack, int lineNumber) {
    TwoArgOperation(stack, lineNumber, add);
}

void SUB(Stack* stack, int lineNumber) {
    TwoArgOperation(stack, lineNumber, sub);
}

void MUL(Stack* stack, int lineNumber) {
    TwoArgOperation(stack, lineNumber, mul);
}

void IS_EQ(Stack* stack, int lineNumber) {
    if (IsStackSingle(stack) || IsStackEmpty(stack)) {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", lineNumber);
        return;
    }
    Poly* PolyA = Top(stack);
    (stack->pointer)--;
    Poly* PolyB = Top(stack);
    (stack->pointer)++;

    if (PolyIsEq(PolyA, PolyB)) {
        printf("1\n");
    } else {
        printf("0\n");
    }
}

void DEG(Stack* stack, int lineNumber) {
    if (IsStackEmpty(stack)) {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", lineNumber);
        return;
    }
    Poly* PolyTop = Top(stack);
    printf("%d\n", PolyDeg(PolyTop));
}

void DEG_BY(Stack* stack, size_t idx, int lineNumber) {
    if (IsStackEmpty(stack)) {
        fprintf(stderr, "ERROR %u STACK UNDERFLOW\n", lineNumber);
        return;
    }
    Poly* PolyTop = Top(stack);
    printf("%d\n", PolyDegBy(PolyTop, idx));
}

void AT(Stack* stack, poly_coeff_t x, int lineNumber) {
    if (IsStackEmpty(stack)) {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", lineNumber);
        return;
    }
    Poly* PolyTop = Top(stack);
    Poly PolyRes = PolyAt(PolyTop, x);
    POP(stack, lineNumber);
    Push(stack, PolyRes);
}

void NEG(Stack* stack, int lineNumber) {
    if (IsStackEmpty(stack)) {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", lineNumber);
        return;
    }
    Poly* polyTop = Top(stack);
    Poly polyRes = PolyNeg(polyTop);
    POP(stack, lineNumber);
    Push(stack, polyRes);
}

void ZERO(Stack* stack) {
    Poly PolyTop = PolyZero();
    Push(stack, PolyTop);
}

void IS_COEFF(const Stack* stack, int lineNumber) {
    if (IsStackEmpty(stack)) {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", lineNumber);
        return;
    }

    if (PolyIsCoeff(Top(stack))) {
        printf("1\n");
    } else {
        printf("0\n");
    }
}

void IS_ZERO(const Stack* stack, int lineNumber) {
    if (IsStackEmpty(stack)) {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", lineNumber);
        return;
    }

    if (PolyIsZero(Top(stack))) {
        printf("1\n");
    } else {
        printf("0\n");
    }
}

void CLONE(Stack* stack, int lineNumber) {
    if (IsStackEmpty(stack)) {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", lineNumber);
        return;
    }
    Poly PolyTop = PolyClone(Top(stack));
    Push(stack, PolyTop);
}

/**
 * Usuwa z pamięci tablicę wielomianów oraz jej elementy.
 * @param[out] polyArr : tablica wielomianów
 * @param[in] polyArrSize : rozmiar tablicy wielomianów
 */
static void DestroyPolyArr(Poly* polyArr, size_t polyArrSize) {
    for (size_t i = 0; i < polyArrSize; i++) {
        PolyDestroy(&(polyArr[i]));
    }
    free(polyArr);
}

/**
 * Zdejmuje ze stosu kolejnych k wielomianów, tworzy z nich tablicę i zwraca ją w wyniku.
 * Wielomiany ze szczytu stosu są zapisywane w tablicy od końca.
 * @param[in, out] stack : stos
 * @param[in] k : ilość wielomianów w wynikowej tablicy
 * @return tablica k wielomianów zdjętych ze szczytu stosu
 */
static Poly* CreateComposeArgumentsArray(Stack* stack, size_t k) {
    Poly* q = malloc(k * sizeof(Poly));
    if (q == NULL) {
        exit(1);
    }
    for (size_t i = 0; i < k; i++) {
        q[k - 1 - i] = *(Top(stack));
        (stack->pointer)--;
    }
    return q;
}

void COMPOSE(Stack* stack, int lineNumber, size_t k) {
    if (!IsStackOfSizeAtLeastN(stack, 1)) {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", lineNumber);
        return;
    }
    Poly mainPoly = *(Top(stack));
    (stack->pointer)--;

    if (!IsStackOfSizeAtLeastN(stack, k)) {
        fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", lineNumber);
        // Zakładamy, że nie zdjęliśmy górnego wielomianu ze stosu.
        (stack->pointer)++;
        return;
    }

    Poly* q = CreateComposeArgumentsArray(stack, k);
    Poly result = PolyCompose(&mainPoly, k, q);

    PolyDestroy(&mainPoly);
    DestroyPolyArr(q, k);
    Push(stack, result);
}