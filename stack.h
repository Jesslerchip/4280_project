#ifndef STACK_H
#define STACK_H

#include "token.h"

#define MAX_STACK_SIZE 100

typedef struct Stack {
    Token items[MAX_STACK_SIZE];
    int top;
} Stack;

void initStack(Stack** stack);
void push(Stack* stack, Token* item);
void pop(Stack* stack);
int find(Stack* stack, const char* identifier);

#endif