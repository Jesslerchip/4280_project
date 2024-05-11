// Jessica Seabolt CMP SCI 4280 Project Updated 05/10/2024

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "parsenode.h"
#include "scanner.h"
#include "token.h"

typedef enum {VAR, LABEL} nameType;

typedef struct Variable {
    char name[255];
    int value;
} Variable;

typedef struct JumpFunc {
    char funcName[255];
    char jumpName[255];
} JumpFunc;

void freeParseTree(ParseNode* node);

ParseNode* createNode(const char* label);
void addChild(ParseNode* parent, ParseNode* child);
void initJumpLabel();
ParseNode* parser(FILE* inputFile, FILE* outputFile);
ParseNode* program(Token* token, FILE* inputFile);
ParseNode* func(Token* token, FILE* inputFile);
ParseNode* block(Token* token, FILE* inputFile);
ParseNode* vars(Token* token, FILE* inputFile, int* varCount);
ParseNode* expr(Token* token, FILE* inputFile);
ParseNode* exprPrime(Token* token, FILE* inputFile);
ParseNode* N(Token* token, FILE* inputFile);
ParseNode* NPrime(Token* token, FILE* inputFile);
ParseNode* A(Token* token, FILE* inputFile);
ParseNode* APrime(Token* token, FILE* inputFile);
ParseNode* M(Token* token, FILE* inputFile);
ParseNode* R(Token* token, FILE* inputFile);
ParseNode* stats(Token* token, FILE* inputFile);
ParseNode* mStat(Token* token, FILE* inputFile);
ParseNode* stat(Token* token, FILE* inputFile);
ParseNode* in(Token* token, FILE* inputFile);
ParseNode* out(Token* token, FILE* inputFile);
ParseNode* ifFunc(Token* token, FILE* inputFile);
ParseNode* loop1(Token* token, FILE* inputFile);
ParseNode* loop2(Token* token, FILE* inputFile);
ParseNode* assign(Token* token, FILE* inputFile);
ParseNode* RO(Token* token);
ParseNode* label(Token* token, FILE* inputFile);
ParseNode* gotoFunc(Token* token, FILE* inputFile);
ParseNode* pick(Token* token, FILE* inputFile);
ParseNode* pickBody(Token* token, FILE* inputFile);
void exprTraversal(ParseNode* node, int* hasLoaded, int* unaryUsed, char* lastUsed);

#endif