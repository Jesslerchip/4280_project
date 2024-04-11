// Jessica Seabolt CMP SCI 4280 Project 1 Updated 03/12/2024

// This line fixes the issue with strdup. I didn't need it on my machine, but it was required for the server.
#define _POSIX_C_SOURCE 200809L

#include "scanner.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKEN_LEN 256

int lineNumber = 1;
int charNumber = 1;
int fsaTable[MAX_STATES][MAX_CHARS];

// Keywords to check for
char reservedWords[16][9] = {
    "start", "stop", "while", "repeat", "until", "label", "return",
    "cin", "cout", "tape", "jump", "if", "then", "pick", "create", "set"
};

// Initialize FSA table
void initFSATable() {

    // Initialize all entries to -1 (invalid transition)
    memset(fsaTable, -1, sizeof(fsaTable));

    // Define valid transitions
    for (int i = 'a'; i <= 'z'; i++) {
        fsaTable[START][i] = IDENTIFIER;
        fsaTable[IDENTIFIER][i] = IDENTIFIER;
    }
    for (int i = 'A'; i <= 'Z'; i++) {
        fsaTable[START][i] = IDENTIFIER;
        fsaTable[IDENTIFIER][i] = IDENTIFIER;
    }
    for (int i = '0'; i <= '9'; i++) {
        fsaTable[START][i] = INTEGER;
        fsaTable[IDENTIFIER][i] = IDENTIFIER;
        fsaTable[INTEGER][i] = INTEGER;
    }
    fsaTable[IDENTIFIER]['_'] = IDENTIFIER;
    fsaTable[START]['/'] = OPERATOR;
    fsaTable[START]['='] = OPERATOR;
    fsaTable[START]['<'] = OPERATOR;
    fsaTable[START]['>'] = OPERATOR;
    fsaTable[START][':'] = OPERATOR;
    fsaTable[START]['+'] = OPERATOR;
    fsaTable[START]['-'] = OPERATOR;
    fsaTable[START]['*'] = OPERATOR;
    fsaTable[START]['^'] = OPERATOR;
    fsaTable[START]['.'] = OPERATOR;
    fsaTable[START]['('] = OPERATOR;
    fsaTable[START][')'] = OPERATOR;
    fsaTable[START][','] = OPERATOR;
    fsaTable[START]['{'] = OPERATOR;
    fsaTable[START]['}'] = OPERATOR;
    fsaTable[START][';'] = OPERATOR;
    fsaTable[START]['['] = OPERATOR;
    fsaTable[START][']'] = OPERATOR;
    fsaTable[START]['|'] = OPERATOR;
    fsaTable[START]['&'] = OPERATOR;
    fsaTable[START]['!'] = OPERATOR;
    fsaTable[OPERATOR]['='] = OPERATOR;
    fsaTable[OPERATOR]['!'] = OPERATOR;
    fsaTable[OPERATOR]['|'] = OPERATOR;
    fsaTable[OPERATOR]['&'] = OPERATOR;
}

// Returns the next state given the current state and input character
int getNextState(int currentState, unsigned char c) {
    return fsaTable[currentState][c];
}


// Driver for FSA table, reads input file and returns tokens
Token getToken(FILE* inputFile) {
    Token token;
    char buffer[MAX_TOKEN_LEN + 1];
    int bufferIndex = 0;
    enum State state = START;
    int c;

    while (1) {
        c = fgetc(inputFile);

    if (c == EOF) {
        // If the buffer contains a token, return it
        if (state == IDENTIFIER || state == INTEGER || state == OPERATOR) {
            buffer[bufferIndex] = '\0';
            token.tokenInstance = strdup(buffer);
            if (state == IDENTIFIER) {
                int i;
                for (i = 0; i < 16; i++) {
                    if (strcmp(reservedWords[i], buffer) == 0) {
                        token.tokenID = KEYWORD_TK;
                        break;
                    }
                }
                if (i == 16) {
                    token.tokenID = ID_TK;
                }
            } else if (state == INTEGER) {
                token.tokenID = INT_TK;
            } else if (state == OPERATOR) {
                token.tokenID = OPERATOR_TK;
            }
            token.lineNumber = lineNumber;
            token.charNumber = charNumber - bufferIndex;
            return token;
        // If the buffer is empty, return EOF
        } else {
            token.tokenID = EOF_TK;
            token.tokenInstance = NULL;
            token.lineNumber = lineNumber;
            token.charNumber = charNumber;
            return token;
        }
    }

        enum State nextState = getNextState(state, c);

        // Handle state transitions
        switch (nextState) {
            case IDENTIFIER:
                if (bufferIndex < MAX_TOKEN_LEN) {
                    buffer[bufferIndex++] = c;
                } else {
                    printf("SCANNER ERROR: Token too long at line %d, char %d\n", lineNumber, charNumber);
                }
                break;

            case INTEGER:
                if (bufferIndex < MAX_TOKEN_LEN) {
                    buffer[bufferIndex++] = c;
                } else {
                    printf("SCANNER ERROR: Token too long at line %d, char %d\n", lineNumber, charNumber);
                }
                break;

            case OPERATOR:
                if (bufferIndex < MAX_TOKEN_LEN) {
                    if (c == '/') {
                        int nextChar = fgetc(inputFile);
                        if (nextChar == '/') {
                            while (c != '\n' && c != EOF) {
                                c = fgetc(inputFile);
                                charNumber++;
                            }
                            if (c == '\n') {
                                lineNumber++;
                                charNumber = 1;
                            }
                            state = START;
                            bufferIndex = 0;
                            continue;
                        } else {
                            ungetc(nextChar, inputFile);
                        }
                    }
                    buffer[bufferIndex++] = c;
                } else {
                    printf("SCANNER ERROR: Token too long at line %d, char %d\n", lineNumber, charNumber);
                }
                break;

            default:
                if (state == IDENTIFIER) {
                    buffer[bufferIndex] = '\0';
                    ungetc(c, inputFile);
                    token.tokenInstance = strdup(buffer);
                    int i;
                    for (i = 0; i < 16; i++) {
                        if (strncmp(reservedWords[i], buffer, strlen(reservedWords[i])) == 0) {
                            token.tokenID = KEYWORD_TK;
                            token.tokenInstance = strdup(reservedWords[i]);
                            token.lineNumber = lineNumber;
                            token.charNumber = charNumber - strlen(reservedWords[i]);
                            fseek(inputFile, -strlen(buffer) + strlen(reservedWords[i]), SEEK_CUR);
                            return token;
                        }
                    }
                    token.tokenID = ID_TK;
                    token.lineNumber = lineNumber;
                    token.charNumber = charNumber - bufferIndex;
                    return token;
                } else if (state == INTEGER) {
                    buffer[bufferIndex] = '\0';
                    ungetc(c, inputFile);
                    token.tokenInstance = strdup(buffer);
                    token.tokenID = INT_TK;
                    token.lineNumber = lineNumber;
                    token.charNumber = charNumber - bufferIndex;
                    return token;
                } else if (state == OPERATOR) {
                    ungetc(c, inputFile);
                    if (bufferIndex > 0) {
                        buffer[bufferIndex] = '\0';
                        token.tokenInstance = strdup(buffer);
                        token.tokenID = OPERATOR_TK;
                        token.lineNumber = lineNumber;
                        token.charNumber = charNumber - bufferIndex;
                        return token;
                    }
                } else if (isspace(c)) {
                    state = START;
                    if (c == '\n') {
                        lineNumber++;
                        charNumber = 1;
                    } else {
                        charNumber++;
                    }
                    bufferIndex = 0;
                    continue;
                } else {
                    printf("SCANNER ERROR: Invalid character '%c' at line %d, char %d\n", c, lineNumber, charNumber);
                    charNumber++;
                }
                break;
        }

        state = nextState;
        charNumber++;
    }
}