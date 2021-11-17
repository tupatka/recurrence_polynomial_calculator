/** @file Kalkulator operacji na wielomianach rzadkich wielu zmiennych
 *  @author Patrycja Stępień
*/

#include "stack.h"
#include "read_input.h"

int main() {
    Stack stack = StackCreate();
    read_input(&stack);
    StackDestroy(&stack);
}
