/** @file
 *  Kalkulator operacji na wielomianach rzadkich wielu zmiennych
 *  @author Patrycja Stępień
*/

/**
 * Makro potrzebne do prawidłowego wykonywania funkcji getline
 */
#define _GNU_SOURCE

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stack.h"
#include "read_input.h"
#include "instruction_scan.h"
#include "poly_execute.h"

/**
 * Początek numerowania wierszy
 */
#define START_COUNT 1

/**
 * Sprawdza, czy znak jest małą lub wielką literą alfabetu angielskiego.
 * @param[in] c : sprawdzany znak
 * @return czy znak jest literą
 */
static bool IsCharLetter(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

/**
 * Sprawdza, czy na wejściu pojawił się komentarz.
 * @param[in] c : pierwszy znak nowego wiersza
 * @return czy wczytywana linia jest komentarzem
 */
static bool IsComment(char c) {
    return c == '#';
}

/**
 * Ignoruje komentarz
 */
static void SkipComment() {
    int sign = getchar();
    while (sign != '\n') {
        sign = getchar();
    }
}

void ReadInput(Stack* stack) {
    int firstSign = getchar();
    int lineNumber = START_COUNT;

    while (firstSign != EOF) {
        if (IsComment(firstSign)) {
            SkipComment();
        } else if (IsCharLetter(firstSign)) {
            ungetc(firstSign, stdin);
            InstructionScan(stack, lineNumber);
        } else if (firstSign == '\n') {
        } else {
            ungetc(firstSign, stdin);
            PolyScan(stack, lineNumber);
        }
        firstSign = getchar();
        lineNumber++;
    }
}