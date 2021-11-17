/** @file
 *  Kalkulator operacji na wielomianach rzadkich wielu zmiennych
 *  @author Patrycja Stępień
*/
#ifndef POLY_EXECUTE_H
#define POLY_EXECUTE_H

#include "stack.h"

/**
 * Wczytuje z wielomian ze standardowego wejścia.
 * @param[in] stack : stos
 * @param[in] lineNumber : numer wczytanego wiersza
 */
void PolyScan(Stack* stack, int lineNumber);

#endif /* POLY_EXECUTE_H */