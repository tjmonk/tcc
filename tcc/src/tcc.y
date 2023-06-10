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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <limits.h>
#include <signal.h>
#include "Registers.h"
#include "SymbolTableManager.h"
#include "codegen.h"
#include "labels.h"
#include "lineno.h"
#include "typecheck.h"
#include "node.h"

/*==============================================================================
        Definitions
==============================================================================*/

#define YYSTYPE struct Node *

/*==============================================================================
        Globals
==============================================================================*/

extern char *yytext;

/* input file for lex */
extern FILE *yyin;

/* output file */
static FILE *fp;

static int stringBufID = 0;
static int scopeLevel = 0;
static int offset = 0;		/* stack offset */
static int size = sizeof(uint32_t);		/* stack element size (return address) */
static int typespec;
static struct Node *program = NULL;
static char ident[128];

/* error flag */
static bool errorFlag = false;

/*==============================================================================
        Function Declarations
==============================================================================*/

int yylex();

void yyerror();

static bool CheckIdent( struct Node *root, char *ident );

%}

%token PROGRAM
%token IF
%token ELSE
%token FOR
%token FOR1
%token FOR2
%token WHILE
%token SWITCH
%token CASE
%token CASE1
%token DEFAULT
%token BREAK
%token RETURN

%token WRITE
%token WRITELN
%token READ
%token READLN
%token APPEND
%token LENGTH
%token CHARAT
%token SETAT
%token SETAT1
%token DELAY
%token SETTIMER
%token CLEARTIMER
%token WAITSIG
%token NOTIFY
%token FILE_OPEN
%token FILE_CLOSE
%token FILE_READ
%token FILE_WRITE
%token OPEN_PRINT_SESSION
%token CLOSE_PRINT_SESSION
%token SYSTEM
%token HANDLE
%token VALIDATE_START
%token VALIDATE_END
%token ID
%token LVAL_ID
%token EXTERN_LVAL_ID
%token SYSVAR_ID
%token VAR_ID
%token PARAM_ID
%token DECL_ID
%token INVOKATION_ID
%token FUNC_ID
%token NUM
%token HEXNUM
%token CHARSTR
%token CHARACTER
%token FLOAT
%token EXTERN

%token TYPE_INT
%token TYPE_FLOAT
%token TYPE_BOOL
%token TYPE_CHAR
%token TYPE_STRING
%token TYPE_INVALID

%token OR
%token AND
%token NOT

%token XOR
%token BOR
%token BAND
%token BNOT

%token EQUALS
%token NOTEQUALS

%token ASSIGN
%token TIMES_EQUALS
%token DIV_EQUALS
%token PLUS_EQUALS
%token MINUS_EQUALS
%token AND_EQUALS
%token OR_EQUALS
%token XOR_EQUALS
%token PLUS
%token MINUS
%token TIMES
%token DIVIDE

%token LPAREN
%token RPAREN
%token LBRACE
%token RBRACE
%token LBRACKET
%token RBRACKET
%token COMMA
%token SEMI
%token DQUOTE
%token DOT
%token COLON

%token LTE
%token GTE
%token LT
%token GT

%token RSHIFT
%token LSHIFT

%token INC
%token DEC
%token TO_INT
%token TO_FLOAT

%token ARG

%token FUNC_HDR
%token FUNC_HDR1
%token DECLN
%token EXTERN_DECLN
%token DECL
%token DECL_LIST
%token EXTERN_DECL_LIST
%token DECLN_LIST
%token ARG_LIST
%token PARAM_LIST
%token PARAMETER
%token FUNC_DEF
%token FUNC_DEF1
%token FUNC_DEF_LIST
%token STAT_LIST
%token COMP_STAT
%token PROC_CALL
%token ARRAY
%token ARRAY_DECL
%token OUTPUT_LIST
%token APPEND_LIST
%token INPUT_LIST

%nonassoc "then"
%nonassoc ELSE

%%
program        :        function_definition_list
			{ program = (struct Node *)createNode(PROGRAM,$1,NULL); }
               ;

function_definition_list
		: 	function_definition function_definition_list
			{
			$$ = (struct Node *)createNode(FUNC_DEF_LIST,$1,$2);
			SetScopeLevel(0);
			}

        | 	{ $$ = NULL; }
        ;

function_definition
		: 	function_header function_header1 compound_statement

			{ $$ = (struct Node *)createNode(FUNC_DEF,$1,
				(struct Node *)createNode(FUNC_DEF1,$2,$3)); }
		;

function_header:    	type_specifier identifier

                        {
			/* insert new function id at scope level 0 */
			scopeLevel = CreateNewScopeLevel();
			$2->type = FUNC_ID;
			$2->ident = (struct identEntry *)InsertID(ident);
			$2->ident->type = typespec;
			$2->ident->scopeID = scopeLevel;
			SetScopeLevel(scopeLevel);

                        $$ = (struct Node *)createNode(FUNC_HDR,$1,$2);
                        }
 		;

function_header1:	LPAREN parameter_list RPAREN
			{
			$$ = $2;
			offset = 0;
			}
		;

statement_list:	statement statement_list
			{ $$ = (struct Node *)createNode(STAT_LIST,$1,$2); }
		|	{ $$ = NULL; }
		;

statement:	expression SEMI
			{ $$ = $1; }
		|	compound_statement
			{ $$ = $1; }
		|	selection_statement
			{ $$ = $1; }
		|   switchcase_statement
			{ $$ = $1; }
		|	iteration_statement
			{ $$ = $1; }
		|	output_statement SEMI
			{ $$ = $1; }
		|	input_statement SEMI
			{ $$ = $1; }
		|	return_statement SEMI
			{ $$ = $1; }
		|	break_statement SEMI
			{ $$ = $1; }
		|   stringbuf_statement SEMI
			{ $$ = $1; }
		|   delay_statement SEMI
			{ $$ = $1; }
		|   settimer_statement SEMI
			{ $$ = $1; }
		|   cleartimer_statement SEMI
			{ $$ = $1; }
		|   waitsignal_statement SEMI
			{ $$ = $1; }
		|   notify_statement SEMI
			{ $$ = $1; }
		|   validate_end_statement SEMI
			{ $$ = $1; }
		|   close_print_session_statement SEMI
			{ $$ = $1; }
		|   file_close_statement SEMI
			{ $$ = $1; }
		|   setat_statement SEMI
			{ $$ = $1; }
		|	SEMI
			{ $$ = NULL; }
		;

compound_statement:
			LBRACE declaration_list statement_list RBRACE
			{
			$$ = (struct Node *)createNode(COMP_STAT,$2,$3);
			offset = 0;
			size = sizeof(uint32_t);  	/* skip return address */
			}
		;

selection_statement
		:	IF LPAREN expression RPAREN statement 	%prec "then"
			{ $$ = (struct Node *)createNode(IF,$3,
    	    	    	    	    (struct Node *)createNode(ELSE,$5,NULL)); }

		|	IF LPAREN expression RPAREN statement ELSE statement
			{ $$ = (struct Node *)createNode(IF,$3,
					(struct Node *)createNode(ELSE,$5,$7)); }
		;

switchcase_statement
		:	SWITCH LPAREN expression RPAREN LBRACE caselist RBRACE
			{ $$ = (struct Node *)createNode(SWITCH, $3, $6); }
		;

caselist
		:   CASE number COLON statement_list caselist
			{ $$ = (struct Node *)createNode( CASE, $2,
					(struct Node *)createNode( CASE1, $4, $5 ));}
		|   CASE character COLON statement_list caselist
			{ $$ = (struct Node *)createNode( CASE, $2,
					(struct Node *)createNode( CASE1, $4, $5 ));}
		|   CASE identifier COLON statement_list caselist
			{ $$ = (struct Node *)createNode( CASE, $2,
					(struct Node *)createNode( CASE1, $4, $5 ));}
		|   DEFAULT COLON statement_list caselist
			{ $$ = (struct Node *)createNode( DEFAULT, $3, $4);}
		|   { $$ = NULL; }
		;

iteration_statement
		:	FOR LPAREN iteration_expression SEMI expression SEMI iteration_expression RPAREN statement
			{ $$ = (struct Node *)createNode(FOR,$3,
					(struct Node *)createNode(FOR1,$5,
						(struct Node *)createNode(FOR2,$9,$7))); }
		|   WHILE LPAREN expression RPAREN statement
			{ $$ = (struct Node *)createNode(WHILE, $5, $3);}
		;

output_statement:	WRITE LPAREN output_list RPAREN
			{ $$ = (struct Node *)createNode(WRITE,$3,NULL); }
		|   FILE_WRITE LPAREN identifier COMMA output_list RPAREN
			{ $$ = (struct Node *)createNode(FILE_WRITE, $3, $5 ); }
		|	WRITELN LPAREN RPAREN
			{ $$ = (struct Node *)createNode(WRITELN,NULL,NULL); }
		;

output_list	:	output COMMA output_list
			{ $$ = (struct Node *)createNode(OUTPUT_LIST,$1,$3); }

		|	output
			{ $$ = (struct Node *)createNode(OUTPUT_LIST,$1,NULL); }
 		;

output		:	identifier
			{ $$ = $1;
			  CheckIdent( $1, ident ); }
		|	number
			{ $$ = $1; }
		|   float
			{ $$ = $1; }
		| 	charstring
			{ $$ = $1; }
		|	character
			{ $$ = $1; }
		|	{ $$ = NULL; }
		;

append_list	:	append COMMA append_list
			{ $$ = (struct Node *)createNode(APPEND_LIST,$1,$3); }

		|	append
			{ $$ = (struct Node *)createNode(APPEND_LIST,$1,NULL); }
 		;

append	:	identifier
			{ $$ = $1;
			  CheckIdent( $1, ident ); }
		|	number
			{ $$ = $1; }
		|   float
			{ $$ = $1; }
		| 	charstring
			{ $$ = $1; }
		|	character
			{ $$ = $1; }
		|	{ $$ = NULL; }
		;

input_statement :	READ LPAREN input_list RPAREN
			{ $$ = (struct Node *)createNode(READ,$3,NULL); }
			|   FILE_READ LPAREN identifier COMMA input_list RPAREN
			{ $$ = (struct Node *)createNode(FILE_READ,$3,$5); }
	       	|	READLN LPAREN RPAREN
			{ $$ = (struct Node *)createNode(READLN,NULL,NULL); }
		;

input_list	:	input COMMA input_list
			{ $$ = (struct Node *)createNode(INPUT_LIST,$1,$3); }

		|	input
			{ $$ = (struct Node *)createNode(INPUT_LIST,$1,NULL); }
 		;

input		:	identifier
			{
				$$ = $1;
			  	CheckIdent( $1, ident );
			}
		;

return_statement:	RETURN LPAREN expression RPAREN
			{
			$$ = (struct Node *)createNode(RETURN,$3,NULL);
			}

		|	RETURN LPAREN RPAREN
			{ $$ = (struct Node *)createNode(RETURN,NULL,NULL); }
		;

delay_statement: DELAY LPAREN expression RPAREN
			{
				if( TypeCheck( $3, 0, false ) == TYPE_INT )
				{
					$$ = (struct Node *)createNode( DELAY, $3, NULL );
				}
				else
				{
					fprintf(stderr, "E: Invalid argument to delay on line %d\n", getlineno() + 1 );
					errorFlag = true;
				}
			}
		;

settimer_statement: SETTIMER LPAREN expression COMMA expression RPAREN
			{
				int type1 = TypeCheck( $3, 0, false );
				int type2 = TypeCheck( $5, 0, false );
				if( ( (type1 == TYPE_INT) || (type1 == TYPE_CHAR) ) &&
					( (type2 == TYPE_INT) || (type2 == TYPE_CHAR) ) )
				{
					$$ = (struct Node *)createNode( SETTIMER, $3, $5 );
				}
				else
				{
					fprintf(stderr, "E: Invalid arguments to set_timer on line %d\n", getlineno() + 1 );
					errorFlag = true;
				}
			}
		;

cleartimer_statement: CLEARTIMER LPAREN expression RPAREN
			{
				if( TypeCheck( $3, 0, false ) == TYPE_INT )
				{
					$$ = (struct Node *)createNode( CLEARTIMER, $3, NULL );
				}
				else
				{
					fprintf(stderr, "E: Invalid argument to clear_timer on line %d\n", getlineno() + 1 );
					errorFlag = true;
				}
			}
		;

waitsignal_statement: WAITSIG LPAREN BAND lval_expression COMMA BAND lval_expression RPAREN
			{
				if( ( TypeCheck($4, 0, false ) == TYPE_INT ) &&
					( TypeCheck($7, 0, false ) == TYPE_INT ) )
				{
					$$ = (struct Node *)createNode( WAITSIG, $4, $7 );
				}
				else
				{
					fprintf(stderr, "E: Invalid arguments to wait_sig on line %d\n", getlineno() + 1 );
					errorFlag = true;
				}
			}
		;

notify_statement: NOTIFY LPAREN identifier COMMA expression RPAREN
			{
				$$ = (struct Node *)createNode( NOTIFY, $3, $5 );
			}
		;

validate_end_statement: VALIDATE_END LPAREN identifier COMMA expression RPAREN
			{
				$$ = (struct Node *)createNode( VALIDATE_END, $3, $5 );
			}
		;

close_print_session_statement: CLOSE_PRINT_SESSION LPAREN identifier COMMA identifier RPAREN
			{
				$$ = (struct Node *)createNode( CLOSE_PRINT_SESSION, $3, $5 );
				if( $3->ident->type != TYPE_INT )
				{
					fprintf( stderr,
							 "E: Invalid type for argument 1 of close_print_session on line %d\n",
							 getlineno() + 1 );
				}

				if( $5->ident->type != TYPE_INT )
				{
					fprintf( stderr,
					         "E: Invalid type for argument 2 of close_print_session on line %d\n",
							 getlineno() + 1 );
				}

			}
		;

file_close_statement: FILE_CLOSE LPAREN expression RPAREN
			{
				$$ = (struct Node *)createNode( FILE_CLOSE, $3, NULL );
			}
		;

break_statement:	BREAK
			{
				$$ = (struct Node *)createNode(BREAK, NULL, NULL );
			}
		;

stringbuf_statement:
		identifier DOT APPEND LPAREN append_list RPAREN
			{
				CheckIdent( $1, ident );
				$$ = (struct Node *)createNode(APPEND,$1,$5);
			}
		;

setat_statement:
		identifier SETAT LPAREN expression COMMA expression RPAREN
			{
				CheckIdent( $1, ident );
				$$ = (struct Node *)createNode(SETAT,
							(struct Node *)createNode(SETAT1,$1,$4), $6 );
			}
		;

iteration_expression
		:	expression
			{ $$ = $1; }
		|	{ $$ = NULL; }
		;

expression	:	assignment_expression
			{
			$$ = $1;
			TypeCheck($1, 0, false);
			}
		;

assignment_expression
		:	logical_OR_expression
		|	lval_expression assignment_operator assignment_expression
			{
				updateNode( $2, $1, $3 );
				$$ = $2;
			}
		;

lval_expression	:	identifier
			{
				if( CheckIdent( $1, ident ) == true )
				{
					if( $1->ident->isExternal == true )
					{
						$1->type = EXTERN_LVAL_ID;
					}
					else
					{
						$1->type = LVAL_ID;
					}
				}
				$$ = $1;
			}

		|	identifier LBRACKET expression RBRACKET
			{
				if( CheckIdent( $1, ident ) == true )
				{
					$1->type = LVAL_ID;
					$$ = (struct Node *)createNode(ARRAY,$1,$3);
				}
			}
		;

logical_OR_expression
		:	logical_AND_expression
		|	logical_OR_expression OR logical_AND_expression
			{ $$ = (struct Node *)createNode(OR,$1,$3); }
		;

logical_AND_expression
		: inclusive_OR_expression
		|	logical_AND_expression AND inclusive_OR_expression
			{ $$ = (struct Node *)createNode(AND,$1,$3); }
		;

inclusive_OR_expression
		: exclusive_OR_expression
		|	inclusive_OR_expression BOR exclusive_OR_expression
			{ $$ = (struct Node *)createNode(BOR,$1,$3); }
		;

exclusive_OR_expression
		:	AND_expression
		|	exclusive_OR_expression XOR AND_expression
			{ $$ = (struct Node *)createNode(XOR,$1,$3); }
		;

AND_expression  :	equality_expression
		|	AND_expression BAND equality_expression
			{ $$ = (struct Node *)createNode(BAND,$1,$3); }
		;

equality_expression
		:	relational_expression
		|	equality_expression EQUALS relational_expression
			{
				$$ = (struct Node *)createNode(EQUALS,$1,$3);
				TypeCheck($$, 0, false);
			}
		|	equality_expression NOTEQUALS relational_expression
			{
				$$ = (struct Node *)createNode(NOTEQUALS,$1,$3);
				TypeCheck($$, 0, false);
			}
		;

relational_expression
		:	shift_expression
		|	relational_expression LT shift_expression
			{
				$$ = (struct Node *)createNode(LT,$1,$3);
			  	TypeCheck($$, 0, false);
		  	}
		|	relational_expression GT shift_expression
			{
				$$ = (struct Node *)createNode(GT,$1,$3);
			  	TypeCheck($$, 0, false);
		  	}
		|	relational_expression LTE shift_expression
			{
 				$$ = (struct Node *)createNode(LTE,$1,$3);
			  	TypeCheck($$, 0, false);
		  	}
		|	relational_expression GTE shift_expression
			{
				$$ = (struct Node *)createNode(GTE,$1,$3);
			  	TypeCheck($$, 0, false);
		  	}

		;

shift_expression:	additive_expression
		|	shift_expression LSHIFT additive_expression
			{ $$ = (struct Node *)createNode(LSHIFT,$1,$3); }
		|	shift_expression RSHIFT additive_expression
			{ $$ = (struct Node *)createNode(RSHIFT,$1,$3); }
		;

additive_expression
		:	multiplicative_expression
		|	additive_expression PLUS multiplicative_expression
			{ $$ = (struct Node *)createNode(PLUS,$1,$3); }
		|	additive_expression MINUS multiplicative_expression
			{ $$ = (struct Node *)createNode(MINUS,$1,$3); }
		;

multiplicative_expression
		:
			unary_expression
		|	multiplicative_expression TIMES unary_expression
			{ $$ = (struct Node *)createNode(TIMES,$1,$3); }
		|	multiplicative_expression DIVIDE unary_expression
			{ $$ = (struct Node *)createNode(DIVIDE,$1,$3); }
		;

unary_expression:	typecast_expression
		|	INC identifier
			{
			  $$ = (struct Node *)createNode(INC,NULL,$2);
			  CheckIdent( $2, ident );
			}
		|	DEC identifier
			{
			  $$ = (struct Node *)createNode(DEC,NULL,$2);
			  CheckIdent( $2, ident );
			}
		|	MINUS unary_expression
			{ $$ = (struct Node *)createNode(MINUS,NULL,$2); }
		|   NOT unary_expression
			{ $$ = (struct Node *)createNode(NOT,NULL,$2); }
		|   BNOT unary_expression
			{ $$ = (struct Node *)createNode(BNOT,NULL,$2); }
		;

typecast_expression: postfix_expression
		|	float_cast number
			{ $$ = (struct Node *)createNode(TO_FLOAT,$2, NULL); }
		|   float_cast identifier
			{
				$$ = (struct Node *)createNode(TO_FLOAT,$2, NULL);
				CheckIdent( $2, ident );
			}
		|   int_cast float
			{ $$ = (struct Node *)createNode(TO_INT,$2,NULL); }
		|   int_cast identifier
			{
				$$ = (struct Node *)createNode(TO_INT,$2,NULL);
				CheckIdent( $2, ident );
			}
		;

float_cast:	LPAREN TYPE_FLOAT RPAREN
		;

int_cast : LPAREN TYPE_INT RPAREN
		;

postfix_expression
		:	primary_expression
			{ $$ = $1; }

		|	identifier LBRACKET expression RBRACKET
			{
				CheckIdent( $1, ident );
				$$ = (struct Node *)createNode(ARRAY,$1,$3);
				$1->type = LVAL_ID;
			}

		|	identifier LPAREN argument_list RPAREN
			{
				CheckIdent( $1, ident );
				$1->type = INVOKATION_ID;
				$$ = (struct Node *)createNode(PROC_CALL,$1,$3);
			}

		|   HANDLE LPAREN identifier RPAREN
			{
				CheckIdent( $3, ident );
				$$ = (struct Node *)createNode(HANDLE, NULL, $3 );
			}

		|   identifier LENGTH LPAREN RPAREN
			{
				CheckIdent( $1, ident );
				$$ = (struct Node *)createNode(LENGTH, $1, NULL );
			}

		|	identifier CHARAT LPAREN expression RPAREN
			{
				CheckIdent( $1, ident );
				$$ = (struct Node *)createNode(CHARAT, $1, $4 );
			}

		|   VALIDATE_START LPAREN identifier RPAREN
			{
				$$ = (struct Node *)createNode(VALIDATE_START, NULL, $3 );
			}

		|   OPEN_PRINT_SESSION LPAREN identifier COMMA BAND identifier RPAREN
			{
				$$ = (struct Node *)createNode( OPEN_PRINT_SESSION, $3, $6 );
				if( $3->ident->type != TYPE_INT )
				{
					fprintf( stderr,
							 "E: Invalid type for argument 1 of open_print_session on line %d\n",
							 getlineno() + 1 );
				}

				if( $6->ident->type != TYPE_INT )
				{
					fprintf( stderr,
							 "E: Invalid type for argument 2 of open_print_session on line %d\n",
							 getlineno() + 1 );
				}
			}

		|   SYSTEM LPAREN expression RPAREN
			{
				$$ = (struct Node *)createNode( SYSTEM, $3, NULL );
				if( $3->type == ID )
				{
					if( $3->ident->type != TYPE_STRING )
					{
						fprintf(stderr, "E: Invalid argument to system on line %d\n", getlineno() + 1 );
						errorFlag = true;
					}
				}
				else if( $3->type != CHARSTR )
				{
					fprintf(stderr, "E: Invalid argument to system on line %d\n", getlineno() + 1 );
					errorFlag = true;
				}
			}

		|   FILE_OPEN LPAREN expression COMMA expression RPAREN
			{
				$$ = (struct Node *)createNode( FILE_OPEN, $3, $5 );
				if( $3->type == ID )
				{
					if( $3->ident->type != TYPE_STRING )
					{
						fprintf(stderr, "E: Invalid filename argument to file_open on line %d\n", getlineno() + 1 );
						errorFlag = true;
					}
				}
				else if( $3->type != CHARSTR )
				{
					fprintf(stderr, "E: Invalid filename argument to file_open on line %d\n", getlineno() + 1 );
					errorFlag = true;
				}

				if( $5->type == ID )
				{
					if( $5->ident->type != TYPE_CHAR )
					{
						fprintf(stderr, "E: Invalid mode argument to file_open on line %d\n", getlineno() + 1 );
						errorFlag = true;
					}
				}
				else if( $5->type != CHARACTER )
				{
					fprintf(stderr, "type = %d\n", $5->type );
					fprintf(stderr, "E: Invalid mode argument to file_open on line %d\n", getlineno() + 1 );
					errorFlag = true;
				}
			}

		|	identifier INC
			{
				CheckIdent( $1, ident );
				$$ = (struct Node *)createNode(INC,$1,NULL);
			}

		|	identifier DEC
			{
				CheckIdent( $1, ident );
				$$ = (struct Node *)createNode(DEC,$1,NULL);
			}
		;

primary_expression
		:	identifier
			{
				CheckIdent( $1, ident );
				$$ = $1;
			}
		|	LPAREN expression RPAREN
			{ $$ = $2; }
		|	number
			{ $$ = $1; }
		|   float
			{ $$ = $1; }
		|	charstring
			{ $$ = $1; }
		|	character
			{ $$ = $1; }
		;

extern_specifier : TYPE_INT
			{
			$$ = (struct Node *)createNode(TYPE_INT,NULL,NULL);
			typespec = TYPE_INT;
			}
		| TYPE_FLOAT
			{
			$$ = (struct Node *)createNode(TYPE_FLOAT,NULL,NULL);
			typespec = TYPE_FLOAT;
			}
		| TYPE_STRING
			{
			$$ = (struct Node *)createNode(TYPE_STRING,NULL,NULL);
			typespec = TYPE_STRING;
			}
		;

type_specifier 	:	TYPE_INT
			{
			$$ = (struct Node *)createNode(TYPE_INT,NULL,NULL);
			typespec = TYPE_INT;
			}
		|   TYPE_FLOAT
			{
			$$ = (struct Node *)createNode(TYPE_FLOAT,NULL,NULL);
			typespec = TYPE_FLOAT;
			}
		| 	TYPE_BOOL
			{
			$$ = (struct Node *)createNode(TYPE_BOOL,NULL,NULL);
			typespec = TYPE_BOOL;
			}

		| 	TYPE_CHAR
			{
			$$ = (struct Node *)createNode(TYPE_CHAR,NULL,NULL);
			typespec = TYPE_CHAR;
			}

		| 	TYPE_STRING
			{
			$$ = (struct Node *)createNode(TYPE_STRING,NULL,NULL);
			typespec = TYPE_STRING;
			}
		;

assignment_operator
		:	ASSIGN		{ $$=(struct Node *)createNode( ASSIGN, NULL, NULL ); }
		|	TIMES_EQUALS 	{ $$=(struct Node *)createNode( TIMES_EQUALS, NULL, NULL ); }
		|	DIV_EQUALS 	{ $$=(struct Node *)createNode( DIV_EQUALS, NULL, NULL ); }
		|	PLUS_EQUALS 	{ $$=(struct Node *)createNode( PLUS_EQUALS, NULL, NULL ); }
		|	MINUS_EQUALS 	{ $$=(struct Node *)createNode( MINUS_EQUALS, NULL, NULL );  }
		|	AND_EQUALS 	{ $$=(struct Node *)createNode( AND_EQUALS, NULL, NULL );  }
		|	OR_EQUALS 	{ $$=(struct Node *)createNode( OR_EQUALS, NULL, NULL ); }
		|	XOR_EQUALS 	{ $$=(struct Node *)createNode( XOR_EQUALS, NULL, NULL );  }
		;

extern_declarator: identifier ASSIGN number
				{
					$$ =  (struct Node *)createNode(ASSIGN,$1,$3);
					$1->ident = (struct identEntry *)InsertID(ident);
					$1->ident->isExternal = true;
					$1->ident->type = typespec;
					TypeCheck( $$, 0, false );
					$1->type = DECL_ID;
					offset -= size;
					$1->ident->offset = offset;
					size = sizeof(uint32_t);
					$1->ident->size = size;
					offset -= sizeof( uint32_t );
					$1->ident->offset2 =offset;
					if ( $1->ident->type == TYPE_STRING )
					{
						$1->ident->stringBufID = ++stringBufID;
					}
					#ifdef SHOW_STACK_INFO
						printf("declarator: %s | offset: %d | size: %d\n",
								$1->ident->name,
								$1->ident->offset,
								$1->ident->size);
					#endif
				}

				| identifier ASSIGN float
				{
					$$ =  (struct Node *)createNode(ASSIGN,$1,$3);
					$1->ident = (struct identEntry *)InsertID(ident);
					$1->ident->isExternal = true;
					$1->ident->type = typespec;
					TypeCheck( $$, 0, false );
					$1->type = DECL_ID;
					offset -= size;
					$1->ident->offset = offset;
					size = sizeof(uint32_t);
					$1->ident->size = size;
					offset -= sizeof( uint32_t );
					$1->ident->offset2 = offset;

					if ( $1->ident->type == TYPE_STRING )
					{
						$1->ident->stringBufID = ++stringBufID;
					}
					#ifdef SHOW_STACK_INFO
						printf("declarator: %s | offset: %d | size: %d\n",
								$1->ident->name,
								$1->ident->offset,
								$1->ident->size);
					#endif
				}
				| identifier
				{
					$$ = $1;
					$1->ident = (struct identEntry *)InsertID(ident);
					$1->ident->isExternal = true;
					$1->ident->type = typespec;
					$1->type = DECL_ID;
					offset -= size;
					$1->ident->offset = offset;
					size = sizeof(uint32_t);
					$1->ident->size = size;
					offset -= sizeof( uint32_t );
					$1->ident->offset2 = offset;

					if ( $1->ident->type == TYPE_STRING )
					{
						$1->ident->stringBufID = ++stringBufID;
					}

					#ifdef SHOW_STACK_INFO
						printf("declarator: %s | offset: %d | size: %d\n",
							$1->ident->name,
							$1->ident->offset,
							$1->ident->size);
					#endif
				}
				;

declarator	:	identifier ASSIGN number
			{
			$$ =  (struct Node *)createNode(ASSIGN,$1,$3);
			$1->ident = (struct identEntry *)InsertID(ident);
			$1->ident->type = typespec;
			TypeCheck( $$, 0, false );
			$1->type = DECL_ID;
			offset -= size;
			$1->ident->offset = offset;
			size = sizeof(uint32_t);
			$1->ident->size = size;
			if ( $1->ident->type == TYPE_STRING )
			{
				$1->ident->stringBufID = ++stringBufID;
			}
			#ifdef SHOW_STACK_INFO
				printf("declarator: %s | offset: %d | size: %d\n",
						$1->ident->name,
						$1->ident->offset,
						$1->ident->size);
			#endif
			}

		|   identifier ASSIGN character
			{
			$$ =  (struct Node *)createNode(ASSIGN,$1,$3);
			$1->ident = (struct identEntry *)InsertID(ident);
			$1->ident->type = typespec;
			TypeCheck( $$, 0, false );
			$1->type = DECL_ID;
			offset -= size;
			$1->ident->offset = offset;
			size = sizeof(uint32_t);
			$1->ident->size = size;
			if ( $1->ident->type == TYPE_STRING )
			{
				$1->ident->stringBufID = ++stringBufID;
			}
			#ifdef SHOW_STACK_INFO
				printf("declarator: %s | offset: %d | size: %d\n",
						$1->ident->name,
						$1->ident->offset,
						$1->ident->size);
			#endif
			}

		|   identifier ASSIGN float
			{
			$$ =  (struct Node *)createNode(ASSIGN,$1,$3);
			$1->ident = (struct identEntry *)InsertID(ident);
			$1->ident->type = typespec;
			TypeCheck( $$, 0, false );
			$1->type = DECL_ID;
			offset -= size;
			$1->ident->offset = offset;
			size = sizeof(uint32_t);
			$1->ident->size = size;
			if ( $1->ident->type == TYPE_STRING )
			{
				$1->ident->stringBufID = ++stringBufID;
			}
			#ifdef SHOW_STACK_INFO
				printf("declarator: %s | offset: %d | size: %d\n",
						$1->ident->name,
						$1->ident->offset,
						$1->ident->size);
			#endif
			}

		|	identifier LBRACKET number RBRACKET
			{
			$$ = (struct Node *)createNode(ARRAY_DECL,$1,$3);
			$1->ident = (struct identEntry *)InsertID(ident);
			$1->ident->type = typespec;
			$1->type = DECL_ID;
			offset -= size;
			$1->ident->offset = offset;
			size = ($3->value) * sizeof(uint32_t);
			$1->ident->size = size;
			#ifdef SHOW_STACK_INFO
				printf("declarator: %s | offset: %d | size %d\n",
					$1->ident->name,
					$1->ident->offset,
					$1->ident->size);
			#endif
			}

		|	identifier
			{
			$$ = $1;
			$1->ident = (struct identEntry *)InsertID(ident);
			$1->ident->type = typespec;
			$1->type = DECL_ID;
			offset -= size;
			$1->ident->offset = offset;
			size = sizeof(uint32_t);
			$1->ident->size = size;
			if ( $1->ident->type == TYPE_STRING )
			{
				$1->ident->stringBufID = ++stringBufID;
			}

			#ifdef SHOW_STACK_INFO
				printf("declarator: %s | offset: %d | size: %d\n",
					$1->ident->name,
					$1->ident->offset,
					$1->ident->size);
			#endif
			}
		;

extern_declarator_list : extern_declarator COMMA extern_declarator_list
						{
							$$ = (struct Node *)createNode(EXTERN_DECL_LIST,$1,$3);
						}
						| extern_declarator
						{
							$$ = (struct Node *)createNode(EXTERN_DECL_LIST,$1,NULL);
						}
						;

declarator_list	:	declarator COMMA declarator_list
			{ $$ = (struct Node *)createNode(DECL_LIST,$1,$3); }

     		|	declarator
			{ $$ = (struct Node *)createNode(DECL_LIST,$1,NULL); }
		;

declaration 	:	type_specifier declarator_list
			{ $$ = (struct Node *)createNode(DECLN,$1,$2); }
		| EXTERN extern_specifier extern_declarator_list
			{ $$ = (struct Node *)createNode(EXTERN_DECLN, $2, $3);}
		;

declaration_list:	declaration SEMI declaration_list
			{ $$ = (struct Node *)createNode(DECLN_LIST,$1,$3); }
		|	{ $$ = NULL; }
		;

argument	:	expression
		|	{ $$ = NULL; }
		;

argument_list	:	argument COMMA argument_list
			{ $$ = (struct Node *)createNode(ARG_LIST,$1,$3); }

		|	argument
			{ $$ = (struct Node *)createNode(ARG_LIST,$1,NULL); }

		;

parameter	:	type_specifier identifier
			{
			offset += sizeof(uint32_t);
			$2->type = PARAM_ID;
			$2->ident = (struct identEntry *)InsertID(ident);
			$2->ident->type = typespec;
			$2->ident->offset = (2*sizeof(uint32_t))+offset;
			$$ = (struct Node *)createNode(PARAMETER,$1,$2);
			#ifdef SHOW_STACK_INFO
				printf("; parameter: %s | offset: %d\n",
					$2->ident->name,
					$2->ident->offset);
			#endif
			}

		|	{ $$ = NULL; }
		;

parameter_list	:	parameter
			{ $$ = (struct Node *)createNode(PARAM_LIST,$1,NULL); }

		|	parameter_list COMMA parameter
			{ $$ = (struct Node *)createNode(PARAM_LIST,$1,$3); }
 		;

identifier	:	ID
			{
				$$ = (struct Node *)createNode(ID,NULL,NULL);
				$$->ident = (struct identEntry *)LookupID(yytext, true);
				strcpy(ident,yytext);
				if ($$->ident == NULL)
				{
					/* look for procedure id (lvl 0) */
					scopeLevel = GetScopeLevel();
					SetScopeLevel(0);
					$$->ident = (struct identEntry *)LookupID(yytext, true);
					SetScopeLevel(scopeLevel);
				}
			}
		;

float       :   FLOAT
			{
			$$ = (struct Node *)createNode(FLOAT,NULL,NULL);
			$$->fvalue = atof(yytext);
			}
			;

number		:	NUM
			{
			$$ = (struct Node *)createNode(NUM,NULL,NULL);
			$$->value = atoi(yytext);
			}
			|   HEXNUM
			{
			$$ = (struct Node *)createNode(NUM,NULL,NULL);
			$$->value = strtoul(yytext+2, NULL, 16);
			}
		;

charstring	:	CHARSTR
			{
			$$ = (struct Node *)createNode(CHARSTR,NULL,NULL);
			$$->ident = (struct identEntry *)InsertID(yytext);
			}
		;

character : CHARACTER
			{
			$$ = (struct Node *)createNode(CHARACTER,NULL,NULL);
			$$->ident = (struct identEntry *)InsertID(yytext);
			}
		;
%%

/*==============================================================================
        Includes
==============================================================================*/

#include <stdio.h>

/*==============================================================================
        Function Definitions
==============================================================================*/

/*============================================================================*/
/*  yyerror                                                                   */
/*!
    Handle Parser Error

	The yyerror function displays an error message with the line number on
	which the error occurred.

==============================================================================*/
void yyerror()
{
    printf("syntax error at line %d\n",getlineno() + 1);
    errorFlag = true;
}

/*============================================================================*/
/*  CheckIdent                                                                */
/*!
    Check a node to make sure it contains an identifier

	The CheckIdent function checks if the specified Node is associated
    with an identifier.

    @param[in]
        root
            pointer to the Node to check

    @param[in]
        ident
            pointer to an identifier

    @retval true identifier is ok
    @retval false identifier not found

==============================================================================*/
static bool CheckIdent( struct Node *root, char *ident )
{
	bool identOK = false;

	if( root != NULL )
	{
		if( root->ident != NULL )
		{
			identOK = true;
		}
		else
		{
			fprintf( stderr,
					 "E: unknown identifier '%s' on line %d\n",
					 ident,
					 getlineno() );
			errorFlag = true;
		}
	}

	return identOK;
}

/*============================================================================*/
/*  main                                                                      */
/*!
    tcc main entry point

	The main function is the main entry point for the tcc compiler.

    @param[in]
        argC
            number of arguments (including the command itself)

    @param[in]
        argV
            pointer to the argument vector.  argV[0] is the command itself.
            argV[1] is the first argument, etc.


    @retval -1 an error occurred
    @retval 0 parsing successful

==============================================================================*/
int main(int argC, char *argV[])
{
    bool output_tree = false;
    int c;
    int errflag = 0;
    char *filename;
    char *output_filename = (char *)NULL;
	CodeGen *pCodeGen = NULL;

    if( argC < 2)
    {
        fprintf(stderr,
                "usage: %s [-t] [-o <outputfile>] <sourcefile>\n"
                "\t-t output parse tree\n"
                "\t-o output file\n",
                argV[0]);
        return -1;
    }

    /* parse the command line options */
    while( ( c = getopt( argC, argV, "to:" ) ) != -1 )
    {
        switch( c )
        {
            case 't':
                output_tree = true;
                break;

            case 'o':
                output_filename = optarg;
                break;

            case '?':
                ++errflag;
                break;

            default:
                break;
        }
    }

    filename = argV[argC - 1];
    if ( filename != (char *)NULL )
    {
        /* input file was specified */
        if ((yyin = fopen(filename, "r")) == (FILE *)NULL)
        {
            fprintf(stderr, "file %s not found.\n", filename);
            return -1;
        }
    }

    if( output_filename != NULL )
    {
        fp = fopen( output_filename, "w" );
        if( fp == (FILE *)NULL )
        {
            fprintf(stderr, "Unable to create file: %s\n", output_filename);
            return -1;
        }
    }
    else
    {
        fp = stdout;
    }


	/* initialize the symbol table */
    InitSymbolTable();

	/* insert global constants */
	InsertConstant( "SIG_TIMER", TYPE_INT, SIGRTMIN+5 );
	InsertConstant( "NOTIFY_MODIFIED", TYPE_INT, 1 );
	InsertConstant( "NOTIFY_CALC", TYPE_INT, 2 );
	InsertConstant( "NOTIFY_VALIDATE", TYPE_INT, 3 );
	InsertConstant( "NOTIFY_PRINT", TYPE_INT, 4 );
	InsertConstant( "SIG_VAR_MODIFIED", TYPE_INT, SIGRTMIN+6 );
	InsertConstant( "SIG_VAR_CALC", TYPE_INT, SIGRTMIN+7 );
	InsertConstant( "SIG_VAR_VALIDATE", TYPE_INT, SIGRTMIN+8 );
	InsertConstant( "SIG_VAR_PRINT", TYPE_INT, SIGRTMIN+9 );

	/* parse the input file */
    yyparse();

	/* output the tree if requested */
    if( output_tree == true )
    {
        parseTree( program, 0 );
		printf("\n");
    }

	/* initialize the code generator */
	pCodeGen = codegen_init( fp );
	if( pCodeGen != NULL )
	{
		if( ( errorFlag == false ) &&
			( TypeErrorDetected() == false ) )
		{
			/* generate the output code */
			GenerateCode( pCodeGen, program);

			/* generate the support code */
			OutputSupportCode( pCodeGen );
		}

		free( pCodeGen );
	}

    return 0;
}

