/** @file
*/

#include "poly.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * Baza systemu dwójkowego, używane w algorytmie szybkiego potęgowania.
 */
#define BINARY_BASE 2

/**
 * Dowolna liczba podniesiona do potęgi zero,
 * używane w algorytmie szybkiego potęgowania.
 */
#define START_EXP_VALUE 1

/**
 * Rozmiar wielomianu postaci @f$cx^0@f$.
 */
#define SINGLE_SIZE 1

/**
 * Zwraca liczbę podniesioną do danej potęgi.
 * @param[in] basis : baza potęgowania
 * @param[in] exp : wykładnik potęgowania
 * @return wynik potęgowania
 */
static poly_coeff_t Exponentiation(poly_coeff_t basis, poly_exp_t exp) {
    if (basis == 0 && exp == 0) {
        return START_EXP_VALUE;
    }

    poly_coeff_t result = START_EXP_VALUE;
    poly_exp_t exponent = exp;

    while (exponent > 0) {
        if (exponent % BINARY_BASE == 1) {
            result *= basis;
        }
        basis *= basis;
        exponent /= BINARY_BASE;
    }

    return result;
}

/**
 * Komparator do sortowania tablicy jednomianów.
 * @param[in] A, B : porównywane jednomiany
 * @return wartość określająca czy pierwszy z nich jest większy.
 */
static int CompareMonos(const void* A, const void* B) {
    struct Mono* monoA = (struct Mono*) A;
    struct Mono* monoB = (struct Mono*) B;

    return monoA->exp - monoB->exp;
}

/**
 * Tworzy wielomian i rezerwuje pamięć dla tablicy jednomianów.
 * zadanej wielkości
 * @param[in] n : wielkość oczekiwanej tablicy jednomianów
 * @return wielomian
 */
static Poly PolyOfSizeN(size_t n) {
    Poly new;
    new.size = n;
    new.arr = malloc(n * sizeof(Mono));
    if (new.arr == NULL) {
        exit(1);
    }

    return new;
}

void PolyDestroy(Poly *p) {
    if (PolyIsCoeff(p)) {
        p->coeff = 0;
    }

    for (size_t i = 0; i < p->size; i++) {
        MonoDestroy(&(p->arr[i]));
    }
    free(p->arr);
    p->size = 0;
}

Poly PolyClone(const Poly *p) {
    if (PolyIsCoeff(p)) {
        return PolyFromCoeff(p->coeff);
    } else {
        Poly copiedPoly = PolyOfSizeN(p->size);
        for (size_t i = 0; i < p->size; i++) {
            copiedPoly.arr[i].p = PolyClone(&(p->arr[i].p));
            copiedPoly.arr[i].exp = p->arr[i].exp;
        }
        return copiedPoly;
    }
}

/**
 * Tworzy wielomian postaci @f$cx^0@f$.
 * @param[in] c : współczynnik czy zwracanym wielomianie
 * @return wielomian @f$cx^0@f$.
 */
static Poly XToZeroFromCoeff(poly_coeff_t c) {
    Poly result = PolyOfSizeN(SINGLE_SIZE);
    result.arr[0].exp = 0;
    result.arr[0].p = PolyFromCoeff(c);

    return result;
}

/**
 * Liczy ile różnych wykładników jest w sumie w dwóch
 * zadanych wielomianach.
 * @param[in] p : pierwszy wielomian
 * @param[in] q : drugi wielomian
 * @return liczba różnych wykładników
 */
static size_t HowManyDifferentExp(Poly *p, Poly *q) {
    size_t indP = 0;
    size_t indQ = 0;
    size_t result = 0;

    while (indP < p->size && indQ < q->size) {
        if (p->arr[indP].exp < q->arr[indQ].exp) {
            indP++;
        } else if (p->arr[indP].exp > q->arr[indQ].exp) {
            indQ++;
        } else {
            indP++;
            indQ++;
        }
        result++;
    }
    result += (p->size - indP);
    result += (q->size - indQ);

    return result;
}

/**
 * Kopiuje zawartość jednej tablicy od indeksu początkowego do jej końca
 * na koniec drugiej tablicy.
 * @param[in] polyStart : wielomian zawierający kopiowaną tablicę
 * @param[in] indStart : indeks od którego zaczynamy kopiowanie
 * @param[in] polyDest : wielomian zawierający tablicę do której kopiujemy
 * @param[in] indDest : indeks od którego zaczynamy kopiowanie do wynikowej tablicy
 */
static void CopyArrayIntoAnother(struct Poly polyStart, size_t *indStart,
                                 Poly* polyDest, size_t *indDest) {
    while (*indStart < polyStart.size) {
        polyDest->arr[*indDest].p = PolyClone(&(polyStart.arr[*indStart].p));
        polyDest->arr[*indDest].exp =
                polyStart.arr[*indStart].exp;
        (*indDest)++;
        (*indStart)++;
    }
}

/**
 * Sumuje dwa wielomiany i zapisuje wynik w wielomianie wynikowym
 * Uwzględnia to że wielomiany mogą mieć zarówno równe jak i różne
 * wykładniki przy kolejnych potęgach.
 * @param[in] poly1, poly2 : wielomiany które sumujemy
 * @param[in] ind1, ind2 : indeksy od których zaczynamy sumowanie
 * @param[in] result : wielomian wynikowy
 * @param[in] indRes : indeks w wielomianie wynikowym
 * @param[in] numOfDifferentElem : liczba różnych elementów w wynikowej tablicy
 */
static void AddTwoArraysTogether(const Poly poly1, size_t *ind1,
                                 const Poly poly2, size_t *ind2,
                                 Poly *result, size_t *indRes,
                                 size_t *numOfDifferentElem) {
    while (*ind1 < poly1.size && *ind2 < poly2.size) {
        if (poly1.arr[*ind1].exp < poly2.arr[*ind2].exp) {
            result->arr[*indRes].p = PolyClone(&(poly1.arr[*ind1].p));
            result->arr[*indRes].exp = poly1.arr[*ind1].exp;
            (*ind1)++;
            (*indRes)++;
        } else if (poly1.arr[*ind1].exp > poly2.arr[*ind2].exp) {
            result->arr[*indRes].p = PolyClone(&(poly2.arr[*ind2].p));
            result->arr[*indRes].exp = poly2.arr[*ind2].exp;
            (*ind2)++;
            (*indRes)++;
        } else {
            Poly temp = PolyAdd(&(poly1.arr[*ind1].p), &(poly2.arr[*ind2].p));
            if (!PolyIsZero(&temp)) {
                result->arr[*indRes].p = temp;
                result->arr[*indRes].exp = poly2.arr[*ind2].exp;
                (*indRes)++;
            } else {
                (*numOfDifferentElem)--;
                PolyDestroy(&temp);
            }
            (*ind2)++;
            (*ind1)++;
        }
    }
}
Poly PolyAdd(const Poly *p, const Poly *q) {
    Poly polyP;
    Poly polyQ;

    if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
        return PolyFromCoeff(p->coeff + q->coeff);
    }

    if (PolyIsCoeff(p)) {
        if (PolyIsZero(p)) {
            return PolyClone(q);
        }
        polyP = XToZeroFromCoeff(p->coeff);
        polyQ = PolyClone(q);

    } else if (PolyIsCoeff(q)) {
        if (PolyIsZero(q)) {
            return PolyClone(p);
        }
        polyQ = XToZeroFromCoeff(q->coeff);
        polyP = PolyClone(p);

    } else {
        polyP = PolyClone(p);
        polyQ = PolyClone(q);
    }

    size_t numOfDifferentElem = HowManyDifferentExp(&polyP, &polyQ);
    Poly result = PolyOfSizeN(numOfDifferentElem);

    size_t indP = 0;
    size_t indQ = 0;
    size_t indNew = 0;

    AddTwoArraysTogether(polyP, &indP, polyQ, &indQ, &result, &indNew,
                         &numOfDifferentElem);
    CopyArrayIntoAnother(polyP, &indP, &result, &indNew);
    CopyArrayIntoAnother(polyQ, &indQ, &result, &indNew);

    PolyDestroy(&polyQ);
    PolyDestroy(&polyP);

    if (numOfDifferentElem == 0) {
        free(result.arr);
        return PolyZero();
    }

    if (numOfDifferentElem == 1 && result.arr[0].exp == 0) {
        if (PolyIsCoeff(&(result.arr[0].p))) {
            result.coeff = result.arr[0].p.coeff;
            free(result.arr);
            result.arr = NULL;
            return result;
        }
    }

    result.size = numOfDifferentElem;
    return result;
}

/**
 * Sprawdza czy tworzony właśnie wielomian jest postaci @f$cx^0@f$.
 * @param[in] size : rozmiar tworzonego wielomianu
 * @param[in] newMonos : tablica jednomianów tworzonego wielomianu
 * @return prawda jeżeli wielomian będzie postaci @f$cx^0@f$,
 * fałsz w przeciwnym przypadku
 */
static bool IsCoeffTimesXToZero(size_t size, Mono* newMonos) {
    return size == 1 && newMonos[0].exp == 0 &&
           PolyIsCoeff(&(newMonos[0].p));
}

/**
 * Usuwa z pamięci tablicę jednomianów.
 * @param[in] arr : usuwana tablica jednomianów
 * @param[in] size : rozmiar usuwanej tablicy jednomianów
 */
static void MonoArrDestroy(Mono* arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        PolyDestroy(&(arr[i].p));
    }
    free(arr);
}

/**
 * Tworzy i zwraca płytką kopię tablicy podanej jako argument.
 * @param[in] monos : kopiowana tablica jednomianów
 * @param[in] count : rozmiar kopiowanej tablicy
 * @return płytka kopia tablicy jednomianów
 */
static Mono* CopyAndSortMonos(const Mono monos[], size_t count) {
    Mono* myMonos = malloc(count * sizeof(Mono));
    if (myMonos == NULL) {
        exit(1);
    }
    for (size_t i = 0; i < count; i++) {
        myMonos[i] = monos[i];
    }
    qsort(myMonos, count, sizeof(struct Mono), CompareMonos);
    return myMonos;
}

/**
 * Przepisuje tablicę jednomianów do nowej tablicy pozbywając się powtórzeń wielomianów
 * o tym samym wykładniku. Jeżeli w skutek dodawania wielomianów o tych samych wykładnika
 * otrzymywane jest zero, nie zapamiętuje ich.
 * @param[in] myMonos, count : tablica z powtórkami potęg wielomianów, jej rozmiar
 * @param[in] newMonos, newInd : tablica bez powtórzeń i indeks w niej
 */
static void GetRidOfSameExponents(Mono* myMonos, Mono* newMonos,
                                  size_t count, size_t *newInd) {
    Poly temp = PolyClone(&(myMonos[0].p));

    for (size_t i = 1; i < count; i++) {
        if (myMonos[i].exp == myMonos[i - 1].exp) {
            Poly temp2 = PolyAdd(&temp, &(myMonos[i].p));
            PolyDestroy(&temp);
            temp = PolyClone(&temp2);
            PolyDestroy(&temp2);
        } else {
            if (PolyIsZero(&temp)) {
                PolyDestroy(&temp);
                temp = PolyClone(&(myMonos[i].p));
                continue;
            }
            newMonos[*newInd].exp = myMonos[i - 1].exp;
            newMonos[*newInd].p = PolyClone(&temp);
            PolyDestroy(&temp);
            temp = PolyClone(&(myMonos[i].p));
            (*newInd)++;
        }
    }

    // ostatni element
    if (!PolyIsZero(&temp)) {
        newMonos[*newInd].exp = myMonos[count - 1].exp;
        newMonos[*newInd].p = PolyClone(&temp);
        (*newInd)++;
    }
    PolyDestroy(&temp);
}

Poly PolyAddMonos(size_t count, const Mono monos[]) {
    if (count == 0) {
        return PolyZero();
    }

    Mono* myMonos = CopyAndSortMonos(monos, count);
    Mono* newMonos = malloc(count * sizeof(Mono)); // wynikowa tablica
    if (newMonos == NULL) {
        exit(1);
    }
    size_t newInd = 0;

    if (count == 1) {
        if (PolyIsZero(&(myMonos[0].p))) {
            MonoArrDestroy(myMonos, count);
            MonoArrDestroy(newMonos, newInd);
            return PolyZero();
        }
        Poly newPoly = PolyClone(&(myMonos[0].p));
        newMonos[newInd] = MonoFromPoly(&newPoly, myMonos[0].exp);
        newInd++;
    } else {
        GetRidOfSameExponents(myMonos, newMonos, count, &newInd);
    }

    MonoArrDestroy(myMonos, count);

    Poly result;
    if (newInd == 0) {
        MonoArrDestroy(newMonos, newInd);
        return PolyZero();
    }

    if (IsCoeffTimesXToZero(newInd, newMonos)) {
        result.coeff = newMonos[0].p.coeff;
        PolyDestroy(&(newMonos[0].p));
        free(newMonos);
        result.arr = NULL;
        return result;
    }

    result.arr = newMonos;
    result.size = newInd;

    return result;
}

Poly PolyNeg(const Poly *p) {
    if (PolyIsCoeff(p)) {
        return PolyFromCoeff((-1) * p->coeff);
    }

    Poly result = PolyOfSizeN(p->size);
    for (size_t i = 0; i < p->size; i++) {
        Poly negatedPoly = PolyNeg(&p->arr[i].p);
        result.arr[i] = MonoFromPoly(&(negatedPoly), p->arr[i].exp);
    }
    return result;
}

Poly PolySub(const Poly *p, const Poly *q) {
    Poly helperQ = PolyNeg(q);
    Poly result = PolyAdd(p, &helperQ);
    PolyDestroy(&helperQ);
    return result;
}

/**
 * Funkcja rekurencyjna obliczająca największy wykładnik przy zadanej zmiennej.
 * @param[in] p : wielomian dla którego liczymy najwyższy wykładnik przy danej zmiennej
 * @param[in] varIdx : zmienna dla której liczmy maksymalny występujący wykładnik
 * @param[in] countRecursion : aktualna głębokość wywołania rekurencyjnego
 * @param[in] maxExpIdx : szukana maksymalna wartość wykładnika
 */
static void PolyDegByHelper(const Poly *p, size_t varIdx,
                            size_t countRecursion, poly_exp_t* maxExpIdx) {
    if (varIdx > countRecursion) {
        if (!PolyIsCoeff(p)) {
            for (size_t i = 0; i < p->size; i++) {
                PolyDegByHelper(&(p->arr[i].p), varIdx,
                                countRecursion + 1, maxExpIdx);
            }
        } else {
            *maxExpIdx = 0;
        }
    } else if (varIdx == countRecursion) {
        if (!PolyIsCoeff(p)) {
            poly_exp_t maxExp = 0;
            for (size_t i = 0; i < p->size; i++) {
                if (p->arr[i].exp > maxExp) {
                    maxExp = p->arr[i].exp;
                }
            }
            if (maxExp > *maxExpIdx) {
                *maxExpIdx = maxExp;
            }
        }
    }
}

poly_exp_t PolyDegBy(const Poly *p, size_t varIdx) {
    if (PolyIsZero(p)) {
        return -1;
    }
    poly_exp_t maxExpIdx = 0;
    PolyDegByHelper(p, varIdx, 0, &maxExpIdx);
    return maxExpIdx;
}

/**
 * Funkcja rekurencyjna obliczająca największy stopień wielomianu dla wszystkich zmiennych.
 * @param[in] p : wielomian dla którego liczymy najwyższy wykładnik
 * @param[in] tempMaxExp : obliczony największy do tej pory wykładnik
 * @param[in] maxExp : wynikowy największy wykładnik wielomianu
 */
static void PolyGetMaxExp(const Poly *p, poly_exp_t tempMaxExp,
                          poly_exp_t* maxExp) {
    if (!PolyIsCoeff(p)) {
        for (size_t i = 0; i < p->size; i++) {
            PolyGetMaxExp(&(p->arr[i].p),
                          tempMaxExp + p->arr[i].exp, maxExp);
        }
    } else {
        if (tempMaxExp > *maxExp) {
            *maxExp = tempMaxExp;
        }
    }
}

poly_exp_t PolyDeg(const Poly *p) {
    if (PolyIsZero(p)) {
        return -1;
    }
    poly_exp_t maxExp = 0;
    PolyGetMaxExp(p, 0, &maxExp);
    return maxExp;
}

bool PolyIsEq(const Poly *p, const Poly *q) {
    if ((PolyIsCoeff(p) && !PolyIsCoeff(q)) ||
        (!PolyIsCoeff(p) && PolyIsCoeff(q))) {
        return false;
    }
    if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
        return p->coeff == q->coeff;
    }
    if (p->size != q->size) {
        return false;
    }

    size_t numberOfX = p->size;
    for (size_t i = 0; i < numberOfX; i++) {
        if (p->arr[i].exp != q->arr[i].exp) {
            return false;
        }
    }
    for (size_t i = 0; i < numberOfX; i++) {
        if (!PolyIsEq(&(p->arr[i].p), &(q->arr[i].p))) {
            return false;
        }
    }
    return true;
}

/**
 * Mnoży wielomian przez współczynnik.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] c : współczynnik przez który mnożymy
 * @return @f$c * q@f$
 */
static Poly PolyMulByCoeff(const Poly *p, poly_coeff_t c) {
    if (c == 0) {
        return PolyZero();
    }
    if (PolyIsCoeff(p)) {
        return PolyFromCoeff(p->coeff * c);
    } else {
        Poly result = PolyOfSizeN(p->size);
        poly_coeff_t indResult = 0;

        for (size_t i = 0; i < p->size; i++) {
            Poly tempResult = PolyMulByCoeff(&(p->arr[i].p), c);
            if (!PolyIsZero(&tempResult)) {
                result.arr[indResult] =
                        MonoFromPoly(&tempResult, p->arr[i].exp);
                indResult++;
            }
        }

        if (indResult == 0) {
            free(result.arr);
            return PolyZero();
        } else {
            result.size = indResult;
            return result;
        }
    }
}

Poly PolyAt(const Poly *p, poly_coeff_t x) {
    if (PolyIsCoeff(p)) {
        return PolyClone(p);
    }

    Poly result = PolyZero();
    for (size_t i = 0; i < p->size; i++) {
        poly_coeff_t mulBy = Exponentiation(x, p->arr[i].exp);
        Poly temp = PolyMulByCoeff(&(p->arr[i].p), mulBy);

        Poly result2 = PolyAdd(&result, &temp);
        PolyDestroy(&result);
        result = PolyClone(&result2);
        PolyDestroy(&result2);

        PolyDestroy(&temp);
    }

    return result;
}

/**
 * Funkcja pomocnicza mnożąca dwa wielomiany które nie
 * są współczynnikami.
 * @param[in] poly1 : wielomian @f$p@f$
 * @param[in] poly2 : wielomian @f$q@f$
 * @param[in] temp : zwracany wynik
 * @return @f$p * q@f$
 */
static Poly MulTwoPolys(const Poly *poly1, const Poly *poly2, Poly *temp) {
    for (size_t i = 0; i < poly1->size; i++) {
        for (size_t j = 0; j < poly2->size; j++) {
            Poly singlePolyCoeff = PolyMul(&(poly1->arr[i].p), &(poly2->arr[j].p));
            poly_exp_t newExp = poly1->arr[i].exp + poly2->arr[j].exp;

            Poly singlePoly;
            if (PolyIsCoeff(poly1) && PolyIsCoeff(poly2)) {
                singlePoly = PolyFromCoeff(singlePolyCoeff.coeff);
            } else {
                singlePoly.arr = malloc(sizeof(Mono));
                if (singlePoly.arr == NULL) {
                    exit(1);
                }
                singlePoly.size = 1;
                singlePoly.arr[0] = MonoFromPoly(&singlePolyCoeff, newExp);
            }

            Poly tempHelp = PolyAdd(temp, &singlePoly);
            PolyDestroy(temp);
            PolyDestroy(&singlePoly);
            *temp = PolyClone(&tempHelp);
            PolyDestroy(&tempHelp);
        }
    }
    return *temp;
}

Poly PolyMul(const Poly *p, const Poly *q) {
    if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
        return PolyFromCoeff(p->coeff * q->coeff);
    }
    if (PolyIsCoeff(p)) {
        return PolyMulByCoeff(q, p->coeff);
    }
    if (PolyIsCoeff(q)) {
        return PolyMulByCoeff(p, q->coeff);
    }

    Poly temp = PolyZero();
    MulTwoPolys(p, q, &temp);
    return temp;
}


