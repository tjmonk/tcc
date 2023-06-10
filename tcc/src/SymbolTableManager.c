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
	and delete identifiers at different scope levels.

	Up to 16 levels of scope are supported.  It is extremely unlikely
	that this number will be exceeded in normal use.

==============================================================================*/

/*!
 * @defgroup symboltablemanager symboltablemanager
 * @brief Symbol Table Manager
 * @{
 */

/*==============================================================================
        Includes
==============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SymbolTableManager.h"

/*==============================================================================
        Private Definitions
==============================================================================*/

/*! Binary Tree Node */
struct ie_Node
{
	/*! pointer to the identifier entry object for an identifier */
    struct identEntry *idEntry;

	/*! pointer to the binary tree of all identifiers
	    which are lexicographically less than this node */
    struct ie_Node *left;

	/*! pointer to the binary tree of all identifiers
	    which are lexicographically greater than this node */
    struct ie_Node *right;
};

/*==============================================================================
        Private Function Declarations
==============================================================================*/

static void replaceUnderscores( char *item );
static struct identEntry *insert( struct ie_Node **ieNode, char *ident );
static struct ie_Node *newNode( char *ident );
static void delete( struct ie_Node *ieNode );

/*==============================================================================
        File Scoped Variables
==============================================================================*/

/*! track scope level */
static int scopeLevel = 0;

/*! track max scope level */
static int maxLevel = 0;

/* symbol tables - one for each level of scope */
static struct ie_Node *table[1000];

/*==============================================================================
        Public Function Definitions
==============================================================================*/

/*============================================================================*/
/*  InitSymbolTable                                                           */
/*!
    Initialize the symbol table

	The InitSymbolTable function initializes an empty symbol table.
    The scope pointers are initialized to NULL

==============================================================================*/
void InitSymbolTable()
{
	int i;

	for ( i=0; i<1000; i++ )
	{
		table[i] = NULL;
	}
}

/*============================================================================*/
/*  CreateNewScopeLevel                                                       */
/*!
    Increase the max scope level

	The CreateNewScopeLevel functin creates a new scope level.
    This is necessary when a new function is encountered during parsing.
    The scope level is created for the new function.

    @retval the new scope level

==============================================================================*/
int CreateNewScopeLevel()
{
	return ++maxLevel;
}

/*============================================================================*/
/*  SetScopeLevel                                                             */
/*!
    Set the current active scope level

	The SetScopeLevel function sets the current active scope level.

    @param[in]
        level
            the new scope level to set

==============================================================================*/
void SetScopeLevel(int level)
{
	scopeLevel = level;
}

/*============================================================================*/
/*  GetScopeLevel                                                             */
/*!
    Get the current active scope level

	The GetScopeLevel function gets the current active scope level.

    @retval the current action scope level

==============================================================================*/
int GetScopeLevel()
{
	return scopeLevel;
}

/*============================================================================*/
/*  LookupID                                                                  */
/*!
    Look up an ID in the symbol table

	The LookupID function looks up the specified identifier at the current scope
	level.  If one is found, a pointer to its identifier entry is
	returned.  If it is not found, the NULL pointer is returned.

    @param[in]
        item
            pointer to the identifier name

    @param[in]
        replace_underscores
            true : replace underscores in the item name with forward slashes
            before searching for it in the symbol table.

    @retval pointer to the identEntry object associated with the item
    @retval NULL if the identEntry object was not found

==============================================================================*/
struct identEntry *LookupID( char *item, bool replace_underscores )
{
	struct ie_Node *temp = NULL;
	struct identEntry *idEntry = NULL;
	struct identEntry *foundEntry = NULL;
	if( item != NULL )
	{
		if( replace_underscores == true )
		{
			/* replace two underscores with a forward slash */
			replaceUnderscores( item );
		}

		temp = table[scopeLevel];

		while ( temp != NULL )
		{
			idEntry = temp->idEntry;
			if( idEntry != NULL )
			{
				if ( strcmp( item, idEntry->name ) == 0 )
				{
					/* found it */
					foundEntry = idEntry;
					break;
				}

				if ( strcmp( item, idEntry->name ) > 0 )
				{
					temp = temp->right;
				}
				else
				{
					temp = temp->left;
				}
			}
		}
	}

	return foundEntry;
}

/*============================================================================*/
/*  InsertID                                                                  */
/*!
    Insert an ID into the symbol table

	The InsertID function looks up the specified identifier in the table at
	the current scope level.  If it is found, a pointer to it is returned
	to the caller, and no further action is taken. If it is not found
	it is inserted into the symbol table tree at the current scope
	level.

    @param[in]
        item
            pointer to the identifier name

    @retval pointer to the identEntry object associated with the item
    @retval NULL if the id could not be inserted into the symbol table

==============================================================================*/
struct identEntry *InsertID( char *item )
{
	struct identEntry *idEntry;

	idEntry = LookupID( item, true );
	if ( idEntry == NULL )
    {
		idEntry = insert( &table[scopeLevel] , item );
	}

	return( idEntry );
}

/*============================================================================*/
/*  InsertConstant                                                            */
/*!
    Insert a constant into the symbol table

	The InsertConstant function inserts a new constant variable at scope level 0
	of the SymbolTable storage.

	It sets the constant flag of the identEntry object to true; stores the
	constant value in the value field of the identEntry object; and sets
	the type to the specified type

    @param[in]
        item
            pointer to the identifier name

    @param[in]
        type
            specify the type of the constant

    @param[in]
        val
            integer value of the constant

    @retval pointer to the identEntry object associated with the item
    @retval NULL if the id could not be inserted into the symbol table

==============================================================================*/
struct identEntry *InsertConstant( char *item, int type, int val )
{
	struct identEntry *idEntry;

	if( item != NULL )
	{
		idEntry = LookupID( item, false );
		if( idEntry == NULL )
		{
			idEntry = insert( &table[0], item );
			if( idEntry != NULL )
			{
				idEntry->constant = true;
				idEntry->type = type;
				idEntry->value = val;
			}
		}
	}

	return idEntry;
}

/*==============================================================================
        Private Function Definitions
==============================================================================*/

/*============================================================================*/
/*  replaceUnderscores                                                        */
/*!
    Replace underscores with forward slashes

	The replaceUnderscores function performs an in-place replacement of
    __ with / in the specified string.

    @param[in]
        item
            pointer to the identifier name

==============================================================================*/
static void replaceUnderscores( char *item )
{
	int i=0;
	int j=0;
	int count = 0;

	if ( item != NULL )
	{
		while ( item[i] != 0 )
		{
			if ( item[i] == '_' )
			{
				if ( ++count == 2 )
				{
					/* two underscores detected */
					/* replace with a single forward slash */
					item[j++] = '/';
					count = 0;
				}
			}
			else if ( count == 1 )
			{
				/* single underscore detected */
				item[j++] = '_';
				item[j++] = item[i];
				count=0;
			}
			else
			{
				/* normal character - no special processing */
				item[j++] = item[i];
			}

			i++;
		}

		/* nul terminate */
		item[j++] = '\0';
	}
}

/*============================================================================*/
/*  insert                                                                    */
/*!
    Insert a new identifier node

	The insert function creates a new identifier node and stores a reference
    to it at the specified ieNode refernce.  The identifier must not
    already exist.

    @param[in,out]
        ieNode
            pointer to a location to store the identifier node reference

    @param[in]
        ident
            pointer to the identifier

    @retval pointer to the identEntry object that was created

==============================================================================*/
static struct identEntry *insert( struct ie_Node **ieNode, char *ident )
{
	struct identEntry *idEntry = NULL;

	if (*ieNode == NULL)
	{
		*ieNode = newNode(ident);

		if (*ieNode)
		{
			idEntry = (struct identEntry *)
                        ((struct ie_Node *)(*ieNode)->idEntry);
		}

		return(idEntry);
	}

	idEntry = (*ieNode)->idEntry;

	if (strcmp(ident,idEntry->name) < 0)
		idEntry = (struct identEntry *)insert(&((*ieNode)->left),ident);
	else
		idEntry = (struct identEntry *)insert(&((*ieNode)->right),ident);

	return(idEntry);
}

/*============================================================================*/
/*  newNode                                                                   */
/*!
    Generate a new identifier node

	The newNode function creates a new identifier node

    @param[in]
        ident
            pointer to the identifier

    @retval pointer to the created ie_Node object
    @retval NULL if the node could not be created

==============================================================================*/
static struct ie_Node *newNode( char *ident )
{
	struct ie_Node *ieNode = NULL;
	struct identEntry *idEntry = NULL;
	char *id = NULL;

	if( ident != NULL )
	{
		ieNode = (struct ie_Node *)calloc(1, sizeof( struct ie_Node ) );
		if( ieNode != NULL )
		{
			idEntry = (struct identEntry *)
						calloc( 1, sizeof( struct identEntry ) );

			if( idEntry != NULL )
			{
                idEntry->name = strdup( ident );
                idEntry->reg[0] = -1;
                idEntry->reg[1] = -1;

                ieNode->idEntry = idEntry;
                ieNode->left = NULL;
                ieNode->right = NULL;
			}
			else
			{
				free( ieNode );
				ieNode = NULL;
			}
		}
	}

	return( ieNode );
}

/*============================================================================*/
/*  DeleteAll                                                                 */
/*!
    Delete all at current scope

	The DeleteAll function deletes all identifiers at the current scope level.
	If the given scope level contains no identifiers then no action
	is taken.

==============================================================================*/
void DeleteAll()
{
	delete(table[scopeLevel]);

	table[scopeLevel] = NULL;
}

/*============================================================================*/
/*  delete                                                                    */
/*!
    Delete the specified ieNode

	The Delete function deletes the given identifier entry node, and all it's
	children recursively.  All memory used by the node is released.

    The given ieNode pointer will be invalid after this call and should
    not be used

    @param[in]
        ieNode
            pointer to the ieNode object to recursively delete

==============================================================================*/
static void delete( struct ie_Node *ieNode )
{
	struct identEntry *idEntry;

	if ( ieNode != NULL )
	{
		idEntry = ieNode->idEntry;
		if( idEntry != NULL )
		{
			if( idEntry->name != NULL )
			{
				/* free id name memory */
				free( idEntry->name );
				idEntry->name = NULL;
			}

			free( idEntry );
		}

		/* delete left child */
		delete(ieNode->left);

		/* delete right child */
		delete(ieNode->right);

		/* free memory used by this node */
		free( ieNode );
	}
}

/*! @}
 * end of symboltablemanager group */