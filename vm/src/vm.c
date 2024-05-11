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
        Includes
==============================================================================*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "vm.h"
#include <vmcore/core.h>
#include <vmasm/asm.h>
#include <vmasm/labels.h>

/*! default virtual machine core memory size */
#define DEFAULT_CORE_SIZE   65536

/*! default virtual machine stack size */
#define DEFAULT_STACK_SIZE  4096

/*==============================================================================
        Function Declarations
==============================================================================*/

int main( int argC, char *argV[]);

/*==============================================================================
        Function Definitions
==============================================================================*/

/*============================================================================*/
/*  main                                                                      */
/*!
    main entry point for the Virtual Machine

    The main function is the main entry point for the virtual machine.
    Depending on the arguments, the virtual machine can assemble or
    assemble  and run a VM assembly file, or run a pre-assembled VM
    executable file. It can also dump various diagnostics about the
    virtual machine state.

    @param[in]
        argC
            argument count

    @param[in]
        argV
            array of argument pointers

    @retval 0 success
    @retval -1 error

==============================================================================*/
int main(int argC, char *argV[])
{
    char *filename;
    char *outputFilename = NULL;
    int c;
    tzVMState vmstate;
    size_t prog_size;
    int result = 0;

    /* flag for post-mortem dump */
    bool postMortem = false;

    /* flag to show registers at each instruction */
    bool showRegs = false;

    /* flag to show all labels when assigned */
    bool showLabels = false;

    /* flag to print load, link, and execute messages */
    bool verbose = false;

    /* flag to execute the binary image */
    bool execute_flag = false;

    /* flag to assemble the input file */
    bool assemble_flag = false;

    /* perform a memory dump */
    bool dumpMemory = false;

    /* core size */
    size_t core_size = DEFAULT_CORE_SIZE;

    /* stack size */
    size_t stack_size = DEFAULT_STACK_SIZE;

    /* pointer to the VM core */
    tzCore *pCore = NULL;

    /* pointer to the externals library */
    char *externalsLib = NULL;

    /* pointer to the VM parser state */
    tzVMState *pVM = &vmstate;

    while( ( c = getopt( argC, argV, "haplrvo:eds:c:L:" ) ) != -1 )
    {
        switch( c )
        {
            case 'a':
                assemble_flag = true;
                break;

            case 'p':
                postMortem = true;
                break;

            case 'd':
                dumpMemory = true;
                break;

            case 'l':
                showLabels = true;
                break;

            case 'r':
                showRegs = true;
                break;

            case 'v':
                verbose = true;
                break;

            case 'o':
                outputFilename = optarg;
                break;

            case 'e':
                execute_flag = true;
                break;

            case 's':
                stack_size = atol( optarg );
                break;

            case 'c':
                core_size = atol( optarg );

            case 'L':
                externalsLib = optarg;
                break;

            case 'h':
                fprintf(stderr,
                        "usage: %s [-a] [-e] [-p] [-l] [-r] [-v]"
                        " [-L <external variable handler library]"
                        " [-c <core size (bytes)>]"
                        " [-s <stack size (longwords)>]"
                        " [-o <output file>] <input file>\n\n"
                        " -a : assemble input file\n"
                        " -e : execute input file\n"
                        " -p : enable postmortem core dump\n"
                        " -l : show labels\n"
                        " -r : show registers\n"
                        " -v : enable verbose operation\n"
                        " -L : specify exernvars library\n"
                        " -c : set core size\n"
                        " -s : set stack size\n"
                        " -o : write out program memory\n",
                        argV[0]);
                return -1;

            default:
                break;

        }
    }

    if( argC == 1 )
    {
        assemble_flag = true;
        execute_flag = true;
    }

    /* allocate the core */
    prog_size = core_size;
    pCore = CORE_fnCreate( core_size, stack_size );
    if( pCore == NULL )
    {
        fprintf(stderr, "%s: Unable to allocate core\n", argV[0]);
        return -1;
    }

    /* initialize the "extern" library */
    CORE_fnInitExternalsLib( pCore, externalsLib );

    /* set the VM core memory pointer and core size */
    pVM->memory = CORE_fnMemory( pCore );
    pVM->pointer = 0;

    /* get the input filename */
    if( argC == 1 )
    {
        filename = NULL;
    }
    else
    {
        filename = argV[argC-1];
    }

    if( assemble_flag == true )
    {
        if (verbose)
        {
           printf("now loading...%s\n", filename);
        }

        if( assemble_program( filename,
                              pVM->memory,
                              &prog_size ) == EOK )
        {
            /* set the program size in the core */
            CORE_fnSetProgramSize( pCore, prog_size );

            if (LinkLabels(pVM->memory, verbose, showLabels) < 0)
            {
                fprintf(stderr, "Error linking: %s\n", filename );
            }
        }
        else
        {
            fprintf(stderr, "Error assembling: %s\n", filename );
            return -1;
        }
    }
    else
    {
        if( verbose )
        {
            fprintf(stdout, "Loading program: %s\n", filename );
        }

        /* assume input file is binary image */
        if( CORE_fnLoad( pCore, filename ) != true )
        {
            fprintf(stderr,
                    "Unable to load binary image file: %s\n",
                    filename );

            return -1;
        }
    }

    if( outputFilename != NULL )
    {
        if( verbose )
        {
            fprintf(stdout, "Writing binary image to %s\n", outputFilename);
        }

        /* write the binary image to the output file */
        if( CORE_fnSave(pCore, outputFilename) != true )
        {
            fprintf(stderr,
                    "unable to output binary image to %s\n",
                    outputFilename);
        }
    }

    if( dumpMemory == true )
    {
        CORE_fnDumpMemory( pCore, 0, 0, stdout );
    }

    if( execute_flag )
    {
        if( verbose )
        {
            fprintf(stdout, "Executing binary image\n");
        }

        result = CORE_fnExecute( pCore );
        if ( result != -1 )
        {
            if (postMortem)
            {
                /* dump the core */
                CORE_fnDump(pCore);
            }

            if( showRegs == true )
            {
                CORE_fnDumpRegisters( pCore, stdout );
            }

            if ( verbose )
            {
                printf("program was successful.\n");
            }
        }
        else
        {
            /* always dump the core if the program fails */
            CORE_fnDump(pCore);
            CORE_fnDumpRegisters(pCore, stderr);
            if ( verbose )
            {
                printf("program terminated.\n");
            }
        }
    }

    /* shutdown the "extern" library */
    CORE_fnShutdownExternalsLib( pCore );

    return result;
}



