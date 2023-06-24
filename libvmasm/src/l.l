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

/* This is the lexical analyser for the virtual machine. */

/*==============================================================================
        Includes
==============================================================================*/

#include <vmasm/asm.h>
#include <vmcore/core.h>
#include "y.h"  /* include token codes from the parser */
#include <vmasm/parseinfo.h>

/*==============================================================================
        External Variables
==============================================================================*/

extern tzParseInfo yylval;

/*==============================================================================
        Definitions
==============================================================================*/

//#define YY_NO_INPUT

%}

%option nounput
%option yylineno

%a 4000
%o 4000

/* regular definitions */

delim			[ \t]
eoln			[\n]
ws				{delim}+
letter			[A-Za-z\_]
digit			[0-9]
label			({letter}|{digit})*
number			[-+]?{digit}+
hexnum			0[xX][0-9a-fA-F]+
comment			[;].*$
sign			[+|-]
float           [-+]?[0-9]*\.[0-9]+([eE][-+]?[0-9]+)?
character		'((\\?.)|\\({digit}+|{hexnum}))'
string			([\"].*[\"])
strerr			(\"[^\"\n]*)$
register		[rR]{digit}+

%%
{ws}			{ /* no action and no return */ }
{comment}		{ /* no action and no return */ }
{eoln}			{ return EOLN;	}

[dD][aA][tT]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HDAT); return DAT ; }
[dD][lL][yY]    { yylval = EncodeOp(yytext, yyleng, yylineno, HDLY); return DLY ; }
[lL][oO][dD](\.[l|L|w|W|b|B])?	{ yylval = EncodeOp(yytext, yyleng, yylineno, HLOD); return LOD; }
[sS][tT][rR](\.[l|L|w|W|b|B])?	{ yylval = EncodeOp(yytext, yyleng, yylineno, HSTR); return STR; }
[mM][oO][vV](\.[l|L|w|W|b|B|f|F])?	{ yylval = EncodeOp(yytext, yyleng, yylineno, HMOV); return MOV; }
[aA][dD][dD](\.[l|L|w|W|b|B|f|F])?	{ yylval = EncodeOp(yytext, yyleng, yylineno, HADD); return ADD; }
[sS][uU][bB](\.[l|L|w|W|b|B|f|F])?	{ yylval = EncodeOp(yytext, yyleng, yylineno, HSUB); return SUB; }
[mM][uU][lL](\.[l|L|w|W|b|B|f|F])?	{ yylval = EncodeOp(yytext, yyleng, yylineno, HMUL); return MUL; }
[dD][iI][vV](\.[l|L|w|W|b|B|f|F])?	{ yylval = EncodeOp(yytext, yyleng, yylineno, HDIV); return DIV; }
[aA][nN][dD](\.[l|L|w|W|b|B])?	{ yylval = EncodeOp(yytext, yyleng, yylineno, HAND); return AND; }
[oO][rR](\.[l|L|w|W|b|B])?	{ yylval = EncodeOp(yytext, yyleng, yylineno, HOR); return OR; }
[nN][oO][tT](\.[l|L|w|W|b|B])?	{ yylval = EncodeOp(yytext, yyleng, yylineno, HNOT); return NOT; }
[sS][hH][rR](\.[l|L|w|W|b|B])?	{ yylval = EncodeOp(yytext, yyleng, yylineno, HSHR); return SHR; }
[sS][hH][lL](\.[l|L|w|W|b|B])?	{ yylval = EncodeOp(yytext, yyleng, yylineno, HSHL); return SHL; }
[jJ][mM][pP]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HJMP); return JMP; }
[jJ][zZ][rR]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HJZR); return JZR; }
[jJ][nN][zZ]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HJNZ); return JNZ; }
[jJ][nN][eE]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HJNE); return JNE; }
[jJ][pP][oO]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HJPO); return JPO; }
[jJ][cC][aA]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HJCA); return JCA; }
[jJ][nN][cC]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HJNC); return JNC; }
[cC][aA][lL]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HCAL); return CAL; }
[rR][eE][tT]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HRET); return RET; }
[nN][oO][pP]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HNOP); return NOP; }
[hH][lL][tT]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HHLT); return HLT; }
[tT][oO][fF]    { yylval = EncodeOp(yytext, yyleng, yylineno, HTOF); return TOF; }
[tT][oO][iI]    { yylval = EncodeOp(yytext, yyleng, yylineno, HTOI); return TOI; }
[pP][sS][hH]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HPSH); return PSH; }
[pP][oO][pP]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HPOP); return POP; }
[cC][mM][pP](\.[f|F])?	{ yylval = EncodeOp(yytext, yyleng, yylineno, HCMP); return CMP; }
[rR][dD][uU][mM][pP]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HRDUMP); return(RDUMP); }
[mM][dD][uU][mM][pP]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HMDUMP); return(MDUMP); }
[wW][rR][nN]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HWRN); return WRN; }
[wW][rR][cC]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HWRC); return WRC; }
[wW][rR][sS]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HWRS); return WRS; }
[wW][rR][fF]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HWRF); return WRF; }
[rR][dD][nN]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HRDN); return RDN; }
[rR][dD][cC]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HRDC); return RDC; }
[cC][sS][bB]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HCSB); return(CSB); }
[wW][sS][bB]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HWSB); return(WSB); }
[aA][sS][nN]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HASN); return(ASN); }
[aA][sS][sS]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HASS); return(ASS); }
[aA][sS][bB]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HASB); return(ASB); }
[zZ][sS][bB]    { yylval = EncodeOp(yytext, yyleng, yylineno, HZSB); return(ZSB); }
[aA][sS][cC]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HASC); return(ASC); }
[eE][xX][tT]	{ yylval = EncodeOp(yytext, yyleng, yylineno, HEXT); return(EXT); }
[sS][eE][tT](\.[f|F|s|S|w|W|h|H])?	{ yylval = EncodeOp(yytext, yyleng, yylineno, HSET); return(SET); }
[gG][eE][tT](\.[f|F|s|S|w|W])?	{ yylval = EncodeOp(yytext, yyleng, yylineno, HGET); return(GET); }
[sS][tT][mM]    { yylval = EncodeOp(yytext, yyleng, yylineno, HSTM); return(STM); }
[cC][tT][mM]    { yylval = EncodeOp(yytext, yyleng, yylineno, HCTM); return(CTM); }
[nN][fF][yY]    { yylval = EncodeOp(yytext, yyleng, yylineno, HNFY); return(NFY); }
[wW][fF][sS]    { yylval = EncodeOp(yytext, yyleng, yylineno, HWFS); return(WFS); }
[eE][vV][sS]    { yylval = EncodeOp(yytext, yyleng, yylineno, HEVS); return(EVS); }
[eE][vV][eE]    { yylval = EncodeOp(yytext, yyleng, yylineno, HEVE); return(EVE); }
[sS][bB][lL]    { yylval = EncodeOp(yytext, yyleng, yylineno, HSBL); return(SBL); }
[sS][bB][oO]    { yylval = EncodeOp(yytext, yyleng, yylineno, HSBO); return(SBO); }
[sS][cC][oO]    { yylval = EncodeOp(yytext, yyleng, yylineno, HSCO); return(SCO); }
[gG][cC][oO]    { yylval = EncodeOp(yytext, yyleng, yylineno, HGCO); return(GCO); }
[oO][fF][dD]    { yylval = EncodeOp(yytext, yyleng, yylineno, HOFD); return(OFD); }
[cC][fF][dD]    { yylval = EncodeOp(yytext, yyleng, yylineno, HCFD); return(CFD); }
[sS][fF][dD]    { yylval = EncodeOp(yytext, yyleng, yylineno, HSFD); return(SFD); }
[eE][xX][eE]    { yylval = EncodeOp(yytext, yyleng, yylineno, HEXE); return(EXE); }
[oO][pP][sS]    { yylval = EncodeOp(yytext, yyleng, yylineno, HOPS); return(OPS); }
[cC][pP][sS]    { yylval = EncodeOp(yytext, yyleng, yylineno, HCPS); return(CPS); }

{number} 		{ yylval = EncodeValue( yytext, eINTEGER, yylineno ); return NUM; }
{float}         { yylval = EncodeValue( yytext, eFLOAT,yylineno ); return FLOAT; }
{hexnum}		{ yylval = EncodeValue( yytext+2, eHEXADECIMAL,yylineno ); return NUM; }
{register}		{ yylval = GetRegister( yytext, yylineno ); return REG; }
[sS][pP]		{ yylval = GetRegister( yytext, yylineno ); return REG; }
[pP][cC]		{ yylval = GetRegister( yytext, yylineno ); return REG; }
{label}			{ yylval = AllocString( eLABEL, yytext, yyleng, true );  return LABEL; }
{character}		{ yylval = EncodeChar( yytext+1, yyleng-2, yylineno ); return CHAR; }
{string}		{ yylval = AllocString( eSTRING, yytext+1, yyleng-2, false ); return STRING; }
{strerr}		{ /* bad string constant */ return STRERR ; }

","				{ return COMMA; }

%%

/*============================================================================*/
/*  yywrap                                                                    */
/*!
    Stub function for yywrap

    This stub function does nothing

    @retval 1

==============================================================================*/
int yywrap()
{
	return 1;
}
