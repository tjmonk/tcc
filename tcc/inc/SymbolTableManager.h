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

/*==============================================================================
  SymbolTableManager

  This module manages the symbol table.  The symbol table stores
  identifier information only - "smart" lexical analyser ; "dumb"
  Symbol Table.

  The Symbol Table Manager provides facilities to Lookup, Insert,
  and delete identifiers at different scope levels indicated by a scope
  ID.  Each procedure is assigned its own scope ID.  Scope level
  0 stores the procedure identifiers.

  Up to 1000 levels of scope are supported.  It is extremely unlikely
  that this number will be exceeded in normal use.

==============================================================================*/

#ifndef SYMBOL_TABLE_MANAGER_H
#define SYMBOL_TABLE_MANAGER_H

/*==============================================================================
        Includes
==============================================================================*/

#include <stdbool.h>

/*==============================================================================
        Public Definitions
==============================================================================*/

/*! identifier entry - pointer to this returned as the result of a lookup. */
struct identEntry
{
    /*! identifies which scope the identifier is associated with */
    int scopeID;

    /*! specifies the name of the identifier */
    char *name;

    /*! assigned */
    short assigned;

    /*! indicates if the identifier is associated with an
        external variable */
    bool isExternal;

    /*! identifier value */
    int value;

    /*! indicates the type of the variable */
    int type;

    /*! current registers associated with this identifier */
    int reg[2];

    /*! stack offset */
    int offset;

    /*! stack offset 2 */
    int offset2;

    /*! size on stack */
    int size;

    /*! string buffer ID */
    int stringBufID;

    /*! constant value? */
    bool constant;

    /*! ie_reserved */
    unsigned long ie_reserved;
};

/*==============================================================================
        Public Function Declarations
==============================================================================*/

void InitSymbolTable();
int CreateNewScopeLevel();
void SetScopeLevel( int level );
int GetScopeLevel();
struct identEntry *LookupID( char *item, bool replace_underscores );
struct identEntry *InsertID( char *item, int lineno, bool unique );
struct identEntry *InsertConstant( char *item, int type, int val );
void DeleteAll();

#endif
