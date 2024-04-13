#define _POSIX_C_SOURCE 200809L // For strdup

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"

void initStack(Stack** stack) {
    *stack = (Stack*)malloc(sizeof(Stack));
    (*stack)->top = -1;
}

void push(Stack* stack, Token* item) {
    if (stack->top == MAX_STACK_SIZE - 1) {
        printf("STACK ERROR: Stack overflow\n");
        exit(1);
    }
    
    (stack->top)++;
    stack->items[stack->top] = *item;
}

void pop(Stack* stack) {
    if (stack->top == -1) {
        printf("STACK ERROR: Stack underflow\n");
        exit(1);
    }
    (stack->top)--;
}

int find(Stack* stack, const char* identifier) {
    for (int i = stack->top; i >= 0; i--) {
        if (strcmp(stack->items[i].tokenInstance, identifier) == 0) {
            return stack->top - i;
        }
    }
    return -1;
}