/** @file
 *  Kalkulator operacji na wielomianach rzadkich wielu zmiennych
 *  @author Patrycja Stępień
*/

#ifndef TOOLS_H
#define TOOLS_H

#include <stdbool.h>

/**
 * Wycina napis od początkowego indeksu do końcowego.
 * @param[in] line : wiersz z którego wycinany jest napis
 * @param[in] wordBegin : pierwszy indeks wycinanego słowa
 * @param[in] wordEnd : za ostatni indeks wycinanego słowa
 * @return wycięty napis
 */
char* Cut(char* line, size_t wordBegin, size_t wordEnd);

/**
 * Zwraca minimum z dwóch wartości typu size_t.
 * @param[in] a, b : porównywane wartości
 * @return mniejsza z porównywanych wartości
 */
static inline size_t Min(size_t a, size_t b){
    return a <= b ? a : b;
}

#endif /* TOOLS_H */