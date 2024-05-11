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
       Definitions
==============================================================================*/
#define YY_NO_INPUT

/*==============================================================================
        Includes
==============================================================================*/

#include "tcc.tab.h"
#include "lineno.h"

%}

%option nounput

letter [a-zA-Z\_]
digit [0-9]
nzdigit [1-9]
hexnum 0[xX][0-9a-fA-F]+

nl [\n]
delim [ \t]
ws {delim}+
if "if"
else "else"
for "for"
while "while"
switch "switch"
case "case"
default "default"
return "return"
delay "delay"
break "break"
Write "write"
WriteLn "writeLn"
Read  "read"
ReadLn "readLn"
append "append"
length ".length"
charat ".charAt"
setat ".setAt"
true "true"
false "false"
int "int"
float "float"
bool "bool"
char "char"
string "string"
extern "extern"
handle "handle"
settimer "set_timer"
cleartimer "clear_timer"
waitsig "wait_sig"
notify "notify"
validate_start "validate_start"
validate_end "validate_end"
file_open "file_open"
file_close "file_close"
file_read "file_read"
file_write "file_write"
system "system"
open_print_session "open_print_session"
close_print_session "close_print_session"

cmt "//"
or "||"
and "&&"
not "!"
xor "^"
bor "|"
band "&"
bnot "~"

equals "=="
notequals "!="

assign "="
times_equals "*="
div_equals "/="
plus_equals "+="
minus_equals "-="
and_equals "&="
or_equals "|="
xor_equals "^="

plus "+"
minus "-"
times "*"
divide "/"

lparen "("
rparen ")"
lbrace "{"
rbrace "}"
lbracket "["
rbracket "]"
doublequote "\""

comma ","
semi ";"
dot "."
colon ":"

lte "<="
gte ">="
lt "<"
gt ">"

rshift ">>"
lshift "<<"

inc "++"
dec "--"

charstr ([\"][^\"]*[\"])
character '((\\?.)|\\({digit}+|{hexnum}))'
comment {cmt}(.*)$
sysvar (\${letter}({letter}|{digit}|{dot})*)
id {letter}({letter}|{digit})*
num [-]?({digit}|({nzdigit}{digit}*))
floatnum  [-+]?[0-9]*\.[0-9]+([eE][-+]?[0-9]+)?

%%
{ws} {/* No action for white space */}
{comment} { /* No action for comments */};
{nl} incrementLineNumber();
{if} return(IF);
{else} return(ELSE);
{for} return(FOR);
{while} return(WHILE);
{break} return(BREAK);
{return} return(RETURN);
{delay} return(DELAY);
{settimer} return(SETTIMER);
{cleartimer} return(CLEARTIMER);
{waitsig} return(WAITSIG);
{notify} return(NOTIFY);
{switch} return(SWITCH);
{case} return(CASE);
{default} return(DEFAULT);
{Write} return(WRITE);
{WriteLn} return(WRITELN);
{Read} return(READ);
{ReadLn} return(READLN);
{true} return(VAL_TRUE);
{false} return(VAL_FALSE);
{int} return(TYPE_INT);
{float} return(TYPE_FLOAT);
{bool} return(TYPE_BOOL);
{char} return(TYPE_CHAR);
{string} return(TYPE_STRING);
{character} return(CHARACTER);
{floatnum} return(FLOAT);
{sysvar} return(SYSVAR_ID);
{extern} return(EXTERN);
{handle} return(HANDLE);
{validate_start} return(VALIDATE_START);
{validate_end} return(VALIDATE_END);
{file_open} return(FILE_OPEN);
{file_close} return(FILE_CLOSE);
{file_read} return(FILE_READ);
{file_write} return(FILE_WRITE);
{system} return(SYSTEM);
{open_print_session} return(OPEN_PRINT_SESSION);
{close_print_session} return(CLOSE_PRINT_SESSION);

{or} return(OR);
{and} return(AND);
{not} return(NOT);
{xor} return(XOR);
{bor} return(BOR);
{band} return(BAND);
{bnot} return(BNOT);

{equals} return(EQUALS);
{notequals} return(NOTEQUALS);

{assign} return(ASSIGN);
{times_equals} return(TIMES_EQUALS);
{div_equals} return(DIV_EQUALS);
{plus_equals} return(PLUS_EQUALS);
{minus_equals} return(MINUS_EQUALS);
{and_equals} return(AND_EQUALS);
{or_equals} return(OR_EQUALS);
{xor_equals} return(XOR_EQUALS);
{plus} return(PLUS);
{minus} return(MINUS);
{times} return(TIMES);
{divide} return(DIVIDE);

{lparen} return(LPAREN);
{rparen} return(RPAREN);
{lbrace} return(LBRACE);
{rbrace} return(RBRACE);
{lbracket} return(LBRACKET);
{rbracket} return(RBRACKET);

{dot} return(DOT);
{comma} return(COMMA);
{semi} return(SEMI);
{colon} return(COLON);
{doublequote} return(DQUOTE);
{lte} return(LTE);
{gte} return(GTE);
{lt} return(LT);
{gt} return(GT);

{append} return(APPEND);
{length} return(LENGTH);
{charat} return(CHARAT);
{setat} return(SETAT);

{rshift} return(RSHIFT);
{lshift} return(LSHIFT);

{inc} return(INC);
{dec} return(DEC);

{id} return(ID);
{num} return(NUM);
{hexnum} return(HEXNUM);
{charstr} return (CHARSTR);
%%

int yywrap()
{
    return 1;
}
