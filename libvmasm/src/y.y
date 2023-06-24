%{
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
#include <vmasm/asm.h>
#include <vmasm/labels.h>
#include <stdio.h>

/*==============================================================================
        Definitions
==============================================================================*/
#define YACC
//#define DEBUG 1

#ifdef DEBUG
#define TRACE printf("reduce at line %d\n", __LINE__);
#else
#define TRACE
#endif

#define MEMORY pASM->memory
#define POINTER pASM->pointer

/*==============================================================================
        Global Variable Declarations
==============================================================================*/

static uint8_t *instptr;
static tzParseInfo *pParseInfo1;
static tzParseInfo *pParseInfo2;
static tzParseInfo *pParseInfo3;
static tzParseInfo *pParseInfo4;

//int yydebug=1;

/*==============================================================================
        External Function Declarations
==============================================================================*/

extern int yylex();

/*==============================================================================
        External Variable Declarations
==============================================================================*/

extern int yylineno;

/*==============================================================================
        Private Function Declarations
==============================================================================*/

void yyerror(tzASMState *pASM, char *s);
void errmsg(char *msg, int line);

%}

%parse-param {tzASMState *pASM}

%token	REG
%token	DAT
%token	COMMA
%token	LABEL
%token	EOLN
%token  JMP
%token	SIGN
%token	CHAR
%token  FOO
%token	STRING
%token	STRERR
%token	RDUMP
%token  NUM
%token  FLOAT
%token	LOD
%token	STR
%token	MOV
%token	ADD
%token	SUB
%token	MUL
%token	DIV
%token	AND
%token	OR
%token	NOT
%token	SHR
%token	SHL
%token	JZR
%token	JNZ
%token	JNE
%token	JPO
%token	JCA
%token	JNC
%token	CAL
%token	RET
%token	NOP
%token	HLT
%token  EXT
%token  SET
%token  GET
%token  EVS
%token  EVE
%token	RDN
%token	RDC
%token	WRN
%token	WRC
%token  WRF
%token	PSH
%token	POP
%token	CMP
%token  MDUMP
%token  WRS
%token  CSB
%token  ZSB
%token  ASB
%token  ASN
%token  ASS
%token  ASC
%token  WSB
%token  TOI
%token  TOF
%token  DLY
%token  NFY
%token  WFS
%token  STM
%token  CTM
%token  SBL
%token  SBO
%token  SCO
%token  GCO
%token  OFD
%token  CFD
%token  SFD
%token  OPS
%token  CPS
%token  EXE

%%

program	: cmdlist
	;

cmdlist	: cmdlist command
	| command
	;

command	: label smplcmd EOLN

	| label EOLN

	| smplcmd EOLN

	| RDUMP EOLN
            {
                instptr = &(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = HNEXT;
                instptr[2] = HRDUMP;
                INCPOINTER(3);
            }

	| error EOLN
            {
                errmsg("invalid command", yylineno);
            }

	| EOLN
    ;

label	: LABEL
            {
                setLabelAddr( ((tzParseInfo *)&$1)->value.pStrVal,
                              pASM->pointer );
            }
	;

smplcmd	: DAT val
            {
                pParseInfo2 = (tzParseInfo *)&$2;
                storeValue( pParseInfo2,
                            &MEMORY[POINTER],
                            pASM->pointer,
                            yylineno );
                INCPOINTER(pParseInfo2->n);
            }

	| DAT STRING
            {
                INCPOINTER( copystring((tzParseInfo *)&$2, &MEMORY[POINTER]) );
            }

	| DAT STRERR
        	{
                /* bad string declaration */
                errmsg("invalid string literal", yylineno);
			}

	| DAT
            {
                INCPOINTER(1);
            }

	| DAT error
            {
                /* bad data declaration */
                errmsg("invalid DAT instruction", yylineno);
            }

	| instr2 args1
            {
                int n;
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = pParseInfo1->value.op;
                if( pParseInfo2->type == eREGISTER )
                {
                    instptr[0] |= MODE_REG;
                }

                CheckParseInfo( instptr, pParseInfo1, pParseInfo2, yylineno );
                n = pParseInfo1->n + pParseInfo2->n;
                INCPOINTER(n);
			}

	| STR args2
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = &MEMORY[POINTER];
                instptr[0] = pParseInfo1->value.op;
                if( pParseInfo2->type == eREGISTER )
                {
                    instptr[0] |= MODE_REG;
                }
                INCPOINTER(pParseInfo2->n + 1 );
            }

    | NOT REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = &(MEMORY[POINTER]);
                instptr[0] = pParseInfo1->value.op;
                instptr[1] = pParseInfo2->value.regnum;
                INCPOINTER(2);
			}

	| shift REG delim NUM
            {
                uint8_t n;
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                pParseInfo4 = (tzParseInfo *)&$4;
                instptr = &(MEMORY[POINTER]);
                instptr[0] = pParseInfo1->value.op;
                instptr[1] = pParseInfo2->value.regnum;
                n = pParseInfo4->value.ucVal;
                if( n == 0 || n > 31 )
                {
                    errmsg("Invalid shift size", yylineno );
                    exit(1);
                }
                instptr[2] = n;
                INCPOINTER(3);
            }

	| jump arg3
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = &(MEMORY[POINTER]);
                instptr[0] = pParseInfo1->value.op | WORD;
                if( pParseInfo2->type == eREGISTER )
                {
                    instptr[0] |= MODE_REG;
                    INCPOINTER(2);
                }
                else
                {
                    /* move forward three spaces to leave room for the
                       jump instruction and a 16-bit address */
                    INCPOINTER(3);
                }
            }

    | CAL arg3
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = &(MEMORY[POINTER]);
                instptr[0] = pParseInfo1->value.op | WORD;
                if( pParseInfo2->type == eREGISTER )
                {
                    instptr[0] |= MODE_REG;
                    INCPOINTER(2);
                }
                else
                {
                    /* move forward three spaces to leave room for the call
                       instruction and a 16-bit address */
                    INCPOINTER(3);
                }
            }

    | CSB REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = pParseInfo2->value.regnum & 0x0F;
                INCPOINTER(3);
            }

    | EXE REG delim REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                pParseInfo4 = (tzParseInfo *)&$4;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = ( ( pParseInfo2->value.regnum & 0x0F ) << 4 ) +
                             ( pParseInfo4->value.regnum & 0x0F );
                INCPOINTER(3);
            }

    | WSB REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = pParseInfo2->value.regnum & 0x0F;
                INCPOINTER(3);
            }

    | ZSB REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = pParseInfo2->value.regnum & 0x0F;
                INCPOINTER(3);
            }

    | ASN REG delim REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                pParseInfo4 = (tzParseInfo *)&$4;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = ( ( pParseInfo2->value.regnum & 0x0F ) << 4 ) +
                             ( pParseInfo4->value.regnum & 0x0F );
                INCPOINTER(3);
            }

    | ASS REG delim REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                pParseInfo4 = (tzParseInfo *)&$4;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = ( ( pParseInfo2->value.regnum & 0x0F ) << 4 ) +
                             ( pParseInfo4->value.regnum & 0x0F );
                INCPOINTER(3);
            }

    | ASB REG delim REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                pParseInfo4 = (tzParseInfo *)&$4;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = ( ( pParseInfo2->value.regnum & 0x0F ) << 4 ) +
                             ( pParseInfo4->value.regnum & 0x0F );
                INCPOINTER(3);
            }

    | ASC REG delim REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                pParseInfo4 = (tzParseInfo *)&$4;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = ( ( pParseInfo2->value.regnum & 0x0F ) << 4 ) +
                             ( pParseInfo4->value.regnum & 0x0F );
                INCPOINTER(3);
            }

    | OFD REG delim REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                pParseInfo4 = (tzParseInfo *)&$4;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = ( ( pParseInfo2->value.regnum & 0x0F ) << 4 ) +
                             ( pParseInfo4->value.regnum & 0x0F );
                INCPOINTER(3);
            }

    | OFD REG delim CHAR
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                pParseInfo4 = (tzParseInfo *)&$4;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | BYTE;
                instptr[2] = pParseInfo2->value.regnum & 0x0F;
                instptr[3] = pParseInfo4->value.ucVal;
                INCPOINTER(4);
            }

    | CFD REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = pParseInfo2->value.regnum & 0x0F;
                INCPOINTER(3);
            }

    | SFD REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = pParseInfo2->value.regnum & 0x0F;
                INCPOINTER(3);
            }

	| RET
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = pParseInfo1->value.op;
                INCPOINTER(1);
            }

	| NOP
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = pParseInfo1->value.op;
                INCPOINTER(1);
            }

	| HLT
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = pParseInfo1->value.op;
                INCPOINTER(1);
            }

    | MDUMP REG delim NUM
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                pParseInfo4 = (tzParseInfo *)&$4;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = HNEXT;
                instptr[2] = pParseInfo1->value.op | MODE_REG;
                instptr[3] = pParseInfo2->value.regnum & 0x0F;
                CheckParseInfo(&instptr[1], pParseInfo2, pParseInfo4, yylineno);
                storeValue( pParseInfo4, &instptr[4], POINTER+4, yylineno );
                INCPOINTER(pParseInfo4->n+4);
            }

	| RDN REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = pParseInfo2->value.regnum & 0x0F;
                INCPOINTER(3);
            }

    | RDC REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = pParseInfo2->value.regnum & 0x0F;
                INCPOINTER(3);
            }

    | WRS REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = pParseInfo2->value.regnum & 0x0F;
                INCPOINTER(3);
            }

    | WRF REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = pParseInfo2->value.regnum & 0x0F;
                INCPOINTER(3);
            }

    | WRF float
            {
                instptr = &MEMORY[POINTER];
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op;
                CheckParseInfo(&instptr[1], pParseInfo1, pParseInfo2, yylineno);
                storeValue( pParseInfo2, &instptr[2], POINTER+2, yylineno );
                INCPOINTER(pParseInfo2->n+2);
            }

    | WRN REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = pParseInfo2->value.regnum & 0x0F;
                INCPOINTER(3);
            }

	| WRN val
            {
                instptr = &MEMORY[POINTER];
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op;
                CheckParseInfo(&instptr[1], pParseInfo1, pParseInfo2, yylineno);
                if( pParseInfo2->type == eLABEL )
                {
                    enterLabel( pParseInfo2->value.pStrVal, POINTER+2 );
                }
                else
                {
                    storeValue( pParseInfo2, &instptr[2], POINTER+2, yylineno );
                }

                INCPOINTER(pParseInfo2->n+2);
            }

    | WRC REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = pParseInfo2->value.regnum & 0x0F;
                INCPOINTER(3);
            }

	| WRC CHAR
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | BYTE;
                instptr[2] = pParseInfo2->value.ucVal;
                INCPOINTER(3);
            }

	| PSH REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = pParseInfo1->value.op | MODE_REG;
                instptr[1] = pParseInfo2->value.regnum & 0x0F;
                INCPOINTER(2);
            }

	| PSH val
			{
                instptr = &MEMORY[POINTER];
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr[0] = pParseInfo1->value.op;
                CheckParseInfo( instptr, pParseInfo1, pParseInfo2, yylineno );
                if( pParseInfo2->type == eLABEL )
                {
                    enterLabel( pParseInfo2->value.pStrVal, POINTER+1 );
                }
                else
                {
                    storeValue( pParseInfo2, &instptr[1], POINTER+1, yylineno );
                }

                INCPOINTER(pParseInfo2->n+1);
            }

	| POP REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = pParseInfo1->value.op;
                instptr[1] = pParseInfo2->value.regnum & 0x0F;
                INCPOINTER(2);
            }

    | TOF REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = pParseInfo1->value.op;
                instptr[1] = pParseInfo2->value.regnum & 0x0F;
                INCPOINTER(2);
            }

    | TOI REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = pParseInfo1->value.op;
                instptr[1] = pParseInfo2->value.regnum & 0x0F;
                INCPOINTER(2);
            }

    | EXT REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = pParseInfo1->value.op;
                instptr[1] = pParseInfo2->value.regnum & 0x0F;
                INCPOINTER(2);
            }

    | GET REG delim REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                pParseInfo4 = (tzParseInfo *)&$4;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = pParseInfo1->value.op;
                instptr[1] = ( ( ( pParseInfo2->value.regnum & 0x0F ) << 4 ) |
                               ( pParseInfo4->value.regnum & 0x0F ) );
                INCPOINTER(2);
            }

    | SET REG delim REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                pParseInfo4 = (tzParseInfo *)&$4;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = pParseInfo1->value.op;
                instptr[1] = ( ( ( pParseInfo2->value.regnum & 0x0F ) << 4 ) |
                               ( pParseInfo4->value.regnum & 0x0F ) );
                INCPOINTER(2);
            }
    | DLY REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = pParseInfo2->value.regnum & 0x0F;
                INCPOINTER(3);
            }

    | NFY REG delim REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                pParseInfo4 = (tzParseInfo *)&$4;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = ( ( pParseInfo2->value.regnum & 0x0F ) << 4 ) +
                             ( pParseInfo4->value.regnum & 0x0F );
                INCPOINTER(3);
            }

    | WFS REG delim REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                pParseInfo4 = (tzParseInfo *)&$4;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = ( ( pParseInfo2->value.regnum & 0x0F ) << 4 ) +
                             ( pParseInfo4->value.regnum & 0x0F );
                INCPOINTER(3);
            }

    | EVS REG delim REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                pParseInfo4 = (tzParseInfo *)&$4;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = ( ( pParseInfo2->value.regnum & 0x0F ) << 4 ) +
                             ( pParseInfo4->value.regnum & 0x0F );
                INCPOINTER(3);
            }

    | EVE REG delim REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                pParseInfo4 = (tzParseInfo *)&$4;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = ( ( pParseInfo2->value.regnum & 0x0F ) << 4 ) +
                             ( pParseInfo4->value.regnum & 0x0F );
                INCPOINTER(3);
            }

    | STM REG delim REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                pParseInfo4 = (tzParseInfo *)&$4;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = ( ( pParseInfo2->value.regnum & 0x0F ) << 4 ) +
                             ( pParseInfo4->value.regnum & 0x0F );
                INCPOINTER(3);
            }

    | CTM REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = pParseInfo2->value.regnum & 0x0F;
                INCPOINTER(3);
            }
	;

    | SBL REG delim REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                pParseInfo4 = (tzParseInfo *)&$4;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = ( ( pParseInfo2->value.regnum & 0x0F ) << 4 ) +
                             ( pParseInfo4->value.regnum & 0x0F );
                INCPOINTER(3);
            }
    ;

    | SBO REG delim REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                pParseInfo4 = (tzParseInfo *)&$4;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = ( ( pParseInfo2->value.regnum & 0x0F ) << 4 ) +
                             ( pParseInfo4->value.regnum & 0x0F );
                INCPOINTER(3);
            }
    ;

    | OPS REG delim REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                pParseInfo4 = (tzParseInfo *)&$4;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = ( ( pParseInfo2->value.regnum & 0x0F ) << 4 ) +
                             ( pParseInfo4->value.regnum & 0x0F );
                INCPOINTER(3);
            }
    ;

    | CPS REG delim REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                pParseInfo4 = (tzParseInfo *)&$4;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = ( ( pParseInfo2->value.regnum & 0x0F ) << 4 ) +
                             ( pParseInfo4->value.regnum & 0x0F );
                INCPOINTER(3);
            }
    ;

    | SCO REG delim REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                pParseInfo4 = (tzParseInfo *)&$4;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = ( ( pParseInfo2->value.regnum & 0x0F ) << 4 ) +
                             ( pParseInfo4->value.regnum & 0x0F );
                INCPOINTER(3);
            }
    ;

	| SCO REG delim CHAR
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | BYTE;
                instptr[2] = pParseInfo2->value.regnum & 0x0F;
                instptr[3] = pParseInfo4->value.ucVal;
                INCPOINTER(4);
            }

    | GCO REG delim REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo2 = (tzParseInfo *)&$2;
                pParseInfo4 = (tzParseInfo *)&$4;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[0] = HNEXT;
                instptr[1] = pParseInfo1->value.op | MODE_REG;
                instptr[2] = ( ( pParseInfo2->value.regnum & 0x0F ) << 4 ) +
                             ( pParseInfo4->value.regnum & 0x0F );
                INCPOINTER(3);
            }
	;

args1	: REG delim val
            {
                tzParseInfo parseInfo;
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo3 = (tzParseInfo *)&$3;
                instptr = &(MEMORY[POINTER]);
                instptr[1] = pParseInfo1->value.regnum & 0x0F;

                if( pParseInfo3->type == eLABEL )
                {
                    enterLabel( pParseInfo3->value.pStrVal, POINTER+2 );
                }
                else
                {
                    storeValue( pParseInfo3, &instptr[2], POINTER+2, yylineno );
                }

                /* pass some information back to out parent */
                parseInfo.n = pParseInfo1->n + pParseInfo3->n;
                parseInfo.width = pParseInfo3->width;
                parseInfo.type = pParseInfo3->type;
                $$ = parseInfo;
			}

	| REG delim REG
            {
                pParseInfo1 = &$1;
                pParseInfo3 = &$3;
                instptr = &MEMORY[POINTER];
                instptr[1] = ( (( pParseInfo1->value.regnum & 0x0F ) << 4 ) |
                                ( pParseInfo3->value.regnum & 0x0F ) );
                $$ = $1;
			}

	| val delim badarg
            {
                /* invalid target */
				errmsg("invalid target for instruction", yylineno);
            }
	;

args2	: val delim REG
            {
                tzParseInfo parseInfo;
                pParseInfo1 = (tzParseInfo *)&$1;
                pParseInfo3 = (tzParseInfo *)&$3;
                if( pParseInfo1->width != 2 )
                {
                    errmsg("Invalid Address", yylineno );
                    exit(1);
                }

                instptr = &MEMORY[POINTER];
                instptr[1] = pParseInfo3->value.regnum;

                if( pParseInfo1->type == eLABEL )
                {
                    enterLabel( pParseInfo1->value.pStrVal, POINTER+2 );
                }
                else
                {
                    storeValue( pParseInfo1, &instptr[2], POINTER+2, yylineno );
                }

                parseInfo.n = pParseInfo1->n + pParseInfo3->n;
                $$ = parseInfo;
            }

    | REG delim REG
            {
                pParseInfo1 = &$1;
                pParseInfo3 = &$3;
                instptr = &MEMORY[POINTER];
                instptr[1] = ( (( pParseInfo1->value.regnum & 0x0F ) << 4 ) |
                                ( pParseInfo3->value.regnum & 0x0F ) );
                $$ = $1;
            }

    | error val
            {
                /* bad source for instruction */
    			errmsg("invalid source for instruction", yylineno);
			}
    ;

arg3	: REG
            {
                pParseInfo1 = (tzParseInfo *)&$1;
                instptr = (unsigned char *)&(MEMORY[POINTER]);
                instptr[1] = pParseInfo1->value.regnum & 0x0F;
                $$ = $1;
			}

	| LABEL
	       {
                pParseInfo1 = (tzParseInfo *)&$1;
                enterLabel( pParseInfo1->value.pStrVal, POINTER+1 );
                $$ = $1;
			}
	;

badarg	: val
            {
                /* bad argument for instruction */
            }

	| REG
	;

instr2	: LOD
	| MOV
	| ADD
	| SUB
	| MUL
	| DIV
	| AND
	| OR
	| CMP
	;

shift	: SHR
	| SHL
	;

jump	: JMP
	| JZR
	| JNZ
	| JNE
	| JPO
	| JCA
	| JNC
	;

delim	: COMMA
	|
	;

val	: LABEL
            {
                $$ = $1;
            }

	| NUM
            {
                $$ = $1;
            }

	| CHAR
            {
                $$ = $1;
            }
    | FLOAT
            {
                $$ = $1;
            }
	;

float : FLOAT
    {
        $$ = $1;
    }
    ;

%%

/*============================================================================*/
/*  yyerror                                                                   */
/*!
    Handle parser error

    The yyerror function is invoked by the parser when a parse error
    occurs.  It outputs the specified error message and sets the
    error flag in the ASM State object.

    @param[in]
        pASM
            pointer to the tzASMState object containing the global error flag

    @param[out]
        s
            error message to display

==============================================================================*/
void yyerror(tzASMState *pASM, char *s)
{
        fprintf(stderr, "Line: %d: %s\n", yylineno, s);
        pASM->error = 1;
}



/*
	errmsg -- print an error message with the line number.
 */
/*============================================================================*/
/*  errmsg                                                                    */
/*!
    Display error message

    The errmsg function displays an error message to stdout containing
    the specified error message and the line number it occured on.

    @param[in]
        msg
            pointer to the error message

    @param[out]
        line
            line number of the error

==============================================================================*/
void errmsg(char *msg, int line)
{
	fprintf(stderr, "ERROR:  %s at line %d.\n", msg, line);
}

