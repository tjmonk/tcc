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
 * @defgroup typecheck typecheck
 * @brief Type Checker functions
 * @{
 */

/*==============================================================================
        Includes
==============================================================================*/
#include <stdio.h>
#include "typecheck.h"
#include "lineno.h"
#include "tcc.tab.h"

/*==============================================================================
        File Scoped Variables
==============================================================================*/

/*! error flag*/
static bool errorFlag = false;

/*! map type error to error message */
static char *typeErr[eMAX_TYPE_ERR] =
{
    "",
    "value out of range",
    "incompatible assignment types",
    "incompatible operand types for logical operator",
    "incompatible operand types for bitwise operator",
    "incompatible operand types for relational operator",
    "incompatible operand types for shift operator",
    "type can't be incremented/decremented",
    "incompatible operand types for arithmetic operator",
    "method cannot be used with this argument type",
    "cannot convert type to float",
    "cannot convert type to int",
    "cannot directly assign one extern variable to another"
};

/*==============================================================================
        Private Function Declarations
==============================================================================*/

static void typeError(teTYPE_ERR errnum );

/*==============================================================================
        Public Function Definitions
==============================================================================*/

bool TypeErrorDetected( void )
{
    return errorFlag;
}

/*============================================================================*/
/*  TypeCheck                                                                 */
/*!
	Perform Type Checking

	The TypeCheck function matches types on the left hand side and right hand
    side of the specified operation.  If the types are compatible, the
    type is returned as the result.  If the types are incompatible, or
    incompatible with the specified operation, then the type TYPE_INVALID
    is returned.

    @param[in]
        root
			pointer to the root node to type check

    @param[in]
        level
            indent level

    @param[in]
        debug
            true: perform type check debugging
            false: no debugging

    @retval the type of the result of the operation

==============================================================================*/
int TypeCheck(struct Node *root, int level, bool debug)
{
    int type1 = TYPE_INVALID;
    int type2 = TYPE_INVALID;
    int i;

    if( debug == true )
    {
	    if( level == 0 )
	    {
	    	printf("Start of TypeCheck\n");
	    }
	}

    if( root == NULL )
    {
    	if( debug == true )
    	{
		    for(i=0;i<level;i++) fprintf(stdout, "    ");
	    	printf("TypeCheck: nothing to do \n");
    	}
    	return TYPE_INVALID;
    }

	if( debug == true )
	{
	    for(i=0;i<level;i++) fprintf(stdout, "    ");
	    printf("TypeCheck: root->type = %d\n", root->type);
	}

    if ( root->type == NUM )
    {
    	if( ( root->value >= -128 ) && ( root->value <= 127 ) )
    	{
    	    return(TYPE_CHAR);
    	}

    	else if ( ( root->value >= -2147483648LL ) &&
                  ( root->value <= 2147483647LL ) )
    	{
    	    return(TYPE_INT);
    	}
    	else
    	{
    	    typeError( eVALUE_OUT_OF_RANGE );
    	    return(TYPE_INVALID);
    	}
    }

    if( root->type == FLOAT )
    {
    	return( TYPE_FLOAT );
    }

	if( root->type == LENGTH )
	{
		return( TYPE_INT );
	}

	if( root->type == HANDLE )
	{
		return( TYPE_INT );
	}

	if( root->type == SYSTEM )
	{
		return( TYPE_INT );
	}

	if( root->type == CHARAT )
	{
		return( TYPE_CHAR );
	}

    if ( ( root->type == ID ) || ( root->type == VAR_ID ) ||
	     ( root->type == PARAM_ID ) || ( root->type == INVOKATION_ID ) ||
	     ( root->type == LVAL_ID ) || ( root->type == EXTERN_LVAL_ID ) )
    {
    	if( root->ident == NULL )
    	{
			if( debug == true )
			{
			    for(i=0;i<level;i++) fprintf(stdout, "    ");
	    		printf("cannot get type from ID\n");
	    	}

    		return TYPE_INVALID;
    	}

		if( debug == true )
		{
		    for(i=0;i<level;i++) fprintf(stdout, "    ");
		    printf("root->ident->type = %d\n", root->ident->type);
		}

	   return(root->ident->type);
    }

    if ( root->type == TYPE_STRING )
    {
    	return(TYPE_STRING);
    }

    if( root->type == TYPE_FLOAT )
    {
    	return( TYPE_FLOAT);
    }

    if (root->left)
    {
        type1 = TypeCheck( root->left, level+1, debug );
		if( debug == true )
		{
		    for(i=0;i<level;i++) fprintf(stdout, "    ");
		    printf("type_left=%d\n", type1);
		}
    }

    if (root->right)
    {
        type2 = TypeCheck( root->right, level+1, debug );
		if( debug == true )
		{
		    for(i=0;i<level;i++) fprintf(stdout, "    ");
		    printf("type_right=%d\n", type2);
		}
    }

    if ( ( !( root->left ) ) && ( root->right ))
    {
        type1 = type2;
    }

    if ( ( root->left ) && ( !( root->right ) ) )
    {
        type2 = type1;
    }

    if ( ( ! root->left ) && ( ( ! ( root->right ) ) ) )
    {
        type1 = type2 = TYPE_INVALID;
    }

    if ( ( type1 == TYPE_INVALID ) || ( type2 == TYPE_INVALID ) )
    {
		if( debug == true )
		{
		    for(i=0;i<level;i++) fprintf(stdout, "    ");
	    	printf("type is invalid\n");
		}

	   	return(TYPE_INVALID);
    }

    switch(root->type)
    {
        case ASSIGN:
        	if (type1 == type2)
        	{
        	    return ( type1 );
        	}
        	else if ( ( type1 == TYPE_INT ) &&
					  ( type2 == TYPE_BOOL ) )
        	{
        	    return( TYPE_INT );
        	}
        	else if ( ( type1 == TYPE_INT ) &&
					  ( type2 == TYPE_CHAR ) )
        	{
        	    return( TYPE_CHAR );
        	}
        	else if ( ( type1 == TYPE_CHAR ) &&
					  ( type2 == TYPE_BOOL ) )
        	{
        	    return(TYPE_CHAR);
        	}
        	else if( ( type1 == TYPE_BOOL ) &&
					 ( type2 == TYPE_CHAR ) )
        	{
        	   return( TYPE_BOOL );
        	}
        	else if ( ( type1 == TYPE_BOOL ) &&
					  ( type2 == TYPE_CHAR ) )
        	{
        		return( TYPE_BOOL );
        	}
        	else
        	{
        	    typeError( eINCOMPATIBLE_ASSIGNMENT_TYPES );
        	    return ( TYPE_INVALID );
        	}
        	break;

    	case APPEND:
    		if( type1 != TYPE_STRING )
    		{
    			typeError( eINVALID_METHOD_FOR_ARGUMENT_TYPE );
    			return ( TYPE_INVALID );
    		}
    		break;

    	case TO_FLOAT:
    		if( ( type2 == TYPE_INT ) ||
				( type2 == TYPE_CHAR ) )
    		{
    			return TYPE_FLOAT;
    		}
    		else
    		{
        	    typeError ( eCANNOT_CONVERT_TO_FLOAT_TYPE );
        	    return ( TYPE_INVALID );
    		}
    		break;

    	case TO_INT:
    		if( type2 == TYPE_FLOAT )
    		{
    			return TYPE_INT;
    		}
    		else
    		{
        	    typeError ( eCANNOT_CONVERT_TO_INT_TYPE );
        	    return ( TYPE_INVALID );
    		}
    		break;

        case TIMES_EQUALS:
        case DIV_EQUALS:
        case PLUS_EQUALS:
        case MINUS_EQUALS:
        case AND_EQUALS:
        case OR_EQUALS:
        case XOR_EQUALS:
        	if ( ( type1 == TYPE_INT ) &&
				 ( type2 == TYPE_INT ) )
        	{
        		root->datatype = TYPE_INT;
        	    return ( TYPE_INT );
        	}
        	else if ( ( type1 == TYPE_CHAR ) &&
					  ( type2 == TYPE_CHAR ) )
        	{
        		root->datatype = TYPE_CHAR;
        	    return( TYPE_CHAR );
        	}
        	else if ( ( type1 == TYPE_INT ) &&
					  ( type2 == TYPE_CHAR ) )
        	{
        		root->datatype = TYPE_INT;
        	    return( TYPE_INT );
        	}
        	else if( ( type1 == TYPE_FLOAT ) &&
					 ( type2 == TYPE_FLOAT ) )
        	{
        		root->datatype = TYPE_FLOAT;
        		return( TYPE_FLOAT );
        	}
        	else
        	{
        	    typeError ( eINCOMPATIBLE_ASSIGNMENT_TYPES );
        	    return ( TYPE_INVALID );
        	}
        	break;

        case OR:
        case AND:
        	if ( ( ( type1 == TYPE_INT ) || ( type1 == TYPE_BOOL ||
                   ( type1 == TYPE_CHAR ) ) )  &&
        	    ( ( type2 == TYPE_INT ) || ( type2 == TYPE_BOOL ) ||
                  ( type2 == TYPE_CHAR ) ) )
        	{
        	    return(TYPE_BOOL);
        	}
        	else
        	{
        	    typeError( eINCOMPATIBLE_OPERAND_TYPES_FOR_LOGICAL_OPERATOR );
        	    return( TYPE_INVALID );
        	}
        	break;

        case XOR:
        case BOR:
        case BAND:
        	if ( ( type1 == TYPE_INT ) && ( type2 == TYPE_INT ) )
        	{
        	    return ( TYPE_INT );
        	}
        	else if ( ( type1 == TYPE_INT) && (type2 == TYPE_CHAR))
        	{
        	    return ( TYPE_INT );
        	}
        	else if ( ( type1 == TYPE_CHAR ) && ( type2 == TYPE_CHAR ) )
        	{
        	    return ( TYPE_CHAR );
        	}
        	else
        	{
        	    typeError( eINCOMPATIBLE_OPERAND_TYPES_FOR_BITWISE_OPERATOR );
        	    return( TYPE_INVALID);
        	}
        	break;

        case EQUALS:
        case NOTEQUALS:
        	if ( ( ( type1 == TYPE_INT ) || ( type1 == TYPE_CHAR ) ) &&
        	    ( ( type2 == TYPE_INT ) || ( type2 == TYPE_CHAR ) ) )
        	{
        	    return( TYPE_BOOL );
        	}

        	else if ( ( ( type1 == TYPE_CHAR ) && ( type2 == TYPE_BOOL ) ) ||
        		 ( ( type1 == TYPE_BOOL ) && ( type2 == TYPE_CHAR ) ) )
        	{
        	    return( TYPE_BOOL );
        	}
        	else if( ( type1 == TYPE_FLOAT ) && ( type2 == TYPE_FLOAT ))
        	{
        		root->datatype = TYPE_FLOAT;
        		return( TYPE_FLOAT );
        	}
        	else
        	{
        	    typeError(eINCOMPATIBLE_OPERAND_TYPES_FOR_RELATIONAL_OPERATOR);
        	    return( TYPE_INVALID );
        	}
        	break;

        case LTE:
        case GTE:
        case LT:
        case GT:
        	if ( ( ( type1 == TYPE_INT ) || ( type1 == TYPE_CHAR ) ) &&
        	    ( ( type2 == TYPE_INT ) || ( type2 == TYPE_CHAR ) ) )
        	{
        	    return(TYPE_BOOL);
        	}
        	else if ( ( ( type1 == TYPE_CHAR ) && ( type2 == TYPE_BOOL ) ) ||
        		 ( ( type1 == TYPE_BOOL ) && ( type2 == TYPE_CHAR ) ) )
        	{
        	    return(TYPE_BOOL);
        	}
        	else if( ( type1 == TYPE_FLOAT ) && ( type2 == TYPE_FLOAT ))
        	{
        		root->datatype = TYPE_FLOAT;
        		return( TYPE_FLOAT );
        	}
        	else
        	{
        	    typeError(eINCOMPATIBLE_OPERAND_TYPES_FOR_RELATIONAL_OPERATOR);
        	    return( TYPE_INVALID );
        	}
        	break;

        case RSHIFT:
        case LSHIFT:
        	if ( ( type1 == TYPE_INT ) && ( type2 == TYPE_INT ) )
        	{
        	    return( TYPE_INT );
        	}

        	else if ( ( type1 == TYPE_INT ) && ( type2 == TYPE_CHAR ) )
        	{
        	    return(TYPE_INT);
        	}

        	else if ( ( type1 == TYPE_INT ) && ( type2 == TYPE_BOOL ) )
        	{
        	    return(TYPE_INT);
        	}

        	else if ( ( type1 == TYPE_CHAR ) && ( type2 == TYPE_CHAR ) )
        	{
        	    return( TYPE_CHAR );
        	}

        	else if ( ( type1 == TYPE_CHAR ) && ( type2 == TYPE_BOOL ) )
        	{
        	    return( TYPE_CHAR );
        	}

        	else
        	{
        	    typeError( eINCOMPATIBLE_OPERAND_TYPES_FOR_SHIFT_OPERATOR );
        	    return( TYPE_INVALID );
        	}
        	break;

        case INC:
        case DEC:
        	if ( ( type1 == TYPE_INT ) || ( type2 == TYPE_INT ) )
        	{
        	    return(TYPE_INT);
        	}

        	else if ( ( type1 == TYPE_CHAR ) || ( type2 == TYPE_CHAR ) )
        	{
        	    return ( TYPE_CHAR );
        	}
        	else
        	{
        	    typeError( eTYPE_CANT_BE_INCREMENTED_DECREMENTED );
        	    return( TYPE_INVALID );
        	}
        	break;

        case PLUS:
        case MINUS:
        case TIMES:
        case DIVIDE:
        	if ( ( ( type1 == TYPE_INT ) && ( type2 == TYPE_CHAR ) ) ||
        	    ( ( type1 == TYPE_INT ) && ( type2 == TYPE_INT ) ) )
        	{
        		root->datatype = TYPE_INT;
        	    return(TYPE_INT);
        	}

        	if ( ( ( type1 == TYPE_CHAR ) && ( type2 == TYPE_CHAR ) ) ||
         	    ( ( type1 == TYPE_CHAR ) && ( type2 == TYPE_BOOL ) ) )
        	{
        		root->datatype = TYPE_CHAR;
        	    return(TYPE_CHAR);
        	}
        	if( ( type1 == TYPE_FLOAT ) && ( type2 == TYPE_FLOAT ) )
        	{
        		root->datatype = TYPE_FLOAT;
        		return( TYPE_FLOAT );
        	}
        	else
        	{
        	    typeError(eINCOMPATIBLE_OPERAND_TYPES_FOR_ARITHMETIC_OPERATOR);
        	    return( TYPE_INVALID );
        	}
        	break;

        default:
        	if ( ( type1 == TYPE_INVALID ) && ( type2 != TYPE_INVALID ) )
        	{
        	    return(type2);
        	}
        	else
        	{
        	    return(type1);
        	}
        	break;
    }

    return TYPE_INVALID;
}

/*==============================================================================
        Private Function Definitions
==============================================================================*/

/*============================================================================*/
/*  typeError                                                                 */
/*!
	Throw a type error

	The typeError function outputs an error message associated with the
    specified type error, and sets the global error flag

    @param[in]
        errnum
			type error number

==============================================================================*/
static void typeError(teTYPE_ERR errnum )
{
    printf( "line %d : %s\n", getlineno(), typeErr[errnum] );
    errorFlag = true;
}

/*! @}
 * end of typecheck group */