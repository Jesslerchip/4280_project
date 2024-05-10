// Jessica Seabolt CMP SCI 4280 Project Updated 05/10/2024

// I tried to make this as modular as I could. 

#include <stdio.h>
#include <stdlib.h>
#include "parsenode.h"
#include "parser.h"
#include "scanner.h"

void printParseTree(ParseNode* node, int indent);
void freeParseTree(ParseNode* node);

int main(int argc, char *argv[]) {
    FILE *inputFile = NULL;
    FILE *outputFile = NULL;

    // Check if a file was provided as an argument
    if (argc > 2) {
        printf("Usage: %s [filename]\n", argv[0]);
        return 1;
    } else if (argc == 2) {
        inputFile = fopen(argv[1], "r");
        if (inputFile == NULL) {
            printf("Unable to open file: %s\n", argv[1]);
            return 1;
        }
        outputFile = fopen("file.asm", "w");
    } else {
        inputFile = stdin;
        outputFile = fopen("kb.asm", "w");
    }

    if (outputFile == NULL) {
        printf("Unable to create output file\n");
        return 1;
    }

    // Initialize the FSA table
    initFSATable();

    ParseNode* parseTree = parser(inputFile, outputFile);
    if (parseTree != NULL) {
        // printParseTree(parseTree, 0);
        freeParseTree(parseTree);
        fclose(outputFile);
    } else {
        printf("Parsing failed!\n");
        freeParseTree(parseTree);
        fclose(outputFile);
        remove("file.asm");
        remove("kb.asm");
    }

    if (inputFile != stdin) {
        fclose(inputFile);
    }

    return 0;
}

void printParseTree(ParseNode* node, int indent) {
    
    if (node == NULL) {
        return;
    }

    for (int i = 0; i < indent; i++) {
        printf("  ");
    }

    printf("%s", node->label);

    printf("\n");

    for (int i = 0; i < node->numChildren; i++) {
        printParseTree(node->children[i], indent + 1);
    }
}

void freeParseTree(ParseNode* node) {
    if (node == NULL) {
        printf("Attempting to free a NULL node.\n");
        return;
    }

    // printf("DEBUG PARSE TREE: Freeing node: %s\n", node->label);

    for (int i = 0; i < node->numChildren; i++) {
        freeParseTree(node->children[i]);
    }

    free(node);
}