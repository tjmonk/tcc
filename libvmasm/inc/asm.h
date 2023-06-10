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

#ifndef ASM_H
#define ASM_H

/*==============================================================================
        Includes
==============================================================================*/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <vmcore/core.h>
#include <vmcore/datatypes.h>
#include "parseinfo.h"
#include "labels.h"

/*==============================================================================
        Public definitions
==============================================================================*/

#define NIL 0

#define INCPOINTER(INCREMENT) pASM->pointer += INCREMENT

#ifndef YYSTYPE
#define YYSTYPE tzParseInfo
#endif

#ifndef YACC
extern YYSTYPE yylval;
#endif

#ifndef EOK
#define EOK  0
#endif

/*! assembler state object */
typedef struct zASMState
{
    /*! memory where the assembled program is stored */
    uint8_t *memory;

    /* the index to the memory array */
    size_t pointer;

    /* error state */
    int error;

} tzASMState;

/*==============================================================================
        Public function declarations
==============================================================================*/

int assemble_program( char *filename, uint8_t *memory, size_t *length );

#endif
