// Jessica Seabolt CMP SCI 4280 Project Updated 05/10/2024

#define _POSIX_C_SOURCE 200809L

#include "scanner.h"
#include "token.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int lineNumber = 1;
int charNumber = 1;
int fsaTable[MAX_STATES][MAX_CHARS];

// Keywords to check for
char reservedWords[17][9] = {
    "start", "stop", "while", "repeat", "until", "label", "return", "func",
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
}

// Returns the next state given the current state and input character
int getNextState(int currentState, unsigned char c) {
    return fsaTable[currentState][c];
}


// Driver for FSA table, reads input file and returns tokens
Token getToken(FILE* inputFile) {
    Token token;
    char buffer[MAX_TOKEN_LEN + 1];

    // Initialize buffer
    memset(buffer, '\0', sizeof(buffer));


    int bufferIndex = 0;
    enum State state = START;
    int c;

    while (1) {

        // Print buffer info

        c = fgetc(inputFile);

        if (c == EOF) {
            // If the buffer contains a token, return it
            if (state == IDENTIFIER || state == INTEGER || state == OPERATOR) {
                buffer[bufferIndex] = '\0';
                token.tokenInstance = strdup(buffer);
                if (state == IDENTIFIER) {
                    int i;
                    for (i = 0; i < 17; i++) {
                        if (strcmp(reservedWords[i], buffer) == 0) {
                            token.tokenID = KEYWORD_TK;
                            break;
                        }
                    }
                    if (i == 17) {
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

        // printf("DEBUG SCANNER: Buffer: %s, Index: %d, Char: %c\n", buffer, bufferIndex, c);

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
                        if (nextChar == '/') { // Comment
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
                    if (c == ':') {
                        int nextChar = fgetc(inputFile);
                        charNumber++;
                        if (nextChar == '=') { // :=
                            buffer[bufferIndex++] = c;
                            c = nextChar;
                        } else {
                            ungetc(nextChar, inputFile);
                            charNumber--;
                        }
                    }
                    if (c == '&') {
                        int nextChar = fgetc(inputFile);
                        charNumber++;
                        if (nextChar == '&') { // &&
                            buffer[bufferIndex++] = c;
                            c = nextChar;
                        } else {
                            ungetc(nextChar, inputFile);
                            charNumber--;
                        }
                    }
                    if (c == '|') {
                        int nextChar = fgetc(inputFile);
                        charNumber++;
                        if (nextChar == '|') { // ||
                            buffer[bufferIndex++] = c;
                            c = nextChar;
                        } else {
                            ungetc(nextChar, inputFile);
                            charNumber--;
                        }
                    }
                    if (c == '=') {
                        int nextChar = fgetc(inputFile);
                        charNumber++;
                        if (nextChar == '=') { // ==
                            buffer[bufferIndex++] = c;
                            c = nextChar;
                        } else if (nextChar == '!') { // =!=
                            int nextNextChar = fgetc(inputFile);
                            charNumber++;
                            if (nextNextChar == '=') {
                                buffer[bufferIndex++] = c;
                                buffer[bufferIndex++] = nextChar;
                                c = nextNextChar;
                            } else {
                                ungetc(nextNextChar, inputFile);
                                ungetc(nextChar, inputFile);
                                charNumber -= 2;
                            }
                        } else {
                            ungetc(nextChar, inputFile);
                            charNumber--;
                        }
                    }
                    if (c == '.') {
                        int nextChar = fgetc(inputFile);
                        charNumber++;
                        if (nextChar == '.') { // ==
                            int nextNextChar = fgetc(inputFile);
                            charNumber++;
                            if (nextNextChar == '.') {
                                buffer[bufferIndex++] = c;
                                buffer[bufferIndex++] = nextChar;
                                c = nextNextChar;
                            } else {
                                ungetc(nextNextChar, inputFile);
                                ungetc(nextChar, inputFile);
                                charNumber -= 2;
                            }
                        } else {
                            ungetc(nextChar, inputFile);
                            charNumber--;
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
                    for (i = 0; i < 17; i++) {
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
                    buffer[bufferIndex] = '\0';
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

void ungetToken(Token token, FILE* inputFile) {
    ungetc(token.tokenInstance[strlen(token.tokenInstance) - 1], inputFile);
    for (int i = strlen(token.tokenInstance) - 2; i >= 0; i--) {
        ungetc(token.tokenInstance[i], inputFile);
    }
    ungetc(' ', inputFile);  // Add a space before the token
}