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
 * @defgroup core core
 * @brief Virtual Machine Processing Core
 * @{
 */

/*==============================================================================
        Includes
==============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <dlfcn.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "core.h"
#include "datatypes.h"
#include "ask.h"
#include "strbuf.h"
#include "externvars.h"
#include "files.h"

/*==============================================================================
        Private definitions
==============================================================================*/

/*! Program Counter  */
#define PC *(pCore->pc)

/*! Stack Pointer */
#define SP *(pCore->sp)

/*! Program Memory */
#define MEMORY pCore->memory

/*! General Purpose Registers */
#define REG pCore->registers.reg

/*! Floating Point Registers */
#define REGF pCore->registers.freg

/*! Status Register */
#define STATUS pCore->status

/*! Stack Size */
#define STACK_SIZE (pCore->stack_size * sizeof( uint32_t ))

/*! Core Program Memory Size */
#define CORE_SIZE pCore->core_size

/*! Program Size */
#define PROGRAM_SIZE pCore->programSize

/*! Stop flag */
#define STOP pCore->running = false

/*! Macro to increment the Program Counter */
#define INC_PC(INC)  PC = PC + ( INC ); if( PC > PROGRAM_SIZE ) \
        { printf("Illegal PC address\n"); STOP;}

/* flags bits */

/*! the sign bit of a long int */
#define SIGNBIT 0x80000000

/*! zero flag bit */
#define ZFLAG   0x00000001

/*! negative flag bit */
#define NFLAG   0x00000002

/*! carry flag bit */
#define CFLAG   0x00000004

/*! zero flag mask */
#define ZMASK   ~ZFLAG

/*! negative flag mask */
#define NMASK   ~NFLAG

/*! carry flag mask */
#define CMASK   ~CFLAG

/*! Set the Zero Flag based on the register value */
#define ZSET(REGVAL) STATUS = (REGVAL == 0) \
                    ? (STATUS | ZFLAG) \
                        : (STATUS & ZMASK)

/*! Set the Negative Flag based on the register value */
#define NSET(REGVAL) STATUS = ((int32_t)REGVAL <  0) \
                    ? (STATUS | NFLAG) \
                    : (STATUS & NMASK)

/*! Set the Carry Flag based on the register value */
#define CSET(REGVAL) STATUS = ((oldvalue & SIGNBIT) ^ (REGVAL & SIGNBIT)) \
                    ? (STATUS | CFLAG) \
                    : (STATUS & CMASK)

/*! Set all the flags based on the register value */
#define SETFLAGS(REGVAL) ZSET(REGVAL); NSET(REGVAL); CSET(REGVAL);

/*! Set Zero and Negative flags based on register value */
#define SETFFLAGS(REGVAL) ZSET(REGVAL); NSET(REGVAL);

/*! define the maximum number of timers allowed in the user program */
#define MAX_TIMERS  ( 20 )

/*! DUMPLINE helper macro for coreDump function */
#define DUMPLINE(N) { \
            fprintf(fp, "  %4x", i); \
            for (j=0; (j<N); j++) \
                fprintf(fp, "  %x", pCore->memory[i++]); \
            fprintf(fp, "\n"); \
            }

/*! the tzRegBytes object maps a 32-bit register to its bytes */
typedef struct zRegBytes
{
    /*! array of register bytes */
    uint8_t b[4];
} tzRegBytes;

/*! the tuRegisters union defines all registers so we can convert
    between floating point, integer, and byte representations */
typedef union uRegisters
{
    /*! 16 32-bit IEEE754 floating point registers */
    float freg[16];

    /*! 16 32-bit integer registers */
    int32_t reg[16];

    /*! 16 32-bit register byte arrays */
    tzRegBytes bytes[16];
} tuRegisters;

/*! the tzCore structure represents the state of the virtual machine core */
struct zCore
{
    /*! core size */
    size_t core_size;

    /*! memory where the program is stored */
    uint8_t *memory;

    /*! size of the stack */
    size_t stack_size;

    /*! length of the currently loaded program */
    size_t programSize;

    /*! the index to the memory array */
    size_t pointer;

    /*! 32-bit ALU registers */
    tuRegisters registers;

    /*! ALU status */
    uint32_t status;

    /*! program counter */
    int32_t *pc;

    /*! stack pointer */
    int32_t *sp;

    /*! call depth */
    uint32_t call_depth;

    /*! error state */
    bool error;

    /*! running state */
    bool running;

    /*! opaque pointer to external variable library */
    void *pExternLib;

    /*! opaque pointer to external variable state */
    void *pExt;
};

/*! The tzZInstruction object maps an OPCODE and description to a
    code fragment to implement the operation */
typedef struct zInstruction
{
    /*! instruction opcode */
    uint8_t opcode;

    /*! instruction name */
    char *name;

    /*! function to implement the instruction */
    void (*exec)(tzCore *);
} tzInstruction;

/*==============================================================================
        Private Function Declarations
==============================================================================*/

static bool core_fnCheckInstructionList();
static uint32_t core_fnGetUnsignedData( tzCore *pCore,
                                        uint8_t *memory,
                                        int32_t pc,
                                        uint8_t offset);

static int32_t core_fnGetSignedData( tzCore *pCore,
                                     uint8_t *memory,
                                     int32_t pc,
                                     uint8_t offset);

static float core_fnGetFloatData( tzCore *pCore,
                                  uint8_t *memory,
                                  int32_t pc,
                                  uint8_t offset);

static void core_fnSetStackData( tzCore *pCore,
                                 uint32_t sp,
                                 uint32_t val );

static uint32_t core_fnGetStackData( tzCore *pCore, uint32_t sp );

static void core_fnStoreData( tzCore *pCore,
                              uint8_t *instr,
                              uint8_t *dest,
                              uint8_t *src );
static int setupTimer( int id, int intervalMS );
static int waitSignal( int *signum, int *id );

/* opcode functions */
static void opNOP(tzCore *pCore);
static void opLOD(tzCore *pCore);
static void opSTR(tzCore *pCore);
static void opMOV(tzCore *pCore);
static void opADD(tzCore *pCore);
static void opSUB(tzCore *pCore);
static void opMUL(tzCore *pCore);
static void opDIV(tzCore *pCore);
static void opAND(tzCore *pCore);
static void opOR(tzCore *pCore);
static void opNOT(tzCore *pCore);
static void opSHR(tzCore *pCore);
static void opSHL(tzCore *pCore);
static void opJMP(tzCore *pCore);
static void opJZR(tzCore *pCore);
static void opJNZ(tzCore *pCore);
static void opJNE(tzCore *pCore);
static void opJPO(tzCore *pCore);
static void opJCA(tzCore *pCore);
static void opJNC(tzCore *pCore);
static void opCAL(tzCore *pCore);
static void opRET(tzCore *pCore);
static void opCMP(tzCore *pCore);
static void opTOF(tzCore *pCore);
static void opTOI(tzCore *pCore);
static void opRDN(tzCore *pCore);
static void opRDC(tzCore *pCore);
static void opWRN(tzCore *pCore);
static void opWRC(tzCore *pCore);
static void opWRF(tzCore *pCore);
static void opPSH(tzCore *pCore);
static void opPOP(tzCore *pCore);
static void opHLT(tzCore *pCore);
static void opRDUMP(tzCore *pCore);
static void opILLEGAL(tzCore *pCore);
static void opEXT( tzCore *pCore );
static void opGET( tzCore *pCore );
static void opSET( tzCore *pCore );

static void opINST1(tzCore *pCore);
static void opINST2(tzCore *pCore);
static void opMDUMP( tzCore *pCore );
static void opWRS( tzCore *pCore );
static void opCSB( tzCore *pCore );
static void opZSB( tzCore *pCore );
static void opWSB( tzCore *pCore );
static void opASN( tzCore *pCore );
static void opASS( tzCore *pCore );
static void opASB( tzCore *pCore );
static void opASC( tzCore *pCore );
static void opASF( tzCore *pCore );
static void opDLY( tzCore *pCore );
static void opSTM( tzCore *pCore );
static void opCTM( tzCore *pCore );
static void opNFY( tzCore *pCore );
static void opWFS( tzCore *pCore );
static void opEVS( tzCore *pCore );
static void opEVE( tzCore *pCore );
static void opSBL( tzCore *pCore );
static void opSBO( tzCore *pCore );
static void opSCO( tzCore *pCore );
static void opGCO( tzCore *pCore );
static void opOFD( tzCore *pCore );
static void opCFD( tzCore *pCore );
static void opSFD( tzCore *pCore );
static void opOPS( tzCore *pCore );
static void opCPS( tzCore *pCore );
static void opEXE( tzCore *pCore );

/*==============================================================================
        File Scoped variables
==============================================================================*/

/*! storage for the program timers */
static timer_t timers[MAX_TIMERS] = {0};

/* virtual machine instructions */
/* THESE MUST BE IN THE SAME ORDER AS THEY ARE DEFINED */
tzInstruction instructions0[HRMAXINST+1] =
{
    { HNOP, "NOP", opNOP }, // 0x00
    { HLOD, "LOD", opLOD }, // 0x01
    { HSTR, "STR", opSTR }, // 0x02
    { HMOV, "MOV", opMOV }, // 0x03
    { HADD, "ADD", opADD }, // 0x04
    { HSUB, "SUB", opSUB }, // 0x05
    { HMUL, "MUL", opMUL }, // 0x06
    { HDIV, "DIV", opDIV }, // 0x07
    { HAND, "AND", opAND }, // 0x08
    { HOR,  "OR",  opOR  }, // 0x09
    { HNOT, "NOT", opNOT }, // 0x0A
    { HSHR, "SHR", opSHR }, // 0x0B
    { HSHL, "SHL", opSHL }, // 0x0C
    { HJMP, "JMP", opJMP }, // 0x0D
    { HJZR, "JZR", opJZR }, // 0x0E
    { HJNZ, "JNZ", opJNZ }, // 0x0F
    { HJNE, "JNE", opJNE }, // 0x10
    { HJPO, "JPO", opJPO }, // 0x11
    { HJCA, "JCA", opJCA }, // 0x12
    { HJNC, "JNC", opJNC }, // 0x13
    { HCAL, "CAL", opCAL }, // 0x14
    { HRET, "RET", opRET }, // 0x15
    { HCMP, "CMP", opCMP }, // 0x16
    { HTOF, "TOF", opTOF }, // 0x17
    { HTOI, "TOI", opTOI }, // 0x18
    { HPSH, "PSH", opPSH }, // 0x19
    { HPOP, "POP", opPOP }, // 0x1A
    { HHLT, "HLT", opHLT }, // 0x1B
    { HEXT, "EXT", opEXT }, // 0x1C
    { HGET, "GET", opGET }, // 0x1D
    { HSET, "SET", opSET }, // 0x1E
    { HNEXT, "NEXT", opINST1 }  // 0x1F
};

/* virtual machine instructions */
/* THESE MUST BE IN THE SAME ORDER AS THEY ARE DEFINED */
tzInstruction instructions1[HRMAXINST+1] =
{
        { HOPS,   "OPS",   opOPS   }, // 0x00
        { HCPS,   "CPS",   opCPS   }, // 0x01
        { HWRS,   "WRS",   opWRS   }, // 0x02
        { HCSB,   "CSB",   opCSB   }, // 0x03
        { HZSB,   "ZSB",   opZSB   }, // 0x04
        { HWSB,   "WSB",   opWSB   }, // 0x05
        { HASS,   "ASS",   opASS   }, // 0x06
        { HASB,   "ASB",   opASB   }, // 0x07
        { HASN,   "ASN",   opASN   }, // 0x08
        { HASC,   "ASC",   opASC   }, // 0x09
        { HASF,   "ASF",   opASF   }, // 0x0A
        { HRDC,   "RDC",   opRDC   }, // 0x0B
        { HRDN,   "RDN",   opRDN   }, // 0x0C
        { HWRF,   "WRF",   opWRF   }, // 0x0D
        { HWRN,   "WRN",   opWRN   }, // 0x0E
        { HWRC,   "WRC",   opWRC   }, // 0x0F
        { HDLY,   "DLY",   opDLY   }, // 0x10
        { HSTM,   "STM",   opSTM   }, // 0x11
        { HCTM,   "CTM",   opCTM   }, // 0x12
        { HNFY,   "NFY",   opNFY   }, // 0x13
        { HWFS,   "WFS",   opWFS   }, // 0x14
        { HEVS,   "EVS",   opEVS   }, // 0x15
        { HEVE,   "EVE",   opEVE   }, // 0x16
        { HSBL,   "SBL",   opSBL   }, // 0x17
        { HSBO,   "SBO",   opSBO   }, // 0x18
        { HSCO,   "SCO",   opSCO   }, // 0x19
        { HGCO,   "GCO",   opGCO   }, // 0x1A
        { HOFD,   "OFD",   opOFD   }, // 0x1B
        { HCFD,   "CFD",   opCFD   }, // 0x1C
        { HSFD,   "SFD",   opSFD   }, // 0x1D
        { HEXE,   "EXE",   opEXE   }, // 0x1E
        { HNEXT,  "NEXT",  opINST2 }  // 0x1F
};

tzInstruction instructions2[HRMAXINST+1] =
{
        { HMDUMP, "MDUMP", opMDUMP     }, // 0x00
        { HRDUMP, "RDUMP", opRDUMP     }, // 0x01
        { 0x02,   "I02",   opILLEGAL   }, // 0x02
        { 0x03,   "I03",   opILLEGAL   }, // 0x03
        { 0x04,   "I04",   opILLEGAL   }, // 0x04
        { 0x05,   "I05",   opILLEGAL   }, // 0x05
        { 0x06,   "I06",   opILLEGAL   }, // 0x06
        { 0x07,   "I07",   opILLEGAL   }, // 0x07
        { 0x08,   "I08",   opILLEGAL   }, // 0x08
        { 0x09,   "I09",   opILLEGAL   }, // 0x09
        { 0x0A,   "I0A",   opILLEGAL   }, // 0x0A
        { 0x0B,   "I0B",   opILLEGAL   }, // 0x0B
        { 0x0C,   "I0C",   opILLEGAL   }, // 0x0C
        { 0x0D,   "I0D",   opILLEGAL   }, // 0x0D
        { 0x0E,   "I0E",   opILLEGAL   }, // 0x0E
        { 0x0F,   "I0F",   opILLEGAL   }, // 0x0F
        { 0x10,   "I10",   opILLEGAL   }, // 0x10
        { 0x11,   "I11",   opILLEGAL   }, // 0x11
        { 0x12,   "I12",   opILLEGAL   }, // 0x12
        { 0x13,   "I13",   opILLEGAL   }, // 0x13
        { 0x14,   "I14",   opILLEGAL   }, // 0x14
        { 0x15,   "I15",   opILLEGAL   }, // 0x15
        { 0x16,   "I16",   opILLEGAL   }, // 0x16
        { 0x17,   "I17",   opILLEGAL   }, // 0x17
        { 0x18,   "I18",   opILLEGAL   }, // 0x18
        { 0x19,   "I19",   opILLEGAL   }, // 0x19
        { 0x1A,   "I1A",   opILLEGAL   }, // 0x1A
        { 0x1B,   "I1B",   opILLEGAL   }, // 0x1B
        { 0x1C,   "I1C",   opILLEGAL   }, // 0x1C
        { 0x1D,   "I1D",   opILLEGAL   }, // 0x1D
        { 0x1E,   "I1E",   opILLEGAL   }, // 0x1E
        { 0x1F,   "I1F",   opILLEGAL   }  // 0x1F
};


/*==============================================================================
        Public Function Definitions
==============================================================================*/

/*============================================================================*/
/*  CORE_fnCreate                                                             */
/*!
    Create the Virtual Machine Core

    The CORE_fnCreate function allocates memory for the Virtual Machine core
    and initializes its state.  The core memory size and stack size are
    configurable.

    @param[in]
        core_size
            core memory size in bytes

    @param[in]
        stack_size
            core stach size in bytes

    @retval pointer to the Virtual Machine tzCore object representing the core
    @retval NULL if the VM core could not be created

==============================================================================*/
tzCore *CORE_fnCreate( size_t core_size, size_t stack_size )
{
    /* allocate memory for the core */
    tzCore *pCore;

    /* Initialize the File Handles */
    InitFiles();

    /* validate the instruction list */
    if( core_fnCheckInstructionList() == false )
    {
        return NULL;
    }

    pCore = calloc( 1, sizeof( tzCore ) );
    if( pCore == NULL )
    {
        return NULL;
    }

    /* allocate space for the program and stack memory */
    pCore->memory = calloc( sizeof(uint8_t), core_size );
    if( pCore->memory == NULL )
    {
        free( pCore );
        return NULL;
    }

    /* initialize the core attributes */
    pCore->stack_size = stack_size;
    pCore->core_size = core_size;

    pCore->pc = &(pCore->registers.reg[15]);
    pCore->sp = &(pCore->registers.reg[14]);

    /* initialize the stack pointer */
    SP = (uint32_t)core_size;
    PC = 0;

    return pCore;
}

/*============================================================================*/
/*  CORE_fnInitExternalsLib                                                   */
/*!
    Initialize the external variables library

    The CORE_fnInitExternalsLib function initializes the external variables
    dynamic library.  If no external variable library is specified,
    then the internal external variable functionality will be used instead.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

    @param[in]
        libname
            pointer to the name of the external variable library
            (eg libvarvm.so)

    @retval EOK external variable library initialized ok
    @retval EINVAL invalid arguments

==============================================================================*/
int CORE_fnInitExternalsLib( tzCore *pCore, char *libname )
{
    int result = EINVAL;
    void *handle;
    void *(*init)(void);
    tzEXTVARAPI *(*getapi)(void);

    if( pCore != NULL )
    {
        if( libname == NULL)
        {
            /* use the internal implementation of "extern" variables */
            pCore->pExt = EXTERNVAR_Init();
            result = EOK;
        }
        else
        {
            /* open library for handling "extern" data types */
            pCore->pExternLib = dlopen( libname, RTLD_NOW );
            if( pCore->pExternLib != NULL )
            {
                init = dlsym( pCore->pExternLib, "init" );
                if( init != NULL )
                {
                    /* initialize the library */
                    pCore->pExt = init();

                    /* get the API list */
                    getapi = dlsym( pCore->pExternLib, "getapi" );
                    if( getapi != NULL )
                    {
                        /* get the API list and initialize the
                        external variable interface */
                        EXTERNVAR_fnSetAPI( getapi() );

                        result = EOK;
                    }
                    else
                    {
                        fprintf( stderr,
                                 "Cannot get API list for %s\n",
                                 libname );
                    }
                }
                else
                {
                    fprintf( stderr, "Cannot initialize %s\n", libname );
                }
            }
            else
            {
                fprintf( stderr, "Error: %s\n", dlerror());
            }
        }
    }

    return result;
}

/*============================================================================*/
/*  CORE_fnShutdownExternalsLib                                               */
/*!
    Shut down the external variables library

    The CORE_fnShutdownExternalsLib function shuts down the external library for
        handling "extern" data types

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

    @retval EOK library shut down ok
    @retval ENOENT no external library
    @retval ENOTSUP external library does not support shutdown
    @retval EINVAL invalid arguments

==============================================================================*/
int CORE_fnShutdownExternalsLib( tzCore *pCore )
{
    int result = EINVAL;
    int (*shutdown)(void *);

    if( pCore != NULL )
    {
        result = ENOENT;

        if( ( pCore->pExt != NULL ) &&
            ( pCore->pExternLib != NULL ) )
        {
            shutdown = dlsym( pCore->pExternLib, "shutdown" );
            if( shutdown != NULL )
            {
                result = shutdown( pCore->pExt );
            }
            else
            {
                result = ENOTSUP;
            }
        }
        else
        {
            /* no externals library to shut down */
            result = ENOENT;
        }

    }

    return result;
}

/*============================================================================*/
/*  CORE_fnMemory                                                             */
/*!
    Get the VM Core memory

    The CORE_fnMemory function gets a pointer to the start of the VM
    core memory.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

    @retval pointer to the core memory
    @retval NULL if no core memory exists

==============================================================================*/
uint8_t *CORE_fnMemory( tzCore *pCore )
{
    /* return a pointer to the memory core */
    if( pCore == NULL )
    {
        return NULL;
    }

    return pCore->memory;
}

/*============================================================================*/
/*  CORE_fnSize                                                               */
/*!
    Get the VM Core memory size

    The CORE_fnSize function gets the size of the VM core memory.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

    @retval size of the VM core memory
    @retval 0 if no core memory exists

==============================================================================*/
size_t CORE_fnSize( tzCore *pCore )
{
    if( pCore == NULL )
    {
        return 0L;
    }

    return pCore->core_size;
}

/*============================================================================*/
/*  CORE_fnStackSize                                                          */
/*!
    Get the VM Core stack size

    The CORE_fnStackSize function gets the size of the VM core stack.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

    @retval size of the VM core stack
    @retval 0 if no core stack exists

==============================================================================*/
size_t CORE_fnStackSize( tzCore *pCore )
{
    if( pCore == NULL )
    {
        return 0L;
    }

    return pCore->stack_size;
}

/*============================================================================*/
/*  CORE_fnSetProgramSize                                                     */
/*!
    Set the VM Core Program size

    The CORE_fnSetProgramSize function sets the size of the VM core program.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

    @param[in]
        programSize
            size of the VM program

==============================================================================*/
void CORE_fnSetProgramSize( tzCore *pCore, size_t programSize )
{
    pCore->programSize = programSize;
}

/*============================================================================*/
/*  CORE_fnGetProgramSize                                                     */
/*!
    Get the VM Core Program size

    The CORE_fnGetProgramSize function gets the size of the VM core program.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

    @retval size of the VM core program
    @retval 0 if the VM core has no program

==============================================================================*/
size_t CORE_fnGetProgramSize( tzCore *pCore )
{
    if( pCore == NULL )
    {
        return 0L;
    }

    return pCore->programSize;
}

/*============================================================================*/
/*  CORE_fnDump                                                               */
/*!
    Dump the VM Core

    The CORE_fnDump function dumps the VM core memory, registers and stack
    into a file called vm.core

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
void CORE_fnDump(tzCore *pCore)
{
	FILE *fp;

    if( pCore == NULL )
    {
        return;
    }

	fp = fopen("vm.core", "w");
    if( fp == NULL )
    {
        return;
    }

    fprintf(fp, "\ncore:\n");
    CORE_fnDumpMemory( pCore, 0, 0, fp );
	CORE_fnDumpRegisters(pCore, fp);
    CORE_fnDumpStack(pCore, fp);
	fclose(fp);
}

/*============================================================================*/
/*  CORE_fnDumpRegisters                                                      */
/*!
    Dump the VM Core Registers

    The CORE_fnDumpRegisters function dumps the VM core registers and flags
    to the specified FILE *

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

    @param[in]
        fp
            output FILE * for dumping the register values

==============================================================================*/
void CORE_fnDumpRegisters(tzCore *pCore, FILE *fp)
{
	int i;

	if( pCore == NULL )
	{
		return;
	}

	fprintf(fp, "\nregisters:\n");

	for (i = 0; (i < 16); i++)
	{
		if (i%4 == 0)
		{
			fprintf(fp, "\n");
		}
		fprintf(fp, "R%02d: 0x%08x    ",i, pCore->registers.reg[i]);
	}

	fprintf( fp,
             "\n\nSTATUS = %x\n PC = 0x%04X  SP = 0x%04X\n\n",
             pCore->status, *(pCore->pc),
             *(pCore->sp));

	fprintf( fp, "zero flag is ");
	if (pCore->status & ZFLAG)
	{
		fprintf(fp, "set.\n");
	}
	else
	{
		fprintf(fp, "cleared.\n");
	}

	fprintf(fp, "negative flag is ");
	if (pCore->status & NFLAG)
	{
		fprintf(fp, "set.\n");
	}
	else
	{
		fprintf(fp, "cleared.\n");
	}
	fprintf(fp, "carry flag is ");
	if (pCore->status & CFLAG)
	{
		fprintf(fp, "set.\n");
	}
	else
	{
		fprintf(fp, "cleared.\n\n");
	}
}

/*============================================================================*/
/*  CORE_fnDumpStack                                                          */
/*!
    Dump the VM Core Stack

    The CORE_fnDumpStack function dumps the VM core stack to the specified
    FILE *.  Only the used part of the stack will be output.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

    @param[in]
        fp
            output FILE * for dumping the stack

==============================================================================*/
void CORE_fnDumpStack(tzCore *pCore, FILE *fp)
{
    int idx;

    if ( pCore != NULL )
    {
        if ( SP < CORE_SIZE )
        {
            fprintf(fp, "stack:\n\n");
            fprintf(fp, "SP = 0x%04X\n", SP);

            /* calculate the index to get a full row of 16 bytes */
            idx = SP - (SP % 16);

            CORE_fnDumpMemory( pCore,
                            idx,
                            STACK_SIZE,
                            fp );
        }
        else
        {
            fprintf( fp, "stack: empty\n");
        }

        fprintf( fp, "\n" );
    }
}

/*============================================================================*/
/*  CORE_fnSave                                                               */
/*!
    Save the VM Core

    The CORE_fnSave function writes the core program memory out to the
    specified file.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

    @param[in]
        outputFile
            pointer to the output file name

    @retval true program saved successfully
    @retval false error saving program

==============================================================================*/
bool CORE_fnSave( tzCore *pCore, char *outputFile )
{
    FILE *fp;

    if( pCore == NULL )
    {
        return false;
    }

    fp = fopen(outputFile, "w");
    if( fp == NULL )
    {
        return false;
    }

    /* output the binary image */
    fwrite(pCore->memory, 1, pCore->programSize, fp );

    /* close the output file */
    fclose( fp );

    return true;
}

/*============================================================================*/
/*  CORE_fnDumpMemory                                                         */
/*!
    Dump the VM Core Memory

    The CORE_fnDumpMemory function writes the core program memory out as a
    hex dump.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

    @param[in]
        address
            start address for the VM core memory dump

    @param[in]
        length
            length of the VM core memory to output

    @param[in]
        fp
            output FILE * to write to

==============================================================================*/
void CORE_fnDumpMemory( tzCore *pCore,
                        uint32_t address,
                        uint32_t length,
                        FILE *fp )
{
    size_t idx;
    char buffer[17];
    uint8_t val = 0;
    int i = 0;
    size_t loc;

    if( length == 0 )
    {
        /* round up length to multiple of 16 bytes */
        length = ((pCore->programSize / 16) + 1) * 16;
    }

    for(idx = address; idx < address + length && idx < CORE_SIZE; idx++ )
    {
        loc = idx - address;

        if( loc % 16 == 0 )
        {
            fprintf(fp, "\n%08lX: ", idx);
        }

        val = pCore->memory[idx];
        fprintf( fp, "%02X ", val );

        if( isprint( val ) )
        {
            buffer[i++] = val;
        }
        else
        {
            buffer[i++] = '.';
        }

        if( ( loc % 16 ) == 15 )
        {
            buffer[i] = '\0';
            fputs( buffer, fp );
            i = 0;
        }
    }

    buffer[i] = '\0';
    fputs( buffer, fp );
    i = 0;

    fprintf(fp, "\n");
}

/*============================================================================*/
/*  CORE_fnLoad                                                               */
/*!
    Load a program into the VM Core Memory

    The CORE_fnLoad function reads the program memory from the specified file
    into the specified core

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

    @param[in]
        programFile
            pointer to the name of the program file to load

==============================================================================*/
bool CORE_fnLoad( tzCore *pCore, char *programFile )
{
    FILE *fp;
    size_t sz;

    if( pCore == NULL )
    {
        return false;
    }

    fp = fopen( programFile, "r" );
    if( fp == NULL )
    {
        return false;
    }

    fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    if( sz > ( CORE_SIZE - STACK_SIZE ) )
    {
        fclose(fp);
        fprintf(stderr, "Program size exceeds memory capacity\n");
        return false;
    }

    /* read the program into memory */
    fread( pCore->memory, sz, 1, fp );

    /* set the program size */
    pCore->programSize = sz;

    return true;
}

/*============================================================================*/
/*  CORE_fnExecute                                                            */
/*!
    Execute a program

    The CORE_fnExecute function executes the program currently loaded into the
    VM Core memory.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

    @retval true program executed with no error
    @retval false error occurred during program execution

==============================================================================*/
bool CORE_fnExecute(tzCore *pCore)
{
    uint8_t opcode;

    pCore->running = true;

    while( ( pCore->running ) && !(pCore->error) )
    {
        opcode = MEMORY[PC] & 0x1F;
        instructions0[opcode].exec(pCore);
    }

    return (pCore->error ? false : true );
}

/*==============================================================================
        Private Function Definitions
==============================================================================*/

/*============================================================================*/
/*  core_fnCheckInstructionList                                               */
/*!
    Check the VM Core instruction list

    The core_fnCheckInstructionList function checks to see if the opcode
    instruction list has been entered in the correct order.

    @retval true opcode instruction list is ok
    @retval false opcode instruction list order is incorrect

==============================================================================*/
static bool core_fnCheckInstructionList()
{
    uint8_t opcode;

    for(opcode=0;opcode<HRMAXINST;opcode++)
    {
        if( instructions0[opcode].opcode != opcode )
        {
            fprintf( stderr, "Instruction0 list order incorrect\n" );
            return false;
        }

        if( instructions1[opcode].opcode != opcode )
        {
            fprintf( stderr,
                     "Instruction1 list order incorrect: opcode = %d\n",
                     opcode);

            return false;
        }
    }

    /* instruction list is ok */
    return true;
}

/*============================================================================*/
/*  core_fnGetSignedData                                                      */
/*!
    Get a signed 32-bit value from core memory

    The core_fnGetSignedData function retrieves a signed literal value from
    the core memory from the address specified by memory[pc+offset].
    The program counter points to the instruction which encodes the
    size of the value to retrieve.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

    @param[in]
        memory
            pointer to the VM core memory

    @param[in]
        pc
            offset in the core memory where the instruction associated
            with the literal value is stored

    @param[in]
        offset
           offset from the instruction address to the start of the literal value

    @retval 32-bit signed value retrieved from the VM core memory

==============================================================================*/
static int32_t core_fnGetSignedData( tzCore *pCore,
                                     uint8_t *memory,
                                     int32_t pc,
                                     uint8_t offset )
{
    uint8_t *instr;
    uint8_t datatype;
    int32_t val32 = 0;
    int16_t val16;
    int8_t val8;

    instr = (uint8_t *)&memory[pc];
    datatype = *instr & (BYTE | WORD);
    switch( datatype )
    {
        case BYTE:
            val8 = (int8_t)instr[offset];
            val32 = (int32_t)val8;
            INC_PC(1);
            break;

        case WORD:
            val16 = (int16_t)( ( instr[offset] << 8 ) + instr[offset+1] );
            val32 = (int32_t)val16;
            INC_PC(2);
            break;

        case LONG:
        case FLOAT32:
            val32 = (int32_t)(( instr[offset] << 24 ) +
                            ( instr[offset+1] << 16 ) +
                            ( instr[offset+2] << 8 ) +
                            ( instr[offset+3] ));
            INC_PC(4);
            break;

        default:
            printf("invalid data size\n");
            break;
    }

    return val32;
}

/*============================================================================*/
/*  core_fnGetFloatData                                                       */
/*!
    Get a float value from core memory

    The core_fnGetFloatData function retrieves a floating point literal value
    from the core memory from the address specified by memory[pc+offset].
    The program counter points to the instruction which encodes the
    size of the value to retrieve.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

    @param[in]
        memory
            pointer to the VM core memory

    @param[in]
        pc
            offset in the core memory where the instruction associated
            with the literal value is stored

    @param[in]
        offset
           offset from the instruction address to the start of the literal value

    @retval 32-bit floating point value retrieved from the VM core memory

==============================================================================*/
static float core_fnGetFloatData( tzCore *pCore,
                                  uint8_t *memory,
                                  int32_t pc,
                                  uint8_t offset)
{
    uint8_t *instr;

    union
    {
        float fVal;
        uint8_t uVal[4];
    } data;

    instr = (uint8_t *)&memory[pc];

    data.uVal[3] = instr[offset];
    data.uVal[2] = instr[offset+1];
    data.uVal[1] = instr[offset+2];
    data.uVal[0] = instr[offset+3];

    INC_PC(4);

    return data.fVal;
}

/*============================================================================*/
/*  core_fnGetUnsignedData                                                    */
/*!
    Get an unsigned 32-bit value from core memory

    The core_fnGetUnsignedData function retrieves an unsigned 23-bit literal
    value from the core memory from the address specified by memory[pc+offset].
    The program counter points to the instruction which encodes the
    size of the value to retrieve.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

    @param[in]
        memory
            pointer to the VM core memory

    @param[in]
        pc
            offset in the core memory where the instruction associated
            with the literal value is stored

    @param[in]
        offset
           offset from the instruction address to the start of the literal value

    @retval 32-bit unsigned value retrieved from the VM core memory

==============================================================================*/
static uint32_t core_fnGetUnsignedData( tzCore *pCore,
                                        uint8_t *memory,
                                        int32_t pc,
                                        uint8_t offset)
{
    uint8_t *instr;
    uint8_t datatype;
    uint32_t val;

    instr = (uint8_t *)&memory[pc];
    datatype = *instr & (BYTE | WORD);
    switch( datatype )
    {
        case BYTE:
            val = (uint32_t)instr[offset];
            INC_PC(1);
            break;

        case WORD:
            val = (uint32_t)( instr[offset] << 8 ) + instr[offset+1];
            INC_PC(2);
            break;

        case LONG:
        case FLOAT32:
            val = (uint32_t)(( instr[offset] << 24 ) +
                  ( instr[offset+1] << 16 ) +
                  ( instr[offset+2] << 8 ) +
                  ( instr[offset+3] ));
            INC_PC(4);
            break;

        default:
            printf("invalid data size\n");
            break;
    }

    return val;
}

/*============================================================================*/
/*  core_fnStoreData                                                          */
/*!
    Copy a data value from source to destination

    The core_fnStoreData function copies a data value from a source to a
    destination location in the VM core memory.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

    @param[in]
        instr
            pointer to the STR instruction which contains information
            regarding the length of data to be stored.

    @param[in]
        dest
            pointer to the location to store the data

    @param[in]
        src
           pointer to the location to get the data to be stored

==============================================================================*/
static void core_fnStoreData( tzCore *pCore,
                              uint8_t *instr,
                              uint8_t *dest,
                              uint8_t *src )
{
    if( ( *instr & BYTE ) == BYTE )
    {
        *dest = *src;
    }
    else if( ( *instr & WORD ) == WORD )
    {
        /* swap endianness */
        dest[0] = src[1];
        dest[1] = src[0];
    }
    else
    {
        /* swap endianness */
        dest[0] = src[3];
        dest[1] = src[2];
        dest[2] = src[1];
        dest[3] = src[0];
    }
}

/*============================================================================*/
/*  core_fnSetStackData                                                       */
/*!
    Store a data value on the VM core stack

    The core_fnSetStackData function stores a 32-bit value on the
    Virtual Machine Core Stack.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

    @param[in]
        sp
            index to the location on the stack to store the data

    @param[in]
        val
            32-bit data to be stored on the stack

==============================================================================*/
static void core_fnSetStackData( tzCore *pCore, uint32_t sp, uint32_t val )
{
    MEMORY[sp] = (val & 0xFF000000L ) >> 24;
    MEMORY[sp+1] = ( val & 0x00FF0000L ) >> 16;
    MEMORY[sp+2] = ( val & 0x0000FF00L ) >> 8;
    MEMORY[sp+3] = ( val & 0x000000FFL );
}

/*============================================================================*/
/*  core_fnGetStackData                                                       */
/*!
    Get a data value from the VM core stack

    The core_fnGetStackData function gets a 32-bit value from the
    Virtual Machine Core Stack.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

    @param[in]
        sp
            index to the location on the stack where the data is located

    @retval value retrieved from the VM core stack

==============================================================================*/
static uint32_t core_fnGetStackData( tzCore *pCore, uint32_t sp )
{
    uint32_t val;

    val = ( MEMORY[sp] << 24 ) +
          ( MEMORY[sp+1] << 16 ) +
          ( MEMORY[sp+2] << 8 ) +
          MEMORY[sp+3];

    return val;
}

/*==============================================================================
        VM OP CODES
==============================================================================*/

/*============================================================================*/
/*  opNOP                                                                     */
/*!
    NOP - No Operation

    The opNOP function implements the VM 'NOP' operation.  This takes
    no action, it only increments the Program Counter

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opNOP(tzCore *pCore)
{
    INC_PC(1);
}

/*============================================================================*/
/*  opLOD                                                                     */
/*!
    LOD - Load a Register

    The opLOD function implements the VM 'LOD' operation.  This loads
    a register value from another register, or from core memory,

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opLOD(tzCore *pCore)
{
    register uint8_t regs;
    register uint8_t src;
    register uint8_t dst;
    register uint32_t addr;
    register uint32_t val;

    if( ( MEMORY[PC] & MODE_REG ) == MODE_REG )
    {
        regs = MEMORY[PC+1];
        src = regs & 0x0F;
        dst = (( regs >> 4 ) & 0x0F );
        addr = REG[src];
        if ( addr > CORE_SIZE )
        {
            printf( "LOD R[%d],R[%d]: Illegal Address in R[%d]: 0x%X @ 0x%X\n",
                    dst,
                    src,
                    src,
                    addr,
                    PC);
            STOP;
            return;
        }

        /* transfer data from memory to register */
        core_fnStoreData( pCore,
                          &MEMORY[PC],
                          (uint8_t *)&REG[dst],
                          &MEMORY[addr] );

        INC_PC(2);
    }
    else
    {
        dst = MEMORY[PC+1] & 0x0F;
        addr = core_fnGetUnsignedData( pCore, MEMORY, PC, 2);
        if ( addr > CORE_SIZE )
        {
            printf("Illegal Address: 0x%X @ 0x%p\n", addr, MEMORY);
            STOP;
            return;
        }

        /* transfer data from memory to register */
        core_fnStoreData( pCore,
                          &MEMORY[PC],
                          (uint8_t *)&REG[dst],
                          &MEMORY[addr] );

        INC_PC(2);
    }
}

/*============================================================================*/
/*  opSTR                                                                     */
/*!
    STR - Store data from a register to memory

    The opSTR function implements the VM 'STR' operation.  This stores
    a register value to memory from a register.  The memory location
    can be specified in a register or as a literal.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opSTR(tzCore *pCore)
{
    register uint8_t regs;
    register uint8_t src;
    register uint8_t dst;
    register uint32_t addr;

    if( ( MEMORY[PC] & MODE_REG ) == MODE_REG )
    {
        /* store data from a register to an address specified in a register */

        /* get register containing source address */
        regs = MEMORY[PC+1];
        src = regs & 0x0F;

        /* get register containing destination address */
        dst = (( regs >> 4 ) & 0x0F );
        addr = REG[dst];
        if ( addr > CORE_SIZE )
        {
            printf("Illegal Address: 0x%X\n", addr);
            STOP;
            return;
        }

        /* store the data in big endian format */
        core_fnStoreData( pCore,
                          &MEMORY[PC],
                          &MEMORY[addr],
                          (uint8_t *)&REG[src] );
        INC_PC(2);
    }
    else
    {
        /* store data from a register to an address specified as a literal */

        /* get source register */
        src = MEMORY[PC+1] & 0x0F;

        /* get destination address from a literal in memory */
        addr = core_fnGetUnsignedData( pCore, MEMORY, PC, 2);
        if ( addr > CORE_SIZE )
        {
            printf("Illegal Program Address: 0x%X\n", addr);
            STOP;
            return;
        }

        /* store the data in big endian format */
        core_fnStoreData( pCore,
                          &MEMORY[PC],
                          &MEMORY[addr],
                          (uint8_t *)&REG[src] );
        INC_PC(2);
    }
}

/*============================================================================*/
/*  opMOV                                                                     */
/*!
    MOV - Move data from register to register or from memory to register

    The opMOV function implements the VM 'MOV' operation.  This moves data
    from one register to another, or from a memory literal into a register.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opMOV(tzCore *pCore)
{
    register uint8_t regs;
    register uint8_t src;
    register uint8_t dst;
    register int32_t val;

    if( ( MEMORY[PC] & MODE_REG ) == MODE_REG )
    {
        /* move data from one register to another */
        regs = MEMORY[PC+1];
        src = regs & 0x0F;
        dst = (( regs >> 4 ) & 0x0F );
        if(( MEMORY[PC] & FLOAT32 ) == FLOAT32 )
        {
            REGF[dst] = REGF[src];
        }
        else
        {
            REG[dst] = REG[src];
        }
        INC_PC(2);
    }
    else
    {
        /* move data from a memory literal to a register */
        dst = MEMORY[PC+1] & 0x0F;
        if(( MEMORY[PC] & FLOAT32 ) == FLOAT32 )
        {
            REGF[dst] = core_fnGetFloatData( pCore, MEMORY, PC, 2 );
        }
        else
        {
            REG[dst] = core_fnGetSignedData( pCore, MEMORY, PC, 2);
        }
        INC_PC(2);
    }
}

/*============================================================================*/
/*  opADD                                                                     */
/*!
    ADD - Add data from register to register or from memory to register

    The opADD function implements the VM 'ADD' operation.  This adds data from
    one register to another or from a memory literal to a register.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opADD(tzCore *pCore)
{
    register uint8_t regs;
    register uint8_t src;
    register uint8_t dst;
    register int32_t val;
    register int32_t oldvalue;
    register float fvalue;

    if( ( MEMORY[PC] & MODE_REG ) == MODE_REG )
    {
        /* add data from one register to another */
        regs = MEMORY[PC+1];
        src = regs & 0x0F;
        dst = (( regs >> 4 ) & 0x0F );
        if(( MEMORY[PC] & FLOAT32 ) == FLOAT32 )
        {
            REGF[dst] += REGF[src];
            SETFFLAGS(REGF[dst]);
        }
        else
        {
            oldvalue = REG[dst];
            REG[dst] += REG[src];
            SETFLAGS(REG[dst]);
        }
        INC_PC(2);
    }
    else
    {
        /* add data from a memory literal to a register */
        dst = MEMORY[PC+1] & 0x0F;

        if(( MEMORY[PC] & FLOAT32 ) == FLOAT32 )
        {
            REGF[dst] += core_fnGetFloatData( pCore, MEMORY, PC, 2 );
            SETFFLAGS(REGF[dst]);
        }
        else
        {
            val = core_fnGetSignedData( pCore, MEMORY, PC, 2);
            oldvalue = REG[dst];
            REG[dst] += val;
            SETFLAGS(REG[dst]);

        }

        INC_PC(2);
    }
}

/*============================================================================*/
/*  opSUB                                                                     */
/*!
    SUB - Subtract data from register to register or from memory to register

    The opSUB function implements the VM 'SUB' operation.  This subtracts data
    in one register from another or in a memory literal from a register.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opSUB(tzCore *pCore)
{
    register uint8_t regs;
    register uint8_t src;
    register uint8_t dst;
    register int32_t val;
    register int32_t oldvalue;

    if( ( MEMORY[PC] & MODE_REG ) == MODE_REG )
    {
        /* subtract data in one register from another */
        regs = MEMORY[PC+1];
        src = regs & 0x0F;
        dst = (( regs >> 4 ) & 0x0F );
        if(( MEMORY[PC] & FLOAT32 ) == FLOAT32 )
        {
            REGF[dst] -= REGF[src];
            SETFFLAGS(REGF[dst]);
        }
        else
        {
            oldvalue = REG[dst];
            REG[dst] -= REG[src];
            SETFLAGS(REG[dst]);
        }
        INC_PC(2);
    }
    else
    {
        /* subtract data in memory from a register */
        dst = MEMORY[PC+1] & 0x0F;
        if(( MEMORY[PC] & FLOAT32 ) == FLOAT32 )
        {
            REGF[dst] -= core_fnGetFloatData( pCore, MEMORY, PC, 2 );
            SETFFLAGS(REGF[dst]);
        }
        else
        {
            val = core_fnGetSignedData( pCore, MEMORY, PC, 2);
            oldvalue = REG[dst];
            REG[dst] -= val;
            SETFLAGS(REG[dst]);
        }
        INC_PC(2);
    }
}

/*============================================================================*/
/*  opMUL                                                                     */
/*!
    MUL - Multiply data from register to register or from memory to register

    The opMUL function implements the VM 'MUL' operation.  This multiplies data
    in one register with another, or data in a register with a memory literal.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opMUL(tzCore *pCore)
{
    register uint8_t regs;
    register uint8_t src;
    register uint8_t dst;
    register int32_t val;
    register int32_t oldvalue;

    if( ( MEMORY[PC] & MODE_REG ) == MODE_REG )
    {
        /* multiply data in one register with data in another register */
        regs = MEMORY[PC+1];
        src = regs & 0x0F;
        dst = (( regs >> 4 ) & 0x0F );
        if(( MEMORY[PC] & FLOAT32 ) == FLOAT32 )
        {
            REGF[dst] *= REGF[src];
            SETFFLAGS(REGF[dst]);
        }
        else
        {
            oldvalue = REG[dst];
            REG[dst] *= REG[src];
            SETFLAGS(REG[dst]);
        }
        INC_PC(2);
    }
    else
    {
        /* multiply data in a register with a memory literal */
        dst = MEMORY[PC+1] & 0x0F;
        if(( MEMORY[PC] & FLOAT32 ) == FLOAT32 )
        {
            REGF[dst] *= core_fnGetFloatData( pCore, MEMORY, PC, 2 );
            SETFFLAGS(REGF[dst]);
        }
        else
        {
            val = core_fnGetSignedData( pCore, MEMORY, PC, 2);
            oldvalue = REG[dst];
            REG[dst] *= val;
            SETFLAGS(REG[dst]);
        }
        INC_PC(2);
    }
}

/*============================================================================*/
/*  opDIV                                                                     */
/*!
    DIV - Divide data from register to register or from memory to register

    The opDIV function implements the VM 'DIV' operation.  This divides data
    in one register with another, or data in a register with a memory literal.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opDIV(tzCore *pCore)
{
    register uint8_t regs;
    register uint8_t src;
    register uint8_t dst;
    register int32_t val;
    register int32_t oldvalue;

    if( ( MEMORY[PC] & MODE_REG ) == MODE_REG )
    {
        /* divide data in one register with data in another register */
        regs = MEMORY[PC+1];
        src = regs & 0x0F;
        dst = (( regs >> 4 ) & 0x0F );
        if(( MEMORY[PC] & FLOAT32 ) == FLOAT32 )
        {
            REGF[dst] /= REGF[src];
            SETFFLAGS(REGF[dst]);
        }
        else
        {
            oldvalue = REG[dst];
            REG[dst] /= REG[src];
            SETFLAGS(REG[dst]);
        }
        INC_PC(2);
    }
    else
    {
        /* divide data in one register with data from a memory literal */
        dst = MEMORY[PC+1] & 0x0F;
        if(( MEMORY[PC] & FLOAT32 ) == FLOAT32 )
        {
            REGF[dst] /= core_fnGetFloatData( pCore, MEMORY, PC, 2 );
            SETFFLAGS(REGF[dst]);
        }
        else
        {
            val = core_fnGetSignedData( pCore, MEMORY, PC, 2);
            oldvalue = REG[dst];
            REG[dst] /= val;
            SETFLAGS(REG[dst]);
        }
        INC_PC(2);
    }
}

/*============================================================================*/
/*  opTOF                                                                     */
/*!
    TOF - Convert an integer to a float

    The opTOF function implements the VM 'TOF' operation.  This converts
    register data from a 32-bit signed integer to an IEEE754 floating point
    value.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opTOF(tzCore *pCore)
{
    register uint8_t regs;
    register uint8_t reg;

    regs = MEMORY[PC+1];
    reg = regs & 0x0F;

    REGF[reg] = (float)REG[reg];

    INC_PC(2);
}

/*============================================================================*/
/*  opTOI                                                                     */
/*!
    TOI - Convert a float to an integer

    The opTOI function implements the VM 'TOI' operation.  This converts
    register data from a 32-bit IEEE754 floating point value into a
    32-bit signed integer value.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opTOI(tzCore *pCore)
{
    register uint8_t regs;
    register uint8_t reg;
    float offset;

    regs = MEMORY[PC+1];
    reg = regs & 0x0F;

    REG[reg] = (int32_t)REGF[reg];

    INC_PC(2);
}

/*============================================================================*/
/*  opAND                                                                     */
/*!
    AND - Bitwise AND

    The opAND function implements the VM 'AND' operation.  This performs
    a bitwise AND between two registers, or between a register and a literal
    memory value.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opAND(tzCore *pCore)
{
    register uint8_t regs;
    register uint8_t src;
    register uint8_t dst;
    register int32_t val;
    register int32_t oldvalue;

    if( ( MEMORY[PC] & MODE_REG ) == MODE_REG )
    {
        /* perform bitwise AND between source and destination registers */
        regs = MEMORY[PC+1];
        src = regs & 0x0F;
        dst = (( regs >> 4 ) & 0x0F );
        oldvalue = REG[dst];
        REG[dst] &= REG[src];
        INC_PC(2);
    }
    else
    {
        /* perform bitwise AND between dest register and memory literal */
        dst = MEMORY[PC+1] & 0x0F;
        val = core_fnGetSignedData( pCore, MEMORY, PC, 2);
        oldvalue = REG[dst];
        REG[dst] &= val;
        INC_PC(2);
    }

    SETFLAGS(REG[dst]);

}

/*============================================================================*/
/*  opOR                                                                      */
/*!
    OR - Bitwise OR

    The opOR function implements the VM 'OR' operation.  This performs
    a bitwise OR between two registers, or between a register and a literal
    memory value.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opOR(tzCore *pCore)
{
    register uint8_t regs;
    register uint8_t src;
    register uint8_t dst;
    register int32_t val;
    register int32_t oldvalue;

    if( ( MEMORY[PC] & MODE_REG ) == MODE_REG )
    {
        /* perform bitwise OR between source and destination registers */
        regs = MEMORY[PC+1];
        src = regs & 0x0F;
        dst = (( regs >> 4 ) & 0x0F );
        oldvalue = REG[dst];
        REG[dst] |= REG[src];
        INC_PC(2);
    }
    else
    {
        /* perform bitwise OR between dest register and memory literal */
        dst = MEMORY[PC+1] & 0x0F;
        val = core_fnGetSignedData( pCore, MEMORY, PC, 2);
        oldvalue = REG[dst];
        REG[dst] |= val;
        INC_PC(2);
    }

    SETFLAGS(REG[dst]);
}

/*============================================================================*/
/*  opNOT                                                                     */
/*!
    NOT - Bitwise Negation

    The opNOT function implements the VM 'NOT' operation.  This performs
    a bitwise NOT on a register.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opNOT(tzCore *pCore)
{
    register uint32_t reg;

    reg = MEMORY[PC+1] & 0x0F;
    REG[reg] = ~REG[reg];
    INC_PC(2);
}

/*============================================================================*/
/*  opSHR                                                                     */
/*!
    SHR - Right Shift

    The opSHR function implements the VM 'SHR' operation.  This performs
    a right shift on a register.  The number of bits to shift is read from
    an 8-bit memory literal.  The data is shifted as an unsigned type and
    the sign bit is not preserved.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opSHR(tzCore *pCore)
{
    register uint32_t reg;
    register uint32_t shift;
    register uint32_t val;

    /* shift data as unsigned type so the sign bit is not preserved */

    reg = MEMORY[PC+1] & 0x0F;
    shift = core_fnGetUnsignedData( pCore, MEMORY, PC, 2);
    val = REG[reg];
    val >>= shift;
    REG[reg] = val;
    INC_PC(2);
}

/*============================================================================*/
/*  opSHL                                                                     */
/*!
    SHL - Left Shift

    The opSHL function implements the VM 'SHL' operation.  This performs
    a left shift on a register.  The number of bits to shift is read from
    an 8-bit memory literal.  The data is shifted as an unsigned type and
    the sign bit is not preserved.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opSHL(tzCore *pCore)
{
    register uint32_t reg;
    register uint32_t val;
    register uint32_t shift;

    reg = MEMORY[PC+1] & 0x0F;
    shift = core_fnGetUnsignedData( pCore, MEMORY, PC, 2);
    val = REG[reg];
    val <<= shift;
    REG[reg] = val;
    INC_PC(2);
}

/*============================================================================*/
/*  opJMP                                                                     */
/*!
    JMP - Jump to Memory Location

    The opJMP function implements the VM 'JMP' operation.  This loads the
    program counter with the memory literal.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opJMP(tzCore *pCore)
{
    register uint32_t val;

    val = core_fnGetUnsignedData( pCore, MEMORY, PC, 1);
    PC = val;
}

/*============================================================================*/
/*  opJZR                                                                     */
/*!
    JZR - Jump to Memory Location if Z Flag is set

    The opJZR function implements the VM 'JZR' operation.  This loads the
    program counter with the memory literal if the Z flag is set.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opJZR(tzCore *pCore)
{
    if (STATUS & ZFLAG)
    {
        opJMP(pCore);
    }
    else
    {
        INC_PC(3);
    }
}

/*============================================================================*/
/*  opJNZ                                                                     */
/*!
    JNZ - Jump to Memory Location if Z Flag is not set

    The opJNZ function implements the VM 'JNZ' operation.  This loads the
    program counter with the memory literal if the Z flag is not set.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opJNZ(tzCore *pCore)
{
    if (!(STATUS & ZFLAG))
    {
        opJMP(pCore);
    }
    else
    {
        INC_PC(3);
    }
}

/*============================================================================*/
/*  opJNE                                                                     */
/*!
    JNE - Jump to Memory Location if N Flag is set

    The opJNE function implements the VM 'JNE' operation.  This loads the
    program counter with the memory literal if the N flag is set.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opJNE(tzCore *pCore)
{
    if (STATUS & NFLAG)
    {
        opJMP(pCore);
    }
    else
    {
        INC_PC(3);
    }
}

/*============================================================================*/
/*  opJPO                                                                     */
/*!
    JPO - Jump to Memory Location if N Flag is not set

    The opJPO function implements the VM 'JPO' operation.  This loads the
    program counter with the memory literal if the N flag is not set.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opJPO(tzCore *pCore)
{
    if (!(STATUS & NFLAG))
    {
        opJMP(pCore);
    }
    else
    {
        INC_PC(3);
    }
}

/*============================================================================*/
/*  opJCA                                                                     */
/*!
    JCA - Jump to Memory Location if C Flag is set

    The opJCA function implements the VM 'JCA' operation.  This loads the
    program counter with the memory literal if the C flag is set.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opJCA(tzCore *pCore)
{
    if (STATUS & CFLAG)
    {
        opJMP(pCore);
    }
    else
    {
        INC_PC(3);
    }
}

/*============================================================================*/
/*  opJNC                                                                     */
/*!
    JNC - Jump to Memory Location if C Flag is not set

    The opJNC function implements the VM 'JNC' operation.  This loads the
    program counter with the memory literal if the C flag is not set.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opJNC(tzCore *pCore)
{
    if (!(STATUS & ZFLAG))
    {
        opJMP(pCore);
    }
    else
    {
        INC_PC(3);
    }
}

/*============================================================================*/
/*  opCAL                                                                     */
/*!
    CAL - Call a subroutine

    The opCAL function implements the VM 'CAL' operation.  This calls a
    subroutine whose address is specified in a register or as a memory literal.
    The Program Counter is pushed onto the stack so execution can continue
    from this point after the subroutine returns. The program counter is
    loaded with the new subroutine's address.  The call depth and
    scope levels are updated.

    Stack overflow will terminate the virtual machine

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opCAL(tzCore *pCore)
{
    register uint32_t val;
    register uint32_t dst;

    if( ( MEMORY[PC] & MODE_REG ) == MODE_REG )
    {
        dst = MEMORY[PC+1] & 0x0F;
        val = REG[dst];
        INC_PC(2);
    }
    else
    {
        val = core_fnGetUnsignedData( pCore, MEMORY, PC, 1);
        INC_PC(1);
    }

    SP -= sizeof( uint32_t );
    if ( SP < ( CORE_SIZE - STACK_SIZE ) )
    {
        printf("Stack Overflow\n");
        STOP;
        return;
    }

    /* set return address */
    core_fnSetStackData( pCore, SP, PC );

    /* set CALL target */
    PC = val;

    /* increment the call depth */
    pCore->call_depth++;

    /* set the new call depth level on the string buffers */
    STRINGBUFFER_fnSetLevel( pCore->call_depth );
}

/*============================================================================*/
/*  opRET                                                                     */
/*!
    RET - Return from a subroutine

    The opRET function implements the VM 'RET' operation.  This updates
    the scope level and call depth, loads the program counter from the
    stack and resumes execution from the point the subroutine call was made.

    Stack underflow will terminate the Virtual Machine

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opRET(tzCore *pCore)
{
    PC = core_fnGetStackData( pCore, SP ); /* get return address */
    SP += sizeof(uint32_t);
    if( SP > CORE_SIZE )
    {
        printf("Stack Underflow\n");
        STOP;
    }

    /* free any string buffers at this level */
    STRINGBUFFER_fnFree( pCore->call_depth );

    if( pCore->call_depth )
    {
        pCore->call_depth--;
    }
}

/*============================================================================*/
/*  opCMP                                                                     */
/*!
    CMP - Compare two values

    The opCMP function implements the VM 'CMP' operation.  This compares
    two registers or one register and a memory literal, and updates
    the VM status flags.

    Stack underflow will terminate the Virtual Machine

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opCMP(tzCore *pCore)
{
    register uint8_t regs;
    register uint8_t src;
    register uint8_t dst;
    register int32_t val;
    register int32_t oldvalue;
    register float fval;

    if( ( MEMORY[PC] & MODE_REG ) == MODE_REG )
    {
        regs = MEMORY[PC+1];
        src = regs & 0x0F;
        dst = (( regs >> 4 ) & 0x0F );
        if(( MEMORY[PC] & FLOAT32 ) == FLOAT32 )
        {
            fval = REGF[dst] - REGF[src];
            SETFFLAGS(fval);
        }
        else
        {
            oldvalue = REG[dst];
            val = oldvalue - REG[src];
            SETFLAGS(val);
        }
        INC_PC(2);
    }
    else
    {
        dst = MEMORY[PC+1] & 0x0F;
        if(( MEMORY[PC] & FLOAT32 ) == FLOAT32 )
        {
            fval = REGF[dst] - core_fnGetFloatData( pCore, MEMORY, PC, 2 );
            SETFFLAGS(fval);
        }
        else
        {
            val = core_fnGetSignedData( pCore, MEMORY, PC, 2);
            oldvalue = REG[dst];
            val = oldvalue - val;
            SETFLAGS(val);
        }
        INC_PC(2);
    }
}

/*============================================================================*/
/*  opRDN                                                                     */
/*!
    RDN - Read a number from a file

    The opRDN function implements the VM 'RDN' operation.  This reads a
    32-bit integer from the active input file descriptor into a destination
    register.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static  void opRDN(tzCore *pCore)
{
    int userNum;
    register uint8_t dest;
    FILE *fp;

    ReadNum( &userNum );
    dest = MEMORY[PC+2] & 0x0F;
    REG[dest] = userNum;
    INC_PC(3);
}

/*============================================================================*/
/*  opRDC                                                                     */
/*!
    RDC - Read a character from a file

    The opRDC function implements the VM 'RDC' operation.  This reads a
    character from the active input file descriptor into a destination
    register.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opRDC(tzCore *pCore)
{
    char userChar;
    register uint8_t dest;
    FILE *fp;

    ReadChar( &userChar );
    dest = MEMORY[PC+2] & 0x0F;
    REG[dest] = userChar;
    INC_PC(3);
}

/*============================================================================*/
/*  opWRN                                                                     */
/*!
    WRN - Write a number to a file

    The opWRN function implements the VM 'WRN' operation.  This writes a
    number from a register or from a memory literal to the file associated
    with the active output file descriptor.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opWRN(tzCore *pCore)
{
    uint32_t src;
    int32_t val;
    FILE *fp;

    if( ( MEMORY[PC+1] & MODE_REG ) == MODE_REG )
    {
        /* write a number from a register */
        src = MEMORY[PC+2] & 0x0F;

        WriteNum( REG[src] );

        INC_PC(3);
    }
    else
    {
        /* write a number from a memory literal */
        val = core_fnGetSignedData( pCore, MEMORY, PC, 1);
        WriteNum( val );

        INC_PC(2);
    }
}

/*============================================================================*/
/*  opWRC                                                                     */
/*!
    WRC - Write a character to a file

    The opWRC function implements the VM 'WRC' operation.  This writes a
    character from a register or from a memory literal to the file associated
    with the active output file descriptor.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opWRC(tzCore *pCore)
{
    register uint8_t regs;
    register uint8_t src;
    register uint8_t dst;
    register uint32_t val;
    int idx;
    FILE *fp;
    char c;

    if( ( MEMORY[PC+1] & MODE_REG ) == MODE_REG )
    {
        /* write a character from a register */
        idx = MEMORY[PC+2] & 0x0F;

        c = (char)(REG[MEMORY[PC+2] & 0x0F] & 0xFF);
        WriteChar( c );
        INC_PC(3);
    }
    else
    {
        /* write a character from a memory literal */
        val = core_fnGetSignedData( pCore, MEMORY, PC+1, 1);
        c =  (char)( val & 0xFF );
        WriteChar( c );
        INC_PC(2);
    }
}

/*============================================================================*/
/*  opWRF                                                                     */
/*!
    WRF - Write a floating point number to a file

    The opWRF function implements the VM 'WRF' operation.  This writes a
    32-bit IEEE754 floating point number from a register or from a memory
    literal to the file associated with the active output file descriptor.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opWRF(tzCore *pCore)
{
    register uint8_t regs;
    register uint8_t src;
    register uint8_t dst;
    register float val;
    FILE *fp;
    int idx;

    if( ( MEMORY[PC+1] & MODE_REG ) == MODE_REG )
    {
        /* write a float from a register */
        idx = MEMORY[PC+2] & 0x0F;
        val = REGF[MEMORY[PC+2] & 0x0F];
        WriteFloat( val );
        INC_PC(3);
    }
    else
    {
        /* write a float from a memory literal */
        val = core_fnGetFloatData( pCore, MEMORY, PC+1, 1);
        WriteFloat( val );
        INC_PC(2);
    }
}

/*============================================================================*/
/*  opPSH                                                                     */
/*!
    PSH - Push a register value onto the stack

    The opPSH function implements the VM 'PSH' operation.  This pushes
    a register value onto the stack.

    If a stack overflow occurs, the virtual machine will terminate.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opPSH(tzCore *pCore)
{
    uint32_t val;

    SP -= sizeof( uint32_t );
    if ( SP < ( CORE_SIZE - STACK_SIZE ))
    {
        printf("Stack Overflow\n");
        STOP;
        return;
    }

    val = REG[MEMORY[PC+1] & 0x0F];
    core_fnSetStackData( pCore, SP, val );
    INC_PC(2);
}

/*============================================================================*/
/*  opPOP                                                                     */
/*!
    POP - Pop a register value off the stack

    The opPOP function implements the VM 'POP' operation.  This pops a value
    off the stack and into a register.

    If a stack underflow occurs, the virtual machine will terminate.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opPOP(tzCore *pCore)
{
    register uint32_t dst;
    register uint32_t val;

    val = core_fnGetStackData( pCore, SP );
    dst = MEMORY[PC+1] & 0x0F;
    REG[dst] = val;        /* pop to register */
    SP += sizeof(uint32_t);
    if( SP > CORE_SIZE )
    {
        STOP;
        printf("Stack Underflow\n");
    }

    INC_PC(2);
}

/*============================================================================*/
/*  opHLT                                                                     */
/*!
    HLT - Halt the Virtual Machine

    The opHLT function implements the VM 'HLT' operation.  This terminates
    the virtual machine.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opHLT(tzCore *pCore)
{
    INC_PC(1);
    STOP;
}

/*============================================================================*/
/*  opEXT                                                                     */
/*!
    EXT - Get External Variable Handle

    The opEXT function implements the VM 'EXT' operation.  This is a register
    operation which references the name of an external variable stored in
    the VM core memory via a register. It then gets the handle to the
    external variable and stores it in register.

    i.e.

    Rx = Handle( memory[Rx] )

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opEXT(tzCore *pCore)
{
    register uint32_t reg;
    char *name;
    void *pExt = NULL;

    if( pCore != NULL )
    {
        pExt = pCore->pExt;
    }

    reg = MEMORY[PC+1] & 0x0F;
    name = (char *)&MEMORY[REG[reg]];

    REG[reg] = EXTERNVAR_fnGetHandle(pExt, name);

    INC_PC(2);
}

/*============================================================================*/
/*  opGET                                                                     */
/*!
    GET - Get External Variable Value

    The opGET function implements the VM 'GET' operation.  This is a register
    operation which gets the value of an external variable using a handle
    specified in the source register, and stores the external variable
    value into the destination register.

    i.e

    Rd = GetVariable ( Rs )

    In the case the external variable is a string, the destination register
    references a string buffer which will have the external variable string
    appended to it.

    i.e
        StringBuffer(Rd) += GetVariable( Rs )

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opGET(tzCore *pCore)
{
    register uint8_t regs;
    register uint8_t src;
    register uint8_t dst;
    register uint8_t datatype;
    void *pExt = NULL;
    char *pStr;
    int stringbufferID;

    if( pCore != NULL )
    {
        pExt = pCore->pExt;
    }

    datatype = MEMORY[PC] & (BYTE | WORD);

    regs = MEMORY[PC+1];
    src = regs & 0x0F;
    dst = (( regs >> 4 ) & 0x0F );

    switch( datatype )
    {
        case BYTE:
            pStr = EXTERNVAR_fnGetString( pExt, REG[ src ]);
            stringbufferID = REG[dst];
            STRINGBUFFER_fnClear( stringbufferID );
            STRINGBUFFER_fnAppendString( stringbufferID, pStr );
            break;

        case FLOAT32:
            REGF[dst] = EXTERNVAR_fnGetFloat( pExt, REG[src] );
            break;

        default:
            REG[dst] = EXTERNVAR_fnGet( pExt, REG[src] );
            break;
    }

    INC_PC(2);
}

/*============================================================================*/
/*  opSET                                                                     */
/*!
    SET - Set External Variable Value

    The opSET function implements the VM 'SET' operation.  This is a register
    operation which sets the value of an external variable using a handle
    specified in the destination register from the data referenced
    by the source register.

    i.e

        SetVariable ( Rd, Rs )

    In the case the external variable is a string, the source register
    references a string buffer which will contain the string to write.

    i.e
        SetVariable ( Rd, StringBuffer(Rs) )

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opSET(tzCore *pCore)
{
    register uint8_t regs;
    register uint8_t src;
    register uint8_t dst;
    register uint8_t stringbuf_id;
    uint32_t handle;

    void *pExt = NULL;

    if( pCore != NULL )
    {
        pExt = pCore->pExt;
    }

    regs = MEMORY[PC+1];
    src = regs & 0x0F;
    dst = (( regs >> 4 ) & 0x0F );

    register uint8_t datatype;

    datatype = MEMORY[PC] & (BYTE | WORD);
    switch( datatype )
    {
        case HANDLE:
        case BYTE:
            stringbuf_id = REG[src];
            handle = REG[dst];
            EXTERNVAR_fnSetString( pExt,
                                   handle,
                                   STRINGBUFFER_fnGet( stringbuf_id ) );

        case FLOAT32:
            EXTERNVAR_fnSetFloat( pExt,
                                  REG[dst],
                                  REGF[src] );
            break;

        default:
            EXTERNVAR_fnSet( pExt,
                             REG[dst],
                             REG[src] );
            break;
    }

    INC_PC(2);

}

/*============================================================================*/
/*  opRDUMP                                                                   */
/*!
    RDUMP - Register Dump

    The opRDUMP function will dump the VM core registers using the
    CORE_fnDumpRegisters utility function.  It will then prompt the operator
    to continue.  If the operator elects not to continue, the Virtual
    Machine will be stopped.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opRDUMP(tzCore *pCore)
{
    CORE_fnDumpRegisters(pCore, stdout);
    if (ask("continue? y/n: ")==0) STOP;
    INC_PC(3);
}

/*============================================================================*/
/*  opDLY                                                                     */
/*!
    DLY - Delay (millisconds)

    The opDLY function implements the VM 'DLY' operation.  This operation
    will delay execution by the specified number of milliseconds.  The
    delay time can be specified in a register, or via a memory literal.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opDLY(tzCore *pCore)
{
    uint32_t src;
    int32_t val;
    uint32_t delay_us;

    if( ( MEMORY[PC+1] & MODE_REG ) == MODE_REG )
    {
        /* delay time specified in a register */
        src = MEMORY[PC+2] & 0x0F;
        delay_us = REG[src] * 1000;
        usleep( delay_us );
        INC_PC(3);
    }
    else
    {
        /* delay time specified in a memory literal */
        val = core_fnGetUnsignedData( pCore, MEMORY, PC, 1);
        delay_us = val * 1000;
        usleep( delay_us );
        INC_PC(2);
    }
}

/*============================================================================*/
/*  opMDUMP                                                                   */
/*!
    MDUMP - Memory Dump

    The opMDUMP function implements the VM 'MDUMP' operation.  This operation
    will dump the VM core memory at the address specified in the register
    with length specified in the following memory literal.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opMDUMP( tzCore *pCore )
{
    register uint8_t src;
    register uint32_t addr;
    register uint32_t len;

    src = MEMORY[PC+2] & 0x0F;
    addr = REG[src];
    len = core_fnGetUnsignedData( pCore, MEMORY, PC+1, 2 );
    CORE_fnDumpMemory( pCore, addr, len, stdout );

    INC_PC(4);
}

/*============================================================================*/
/*  opWRS                                                                     */
/*!
    WRS - Write String

    The opWRS function implements the VM 'WRS' operation.  This operation
    will dump the string in the VM core memory at the address specified
    in the source register.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opWRS( tzCore *pCore )
{
    register uint8_t src;
    register char *addr;
    FILE *fp = NULL;

    src = MEMORY[PC+2] & 0x0F;

    addr = (char *)&MEMORY[REG[src]];

    WriteString( addr );

    INC_PC(3);
}

/*============================================================================*/
/*  opCSB                                                                     */
/*!
    CSB - Create String Buffer

    The opCSB function implements the VM 'CSB' operation.  This operation
    will create a string buffer with the id specified in the source register.

    CSB Rs
    [in] Rs - string buffer identifier

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opCSB( tzCore *pCore )
{
    register uint8_t stringbuf_id;

    stringbuf_id = REG[MEMORY[PC+2] & 0x0F];
    (void)STRINGBUFFER_fnCreate( stringbuf_id );
    INC_PC(3);
}

/*============================================================================*/
/*  opWSB                                                                     */
/*!
    WSB - Write String Buffer

    The opWSB function implements the VM 'WSB' operation.  This operation
    will write the string buffer with the id specified to the standard
    output.

    WSB Rs
    [in] Rs - string buffer identifier

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opWSB( tzCore *pCore )
{
    register uint8_t stringbuf_id;

    stringbuf_id = REG[MEMORY[PC+2] & 0x0F];
    STRINGBUFFER_fnWrite( stdout, stringbuf_id );
    INC_PC(3);
}

/*============================================================================*/
/*  opEXE                                                                     */
/*!
    EXE - Execute String Buffer

    The opEXE function implements the VM 'EXE' operation.  This operation
    will execute the contents of the specified string buffer via a
    system() call.

    EXE Ra,Rb
    [in] Rb - string buffer identifier
    [out] Ra - command result

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opEXE( tzCore *pCore )
{
    char *pCmd;
    register uint8_t regs;
    register uint8_t Ra;
    register uint8_t Rb;
    register uint8_t stringbuf_id;

    regs = MEMORY[PC+2];
    Ra = (regs & 0xF0) >> 4;
    Rb = regs & 0x0F;

    stringbuf_id = REG[Rb];

    pCmd = STRINGBUFFER_fnGet(stringbuf_id);
    if( pCmd != NULL )
    {
        REG[Ra] = system( pCmd );
    }

    INC_PC(3);
}

/*============================================================================*/
/*  opASN                                                                     */
/*!
    ASN - Append Number to String Buffer

    The opASN function implements the VM 'ASN' operation.  This operation
    will append a number from the source register to the string buffer
    specified in the destination register.

    ASN Ra,Rb
    [in] Ra - destination string buffer identifier
    [in] Rb - number

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opASN( tzCore *pCore )
{
    register uint8_t stringbuf_id;
    register uint8_t dst;
    register uint8_t src;
    register uint8_t regs;
    register int32_t value;

    regs = MEMORY[PC+2];
    dst = (regs & 0xF0) >> 4;
    stringbuf_id = REG[dst];
    src = regs & 0x0F;
    value = REG[src];
    STRINGBUFFER_fnAppendNumber( stringbuf_id, value );
    INC_PC(3);
}

/*============================================================================*/
/*  opASC                                                                     */
/*!
    ASC - Append Character to String Buffer

    The opASC function implements the VM 'ASC' operation.  This operation
    will append a character from the source register to the string buffer
    specified in the destination register.

    ASC Ra,Rb
    [in] Ra - destination string buffer identifier
    [in] Rb - character

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opASC( tzCore *pCore )
{
    register uint8_t stringbuf_id;
    register uint8_t dst;
    register uint8_t src;
    register uint8_t regs;
    register int32_t value;

    regs = MEMORY[PC+2];
    dst = (regs & 0xF0) >> 4;
    stringbuf_id = REG[dst];
    src = regs & 0x0F;
    value = REG[src];
    STRINGBUFFER_fnAppendChar( stringbuf_id, value );
    INC_PC(3);
}

/*============================================================================*/
/*  opASS                                                                     */
/*!
    ASS - Append String to String Buffer

    The opASS function implements the VM 'ASS' operation.  This operation
    will append a string in memory pointed to by the source register to
    to the string buffer specified in the destination register.

    ASS Ra,Rb
    [in] Ra - destination string buffer identifier
    [in] Rb - pointer to source string in memory

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opASS( tzCore *pCore )
{
    register uint8_t stringbuf_id;
    register uint8_t dst;
    register uint8_t src;
    register uint8_t regs;
    register char *value;

    regs = MEMORY[PC+2];
    dst = (regs & 0xF0) >> 4;
    stringbuf_id = REG[dst];
    src = regs & 0x0F;
    value = (char *)&MEMORY[REG[src]];
    STRINGBUFFER_fnAppendString( stringbuf_id, value );
    INC_PC(3);
}

/*============================================================================*/
/*  opASB                                                                     */
/*!
    ASB - Append String Buffer to String Buffer

    The opASB function implements the VM 'ASB' operation.  This operation
    will append a source string buffer to a destination string buffer.

    ASB Ra,Rb
    [in] Ra - destination string buffer identifier
    [in] Rb - source string buffer identifer

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opASB( tzCore *pCore )
{
    register uint8_t dst;
    register uint8_t src;
    register uint8_t regs;

    regs = MEMORY[PC+2];
    dst= REG[((regs & 0xF0) >> 4)];
    src = REG[(regs & 0x0F)];
    STRINGBUFFER_fnAppendBuffer( dst, src );
    INC_PC(3);
}

/*============================================================================*/
/*  opZSB                                                                     */
/*!
    ZSB - Zero String Buffer

    The opZSB function implements the VM 'ZSB' operation.  This operation
    will zero (or clear) the specified string buffer

    ZSB Rs
    [in] Rs - string buffer id

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opZSB( tzCore *pCore )
{
    register uint8_t stringbuf_id;

    stringbuf_id = REG[MEMORY[PC+2] & 0x0F];
    STRINGBUFFER_fnClear( stringbuf_id );
    INC_PC(3);
}

/*============================================================================*/
/*  opASF                                                                     */
/*!
    ASF - Append Float to String Buffer

    The opASF function implements the VM 'ASF' operation.  This operation
    will append the floating point value in the source register to the
    destination string buffer specified by the destination register

    ASF Ra, Rb
    [in] Ra - destination string buffer identifier
    [in] Rb - floating point register value

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opASF( tzCore *pCore )
{
    register uint8_t stringbuf_id;
    register uint8_t dst;
    register uint8_t src;
    register uint8_t regs;
    register float value;

    regs = MEMORY[PC+2];
    dst = (regs & 0xF0) >> 4;
    stringbuf_id = REG[dst];
    src = regs & 0x0F;
    value = REGF[src];
    STRINGBUFFER_fnAppendFloat( stringbuf_id, value );
    INC_PC(3);
}

/*============================================================================*/
/*  setupTimer                                                                */
/*!
    set up a timer

    The setupTimer function sets up the specified timer to fire at
    the specified interval in milliseconds.

    @param[in]
        id
            timer identifier

    @param[in]
        intervalMS
            timer interval in milliseconds

    @retval 0 timer set ok
    @retval -1 error setting timer

==============================================================================*/
static int setupTimer( int id, int intervalMS )
{
    struct sigevent         te;
    struct itimerspec       its;
    int                     sigNo = SIGRTMIN+5;
    long                    secs;
    long                    msecs;
    timer_t                 *timerID;
    int                     result = -1;

    secs = intervalMS / 1000;
    msecs = intervalMS % 1000;

    if( ( id > 0 ) && ( id < MAX_TIMERS ) )
    {
        timerID = &timers[id];

        /* Set and enable alarm */
        te.sigev_notify = SIGEV_SIGNAL;
        te.sigev_signo = sigNo;
        te.sigev_value.sival_int = id;
        timer_create(CLOCK_REALTIME, &te, timerID);

        its.it_interval.tv_sec = secs;
        its.it_interval.tv_nsec = msecs * 1000000L;
        its.it_value.tv_sec = secs;
        its.it_value.tv_nsec = msecs * 1000000L;
        timer_settime(*timerID, 0, &its, NULL);

        result = 0;
    }

    return result;
}

/*============================================================================*/
/*  waitSignal                                                                */
/*!
    wait for a realtime signal

    The waitSignal function waits for one of the system real-time
    signals: [SIGRTMIN+5 .. SIGRTMIN+9]

    The function blocks until one of these signals is received.

    When the signal is received the signal id and signal value are
    returned to the caller.

    @param[in]
        signum
            pointer to the location to store the received signal number

    @param[in]
        id
            pointer to the location to store the received signal id

    @retval 0 signal received ok
    @retval -1 error receiving signal

==============================================================================*/
static int waitSignal( int *signum, int *id )
{
    sigset_t mask;
    siginfo_t info;
    int result = -1;
    int timer_id = 0;
    int sig;

    if( ( signum != NULL ) &&
        ( id != NULL ) )
    {
        sigemptyset( &mask );
        /* timer notification */
        sigaddset( &mask, SIGRTMIN+5 );
        /* modified notification */
        sigaddset( &mask, SIGRTMIN+6 );
        /* calc notification */
        sigaddset( &mask, SIGRTMIN+7 );
        /* validate notification */
        sigaddset( &mask, SIGRTMIN+8 );
        /* print notification */
        sigaddset( &mask, SIGRTMIN+9 );

        sigprocmask( SIG_BLOCK, &mask, NULL );

        sig = sigwaitinfo( &mask, &info );
        *signum = sig;
        *id = info.si_value.sival_int;

        result = 0;
    }

    return result;
}

/*============================================================================*/
/*  opSTM                                                                     */
/*!
    STM - Set Timer

    The opSTM function implements the VM 'STM' operation.  This operation
    will set up the timer specified in the destination register with the
    delay specified in the source register

    STM Rd, Rs
    [in] Rd - timer identifier
    [in] Rs - timer delay

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opSTM( tzCore *pCore )
{
    int timer_id;
    register uint32_t timer_ms;
    register uint8_t dst;
    register uint8_t src;
    register uint8_t regs;

    regs = MEMORY[PC+2];
    dst = (regs & 0xF0) >> 4;
    src = regs & 0x0F;
    timer_ms = REG[src];
    timer_id = REG[dst];

    if ( setupTimer( timer_id, timer_ms ) == -1 )
    {
        fprintf(stderr, "Illegal timer\n");
        CORE_fnDumpRegisters( pCore, stderr );
        STOP;
    }

    INC_PC(3);

}

/*============================================================================*/
/*  opCTM                                                                     */
/*!
    CTM - Clear Timer

    The opCTM function implements the VM 'CTM' operation.  This operation
    will clear the timer specified in the source register.

    If an invalid timer ID is specified the virtual machine will stop
    and throw an error.

    CTM Rs
    [in] Rs - timer delay

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opCTM( tzCore *pCore )
{
    register uint8_t src;
    int timer_id;

    src = MEMORY[PC+2] & 0x0F;
    timer_id = REG[src];

    if( ( timer_id > 0 ) && ( timer_id < MAX_TIMERS ) )
    {
        printf("deleting timer %d\n", timer_id);
        timer_delete( timers[timer_id] );

        INC_PC(3);
    }
    else
    {
        fprintf(stderr, "Illegal timer\n");
        CORE_fnDumpRegisters( pCore, stderr );
        STOP;
    }
}

/*============================================================================*/
/*  opNFY                                                                     */
/*!
    NFY - Request Notification

    The opNFY function implements the VM 'NFY' operation.  This operation
    will request an external variable notification for the external
    variable specified in the destination register, with the request type
    in the source register.

    NFY Rd, Rs
    [in] Rd - external variable handle
    [in] Rs - notification type

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opNFY( tzCore *pCore )
{
    register uint8_t regs;
    register uint8_t src;
    register uint8_t dst;
    uint32_t request;
    uint32_t handle;
    int rc;

    void *pExt = NULL;

    if( pCore != NULL )
    {
        pExt = pCore->pExt;
    }

    regs = MEMORY[PC+2];
    src = regs & 0x0F;
    dst = (( regs >> 4 ) & 0x0F );

    handle = REG[dst];
    request = REG[src];

    rc = EXTERNVAR_fnNotify( pExt, handle, request );
    if( rc != 0 )
    {
        fprintf( stderr, "Notification Request Failure\n" );
        CORE_fnDumpRegisters( pCore, stderr );
        STOP;
    }

    INC_PC(3);
}

/*============================================================================*/
/*  opWFS                                                                     */
/*!
    WFS - Wait For Signal

    The opWFS function implements the VM 'WFS' operation.  This operation
    will wait for a real time signal via the waitSignal function and
    store the received signal number in Ra and the signal id in Rb.

    WFS Ra, Rb
    [out] Ra - received signal number
    [out] Rb - received signal id

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opWFS( tzCore *pCore )
{
    register uint8_t r1;
    register uint8_t r2;
    register uint8_t regs;
    register uint32_t signal;
    int signum;
    int id;

    regs = MEMORY[PC+2];
    r1 = (regs & 0xF0) >> 4;
    r2 = regs & 0x0F;

    waitSignal( &signum, &id );

    REG[r1] = signum;
    REG[r2] = id;

    INC_PC(3);
}

/*============================================================================*/
/*  opEVS                                                                     */
/*!
    EVS - External Validation Start

    The opEVS function implements the VM 'EVS' operation.  This operation
    will start an external variable validation using a received validation
    notification reference in Rb, and will return the handle of the
    variable being validated in Ra.

    EVS Ra, Rb
    [out] Ra - handle of the variable being validated
    [in] Rb - validation notification reference

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opEVS( tzCore *pCore )
{
    register uint8_t Ra;
    register uint8_t Rb;
    register uint8_t regs;
    void *pExt = NULL;
    int rc;
    uint32_t handle;
    uint32_t hVar;

    if( pCore != NULL )
    {
        pExt = pCore->pExt;
    }

    regs = MEMORY[PC+2];
    Ra = (regs & 0xF0) >> 4;
    Rb = regs & 0x0F;

    handle = REG[Rb];
    rc = EXTERNVAR_fnValidateStart( pExt, handle, &hVar );
    if( rc == 0 )
    {
        REG[Ra] = hVar;
    }
    else
    {
        fprintf( stderr, "Notification Start Failure\n" );
        CORE_fnDumpRegisters( pCore, stderr );
        STOP;
    }

    INC_PC(3);
}

/*============================================================================*/
/*  opEVE                                                                     */
/*!
    EVE - External Validation End

    The opEVE function implements the VM 'EVE' operation.  This operation
    will end an external variable validation using a received validation
    notification reference in Rb, and will return the handle of the
    variable being validated in Ra.

    EVE Ra, Rb
    [in] Ra - validation notification reference
    [in] Rb - validation result (0 = ok, or non-zero = errno)

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opEVE( tzCore *pCore )
{
    register uint8_t Ra;
    register uint8_t Rb;
    register uint8_t regs;
    void *pExt = NULL;
    uint32_t handle;
    uint32_t response;

    if( pCore != NULL )
    {
        pExt = pCore->pExt;
    }

    regs = MEMORY[PC+2];
    Ra = (regs & 0xF0) >> 4;
    Rb = regs & 0x0F;

    handle = REG[Ra];
    response = REG[Rb];

    EXTERNVAR_fnValidateEnd( pExt, handle, response );

    INC_PC(3);
}

/*============================================================================*/
/*  opSBL                                                                     */
/*!
    SBL - String Buffer Length

    The opSBL function implements the VM 'SBL' operation.  This operation
    will get the length of the string buffer specified in Rb and put the
    length into Ra.

    SBL Ra, Rb
    [out] Ra - string buffer length
    [in] Rb - string buffer id

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opSBL( tzCore *pCore )
{
    register uint8_t Ra;
    register uint8_t Rb;
    register uint8_t regs;
    uint32_t stringbuf_id;
    size_t len;

    regs = MEMORY[PC+2];
    Ra = (regs & 0xF0) >> 4;
    Rb = regs & 0x0F;

    stringbuf_id = REG[Rb];

    len = STRINGBUFFER_fnGetLength( stringbuf_id );

    REG[Ra] = len;

    INC_PC(3);
}

/*============================================================================*/
/*  opSBO                                                                     */
/*!
    SBO - String Buffer Offset

    The opSBO function implements the VM 'SBO' operation.  This operation
    will set the read/write offset of the string buffer specified in
    Ra with the offset specified in Rb.

    SBO Ra, Rb
    [in] Ra - string buffer id
    [in] Rb - read/write offset

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opSBO( tzCore *pCore )
{
    register uint8_t Ra;
    register uint8_t Rb;
    register uint8_t regs;
    uint32_t stringbuf_id;
    uint32_t offset;
    size_t len;
    char c;

    regs = MEMORY[PC+2];
    Ra = (regs & 0xF0) >> 4;
    Rb = regs & 0x0F;

    stringbuf_id = REG[Ra];
    offset = REG[Rb];

    STRINGBUFFER_fnSetRWOffset( stringbuf_id, offset );

    INC_PC(3);
}

/*============================================================================*/
/*  opGCO                                                                     */
/*!
    GCO - Get Character at Offset

    The opGCO function implements the VM 'GCO' operation.  This operation
    will get the character at the current read/write offset of the string buffer
    specified in Rb.  The character will be stored in Ra

    GCO Ra, Rb
    [out] Ra - character at current read/write offset in stringbuffer
    [in] Rb - string buffer id

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opGCO( tzCore *pCore )
{
    register uint8_t Ra;
    register uint8_t Rb;
    register uint8_t regs;
    uint32_t stringbuf_id;
    uint32_t offset;
    size_t len;
    char c;

    regs = MEMORY[PC+2];
    Ra = (regs & 0xF0) >> 4;
    Rb = regs & 0x0F;

    stringbuf_id = REG[Rb];

    c = STRINGBUFFER_fnGetCharAtOffset( stringbuf_id );

    REG[Ra] = c;

    INC_PC(3);
}

/*============================================================================*/
/*  opSCO                                                                     */
/*!
    SCO - Set Character at Offset

    The opSCO function implements the VM 'SCO' operation.  This operation
    will set the character at the current read/write offset of the string buffer
    specified in Ra with the character specified in Rb or a character literal
    from the memory core.

    Character from register

    SCO Ra, Rb
    [in] Ra - string buffer id
    [in] Rb - character to write

    Literal character from memory core

    SCO Ra, n
    [in] Ra - string buffer id
    [in] n - literal character

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opSCO( tzCore *pCore )
{
    register uint8_t Ra;
    register uint8_t Rb;
    register uint8_t regs;
    uint32_t stringbuf_id;
    uint32_t offset;
    size_t len;
    char c;

    if( ( MEMORY[PC+1] & MODE_REG ) == MODE_REG )
    {
        regs = MEMORY[PC+2];
        Ra = (regs & 0xF0) >> 4;
        Rb = regs & 0x0F;

        stringbuf_id = REG[Ra];

        c = REG[Rb];

        INC_PC( 3 );
        STRINGBUFFER_fnSetCharAtOffset( stringbuf_id, c );
    }
    else
    {
        regs = MEMORY[PC+2];
        Ra = regs & 0x0F;

        stringbuf_id = REG[Ra];

        c = MEMORY[PC+3];

        INC_PC(4);
    }

    STRINGBUFFER_fnSetCharAtOffset( stringbuf_id, c );
}

/*============================================================================*/
/*  opOFD                                                                     */
/*!
    OFD - Open File Descriptor

    The opOFD function implements the VM 'OFD' operation.  This operation
    will open a file and assign a file descriptor. The file can be opened
    either in read ('r' or 'R') or write ('w' or 'W') mode.
    Ra specifies a stringbuffer id of a string buffer which contains the name
    of the file to open. Rb contains the read/write open mode.
    The read/write mode can also be specified as a character literal.
    The file descriptor of the open file is written back to Ra.  If the
    file fails to open then -1 is written back to Ra.

    Open mode from register

    OFD Ra, Rb
    [in] Ra - id of string buffer containing file name
    [in] Rb - open mode: one of 'r', 'w', 'R', or 'W'
    [out] Ra - file descriptor of open file

    Open mode from literal

    SCO Ra, n
    [in] Ra - id of string buffer containing file name
    [in] n - open mode literal character:  one of: 'r', 'R', 'w', 'W'
    [out] Ra - file descriptor of open file

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opOFD( tzCore *pCore )
{
    register uint8_t Ra;
    register uint8_t Rb;
    register uint8_t regs;
    uint32_t stringbuf_id;
    char mode;
    uint32_t fd;

    regs = MEMORY[PC+2];

    if( ( MEMORY[PC+1] & MODE_REG ) == MODE_REG )
    {
        Ra = (regs & 0xF0) >> 4;
        Rb = regs & 0x0F;

        stringbuf_id = REG[Ra];
        mode = REG[Rb];
        INC_PC(3);

    }
    else
    {
        Ra = regs & 0x0F;

        stringbuf_id = REG[Ra];

        mode = MEMORY[PC+3];

        INC_PC(4);
    }

    if( OpenFileDescriptor( stringbuf_id, mode, &fd ) == EOK )
    {
        REG[Ra] = fd;
    }
    else
    {
        REG[Ra] = -1;
    }
}

/*============================================================================*/
/*  opCFD                                                                     */
/*!
    CFD - Close File Descriptor

    The opCFD function implements the VM 'CFD' operation.  This operation
    will close the file with the file descriptor specified in Ra.

    CFD Ra,
    [in] Ra - file descriptor of open file

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opCFD( tzCore *pCore )
{
    register uint8_t src;
    uint32_t fd;

    src = MEMORY[PC+2] & 0x0F;
    fd = REG[src];

    INC_PC(3);

    CloseFileDescriptor( fd );
}

/*============================================================================*/
/*  opSFD                                                                     */
/*!
    SFD - Select File Descriptor

    The opSFD function implements the VM 'SFD' operation.  This operation
    will set the active file descriptor used by the I/O operations to the
    file descriptor specfied in Ra.

    SFD Ra,
    [in] Ra - file descriptor of open file

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opSFD( tzCore *pCore )
{
    register uint8_t src;
    uint32_t fd;

    src = MEMORY[PC+2] & 0x0F;
    fd = REG[src];

    INC_PC(3);

    SetActiveFileDescriptor( fd );
}

/*============================================================================*/
/*  opOPS                                                                     */
/*!
    OPS - Open Print Session

    The opOPS function implements the VM 'OPS' operation.  This operation
    is used to open a print session in response to a PRINT notification.
    Ra specifies the print notifcation handle (id recevied from WFS)
    The output file descriptor is returned in Ra, and the external variable
    handle is returned in Rb

    OPS Ra, Rb
    [in] Ra - print notification handle
    [out] Ra - output file descriptor
    [out] Rb - external variable handle

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opOPS( tzCore *pCore )
{
    register uint8_t Ra;
    register uint8_t Rb;
    register uint8_t regs;
    void *pExt = NULL;
    uint32_t handle;
    uint32_t hVar;
    FILE *fp;
    int fd;

    if( pCore != NULL )
    {
        pExt = pCore->pExt;
    }

    regs = MEMORY[PC+2];
    Ra = (regs & 0xF0) >> 4;
    Rb = regs & 0x0F;

    handle = REG[Ra];

    if ( EXTERNVAR_fnOpenPrintSession( pExt, handle, &hVar, &fd ) == EOK )
    {
        SetExternWriteFileDescriptor( fd, 'w' );
        SetActiveFileDescriptor( fd );
        REG[Rb] = hVar;
        REG[Ra] = fd;
    }
    else
    {
        REG[Ra] = 0;
        REG[Rb] = 0;
    }

    INC_PC(3);
}

/*============================================================================*/
/*  opCPS                                                                     */
/*!
    CPS - Close Print Session

    The opCPS function implements the VM 'CPS' operation.  This operation
    is used to close or terminate a print session.
    Ra specifies the print notifcation handle (id recevied from WFS)
    Rb specifies the output file descriptor

    CPS Ra, Rb
    [in] Ra - notification handle
    [in] Rb - output file descriptor

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opCPS( tzCore *pCore )
{
    register uint8_t Ra;
    register uint8_t Rb;
    register uint8_t regs;
    void *pExt = NULL;
    uint32_t handle;
    int fd;

    if( pCore != NULL )
    {
        pExt = pCore->pExt;
    }

    regs = MEMORY[PC+2];
    Ra = (regs & 0xF0) >> 4;
    Rb = regs & 0x0F;

    handle = REG[Ra];
    fd = REG[Rb];

    EXTERNVAR_fnClosePrintSession( pExt, handle, fd );

    ClearExternFileDescriptor( fd );

    INC_PC(3);
}

/*============================================================================*/
/*  opINST1                                                                   */
/*!
    Instruction Set 1

    The opINST1 function selects the first set of extension operations
    The opcode for the extension operation is obtained from MEMORY[PC+1]
    and executed via the instructions1 operation map.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opINST1(tzCore *pCore)
{
    uint8_t opcode;

    opcode = MEMORY[PC+1] & 0x1F;
    instructions1[opcode].exec(pCore);
}

/*============================================================================*/
/*  opINST2                                                                   */
/*!
    Instruction Set 2

    The opINST2 function selects the second set of extension operations
    The opcode for the extension operation is obtained from MEMORY[PC+2]
    and executed via the instructions2 operation map.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opINST2(tzCore *pCore)
{
    uint8_t opcode;

    opcode = MEMORY[PC+2] & 0x1F;
    instructions2[opcode].exec(pCore);
}

/*============================================================================*/
/*  opILLEGAL                                                                 */
/*!
    Illegal Operation

    The opILLEGAL function is triggered for any illegal operation.
    It outputs an error message and stops the virtual machine.

    @param[in]
        pCore
            pointer to the tzCore object representing the virtual memory core

==============================================================================*/
static void opILLEGAL(tzCore *pCore)
{
    printf("Illegal operation\n");
    CORE_fnDumpRegisters( pCore, stderr );
    STOP;
}

/*! @}
 * end of core group */