// Jessica Seabolt CMP SCI 4280 Project 1 Updated 03/12/2024

// I tried to make this as modular as I could. 

#include <stdio.h>
#include "scanner.h"
#include "testScanner.h"

int main(int argc, char *argv[]) {
    FILE *inputFile = NULL;

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
    } else {
        inputFile = stdin;
    }

    // Initialize the FSA table and scan that hecking file
    initFSATable();
    testScanner(inputFile);

    if (inputFile != stdin) {
        fclose(inputFile);
    }

    return 0;
}