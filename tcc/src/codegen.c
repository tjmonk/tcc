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
 * @defgroup codegen codegen
 * @brief Assmebly Code Generation for tcc compiler
 * @{
 */

/*==============================================================================
        Includes
==============================================================================*/

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"
#include "Registers.h"
#include "labels.h"
#include "lineno.h"
#include "typecheck.h"
#include "tcc.tab.h"   /* for bison generated token values */
#include "codegen.h"

/*==============================================================================
        Definitions
==============================================================================*/

#define SUPPORTCODE "tcc_support.v"

/*! defines the break statement type */
typedef enum eBREAK_TYPE
{
    /*! unknown (invalid) break statement */
    eBreakTypeUnknown = 0,

    /*! break statement associated with a while loop */
    eBreakTypeWhile,

    /*! break statement associated with a for loop */
    eBreakTypeFor,

    /*! break statement associated with a switch statement */
    eBreakTypeSwitch
} teBREAK_TYPE;

/*! Code Generator State object */
struct _CodeGen
{
    /*! handle to the output stream for the generated code */
    FILE *fp;

};

/*==============================================================================
        File Scoped Variables
==============================================================================*/

/*! track if level, used in IF label generation */
static int ifLevel = 0;

/*! track level of FOR loop nesting, used in FOR label generation */
static int forLevel = 0;

/*! track break level, used in break label generation  */
static int breakLevel = 0;

/*! track level for WHILE loop nesting, used in WHILE label generation */
static int whileLevel = 0;

/*! track level for SWITCH loop nesting */
static int switchLevel = 0;

/*! track case label index */
static int caseLabelIndex = 0;

/*! used for generating labels in an IF loop */
static char startELSE[7][10];

/*! used for generating labels in an IF loop */
static char endELSE[7][10];

/*! used for generating labels in a FOR loop */
static char startFOR[7][10];

/*! used for generating labels in a FOR loop */
static char endFOR[7][10];

/*! used for generating labels in a WHILE loop */
static char startWHILE[7][10];

/*! used for generating labels in a WHILE loop */
static char endWHILE[7][10];

/*! used for generating labels in a switch statement */
static char startSWITCH[7][10];

/*! used for generating labels in a switch statement */
static char endSWITCH[7][10];

/*! used for generating labels in a switch statement */
static char dfltSWITCH[7][10];

/*! stack for managing break types */
static teBREAK_TYPE breakType;

/*! stack for managing break types */
static teBREAK_TYPE breakTypes[20];

/*! track the register ID used in append register operations */
static int append_reg;

/*! used to track if 'default' case is found in switch/case statements */
static bool foundDefault[7];

/* used for storing the register we are using for register comparison */
static int32_t regSwitch[7];

/*==============================================================================
        Private function declarations
==============================================================================*/

static int generateProgram( CodeGen *pCodeGen, struct Node *root );

static int generateDeclarationID( CodeGen *pCodeGen, struct Node *root );
static int generateLValueID( CodeGen *pCodeGen, struct Node *root );
static int generateExternLValueID( CodeGen *pCodeGen, struct Node *root );

static int generateInvocationID( CodeGen *pCodeGen, struct Node *root );

static int generateFuncID( CodeGen *pCodeGen, struct Node *root );
static int generateFuncDefList( CodeGen *pCodeGen, struct Node *root );
static int generateFuncDef( CodeGen *pCodeGen, struct Node *root );
static int generateFuncDef1( CodeGen *pCodeGen, struct Node *root );

static int generateID( CodeGen *pCodeGen, struct Node *root );

static int generateNum( CodeGen *pCodeGen, struct Node *root );
static int generateFloat( CodeGen *pCodeGen, struct Node *root );
static int generateCharStr( CodeGen *pCodeGen, struct Node *root );
static int generateChar( CodeGen *pCodeGen, struct Node *root );

static int generateFuncHdr( CodeGen *pCodeGen, struct Node *root );
static int generateFuncHdr1( CodeGen *pCodeGen, struct Node *root );

static int generateStatementList( CodeGen *pCodeGen, struct Node *root );
static int generateCompoundStatement( CodeGen *pCodeGen, struct Node *root );

static int generateIf( CodeGen *pCodeGen, struct Node *root );
static int generateElse( CodeGen *pCodeGen, struct Node *root );

static int generateFor( CodeGen *pCodeGen, struct Node *root );
static int generateFor1( CodeGen *pCodeGen, struct Node *root );
static int generateFor2( CodeGen *pCodeGen, struct Node *root );

static int generateWhile( CodeGen *pCodeGen, struct Node *root );
static int generateBreak( CodeGen *pCodeGen, struct Node *root );
static int generateReturn( CodeGen *pCodeGen, struct Node *root );

static int generateSwitch( CodeGen *pCodeGen, struct Node *root );
static int generateCase( CodeGen *pCodeGen, struct Node *root );
static int generateCase1( CodeGen *pCodeGen, struct Node *root );
static int generateDefault( CodeGen *pCodeGen, struct Node *root );

static int generateAppendList( CodeGen *pCodeGen, struct Node *root );
static int generateAppend( CodeGen *pCodeGen, struct Node *root );
static int generateLength( CodeGen *pCodeGen, struct Node *root );
static int generateCharAt( CodeGen *pCodeGen, struct Node *root );
static int generateSetAt( CodeGen *pCodeGen, struct Node *root );
static int generateSetAt1( CodeGen *pCodeGen, struct Node *root );

static int generateAssign( CodeGen *pCodeGen, struct Node *root );

static int generateWrite( CodeGen *pCodeGen, struct Node *root );
static int generateWriteLn( CodeGen *pCodeGen, struct Node *root );
static int generateRead( CodeGen *pCodeGen, struct Node *root );
static int generateOutputList( CodeGen *pCodeGen, struct Node *root );
static int generateInputList( CodeGen *pCodeGen, struct Node *root );

static int generateDeclarationList( CodeGen *pCodeGen, struct Node *root );
static int generateDeclaration( CodeGen *pCodeGen, struct Node *root );
static int generateArrayDeclaration( CodeGen *pCodeGen, struct Node *root );
static int generateArray( CodeGen *pCodeGen, struct Node *root );

static int generateExternDeclarationList( CodeGen *pCodeGen,
                                          struct Node *root );

static int generateExternDeclaration( CodeGen *pCodeGen, struct Node *root );

static int generateProcCall( CodeGen *pCodeGen, struct Node *root );
static int generateArgList( CodeGen *pCodeGen, struct Node *root );
static int generateParameterList( CodeGen *pCodeGen, struct Node *root );
static int generateParameter( CodeGen *pCodeGen, struct Node *root );

static int generateDelay( CodeGen *pCodeGen, struct Node *root );
static int generateWaitSig( CodeGen *pCodeGen, struct Node *root );
static int generateSetTimer( CodeGen *pCodeGen, struct Node *root );
static int generateClearTimer( CodeGen *pCodeGen, struct Node *root );
static int generateNotify( CodeGen *pCodeGen, struct Node *root );
static int generateHandle( CodeGen *pCodeGen, struct Node *root );
static int generateValidateStart( CodeGen *pCodeGen, struct Node *root );
static int generateValidateEnd( CodeGen *pCodeGen, struct Node *root );
static int generateOpenPrintSession( CodeGen *pCodeGen, struct Node *root );
static int generateClosePrintSession( CodeGen *pCodeGen, struct Node *root );
static int generateSystem( CodeGen *pCodeGen, struct Node *root );
static int generateFileOpen( CodeGen *pCodeGen, struct Node *root );
static int generateFileClose( CodeGen *pCodeGen, struct Node *root );
static int generateFileRead( CodeGen *pCodeGen, struct Node *root );
static int generateFileWrite( CodeGen *pCodeGen, struct Node *root );

static int generateAnd( CodeGen *pCodeGen, struct Node *root );
static int generateOr( CodeGen *pCodeGen, struct Node *root );
static int generateBand( CodeGen *pCodeGen, struct Node *root );
static int generateBnot( CodeGen *pCodeGen, struct Node *root );
static int generateBor( CodeGen *pCodeGen, struct Node *root );
static int generateXor( CodeGen *pCodeGen, struct Node *root );
static int generateNot( CodeGen *pCodeGen, struct Node *root );
static int generateToInt( CodeGen *pCodeGen, struct Node *root );
static int generateToFloat( CodeGen *pCodeGen, struct Node *root );

static int generateTimesEquals( CodeGen *pCodeGen, struct Node *root );
static int generateDivEquals( CodeGen *pCodeGen, struct Node *root );
static int generatePlusEquals( CodeGen *pCodeGen, struct Node *root );
static int generateMinusEquals( CodeGen *pCodeGen, struct Node *root );
static int generateAndEquals( CodeGen *pCodeGen, struct Node *root );
static int generateOrEquals( CodeGen *pCodeGen, struct Node *root );
static int generateXorEquals( CodeGen *pCodeGen, struct Node *root );

static int generateEquals( CodeGen *pCodeGen, struct Node *root );
static int generateNotEquals( CodeGen *pCodeGen, struct Node *root );
static int generateLessThanOrEqual( CodeGen *pCodeGen, struct Node *root );
static int generateGreaterThanOrEqual( CodeGen *pCodeGen, struct Node *root );
static int generateLessThan( CodeGen *pCodeGen, struct Node *root );
static int generateGreaterThan( CodeGen *pCodeGen, struct Node *root );

static int generateRShift( CodeGen *pCodeGen, struct Node *root );
static int generateLShift( CodeGen *pCodeGen, struct Node *root );

static int generateIncrement( CodeGen *pCodeGen, struct Node *root );
static int generateDecrement( CodeGen *pCodeGen, struct Node *root );

static int generatePlus( CodeGen *pCodeGen, struct Node *root );
static int generateMinus( CodeGen *pCodeGen, struct Node *root );
static int generateTimes( CodeGen *pCodeGen, struct Node *root );
static int generateDivide( CodeGen *pCodeGen, struct Node *root );

static int GetExternal( CodeGen *pCodeGen,
                         struct identEntry *ident,
                         int src,
                         char *comment );

static int SetExternal( CodeGen *pCodeGen,
                  struct identEntry *ident,
                  int src,
                  char *comment );

static int generateLeftChild( CodeGen *pCodeGen, struct Node *root );
static int generateChildren( CodeGen *pCodeGen, struct Node *root );
static int generateOutputID( CodeGen *pCodeGen,
                             struct identEntry *ident,
                             int a );

static void Append( int dst, int src, struct Node *root, CodeGen *pCodeGen );
static bool isExternal( struct Node *root );
static struct identEntry *GetIdentEntry( struct Node *root );
static bool isStringBuffer( struct identEntry *idEntry );

/*==============================================================================
        Private Function Definitions
==============================================================================*/

/*============================================================================*/
/*  codegen_init                                                              */
/*!
    Initialize the code generator

    The codegen_init function allocates a CodeGen object and initializes
    it with the specified file pointer.

    @param[in]
        fp
            output FILE *


    @retval pointer to the initialized CodeGen object
    @retval NULL if memory allocation failed

==============================================================================*/
CodeGen *codegen_init( FILE *fp )
{
    CodeGen *pCodeGen = NULL;

    if( fp != NULL )
    {
        breakTypes[0] = eBreakTypeUnknown;

        pCodeGen = (CodeGen *)calloc( 1, sizeof( CodeGen ) );
        if( pCodeGen != NULL )
        {
            pCodeGen->fp = fp;
        }
    }

    return pCodeGen;
}

/*============================================================================*/
/*  GenerateCode                                                              */
/*!
    Generate assembly code from the specified parse tree

    The GenerateCode function generates code recursively from the root node
    of the parse tree (generated by the parser).  Code is written to the
    output FILE * in the specified CodeGen object.
    Each node in the parse tree generates its own section of code.

    @param[in]
        pCodeGen
            pointer to the CodeGen object

    @param[in]
        root
            pointer to the root node in the parse (sub)tree

    @retval pointer to the initialized CodeGen object
    @retval NULL if memory allocation failed

==============================================================================*/
int GenerateCode( CodeGen *pCodeGen, struct Node *root)
{
    int result = -1;

    if (root == NULL)
    {
        return(-1); /* do nothing on empty node */
    }

    switch(root->type)
    {
        case PROGRAM:
            result = generateProgram( pCodeGen, root );
            break;

        case FUNC_DEF_LIST:
            result = generateFuncDefList( pCodeGen, root );
            break;

        case FUNC_DEF:
            result = generateFuncDef( pCodeGen, root );
            break;

        case FUNC_DEF1:
            result = generateFuncDef1( pCodeGen, root );
            break;

        case ID:
            result = generateID( pCodeGen, root );
            break;

        case LVAL_ID:
            result = generateLValueID( pCodeGen, root );
            break;

        case EXTERN_LVAL_ID:
            result = generateExternLValueID( pCodeGen, root );
            break;

        case DECL_ID:
            result = generateDeclarationID( pCodeGen, root );
            break;

        case FUNC_ID:
            result = generateFuncID( pCodeGen, root );
            break;

        case INVOKATION_ID:
            result = generateInvocationID( pCodeGen, root );
            break;

        case PARAM_ID:
            break;

        case NUM:
            result = generateNum( pCodeGen, root );
            break;

        case FLOAT:
            result = generateFloat( pCodeGen, root );
            break;

        case CHARSTR:
            result = generateCharStr( pCodeGen, root );
            break;

        case CHARACTER:
            result = generateChar( pCodeGen, root );
            break;

        case FUNC_HDR:
            result = generateFuncHdr( pCodeGen, root );
            break;

        case FUNC_HDR1:
            result = generateFuncHdr1( pCodeGen, root );
            break;

        case STAT_LIST:
            result = generateStatementList( pCodeGen, root );
            break;

        case COMP_STAT:
            result = generateCompoundStatement( pCodeGen, root );
            break;

        case IF:
            result = generateIf( pCodeGen, root );
            break;

        case ELSE:
            result = generateElse( pCodeGen, root );
            break;

        case FOR:
            result = generateFor( pCodeGen, root );
            break;

        case FOR1:
            result = generateFor1( pCodeGen, root );
            break;

        case FOR2:
            result = generateFor2( pCodeGen, root );
            break;

        case WHILE:
            result = generateWhile( pCodeGen, root );
            break;

        case BREAK:
            result = generateBreak( pCodeGen, root );
            break;

        case RETURN:
            result = generateReturn( pCodeGen, root );
            break;

        case SWITCH:
            result = generateSwitch( pCodeGen, root );
            break;

        case CASE:
            result = generateCase( pCodeGen, root );
            break;

        case CASE1:
            result = generateCase1( pCodeGen, root );
            break;

        case DEFAULT:
            result = generateDefault( pCodeGen, root );
            break;

        case WRITE:
            result = generateWrite( pCodeGen, root );
            break;

        case OUTPUT_LIST:
            result = generateOutputList( pCodeGen, root );
            break;

        case APPEND_LIST:
            result = generateAppendList( pCodeGen, root );
            break;

        case WRITELN:
            result = generateWriteLn( pCodeGen, root );
            break;

        case READ:
            result = generateRead( pCodeGen, root );
            break;

        case INPUT_LIST:
            result = generateInputList( pCodeGen, root );
            break;

        case TO_FLOAT:
            result = generateToFloat( pCodeGen, root );
            break;

        case TO_INT:
            result = generateToInt( pCodeGen, root );
            break;

        case ASSIGN:
            result = generateAssign( pCodeGen, root );
            break;

        case APPEND:
            result = generateAppend( pCodeGen, root );
            break;

        case LENGTH:
            result = generateLength( pCodeGen, root );
            break;

        case CHARAT:
            result = generateCharAt( pCodeGen, root );
            break;

        case SETAT:
            result = generateSetAt( pCodeGen, root );
            break;

        case SETAT1:
            result = generateSetAt1( pCodeGen, root );
            break;

        case DELAY:
            result = generateDelay( pCodeGen, root );
            break;

        case WAITSIG:
            result = generateWaitSig( pCodeGen, root );
            break;

        case NOTIFY:
            result = generateNotify( pCodeGen, root );
            break;

        case HANDLE:
            result = generateHandle( pCodeGen, root );
            break;

        case VALIDATE_START:
            result = generateValidateStart( pCodeGen, root );
            break;

        case VALIDATE_END:
            result = generateValidateEnd( pCodeGen, root );
            break;

        case OPEN_PRINT_SESSION:
            result = generateOpenPrintSession( pCodeGen, root );
            break;

        case CLOSE_PRINT_SESSION:
            result = generateClosePrintSession( pCodeGen, root );
            break;

        case SYSTEM:
            result = generateSystem( pCodeGen, root );
            break;

        case FILE_OPEN:
            result = generateFileOpen( pCodeGen, root );
            break;

        case FILE_CLOSE:
            result = generateFileClose( pCodeGen, root );
            break;

        case FILE_READ:
            result = generateFileRead( pCodeGen, root );
            break;

        case FILE_WRITE:
            result = generateFileWrite( pCodeGen, root );
            break;

        case SETTIMER:
            result = generateSetTimer( pCodeGen, root );
            break;

        case CLEARTIMER:
            result = generateClearTimer( pCodeGen, root );
            break;

        case TIMES_EQUALS:
            result = generateTimesEquals( pCodeGen, root );
            break;

        case DIV_EQUALS:
            result = generateDivEquals( pCodeGen, root );
            break;

        case PLUS_EQUALS:
            result = generatePlusEquals( pCodeGen, root );
            break;

        case MINUS_EQUALS:
            result = generateMinusEquals( pCodeGen, root );
            break;

        case AND_EQUALS:
            result = generateAndEquals( pCodeGen, root );
            break;

        case OR_EQUALS:
            result = generateOrEquals( pCodeGen, root );
            break;

        case XOR_EQUALS:
            result = generateXorEquals( pCodeGen, root );
            break;

        case ARRAY:
            result = generateArray( pCodeGen, root );
            break;

        case ARRAY_DECL:
            result = generateArrayDeclaration( pCodeGen, root );
            break;

        case DECL_LIST:
            result = generateDeclarationList( pCodeGen, root );
            break;

        case EXTERN_DECL_LIST:
            result = generateExternDeclarationList( pCodeGen, root );
            break;

        case EXTERN_DECLN:
            result = generateExternDeclaration( pCodeGen, root );
            break;

        case DECLN:
            result = generateDeclaration( pCodeGen, root );
            break;

        case DECLN_LIST:
            result = generateExternDeclarationList( pCodeGen, root );
            break;

        case PROC_CALL:
            result = generateProcCall( pCodeGen, root );
            break;

        case ARG_LIST:
            result = generateArgList( pCodeGen, root );
            break;

        case PARAM_LIST:
            result = generateParameterList( pCodeGen, root );
            break;

        case PARAMETER:
            result = generateParameter( pCodeGen, root );
            break;

        case READLN:
        case TYPE_FLOAT:
        case TYPE_INT:
        case TYPE_BOOL:
        case TYPE_CHAR:
        case TYPE_STRING:
            result = -1;
            break;

        case OR:
            result = generateOr( pCodeGen, root );
            break;

        case AND:
            result = generateAnd( pCodeGen, root );
            break;

        case XOR:
            result = generateXor( pCodeGen, root );
            break;

        case BOR:
            result = generateBor( pCodeGen, root );
            break;

        case BAND:
            result = generateBand( pCodeGen, root );
            break;

        case BNOT:
            result = generateBnot( pCodeGen, root );
            break;

        case NOT:
            result = generateNot( pCodeGen, root );
            break;

        case EQUALS:
            result = generateEquals( pCodeGen,root );
            break;

        case NOTEQUALS:
            result = generateNotEquals( pCodeGen, root );
            break;

        case LTE:
            result = generateLessThanOrEqual( pCodeGen, root );
            break;

        case GTE:
            result = generateGreaterThanOrEqual( pCodeGen, root );
            break;

        case LT:
            result = generateLessThan( pCodeGen, root );
            break;

        case GT:
            result = generateGreaterThan( pCodeGen, root );
            break;

        case RSHIFT:
            result = generateRShift( pCodeGen, root );
            break;

        case LSHIFT:
            result = generateLShift( pCodeGen, root );
            break;

        case INC:
            result = generateIncrement( pCodeGen, root );
            break;

        case DEC:
            result = generateDecrement( pCodeGen, root );
            break;

        case PLUS:
            result = generatePlus( pCodeGen, root );
            break;

        case MINUS:
            result = generateMinus( pCodeGen, root );
            break;

        case TIMES:
            result = generateTimes( pCodeGen, root );
            break;

        case DIVIDE:
            result = generateDivide( pCodeGen, root );
            break;

        default:
            fprintf( stderr,
                     "GenerateCode - error, unknown node type: %d\n",
                     root->type);
            break;
    }

    return result;
}

/*============================================================================*/
/*  OutputSupportCode                                                         */
/*!
    Output the support assembly code

    The OutputSupportCode function appends the support assembly code to the
    output file specified by the output FILE *

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

==============================================================================*/
void OutputSupportCode( CodeGen *pCodeGen )

{
    char line[80];
    FILE *fp_out;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) )
    {
        fp = (FILE *)fopen( SUPPORTCODE, "r" );
        if( fp != NULL )
        {
            fp_out = pCodeGen->fp;
            fprintf(fp_out, "\n\n");

            while(!feof(fp))
            {
                memset( line, 0, sizeof(line) );
                fgets( line, 80, fp );
                fprintf(fp_out, "%s", line );
            }

            fclose(fp);
        }
    }
}

/*==============================================================================
        Private Function Definitions
==============================================================================*/

/*============================================================================*/
/*  generateProgram                                                           */
/*!
    Output the PROGRAM assembly code

    The generateProgram function generates assembly code for the
    PROGRAM node.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateProgram( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    FILE *fp;

    if( ( root != NULL ) &&
        ( pCodeGen != NULL ) )
    {
        fp = pCodeGen->fp;

        fprintf( fp, "\tMOV R0,_exit" );
        fprintf( fp, "\t;program startup code\n" );
        fprintf( fp, "\tPSH R0\t\t;program end\n" );
        fprintf( fp, "\tJMP _main" );
        fprintf( fp, "\t;jump to program start\n" );
        GenerateCode( pCodeGen, root->left );
        fprintf( fp, "\n_exit\tHLT\n" );
    }

    return result;
}

/*============================================================================*/
/*  generateFuncDefList                                                       */
/*!
    Output the function definition list assembly code

    The generateFuncDefList function generates assembly code for the
    FUNC_DEF_LIST node.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateFuncDefList( CodeGen *pCodeGen, struct Node *root )
{
    return generateChildren( pCodeGen, root );
}

/*============================================================================*/
/*  generateFuncID                                                            */
/*!
    Output the function id assembly code

    The generateFuncID function processes the FUNC_ID node, sets the
    function scope, and generates the function label in the output

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateFuncID( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    struct identEntry *idEntry;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) &&
        ( root->ident != NULL ) )
    {
        fp = pCodeGen->fp;

        /* set the scope level for this function */
        idEntry = root->ident;
        SetScopeLevel( idEntry->scopeID );
        if( idEntry->name != NULL )
        {
            fprintf( fp, "_%s\n", idEntry->name );
        }
    }

    return result;
}

/*============================================================================*/
/*  generateFuncDef                                                           */
/*!
    Output the function definition list assembly code

    The generateFuncDef function processes the FUNC_DEF node, processes
    the node's children and outputs the end of function assembly code.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateFuncDef( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        GenerateCode( pCodeGen, root->left );
        GenerateCode( pCodeGen, root->right );

        fprintf( fp, "\tMOV SP,R1" );
        fprintf( fp, "\t;free locals\n" );
        fprintf( fp, "\tRET" );
        fprintf( fp, "\t\t;return to caller\n" );
    }

    return result;
}

/*============================================================================*/
/*  generateFuncDef1                                                          */
/*!
    Output the function definition list assembly code

    The generateFuncDef1 function processes the FUNC_DEF1 node, generates the
    start of function assembly code, and Generates code for the node's
    children.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateFuncDef1( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        fprintf( fp, "\tMOV R1,SP" );
        fprintf( fp, "\t;stack frame pointer\n" );
        GenerateCode( pCodeGen, root->left );
        GenerateCode( pCodeGen, root->right );
    }

    return result;
}

/*============================================================================*/
/*  generateDeclarationID                                                     */
/*!
    Output the variable declaration assembly code

    The generateDeclarationID function processes the DECL_ID node and
    generates the assembly code for the variable declaration.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateDeclarationID( CodeGen *pCodeGen, struct Node *root )
{
    int n;
    int h;
    struct identEntry *idEntry;    /* access to identifiers */
    char label[7];
    char label1[7];            /* label generation */
    FILE *fp;
    int result = -1;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        idEntry = root->ident;
        if( idEntry != NULL )
        {
            /* allocate a new working register */
            n = AllocReg( idEntry, 0 );
            if( idEntry->isExternal == true )
            {
                /* generate a label */
                sprintf( (char *)label, "_SV%d", GetLabelNumber() );
                fprintf( fp, "\tJMP %s\n", label );
                sprintf( (char *)label1, "_sv%d", GetLabelNumber() );

                /* generate the variable name */
                fprintf( fp, "%s\n\tDAT \"%s\"\n", label1,idEntry->name );
                fprintf( fp, "%s\n\tMOV R%d,%s\n", label, n, label1 );
                fprintf( fp, "\tEXT R%d\n", n );
                fprintf( fp, "\tSUB SP,%d\n", idEntry->size );
                fprintf( fp, "\tMOV R2,SP\n" );
                fprintf( fp, "\tSTR R2,R%d", n );
                fprintf( fp, "\t;extern handle: %s\n", idEntry->name );

                /* store the string buffer ID on the stack */
                h = n;
                n = AllocReg( idEntry, 1 );
                fprintf( fp, "\tSUB SP,%ld\n", sizeof( uint32_t) );
                fprintf( fp, "\tMOV R2,SP\n" );

                switch( idEntry->type )
                {
                    case TYPE_FLOAT:
                        /* get the floating point value
                        from the external variable */
                        fprintf( fp, "\tGET.F R%d,R%d\n", n, h );
                        break;

                    case TYPE_STRING:
                        /* get the string buffer ID */
                        fprintf( fp,
                                "\tMOV R%d, %d\n",
                                n,
                                idEntry->stringBufID );
                        /* and create the string buffer */
                        fprintf( fp, "\tCSB R%d", n );
                        fprintf( fp, "\t\t;create new string buffer\n" );
                        /* load the initial content of the string buffer */
                        fprintf( fp, "\tGET.S R%d,R%d\n", n, h );
                        break;

                    default:
                        /* get the value from the external variable */
                        fprintf( fp, "\tGET R%d,R%d\n", n, h );
                        break;
                }

                /* and put the result on the stack */
                fprintf( fp, "\tSTR R2,R%d", n );
                fprintf( fp, "\t;extern value: %s\n", idEntry->name );
                result = n;
            }
            else
            {
                fprintf( fp, "\tSUB SP,%d\n", idEntry->size );
                fprintf( fp, "\tMOV R2,SP" );
                fprintf( fp, "\t;decl: %s\n", idEntry->name );
                if( idEntry->type == TYPE_STRING )
                {
                    /* store the string buffer ID on the stack */
                    fprintf( fp, "\tMOV R%d, %d\n", n, idEntry->stringBufID );
                    fprintf( fp, "\tSTR R2, R%d\n", n );
                    /* and create the string buffer */
                    fprintf( fp, "\tCSB R%d", n );
                    fprintf( fp, "\t\t;create new string buffer\n" );
                }
            }
        }
    }

    return result;
}

/*============================================================================*/
/*  generateLValueID                                                          */
/*!
    Output the assembly code for an LVALUE

    The generateLValueID function processes the LVAL_ID node and
    generates the assembly code for the variable declaration.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateLValueID( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    struct identEntry *idEntry;
    int n;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        idEntry = root->ident;
        if( idEntry != NULL )
        {
            n = AllocReg( idEntry, 0 );
            fprintf( fp, "\tMOV R2,R1\n" );
            fprintf( fp, "\tADD R2,%d", idEntry->offset );
            fprintf( fp, "\t;l-value: %s\n", idEntry->name );
            fprintf( fp, "\tLOD R%d,R2\n", n );
            result = n;
        }
    }

    return result;
}

/*============================================================================*/
/*  generateExternLValueID                                                    */
/*!
    Output the assembly code for an LVALUE

    The generateExternLValueID function processes the EXTERN_LVAL_ID node and
    generates the assembly code for the external LVALUE.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateExternLValueID( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    struct identEntry *idEntry;
    int n;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        idEntry = root->ident;
        if( idEntry != NULL )
        {
            n = AllocReg( root->ident, 0 );
            fprintf( fp, "\tMOV R2,R1\n" );
            fprintf( fp, "\tADD R2,%d\n", idEntry->offset );
            fprintf( fp, "\tLOD R%d,R2", n );
            fprintf( fp, "\t;external l-value: %s\n", idEntry->name );

            result = n;
        }
    }

    return result;
}

/*============================================================================*/
/*  generateInvocationID                                                      */
/*!
    Output the assembly code for a function/procedure call

    The generateInvocationID function processes the INVOKATION_ID node and
    generates the assembly code for the function call.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateInvocationID( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    struct identEntry *idEntry;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        idEntry = root->ident;
        if( idEntry != NULL )
        {
            fprintf( fp, "\tPSH R0" );
            fprintf( fp, "\t\t;procedure invokation\n" );
            fprintf( fp, "\tPSH R1\n" );
            fprintf( fp, "\tCAL _%s\n", idEntry->name );
            fprintf( fp, "\tPOP R1\n" );
            fprintf( fp, "\tPOP R2\n" );
            fprintf( fp, "\tMOV SP,R2\n" );
        }
    }

    return result;
}

/*============================================================================*/
/*  generateID                                                                */
/*!
    Output the assembly code for a local variable

    The generateID function processes the ID node and
    generates the assembly code for the local variable reference

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number for identifier reference
    @retval -1

==============================================================================*/
static int generateID( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    struct identEntry *idEntry;
    int n;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        idEntry = root->ident;
        if( idEntry != NULL )
        {
            n = AllocReg( idEntry, 0 );
            if( idEntry->constant == true )
            {
                fprintf( fp, "\tMOV R%d,%d", n, idEntry->value );
                fprintf( fp, "\t;constant\n");
            }
            else
            {
                fprintf( fp, "\tMOV R2,R1\n" );
                fprintf( fp, "\tADD R2,%d\n", idEntry->offset );
                fprintf( fp, "\tLOD R%d,R2", n );
                fprintf( fp, "\t;id: %s\n", idEntry->name );

                if( idEntry->isExternal == true )
                {
                    n = GetExternal( pCodeGen,
                                     idEntry,
                                     n,
                                     "retrieve external variable" );
                }
            }

            result = n;
        }
        else
        {
            fprintf(stderr, "E: unknown identifier at line: %d\n", getlineno());
        }
    }

    return result;
}

/*============================================================================*/
/*  generateNum                                                               */
/*!
    Output the assembly code for a constant number

    The generateNum function processes the NUM node and
    generates the assembly code for the constant number

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number for numeric constant
    @retval -1 invalid inputs

==============================================================================*/
static int generateNum( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int n;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        n = AllocReg( NULL, 0 );
        fprintf( fp, "\tMOV R%d,%d", n, root->value );
        fprintf( fp, "\t;constant\n" );
        result = n;
    }

    return result;
}

/*============================================================================*/
/*  generateFloat                                                             */
/*!
    Output the assembly code for a constant floating point number

    The generateFloat function processes the FLOAT node and
    generates the assembly code for the constant floating point number

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number for floating point register
    @retval -1 invalid inputs

==============================================================================*/
static int generateFloat( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int n;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        n = AllocReg( NULL, 0 );
        fprintf( fp, "\tMOV.F R%d,%e", n, root->fvalue );
        fprintf( fp, "\t;constant float\n" );
        result = n;
    }

    return result;
}

/*============================================================================*/
/*  generateCharStr                                                           */
/*!
    Output the assembly code for a constant character string

    The generateCharStr function processes the CHARSTR node and
    generates the assembly code for the constant character string

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number for character string reference
    @retval -1 invalid inputs or error

==============================================================================*/
static int generateCharStr( CodeGen *pCodeGen, struct Node *root )
{
    char label[7];
    char label1[7];            /* label generation */
    int result = -1;
    struct identEntry *idEntry;
    int n;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        idEntry = root->ident;
        if( idEntry != NULL )
        {
            n = AllocReg( NULL, 0 );
            sprintf( (char *)label, "_STR%d", GetLabelNumber() );
            fprintf( fp, "\tJMP %s\n", label );
            sprintf( (char *)label1, "_txt%d", GetLabelNumber() );

            fprintf( fp, "%s\n\tDAT %s\n", label1, idEntry->name );
            fprintf( fp, "%s\n\tMOV R%d,%s\n", label, n, label1 );

            result = n;
        }
        else
        {
            printf("literal string error at line: %d\n", getlineno());
        }
    }

    return result;
}

/*============================================================================*/
/*  generateChar                                                              */
/*!
    Output the assembly code for a constant character

    The generateChar function processes the CHARACTER node and
    generates the assembly code for the constant character

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number for the character reference
    @retval -1 invalid inputs or error

==============================================================================*/
static int generateChar( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    struct identEntry *idEntry;
    int n;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        idEntry = root->ident;
        if( idEntry != NULL )
        {
            n = AllocReg( NULL, 0 );
            fprintf( fp, "\tMOV R%d,%s", n, idEntry->name );
            fprintf( fp, "\t;character literal\n" );
            result = n;
        }
        else
        {
            printf( "invalid identifier at line: %d\n", getlineno() );
        }
    }

    return result;
}

/*============================================================================*/
/*  generateFuncHdr                                                           */
/*!
    Process function header

    The generateFuncHdr function processes the FUNC_HDR node and
    generates the assembly code for the function header via the
    generateChildren function

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval result of generateChildren function

==============================================================================*/
static int generateFuncHdr( CodeGen *pCodeGen, struct Node *root )
{
    return generateChildren( pCodeGen, root );
}

/*============================================================================*/
/*  generateFuncHdr1                                                          */
/*!
    Process function header

    The generateFuncHdr1 function processes the FUNC_HDR1 node and
    generates the assembly code for the function header via the
    generateChildren function

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval result of generateChildren function

==============================================================================*/
static int generateFuncHdr1( CodeGen *pCodeGen, struct Node *root )
{
    return generateChildren( pCodeGen, root );
}

/*============================================================================*/
/*  generateStatementList                                                     */
/*!
    Generate assembly code for the statement list

    The generateStatementList function processes the STAT_LIST node and
    generates the assembly code for the statement list via the
    generateChildren function

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval result of generateChildren function

==============================================================================*/
static int generateStatementList( CodeGen *pCodeGen, struct Node *root )
{
    return generateChildren( pCodeGen, root );
}

/*============================================================================*/
/*  generateCompoundStatement                                                 */
/*!
    Generate assembly code for the compound statement

    The generateCompoundStatement function processes the COMP_STAT node and
    generates the assembly code for the compound statement via the
    generateChildren function

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval result of generateChildren function

==============================================================================*/
static int generateCompoundStatement( CodeGen *pCodeGen, struct Node *root )
{
    return generateChildren( pCodeGen, root );
}

/*============================================================================*/
/*  generateIf                                                                */
/*!
    Generate assembly code for the if statement

    The generateIf function processes the IF node and
    generates the assembly code for the if statement

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateIf( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        ifLevel++;

        sprintf( (char *)startELSE[ifLevel], "_IF%d", GetLabelNumber() );
        sprintf( (char *)endELSE[ifLevel], "_IF%d", GetLabelNumber() );

        a = GenerateCode( pCodeGen, root->left );

        fprintf( fp, "\tCMP R%d,0\n", a );
        fprintf( fp, "\tJZR %s\n", startELSE[ifLevel] );

        GenerateCode( pCodeGen, root->right );

        ifLevel--;
    }

    return result;
}

/*============================================================================*/
/*  generateElse                                                              */
/*!
    Generate assembly code for the else statement

    The generateElse function processes the ELSE node and
    generates the assembly code for the else statement

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateElse( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        GenerateCode( pCodeGen, root->left );

        fprintf( fp, "\tJMP %s\n", endELSE[ifLevel] );
        fprintf( fp, "%s\n", startELSE[ifLevel] );

        GenerateCode( pCodeGen, root->right );

        fprintf( fp, "%s\n", endELSE[ifLevel] );
    }

    return result;
}

/*============================================================================*/
/*  generateFor                                                               */
/*!
    Generate assembly code for the for statement

    The generateFor function processes the FOR node and
    generates the assembly code for the for statement.  It constructs
    the FOR labels and calls GenerateCode on the children nodes to
    generate the assembly language for the for statement,

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateFor( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        forLevel++;

        sprintf( (char *)startFOR[forLevel], "_F%d", GetLabelNumber() );
        sprintf( (char *)endFOR[forLevel], "_F%d", GetLabelNumber());

        breakType = breakTypes[++breakLevel] = eBreakTypeFor;

        GenerateCode( pCodeGen, root->left );
        GenerateCode( pCodeGen, root->right );

        forLevel--;
    }

    return result;
}

/*============================================================================*/
/*  generateFor1                                                              */
/*!
    Generate assembly code for the for statement

    The generateFor1 function processes the FOR1 node and
    generates the assembly code for the for statement.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateFor1( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        fprintf( fp, "%s\n", startFOR[forLevel] );
        a = GenerateCode( pCodeGen, root->left );

        fprintf( fp, "\tCMP R%d,0\n", a );
        fprintf( fp, "\tJZR %s\n", endFOR[forLevel] );

        GenerateCode( pCodeGen, root->right );
    }

    return result;
}

/*============================================================================*/
/*  generateFor2                                                              */
/*!
    Generate assembly code for the for statement

    The generateFor2 function processes the FOR2 node and
    generates the assembly code for the for statement.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateFor2( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        GenerateCode( pCodeGen, root->left );
        GenerateCode( pCodeGen, root->right );

        fprintf( fp, "\tJMP %s\n", startFOR[forLevel] );
        fprintf( fp, "%s\n", endFOR[forLevel] );

        /* restore the previous breakType */
        breakType = breakTypes[--breakLevel];

    }

    return result;
}

/*============================================================================*/
/*  generateWhile                                                             */
/*!
    Generate assembly code for the while statement

    The generateWhile function processes the WHILE node and
    generates the assembly code for the while statement.
    It constructs the labels for the WHILE loop, generates the assembly
    for the logic inside the while loop by calling GenerateCode on
    the child nodes, and then generates the assembly
    language for jumping back to the top of the while loop.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateWhile( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int b;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        whileLevel++;

        breakType = breakTypes[++breakLevel] = eBreakTypeWhile;

        sprintf( (char *)startWHILE[whileLevel], "_W%d", GetLabelNumber() );
        sprintf( (char *)endWHILE[whileLevel], "_W%d", GetLabelNumber() );

        fprintf( fp, "%s\n", startWHILE[whileLevel] );

        GenerateCode( pCodeGen, root->left );
        b = GenerateCode( pCodeGen, root->right );

        fprintf( fp, "\tCMP R%d, 0\n", b );
        fprintf( fp, "\tJNZ %s\n", startWHILE[whileLevel] );
        fprintf( fp, "%s\n", endWHILE[whileLevel] );

        /* restore the previous breakType */
        breakType = breakTypes[--breakLevel];
    }

    return result;
}

/*============================================================================*/
/*  generateBreak                                                             */
/*!
    Generate assembly code for the break statement

    The generateBreak function processes the BREAK node and
    generates the assembly code to break out of the containing
    for, while, or switch statement.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateBreak( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        switch(breakType)
        {
            case eBreakTypeWhile:
                fprintf( fp, "\tJMP %s\t;break\n", endWHILE[whileLevel] );
                break;

            case eBreakTypeFor:
                fprintf( fp, "\tJMP %s\t;break\n", endFOR[forLevel] );
                break;

            case eBreakTypeSwitch:
                fprintf( fp, "\tJMP %s\t;break\n", endSWITCH[switchLevel] );
                break;

            default:
                fprintf( fp, "error undefined break type\n" );
                break;
        }
    }

    return result;
}

/*============================================================================*/
/*  generateReturn                                                            */
/*!
    Generate assembly code for the return statement

    The generateBreak function processes the RETURN node and
    generates the assembly code to clean up the stack, collect the
    function result,  and return from the current function.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateReturn( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        fprintf( fp, "\tMOV R0,R%d",a );
        fprintf( fp, "\t;return result\n" );
        fprintf( fp, "\tMOV SP,R1" );
        fprintf( fp, "\t;free locals\n" );
        fprintf( fp, "\tRET" );
        fprintf( fp, "\t\t;return to caller\n" );
    }

    return result;
}

/*============================================================================*/
/*  generateSwitch                                                            */
/*!
    Generate assembly code for the switch statement

    The generateSwitch function processes the SWITCH node and
    generates the switch/case labels.  and generates the assembly
    for the switch/case statement by invoking GenerateCode on the child
    nodes.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateSwitch( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        switchLevel++;

        foundDefault[switchLevel] = false;

        sprintf( (char *)startSWITCH[switchLevel], "_SW%d", GetLabelNumber() );
        sprintf( (char *)endSWITCH[switchLevel], "_SW%d", GetLabelNumber() );
        sprintf( (char *)dfltSWITCH[switchLevel], "_DFLT%d", GetLabelNumber());

        breakType = breakTypes[++breakLevel] = eBreakTypeSwitch;

        fprintf( fp,"%s\n", startSWITCH[switchLevel] );

        a = GenerateCode( pCodeGen, root->left );

        regSwitch[switchLevel] = a;

        GenerateCode( pCodeGen, root->right );

        /* print the case label for the n+1th case
           which does not really exist */
        fprintf( fp, "_CASE%d\n", caseLabelIndex++ );

        if( foundDefault[switchLevel] == false )
        {
            fprintf( stderr,
                     "ERROR: default state for switch/case is required\n" );
        }
        else
        {
            fprintf( fp,
                     "\tJMP %s\t; jump to default case\n",
                     dfltSWITCH[switchLevel] );
        }

        fprintf( fp, "%s\n", endSWITCH[switchLevel] );

        /* restore the previous breakType */
        breakType = breakTypes[--breakLevel];

        switchLevel--;
    }

    return result;
}

/*============================================================================*/
/*  generateCase                                                              */
/*!
    Generate assembly code for the case statement

    The generateCase function processes the CASE node and
    outputs the case label, and generates the assembly for the case
    comparison.  It generates the logic for the case by calling
    GenerateCode on the left child node.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateCase( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        fprintf( fp, "_CASE%d\n", caseLabelIndex++ );

        a = GenerateCode( pCodeGen, root->left );

        fprintf( fp, "\tCMP R%d, R%d\n", a, regSwitch[switchLevel] );
        fprintf( fp, "\tJNZ _CASE%d\n", caseLabelIndex );

        GenerateCode( pCodeGen, root->right );
    }

    return result;
}

/*============================================================================*/
/*  generateCase1                                                             */
/*!
    Generate assembly code for the case statement

    The generateCase1 function processes the CASE1 node and
    handles output of the case assembly by calling generateChildren to
    process the child nodes.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval result of generateChildren

==============================================================================*/
static int generateCase1( CodeGen *pCodeGen, struct Node *root )
{
    return generateChildren( pCodeGen, root );
}

/*============================================================================*/
/*  generateDefault                                                           */
/*!
    Generate assembly code for the default statement

    The generateDefault function processes the DEFAULT node and
    handles output of the default case assembly.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval result of generateChildren

==============================================================================*/
static int generateDefault( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        foundDefault[switchLevel] = true;

        fprintf( fp, "%s\n", dfltSWITCH[switchLevel] );

        GenerateCode( pCodeGen, root->left );

        fprintf( fp, "\tJMP %s\n", endSWITCH[switchLevel] );

        GenerateCode( pCodeGen, root->right );
    }

    return result;
}

/*============================================================================*/
/*  generateWrite                                                             */
/*!
    Generate assembly code for the write statement

    The generateWrite function processes the WRITE node and
    generates the assembly code to set the output file descriptor.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval result of generateLeftChild

==============================================================================*/
static int generateWrite( CodeGen *pCodeGen, struct Node *root )
{
    int n;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( root != NULL ) &&
        ( pCodeGen->fp != NULL ) )
    {
        fp = pCodeGen->fp;
        n = AllocReg( NULL, 0 );
        fprintf( fp, "\tMOV R%d,2\n", n );
        fprintf( fp, "\tSFD R%d\n", n );
    }

    return generateLeftChild( pCodeGen, root );
}

/*============================================================================*/
/*  generateOutputList                                                        */
/*!
    Generate assembly code for an output list

    The generateOutputList function processes the OUTPUT_LIST node and
    generates the assembly code to output a string, character, number,
    or float value
    set the output file descriptor.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval result of generateLeftChild

==============================================================================*/
static int generateOutputList( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    FILE *fp;
    struct identEntry *idEntry;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        if( root->left != NULL )
        {
            a = GenerateCode( pCodeGen, root->left );
            switch(root->left->type)
            {
                case CHARSTR:
                    fprintf( fp, "\tWRS R%d", a );
                    fprintf( fp, "\t\t;output string\n" );
                    break;

                case CHARACTER:
                    fprintf( fp, "\tWRC R%d", a );
                    fprintf( fp, "\t\t;output character\n" );
                    break;

                case NUM:
                    fprintf( fp, "\tWRN R%d", a );
                    fprintf( fp, "\t\t;output integer\n" );
                    break;

                case FLOAT:
                    fprintf( fp, "\tWRF R%d", a );
                    fprintf( fp, "\t\t;output float\n" );
                    break;

                case ID:
                    idEntry = root->left->ident;
                    result = generateOutputID( pCodeGen, idEntry, a );
                    break;

                default:
                    printf( "undefined type at line %d\n",
                            getlineno() );
                    break;
            }
        }

        GenerateCode( pCodeGen, root-> right );
    }

    return result;
}

/*============================================================================*/
/*  generateWriteLn                                                           */
/*!
    Generate assembly code for a writeln statement

    The generateWriteLn function processes the WRITELN node and
    generates the assembly code to output a newline to the output
    file descriptor.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateWriteLn( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    FILE *fp;
    int n;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        n = AllocReg( NULL, 0 );
        fprintf( fp, "\tMOV R%d,2\n", n );
        fprintf( fp, "\tSFD R%d\n", n );
        fprintf( fp, "\tWRC '\\n'" );
        fprintf( fp, "\t;newline\n" );
    }

    return result;
}

/*============================================================================*/
/*  generateAppendList                                                        */
/*!
    Generate assembly code for an append list

    The generateAppendList function processes the APPEND_LIST node and
    generates the assembly code for the append list.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateAppendList( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        a = GenerateCode( pCodeGen, root->left );

        Append( append_reg, a, root->left, pCodeGen );

        GenerateCode( pCodeGen, root->right );
    }

    return result;
}

/*============================================================================*/
/*  generateAppend                                                            */
/*!
    Generate assembly code for an append operation

    The generateAppendList function processes the APPEND node and
    generates the assembly code for the append operation.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateAppend( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    bool external;
    struct identEntry *idEntry;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        external = isExternal( root->left );

        idEntry = GetIdentEntry( root->left );

        append_reg = GenerateCode( pCodeGen, root->left );

        GenerateCode( pCodeGen, root->right );

        if( external == true )
        {
            SetExternal( pCodeGen,
                         idEntry,
                         append_reg,
                         "store append result" );
        }

        TypeCheck( root, 0, false );
    }

    return result;
}

/*============================================================================*/
/*  generateLength                                                            */
/*!
    Generate assembly code for a length operation

    The generateLength function processes the LENGTH node and
    generates the assembly code for the length operation.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateLength( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );

        b = AllocReg( NULL, 0 );

        fprintf( fp, "\tSBL R%d,R%d", b, a );
        fprintf( fp, "\t; get length of string buffer\n" );

        result = b;
    }

    return result;
}

/*============================================================================*/
/*  generateCharAt                                                            */
/*!
    Generate assembly code for a character index operation

    The generateCharAt function processes the CHARAT node and
    generates the assembly code for the character index operation.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval reference the register containing the retrieved character

==============================================================================*/
static int generateCharAt( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;

    int a;
    int b;
    int c;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        fprintf( fp, ";generateCharAt\n");
        a = GenerateCode( pCodeGen, root->left );
        b = GenerateCode( pCodeGen, root->right );

        c = AllocReg( NULL, 0 );
        fprintf( fp, "\tSBO R%d,R%d", a, b );
        fprintf( fp, "\t; set offset in string buffer\n" );
        fprintf( fp, "\tGCO R%d,R%d", c, a );
        fprintf( fp, "\t; Get character from string at offset\n" );
        result = c;
    }

    return result;
}

/*============================================================================*/
/*  generateSetAt                                                            */
/*!
    Generate assembly code for setting a character at a string index

    The generateSetAt function processes the SETAT node and
    generates the assembly code for setting the character at a specified
    string index offset.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval reference the register containing the stored character

==============================================================================*/
static int generateSetAt( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        fprintf( fp, ";generateSetAt\n");

        /* set up string buffer offset and return string buffer
           handle in register a */
        a = GenerateCode( pCodeGen, root->left );

        /* get character to store in register b */
        b = GenerateCode( pCodeGen, root->right );

        /* store the character */
        fprintf( fp, "\tSCO R%d,R%d", a, b );
        fprintf( fp, "\t; Store character in string at offset\n" );

        /* return the register that contains the character stored */
        result = b;
    }

    return result;
}

/*============================================================================*/
/*  generateSetAt1                                                            */
/*!
    Generate assembly code for setting a character at a string index

    The generateSetAt function processes the SETAT1 node and
    generates the assembly code for setting the character string
    buffer offset.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval reference the register containing the string buffer handle

==============================================================================*/
static int generateSetAt1( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        fprintf( fp, ";generateSetAt1\n");
        a = GenerateCode( pCodeGen, root->left );
        b = GenerateCode( pCodeGen, root->right );

        fprintf( fp, "\tSBO R%d,R%d", a, b );
        fprintf( fp, "\t; set offset in string buffer\n" );

        /* return register containing string buffer handle */
        result = a;
    }

    return result;
}

/*============================================================================*/
/*  generateRead                                                              */
/*!
    Generate assembly code for read operation

    The generateRead function processes the READ node and
    generates the assembly code for setting the read file descriptor

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval result of generateLeftChild

==============================================================================*/
static int generateRead( CodeGen *pCodeGen, struct Node *root )
{
    int n;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( root != NULL ) &&
        ( pCodeGen->fp != NULL ) )
    {
        fp = pCodeGen->fp;
        n = AllocReg( NULL, 0 );
        fprintf( fp, "\tMOV R%d,2\n", n );
        fprintf( fp, "\tSFD R%d\n", n );
    }

    return generateLeftChild( pCodeGen, root );
}

/*============================================================================*/
/*  generateOpenPrintSession                                                  */
/*!
    Generate assembly code for opening a print session

    The generateOpenPrintSession function processes the OPEN_PRINT_SESSION node
    and generates the assembly code for opening a print session

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval result of GenerateCode on left child

==============================================================================*/
static int generateOpenPrintSession( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    FILE *fp;
    struct identEntry *idEntry2;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        idEntry2 = GetIdentEntry( root->right );
        if( idEntry2 != NULL )
        {
            /* allocate a register for the result */
            b = AllocReg( idEntry2, 0 );
            fprintf( fp, "\tOPS R%d,R%d", a, b );
            fprintf( fp, "\t; open the print session\n");

            fprintf( fp, "\tMOV R2,R1\n" );
            fprintf( fp, "\tADD R2,%d\n", idEntry2->offset );
            fprintf( fp, "\tSTR R2,R%d", b );
            fprintf( fp, "\t;variable handle\n" );
        }

        result = a;
    }

    return result;
}

/*============================================================================*/
/*  generateClosePrintSession                                                 */
/*!
    Generate assembly code for closing a print session

    The generateClosePrintSession function processes the CLOSE_PRINT_SESSION
    node and generates the assembly code for closing a print session

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval result of GenerateCode on left child

==============================================================================*/
static int generateClosePrintSession( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        b = GenerateCode( pCodeGen, root->right );

        /* allocate a register for the result */
        fprintf( fp, "\tCPS R%d,R%d", a, b );
        fprintf( fp, "\t; close the print session\n");

        result = a;
    }

    return result;
}

/*============================================================================*/
/*  generateSystem                                                            */
/*!
    Generate assembly code for a system call

    The generateSystem function processes the SYSTEM node and generates
    the assembly code for making a system call

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number of register containing the result code

==============================================================================*/
static int generateSystem( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int n = -1;
    int r;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) &&
        ( root->left != NULL ))
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );

        if( root->left->type == CHARSTR )
        {
            /* create a string buffer to store the system string */
            n = AllocReg( NULL, 0 );
            fprintf( fp, "\tCSB R%d", n );
            fprintf( fp, "\t; string buffer to store system() string\n");
            fprintf( fp, "\tASS R%d,R%d\n", n, a );
            a = n;
        }

        /* allocate a register for the result */
        r = AllocReg( NULL, 0 );
        fprintf( fp, "\tEXE R%d,R%d", r, a );
        fprintf( fp, "\t; execute the string buffer\n");

        result = r;
    }

    return result;
}

/*============================================================================*/
/*  generateFileOpen                                                          */
/*!
    Generate assembly code for a file open call

    The generateFileOpen function processes the FILE_OPEN node and generates
    the assembly code for opening a file

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number of register referencing the open file descriptor

==============================================================================*/
static int generateFileOpen( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    int n;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) &&
        ( root->left != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );

        if( root->left->type == CHARSTR )
        {
            /* create a string buffer to store the system string */
            n = AllocReg( NULL, 0 );
            fprintf( fp, "\tCSB R%d", n );
            fprintf( fp, "\t; string buffer to store the file name\n");
            fprintf( fp, "\tASS R%d,R%d\n", n, a );
            a = n;
        }

        b = GenerateCode( pCodeGen, root->right );
        fprintf( fp, "\tOFD R%d,R%d", a, b);
        fprintf( fp, "\t; open file\n");
        result = a;
    }

    return result;
}

/*============================================================================*/
/*  generateFileClose                                                         */
/*!
    Generate assembly code for a file close call

    The generateFileClose function processes the FILE_CLOSE node and generates
    the assembly code for closing a file

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateFileClose( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        fprintf( fp, "\tCFD R%d", a );
        fprintf( fp, "\t; close file\n");
        result = -1;
    }

    return result;
}

/*============================================================================*/
/*  generateFileRead                                                          */
/*!
    Generate assembly code for a file read call

    The generateFileRead function processes the FILE_READ node and generates
    the assembly code for setting up the file descriptor to read from a file

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateFileRead( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        fprintf( fp, "\tSFD R%d", a );
        fprintf( fp, "\t\t; select input file descriptor\n" );
        GenerateCode( pCodeGen, root->right );
        result = -1;
    }

    return result;
}

/*============================================================================*/
/*  generateFileWrite                                                         */
/*!
    Generate assembly code for a file write call

    The generateFileWrite function processes the FILE_WRITE node and generates
    the assembly code for setting up the file descriptor to write to a file

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateFileWrite( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        fprintf( fp, "\tSFD R%d", a );
        fprintf( fp, "\t\t; select output file descriptor\n" );
        GenerateCode( pCodeGen, root->right );
        result = -1;
    }

    return result;
}

/*============================================================================*/
/*  generateAssign                                                            */
/*!
    Generate assembly code for a variable assignment

    The generateAssign function processes the ASSIGN node and generates
    the assembly code for setting up the file descriptor to write to a file

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number of target register

==============================================================================*/
int generateAssign( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    struct identEntry *idEntry;
    int a;
    int b;
    int n;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        b = GenerateCode( pCodeGen, root->right );
        a = GenerateCode( pCodeGen, root->left );

        if( root->left != NULL )
        {
            idEntry = root->left->ident;
        }

        if (a==-1)
        {
            a = b;
            if( idEntry != NULL )
            {
                idEntry->reg[0] = a;
            }
        }

        if( idEntry == NULL )
        {
            fprintf( fp, "\tSTR R2,R%d", b );
            fprintf( fp, "\t;assignment\n" );
        }
        else if ( ( idEntry->stringBufID != 0 ) &&
                  ( idEntry->isExternal == true ) )
        {
            n = AllocReg( NULL, 0 );
            fprintf( fp, "\tADD R2,-%d\n", idEntry->size );
            fprintf( fp, "\tLOD R%d,R2", n );
            fprintf( fp, "\t;string buffer for: %s\n", idEntry->name );

            fprintf( fp, "\tZSB R%d", n);
            fprintf( fp, "\t\t;string buffer assignment\n" );
            Append( n, b, root->right, pCodeGen );

            SetExternal( pCodeGen, idEntry, n, "; set external string buffer" );
            FreeReg( n );
        }
        else if (idEntry->stringBufID != 0 )
        {
            fprintf( fp, "\tZSB R%d", a);
            fprintf( fp, "\t;string buffer assignment\n" );
            Append( a, b, root->right, pCodeGen );
        }
        else if( idEntry->isExternal == true )
        {
            SetExternal( pCodeGen,
                         idEntry,
                         b,
                         "external assignment" );
        }
        else
        {
            /* store the result on the stack */
            fprintf( fp,"\tSTR R2,R%d",b );
            fprintf( fp,"\t;assignment\n" );
        }

        result = a;
    }

    return result;
}

/*============================================================================*/
/*  generateArrayDeclaration                                                  */
/*!
    Generate assembly code for a variable assignment

    The generateArrayDeclaration function processes the ARRAY_DECL node
    and generates the assembly code for setting up an array declaration
    via generateLeftChild

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval result of generateLeftChild

==============================================================================*/
static int generateArrayDeclaration( CodeGen *pCodeGen, struct Node *root )
{
    return generateLeftChild( pCodeGen, root );
}

/*============================================================================*/
/*  Append                                                                    */
/*!
    Generate assembly code for an Append operation

    The Append function generates the code for an append operation

    @param[in]
        dst
            target register number

    @param[in]
        src
            source register number

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

==============================================================================*/
static void Append( int dst, int src, struct Node *root, CodeGen *pCodeGen )
{
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        switch(root->type)
        {
            case CHARSTR:
                fprintf( fp, "\tASS R%d,R%d", dst, src );
                fprintf( fp, "\t;append string literal\n" );
                break;

            case CHARACTER:
                fprintf( fp, "\tASC R%d,R%d", dst, src );
                fprintf( fp, "\t;append integer literal\n" );
                break;

            case NUM:
                fprintf( fp, "\tASN R%d,R%d", dst, src );
                fprintf( fp, "\t;append integer literal\n" );
                break;

            case FLOAT:
                fprintf( fp, "\tASF R%d,R%d", dst, src );
                fprintf( fp, "\t;append float literal\n" );
                break;

            case ID:
                switch(root->ident->type)
                {
                    case TYPE_CHAR:
                        fprintf( fp, "\tASC R%d,R%d", dst, src );
                        fprintf( fp, "\t;append integer\n" );
                        break;

                    case TYPE_INT:
                        fprintf( fp, "\tASN R%d,R%d", dst, src );
                        fprintf( fp, "\t;append integer\n" );
                        break;

                    case TYPE_FLOAT:
                        fprintf( fp, "\tASF R%d,R%d", dst, src );
                        fprintf( fp, "\t;append float\n" );
                        break;

                    case TYPE_STRING:
                        fprintf( fp, "\tASB R%d,R%d", dst, src );
                        fprintf( fp, "\t;append string buffer\n" );
                        break;
                }

            default:
                break;
        }

        FreeTempReg( src );
    }
}

/*============================================================================*/
/*  generateOutputID                                                          */
/*!
    Generate assembly code for an output identifier

    The generateOutputID function processes the ID node of an OUTPUT_LIST
    and generates the assembly code for writing an output.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        ident
            pointer to an output identifier

    @param[in]
        a
            register number containing the item to output

    @retval -1

==============================================================================*/
static int generateOutputID( CodeGen *pCodeGen,
                             struct identEntry *ident,
                             int a )
{
    int result = -1;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( ident != NULL ) )
    {
        fp = pCodeGen->fp;

        switch( ident->type )
        {
            case TYPE_CHAR:
                fprintf( fp, "\tWRC R%d", a );
                fprintf( fp, "\t\t;output character!!\n" );
                break;

            case TYPE_INT:
                fprintf( fp, "\tWRN R%d", a );
                fprintf( fp, "\t\t;output integer\n" );
                break;

            case TYPE_STRING:
                fprintf( fp, "\tWSB R%d", a );
                fprintf( fp, "\t\t;output string\n" );
                break;

            case TYPE_FLOAT:
                fprintf( fp, "\tWRF R%d", a );
                fprintf( fp, "\t\t;output float\n" );
                break;

            default:
                printf( "undefined type at line %d\n",
                        getlineno() );
                break;
        }
    }

    return result;
}

/*============================================================================*/
/*  generateInputList                                                         */
/*!
    Generate assembly code for an input list

    The generateInputList function processes the INPUT_LIST node
    and generates the assembly code for reading an integer from
    an input list.  Only integers are supported at this time.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateInputList( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    struct identEntry *idEntry;
    int a;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        if( root->left != NULL )
        {
            fp = pCodeGen->fp;

            a = GenerateCode( pCodeGen, root->left );

            idEntry = root->left->ident;
            if( idEntry != NULL )
            {
                switch( idEntry->type )
                {
                    case TYPE_INT:
                        fprintf( fp, "\tRDN R%d", a );
                        fprintf( fp, "\t\t;read integer\n" );
                        fprintf( fp, "\tSTR R2,R%d\n", a );

                }
            }

            GenerateCode( pCodeGen, root->right );
        }
    }

    return result;
}

/*============================================================================*/
/*  generateToFloat                                                           */
/*!
    Generate assembly code for a type conversion to float

    The generateToFloat function processes the TO_FLOAT node
    and generates the assembly code for a type conversions to a float

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval reference to the register containing the float

==============================================================================*/
static int generateToFloat( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        fprintf( fp, "\tTOF R%d\n", a );
        result = a;
    }

    return result;
}

/*============================================================================*/
/*  generateToInt                                                             */
/*!
    Generate assembly code for a type conversion to int

    The generateToInt function processes the TO_INT node
    and generates the assembly code for a type conversions to an integer

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval reference to the register containing the float

==============================================================================*/
static int generateToInt( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        fprintf( fp, "\tTOI R%d\n", a );
        result = a;
    }

    return result;
}

/*============================================================================*/
/*  generateDelay                                                             */
/*!
    Generate assembly code for a time delay

    The generateDelay function processes the DELAY node
    and generates the assembly code for a time delay

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateDelay( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        GenerateCode( pCodeGen, root->right );
        a = GenerateCode( pCodeGen, root->left );
        fprintf( fp, "\tDLY R%d", a );
        fprintf( fp, "\t\t;delay milliseconds\n" );
    }

    return result;
}

/*============================================================================*/
/*  generateWaitSig                                                           */
/*!
    Generate assembly code to wait for a signal

    The generateWaitSig function processes the WAITSIG node
    and generates the assembly code to wait for a signal

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number for the register containing the signal value

==============================================================================*/
static int generateWaitSig( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    struct identEntry *idEntry1;
    struct identEntry *idEntry2;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        idEntry1 = GetIdentEntry( root->left );
        idEntry2 = GetIdentEntry( root->right );

        if( ( idEntry1 != NULL ) &&
            ( idEntry2 != NULL ) )
        {
            a = AllocReg( idEntry1, 0 );
            b = AllocReg( idEntry2, 0 );

            fprintf( fp, "\tWFS R%d,R%d", a, b );
            fprintf( fp, "\t;wait for signal\n" );

            fprintf( fp, "\tMOV R2,R1\n" );
            fprintf( fp, "\tADD R2,%d\n", idEntry1->offset );
            fprintf( fp, "\tSTR R2,R%d", a );
            fprintf( fp, "\t;signal number\n" );

            fprintf( fp, "\tMOV R2,R1\n" );
            fprintf( fp, "\tADD R2,%d\n", idEntry2->offset );
            fprintf( fp, "\tSTR R2,R%d", b );
            fprintf( fp, "\t;signal identifier\n" );

            result = b;
        }
    }

    return result;
}

/*============================================================================*/
/*  generateNotify                                                            */
/*!
    Generate assembly code to request a notification

    The generateNotify function processes the NOTIFY node
    and generates the assembly code to request a notification

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateNotify( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    FILE *fp;
    struct identEntry *idEntry;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        idEntry = GetIdentEntry( root->left );
        if( idEntry != NULL )
        {
            a = AllocReg( NULL, 0 );
            fprintf( fp, "\tMOV R2,R1\n" );
            fprintf( fp, "\tADD R2,%d\n", idEntry->offset );
            fprintf( fp, "\tLOD R%d,R2", a );
            fprintf( fp, "\t;var handle\n");

            b = GenerateCode( pCodeGen, root->right );

            fprintf( fp, "\tNFY R%d,R%d", a, b );
            fprintf( fp, "\t;request for notification\n" );
        }
    }

    return result;
}

/*============================================================================*/
/*  generateHandle                                                            */
/*!
    Generate assembly code to get a variable handle

    The generateHandle function processes the HANDLE node
    and generates the assembly code to get a variable handle

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number for the register containing the handle

==============================================================================*/
static int generateHandle( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int b;
    FILE *fp;
    struct identEntry *idEntry;
    bool external;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        idEntry = GetIdentEntry( root->right );
        if( idEntry != NULL )
        {
            /* find out of the lvalue is external */
            external = isExternal( root->right );
            if( external == true )
            {
                b = AllocReg( NULL, 0 );
                fprintf( fp, "\tMOV R2,R1\n" );
                fprintf( fp, "\tADD R2,%d\n", idEntry->offset );
                fprintf( fp, "\tLOD R%d,R2", b );
                fprintf( fp, "\t;var handle\n");

                result = b;
            }
            else
            {
                fprintf( stderr,
                        "E: invalid reference on line %d, "
                        "handle() can only be used with external variables\n",
                        getlineno() );
            }

        }
    }

    return result;
}

/*============================================================================*/
/*  generateValidateStart                                                     */
/*!
    Generate assembly code to start variable validation

    The generateValidateStart function processes the VALIDATE_START node
    and generates the assembly code to start a varible validation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number for the register containing the variable handle

==============================================================================*/
static int generateValidateStart( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    FILE *fp;
    struct identEntry *idEntry;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        fprintf( fp, ";Start Validation\n");
        idEntry = GetIdentEntry( root->right );
        if( idEntry != NULL )
        {
            b = AllocReg( NULL, 0 );
            fprintf( fp, "\tMOV R2,R1\n" );
            fprintf( fp, "\tADD R2,%d\n", idEntry->offset );
            fprintf( fp, "\tLOD R%d,R2", b );
            fprintf( fp, "\t;notification identifier\n");

            a = AllocReg( NULL, 0 );
            fprintf( fp, "\tEVS R%d,R%d", a, b );
            fprintf( fp, "\t; start validation: "
                         "get var handle from notification id\n");

            result = a;
        }
    }

    return result;
}

/*============================================================================*/
/*  generateValidateEnd                                                       */
/*!
    Generate assembly code to end variable validation

    The generateValidateEnd function processes the VALIDATE_END node
    and generates the assembly code to end a varible validation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateValidateEnd( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        fprintf( fp, ";End Validation\n");
        b = GenerateCode( pCodeGen, root->right );
        a = GenerateCode( pCodeGen, root->left );
        fprintf( fp, "\tEVE R%d,R%d", a, b );
        fprintf( fp, "\t; end validation\n");
    }

    return result;
}

/*============================================================================*/
/*  generateSetTimer                                                          */
/*!
    Generate assembly code to set a timer

    The generateSetTimer function processes the SETTIMER node
    and generates the assembly code to start a timer

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateSetTimer( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        b = GenerateCode( pCodeGen, root->right );

        fprintf( fp, "\tSTM R%d,R%d", a, b );
        fprintf( fp, "\t;set timer\n" );
    }

    return result;

}

/*============================================================================*/
/*  generateClearTimer                                                        */
/*!
    Generate assembly code to clear a timer

    The generateClearTimer function processes the CLEARTIMER node
    and generates the assembly code to clear a timer

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateClearTimer( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        fprintf( fp, "\tCTM R%d", a );
        fprintf( fp, "\t;clear timer\n" );
    }

    return result;

}

/*============================================================================*/
/*  isExternal                                                                */
/*!
    Check if a node refers to an external variable

    The isExternal function checks if the identEntry associated with the
    specified node refers to an external variable.

    @param[in]
        root
            pointer to the Node object to check

    @retval true - the node is associated with an external variable
    @retval false - the node is not associated with an external variable

==============================================================================*/
static bool isExternal( struct Node *root )
{
    bool isExternal = false;
    struct identEntry *idEntry;

    if( root != NULL )
    {
        idEntry = root->ident;
        if( idEntry != NULL )
        {
            isExternal = idEntry->isExternal;
        }
    }

    return isExternal;
}

/*============================================================================*/
/*  isStringBuffer                                                            */
/*!
    Check if an identEntry object refers to a string buffer

    The isStringBuffer function checks if the specified identEntry object
    is associated with a string buffer.

    @param[in]
        idEntry
            pointer to the identEntry object to check

    @retval true - the identEntry object is associated with a string buffer
    @retval false - the identEntry object is not associated with a string buffer

==============================================================================*/
static bool isStringBuffer( struct identEntry *idEntry )
{
    bool isString = false;

    if( idEntry != NULL )
    {
        if( idEntry->stringBufID != 0 )
        {
            isString = true;
        }
    }

    return isString;
}

/*============================================================================*/
/*  GetIdentEntry                                                             */
/*!
    Get the identEntry object associated with the Node

    The GetIdentEntry function gets the identEntry object associated with the
    Node.

    @param[in]
        node
            pointer to the Node to get the identEntry for.

    @retval pointer to the associated identEntry object
    @retval NULL - no identEntry object associated with the node

==============================================================================*/
static struct identEntry *GetIdentEntry( struct Node *root )
{
    struct identEntry *idEntry = NULL;

    if( root != NULL )
    {
        idEntry = root->ident;
    }

    return idEntry;
}

/*============================================================================*/
/*  generateTimesEquals                                                       */
/*!
    Generate assembly code to perform a *= operation

    The generateTimesEquals function processes the TIMES_EQUALS node
    and generates the assembly code perform a *= operation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number for the register containing the result

==============================================================================*/
static int generateTimesEquals( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    struct identEntry *idEntry;
    bool external = false;
    int a;
    int b;
    int c;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        b = GenerateCode( pCodeGen, root->right );
        a = GenerateCode( pCodeGen, root->left );
        c = a;

        /* find out of the lvalue is external */
        external = isExternal( root->left );

        /* get the ident entry for the lvalue */
        idEntry = GetIdentEntry( root->left );

        if( external == true )
        {
            c = GetExternal( pCodeGen,
                             idEntry,
                             a,
                             "retrieve external variable" );
        }

        if( root->datatype == TYPE_FLOAT )
        {
            fprintf( fp, "\tMUL.F R%d,R%d", c, b );
            fprintf( fp, "\t;floating point multiplication\n" );
        }
        else
        {
            fprintf( fp, "\tMUL R%d,R%d", c, b );
            fprintf( fp, "\t;integer multiplication\n" );
        }

        if( external == true )
        {
            SetExternal( pCodeGen, idEntry, c, "external assignment" );
        }
        else
        {
            fprintf( fp, "\tSTR R2,R%d", a );
            fprintf( fp, "\t;assignment\n" );
        }

        result = c;
    }

    return result;
}

/*============================================================================*/
/*  generateDivEquals                                                         */
/*!
    Generate assembly code to perform a /= operation

    The generateTimesEquals function processes the DIV_EQUALS node
    and generates the assembly code perform a /= operation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number for the register containing the result

==============================================================================*/
static int generateDivEquals( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    struct identEntry *idEntry;
    bool external = false;
    int a;
    int b;
    int c;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        /* find out of the lvalue is external */
        external = isExternal( root->left );

        /* get the ident entry for the lvalue */
        idEntry = GetIdentEntry( root->left );

        b = GenerateCode( pCodeGen, root->right );
        a = GenerateCode( pCodeGen, root->left );
        c = a;

        if( external == true )
        {
            c = GetExternal( pCodeGen,
                             idEntry,
                             a,
                             "retrieve external variable" );
        }

        if( root->datatype == TYPE_FLOAT )
        {
            fprintf( fp, "\tDIV.F R%d,R%d", c, b );
            fprintf( fp, "\t;floating point division\n" );
        }
        else
        {
            fprintf( fp, "\tDIV R%d,R%d", c, b );
            fprintf( fp, "\t;integer division\n" );
        }

        if( external == true )
        {
            SetExternal( pCodeGen, idEntry, a, "external assignment" );
        }
        else
        {
            fprintf( fp, "\tSTR R2,R%d", a );
            fprintf( fp, "\t;assignment\n" );
        }

        result = c;
    }

    return result;
}

/*============================================================================*/
/*  generatePlusEquals                                                        */
/*!
    Generate assembly code to perform a += operation

    The generatePlusEquals function processes the PLUS_EQUALS node
    and generates the assembly code perform a += operation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number for the register containing the result

==============================================================================*/
static int generatePlusEquals( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    struct identEntry *idEntry;
    bool external;
    bool isString;
    int a;
    int b;
    int c;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        /* find out of the lvalue is external */
        external = isExternal( root->left );

        /* get the ident entry for the lvalue */
        idEntry = GetIdentEntry( root->left );

        /* see if the lvalue is a string */
        isString = isStringBuffer( idEntry );

        b = GenerateCode( pCodeGen, root->right );
        a = GenerateCode( pCodeGen, root->left );
        c = a;

        if( external  == true )
        {
            c = GetExternal( pCodeGen,
                             idEntry,
                             a,
                             "retrieve external variable" );
        }

        if( root->datatype == TYPE_FLOAT )
        {
            fprintf( fp, "\tADD.F R%d,R%d", c, b );
            fprintf( fp, "\t;floating point addition\n" );
        }
        else
        {
            fprintf( fp, "\tADD R%d,R%d", c , b );
            fprintf( fp, "\t;integer addition\n" );
        }

        if ( idEntry == NULL )
        {
            fprintf( fp,"\tSTR R2,R%d",a );
            fprintf( fp,"\t;assignment\n" );
        }
        else if( external == true )
        {
            SetExternal( pCodeGen, idEntry, c, "external assignment" );
        }
        else if ( isString == true )
        {
            fprintf( fp, "\t;ASB R%d,R%d", a, b );
            fprintf( fp, "\t;append to string buffer\n" );
        }
        else
        {
            fprintf( fp, "\tSTR R2,R%d", a );
            fprintf( fp, "\t;assignment\n" );
        }

        result = c;
    }

    return result;
}

/*============================================================================*/
/*  generateMinusEquals                                                       */
/*!
    Generate assembly code to perform a -= operation

    The generatePlusEquals function processes the MINUS_EQUALS node
    and generates the assembly code perform a -= operation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number for the register containing the result

==============================================================================*/
static int generateMinusEquals( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    struct identEntry *idEntry;
    bool external;
    int a;
    int b;
    int c;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        /* find out of the lvalue is external */
        external = isExternal( root->left );

        /* get the ident entry for the lvalue */
        idEntry = GetIdentEntry( root->left );

        b = GenerateCode( pCodeGen, root->right );
        a = GenerateCode( pCodeGen, root->left );
        c = a;

        if( external == true )
        {
            c = GetExternal( pCodeGen,
                             idEntry,
                             a,
                             "retrieve external variable" );
        }

        if( root->datatype == TYPE_FLOAT )
        {
            fprintf( fp, "\tSUB.F R%d,R%d", c, b );
            fprintf( fp, "\t;floating point subtraction\n" );
        }
        else
        {
            fprintf( fp, "\tSUB R%d,R%d", c, b );
            fprintf( fp, "\t;integer subtraction\n" );
        }

        if( external == true )
        {
            SetExternal( pCodeGen, idEntry, b, "external assignment" );
        }
        else
        {
            fprintf( fp, "\tSTR R2,R%d", a );
            fprintf( fp, "\t;assignment\n" );
        }

        result = c;

    }

    return result;
}

/*============================================================================*/
/*  generateAndEquals                                                         */
/*!
    Generate assembly code to perform a &= operation

    The generateAndEquals function processes the AND_EQUALS node
    and generates the assembly code perform a &= operation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number for the register containing the result

==============================================================================*/
static int generateAndEquals( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    struct identEntry *idEntry;
    bool external;
    int a;
    int b;
    int c;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        /* find out of the lvalue is external */
        external = isExternal( root->left );

        /* get the ident entry for the lvalue */
        idEntry = GetIdentEntry( root->left );

        b = GenerateCode( pCodeGen, root->right );
        a = GenerateCode( pCodeGen, root->left );
        c = a;

        if( external == true )
        {
            c = GetExternal( pCodeGen,
                             idEntry,
                             a,
                             "retrieve external variable" );
        }

        fprintf( fp,"\tAND R%d,R%d", c, b );
        fprintf( fp,"\t;Bitwise AND\n" );

        if( external == true )
        {
            SetExternal( pCodeGen, idEntry, b, "external assignment" );
        }
        else
        {
            fprintf( fp, "\tSTR R2,R%d", a );
            fprintf( fp, "\t;assignment\n" );
        }

        result = c;
    }

    return result;
}

/*============================================================================*/
/*  generateOrEquals                                                         */
/*!
    Generate assembly code to perform a |= operation

    The generateOrEquals function processes the OR_EQUALS node
    and generates the assembly code perform a |= operation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number for the register containing the result

==============================================================================*/
static int generateOrEquals( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    struct identEntry *idEntry;
    bool external;
    int a;
    int b;
    int c;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        /* find out of the lvalue is external */
        external = isExternal( root->left );

        /* get the ident entry for the lvalue */
        idEntry = GetIdentEntry( root->left );

        b = GenerateCode( pCodeGen, root->right );
        a = GenerateCode( pCodeGen, root->left );
        c = a;

        if( external == true )
        {
            c = GetExternal( pCodeGen,
                             idEntry,
                             a,
                             "retrieve external variable" );
        }

        fprintf( fp, "\tOR R%d,R%d", c, b );
        fprintf( fp, "\t;Bitwise OR\n" );

        if( external == true )
        {
            SetExternal( pCodeGen, idEntry, b, "external assignment" );
        }
        else
        {
            fprintf( fp, "\tSTR R2,R%d", a );
            fprintf( fp, "\t;assignment\n" );
        }

        result = c;
    }

    return result;
}

/*============================================================================*/
/*  generateXorEquals                                                         */
/*!
    Generate assembly code to perform a ^= operation

    The generateOrEquals function processes the XOR_EQUALS node
    and generates the assembly code perform a ^= operation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number for the register containing the result

==============================================================================*/
static int generateXorEquals( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    struct identEntry *idEntry;
    bool external;
    int a;
    int b;
    int c;
    int d;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        /* find out of the lvalue is external */
        external = isExternal( root->left );

        /* get the ident entry for the lvalue */
        idEntry = GetIdentEntry( root->left );

        b = GenerateCode( pCodeGen, root->right );
        a = GenerateCode( pCodeGen, root->left );

        c = AllocReg( NULL, 0 );
        d = a;

        if( external == true )
        {
            d = GetExternal( pCodeGen,
                             idEntry,
                             a,
                             "retrieve external variable" );
        }

        fprintf( fp,"\tMOV R%d,R%d", c, d );
        fprintf( fp,"\t;exclusive or\n" );
        fprintf( fp,"\tOR R%d,R%d\n", c, b );
        fprintf( fp,"\tAND R%d,R%d\n", d, b );
        fprintf( fp,"\tNOT R%d\n", d );
        fprintf( fp,"\tAND R%d,R%d\n", c, d );

        if( external == true )
        {
            SetExternal( pCodeGen, idEntry, c, "external assignment" );
        }
        else
        {
            fprintf( fp, "\tSTR R2,R%d", c );
            fprintf( fp, "\t;assignment\n" );
        }

        result = c;
    }

    return result;
}

/*============================================================================*/
/*  generateArray                                                             */
/*!
    Generate assembly code to perform array indexing

    The generateArray function processes the ARRAY node
    and generates the assembly code to perform array indexing

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number for the register containing the array value

==============================================================================*/
static int generateArray( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        b = GenerateCode( pCodeGen, root->right );
        a = GenerateCode( pCodeGen, root->left );
        if (a != -1)
        {
            fprintf( fp, "\tMUL R%d,%lu", b, (unsigned long)sizeof(uint32_t) );
            fprintf( fp, "\t;multiply array offset by stack element size\n" );
            fprintf( fp, "\tSUB R2,R%d", b );
            fprintf( fp, "\t;calculate array offset\n" );
            fprintf( fp, "\tLOD R%d,R2\n", a );
        }

        result = a;
    }

    return result;
}

/*============================================================================*/
/*  generateDeclarationList                                                   */
/*!
    Generate assembly code to handle a declaration list

    The generateDeclarationList function processes the DECL_LIST node
    and generates the assembly code to process a declaration list

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval result of generateChildren

==============================================================================*/
static int generateDeclarationList( CodeGen *pCodeGen, struct Node *root )
{
    return generateChildren( pCodeGen, root );
}

/*============================================================================*/
/*  generateDeclaration                                                       */
/*!
    Generate assembly code to handle a declaration

    The generateDeclaration function processes the DECLN node
    and generates the assembly code to process a variable declaration

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval result of generateChildren

==============================================================================*/
static int generateDeclaration( CodeGen *pCodeGen, struct Node *root )
{
    return generateChildren( pCodeGen, root );
}

/*============================================================================*/
/*  generateExternDeclarationList                                             */
/*!
    Generate assembly code to handle an external declaration list

    The generateExternDeclarationList function processes the EXTERN_DECL_LIST
    node and generates the assembly code to process the external
    declaration list

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval result of generateChildren

==============================================================================*/
static int generateExternDeclarationList( CodeGen *pCodeGen, struct Node *root )
{
    return generateChildren( pCodeGen, root );
}

/*============================================================================*/
/*  generateExternDeclaration                                                 */
/*!
    Generate assembly code to handle an external variable declaration

    The generateExternDeclaration function processes the EXTERN_DECLN
    node and generates the assembly code to process the external variable
    declaration

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval result of generateChildren

==============================================================================*/
static int generateExternDeclaration( CodeGen *pCodeGen, struct Node *root )
{
    return generateChildren( pCodeGen, root );
}

/*============================================================================*/
/*  generateProcCall                                                          */
/*!
    Generate assembly code to handle a function call

    The generateProcCall function processes the PROC_CALL
    node and generates the assembly code to process the function call

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval 0
    @retval -1 invalid arguments

==============================================================================*/
static int generateProcCall( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        fprintf( fp, "\tMOV R0,SP" );
        fprintf( fp, "\t;save stack pointer\n" );

        GenerateCode( pCodeGen, root->right );
        GenerateCode( pCodeGen, root->left );

        result = 0;
    }

    return result;
}

/*============================================================================*/
/*  generateArgList                                                           */
/*!
    Generate assembly code for an argument list

    The generateArgList function processes the ARG_LIST
    node and generates the assembly code for an argument list

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number containing the argument

==============================================================================*/
static int generateArgList( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        GenerateCode( pCodeGen, root->right );
        a = GenerateCode( pCodeGen, root->left );
        if (a != -1)
        {
            fprintf( fp, "\tPSH R%d", a );
            fprintf( fp, "\t\t;push argument\n" );
        }

        result = a;
    }

    return result;
}

/*============================================================================*/
/*  generateParameterList                                                     */
/*!
    Generate assembly code for a parameter list

    The generateParameterList function processes the PARAM_LIST
    node and generates the assembly code for the parameter list

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval result of generateChildren

==============================================================================*/
static int generateParameterList( CodeGen *pCodeGen, struct Node *root )
{
    return generateChildren( pCodeGen, root );
}

/*============================================================================*/
/*  generateParameter                                                         */
/*!
    Generate assembly code for a parameter

    The generateParameter function processes the PARAMETER
    node and generates the assembly code for the parameter

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval result of generateChildren

==============================================================================*/
static int generateParameter( CodeGen *pCodeGen, struct Node *root )
{
    return generateChildren( pCodeGen, root );
}

/*============================================================================*/
/*  generateOr                                                                */
/*!
    Generate assembly code for a logical OR operation

    The generateOr function processes the OR node and generates the
    assembly code for the logical OR operation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number containing the result

==============================================================================*/
static int generateOr( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    int n;
    FILE *fp;
    char label[7];
    char label1[7];

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        n = AllocReg( NULL, 0 );

        sprintf( (char *)label, "_OR%d", GetLabelNumber() );
        sprintf( (char *)label1, "_OR%d", GetLabelNumber() );

        a = GenerateCode( pCodeGen, root->left );
        b = GenerateCode( pCodeGen, root->right );

        fprintf( fp, "\tMOV R%d,0\n", n );
        fprintf( fp, "\tCMP R%d,0\n", a );
        fprintf( fp, "\tJNZ %s\n", label );
        fprintf( fp, "\tCMP R%d,0\n", b );
        fprintf( fp, "\tJNZ %s\n", label );
        fprintf( fp, "\tJMP %s\n", label1 );
        fprintf( fp, "%s\n", label );
        fprintf( fp, "\tMOV R%d,1", n );
        fprintf( fp, "\t;logical OR\n");
        fprintf( fp, "%s\n", label1 );
        result = n;
    }

    return result;
}

/*============================================================================*/
/*  generateAnd                                                               */
/*!
    Generate assembly code for a logical AND operation

    The generateAnd function processes the AND node and generates the
    assembly code for the logical AND operation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number containing the result

==============================================================================*/
static int generateAnd( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    int n;
    FILE *fp;
    char label[7];

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        n = AllocReg( NULL, 0 );

        sprintf( (char *)label, "_AND%d", GetLabelNumber() );

        a = GenerateCode( pCodeGen, root->left );
        b = GenerateCode( pCodeGen, root->right );

        fprintf( fp, "\tMOV R%d,0\n", n );
        fprintf( fp, "\tCMP R%d,0\n", a );
        fprintf( fp, "\tJZR %s\n", label );
        fprintf( fp, "\tCMP R%d,0\n", b );
        fprintf( fp, "\tJZR %s\n", label );
        fprintf( fp, "\tMOV R%d,1", n );
        fprintf( fp, "\t;Logical AND \n" );
        fprintf( fp, "%s\n", label );

        result = n;
    }

    return result;
}

/*============================================================================*/
/*  generateXor                                                               */
/*!
    Generate assembly code for an XOR operation

    The generateXor function processes the XOR node and generates the
    assembly code for the XOR operation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number containing the result

==============================================================================*/
static int generateXor( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    int c;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        b = GenerateCode( pCodeGen, root->right );
        c = AllocReg( NULL, 0 );

        fprintf( fp, "\tMOV R%d,R%d", c, a );
        fprintf( fp, "\t;Exclusive OR \n" );
        fprintf( fp, "\tOR R%d,R%d\n", c, b );
        fprintf( fp, "\tAND R%d,R%d\n", a, b );
        fprintf( fp, "\tNOT R%d\n", a );
        fprintf( fp, "\tAND R%d,R%d\n", c, a );

        result = c;
    }

    return result;
}

/*============================================================================*/
/*  generateBor                                                               */
/*!
    Generate assembly code for a bitwise OR operation

    The generateBor function processes the BOR node and generates the
    assembly code for the bitwise OR operation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number containing the result

==============================================================================*/
static int generateBor( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left) ;
        b = GenerateCode( pCodeGen, root->right );

        fprintf( fp, "\tOR R%d,R%d", a, b );
        fprintf( fp, "\t;Bitwise OR \n" );

        result = a;
    }

    return result;
}

/*============================================================================*/
/*  generateBand                                                              */
/*!
    Generate assembly code for a bitwise AND operation

    The generateBand function processes the BAND node and generates the
    assembly code for the bitwise AND operation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number containing the result

==============================================================================*/
static int generateBand( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        b = GenerateCode( pCodeGen, root->right );

        fprintf( fp, "\tAND R%d,R%d", a, b );
        fprintf( fp, "\t;Bitwise AND \n" );

        result = a;
    }

    return result;
}

/*============================================================================*/
/*  generateNot                                                               */
/*!
    Generate assembly code for a logical NOT operation

    The generateNot function processes the NOT node and generates the
    assembly code for the logical NOT operation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number containing the result

==============================================================================*/
static int generateNot( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int b;
    int n;
    FILE *fp;
    char label[7];

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        sprintf( (char *)label, "_BNOT%d", GetLabelNumber() );

        n = AllocReg( NULL, 0 );
        b = GenerateCode( pCodeGen, root->right );

        fprintf( fp, "\tMOV R%d,1\n", n );
        fprintf( fp, "\tCMP R%d,0\n", b );
        fprintf( fp, "JZR %s\n", label );
        fprintf( fp, "\tMOV R%d,0", n );
        fprintf( fp, "\t;Logical NOT \n" );
        fprintf( fp, "%s\n", label );
        result = n;
    }

    return result;
}

/*============================================================================*/
/*  generateBnot                                                              */
/*!
    Generate assembly code for a bitwise NOT operation

    The generateBnot function processes the BNOT node and generates the
    assembly code for the bitwise NOT operation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number containing the result

==============================================================================*/
static int generateBnot( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int b;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        b = GenerateCode( pCodeGen, root->right );

        fprintf( fp, "\tNOT R%d", b );
        fprintf( fp, "\t;Bitwise NOT \n" );

        result = b;
    }

    return result;
}

/*============================================================================*/
/*  generateEquals                                                            */
/*!
    Generate assembly code for a equals (==) comparison operation

    The generateEquals function processes the EQUALS node and generates the
    assembly code for the equals comparison operation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number containing the result (0 or 1)

==============================================================================*/
static int generateEquals( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    int c;
    char label[7];
    char label1[7];            /* label generation */
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        b = GenerateCode( pCodeGen, root->right );

        c = AllocReg( NULL, 0 );

        sprintf( (char *)label, "_EQU%d", GetLabelNumber() );
        sprintf( (char *)label1, "_EQU%d", GetLabelNumber() );

        if( root->datatype == TYPE_FLOAT )
        {
            fprintf( fp, "\tCMP.F R%d,R%d", a, b );
            fprintf( fp, "\t;floating point equals comparison\n" );
        }
        else
        {
            fprintf( fp, "\tCMP R%d,R%d", a ,b );
            fprintf( fp, "\t;equals comparison\n" );
        }

        fprintf( fp, "\tJZR %s\n", label );
        fprintf( fp, "\tMOV R%d,0\n", c );
        fprintf( fp, "\tJMP %s\n", label1 );
        fprintf( fp, "%s\n\tMOV R%d,1\n", label, c );
        fprintf( fp, "%s\n", label1 );

        result = c;
    }

    return result;
}

/*============================================================================*/
/*  generateNotEquals                                                         */
/*!
    Generate assembly code for a not equals (!=) comparison operation

    The generateNotEquals function processes the NOTEQUALS node and generates
    the assembly code for the not equals comparison operation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number containing the result (0 or 1)

==============================================================================*/
static int generateNotEquals( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    int c;
    char label[7];
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        b = GenerateCode( pCodeGen, root->right );

        c = AllocReg( NULL, 0 );

        sprintf( (char *)label, "_NEQ%d", GetLabelNumber() );
        if( root->datatype == TYPE_FLOAT )
        {
            fprintf( fp, "\tCMP.F R%d,R%d", a, b );
            fprintf( fp, "\t;floating point not equals comparison\n" );
        }
        else
        {
            fprintf( fp, "\tCMP R%d,R%d", a, b );
            fprintf( fp, "\t;not equals comparison\n" );
        }

        fprintf( fp, "\tMOV R%d,0\n", c );
        fprintf( fp, "\tJZR %s\n", label );
        fprintf( fp, "\tMOV R%d,1\n", c );
        fprintf( fp, "%s\n", label );

        result = c;
    }

    return result;
}

/*============================================================================*/
/*  generateLessThanOrEqual                                                   */
/*!
    Generate assembly code for a less than or equal (<=) comparison operation

    The generateLessThanOrEqual function processes the LTE node and generates
    the assembly code for the less than or equal comparison operation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number containing the result (0 or 1)

==============================================================================*/
static int generateLessThanOrEqual( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    int c;
    char label[7];
    char label1[7];            /* label generation */
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        b = GenerateCode( pCodeGen, root->right );

        c = AllocReg( NULL, 0 );

        sprintf( (char *)label, "_LTE%d", GetLabelNumber() );
        sprintf( (char *)label1, "_LTE%d", GetLabelNumber() );

        if( root->datatype == TYPE_FLOAT )
        {
            fprintf( fp, "\tCMP.F R%d,R%d", b, a );
            fprintf( fp, "\t;floating point LTE comparison\n" );
        }
        else
        {
            fprintf( fp, "\tCMP R%d,R%d", b, a );
            fprintf( fp, "\t;LTE comparison\n" );
        }

        fprintf( fp, "\tJPO %s\n", label );
        fprintf( fp, "\tMOV R%d,0\n", c );
        fprintf( fp, "\tJMP %s\n", label1 );
        fprintf( fp, "%s\n\tMOV R%d,1\n", label, c );
        fprintf( fp, "%s\n", label1 );

        result = c;
    }

    return result;
}

/*============================================================================*/
/*  generateGreaterThanOrEqual                                                */
/*!
    Generate assembly code for a greater than or equal (>=) comparison operation

    The generateGreaterThanOrEqual function processes the GTE node and generates
    the assembly code for the greater than or equal comparison operation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number containing the result (0 or 1)

==============================================================================*/
static int generateGreaterThanOrEqual( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    int c;
    char label[7];
    char label1[7];            /* label generation */
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        b = GenerateCode( pCodeGen, root->right );

        c = AllocReg( NULL, 0 );
        sprintf( (char *)label, "_GTE%d", GetLabelNumber() );
        sprintf( (char *)label1, "_GTE%d", GetLabelNumber() );

        if( root->datatype == TYPE_FLOAT )
        {
            fprintf( fp, "\tCMP.F R%d,R%d", a, b );
            fprintf( fp, "\t;floating point GTE comparison\n" );
        }
        else
        {
            fprintf( fp, "\tCMP R%d,R%d", a, b );
            fprintf( fp, "\t;GTE comparison\n" );
        }

        fprintf( fp,"\tJPO %s\n", label );
        fprintf( fp,"\tMOV R%d,0\n", c );
        fprintf( fp,"\tJMP %s\n", label1 );
        fprintf( fp,"%s\n\tMOV R%d,1\n", label, c );
        fprintf( fp,"%s\n", label1 );

        result = c;
    }

    return result;
}

/*============================================================================*/
/*  generateLessThan                                                          */
/*!
    Generate assembly code for a less than (<) comparison operation

    The generateLessThan function processes the LT node and generates
    the assembly code for the less than operation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number containing the result (0 or 1)

==============================================================================*/
static int generateLessThan( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    int c;
    char label[7];
    char label1[7];            /* label generation */
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        b = GenerateCode( pCodeGen, root->right );

        c = AllocReg( NULL, 0 );

        sprintf( (char *)label, "_LT%d", GetLabelNumber() );
        sprintf( (char *)label1, "_LT%d", GetLabelNumber() );

        if( root->datatype == TYPE_FLOAT )
        {
            fprintf( fp, "\tCMP.F R%d,R%d", a, b );
            fprintf( fp, "\t;floating point LT comparison\n" );
        }
        else
        {
            fprintf( fp, "\tCMP R%d,R%d", a, b );
            fprintf( fp, "\t;LT comparison\n" );
        }

        fprintf( fp, "\tJNE %s\n", label );
        fprintf( fp, "\tMOV R%d,0\n", c );
        fprintf( fp, "\tJMP %s\n", label1 );
        fprintf( fp, "%s\n\tMOV R%d,1\n", label, c );
        fprintf( fp, "%s\n", label1 );

        result = c;
    }

    return result;
}

/*============================================================================*/
/*  generateGreaterThan                                                       */
/*!
    Generate assembly code for a greater than (>) comparison operation

    The generateGreaterThan function processes the GT node and generates
    the assembly code for the greater than operation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number containing the result (0 or 1)

==============================================================================*/
static int generateGreaterThan( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    int c;
    char label[7];
    char label1[7];            /* label generation */
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        b = GenerateCode( pCodeGen, root->right );

        c = AllocReg( NULL, 0 );

        sprintf( (char *)label, "_GT%d", GetLabelNumber() );
        sprintf( (char *)label1, "_GT%d", GetLabelNumber() );

        if( root->datatype == TYPE_FLOAT )
        {
            fprintf( fp, "\tCMP.F R%d,R%d", b, a );
            fprintf( fp, "\t;floating point GT comparison\n" );
        }
        else
        {
            fprintf( fp, "\tCMP R%d,R%d", b, a );
            fprintf( fp, "\t;GT comparison\n" );
        }

        fprintf( fp, "\tJNE %s\n", label );
        fprintf( fp, "\tMOV R%d,0\n", c );
        fprintf( fp, "\tJMP %s\n", label1 );
        fprintf( fp, "%s\n\tMOV R%d,1\n", label, c );
        fprintf( fp, "%s\n", label1 );

        result = c;
    }

    return result;
}

/*============================================================================*/
/*  generateRShift                                                            */
/*!
    Generate assembly code for a right shift operation

    The generateRShift function processes the RSHIFT node and generates
    the assembly code for the right shift operation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number containing the result

==============================================================================*/
static int generateRShift( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    char label[7];
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        b = GenerateCode( pCodeGen, root->right );

        sprintf( (char *)label, "_RS%d", GetLabelNumber() );
        fprintf( fp, "%s\tSHR R%d,1", label, a );
        fprintf( fp, "\t;Right shift\n" );
        fprintf( fp, "\tSUB R%d,1\n", b );
        fprintf( fp, "\tJNZ %s\n", label );

        result = a;

    }

    return result;
}

/*============================================================================*/
/*  generateLShift                                                            */
/*!
    Generate assembly code for a left shift operation

    The generateLShift function processes the LSHIFT node and generates
    the assembly code for the left shift operation

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number containing the result

==============================================================================*/
static int generateLShift( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    char label[7];
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        b = GenerateCode( pCodeGen, root->right );

        sprintf( (char *)label, "_LS%d", GetLabelNumber() );

        fprintf( fp, "%s\tSHL R%d,1", label, a );
        fprintf( fp, "\t;left shift \n" );
        fprintf( fp, "\tSUB R%d,1\n", b );
        fprintf( fp, "\tJNZ %s\n", label );

        result = a;

    }

    return result;
}

/*============================================================================*/
/*  generateIncrement                                                         */
/*!
    Generate assembly code for an increment operation

    The generateIncrement function processes the INC node and generates
    the assembly code for the increment operation.  Both pre and post
    increment are supported.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number containing the result

==============================================================================*/
static int generateIncrement( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int n;
    int a;
    int b;
    struct identEntry *idEntry;
    bool external;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        if( root->left != NULL )
        {
            /* post increment */
            /* find out of the lvalue is external */
            external = isExternal( root->left );

            /* get the ident entry for the lvalue */
            idEntry = GetIdentEntry( root->left );

            /* a new register is needed to store the result because
               we need to return the value of the input BEFORE it is
               incremented */
            n = AllocReg( NULL, 0 );

            /* generate code for getting the variable identifier */
            a = GenerateCode( pCodeGen, root->left );

            fprintf( fp, "\tMOV R%d,R%d\n", n, a );
            fprintf( fp, "\tADD R%d,1\n", a );

            if( external == true )
            {
                SetExternal( pCodeGen, idEntry, a, "post increment external" );
            }
            else
            {
                fprintf( fp, "\tSTR R2,R%d", a );
                fprintf( fp, "\t;post-increment\n" );
            }

            result = n;
        }
        else if ( root->right != NULL )
        {
            /* pre-increment */
            /* find out of the lvalue is external */
            external = isExternal( root->right );

            /* get the ident entry for the lvalue */
            idEntry = GetIdentEntry( root->right );

            b = GenerateCode( pCodeGen, root->right );

            fprintf( fp, "\tADD R%d,1\n", b );

            if( external == true )
            {
                SetExternal( pCodeGen, idEntry, b, "pre-increment external" );
            }
            else
            {
                fprintf( fp, "\tSTR R2,R%d", b );
                fprintf( fp, "\t;pre-increment\n" );
            }

            result = b;
        }
    }

    return result;
}

/*============================================================================*/
/*  generateDecrement                                                         */
/*!
    Generate assembly code for a decrement operation

    The generateDecrement function processes the DEC node and generates
    the assembly code for the decrement operation.  Both pre and post
    decrement are supported.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number containing the result

==============================================================================*/
static int generateDecrement( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int n;
    int a;
    int b;
    struct identEntry *idEntry;
    bool external;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;
        if ( root->left != NULL )
        {
            /* post decrement */
            /* find out of the lvalue is external */
            external = isExternal( root->left );

            /* get the ident entry for the lvalue */
            idEntry = GetIdentEntry( root->left );

            /* a new register is needed to store the result because
               we need to return the value of the input BEFORE it is
               decremented */
            n = AllocReg( NULL, 0 );
            a = GenerateCode( pCodeGen, root->left );

            fprintf( fp, "\tMOV R%d,R%d\n", n, a );
            fprintf( fp, "\tSUB R%d,1\n", a );

            if( external == true )
            {
                SetExternal( pCodeGen, idEntry, a, "post decrement external" );
            }
            else
            {
                fprintf( fp, "\tSTR R2,R%d", a );
                fprintf( fp, "\t;post-decrement\n" );
            }

            result = n;
        }
        else if (root->right)
        {
            /* pre decrement */
            /* find out of the lvalue is external */
            external = isExternal( root->right );

            /* get the ident entry for the lvalue */
            idEntry = GetIdentEntry( root->right );

            b = GenerateCode( pCodeGen, root->right );

            fprintf( fp, "\tSUB R%d,1\n", b );

            if( external == true )
            {
                SetExternal( pCodeGen, idEntry, b, "pre-decrement external" );
            }
            else
            {
                fprintf( fp, "\tSTR R2,R%d", b );
                fprintf( fp, "\t;pre-decrement\n" );
            }

            result = b;
        }

    }

    return result;
}

/*============================================================================*/
/*  generatePlus                                                              */
/*!
    Generate assembly code for a addition operation

    The generatePlus function processes the PLUS node and generates
    the assembly code for the addition operation.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number containing the result

==============================================================================*/
static int generatePlus( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        b = GenerateCode( pCodeGen, root->right );

        if( root->datatype == TYPE_FLOAT )
        {
            fprintf( fp, "\tADD.F R%d,R%d", a, b );
            fprintf( fp, "\t;floating point addition\n" );
        }
        else
        {
            fprintf( fp, "\tADD R%d,R%d", a, b );
            fprintf( fp, "\t;integer addition\n" );
        }

        result = a;

    }

    return result;
}

/*============================================================================*/
/*  generateMinus                                                             */
/*!
    Generate assembly code for a subtraction operation

    The generateMinus function processes the MINUS node and generates
    the assembly code for the subtraction operation.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number containing the result

==============================================================================*/
static int generateMinus( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        b = GenerateCode( pCodeGen, root->right );
        if( root->datatype == TYPE_FLOAT )
        {
            fprintf( fp, "\tSUB.F R%d,R%d", a, b );
            fprintf( fp, "\t;floating point subtraction\n" );
        }
        else
        {
            fprintf( fp, "\tSUB R%d,R%d", a, b );
            fprintf( fp, "\t;integer subtraction\n" );
        }

        result = a;
    }

    return result;
}

/*============================================================================*/
/*  generateTimes                                                             */
/*!
    Generate assembly code for a multiplication operation

    The generateTimes function processes the TIMES node and generates
    the assembly code for the multiplication operation.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number containing the result

==============================================================================*/
static int generateTimes( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        b = GenerateCode( pCodeGen, root->right );

        if( root->datatype == TYPE_FLOAT )
        {
            fprintf( fp, "\tMUL.F R%d,R%d", a, b );
            fprintf( fp, "\t;floating point multiplication\n" );
        }
        else
        {
            fprintf( fp, "\tMUL R%d,R%d", a, b );
            fprintf( fp, "\t;integer multiplication\n" );
        }

        result = a;
    }

    return result;
}

/*============================================================================*/
/*  generateDivide                                                            */
/*!
    Generate assembly code for a division operation

    The generateDivide function processes the DIVIDE node and generates
    the assembly code for the division operation.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval register number containing the result

==============================================================================*/
static int generateDivide( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;
    int a;
    int b;
    FILE *fp;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( root != NULL ) )
    {
        fp = pCodeGen->fp;

        a = GenerateCode( pCodeGen, root->left );
        b = GenerateCode( pCodeGen, root->right );
        if( root->datatype == TYPE_FLOAT )
        {
            fprintf( fp, "\tDIV.F R%d,R%d", a, b );
            fprintf( fp, "\t;floating point division\n" );
        }
        else
        {
            fprintf( fp, "\tDIV R%d,R%d", a, b );
            fprintf( fp, "\t;integer division\n" );
        }

        result = a;
    }

    return result;
}

/*============================================================================*/
/*  generateLeftChild                                                         */
/*!
    Generate assembly code for the left subchild of a node

    The generateLeftChild function generates the assembly code for the
    left subchild of the node.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateLeftChild( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;

    if( ( pCodeGen != NULL ) &&
        ( root != NULL ) )
    {
        GenerateCode( pCodeGen, root->left );
    }

    return result;
}

/*============================================================================*/
/*  generateChildren                                                          */
/*!
    Generate assembly code for both subchildren of a node

    The generateChildren function generates the assembly code for both the
    subchildren of the node.  Left first, then right.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval -1

==============================================================================*/
static int generateChildren( CodeGen *pCodeGen, struct Node *root )
{
    int result = -1;

    if( ( pCodeGen != NULL ) &&
        ( root != NULL ) )
    {
        GenerateCode( pCodeGen, root->left );
        GenerateCode( pCodeGen, root->right );
    }

    return result;
}

/*============================================================================*/
/*  GetExternal                                                               */
/*!
    Generate assembly code to get an external variable

    The GetExternal function generates the assembly code to access
    an external variable.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval number of the register containing the variable value

==============================================================================*/
static int GetExternal( CodeGen *pCodeGen,
                         struct identEntry *ident,
                         int src,
                         char *comment )
{
    char *modifier;
    FILE *fp;
    int a;
    int result = -1;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( ident != NULL ) &&
        ( comment != NULL ) )
    {
        fp = pCodeGen->fp;

        /* get a register to store the external value */
        a = AllocReg( ident, 1 );
        fprintf( fp, "\tMOV R2, R1\n" );
        fprintf( fp, "\tADD R2, %d\n", ident->offset2 );
        fprintf( fp, "\tLOD R%d,R2", a );
        fprintf( fp, "\t;external value for %s\n", ident->name );

        switch( ident->type )
        {
            case TYPE_INT:
            case TYPE_BOOL:
            case TYPE_CHAR:
                modifier = "";
                break;

            case TYPE_FLOAT:
                modifier = ".F";
                break;

            case TYPE_STRING:
                modifier = ".S";
                break;

            default:
                modifier = "";
                break;
        }

        fprintf( fp, "\tGET%s R%d,R%d", modifier, a, src );

        if( strlen(modifier) != 0 )
        {
            fprintf( fp, "\t;retrieve external variable : %s\n", ident->name );
        }
        else
        {
            fprintf( fp, "\t;retrieve external variable : %s\n", ident->name );
        }

        result = a;
    }

    return result;
}

/*============================================================================*/
/*  SetExternal                                                               */
/*!
    Generate assembly code to set an external variable

    The SetExternal function generates the assembly code to store a value to
    an external variable.

    @param[in]
        pCodeGen
            pointer to the CodeGen object containing the output FILE *

    @param[in]
        root
            pointer to the root node from the parse (sub)tree

    @retval number of the register containing the variable value

==============================================================================*/
static int SetExternal( CodeGen *pCodeGen,
                        struct identEntry *ident,
                        int src,
                        char *comment )
{
    char *modifier;
    FILE *fp;
    int result = -1;
    int dest;

    if( ( pCodeGen != NULL ) &&
        ( pCodeGen->fp != NULL ) &&
        ( ident != NULL ) &&
        ( comment != NULL ) &&
        ( ident->name != NULL ) )
    {
        fp = pCodeGen->fp;

        switch( ident->type )
        {
            case TYPE_INT:
            case TYPE_BOOL:
            case TYPE_CHAR:
                modifier = "";
                break;

            case TYPE_FLOAT:
                modifier = ".F";
                break;

            case TYPE_STRING:
                modifier = ".S";
                break;

            default:
                modifier = "";
                break;
        }

        dest = ident->reg[0];
        if( dest == -1 )
        {
            dest = AllocReg( ident, 0 );
            fprintf(fp, "\tMOV R2,R1\n");
            fprintf( fp, "\tADD R2,%d\n", ident->offset );
            fprintf( fp, "\tLOD R%d,R2\n", dest );
        }

        fprintf( fp, "\tMOV R2,R1\n");
        fprintf( fp, "\tADD R2,%d\n", ident->offset2);
        fprintf( fp, "\tSTR R2,R%d\n", src);
        fprintf( fp,
                "\tSET%s R%d,R%d\t;%s : %s\n",
                modifier,
                dest,
                src,
                comment,
                ident->name );

        result = src;
    }

    return result;
}

/*! @}
 * end of codegen group */
