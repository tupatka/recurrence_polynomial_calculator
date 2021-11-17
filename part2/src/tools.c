/** @file Kalkulator operacji na wielomianach rzadkich wielu zmiennych
 *  @author Patrycja Stępień
*/

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tools.h"

char* cut(char* line, size_t wordBegin, size_t wordEnd) {
    char *word = &line[wordBegin];
    char c = line[wordEnd];
    line[wordEnd] = '\0';

    size_t sizeOfExtractedWord = wordEnd - wordBegin + 1;
    char *extractedWord = malloc(sizeOfExtractedWord * sizeof(char));
    if (extractedWord == NULL) {
        exit(1);
    }
    sscanf(word, "%s", extractedWord);
    line[wordEnd] = c;

    return extractedWord;
}

bool isDigit(char c) {
    return c >= '0' && c <= '9';
}