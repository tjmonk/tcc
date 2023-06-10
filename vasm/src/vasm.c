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
 * @defgroup vasm vasm
 * @brief Virtual Machine Assembler
 * @{
 */

/*============================================================================*/
/*!
@file vasm.c

    Virtual Machine Assembler

    The vasm Application is an assembler for the gateway project
    Virtual Machine

*/
/*============================================================================*/

/*==============================================================================
        Includes
==============================================================================*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <vmcore/core.h>
#include <vmasm/asm.h>
#include <vmasm/labels.h>

/*==============================================================================
        Private definitions
==============================================================================*/

/*! Default core size for the VM core */
#define DEFAULT_CORE_SIZE ( 65536 )

/*! Default stack size for the VM core */
#define DEFAULT_STACK_SIZE ( 4096 )

/*==============================================================================
        Public function declarations
==============================================================================*/
void main( int argc, char **argv );

/*==============================================================================
        Private function declarations
==============================================================================*/
void usage( void );

/*==============================================================================
        Function definitions
==============================================================================*/

/*============================================================================*/
/*  main                                                                      */
/*!
    Main entry point for the vasm application

    The main function starts the vasm application

    @param[in]
        argc
            number of arguments on the command line
            (including the command itself)

    @param[in]
        argv
            array of pointers to the command line arguments

    @return none

==============================================================================*/
void main(int argc, char **argv)
{
    char *outputFile = NULL;
    char *inputFile = NULL;
    size_t core_size = DEFAULT_CORE_SIZE;
    size_t stack_size = DEFAULT_STACK_SIZE;
    uint8_t *pMem;
    size_t prog_size;
    tzCore *pCore;
    int c;

    while( ( c = getopt( argc, argv, "c:o:s:h" ) ) != -1 )
    {
        switch( c )
        {
            case 'o':
                outputFile = optarg;
		printf("outputFile= %s\n", outputFile );
                break;

            case 'c':
                core_size = atol( optarg );
                break;

            case 's':
                stack_size = atol( optarg );
                break;

            case 'h':
                usage();
                break;

        }
    }

    /* get the name of the input file */
    inputFile = argv[optind];

    printf("inputFile = %s\n", inputFile );

    if( outputFile != NULL )
    {
        /* create the virtual machine core to write the
           Virtual Machine code into */
        pCore = CORE_fnCreate( core_size, stack_size );
        if( pCore != NULL )
        {
            prog_size = core_size;

            /* get a pointer to the start of the VM memory core */
            pMem = CORE_fnMemory( pCore );

            /* assemble the assembly language program */
            if( assemble_program( inputFile, pMem, &prog_size ) == EOK )
            {
		        printf("assembly done\n");

                /* link the labels */
                if( LinkLabels( pMem, false, false ) >= 0 )
                {
                    /* output the program */
                    CORE_fnSetProgramSize( pCore, prog_size );
                    CORE_fnSave( pCore, outputFile );
                }
                else
                {
                    fprintf( stderr, "Link labels failed\n" );
                }
            }
            else
            {
                fprintf( stderr, "Assembly failed\n" );
            }
        }
        else
        {
            fprintf( stderr, "Unable to create VM core\n" );
        }
    }
    else
    {
        fprintf( stderr, "Specify output file with -o\n" );
    }
}

/*============================================================================*/
/*  usage                                                                     */
/*!
    Program usage message

    The usage function outputs the program usage message to stdout
    and aborts the application

==============================================================================*/
void usage( void )
{
    printf("usage: vasm [-c core size] [-s stack size] [-h]"
           " [-o output filename] <assembly file>\n" );
    exit( 0 );
}

/*! @}
 * end of vasm group */
