/** @file
 *  Kalkulator operacji na wielomianach rzadkich wielu zmiennych
 *  @author Patrycja Stępień
*/
#ifndef INSTRUCTION_SCAN_H
#define INSTRUCTION_SCAN_H

#include "stack.h"

/**
 * Wczytuje polecenie kalkulatora ze standardowego wejścia
 * i wstępnie rozpoznaje typ polecenia (z argumentem lub bez).
 * @param[in, out] stack : stos
 * @param[in] lineNumber : numer aktualnie wczytywanej linii
 */
void InstructionScan(Stack* stack, int lineNumber);

#endif /* INSTRUCTION_SCAN_H */