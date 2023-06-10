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
 * @defgroup lextest Lexical Analyzer Test
 * @brief VM Assembler Lexical Analyzer Test
 * @{
 */

/*==============================================================================
        Includes
==============================================================================*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>

/*==============================================================================
        External Variables
==============================================================================*/

extern long yylval;
extern char *yytext;

/*==============================================================================
        External Functions
==============================================================================*/

extern int yylex();

/*==============================================================================
        Public Function Definitions
==============================================================================*/

/*============================================================================*/
/*  main                                                                      */
/*!
    Main entry point

    The main function is the main entry point for the lexical analyzer
    test for the Virtual Machine Assembler.

    @param[in]
        argC
            number of arguments

    @param[in]
        argV
            array of arguments

    @retval 0 no errors
    @retval -1 an error occurred

==============================================================================*/
int main(int argC, char **argV )
{
    extern FILE *yyin;
    int result = 1;

    if( argC != 2 )
    {
    	fprintf( stderr, "usage: %s <filename>\n", argV[0] );
    	return -1;
    }

    fprintf( stderr, "sizeof(int) = %lu\n", sizeof(int) );

    /* input file was specified */
    if ( ( yyin = fopen( argV[1], "r" ) ) == (FILE *)NULL )
    {
        fprintf( stderr, "file not found.\n" );
    }

    while( result )
    {
    	result = yylex();
    	printf("%s ", yytext);
    }

    if( yyin != stdin )
    {
    	fclose(yyin);
    }

    return 0;
}

/*! @}
 * end of lextest group */