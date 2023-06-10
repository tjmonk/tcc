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
 * @defgroup lineno lineno
 * @brief Track line number during parsing
 * @{
 */

/*==============================================================================
        Includes
==============================================================================*/

#include "lineno.h"

/*==============================================================================
        File Scoped Variables
==============================================================================*/

/*! track the line number during parsing */
static int lineno = 0;

/*==============================================================================
        Public Function Definitions
==============================================================================*/

/*============================================================================*/
/*  getlineno                                                                 */
/*!
    Get Line Number

    The getlineno function returns the current line number being parsed

    @return the current line number being parsed

==============================================================================*/
int getlineno( void )
{
    return lineno;
}

/*============================================================================*/
/*  incrementLineNumber                                                       */
/*!
    Increment the current line number

    The incrementLineNumber method increments the current line number
    being parsed.

==============================================================================*/
void incrementLineNumber( void )
{
    lineno++;
}

/*! @}
 * end of lineno group */