// Jessica Seabolt CMP SCI 4280 Project Updated 05/10/2024

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "parsenode.h"
#include "parser.h"
#include "stack.h"

Stack* stack = NULL;
FILE* targetFile = NULL;
int varCountGlobal = 0;

static int LabelCntr=0; /* counting unique labels generated */
static int VarCntr=0; /* counting unique temporaries generated */
static char labelNames[255][20]; /* for creation of unique label names */
static Variable variableEntries[255]; // Variable names w/ values
static JumpFunc jumpFuncEntries[255]; // Variable names w/ values
static int numJumpFuncs = 0; // A really dumb way of handling this lol


void newName(nameType type) { 
    if (type==VAR) { // creating new temporary
        sprintf(variableEntries[VarCntr].name, "T%d", (VarCntr - varCountGlobal));
        VarCntr++;
    }
    else { // creating new Label
        sprintf(labelNames[LabelCntr],"L%d",LabelCntr); /* new lables as L0, L1, etc */
        LabelCntr++;
    }
}   

ParseNode* createNode(const char* label) {
    ParseNode* node = (ParseNode*)malloc(sizeof(ParseNode));
    strcpy(node->label, label);
    node->numChildren = 0;
    return node;
}

void addChild(ParseNode* parent, ParseNode* child) {
    parent->children[parent->numChildren] = child;
    parent->numChildren++;
}

void initVariableEntries() {
    for (int i = 0; i < 255; i++) {
        variableEntries[i].value = -7;
    }
}

ParseNode* parser(FILE* inputFile, FILE* outputFile) {

    targetFile = outputFile;

    initStack(&stack);
    initVariableEntries();

    Token token = getToken(inputFile);
    
    ParseNode* root = program(&token, inputFile);

    // Consume the next token after parsing the program
    token = getToken(inputFile);

    if (token.tokenID == EOF_TK) {
        return root;
    } else {
        printf("PARSER ERROR: Unexpected token '%s' at line %d, char %d\n", token.tokenInstance, token.lineNumber, token.charNumber);
        freeParseTree(root);
        return NULL;
    }

}

ParseNode* program(Token* token, FILE* inputFile) {
    ParseNode* node = createNode("<program>");

    ParseNode* varsNode = vars(token, inputFile, &varCountGlobal);
    if (varsNode != NULL) {
        addChild(node, varsNode);
    }

    else {
        printf("PARSER ERROR: Expected 'vars' at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }

    if (token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "tape") == 0) {
        addChild(node, createNode("tape"));
        *token = getToken(inputFile);

        if (token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "func") == 0) {
            ParseNode* funcNode = func(token, inputFile);
            if (funcNode != NULL) {
                addChild(node, funcNode);
                *token = getToken(inputFile);
            } else {
                freeParseTree(node);
                return NULL;
            }
        }

        ParseNode* blockNode = block(token, inputFile);
        if (blockNode != NULL) {
            addChild(node, blockNode);
        } else {
            freeParseTree(node);
            return NULL;
        }
    } else {
        printf("PARSER ERROR: Expected 'tape' keyword at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }

    fprintf(targetFile, "STOP\n");
    for (int i = 0; i < varCountGlobal; i++) {
        pop(stack);
    }
    for (int i = 0; i < (VarCntr); i++) {
        fprintf(targetFile, "%s %d\n", variableEntries[i].name, variableEntries[i].value);
    }

    return node;
}

ParseNode* func(Token* token, FILE* inputFile) {
    if (token->tokenID != KEYWORD_TK || strcmp(token->tokenInstance, "func") != 0) {
        printf("PARSER ERROR: Expected 'func' keyword at line %d, char %d\n", token->lineNumber, token->charNumber);
        return NULL;
    }

    ParseNode* node = createNode("<func>");
    addChild(node, createNode("func"));

    *token = getToken(inputFile);
    if (token->tokenID != ID_TK) {
        printf("PARSER ERROR: Expected identifier after 'func' at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }

    int lookup = find(stack, token->tokenInstance);
    if (lookup != -1) {
        printf("STACK ERROR: Identifier '%s' already found in stack at line %d, char %d\n", token->tokenInstance, token->lineNumber, token->charNumber);
        freeParseTree(node);
        fclose(targetFile);
        remove("file.asm");
        remove("kb.asm");
        exit(1);
    }

    addChild(node, createNode(token->tokenInstance));
    push(stack, token);

    newName(LABEL);
    char* skipLabel = labelNames[LabelCntr - 1];
    fprintf(targetFile, "BR %s\n", skipLabel);// Used to pass the func until it's called

    fprintf(targetFile, "%s: NOOP\n", token->tokenInstance);
    strcpy(jumpFuncEntries[numJumpFuncs].funcName, token->tokenInstance);

    *token = getToken(inputFile);
    ParseNode* blockNode = block(token, inputFile);
    if (blockNode != NULL) {
        addChild(node, blockNode);
    } else {
        freeParseTree(node);
        return NULL;
    }

    newName(LABEL);
    strcpy(jumpFuncEntries[numJumpFuncs].jumpName, labelNames[LabelCntr -1]);
    fprintf(targetFile, "BR %s\n", jumpFuncEntries[numJumpFuncs].jumpName);
    numJumpFuncs++;

    fprintf(targetFile, "%s: NOOP\n", skipLabel);

    return node;
}

ParseNode* block(Token* token, FILE* inputFile) {
    if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, "{") != 0) {
        printf("PARSER ERROR: Expected '{' at line %d, char %d\n", token->lineNumber, token->charNumber);
        return NULL;
    }

    ParseNode* node = createNode("<block>");

    int* varCountBlock = (int*)malloc(sizeof(int));
    *varCountBlock = 0;

    *token = getToken(inputFile);
    ParseNode* varsNode = vars(token, inputFile, varCountBlock);
    
    if (varsNode != NULL) {
        addChild(node, varsNode);
    }

    ParseNode* statsNode = stats(token, inputFile);

    if (statsNode != NULL) {
        addChild(node, statsNode);
    } else {
        freeParseTree(node);
        return NULL;
    }

    if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, "}") != 0) {
        printf("PARSER ERROR: Expected '}' at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }

    for (int i = 0; i < *varCountBlock; i++) {
        // printf("STACK: Stack top: %d, Stack item: %s\n", stack->top, stack->items[stack->top].tokenInstance);
        pop(stack);
    }

    free(varCountBlock);

    return node;
}

ParseNode* vars(Token* token, FILE* inputFile, int* varCount) {
    ParseNode* node = createNode("<vars>");

    if (token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "create") == 0) {
        addChild(node, createNode("create"));

        *token = getToken(inputFile);

        if (token->tokenID != ID_TK) {
            printf("PARSER ERROR: Expected identifier after 'create' at line %d, char %d\n", token->lineNumber, token->charNumber);
            freeParseTree(node);
            return NULL;
        }

        int lookup = find(stack, token->tokenInstance);
        if (lookup != -1) {
            printf("STACK ERROR: Identifier '%s' already found in stack at line %d, char %d\n", token->tokenInstance, token->lineNumber, token->charNumber);
            freeParseTree(node);
            fclose(targetFile);
            remove("file.asm");
            remove("kb.asm");
            exit(1);
        }

        addChild(node, createNode(token->tokenInstance));

        sprintf(variableEntries[VarCntr].name, "%s", token->tokenInstance);
        VarCntr++;

        push(stack, token);

        (*varCount)++;

        *token = getToken(inputFile);

        if (token->tokenID == OPERATOR_TK && strcmp(token->tokenInstance, ":=") == 0) {
            addChild(node, createNode(":="));

            *token = getToken(inputFile);

            if (token->tokenID != INT_TK) {
                printf("PARSER ERROR: Expected integer after ':=' at line %d, char %d\n", token->lineNumber, token->charNumber);
                freeParseTree(node);
                return NULL;
            }

            addChild(node, createNode(token->tokenInstance));
            variableEntries[VarCntr - 1].value = atoi(token->tokenInstance);
            
            *token = getToken(inputFile);
        }

        if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, ";") != 0) {
            printf("PARSER ERROR: Expected ';' after variable declaration at line %d, char %d\n", token->lineNumber, token->charNumber);
            freeParseTree(node);
            return NULL;
        }

        *token = getToken(inputFile);

        if (token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "create") == 0) {
            ParseNode* varsNode = vars(token, inputFile, varCount);
            if (varsNode != NULL) {
                addChild(node, varsNode);
            }

        }

    }

    return node;
}

ParseNode* expr(Token* token, FILE* inputFile) {

    ParseNode* node = createNode("<expr>");

    ParseNode* mNode = M(token, inputFile);
    if (mNode != NULL) {
        addChild(node, mNode);
    } else {
        freeParseTree(node);
        return NULL;
    }

    ParseNode* exprPrimeNode = exprPrime(token, inputFile);
    if (exprPrimeNode != NULL) {
        addChild(node, exprPrimeNode);
    }

    return node;

}

ParseNode* exprPrime(Token* token, FILE* inputFile) {

    ParseNode* node = createNode("<expr'>");


    if (strcmp(token->tokenInstance, "+") != 0 &&
        strcmp(token->tokenInstance, "-") != 0 &&
        strcmp(token->tokenInstance, "*") != 0 &&
        strcmp(token->tokenInstance, "/") != 0) {
            freeParseTree(node);
            return NULL;
    }

    addChild(node, createNode(token->tokenInstance));
    *token = getToken(inputFile);

    ParseNode* exprNode = expr(token, inputFile);
    if (exprNode != NULL) {
        addChild(node, exprNode);
    }
        
    return node;
}

ParseNode* M(Token* token, FILE* inputFile) {
    ParseNode* node = createNode("<M>");

    if (strcmp(token->tokenInstance, "^") == 0) {
        addChild(node, createNode("^"));
        *token = getToken(inputFile);
        ParseNode* mNode = M(token, inputFile);
        if (mNode != NULL) {
            addChild(node, mNode);
        } else {
            printf("PARSER ERROR: M cannot be empty!\n");
            freeParseTree(node);
            return NULL;
        }
    } else {
        ParseNode* rNode = R(token, inputFile);
        if (rNode != NULL) {
            addChild(node, rNode);
        } else {
            printf("PARSER ERROR: R cannot be empty!\n");
            freeParseTree(node);
            return NULL;
        }
    }

    return node;
}


ParseNode* R(Token* token, FILE* inputFile) {
    ParseNode* node = createNode("<R>");

    if (token->tokenID == OPERATOR_TK && strcmp(token->tokenInstance, "(") == 0) {
        *token = getToken(inputFile);
    }

    if (token->tokenID == ID_TK) {

        int lookup = find(stack, token->tokenInstance);
        if (lookup == -1) {
            printf("STACK ERROR: Identifier '%s'not found in stack at line %d, char %d\n", token->tokenInstance, token->lineNumber, token->charNumber);
            freeParseTree(node);
            fclose(targetFile);
            remove("file.asm");
            remove("kb.asm");
            exit(1);
        }

        addChild(node, createNode(token->tokenInstance));
        *token = getToken(inputFile);
    } else if (token->tokenID == INT_TK) {
        addChild(node, createNode(token->tokenInstance));
        *token = getToken(inputFile);
    } else {
        ParseNode* exprNode = expr(token, inputFile);
        if (exprNode != NULL) {
            addChild(node, exprNode);
        } else {
            freeParseTree(node);
            return NULL;
        }
    }

    if (token->tokenID == OPERATOR_TK && strcmp(token->tokenInstance, ")") == 0) {
        *token = getToken(inputFile);
    }

    return node;
}

ParseNode* stats(Token* token, FILE* inputFile) {
    ParseNode* node = createNode("<stats>");

    ParseNode* statNode = stat(token, inputFile);
    if (statNode != NULL) {
        addChild(node, statNode);

        ParseNode* mStatNode = mStat(token, inputFile);
        if (mStatNode != NULL) {
            addChild(node, mStatNode);
        }
    } else {
        printf("PARSER ERROR: Unexpected token '%s' at line %d, char %d\n", token->tokenInstance, token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }

    return node;
}

ParseNode* mStat(Token* token, FILE* inputFile) {
    ParseNode* node = createNode("<mStat>");

    ParseNode* statNode = stat(token, inputFile);
    if (statNode != NULL) {
        addChild(node, statNode);

        ParseNode* mStatNode = mStat(token, inputFile);
        if (mStatNode != NULL) {
            addChild(node, mStatNode);
        }
    }

    return node;
}

ParseNode* stat(Token* token, FILE* inputFile) {
    ParseNode* node = createNode("<stat>");

    if (token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "cin") == 0) {
        ParseNode* inNode = in(token, inputFile);
        if (inNode != NULL) {
            addChild(node, inNode);

            *token = getToken(inputFile);

            if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, ";") != 0) {
                printf("PARSER ERROR: Expected ';' after 'cin' statement at line %d, char %d\n", token->lineNumber, token->charNumber);
                freeParseTree(node);
                return NULL;
            }

            *token = getToken(inputFile);

        } else {
            freeParseTree(node);
            return NULL;
        }
    } else if (token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "cout") == 0) {
        ParseNode* outNode = out(token, inputFile);
        if (outNode != NULL) {
            addChild(node, outNode);
            if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, ";") != 0) {
                printf("PARSER ERROR: Expected ';' after 'cout' statement at line %d, char %d\n", token->lineNumber, token->charNumber);
                freeParseTree(node);
                return NULL;
            }

            *token = getToken(inputFile);

        } else {
            freeParseTree(node);
            return NULL;
        }
    } else if (token->tokenID == OPERATOR_TK && strcmp(token->tokenInstance, "{") == 0) {
        ParseNode* blockNode = block(token, inputFile);
        if (blockNode != NULL) {
            addChild(node, blockNode);
        } else {
            freeParseTree(node);
            return NULL;
        }
    } else if (token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "if") == 0) {
        ParseNode* ifNode = ifFunc(token, inputFile);
        if (ifNode != NULL) {
            addChild(node, ifNode);
            *token = getToken(inputFile);

            if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, ";") != 0) {
                printf("PARSER ERROR: Expected ';' after 'if' statement at line %d, char %d\n", token->lineNumber, token->charNumber);
                freeParseTree(node);
                return NULL;
            }

            *token = getToken(inputFile);

        } else {
            freeParseTree(node);
            return NULL;
        }
    } else if (token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "while") == 0) {
        ParseNode* loop1Node = loop1(token, inputFile);
        if (loop1Node != NULL) {
            addChild(node, loop1Node);

            *token = getToken(inputFile);

            if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, ";") != 0) {
                printf("PARSER ERROR: Expected ';' after 'while' loop at line %d, char %d\n", token->lineNumber, token->charNumber);
                freeParseTree(node);
                return NULL;
            }

            *token = getToken(inputFile);

        } else {
            freeParseTree(node);
            return NULL;
        }
    } else if (token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "repeat") == 0) {
        ParseNode* loop2Node = loop2(token, inputFile);
        if (loop2Node != NULL) {
            addChild(node, loop2Node);

            *token = getToken(inputFile);

            if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, ";") != 0) {
                printf("PARSER ERROR: Expected ';' after 'repeat' loop at line %d, char %d\n", token->lineNumber, token->charNumber);
                freeParseTree(node);
                return NULL;
            }

            *token = getToken(inputFile);
        } else {
            freeParseTree(node);
            return NULL;
        }
    } else if ((token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "set") == 0) || (token->tokenID == ID_TK)) {
        ParseNode* assignNode = assign(token, inputFile);
        if (assignNode != NULL) {
            addChild(node, assignNode);

            if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, ";") != 0) {
                printf("PARSER ERROR: Expected ';' after 'set' statement at line %d, char %d\n", token->lineNumber, token->charNumber);
                freeParseTree(node);
                return NULL;
            }

            *token = getToken(inputFile);

        } else {
            freeParseTree(node);
            return NULL;
        }
    } else if (token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "label") == 0) {
        ParseNode* labelNode = label(token, inputFile);
        if (labelNode != NULL) {
            addChild(node, labelNode);

            *token = getToken(inputFile);

            if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, ";") != 0) {
                printf("PARSER ERROR: Expected ';' after 'label' at line %d, char %d\n", token->lineNumber, token->charNumber);
                freeParseTree(node);
                return NULL;
            }

            *token = getToken(inputFile);

        } else {
            freeParseTree(node);
            return NULL;
        }
    } else if (token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "jump") == 0) {
        ParseNode* gotoNode = gotoFunc(token, inputFile);
        if (gotoNode != NULL) {
            addChild(node, gotoNode);
            *token = getToken(inputFile);

            if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, ";") != 0) {
                printf("PARSER ERROR: Expected ';' after 'jump' statement at line %d, char %d\n", token->lineNumber, token->charNumber);
                freeParseTree(node);
                return NULL;
            }

            *token = getToken(inputFile);

        } else {
            freeParseTree(node);
            return NULL;
        }
    } else if (token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "pick") == 0) {
        ParseNode* pickNode = pick(token, inputFile);
        if (pickNode != NULL) {
            addChild(node, pickNode);

            if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, ";") != 0) {
                printf("PARSER ERROR: Expected ';' after pick at line %d, char %d\n", token->lineNumber, token->charNumber);
                freeParseTree(node);
                return NULL;
            }

            *token = getToken(inputFile);

        } else {
            freeParseTree(node);
            return NULL;
        }
    } else if (token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "create") == 0) {
        int varsCount = 0;

        ParseNode* varsNode = vars(token, inputFile, &varsCount);
        if (varsNode != NULL) {
            addChild(node, varsNode);

        } else {
            freeParseTree(node);
            return NULL;
        }
    } else if (token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "func") == 0) {
            ParseNode* funcNode = func(token, inputFile);
            if (funcNode != NULL) {
                addChild(node, funcNode);
                *token = getToken(inputFile);
            } else {
                freeParseTree(node);
                return NULL;
            }
    } else {
        freeParseTree(node);
        return NULL;
    }

    return node;
}

ParseNode* in(Token* token, FILE* inputFile) {
    if (token->tokenID != KEYWORD_TK || strcmp(token->tokenInstance, "cin") != 0) {
        printf("PARSER ERROR: Expected 'cin' keyword at line %d, char %d\n", token->lineNumber, token->charNumber);
        return NULL;
    }

    ParseNode* node = createNode("<in>");

    addChild(node, createNode("cin"));

    *token = getToken(inputFile);

    if (token->tokenID != ID_TK) {
        printf("PARSER ERROR: Expected identifier after 'cin' at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }

    int lookup = find(stack, token->tokenInstance);
    if (lookup == -1) {
        printf("STACK ERROR: Identifier '%s' not found in stack at line %d, char %d\n", token->tokenInstance, token->lineNumber, token->charNumber);
        freeParseTree(node);
        fclose(targetFile);
        remove("file.asm");
        remove("kb.asm");
        exit(1);
    }
    
    fprintf(targetFile, "READ %s\n", token->tokenInstance);

    addChild(node, createNode(token->tokenInstance));

    return node;
}

ParseNode* out(Token* token, FILE* inputFile) {
    
    if (token->tokenID != KEYWORD_TK || strcmp(token->tokenInstance, "cout") != 0) {
        printf("PARSER ERROR: Expected 'cout' keyword at line %d, char %d\n", token->lineNumber, token->charNumber);
        return NULL;
    }

    ParseNode* node = createNode("<out>");

    addChild(node, createNode("cout"));

    *token = getToken(inputFile);

    ParseNode* exprNode = expr(token, inputFile);
    if (exprNode != NULL) {
        addChild(node, exprNode);
    } else {
        freeParseTree(node);
        return NULL;
    }

    // Traversal and code gen
    newName(VAR);
    int hasLoaded = 0;
    int unaryUsed = 0;
    char lastUsed = '0';
    exprTraversal(exprNode, &hasLoaded, &unaryUsed, &lastUsed);

    fprintf(targetFile, "WRITE %s\n", variableEntries[VarCntr - 1].name);

    return node;
}

ParseNode* ifFunc(Token* token, FILE* inputFile) {
    if (token->tokenID != KEYWORD_TK || strcmp(token->tokenInstance, "if") != 0) {
        printf("PARSER ERROR: Expected 'if' keyword at line %d, char %d\n", token->lineNumber, token->charNumber);
        return NULL;
    }

    ParseNode* node = createNode("<if>");
    addChild(node, createNode("if"));

    *token = getToken(inputFile);
    if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, "[") != 0) {
        printf("PARSER ERROR: Expected '[' after 'if' at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }

    *token = getToken(inputFile);
    ParseNode* expr1Node = expr(token, inputFile);
    if (expr1Node != NULL) {
        addChild(node, expr1Node);
    } else {
        freeParseTree(node);
        return NULL;
    }

    ParseNode* roNode = RO(token);
    if (roNode != NULL) {
        addChild(node, roNode);
        *token = getToken(inputFile);
    } else {
        freeParseTree(node);
        return NULL;
    }

    ParseNode* expr2Node = expr(token, inputFile);
    if (expr2Node != NULL) {
        addChild(node, expr2Node);
    } else {
        freeParseTree(node);
        return NULL;
    }

    // Traversal and code gen
    newName(VAR);
    int hasLoaded = 0;
    int unaryUsed = 0;
    char lastUsed = 0;
    exprTraversal(expr1Node, &hasLoaded, &unaryUsed, &lastUsed);
    newName(VAR);
    hasLoaded = 0;
    exprTraversal(expr2Node, &hasLoaded, &unaryUsed, &lastUsed);

    if (strcmp(roNode->children[0]->label, "<") == 0) {

        newName(LABEL);
        fprintf(targetFile, "LOAD %s\n", variableEntries[VarCntr - 2].name);
        fprintf(targetFile, "SUB %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "BRPOS %s\n", labelNames[LabelCntr - 1]);
        fprintf(targetFile, "BRZERO %s\n", labelNames[LabelCntr - 1]);

    } else if (strcmp(roNode->children[0]->label, ">") == 0) {

        newName(LABEL);
        fprintf(targetFile, "LOAD %s\n", variableEntries[VarCntr - 2].name);
        fprintf(targetFile, "SUB %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "BRNEG %s\n", labelNames[LabelCntr - 1]);
        fprintf(targetFile, "BRZERO %s\n", labelNames[LabelCntr - 1]);

    } else if (strcmp(roNode->children[0]->label, "==") == 0) {

        newName(LABEL);
        fprintf(targetFile, "LOAD %s\n", variableEntries[VarCntr - 2].name);
        fprintf(targetFile, "SUB %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "BRNEG %s\n", labelNames[LabelCntr - 1]);
        fprintf(targetFile, "BRPOS %s\n", labelNames[LabelCntr - 1]);

    } else if (strcmp(roNode->children[0]->label, "=!=") == 0) {
        newName(LABEL);
        fprintf(targetFile, "LOAD %s\n", variableEntries[VarCntr - 2].name);
        fprintf(targetFile, "SUB %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "BRZERO %s\n", labelNames[LabelCntr - 1]);
    } else {
        newName(LABEL);
        newName(LABEL);
        newName(LABEL);
        newName(LABEL);
        fprintf(targetFile, "LOAD %s\n", variableEntries[VarCntr - 2].name);
        fprintf(targetFile, "DIV 2\n");
        fprintf(targetFile, "MULT 2\n");
        fprintf(targetFile, "SUB %s\n", variableEntries[VarCntr - 2].name);
        fprintf(targetFile, "BRZERO %s\n", labelNames[LabelCntr - 4]);
        fprintf(targetFile, "BR %s\n", labelNames[LabelCntr - 3]);
        fprintf(targetFile, "%s: LOAD %s\n", labelNames[LabelCntr - 4], variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "DIV 2\n");
        fprintf(targetFile, "MULT 2\n");
        fprintf(targetFile, "SUB %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "BRZERO %s\n", labelNames[LabelCntr - 2]);
        fprintf(targetFile, "BR %s\n", labelNames[LabelCntr - 1]);
        fprintf(targetFile, "%s: LOAD %s\n", labelNames[LabelCntr - 3], variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "DIV 2\n");
        fprintf(targetFile, "MULT 2\n");
        fprintf(targetFile, "SUB %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "BRZERO %s\n", labelNames[LabelCntr - 1]);
        fprintf(targetFile, "BR %s\n", labelNames[LabelCntr - 2]);
        fprintf(targetFile, "%s: NOOP\n", labelNames[LabelCntr - 2]);
    }

    if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, "]") != 0) {
        printf("PARSER ERROR: Expected ']' after 'if' condition at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }

    *token = getToken(inputFile);
    if (token->tokenID != KEYWORD_TK || strcmp(token->tokenInstance, "then") != 0) {
        printf("PARSER ERROR: Expected 'then' keyword after 'if' condition at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }
    addChild(node, createNode("then"));

    *token = getToken(inputFile);
    ParseNode* statNode = stat(token, inputFile);
    if (statNode != NULL) {
        addChild(node, statNode);
    } else {
        freeParseTree(node);
        return NULL;
    }

    fprintf(targetFile, "%s: NOOP\n", labelNames[LabelCntr - 1]);

    return node;
}

ParseNode* loop1(Token* token, FILE* inputFile) {
    if (token->tokenID != KEYWORD_TK || strcmp(token->tokenInstance, "while") != 0) {
        printf("PARSER ERROR: Expected 'while' keyword at line %d, char %d\n", token->lineNumber, token->charNumber);
        return NULL;
    }

    ParseNode* node = createNode("<loop1>");
    addChild(node, createNode("while"));

    *token = getToken(inputFile);
    if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, "[") != 0) {
        printf("PARSER ERROR: Expected '[' after 'while' at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }

    *token = getToken(inputFile);
    ParseNode* expr1Node = expr(token, inputFile);
    if (expr1Node != NULL) {
        addChild(node, expr1Node);

    } else {
        freeParseTree(node);
        return NULL;
    }

    ParseNode* roNode = RO(token);
    if (roNode != NULL) {
        addChild(node, roNode);
        *token = getToken(inputFile);
    } else {
        freeParseTree(node);
        return NULL;
    }

    ParseNode* expr2Node = expr(token, inputFile);
    if (expr2Node != NULL) {
        addChild(node, expr2Node);
    } else {
        freeParseTree(node);
        return NULL;
    }

    if (strcmp(token->tokenInstance, "]") != 0) {
        printf("PARSER ERROR: Expected ']' after 'while' condition at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }

    // Traversal and code gen
    newName(LABEL);
    newName(LABEL);
    char* originalLabel = labelNames[LabelCntr - 2];
    char* endLabel = labelNames[LabelCntr - 1];
    fprintf(targetFile, "%s: NOOP\n", originalLabel);
    newName(VAR);
    int hasLoaded = 0;
    int unaryUsed = 0;
    char lastUsed = 'o';
    exprTraversal(expr1Node, &hasLoaded, &unaryUsed, &lastUsed);
    newName(VAR);
    hasLoaded = 0;
    exprTraversal(expr2Node, &hasLoaded, &unaryUsed, &lastUsed);

    if (strcmp(roNode->children[0]->label, "<") == 0) {

        fprintf(targetFile, "LOAD %s\n", variableEntries[VarCntr - 2].name);
        fprintf(targetFile, "SUB %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "BRPOS %s\n", endLabel);

    } else if (strcmp(roNode->children[0]->label, ">") == 0) {

        fprintf(targetFile, "LOAD %s\n", variableEntries[VarCntr - 2].name);
        fprintf(targetFile, "SUB %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "BRNEG %s\n", endLabel);

    } else if (strcmp(roNode->children[0]->label, "==") == 0) {

        fprintf(targetFile, "LOAD %s\n", variableEntries[VarCntr - 2].name);
        fprintf(targetFile, "SUB %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "BRNEG %s\n", endLabel);
        fprintf(targetFile, "BRPOS %s\n", endLabel);

    } else if (strcmp(roNode->children[0]->label, "=!=") == 0) {
        fprintf(targetFile, "LOAD %s\n", variableEntries[VarCntr - 2].name);
        fprintf(targetFile, "SUB %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "BRZERO %s\n", endLabel);
    } else {
        newName(LABEL);
        char* evenLabel = labelNames[LabelCntr - 1];
        newName(LABEL);
        char* oddLabel = labelNames[LabelCntr - 1];
        newName(LABEL);
        char* endOfCheckLabel = labelNames[LabelCntr - 1];
        fprintf(targetFile, "LOAD %s\n", variableEntries[VarCntr - 2].name);
        fprintf(targetFile, "DIV 2\n");
        fprintf(targetFile, "MULT 2\n");
        fprintf(targetFile, "SUB %s\n", variableEntries[VarCntr - 2].name);
        fprintf(targetFile, "BRZERO %s\n", evenLabel);
        fprintf(targetFile, "BR %s\n", oddLabel);
        fprintf(targetFile, "%s: LOAD %s\n", evenLabel, variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "DIV 2\n");
        fprintf(targetFile, "MULT 2\n");
        fprintf(targetFile, "SUB %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "BRPOS %s\n", endLabel);
        fprintf(targetFile, "BRNEG %s\n", endLabel);
        fprintf(targetFile, "BR %s\n", endOfCheckLabel);
        fprintf(targetFile, "%s: LOAD %s\n", oddLabel, variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "DIV 2\n");
        fprintf(targetFile, "MULT 2\n");
        fprintf(targetFile, "SUB %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "BRZERO %s\n", endLabel);
        fprintf(targetFile, "BR %s\n", endOfCheckLabel);
        fprintf(targetFile, "%s: NOOP\n", endOfCheckLabel);
    }

    *token = getToken(inputFile);

    ParseNode* statNode = stat(token, inputFile);
    if (statNode != NULL) {
        addChild(node, statNode);
    } else {
        freeParseTree(node);
        return NULL;
    }

    fprintf(targetFile, "BR %s\n", originalLabel);
    fprintf(targetFile, "%s: NOOP\n", endLabel);

    return node;
}

ParseNode* loop2(Token* token, FILE* inputFile) {
    if (token->tokenID != KEYWORD_TK || strcmp(token->tokenInstance, "repeat") != 0) {
        printf("PARSER ERROR: Expected 'repeat' keyword at line %d, char %d\n", token->lineNumber, token->charNumber);
        return NULL;
    }

    ParseNode* node = createNode("<loop2>");
    addChild(node, createNode("repeat"));

    newName(LABEL);
    char* originalLabel = labelNames[LabelCntr - 1];
    fprintf(targetFile, "%s: NOOP\n", originalLabel);

    *token = getToken(inputFile);
    ParseNode* statNode = stat(token, inputFile);
    if (statNode != NULL) {
        addChild(node, statNode);
        *token = getToken(inputFile);
    } else {
        freeParseTree(node);
        return NULL;
    }


    if (token->tokenID != KEYWORD_TK || strcmp(token->tokenInstance, "until") != 0) {
        printf("PARSER ERROR: Expected 'until' keyword after 'repeat' at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }
    addChild(node, createNode("until"));

    *token = getToken(inputFile);
    if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, "[") != 0) {
        printf("PARSER ERROR: Expected '[' after 'until' at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }

    *token = getToken(inputFile);
    ParseNode* expr1Node = expr(token, inputFile);
    if (expr1Node != NULL) {
        addChild(node, expr1Node);
    } else {
        freeParseTree(node);
        return NULL;
    }

    ParseNode* roNode = RO(token);
    if (roNode != NULL) {
        addChild(node, roNode);
        *token = getToken(inputFile);
    } else {
        freeParseTree(node);
        return NULL;
    }

    ParseNode* expr2Node = expr(token, inputFile);
    if (expr2Node != NULL) {
        addChild(node, expr2Node);
    } else {
        freeParseTree(node);
        return NULL;
    }

    if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, "]") != 0) {
        printf("PARSER ERROR: Expected ']' after 'until' condition at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }

    // Traversal and code gen
    newName(VAR);
    int hasLoaded = 0;
    int unaryUsed = 0;
    char lastUsed = 'o';
    exprTraversal(expr1Node, &hasLoaded, &unaryUsed, &lastUsed);
    newName(VAR);
    hasLoaded = 0;
    exprTraversal(expr2Node, &hasLoaded, &unaryUsed, &lastUsed);

    if (strcmp(roNode->children[0]->label, "<") == 0) {

        fprintf(targetFile, "LOAD %s\n", variableEntries[VarCntr - 2].name);
        fprintf(targetFile, "SUB %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "BRPOS %s\n", originalLabel);
        fprintf(targetFile, "BRZERO %s\n", originalLabel);

    } else if (strcmp(roNode->children[0]->label, ">") == 0) {

        fprintf(targetFile, "LOAD %s\n", variableEntries[VarCntr - 2].name);
        fprintf(targetFile, "SUB %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "BRNEG %s\n", originalLabel);
        fprintf(targetFile, "BRZERO %s\n", originalLabel);

    } else if (strcmp(roNode->children[0]->label, "==") == 0) {

        fprintf(targetFile, "LOAD %s\n", variableEntries[VarCntr - 2].name);
        fprintf(targetFile, "SUB %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "BRPOS %s\n", originalLabel);
        fprintf(targetFile, "BRNEG %s\n", originalLabel);

    } else if (strcmp(roNode->children[0]->label, "=!=") == 0) {
        fprintf(targetFile, "LOAD %s\n", variableEntries[VarCntr - 2].name);
        fprintf(targetFile, "SUB %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "BRZERO %s\n", originalLabel);
    } else {
        newName(LABEL);
        char* evenLabel = labelNames[LabelCntr - 1];
        newName(LABEL);
        char* oddLabel = labelNames[LabelCntr - 1];
        newName(LABEL);
        char* endOfCheckLabel = labelNames[LabelCntr - 1];
        fprintf(targetFile, "LOAD %s\n", variableEntries[VarCntr - 2].name);
        fprintf(targetFile, "DIV 2\n");
        fprintf(targetFile, "MULT 2\n");
        fprintf(targetFile, "SUB %s\n", variableEntries[VarCntr - 2].name);
        fprintf(targetFile, "BRZERO %s\n", evenLabel);
        fprintf(targetFile, "BR %s\n", oddLabel);
        fprintf(targetFile, "%s: LOAD %s\n", evenLabel, variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "DIV 2\n");
        fprintf(targetFile, "MULT 2\n");
        fprintf(targetFile, "SUB %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "BRPOS %s\n", originalLabel);
        fprintf(targetFile, "BRNEG %s\n", originalLabel);
        fprintf(targetFile, "BR %s\n", endOfCheckLabel);
        fprintf(targetFile, "%s: LOAD %s\n", oddLabel, variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "DIV 2\n");
        fprintf(targetFile, "MULT 2\n");
        fprintf(targetFile, "SUB %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "BRZERO %s\n", originalLabel);
        fprintf(targetFile, "BR %s\n", endOfCheckLabel);
        fprintf(targetFile, "%s: NOOP\n", endOfCheckLabel);
    }

    return node;
}

ParseNode* assign(Token* token, FILE* inputFile) {
    ParseNode* node = createNode("<assign>");

    int set = 0;

    if (token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "set") == 0) {
        addChild(node, createNode("set"));
        *token = getToken(inputFile);
        set = 1;
    }

    char* idName = token->tokenInstance;

    if (token->tokenID == ID_TK) {

        int lookup = find(stack, token->tokenInstance);
        if (lookup == -1) {
            printf("STACK ERROR: Identifier '%s'not found in stack at line %d, char %d\n", token->tokenInstance, token->lineNumber, token->charNumber);
            freeParseTree(node);
            fclose(targetFile);
            remove("file.asm");
            remove("kb.asm");
            exit(1);
        }

        addChild(node, createNode(token->tokenInstance));

        *token = getToken(inputFile);

        if (strcmp(token->tokenInstance, "=") != 0) {
            printf("PARSER ERROR: Expected '=' after identifier in assignment at line %d, char %d\n", token->lineNumber, token->charNumber);
            freeParseTree(node);
            return NULL;
        }
        addChild(node, createNode("="));
        *token = getToken(inputFile);
    } else {
        printf("PARSER ERROR: Expected 'set' or identifier in assignment at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }


    if (set == 1) {
        ParseNode* exprNode = expr(token, inputFile);
        if (exprNode != NULL) {
            addChild(node, exprNode);
        } else {
            freeParseTree(node);
            return NULL;
        }

        // Traversal and code gen
        newName(VAR);
        int hasLoaded = 0;
        int unaryUsed = 0;
        char lastUsed = 'o';
        exprTraversal(exprNode, &hasLoaded, &unaryUsed, &lastUsed);
        fprintf(targetFile, "LOAD %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "STORE %s\n", idName);
    } else {
        fprintf(targetFile, "LOAD %s\n", token->tokenInstance);
        fprintf(targetFile, "STORE %s\n", idName);
        *token = getToken(inputFile);
    }

    return node;
}

ParseNode* RO(Token* token) {
    ParseNode* node = createNode("<RO>");

    if (token->tokenID == OPERATOR_TK && (
        strcmp(token->tokenInstance, "<") == 0 ||
        strcmp(token->tokenInstance, ">") == 0 ||
        strcmp(token->tokenInstance, "==") == 0 ||
        strcmp(token->tokenInstance, "=!=") == 0 ||
        strcmp(token->tokenInstance, "...") == 0 ||
        strcmp(token->tokenInstance, ">=") == 0 || 
        strcmp(token->tokenInstance, "<=") == 0)) {
        addChild(node, createNode(token->tokenInstance));
    } else {
        printf("PARSER ERROR: Expected relational operator at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }

    return node;
}

ParseNode* label(Token* token, FILE* inputFile) {
    if (token->tokenID != KEYWORD_TK || strcmp(token->tokenInstance, "label") != 0) {
        printf("PARSER ERROR: Expected 'label' keyword at line %d, char %d\n", token->lineNumber, token->charNumber);
        return NULL;
    }

    ParseNode* node = createNode("<label>");
    addChild(node, createNode("label"));

    *token = getToken(inputFile);
    if (token->tokenID != ID_TK) {
        printf("PARSER ERROR: Expected identifier after 'label' at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }

    int lookup = find(stack, token->tokenInstance);
    if (lookup == -1) {
        printf("STACK ERROR: Identifier '%s'not found in stack at line %d, char %d\n", token->tokenInstance, token->lineNumber, token->charNumber);
        freeParseTree(node);
        fclose(targetFile);
        remove("file.asm");
        remove("kb.asm");
        exit(1);
    }

    addChild(node, createNode(token->tokenInstance));

    fprintf(targetFile, "%s: NOOP\n", token->tokenInstance);

    return node;
}

ParseNode* gotoFunc(Token* token, FILE* inputFile) {
    if (token->tokenID != KEYWORD_TK || strcmp(token->tokenInstance, "jump") != 0) {
        printf("PARSER ERROR: Expected 'jump' keyword at line %d, char %d\n", token->lineNumber, token->charNumber);
        return NULL;
    }

    ParseNode* node = createNode("<goto>");
    addChild(node, createNode("jump"));

    *token = getToken(inputFile);
    if (token->tokenID != ID_TK) {
        printf("PARSER ERROR: Expected identifier after 'jump' at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }

    int lookup = find(stack, token->tokenInstance);
    if (lookup == -1) {
        printf("STACK ERROR: Identifier '%s'not found in stack at line %d, char %d\n", token->tokenInstance, token->lineNumber, token->charNumber);
        freeParseTree(node);
        fclose(targetFile);
        remove("file.asm");
        remove("kb.asm");
        exit(1);
    }

    fprintf(targetFile, "BR %s\n", token->tokenInstance);

    for (int i = 0; i < numJumpFuncs; i++) {
        if (strcmp(jumpFuncEntries[i].funcName, token->tokenInstance) == 0) {
            fprintf(targetFile, "%s: NOOP\n", jumpFuncEntries[i].jumpName);
        }
    }

    addChild(node, createNode(token->tokenInstance));

    return node;
}

ParseNode* pick(Token* token, FILE* inputFile) {

    if (token->tokenID != KEYWORD_TK || strcmp(token->tokenInstance, "pick") != 0) {
        printf("PARSER ERROR: Expected 'pick' keyword at line %d, char %d\n", token->lineNumber, token->charNumber);
        return NULL;
    }

    ParseNode* node = createNode("<pick>");
    addChild(node, createNode("pick"));

    *token = getToken(inputFile);
    ParseNode* exprNode = expr(token, inputFile);
    if (exprNode == NULL) {
        printf("PARSER ERROR: Invalid or missing expression after 'pick' at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }
    addChild(node, exprNode);

    // Traversal and code gen
    newName(VAR);
    int hasLoaded = 0;
    int unaryUsed = 0;
    char lastUsed = 0;
    exprTraversal(exprNode, &hasLoaded, &unaryUsed, &lastUsed);

    newName(LABEL);
    newName(LABEL);
    fprintf(targetFile, "LOAD %s\n", variableEntries[VarCntr - 1].name);
    fprintf(targetFile, "BRZERO %s\n", labelNames[LabelCntr - 1]);

    ParseNode* pickBodyNode = pickBody(token, inputFile);
    if (pickBodyNode == NULL) {
        printf("PARSER ERROR: Invalid or missing pick body after expression\n");
        freeParseTree(node);
        return NULL;
    }
    addChild(node, pickBodyNode);

    return node;
}

ParseNode* pickBody(Token* token, FILE* inputFile) {
    ParseNode* pickBodyNode = createNode("<pickbody>");

    ParseNode* firstStatNode = stat(token, inputFile);
    if (firstStatNode == NULL) {
        printf("PARSER ERROR: Expected valid statement in pickbody\n");
        freeParseTree(pickBodyNode);
        return NULL;
    }
    addChild(pickBodyNode, firstStatNode);

    fprintf(targetFile, "BR %s\n", labelNames[LabelCntr - 2]);

    if (strcmp(token->tokenInstance, ":") != 0) {
        printf("PARSER ERROR: Expected ':' after first statement in pickbody at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(pickBodyNode);
        return NULL;
    }

    fprintf(targetFile, "%s: ", labelNames[LabelCntr - 1]);

    *token = getToken(inputFile);
    ParseNode* secondStatNode = stat(token, inputFile);
    if (secondStatNode == NULL) {
        printf("PARSER ERROR: Expected valid statement after ':' in pickbody\n");
        freeParseTree(pickBodyNode);
        return NULL;
    }
    addChild(pickBodyNode, secondStatNode);

    fprintf(targetFile, "%s: NOOP\n", labelNames[LabelCntr - 2]);

    return pickBodyNode;
}

void exprTraversal(ParseNode* node, int* hasLoaded, int* unaryUsed, char* lastUsed) {

    if (node->numChildren > 0) {
        for (int i = 0; i < node->numChildren; i++) {
            exprTraversal(node->children[i], hasLoaded, unaryUsed, lastUsed);
        }
    }

    if (strcmp(node->label, "+") == 0) {
        fprintf(targetFile, "LOAD %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "ADD ");
        *lastUsed = 'a';
    } else if (strcmp(node->label, "-") == 0) {
        fprintf(targetFile, "LOAD %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "SUB ");
        *lastUsed = 's';
    } else if (strcmp(node->label, "*") == 0) {
        fprintf(targetFile, "LOAD %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "MULT ");
        *lastUsed = 'm';
    } else if (strcmp(node->label, "/") == 0) {
        fprintf(targetFile, "LOAD %s\n", variableEntries[VarCntr - 1].name);
        fprintf(targetFile, "DIV ");
        *lastUsed = 'd';
    } else if (strcmp(node->label, "^") == 0) {
            *unaryUsed = 1;
    } else if (strcmp(node->label, "<expr>") != 0
                && strcmp(node->label, "<N>") != 0
                && strcmp(node->label, "<expr'>") != 0
                && strcmp(node->label, "<A>") != 0
                && strcmp(node->label, "<N'>") != 0
                && strcmp(node->label, "<M>") != 0
                && strcmp(node->label, "<A'>") != 0
                && strcmp(node->label, "<R>") != 0
                && strcmp(node->label, "<expr>") != 0) {

                    if (*hasLoaded == 0) {
                        fprintf(targetFile, "LOAD %s\n", node->label);
                        if (*unaryUsed == 1) {
                            *unaryUsed = 0;
                            if (strcmp(node->label, "0") == 0) {
                                fprintf(targetFile, "ADD 1\n");
                            }
                            else if (strcmp(node->label, "1") == 0) {
                                fprintf(targetFile, "SUB 1\n");
                            } else {
                                    fprintf(targetFile, "MULT -1\n");
                                }
                            }

                            fprintf(targetFile, "STORE %s\n", variableEntries[VarCntr - 1].name);
                            *hasLoaded = 1;
                    } else {
                        if (*unaryUsed == 1) {
                            *unaryUsed = 0;
                            if (strcmp(node->label, "0") == 0) {
                                fprintf(targetFile, "ADD 1\n");
                            }
                            else if (strcmp(node->label, "1") == 0) {
                                fprintf(targetFile, "SUB 1\n");
                            } else {
                                if (*lastUsed == 'a') {
                                    fprintf(targetFile, "0\n");
                                    fprintf(targetFile, "SUB %s\n", node->label);
                                }
                                else if (*lastUsed == 's') {
                                    fprintf(targetFile, "0\n");
                                    fprintf(targetFile, "ADD %s\n", node->label);
                                } else {
                                    fprintf(targetFile, "MULT -1\n");
                                }
                            }
                            } else {
                                fprintf(targetFile, "%s\n", node->label);
                            }
                        fprintf(targetFile, "STORE %s\n", variableEntries[VarCntr - 1].name);
                    }
                }
}