/** @file
 *  Kalkulator operacji na wielomianach rzadkich wielu zmiennych
 *  @author Patrycja Stępień
*/
#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "stack.h"
#include "poly.h"

/**
 * Typy operacji dwuargumentowych.
 */
enum TwoArgOp {
    // Używam małych liter, ponieważ nazwy operacji pisane wielkimi literami
    // są zarezerwowane jako nazwy funkcji.
    add,
    sub,
    mul
};

/**
 * Usuwa wielomian z wierzchołka stosu.
 * @param[in, out] stack : stos
 * @param[in] lineNumber : numer wykonywanego wiersza
 */
void POP(Stack* stack, int lineNumber);

/**
 * Wypisuje na standardowe wyjście wielomian z wierzchołka stosu.
 * @param[in] stack : stos
 * @param[in] lineNumber : numer wykonywanego wiersza
 */
void PRINT(Stack* stack, int lineNumber);

/**
 * Dodaje dwa wielomiany z wierzchu stosu, usuwa je i wstawia na wierzchołek stosu ich sumę.
 * @param[in, out] stack : stos
 * @param[in] lineNumber : numer wykonywanego wiersza
 */
void ADD(Stack* stack, int lineNumber);

/**
 * Odejmuje od wielomianu z wierzchołka wielomian pod wierzchołkiem,
 * usuwa je i wstawia na wierzchołek stosu różnicę.
 * @param[in, out] stack : stos
 * @param[in] lineNumber : numer wykonywanego wiersza
 */
void SUB(Stack* stack, int lineNumber);

/**
 * Mnoży dwa wielomiany z wierzchu stosu, usuwa je i wstawia na wierzchołek stosu ich iloczyn.
 * @param[in, out] stack : stos
 * @param[in] lineNumber : numer wykonywanego wiersza
 */
void MUL(Stack* stack, int lineNumber);

/**
 * Sprawdza, czy dwa wielomiany na wierzchu stosu są równe
 * – wypisuje na standardowe wyjście 0 lub 1.
 * @param[in] stack : stos
 * @param[in] lineNumber : numer wykonywanego wiersza
 */
void IS_EQ(Stack* stack, int lineNumber);

/**
 * Wypisuje na standardowe wyjście stopień wielomianu
 * (−1 dla wielomianu tożsamościowo równego zeru).
 * @param[in] stack : stos
 * @param[in] lineNumber : numer wykonywanego wiersza
 */
void DEG(Stack* stack, int lineNumber);

/**
 * Wypisuje na standardowe wyjście stopień wielomianu ze względu na zmienną o numerze idx
 * (−1 dla wielomianu tożsamościowo równego zeru).
 * @param[in] stack : stos
 * @param[in] idx : zmienna, ze względu na którą wypisywany jest stopień wielomianu
 * @param[in] lineNumber : numer wykonywanego wiersza
 */
void DEG_BY(Stack* stack, size_t idx, int lineNumber);

/**
 * Wylicza wartość wielomianu w punkcie x,
 * usuwa wielomian z wierzchołka i wstawia na stos wynik operacji.
 * @param[in, out] stack : stos
 * @param[in] x : punkt, dla którego liczona jest wartość wielomianu
 * @param[in] lineNumber : numer wykonywanego wiersza
 */
void AT(Stack* stack, poly_coeff_t x, int lineNumber);

/**
 * Neguje wielomian na wierzchołku stosu.
 * @param[in, out] stack : stos
 * @param[in] lineNumber : numer wykonywanego wiersza
 */
void NEG(Stack* stack, int lineNumber);

/**
 * Wstawia na wierzchołek stosu wielomian tożsamościowo równy zeru.
 * @param[in] stack : stos
 */
void ZERO(Stack* stack);

/**
 * Sprawdza, czy wielomian na wierzchołku stosu jest współczynnikiem
 * – wypisuje na standardowe wyjście 0 lub 1.
 * @param[in] stack : stos
 * @param[in] lineNumber : numer wykonywanego wiersza
 */
void IS_COEFF(const Stack* stack, int lineNumber);

/**
 * Sprawdza, czy wielomian na wierzchołku stosu jest tożsamościowo równy zeru
 * – wypisuje na standardowe wyjście 0 lub 1.
 * @param[in] stack : stos
 * @param[in] lineNumber : numer wykonywanego wiersza
 */
void IS_ZERO(const Stack* stack, int lineNumber);

/**
 * Wstawia na stos kopię wielomianu z wierzchołka.
 * @param[in] stack : stos
 * @param[in] lineNumber : numer wykonywanego wiersza
 */
void CLONE(Stack* stack, int lineNumber);

/**
 * Pod l zmiennych wielomianu p ze szczytu stosu, oznaczonych jako @f$x_0@f$, @f$x_1@f$, ..., @f$x_{l-1}@f$,
 * podstawia k kolejnych wielomianów ze stosu,
 * oznaczonych jako @f$q_0@f$, @f$q_1@f$, ..., @f$q_{k-1}@f$,.
 * Jeśli @f$k < l@f$ pod zmienne @f$x_k@f$, ..., @f$x_{l-1}@f$ podstawiane są zera.
 * Zdejmuje ze stosu wymienione wielomiany
 * i wstawia na wierzchołek stosu wynik operacji składania wielomianów.
 * @param[in, out] stack : stos
 * @param[in] lineNumber : numer wykonywanego wiersza
 * @param[in] k : liczba wielomianów podstawianych pod zmienne
 */
void COMPOSE (Stack* stack, int lineNumber, size_t k);

#endif /* INSTRUCTIONS_H */