// Jessica Seabolt CMP SCI 4280 Project Updated 04/12/2024

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "scanner.h"
#include "token.h"

#define MAX_CHILDREN 6

typedef struct ParseNode {
    char label[50];
    char value[50];
    struct ParseNode* children[MAX_CHILDREN];
    int numChildren;
} ParseNode;

void freeParseTree(ParseNode* node);

ParseNode* createNode(const char* label);
void addChild(ParseNode* parent, ParseNode* child);
ParseNode* parser(FILE* inputFile);
ParseNode* program(Token* token, FILE* inputFile);
ParseNode* func(Token* token, FILE* inputFile);
ParseNode* block(Token* token, FILE* inputFile);
ParseNode* vars(Token* token, FILE* inputFile);
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
ParseNode* pickBody(FILE* inputFile);

#endif