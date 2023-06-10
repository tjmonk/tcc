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

#ifndef TYPECHECK_H
#define TYPECHECK_H

/*==============================================================================
        Includes
==============================================================================*/

#include <stdbool.h>
#include "node.h"

/*==============================================================================
        Public Definitions
==============================================================================*/

/*! The teTYPE_ERR is an enumeration for the different parser type errors */
typedef enum eTYPE_ERR
{
	eUNKNOWN = 0,
	eVALUE_OUT_OF_RANGE=1,
	eINCOMPATIBLE_ASSIGNMENT_TYPES=2,
	eINCOMPATIBLE_OPERAND_TYPES_FOR_LOGICAL_OPERATOR=3,
	eINCOMPATIBLE_OPERAND_TYPES_FOR_BITWISE_OPERATOR=4,
	eINCOMPATIBLE_OPERAND_TYPES_FOR_RELATIONAL_OPERATOR=5,
	eINCOMPATIBLE_OPERAND_TYPES_FOR_SHIFT_OPERATOR=6,
	eTYPE_CANT_BE_INCREMENTED_DECREMENTED=7,
	eINCOMPATIBLE_OPERAND_TYPES_FOR_ARITHMETIC_OPERATOR=8,
	eINVALID_METHOD_FOR_ARGUMENT_TYPE=9,
	eCANNOT_CONVERT_TO_FLOAT_TYPE=10,
	eCANNOT_CONVERT_TO_INT_TYPE=11,
	eCANNOT_ASSIGN_ONE_EXTERN_TO_ANOTHER=12,
	eMAX_TYPE_ERR=13
} teTYPE_ERR;

/*==============================================================================
        Public Function Declarations
==============================================================================*/

int TypeCheck(struct Node *root, int level, bool debug);
bool TypeErrorDetected( void );

#endif
