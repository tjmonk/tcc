/*==============================================================================
MIT License

Copyright (c) 2023 Trevor Monk

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
==============================================================================*/

#ifndef NODE_H
#define NODE_H

/*==============================================================================
        Includes
==============================================================================*/

#include "SymbolTableManager.h"

/*==============================================================================
        Public Definitions
==============================================================================*/

/*! The Node object is used to construct a binary tree during parsing,
which when traversed is converted into assembly language depending
on the node type */
struct Node
{
    /*! Node Type */
    int type;

    /*! data Type */
    int datatype;

    /*! data value */
    int value;

    /*! floating point data value */
    float fvalue;

    /*! pointer to an identifier entry */
    struct identEntry *ident;

    /*! pointer to the node's left subtree */
    struct Node *left;

    /*! pointer to the node's right subtree */
    struct Node *right;
};

/*==============================================================================
        Public Function Declarations
==============================================================================*/

struct Node *createNode( int type, struct Node *left, struct Node *right );
void updateNode( struct Node *pNode, struct Node *left, struct Node *right );
void parseTree(struct Node *root, int lvl);

#endif