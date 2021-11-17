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
 * Dowolny wielomian podniesiony do potęgi zero,
 * używane w algorytmie szybkiego potęgowania wielomianów.
 */
#define START_POLY_EXP_VALUE PolyFromCoeff(1)

/**
 * Rozmiar wielomianu postaci @f$cx^0@f$.
 */
#define SINGLE_SIZE 1

/**
 * Zwraca liczbę podniesioną do danej potęgi.
 * @param[in, out] basis : baza potęgowania
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
static poly_exp_t CompareMonos(const void* A, const void* B) {
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
 * Liczy ile różnych wykładników jest w sumie w dwóch
 * zadanych wielomianach.
 * @param[in] p : pierwszy wielomian
 * @param[in] q : drugi wielomian
 * @return liczba różnych wykładników
 */
static size_t HowManyDifferentExp(const Poly *p, const Poly *q) {
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
 * @param[out] polyDest : wielomian zawierający tablicę do której kopiujemy
 * @param[out] indDest : indeks od którego zaczynamy kopiowanie do wynikowej tablicy
 */
static void DeepCopyArrayIntoAnother(const Poly* polyStart, size_t indStart,
                                     Poly* polyDest, size_t indDest) {
    while (indStart < polyStart->size) {
        polyDest->arr[indDest].p = PolyClone(&(polyStart->arr[indStart].p));
        polyDest->arr[indDest].exp = polyStart->arr[indStart].exp;
        indDest++;
        indStart++;
    }
}

/**
 * Sumuje dwa wielomiany i zapisuje wynik w wielomianie wynikowym.
 * Uwzględnia to, że wielomiany mogą mieć zarówno równe, jak i różne
 * wykładniki przy kolejnych potęgach.
 * @param[in] poly1, poly2 : wielomiany które sumujemy
 * @param[in, out] ind1, ind2 : indeksy, od których zaczynamy sumowanie
 * @param[out] result : wielomian wynikowy
 * @param[out] indRes : indeks w wielomianie wynikowym
 * @param[out] numOfDifferentElem : liczba różnych elementów w wynikowej tablicy
 */
static void AddTwoArraysTogether(const Poly poly1, size_t *ind1, const Poly poly2, size_t *ind2,
                                 Poly *result, size_t *indRes, size_t *numOfDifferentElem) {
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
            Poly sum = PolyAdd(&(poly1.arr[*ind1].p), &(poly2.arr[*ind2].p));
            if (!PolyIsZero(&sum)) {
                result->arr[*indRes].p = sum;
                result->arr[*indRes].exp = poly2.arr[*ind2].exp;
                (*indRes)++;
            } else {
                (*numOfDifferentElem)--;
                PolyDestroy(&sum);
            }
            (*ind2)++;
            (*ind1)++;
        }
    }
}

/**
 * Sumuje dwa wielomiany będące współczynnikami.
 * @param[in] p, q : wielomiany które sumujemy
 * @return suma wielomianów
 */
static Poly AddTwoCoeff(const Poly *p, const Poly *q) {
    return PolyFromCoeff(p->coeff + q->coeff);
}

/**
 * Sumuje wielomian będący współczynnikiem oraz niebędący współczynnikiem.
 * @param[in] polyCoeff, polyNonCoeff : wielomiany które sumujemy
 * @return suma wielomianów
 */
static Poly AddCoeffNonCoeff(const Poly *polyCoeff, const Poly *polyNonCoeff) {
    if (PolyIsZero(polyCoeff)) {
        return PolyClone(polyNonCoeff);
    }

    Poly result;
    if (polyNonCoeff->arr[0].exp == 0) {
        Poly sumAtFirstInd = PolyAdd(&polyNonCoeff->arr[0].p, polyCoeff);
        if (!PolyIsZero(&sumAtFirstInd)) {
            result = PolyOfSizeN(polyNonCoeff->size);
            result.arr[0] = MonoFromPoly(&sumAtFirstInd, 0);
            DeepCopyArrayIntoAnother(polyNonCoeff, 1, &result, 1);
        } else {
            result = PolyOfSizeN(polyNonCoeff->size - 1);
            DeepCopyArrayIntoAnother(polyNonCoeff, 1, &result, 0);
        }
    } else {
        result = PolyOfSizeN(polyNonCoeff->size + 1);
        Poly polyAtFirstInd = PolyClone(polyCoeff);
        result.arr[0] = MonoFromPoly(&polyAtFirstInd, 0);
        DeepCopyArrayIntoAnother(polyNonCoeff, 0, &result, 1);
    }
    return result;
}

Poly PolyAdd(const Poly *p, const Poly *q) {
    if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
        return AddTwoCoeff(p, q);
    }
    if (PolyIsCoeff(p)) {
        return AddCoeffNonCoeff(p, q);
    }
    if (PolyIsCoeff(q)) {
        return AddCoeffNonCoeff(q, p);
    }

    size_t numOfDifferentElem = HowManyDifferentExp(p, q);
    Poly result = PolyOfSizeN(numOfDifferentElem);

    size_t indP = 0;
    size_t indQ = 0;
    size_t indNew = 0;

    AddTwoArraysTogether(*p, &indP, *q, &indQ, &result, &indNew,
                         &numOfDifferentElem);
    DeepCopyArrayIntoAnother(p, indP, &result, indNew);
    DeepCopyArrayIntoAnother(q, indQ, &result, indNew);

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
 * Sprawdza, czy tablica jednomianów utworzy wielomian postaci @f$cx^0@f$.
 * @param[in, out] size : rozmiar tworzonego wielomianu
 * @param[in] newMonos : tablica jednomianów tworzonego wielomianu
 * @return Prawda jeżeli wielomian będzie postaci @f$cx^0@f$,
 * fałsz w przeciwnym przypadku.
 */
static bool IsCoeffTimesXToZero(size_t size, Mono const* newMonos) {
    return size == 1 && newMonos[0].exp == 0 &&
           PolyIsCoeff(&(newMonos[0].p));
}

/**
 * Usuwa z pamięci tablicę jednomianów.
 * @param[out] arr : usuwana tablica jednomianów
 * @param[in] size : rozmiar usuwanej tablicy jednomianów
 */
static void MonoArrDestroy(Mono* arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        PolyDestroy(&(arr[i].p));
    }
    free(arr);
}

/**
 * Sortuje tablice jednomianów rosnąco według wykładników.
 * @param[out] monoArray : sortowana tablica
 * @param[in] count : rozmiar sortowanej tablicy jednomianów
 */
static void SortMonos(Mono* monoArray, size_t count) {
    qsort(monoArray, count, sizeof(struct Mono), CompareMonos);
}

/**
 * Tworzy i zwraca w wyniku płytką kopię tablicy jednomianów.
 * @param[in] monos : kopiowana tablica jednomianów
 * @param[in] count : rozmiar kopiowanej tablicy jednomianów
 */
static Mono* ShallowCopyOfMonoArr(const Mono* monos, size_t count) {
    Mono* myMonos = malloc(count * sizeof(Mono));
    if (myMonos == NULL) {
        exit(1);
    }
    for (size_t i = 0; i < count; i++) {
        myMonos[i] = monos[i];
    }
    return myMonos;
}

/**
 * Tworzy i zwraca w wyniku głęboką kopię tablicy jednomianów.
 * @param[in] monos : kopiowana tablica jednomianów
 * @param[in] count : rozmiar kopiowanej tablicy jednomianów
 */
static Mono* DeepCopyofMonoArr(const Mono* monos, size_t count) {
    Mono* myMonos = malloc(count * sizeof(Mono));
    if (myMonos == NULL) {
        exit(1);
    }
    for (size_t i = 0; i < count; i++) {
        myMonos[i] = MonoClone(&(monos[i]));
    }
    return myMonos;
}

/**
 * Na podstawie tablicy myMonos wypełnia kolejną tablicę,
 * w której zsumowane są jednomiany o tych samych potęgach.
 * @param[in] myMonos : tablica jednomianów mogąca zawierać kilka jednomianów o tym samym wykładniku
 * @param[out] newMonos : tablica jednomianów bez powtórzonych wykładników
 * @param[in] count : rozmiar początkowej tablicy jednomianów
 * @param[in, out] newInd : indeks wynikowej tablicy
 */
static void DeleteSameExponents(Mono* myMonos, Mono* newMonos,
                                size_t count, size_t *newInd) {
    if (count == 1) {
        newMonos[*newInd] = myMonos[0];
        (*newInd)++;
    } else {
        Poly sum = myMonos[0].p;

        for (size_t i = 1; i < count; i++) {
            if (myMonos[i].exp == myMonos[i - 1].exp) {
                Poly sum2 = PolyAdd(&sum, &(myMonos[i].p));
                PolyDestroy(&sum);
                PolyDestroy(&(myMonos[i].p));
                sum = sum2;
            } else {
                if (PolyIsZero(&sum)) {
                    PolyDestroy(&sum);
                    sum = myMonos[i].p;
                    continue;
                }
                newMonos[*newInd].exp = myMonos[i - 1].exp;
                newMonos[*newInd].p = sum;
                sum = myMonos[i].p;
                (*newInd)++;
            }
        }

        // Algorytm nie uwzględnia rozpatrywania ostatnich sumowanych elementów,
        // są one uwzględniane w tym miejscu.
        if (!PolyIsZero(&sum)) {
            newMonos[*newInd].exp = myMonos[count - 1].exp;
            newMonos[*newInd].p = sum;
            (*newInd)++;
        } else {
            PolyDestroy(&sum);
        }
    }
    free(myMonos);
}

/**
 * Tworzy wielomian na podstawie tablicy jednomianów. Przejmuje na własnośc tablicę oraz jej zawartość.
 * @param[in] count : rozmiar początkowej tablicy jednomianów
 * @param[in] myMonos : tablica jednomianów z której powstaje wielomian
 * @return wielomian utworzony z początkowej tablicy
 */
static Poly PolyCreateFromMonos(size_t count, Mono* myMonos) {
    if (count == 0 || myMonos == NULL) {
        return PolyZero();
    }

    // Wielomian postaci 0x^k.
    if (count == 1 && PolyIsZero(&(myMonos[0].p))) {
        MonoArrDestroy(myMonos, count);
        return PolyZero();
    }

    SortMonos(myMonos, count);

    // Wynikowa tablica.
    Mono* newMonos = malloc(count * sizeof(Mono));
    if (newMonos == NULL) {
        exit(1);
    }
    size_t newInd = 0;
    // Zwalnia myMonos, w newMonos umieszcza oczekiwany wynik.
    DeleteSameExponents(myMonos, newMonos, count, &newInd);

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

Poly PolyOwnMonos(size_t count, Mono* monos) {
    Mono* myMonos = monos;
    return PolyCreateFromMonos(count, myMonos);
}

Poly PolyCloneMonos(size_t count, const Mono* monos) {
    Mono* myMonos = DeepCopyofMonoArr(monos, count);
    return PolyCreateFromMonos(count, myMonos);
}

Poly PolyAddMonos(size_t count, const Mono* monos) {
    Mono* myMonos = ShallowCopyOfMonoArr(monos, count);
    return PolyCreateFromMonos(count, myMonos);
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
 * @param[in, out] currentMaxExp : obliczony największy do tej pory wykładnik
 * @param[out] maxExp : wynikowy największy wykładnik wielomianu
 */
static void PolyGetMaxExp(const Poly *p, poly_exp_t currentMaxExp, poly_exp_t* maxExp) {
    if (!PolyIsCoeff(p)) {
        for (size_t i = 0; i < p->size; i++) {
            PolyGetMaxExp(&(p->arr[i].p),currentMaxExp + p->arr[i].exp, maxExp);
        }
    } else {
        if (currentMaxExp > *maxExp) {
            *maxExp = currentMaxExp;
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
            Poly partialResult = PolyMulByCoeff(&(p->arr[i].p), c);
            if (!PolyIsZero(&partialResult)) {
                result.arr[indResult] = MonoFromPoly(&partialResult, p->arr[i].exp);
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
        Poly partialResult = PolyMulByCoeff(&(p->arr[i].p), mulBy);

        Poly result2 = PolyAdd(&result, &partialResult);
        PolyDestroy(&result);
        result = PolyClone(&result2);
        PolyDestroy(&result2);

        PolyDestroy(&partialResult);
    }

    return result;
}

/**
 * Funkcja pomocnicza mnożąca dwa wielomiany które nie
 * są współczynnikami.
 * @param[in] poly1 : wielomian @f$p@f$
 * @param[in] poly2 : wielomian @f$q@f$
 * @param[in] result : zwracany wynik
 * @return @f$p * q@f$
 */
static Poly MulTwoPolys(const Poly *poly1, const Poly *poly2, Poly *result) {
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

            Poly resultHelp = PolyAdd(result, &singlePoly);
            PolyDestroy(result);
            PolyDestroy(&singlePoly);
            *result = PolyClone(&resultHelp);
            PolyDestroy(&resultHelp);
        }
    }
    return *result;
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

    Poly result = PolyZero();
    MulTwoPolys(p, q, &result);
    return result;
}

/**
 * Rekurencyjne szybkie potęgowanie wielomianów.
 * @param[in] basis : wielomian podnoszony do potęgi
 * @param[in] exp : potęga
 * @return @f$basis ^ exp@f$
 */
static Poly PolyExpBySquaring(const Poly* basis, poly_exp_t exp) {
    if (exp == 0) {
        return START_POLY_EXP_VALUE;
    }
    if (PolyIsZero(basis)) {
        return PolyZero();
    }
    if (PolyIsCoeff(basis)) {
        return PolyFromCoeff(Exponentiation(basis->coeff, exp));
    }
    Poly sqrtResult = PolyExpBySquaring(basis, exp / 2);
    Poly result = PolyMul(&sqrtResult, &sqrtResult);

    if (exp % 2 == 1) {
        Poly partialResult = PolyMul(&result, basis);
        PolyDestroy(&result);
        result = partialResult;
    }

    PolyDestroy(&sqrtResult);
    return result;
}

/**
 * Rekurencyjna funkcja pomocnicza obliczająca wynik operacji podstawiania k wielomianów
 * z tablicy q pod zmienne danego wielomianu p,
 * zależnie od aktualnego poziomu zagnieżdżenia rekurencji.
 * @param[in] p : wielomian @f$p(x_0, x_1, \ldots, x_{l-1})@f$
 * @param[in] k : liczba wielomianów podstawianych pod zmienne
 * @param[in] q : tablica wielomianów które zostaną podstawione
 * @param[in, out] recurrenceLevel : aktualny poziom zagnieżdżenia rekurencji
 * @return @f$p(q_0, q_1, \ldots)@f$
 * */
static Poly PolyComposeHelper(const Poly *p, size_t k, const Poly q[], size_t recurrenceLevel) {
    if (PolyIsCoeff(p)) {
        return PolyClone(p);
    }

    Poly result = PolyZero();
    for (size_t i = 0; i < p->size; i++) {
        Poly composedCoeff = PolyComposeHelper(&(p->arr[i].p), k, q, recurrenceLevel + 1);
        Poly substitutedVar;
        if (recurrenceLevel >= k) {
            Poly polyZero = PolyZero();
            substitutedVar = PolyExpBySquaring(&polyZero, p->arr[i].exp);
            PolyDestroy(&polyZero);
        } else {
            substitutedVar = PolyExpBySquaring(&(q[recurrenceLevel]), p->arr[i].exp);
        }

        Poly composedMono = PolyMul(&composedCoeff, &substitutedVar);
        PolyDestroy(&composedCoeff);
        PolyDestroy(&substitutedVar);

        Poly partialResult = PolyAdd(&result, &composedMono);
        PolyDestroy(&result);
        result = partialResult;

        PolyDestroy(&composedMono);
    }
    return result;
}

Poly PolyCompose(const Poly *p, size_t k, const Poly* q) {
    return PolyComposeHelper(p, k, q, 0);
}