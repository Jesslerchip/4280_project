// Jessica Seabolt CMP SCI 4280 Project Updated 04/11/2024

#include "scanner.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>

// Fairly simple. Checks the token type and prints relevant information.
void testScanner(FILE* inputFile) {
    Token token;
    do {
        token = getToken(inputFile);
        const char* tokenTypeName;
        switch (token.tokenID) {
            case ID_TK:
                tokenTypeName = "Identifier";
                break;
            case KEYWORD_TK:
                tokenTypeName = "Keyword";
                break;
            case INT_TK:
                tokenTypeName = "Integer";
                break;
            case OPERATOR_TK:
                tokenTypeName = "Operator";
                break;
            case EOF_TK:
                tokenTypeName = "End of File";
                break;
            default:
                tokenTypeName = "Unknown";
                break;
        }
        if (token.tokenID != EOF_TK) {
            printf("%s: %s, Line: %d, Char: %d\n", tokenTypeName, token.tokenInstance ? token.tokenInstance : "", token.lineNumber, token.charNumber);
            if (token.tokenInstance) {
                free(token.tokenInstance);
            }
        }

    } while (token.tokenID != EOF_TK);
}