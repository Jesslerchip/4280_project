#ifndef PARSENODE_H
#define PARSENODE_H

#define MAX_CHILDREN 6 // Any production rule in the grammar has at most 6 children, counting keywords and operators

typedef struct ParseNode {
    char label[50];
    struct ParseNode* children[MAX_CHILDREN];
    int numChildren;
} ParseNode;

#endif