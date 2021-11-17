/** @file Kalkulator operacji na wielomianach rzadkich wielu zmiennych
 *  @author Patrycja Stępień
*/

#define _GNU_SOURCE
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stack.h"
#include "instruction_scan.h"
#include "instructions.h"
#include "tools.h"

/**
 * Spacja, używane do podziału instrukcji na polecenie i parametr.
 */
#define SPACE ' '

/**
 * Baza systemu dziesiętnego.
 */
#define DECIMAL_BASE 10

/**
 * Długość polecenia 'AT', potrzebne do wycinania napisu.
 */
#define AT_LENGTH 2

/**
 * Długość polecenia 'DEG_BY', potrzebne do wycinania napisu.
 */
#define DEG_BY_LENGTH 6

/**
 * Wykonuje instrukcje bez parametrów.
 * @param[in] stack : stos
 * @param[in] lineNumber : numer wczytanego wiersza
 * @param[in] instruction : napis zawierający instrukcję do wykonania
 * @param[in] lineSize : rozmiar wiersza z instrukcją
 */
static void ExecuteInstruction(Stack* stack, int lineNumber, char* instruction, size_t lineSize) {
    if (instruction[lineSize - 1] != '\n') {
        instruction[lineSize] = '\n';
    }

    if (memcmp(instruction, "ADD\n", lineSize + 1) == 0) {
        ADD(stack, lineNumber);
    } else if (memcmp(instruction, "ZERO\n", lineSize + 1) == 0) {
        ZERO(stack);
    } else if (memcmp(instruction, "SUB\n", lineSize + 1) == 0) {
        SUB(stack, lineNumber);
    } else if (memcmp(instruction, "MUL\n", lineSize + 1) == 0) {
        MUL(stack, lineNumber);
    } else if (memcmp(instruction, "IS_COEFF\n", lineSize + 1) == 0) {
        IS_COEFF(stack, lineNumber);
    } else if (memcmp(instruction, "IS_ZERO\n", lineSize + 1) == 0) {
        IS_ZERO(stack, lineNumber);
    } else if (memcmp(instruction, "NEG\n", lineSize + 1) == 0) {
        NEG(stack, lineNumber);
    } else if (memcmp(instruction, "IS_EQ\n", lineSize + 1) == 0) {
        IS_EQ(stack, lineNumber);
    } else if (memcmp(instruction, "DEG\n", lineSize + 1) == 0) {
        DEG(stack, lineNumber);
    } else if (memcmp(instruction, "POP\n", lineSize + 1) == 0) {
        POP(stack, lineNumber);
    } else if (memcmp(instruction, "PRINT\n", lineSize + 1) == 0) {
        PRINT(stack, lineNumber);
    } else if (memcmp(instruction, "CLONE\n", lineSize + 1) == 0) {
        CLONE(stack, lineNumber);
    } else {
        if (memcmp(instruction, "DEG_BY\n", lineSize + 1) == 0) {
            fprintf(stderr, "ERROR %u DEG BY WRONG VARIABLE\n", lineNumber);
        } else if (memcmp(instruction, "AT\n", lineSize + 1) == 0) {
            fprintf(stderr, "ERROR %u AT WRONG VALUE\n", lineNumber);
        } else {
            fprintf(stderr, "ERROR %u WRONG COMMAND\n", lineNumber);
        }
        return;
    }
}

/**
 * Sprawdza czy wczytane polecenie jest poleceniem z parametrem
 * sprawdzając czy zawiera ono spację.
 * @param[in] line : wiersz z poleceniem
 * @param[in] lineSize : rozmiar wiersza z poleceniem
 * @return czy wczytany wiersz zawiera spację
 */
static bool DoesLineContainSpace(char* line, size_t lineSize) {
    for (size_t i = 0; i < lineSize; i++) {
        if (line[i] == SPACE) {
            return true;
        }
    }
    return false;
}

/**
 * Znajduje indeks spacji odddzielającej polecenie od parametru.
 * @param[in] line : wiersz z poleceniem
 * @param[in] lineSize : rozmiar wiersza z poleceniem
 * @return indeks pod którym występuje spacja
 */
static size_t FindSpace(char* line, size_t lineSize) {
    for (size_t i = 0; i < lineSize; i++) {
        if (line[i] == SPACE) {
            return i;
        }
    }
    return 0;
}

/**
 * Sprawdza czy po parametrze polecenia DEG_BY nie występują zbędne znaki.
 * @param[in] line : wiersz z poleceniem
 * @param[in] lineSize : rozmiar wiersza z poleceniem
 * @param[in] spaceInd : indeks spacji, początek sprawdzania
 * @return czy linia nie zawierała błędnych znaków
 */
static bool IsCorrectDegByEnd(char* line, size_t lineSize, size_t spaceInd) {
    // po spacji powinny wystepowac tylko cyfry
    // jedyna dozwolona sytuacja to znak nowej linii na koncu
    for (size_t i = spaceInd + 1; i < lineSize; i++) {
        if (!isDigit(line[i])) {
            if (!(i == lineSize - 1 && line[i] == '\n')) {
                return false;
            }
        }
    }
    return true;
}

/**
 * Sprawdza czy po parametrze polecenia AT nie występują zbędne znaki.
 * @param[in] line : wiersz z poleceniem
 * @param[in] lineSize : rozmiar wiersza z poleceniem
 * @param[in] spaceInd : indeks spacji, początek sprawdzania
 * @return czy linia nie zawierała błędnych znaków
 */
static bool IsCorrectAtEnd(char* line, size_t lineSize, size_t spaceInd) {
    // po spacji powinny wystepowac tylko cyfry
    // jedyna dozwolona sytuacja to znak nowej linii na koncu
    for (size_t i = spaceInd + 1; i < lineSize; i++) {
        if (line[i] == '-') {
            if (i != (spaceInd + 1)) {
                return false;
            }
        } else {
            if (!isDigit(line[i])) {
                if (!(i == lineSize - 1 && line[i] == '\n')) {
                    return false;
                }
            }
        }
    }
    return true;
}

/**
 * W przypadku wystąpienia błędu przy parsowaniu poleceniem z argumentem
 * kończy działanie funkcji ExecuteInstructionWithParametr zwalniając
 * pamięć i wypisując komunikat o błędzie
 * @param[in] message : komunikat o błędzie
 * @param[in] lineNumber : numer aktualnie wczytywanej linii
 * @param[in] line : wiersz z wczytanym poleceniem
 * @param[in] lineSize : długość wiersza z wczytanym poleceniem
 */
static void EndExecution(char* message, char* instruction, char* parametr, int lineNumber) {
    fprintf(stderr, message, lineNumber);
    free(instruction);
    free(parametr);
}

/**
 * Wykonuje polecenia z parametrem - AT oraz DEG_BY
 * @param[in] stack : stos
 * @param[in] lineNumber : numer aktualnie wczytywanej linii
 * @param[in] line : wiersz z wczytanym poleceniem
 * @param[in] lineSize : długość wiersza z wczytanym poleceniem
 */
static void ExecuteInstructionWithParametr(Stack* stack, int lineNumber,
                                           char* line, size_t lineSize) {
    char* instruction = cut(line, 0, lineSize);
    size_t spaceInd = FindSpace(line, lineSize);
    char* parametr = cut(line, spaceInd, lineSize);

    if (memcmp(instruction, "DEG_BY", DEG_BY_LENGTH) == 0) {
        if (line[DEG_BY_LENGTH] != SPACE || !isDigit(line[DEG_BY_LENGTH + 1]) ||
            !IsCorrectDegByEnd(line, lineSize, spaceInd)) {
            EndExecution("ERROR %u DEG BY WRONG VARIABLE\n", instruction, parametr, lineNumber);
            return;
        }
        char* pEnd;
        unsigned long int x = strtoul(parametr, &pEnd, DECIMAL_BASE);
        if (errno == ERANGE || (strcmp(pEnd, "\n") != 0 && strcmp(pEnd, "\0") != 0)) {
            EndExecution("ERROR %u DEG BY WRONG VARIABLE\n", instruction, parametr, lineNumber);
            return;
        }
        DEG_BY(stack, x, lineNumber);
    } else if (memcmp(instruction, "AT", AT_LENGTH) == 0) {
        if (line[AT_LENGTH] != SPACE || !(isDigit(line[AT_LENGTH + 1]) || line[AT_LENGTH + 1] == '-') ||
            !IsCorrectAtEnd(line, lineSize, spaceInd)) {
            EndExecution("ERROR %u AT WRONG VALUE\n", instruction, parametr, lineNumber);
            return;
        }
        char* pEnd;
        long int x = strtol(parametr, &pEnd, DECIMAL_BASE);
        if (errno == ERANGE || (strcmp(pEnd, "\n") != 0 && strcmp(pEnd, "\0") != 0)) {
            EndExecution("ERROR %u AT WRONG VALUE\n", instruction, parametr, lineNumber);
            return;
        }
        AT(stack, x, lineNumber);
    } else {
        EndExecution("ERROR %u WRONG COMMAND\n", instruction, parametr, lineNumber);
        return;
    }

    free(instruction);
    free(parametr);
}

void InstructionScan(Stack* stack, int lineNumber) {
    char* instruction = NULL;
    size_t length = 0;
    size_t lineSize = 0;

    lineSize = getline(&instruction, &length, stdin);
    if (instruction == NULL) {
        exit(1);
    }

    if (DoesLineContainSpace(instruction, lineSize)) {
        ExecuteInstructionWithParametr(stack, lineNumber, instruction, lineSize);
    } else {
        ExecuteInstruction(stack, lineNumber, instruction, lineSize);
    }
    free(instruction);
}
