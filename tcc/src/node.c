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

/*!
 * @defgroup node node
 * @brief Parser Node Management
 * @{
 */

/*==============================================================================
        Includes
==============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"
#include "tcc.tab.h"

/*==============================================================================
        Public function definitions
==============================================================================*/

/*============================================================================*/
/*  createNode                                                                */
/*!
    Create and populate a new node

    The createNode function allocates a new Node object and populates
    the node type and left and right children.

    @param[in]
        type
            specifies the node type

    @param[in]
        left
            pointer to the left childnode to attach to the new parent node

    @param[in]
        right
            pointer to the right child node to attach to the new parent node


    @retval pointer to the newly created node
    @retval NULL if memory allocation failed

==============================================================================*/
struct Node *createNode( int type, struct Node *left, struct Node *right )
{
    struct Node *temp = NULL;

    temp = (struct Node *)calloc(1, sizeof(struct Node));
    if ( temp != (struct Node *)NULL )
    {
        /* initialize node values */
        temp->type = type;
        temp->value = 0;
        temp->left = left;
        temp->right = right;
    }

    return temp;
}

/*============================================================================*/
/*  updateNode                                                                */
/*!
    Update the children of the node

	The updateNode function sets the left and right children of
	the specified node.

    @param[in]
        pNode
            pointer to the node to update

    @param[in]
        left
            pointer to the left childnode to attach to the new parent node

    @param[in]
        right
            pointer to the right child node to attach to the new parent node

==============================================================================*/
void updateNode( struct Node *pNode, struct Node *left, struct Node *right )
{
	if( pNode != NULL )
	{
		pNode->left = left;
		pNode->right = right;
	}
}

/*============================================================================*/
/*  parseTree                                                                 */
/*!
    Display the parse tree

	The parseTree function recursively displays the parse tree including
    identifier names

    @param[in]
        root
            pointer to the root node to display

    @param[in]
        lvl
            the current recurse level used for managing indentation

==============================================================================*/
void parseTree(struct Node *root, int lvl)
{
    int i;
    struct identEntry *idEntry;

    if ( root == (struct Node *)NULL )
    {
        return;
    }

    printf("\n");

    for (i=0;i<lvl;i++)
    {
        printf("    ");
    }
    printf("(");

    switch(root->type)
    {
        case PROGRAM:
        	printf("program");
        	break;

        case FUNC_DEF_LIST:
        	printf("fd_List");
        	break;

        case FUNC_DEF:
        	printf("funcDef");
        	break;

        case FUNC_DEF1:
        	printf("funcDef1");
        	break;

        case ID:
        	idEntry = root->ident;
        	if( idEntry != NULL )
        	{
        		printf("ID %s",idEntry->name);
        	}
        	else
        	{
        		printf("ID <unknown>\n");
        	}
        	break;

        case LVAL_ID:
        	idEntry = root->ident;
        	if( idEntry != NULL )
        	{
        		printf("LVAL_ID %s",idEntry->name);
        	}
        	else
        	{
        		printf("LVAL_ID <unknown>\n");
        	}
        	break;

        case EXTERN_LVAL_ID:
        	idEntry = root->ident;
        	if( idEntry != NULL )
        	{
        		printf("EXTERN ID %s",idEntry->name);
        	}
        	else
        	{
        		printf("EXTERN ID <unknown>\n");
        	}
        	break;

        case FUNC_ID:
        	idEntry = root->ident;
        	if( idEntry != NULL )
        	{
	        	printf("FUNC_ID %s",idEntry->name);
        	}
        	else
        	{
	        	printf("FUNC_ID unknown");
        	}

        	break;

        case INVOKATION_ID:
        	idEntry = root->ident;
        	printf("INVOKATION_ID %s",idEntry->name);
        	break;

        case PARAM_ID:
        	idEntry = root->ident;
        	printf("PARAM_ID %s",idEntry->name);
        	break;

        case DECL_ID:
        	idEntry = root->ident;
        	printf("DECL_ID %s",idEntry->name);
        	break;

        case NUM:
        	printf("%d",root->value);
        	break;

        case CHARSTR:
        	idEntry = root->ident;
        	printf("%s",idEntry->name);
        	break;

        case FUNC_HDR:
        	printf("funcHdr");
        	break;

        case FUNC_HDR1:
        	printf("funcHdr1");
        	break;

        case STAT_LIST:
        	printf("statList");
        	break;

        case COMP_STAT:
        	printf("compStat");
        	break;

        case IF:
        	printf("if");
        	break;

        case ELSE:
        	printf("else");
        	break;

    	case WHILE:
    		printf("while");
    		break;

    	case CASE:
    		printf("case");
    		break;

    	case CASE1:
    		printf("case1");
    		break;

    	case DEFAULT:
    		printf("default");
    		break;

        case FOR:
        	printf("for");
        	break;

        case FOR1:
        	printf("for1");
        	break;

        case FOR2:
        	printf("for2");
        	break;

        case RETURN:
        	printf("return");
        	break;

        case BREAK:
        	printf("break");
        	break;

        case WRITE:
        	printf("write");
        	break;

		case FILE_WRITE:
			printf("file_write");
			break;

        case WRITELN:
        	printf("writeln");
        	break;

        case READ:
        	printf("read");
        	break;

		case FILE_READ:
			printf("file_read");
			break;

        case READLN:
        	printf("readln");
        	break;

		case SYSTEM:
			printf("system");
			break;

		case FILE_OPEN:
			printf("file_open");
			break;

		case FILE_CLOSE:
			printf("file_close");
			break;

		case OPEN_PRINT_SESSION:
			printf("open_print_session");
			break;

		case CLOSE_PRINT_SESSION:
			printf("close_print_session");
			break;

        case ASSIGN:
        	printf("=");
        	break;

        case TIMES_EQUALS:
        	printf("*=");
        	break;

        case DIV_EQUALS:
        	printf("/=");
        	break;

        case PLUS_EQUALS:
        	printf("+=");
        	break;

        case MINUS_EQUALS:
        	printf("-=");
        	break;

        case AND_EQUALS:
        	printf("&=");
        	break;

        case OR_EQUALS:
        	printf("|=");
        	break;

        case XOR_EQUALS:
        	printf("^=");
        	break;

        case ARRAY:
        	printf("array");
        	break;

        case ARRAY_DECL:
        	printf("arrayDecl");
        	break;

    	case EXTERN_DECL_LIST:
    		printf("externDeclList");
    		break;

        case DECL_LIST:
        	printf("declList");
        	break;

    	case EXTERN_DECLN:
    		printf("externDecln");
    		break;

        case DECLN:
        	printf("decln");
        	break;

        case DECLN_LIST:
        	printf("declnList");
        	break;

        case PROC_CALL:
        	printf("proc");
        	break;

        case ARG_LIST:
        	printf("argList");
        	break;

        case PARAM_LIST:
        	printf("paramlist");
        	break;

        case PARAMETER:
        	printf("parameter");
        	break;

        case INPUT_LIST:
        	printf("inputlist");
        	break;

        case OUTPUT_LIST:
        	printf("outputlist");
        	break;

    	case APPEND_LIST:
    		printf("appendlist");
    		break;

        case TYPE_INT:
        	printf("type_int");
        	break;

        case TYPE_FLOAT:
        	printf("type_float");
        	break;

        case TYPE_BOOL:
        	printf("type_bool");
        	break;

        case TYPE_CHAR:
        	printf("type_char");
        	break;

        case TYPE_STRING:
        	printf("type_string");
        	break;

        case OR:
        	printf("||");
        	break;

        case AND:
        	printf("&&");
        	break;

        case XOR:
        	printf("^");
        	break;

        case BOR:
        	printf("|");
        	break;

        case BAND:
        	printf("&");
        	break;

		case NOT:
			printf("!");
			break;

        case EQUALS:
        	printf("==");
        	break;

        case NOTEQUALS:
        	printf("!=");
        	break;

        case LTE:
        	printf("<=");
        	break;

        case GTE:
        	printf(">=");
        	break;

        case LT:
        	printf("<");
        	break;

        case GT:
        	printf(">");
        	break;

        case RSHIFT:
        	printf(">>");
        	break;

        case LSHIFT:
        	printf("<<");
        	break;

        case INC:
        	printf("++");
        	break;

        case DEC:
        	printf("--");
        	break;

        case PLUS:
        	printf("+");
        	break;

        case MINUS:
        	printf("-");
        	break;

        case TIMES:
        	printf("*");
        	break;

        case DIVIDE:
        	printf("/");
        	break;

    	case APPEND:
    		printf("APPEND");
    		break;

		case LENGTH:
			printf("LENGTH");
			break;

		case SETAT:
			printf("SETAT");
			break;

		case SETAT1:
			printf("SETAT1");
			break;

		case CHARAT:
			printf("CHARAT");
			break;

    	case DELAY:
    		printf("DELAY");
    		break;

		case WAITSIG:
			printf("WAITSIG");
			break;

		case NOTIFY:
			printf("NOTIFY");
			break;

		case VALIDATE_START:
			printf("VALIDATE_START");
			break;

		case VALIDATE_END:
			printf("VALIDATE_END");
			break;

		case SETTIMER:
			printf("SETTIMER");
			break;

		case CLEARTIMER:
			printf("CLEARTIMER");
			break;

		case HANDLE:
			printf("HANDLE");
			break;

    	case SWITCH:
    		printf("SWITCH");
    		break;

    	case TO_FLOAT:
    		printf("TO_FLOAT");
    		break;

    	case TO_INT:
    		printf("TO_INT");
    		break;

    	case FLOAT:
    		printf("%e", root->fvalue);
    		break;

        default:
        	printf("NARG: %d", root->type);
        	break;
    }

    parseTree(root->left,lvl+1);
    parseTree(root->right,lvl+1);

    printf(")");
}


/*! @}
 * end of node group */