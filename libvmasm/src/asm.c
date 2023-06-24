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
 * @defgroup asm asm
 * @brief VM Assembler
 * @{
 */

/*============================================================================*/
/*!
@file asm.c

    Virtual Machine Assembler

    The Virtual Machine Assembler module provides APIs for assembling
    a Virtual Machine Assembly Language file into a Virtual Machine
    core image (binary machine code)

*/
/*============================================================================*/


/*==============================================================================
        Includes
==============================================================================*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <vmasm/asm.h>


/*==============================================================================
        External variables
==============================================================================*/
extern FILE *yyin;		/* input file for lex */

/*==============================================================================
        External functions
==============================================================================*/

extern int yyparse(tzASMState *pASM);

/*==============================================================================
        Function definitions
==============================================================================*/

/*============================================================================*/
/*  assemble_program                                                          */
/*!
    Assemble a Virtual Machine Assembly Language File

    The assemble_program function parses the specified input
    Virtual Machine Assembly Language file into a binary object
    containing Virtual Machine Assembly code.

    @param[in]
        filename
            pointer to the name of the file to assemble, if NULL,
            the program will be assembled from standard input

    @param[out]
        memory
            the base memory location of the VM Core binary

    @param[in,out]
        length
            pointer to the max length of the VM Core binary
            The assembled binary object size is returned via this argument

    @retval EOK - the VarObject was created ok
    @retval ENOMEM - memory allocation failed
    @retval EINVAL - invalid arguments

==============================================================================*/
int assemble_program( char *filename, uint8_t *memory, size_t *length )
{
    int result = EINVAL;
    tzASMState asmState;
    tzASMState *pASM = &asmState;

    if( ( memory != NULL ) &&
        ( length != NULL ) &&
        ( *length > 0 ) )
    {
        result = EOK;

        /* default input is stdin */
        yyin = stdin;

        /* intialize base memory pointer */
        pASM->memory = memory;

        /* initialize memory pointer */
        pASM->pointer = 0;

        /* initialize semantic error flag */
        pASM->error = 0;

        /* open input file */
        if ( filename != NULL )
        {
            if( *filename == '-' )
            {
                yyin = stdin;
            }
            else
            {
                /* input file was specified */
                if ((yyin = fopen(filename, "r")) == (FILE *)NULL)
                {
                    fprintf(stderr, "file %s not found.\n", filename);
                    result = ENOENT;
                }
            }
        }

        if( result == EOK )
        {
            /* run the parser */
            if ( (yyparse(pASM) == 0 ) &&
                 (pASM->error == 0 ) )
            {
                *length = pASM->pointer;
                result = EOK;
            }
            else
            {
                *length = 0;
                fprintf( stderr,
                         "errors encountered in program\n");
                result = EINVAL;
            }
        }
    }

    return result;
}

/*! @}
 * end of asm group */
