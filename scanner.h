// Jessica Seabolt CMP SCI 4280 Project Updated 05/10/2024

#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"
#include <stdio.h>

#define MAX_STATES 4
#define MAX_CHARS 128
#define _POSIX_C_SOURCE 200809L // For strdup

// The states of the FSA. Comments don't have a state because they just get ignored. Hopefully this is okay?
enum State {
    START,
    IDENTIFIER,
    INTEGER,
    OPERATOR
};

extern int fsaTable[MAX_STATES][MAX_CHARS];
void initFSATable();
int getNextState(int currentState, unsigned char c);
Token getToken(FILE* inputFile);
void ungetToken(Token token, FILE* inputFile);

#endif