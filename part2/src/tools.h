/** @file Kalkulator operacji na wielomianach rzadkich wielu zmiennych
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
char* cut(char* line, size_t wordBegin, size_t wordEnd);

/**
 * Sprawdza czy znak jest cyfrą.
 * @param[in] c : sprawdzany znak
 * @return czy znak jest cyfrą
 */
bool isDigit(char c);

#endif /* TOOLS_H */