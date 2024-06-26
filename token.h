// Jessica Seabolt CMP SCI 4280 Project Updated 05/10/2024

#ifndef TOKEN_H
#define TOKEN_H

#define MAX_TOKEN_LEN 256

// Tokens have an ID, an instance, and a line and character number for error reporting and printing
typedef struct {
    int tokenID;
    char* tokenInstance;
    int lineNumber;
    int charNumber;
} Token;

// Token types are used to determine what kind of token was found and how to handle it
enum TokenType {
    ID_TK,
    KEYWORD_TK,
    INT_TK,
    OPERATOR_TK,
    EOF_TK
};

#endif