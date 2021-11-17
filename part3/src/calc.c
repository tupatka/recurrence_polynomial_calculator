/** @file
 *  Kalkulator operacji na wielomianach rzadkich wielu zmiennych.
 *  @author Patrycja Stępień
*/

#include "stack.h"
#include "read_input.h"

/**
 * Tworzy stos, wczytuje polecenia ze standardowego wejścia,
 * wykonuje żądane polecania, usuwa stos.
 */
int main() {
    Stack stack = StackCreate();
    ReadInput(&stack);
    StackDestroy(&stack);
}
