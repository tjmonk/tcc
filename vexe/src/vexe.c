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
 * @defgroup vexe vexe
 * @brief Virtual Machine Executor
 * @{
 */

/*============================================================================*/
/*!
@file vexe.c

    Virtual Machine Executor

    The vexe Application is an executor for the gateway project
    Virtual Machine.  It executes programs assembled with the vasm
    assembler.

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
int main( int argc, char **argv );

/*==============================================================================
        Private function declarations
==============================================================================*/
void usage( void );

/*==============================================================================
        Function definitions
==============================================================================*/

/*============================================================================*/
/*  main                                                                      */
/*!
    Main entry point for the vexe application

    The main function starts the vexe application

    @param[in]
        argc
            number of arguments on the command line
            (including the command itself)

    @param[in]
        argv
            array of pointers to the command line arguments

    @retval -1 Core Execution Failed
    @retval other value of VM core register R0

==============================================================================*/
int main(int argc, char **argv)
{
    char *inputFile = NULL;
    size_t core_size = DEFAULT_CORE_SIZE;
    size_t stack_size = DEFAULT_STACK_SIZE;
    char *externalsLib = NULL;
    bool verbose = false;
    tzCore *pCore;
    int result = -1;
    int c;

    while( ( c = getopt( argc, argv, "L:c:s:hv" ) ) != -1 )
    {
        switch( c )
        {
            case 'c':
                core_size = atol( optarg );
                break;

            case 's':
                stack_size = atol( optarg );
                break;

            case 'v':
                verbose = true;
                break;

            case 'L':
                externalsLib = optarg;
                break;

            case 'h':
                usage();
                break;

        }
    }

    /* get the name of the input file */
    inputFile = argv[optind];

    if( inputFile != NULL )
    {
        /* create the virtual machine core to write the
           Virtual Machine code into */
        pCore = CORE_fnCreate( core_size, stack_size );
        if( pCore != NULL )
        {
            /* initialize the externals library */
            CORE_fnInitExternalsLib( pCore, externalsLib );

            if( verbose == true )
            {
                fprintf( stdout, "Loading program: %s\n", inputFile );
            }

            /* load the program */
            if( CORE_fnLoad( pCore, inputFile ) == true )
            {
                /* execute the program */
                result = CORE_fnExecute( pCore );
                if( result != -1 )
                {
                    if( verbose )
                    {
                        fprintf( stdout,
                                 "Executing program %s\n",
                                 inputFile );
                    }
                }
                else
                {
                    fprintf( stderr, "Execution failed: %s\n", inputFile );
                }
            }
            else
            {
                fprintf( stderr, "Program load failed: %s\n", inputFile );
            }

            /* shut down the externals library */
            CORE_fnShutdownExternalsLib( pCore );
        }
        else
        {
            fprintf( stderr, "Unable to create VM core\n" );
        }
    }
    else
    {
        fprintf( stderr, "No execution binary specified\n" );
    }

    return result;
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
    printf( "usage: vexe [-c core size] [-s stack size] [-h] [-v] "
            " [-L externals lib name] <binary image>\n" );
    exit( 0 );
}

/*! @}
 * end of vexe group */
