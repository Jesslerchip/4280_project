// Jessica Seabolt CMP SCI 4280 Project Updated 04/12/2024

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

ParseNode* createNode(const char* label) {
    ParseNode* node = (ParseNode*)malloc(sizeof(ParseNode));
    strcpy(node->label, label);
    node->value[0] = '\0';
    node->numChildren = 0;
    return node;
}

void addChild(ParseNode* parent, ParseNode* child) {
    parent->children[parent->numChildren] = child;
    parent->numChildren++;
}

ParseNode* parser(FILE* inputFile) {
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

    ParseNode* varsNode = vars(token, inputFile);
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
            // *token = getToken(inputFile);
        } else {
            freeParseTree(node);
            return NULL;
        }
    } else {
        printf("PARSER ERROR: Expected 'tape' keyword at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
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
    addChild(node, createNode(token->tokenInstance));

    *token = getToken(inputFile);
    ParseNode* blockNode = block(token, inputFile);
    if (blockNode != NULL) {
        addChild(node, blockNode);
    } else {
        freeParseTree(node);
        return NULL;
    }

    return node;
}

ParseNode* block(Token* token, FILE* inputFile) {
    if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, "{") != 0) {
        printf("PARSER ERROR: Expected '{' at line %d, char %d\n", token->lineNumber, token->charNumber);
        return NULL;
    }

    ParseNode* node = createNode("<block>");

    *token = getToken(inputFile);
    ParseNode* varsNode = vars(token, inputFile);
    
    if (varsNode != NULL) {
        addChild(node, varsNode);
    }

    // printf("DEBUG BLOCK: Token: %s, Token ID: %d, Line: %d, Char: %d\n", token->tokenInstance, token->tokenID, token->lineNumber, token->charNumber);

    ParseNode* statsNode = stats(token, inputFile);

    // printf("DEBUG BLOCK: Token: %s, Token ID: %d, Line: %d, Char: %d\n", token->tokenInstance, token->tokenID, token->lineNumber, token->charNumber);

    if (statsNode != NULL) {
        addChild(node, statsNode);
        // *token = getToken(inputFile);
    } else {
        freeParseTree(node);
        return NULL;
    }

    if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, "}") != 0) {
        printf("PARSER ERROR: Expected '}' at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }

    return node;
}

ParseNode* vars(Token* token, FILE* inputFile) {
    ParseNode* node = createNode("<vars>");

    if (token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "create") == 0) {
        addChild(node, createNode("create"));

        *token = getToken(inputFile);

        if (token->tokenID != ID_TK) {
            printf("PARSER ERROR: Expected identifier after 'create' at line %d, char %d\n", token->lineNumber, token->charNumber);
            freeParseTree(node);
            return NULL;
        }

        addChild(node, createNode(token->tokenInstance));

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

            // printf("DEBUG VARS: Token: %s, Token ID: %d, Line: %d, Char: %d\n", token->tokenInstance, token->tokenID, token->lineNumber, token->charNumber);
            *token = getToken(inputFile);
            // printf("DEBUG VARS: Token: %s, Token ID: %d, Line: %d, Char: %d\n", token->tokenInstance, token->tokenID, token->lineNumber, token->charNumber);
        }

        if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, ";") != 0) {
            printf("PARSER ERROR: Expected ';' after variable declaration at line %d, char %d\n", token->lineNumber, token->charNumber);
            freeParseTree(node);
            return NULL;
        }

        *token = getToken(inputFile);

        if (token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "create") == 0) {
            ParseNode* varsNode = vars(token, inputFile);
            if (varsNode != NULL) {
                addChild(node, varsNode);
            }

        }

    }

    return node;
}

ParseNode* expr(Token* token, FILE* inputFile) {
    ParseNode* node = createNode("<expr>");

    ParseNode* nNode = N(token, inputFile);
    if (nNode != NULL) {
        addChild(node, nNode);
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

    if (token->tokenID == OPERATOR_TK && strcmp(token->tokenInstance, "-") == 0) {
        addChild(node, createNode("-"));
        *token = getToken(inputFile);

        ParseNode* exprNode = expr(token, inputFile);
        if (exprNode != NULL) {
            addChild(node, exprNode);
        } else {
            freeParseTree(node);
            return NULL;
        }
    }

    return node;
}

ParseNode* N(Token* token, FILE* inputFile) {
    ParseNode* node = createNode("<N>");

    ParseNode* aNode = A(token, inputFile);
    if (aNode != NULL) {
        addChild(node, aNode);
        // *token = getToken(inputFile);
    } else {
        freeParseTree(node);
        return NULL;
    }

    ParseNode* nPrimeNode = NPrime(token, inputFile);
    if (nPrimeNode != NULL) {
        addChild(node, nPrimeNode);
    }

    return node;
}

ParseNode* NPrime(Token* token, FILE* inputFile) {
    ParseNode* node = createNode("<N'>");

    if (token->tokenID == OPERATOR_TK && strcmp(token->tokenInstance, "/") == 0) {
        addChild(node, createNode("/"));
        *token = getToken(inputFile);

        ParseNode* aNode = A(token, inputFile);
        if (aNode != NULL) {
            addChild(node, aNode);
            *token = getToken(inputFile);
        } else {
            freeParseTree(node);
            return NULL;
        }

        ParseNode* nPrimeNode = NPrime(token, inputFile);
        if (nPrimeNode != NULL) {
            addChild(node, nPrimeNode);
        }
    } else if (token->tokenID == OPERATOR_TK && strcmp(token->tokenInstance, "+") == 0) {
        addChild(node, createNode("+"));
        *token = getToken(inputFile);

        ParseNode* aNode = A(token, inputFile);
        if (aNode != NULL) {
            addChild(node, aNode);
            *token = getToken(inputFile);
        } else {
            freeParseTree(node);
            return NULL;
        }

        ParseNode* nPrimeNode = NPrime(token, inputFile);
        if (nPrimeNode != NULL) {
            addChild(node, nPrimeNode);
        }
    }

    return node;
}

ParseNode* A(Token* token, FILE* inputFile) {
    ParseNode* node = createNode("<A>");

    ParseNode* mNode = M(token, inputFile);
    if (mNode != NULL) {
        addChild(node, mNode);
        // *token = getToken(inputFile);
    } else {
        freeParseTree(node);
        return NULL;
    }

    ParseNode* aPrimeNode = APrime(token, inputFile);
    if (aPrimeNode != NULL) {
        addChild(node, aPrimeNode);
    }

    return node;
}

ParseNode* APrime(Token* token, FILE* inputFile) {
    ParseNode* node = createNode("<A'>");

    if (token->tokenID == OPERATOR_TK && strcmp(token->tokenInstance, "*") == 0) {
        addChild(node, createNode("*"));
        *token = getToken(inputFile);

        ParseNode* aNode = A(token, inputFile);
        if (aNode != NULL) {
            addChild(node, aNode);
        } else {
            freeParseTree(node);
            return NULL;
        }
    }

    return node;
}

ParseNode* M(Token* token, FILE* inputFile) {
    ParseNode* node = createNode("<M>");

    if (token->tokenID == OPERATOR_TK && strcmp(token->tokenInstance, "^") == 0) {
        addChild(node, createNode("^"));
        *token = getToken(inputFile);

        ParseNode* mNode = M(token, inputFile);
        if (mNode != NULL) {
            addChild(node, mNode);
        } else {
            freeParseTree(node);
            return NULL;
        }
    } else {
        ParseNode* rNode = R(token, inputFile);
        if (rNode != NULL) {
            addChild(node, rNode);
        } else {
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

        ParseNode* exprNode = expr(token, inputFile);
        if (exprNode != NULL) {
            addChild(node, exprNode);
            *token = getToken(inputFile);
        } else {
            freeParseTree(node);
            return NULL;
        }

        if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, ")") != 0) {
            printf("PARSER ERROR: Expected ')' at line %d, char %d\n", token->lineNumber, token->charNumber);
            freeParseTree(node);
            return NULL;
        }
        
    } else if (token->tokenID == ID_TK) {
        addChild(node, createNode(token->tokenInstance));
    } else if (token->tokenID == INT_TK) {
        addChild(node, createNode(token->tokenInstance));
    } else {
        freeParseTree(node);
        return NULL;
    }

    return node;
}

ParseNode* stats(Token* token, FILE* inputFile) {
    ParseNode* node = createNode("<stats>");

    ParseNode* statNode = stat(token, inputFile);
    if (statNode != NULL) {
        addChild(node, statNode);
        *token = getToken(inputFile);

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
        *token = getToken(inputFile);

        ParseNode* mStatNode = mStat(token, inputFile);
        if (mStatNode != NULL) {
            addChild(node, mStatNode);
        }
    }

    return node;
}

ParseNode* stat(Token* token, FILE* inputFile) {
    ParseNode* node = createNode("<stat>");
    // printf("DEBUG STAT: Token: %s, Token ID: %d, Line: %d, Char: %d\n", token->tokenInstance, token->tokenID, token->lineNumber, token->charNumber);

    if (token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "cin") == 0) {
        ParseNode* inNode = in(token, inputFile);
        if (inNode != NULL) {
            addChild(node, inNode);

            *token = getToken(inputFile);

            if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, ";") != 0) {
                printf("PARSER ERROR: Expected ';' after 'cout' statement at line %d, char %d\n", token->lineNumber, token->charNumber);
                freeParseTree(node);
                return NULL;
            }

        } else {
            freeParseTree(node);
            return NULL;
        }
    } else if (token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "cout") == 0) {
        ParseNode* outNode = out(token, inputFile);
        if (outNode != NULL) {
            addChild(node, outNode);

            *token = getToken(inputFile);

            if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, ";") != 0) {
                printf("PARSER ERROR: Expected ';' after 'cout' statement at line %d, char %d\n", token->lineNumber, token->charNumber);
                freeParseTree(node);
                return NULL;
            }

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
                printf("PARSER ERROR: Expected ';' after 'cout' statement at line %d, char %d\n", token->lineNumber, token->charNumber);
                freeParseTree(node);
                return NULL;
            }

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
                printf("PARSER ERROR: Expected ';' after 'cout' statement at line %d, char %d\n", token->lineNumber, token->charNumber);
                freeParseTree(node);
                return NULL;
            }

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
                printf("PARSER ERROR: Expected ';' after 'cout' statement at line %d, char %d\n", token->lineNumber, token->charNumber);
                freeParseTree(node);
                return NULL;
            }


        } else {
            freeParseTree(node);
            return NULL;
        }
    } else if ((token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "set") == 0) || (token->tokenID == ID_TK)) {
        ParseNode* assignNode = assign(token, inputFile);
        if (assignNode != NULL) {
            addChild(node, assignNode);

            *token = getToken(inputFile);

            if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, ";") != 0) {
                printf("PARSER ERROR: Expected ';' after 'cout' statement at line %d, char %d\n", token->lineNumber, token->charNumber);
                freeParseTree(node);
                return NULL;
            }

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
                printf("PARSER ERROR: Expected ';' after 'cout' statement at line %d, char %d\n", token->lineNumber, token->charNumber);
                freeParseTree(node);
                return NULL;
            }

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
                printf("PARSER ERROR: Expected ';' after 'cout' statement at line %d, char %d\n", token->lineNumber, token->charNumber);
                freeParseTree(node);
                return NULL;
            }

        } else {
            freeParseTree(node);
            return NULL;
        }
    } else if (token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "pick") == 0) {
        ParseNode* pickNode = pick(token, inputFile);
        if (pickNode != NULL) {
            addChild(node, pickNode);

            *token = getToken(inputFile);

            if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, ";") != 0) {
                printf("PARSER ERROR: Expected ';' after 'cout' statement at line %d, char %d\n", token->lineNumber, token->charNumber);
                freeParseTree(node);
                return NULL;
            }

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
        *token = getToken(inputFile);
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
        *token = getToken(inputFile);
    } else {
        freeParseTree(node);
        return NULL;
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
        *token = getToken(inputFile);

        printf("DEBUG LOOP1: Token: %s, Token ID: %d, Line: %d, Char: %d\n", token->tokenInstance, token->tokenID, token->lineNumber, token->charNumber);

    } else {
        freeParseTree(node);
        return NULL;
    }

    ParseNode* roNode = RO(token);
    if (roNode != NULL) {
        addChild(node, roNode);
        *token = getToken(inputFile);

        printf("DEBUG LOOP1: Token: %s, Token ID: %d, Line: %d, Char: %d\n", token->tokenInstance, token->tokenID, token->lineNumber, token->charNumber);

    } else {
        freeParseTree(node);
        return NULL;
    }

    ParseNode* expr2Node = expr(token, inputFile);
    if (expr2Node != NULL) {
        addChild(node, expr2Node);
        *token = getToken(inputFile);
    } else {
        freeParseTree(node);
        return NULL;
    }

    printf("DEBUG LOOP1: Token: %s, Token ID: %d, Line: %d, Char: %d\n", token->tokenInstance, token->tokenID, token->lineNumber, token->charNumber);

    if (strcmp(token->tokenInstance, "]") != 0) {
        printf("PARSER ERROR: Expected ']' after 'while' condition at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }

    *token = getToken(inputFile);
    ParseNode* statNode = stat(token, inputFile);
    if (statNode != NULL) {
        addChild(node, statNode);
    } else {
        freeParseTree(node);
        return NULL;
    }

    return node;
}

ParseNode* loop2(Token* token, FILE* inputFile) {
    if (token->tokenID != KEYWORD_TK || strcmp(token->tokenInstance, "repeat") != 0) {
        printf("PARSER ERROR: Expected 'repeat' keyword at line %d, char %d\n", token->lineNumber, token->charNumber);
        return NULL;
    }

    ParseNode* node = createNode("<loop2>");
    addChild(node, createNode("repeat"));

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
        *token = getToken(inputFile);
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
        *token = getToken(inputFile);
    } else {
        freeParseTree(node);
        return NULL;
    }

    if (token->tokenID != OPERATOR_TK || strcmp(token->tokenInstance, "]") != 0) {
        printf("PARSER ERROR: Expected ']' after 'until' condition at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }

    return node;
}

ParseNode* assign(Token* token, FILE* inputFile) {
    ParseNode* node = createNode("<assign>");

    if (token->tokenID == KEYWORD_TK && strcmp(token->tokenInstance, "set") == 0) {
        addChild(node, createNode("set"));
        *token = getToken(inputFile);
    }

    printf("DEBUG ASSIGN: Token: %s, Token ID: %d, Line: %d, Char: %d\n", token->tokenInstance, token->tokenID, token->lineNumber, token->charNumber);

    if (token->tokenID == ID_TK) {
        addChild(node, createNode(token->tokenInstance));
        *token = getToken(inputFile);

        printf("DEBUG ASSIGN: Token: %s, Token ID: %d, Line: %d, Char: %d\n", token->tokenInstance, token->tokenID, token->lineNumber, token->charNumber);

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

    ParseNode* exprNode = expr(token, inputFile);
    if (exprNode != NULL) {
        addChild(node, exprNode);
    } else {
        freeParseTree(node);
        return NULL;
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
        strcmp(token->tokenInstance, "<=") == 0 ||
        strcmp(token->tokenInstance, ">=") == 0)) {
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
    addChild(node, createNode(token->tokenInstance));

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
    addChild(node, createNode(token->tokenInstance));

    return node;
}

ParseNode* pick(Token* token, FILE* inputFile) {
    // Check for the 'pick' keyword
    if (token->tokenID != KEYWORD_TK || strcmp(token->tokenInstance, "pick") != 0) {
        printf("PARSER ERROR: Expected 'pick' keyword at line %d, char %d\n", token->lineNumber, token->charNumber);
        return NULL;
    }

    // Create a node for the 'pick' construct
    ParseNode* node = createNode("<pick>");
    addChild(node, createNode("pick"));

    // Parse the expression after 'pick'
    *token = getToken(inputFile);
    ParseNode* exprNode = expr(token, inputFile);
    if (exprNode == NULL) {
        printf("PARSER ERROR: Invalid or missing expression after 'pick' at line %d, char %d\n", token->lineNumber, token->charNumber);
        freeParseTree(node);
        return NULL;
    }
    addChild(node, exprNode);

    // Parse the body of the 'pick' structure
    ParseNode* pickBodyNode = pickBody(inputFile);
    if (pickBodyNode == NULL) {
        printf("PARSER ERROR: Invalid or missing pick body after expression\n");
        freeParseTree(node);
        return NULL;
    }
    addChild(node, pickBodyNode);

    return node;
}

ParseNode* pickBody(FILE* inputFile) {
    ParseNode* pickBodyNode = createNode("<pickbody>");

    // Get the token for the first statement
    Token token = getToken(inputFile);

    // Parse the first statement
    ParseNode* firstStatNode = stat(&token, inputFile);
    if (firstStatNode == NULL) {
        printf("PARSER ERROR: Expected valid statement in pickbody\n");
        freeParseTree(pickBodyNode);
        return NULL;
    }
    addChild(pickBodyNode, firstStatNode);

    // Expecting a colon as a delimiter
    token = getToken(inputFile);
    if (strcmp(token.tokenInstance, ":") != 0) {
        printf("PARSER ERROR: Expected ':' after first statement in pickbody at line %d, char %d\n", token.lineNumber, token.charNumber);
        freeParseTree(pickBodyNode);
        return NULL;
    }

    // Parse the second statement
    token = getToken(inputFile);
    ParseNode* secondStatNode = stat(&token, inputFile);
    if (secondStatNode == NULL) {
        printf("PARSER ERROR: Expected valid statement after ':' in pickbody\n");
        freeParseTree(pickBodyNode);
        return NULL;
    }
    addChild(pickBodyNode, secondStatNode);

    return pickBodyNode;
}