/** @file
 *  Kalkulator operacji na wielomianach rzadkich wielu zmiennych
 *  @author Patrycja Stępień
*/

/**
 * Makro potrzebne do prawidłowego wykonywania funkcji getline
 */
#define _GNU_SOURCE

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "poly_execute.h"
#include "tools.h"
#include "poly.h"
#include "stack.h"

/**
 * Baza systemu dziesiętnego.
 */
#define DECIMAL_BASE 10

/**
 * Brak wczytanych danych.
 */
#define NONE -1

/**
 * Sprawdza, czy napis reprezentuje poprawny współczynnik.
 * @param[in] polyS : sprawdzany napis
 * @param[in] lineSize : długość napisu
 * @return czy jest poprawnym współczynnikiem
 */
static bool IsStringCoeff(char const* polyS, size_t lineSize) {
    // lineSize - 1 bo ostatni przechowywany znak to znak nowej linii
    for (size_t i = 0; i < lineSize - 1; i++) {
        if (polyS[i] == '-') {
            if (i != 0) {
                return false;
            } else {
                continue;
            }
        }
        if (!isdigit(polyS[i])) {
            return false;
        }
    }
    return true;
}

/**
 * Sprawdza, czy znak jest dozwolonym w wielomianach.
 * @param[in] c : sprawdzany znak
 * @return czy jest dozwolonym znakiem
 */
static bool IsCorrectSign(char c) {
    return c == '+' || c == '-' || c == '(' || c == ')' || isdigit(c) || c == ',';
}

/**
 * Sprawdza, z ilu osobnych jednomianów składa się wczytywany wielomian.
 * @param[in] polyS : wczytywany wielomian
 * @param[in] lineSize : długość napisu reprezentującego wczytywany wielomian
 * @return z ilu jednomianów składa się wielomian
 */
static int HowManyMonos(char const* polyS, size_t lineSize) {
    int howMany = 1;
    int numOfLeftBraces = 0;
    int numOfRightBraces = 0;

    for (size_t i = 0; i < lineSize - 1; i++) {
        if (polyS[i] == '(')
            numOfLeftBraces++;
        if (polyS[i] == ')')
            numOfRightBraces++;
        if (numOfLeftBraces == numOfRightBraces && polyS[i] == '+' && numOfRightBraces > 0)
            howMany++;
    }
    return howMany;
}

/**
 * Sprawdza, czy napis reprezentuje poprawny wielomian.
 * @param[in] polyS : wczytywany wielomian
 * @param[in] lineSize : długość napisu reprezentującego wczytywany wielomian
 * @return czy napis to poprawny wielomian
 */
static bool IsCorrectPoly(char const* polyS, size_t lineSize) {
    if (IsStringCoeff(polyS, lineSize)) {
        return true;
    }
    if (polyS == NULL) {
        return false;
    }
    if (polyS[0] != '(') {
        return false;
    }
    if (polyS[lineSize - 1] != '\n' && polyS[lineSize - 1] != ')') {
        return false;
    }
    int numOfLeftBraces = 1;
    int numOfRightBraces = 0;
    int numOfComas = 0;
    for (size_t i = 1; i < lineSize - 1; i++) {
        if (!IsCorrectSign(polyS[i])) {
            return false;
        }
        if (polyS[i] == ',') {
            if (polyS[i - 1] != ')' && !isdigit(polyS[i - 1])) {
                return false;
            }
            if (!isdigit(polyS[i + 1])) {
                return false;
            }
            numOfComas++;
        }
        if (polyS[i] == '(') {
            if (polyS[i - 1] != '+' && polyS[i - 1] != '(') {
                return false;
            }
            if (polyS[i + 1] != '(' && polyS[i + 1] != '-' && !isdigit(polyS[i + 1])) {
                return false;
            }
            numOfLeftBraces++;
        }
        if (polyS[i] == ')') {
            if (!isdigit(polyS[i - 1])) {
                return false;
            }
            if (polyS[i + 1] != '+' && polyS[i + 1] != ',' && polyS[i + 1] != '\n') {
                return false;
            }
            numOfRightBraces++;
        }
        if (polyS[i] == '-') {
            if (polyS[i - 1] != '(') {
                return false;
            }
            if (!isdigit(polyS[i + 1])) {
                return false;
            }
        }
        if (polyS[i] == '+') {
            if (polyS[i - 1] != ')') {
                return false;
            }
            if (polyS[i + 1] != '(') {
                return false;
            }
        }
        if (numOfRightBraces > numOfLeftBraces) {
            return false;
        }
    }
    if (numOfLeftBraces != numOfRightBraces) {
        return false;
    }
    if (numOfRightBraces != numOfComas) {
        return false;
    }

    return true;
}

/**
 * Parsuje współczynnik wielomianu i sprawdza czy jest on w dozwolonym zakresie.
 * @param[in] polyS : wczytywany wielomian
 * @param[in, out] okPoly : czy wczytywany wielomian jest poprawny
 * @return sparsowany wspołczynnik wielomianu
 */
static poly_coeff_t ParseCoeff(char const* polyS, bool* okPoly) {
    char* pEnd;
    poly_coeff_t c = strtol(polyS, &pEnd, DECIMAL_BASE);
    if (errno == ERANGE || (strcmp(pEnd, "\n") != 0 && strcmp(pEnd, "\0") != 0)) {
        *okPoly = false;
    }
    return c;
}

/**
 * Zwraca indeks końca wczytywanego jednomianu.
 * @param[in] indStart : początek sprawdzania
 * @param[in] lineSize : długość sprawdzanej linii
 * @param[in] polyS : sprawdzana linia
 * @return indeks końca jednomianu
 */
static size_t IterateTillMonoEnd(size_t indStart, size_t lineSize, char const* polyS) {
    size_t i = indStart;
    int numOfLeftBraces = 0;
    int numOfRightBraces = 0;

    while (i < lineSize) {
        if (polyS[i] == '(') {
            numOfLeftBraces++;
        }
        if (polyS[i] == ')') {
            numOfRightBraces++;
        }
        if (numOfLeftBraces == numOfRightBraces &&
            polyS[i] == '+' &&
            numOfRightBraces > 0) {
            return i;
        }
        i++;
    }
    // koniec ostatniego jednomianu to znak nowej linii
    i--;
    return i;
}

/**
 * Wycina z napisu reprezentującego wielomian napis reprezentujący jednomian.
 * @param[in, out] polyS : napis reprezentujący wielomian
 * @param[in] lineSize : długość linii
 * @param[in] indStart : początek wycinania
 * @return napis reprezentujący jednomian
 */
static char* CutMono(char* polyS, size_t lineSize, size_t* indStart) {
    size_t wordBegin = *indStart;
    char* word = &(polyS[wordBegin]);

    size_t firstIndexAfterMono = IterateTillMonoEnd(*indStart, lineSize, polyS);
    polyS[firstIndexAfterMono] = '\0';

    size_t sizeOfExtractedMono = firstIndexAfterMono - wordBegin + 1;
    char* extractedMono = malloc(sizeOfExtractedMono * sizeof(char));
    if (extractedMono == NULL) {
        exit(1);
    }
    sscanf(word, "%s", extractedMono);

    *indStart = firstIndexAfterMono;
    return extractedMono;
}

/**
 * Zwraca indeks przecinka poprzedzającego wykładnik jednomianu,
 * używane przy parsowaniu wielomianu.
 * @param[in] monoToParse : napis reprezentujący jednomian
 * @param[in] monoSize : długość napisu reprezentującego jednomian
 * @return indeks przecinka
 */
static size_t FindComaBeforeExp(char const* monoToParse, size_t monoSize) {
    size_t i = monoSize - 1;
    while (true) {
        if (monoToParse[i] == ',') {
            return i;
        }
        i--;
    }
    return i;
}

/**
 * Wycina z napisu reprezentującego jednomian napis reprezentujący wykładnik.
 * @param[in] monoToParse : napis reprezentujący jednomian
 * @param[in] monoSize : długość linii
 * @param[in] expBegin : początek wycinania
 * @return napis reprezentujący wykładnik
 */
static char* CutExp(char* monoToParse, size_t monoSize, size_t expBegin) {
    char* word = &monoToParse[expBegin];

    // prawy nawias za wykladnikiem jednomianu
    size_t firstIndexAfterExp = monoSize - 1;
    monoToParse[firstIndexAfterExp] = '\0';

    size_t sizeOfExtractedExp = firstIndexAfterExp - expBegin + 1;
    char* extractedExp = malloc(sizeOfExtractedExp * sizeof(char));
    if (extractedExp == NULL) {
        exit(1);
    }
    sscanf(word, "%s", extractedExp);

    monoToParse[firstIndexAfterExp] = ')';
    return extractedExp;
}

/**
 * Parsuje wykładnik jednomianu i sprawdza czy jest on w dozwolonym zakresie.
 * @param[in] expS : napis reprezentujący wykładnik
 * @param[in, out] okPoly : czy wczytywany wielomian jest poprawny
 * @return sparsowany wykładnik jednomianu
 */
static poly_exp_t ParseExp(char const* expS, bool* okPoly) {
    poly_exp_t e;
    int itemsRead = sscanf(expS, "%d", &e);
    if (itemsRead == NONE) {
        *okPoly = false;
    }
    return e;
}

/**
 * Znajduje początek wielomianu będącego współczynnikiem jednomianu.
 * @param[in] monoToParse : napis reprezentujący parsowany jednomian
 * @param[in] monoSize : długość napisu reprezentującego parsowany jednomian
 * @return indeks początku wielomianu
 */
static size_t FindPolyAsCoeffBegin(char const* monoToParse, size_t monoSize) {
    size_t i = monoSize;
    int numOfLeftBraces = 0;
    int numOfRightBraces = 0;

    while (true) {
        if (monoToParse[i] == ')') {
            numOfRightBraces++;
        }
        if (monoToParse[i] == '(') {
            numOfLeftBraces++;
        }
        if (numOfLeftBraces == numOfRightBraces &&
            monoToParse[i] == '(' &&
            numOfRightBraces > 0) {
            return i;
        }
        i--;
    }

    return i;
}

/**
 * Wycina z napisu jednomiany, parsuje je i tworzy z nich wielomian.
 * @param[in] polyS : napis reprezentujący parsowany wielomian
 * @param[in] lineSize : długość napisu reprezentującego parsowany wielomian
 * @param[in, out] okPoly : czy wczytywany wielomian jest poprawny
 * @return sparsowany wielomian
 */
static Poly Parse(char* polyS, size_t lineSize, bool* okPoly);

/**
 * Parsuje jednomian.
 * @param[in] monoToParse : napis reprezentujący parsowany jednomian
 * @param[in] monoSize : długość napisu reprezentującego parsowany jednomian
 * @param[in, out] okPoly : czy wczytywany wielomian jest poprawny
 * @return sparsowany jednomian
 */
static Mono ParseMono(char* monoToParse, size_t monoSize, bool* okPoly) {
    Mono parsedMono;

    size_t expBegin = FindComaBeforeExp(monoToParse, monoSize) + 1;
    char* expS = CutExp(monoToParse, monoSize, expBegin);

    parsedMono.exp = ParseExp(expS, okPoly);
    free(expS);
    size_t polyAsCoeffBegin = FindPolyAsCoeffBegin(monoToParse, monoSize) + 1;
    size_t polyAsCoeffEnd = expBegin - 1;

    char* coeffS = Cut(monoToParse, polyAsCoeffBegin, polyAsCoeffEnd);
    if (monoToParse[polyAsCoeffEnd - 1] == ')') {
        size_t lineSize = polyAsCoeffEnd - polyAsCoeffBegin + 1;
        parsedMono.p = Parse(coeffS, lineSize, okPoly);
    } else {
        char* pEnd;
        poly_coeff_t c = strtol(coeffS, &pEnd, 0);
        if (errno == ERANGE) {
            *okPoly = false;
        }
        parsedMono.p = PolyFromCoeff(c);
    }
    free(coeffS);

    return parsedMono;
}

static Poly Parse(char* polyS, size_t lineSize, bool* okPoly) {
    int howMany = HowManyMonos(polyS, lineSize);
    Mono* monos = malloc(howMany * sizeof(Mono));
    if(monos == NULL) {
        exit(1);
    }

    // indStart będzie wskazywał na początek następnego wycinanego napisu.
    size_t indStart = 0;
    for (int i = 0; i < howMany; i++) {
        // Po wywołaniu CutMono indStart wskazuje na plus pomiedzy
        // jednomianami lub na pierwszy indeks za wielomianem.
        size_t monoBegin = indStart;
        char* monoToParse = CutMono(polyS, lineSize, &indStart);

        // Ustawiamy go tak, aby przed kolejnym wywołaniem wskazywał na
        // pierwszy lewy nawias następnego jednomianu.
        if (indStart < lineSize) {
            indStart++;
        }

        // Rozmiar bez znaku '\0'.
        size_t monoSize = indStart - monoBegin - 1;
        Mono mono = ParseMono(monoToParse, monoSize, okPoly);
        monos[i] = mono;
        free(monoToParse);
    }
    Poly res = PolyAddMonos(howMany, monos);
    free(monos);

    return res;
}

/**
 * Parsuje i wrzuca na stos wczytany ze standardowego wejścia napis reprezentujący wielomian
 * @param[in, out] stack : stos
 * @param[in] polyS : napis reprezentujący parsowany wielomian
 * @param[in] lineSize : długość napisu reprezentującego parsowany wielomian
 * @param[in] lineNumber : numer wczytanego wiersza
 */
static void ParsePoly(Stack* stack, char* polyS, size_t lineSize, int lineNumber) {
    if (!IsCorrectPoly(polyS, lineSize)) {
        fprintf(stderr, "ERROR %u WRONG POLY\n", lineNumber);
        return;
    }

    Poly newPoly;
    bool okPoly = true;
    if (IsStringCoeff(polyS, lineSize)) {
        poly_coeff_t c = ParseCoeff(polyS, &okPoly);
        if (okPoly) {
            newPoly = PolyFromCoeff(c);
        }
    } else {
        newPoly = Parse(polyS, lineSize, &okPoly);
    }

    if (okPoly) {
        Push(stack, newPoly);
    } else {
        fprintf(stderr, "ERROR %u WRONG POLY\n", lineNumber);
    }
}

void PolyScan(Stack* stack, int lineNumber) {
    char* polyS = NULL;
    size_t length = 0;
    ssize_t lineSize = 0;

    lineSize = getline(&polyS, &length, stdin);
    if (polyS == NULL) {
        exit(1);
    }

    ParsePoly(stack, polyS, lineSize, lineNumber);
    free(polyS);
}
