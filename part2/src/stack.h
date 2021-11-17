/** @file Kalkulator operacji na wielomianach rzadkich wielu zmiennych
 *  @author Patrycja Stępień
*/

#ifndef STACK_H
#define STACK_H

#include "poly.h"

/**
 * Struktura reprezentująca stos
 */
typedef struct Stack {
    Poly* arr;  /**< Tablica implementująca stos */
    size_t pointer;  /**< Wskaźnik na wierzchołek stosu (na pierwsze wolne pole) */
    size_t curr_size; /**< Wielkość tablicy implemetującej stos */
} Stack;

/**
 * Tworzy pusty stos.
 * @return pusty stos
 */
struct Stack StackCreate();

/**
 * Usuwa stos i zwalnia pamięć po nim.
 * @param[in] stack : stos
 */
void StackDestroy(Stack* stack);

/**
 * Zwraca wskaźnik na wielomian na wierzchu stosu.
 * @param[in] stack : stos
 * @return wskaźnik na wielomian na wierzchu stosu
 */
Poly* Top(const Stack* stack);

/**
 * Wrzuca wielomian na wierzch stosu.
 * @param[in] stack : stos
 * @param[in] newPoly : wrzucany wielomian
 */
void Push(Stack* stack, Poly newPoly);

/**
 * Usuwa wielomian z wierzchołka stosu.
 * @param[in] stack : stos
 * @param[in] lineNumber : numer aktualnego wiersza
 */
void Pop(Stack* stack, int lineNumber);

/**
 * Sprawdza czy stos jest pusty.
 * @param[in] stack : stos
 * @return czy stos jest pusty
 */
bool IsStackEmpty(const Stack* stack);

/**
 * Sprawdza czy stos jest do połowy pusty.
 * @param[in] stack : stos
 * @return czy stos jest do połowy pusty pusty
 */
bool IsStackHalfEmpty(const Stack* stack);

/**
 * Sprawdza czy na stosie znajduje się dokładnie jeden wielomian.
 * @param[in] stack : stos
 * @return czy na stosie znajduje się dokładnie jeden wielomian
 */
bool IsStackSingle(const Stack* stack);

#endif /* STACK_H */