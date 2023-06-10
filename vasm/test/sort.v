	MOV R0,_exit	;program startup code
	PSH R0		;program end
	JMP _main	;jump to program start
_main
	MOV R1,SP	;stack frame pointer
	SUB SP,400
	MOV R2,SP	;decl: list
	SUB SP,4
	MOV R2,SP	;decl: numbers
	SUB SP,4
	MOV R2,SP	;decl: num
	SUB SP,4
	MOV R2,SP	;decl: sorted
	SUB SP,4
	MOV R2,SP	;decl: i
	JMP _STR0
_txt1
	DAT "How many numbers do u wish to sort? "
_STR0
	MOV R8,_txt1
	WRS R8		;output string
	MOV R2,R1
	ADD R2,-404
	LOD R9,R2	;id: numbers
	RDN R9		;read integer
	STR R2,R9
	JMP _STR2
_txt3
	DAT "You selected "
_STR2
	MOV R10,_txt3
	WRS R10		;output string
	MOV R2,R1
	ADD R2,-404
	LOD R11,R2	;id: numbers
	WRN R11		;output integer
	JMP _STR4
_txt5
	DAT "numbers.\n"
_STR4
	MOV R12,_txt5
	WRS R12		;output string
	MOV R2,R1
	ADD R2,-404
	LOD R13,R2	;id: numbers
	MOV R3,0	;constant
	CMP R13,R3	;LT comparison
	JNE _LT8
	MOV R13,0
	JMP _LT9
_LT8
	MOV R13,1
_LT9
	CMP R13,0
	JZR _IF6
	JMP _STR10
_txt11
	DAT "I cant sort a negative number of values!\n"
_STR10
	MOV R4,_txt11
	WRS R4		;output string
	MOV R5,0	;constant
	MOV R0,R5	;return result
	;TODO: we do not seem to return when we exit a function early
	MOV SP,R1	;free locals
	RET		;return to caller
	JMP _IF7
_IF6
	MOV R2,R1
	ADD R2,-404
	LOD R6,R2	;id: numbers
	MOV R7,100	;constant
	CMP R7,R6	;GT comparison
	JNE _GT14
	MOV R6,0
	JMP _GT15
_GT14
	MOV R6,1
_GT15
	CMP R6,0
	JZR _IF12
	JMP _STR16
_txt17
	DAT "too large   max 100!\n"
_STR16
	MOV R8,_txt17
	WRS R8		;output string
	MOV R9,0	;constant
	MOV R0,R9	;return result
	;TODO: we do not seem to return when we exit a function early
	MOV SP,R1	;free locals
	RET		;return to caller
	JMP _IF13
_IF12
	MOV R2,R1
	ADD R2,-404
	LOD R10,R2	;id: numbers
	MOV R11,0	;constant
	CMP R10,R11	;equals comparison
	JZR _EQU20
	MOV R10,0
	JMP _EQU21
_EQU20
	MOV R10,1
_EQU21
	CMP R10,0
	JZR _IF18
	JMP _STR22
_txt23
	DAT "what is the point of sorting nothing at all?\n"
_STR22
	MOV R12,_txt23
	WRS R12		;output string
	MOV R13,0	;constant
	MOV R0,R13	;return result
	;TODO: we do not seem to return when we exit a function early
	MOV SP,R1	;free locals
	RET		;return to caller
	JMP _IF19
_IF18
_IF19
_IF13
_IF7
	JMP _STR24
_txt25
	DAT "Enter "
_STR24
	MOV R3,_txt25
	WRS R3		;output string
	MOV R2,R1
	ADD R2,-404
	LOD R4,R2	;id: numbers
	WRN R4		;output integer
	JMP _STR26
_txt27
	DAT " values to be sorted:\n"
_STR26
	MOV R5,_txt27
	WRS R5		;output string
	MOV R6,0	;constant
	MOV R2,R1
	ADD R2,-416	;l-value: i
	LOD R7,R2
	STR R2,R6	;assignment
_F28
	MOV R2,R1
	ADD R2,-416
	LOD R8,R2	;id: i
	MOV R2,R1
	ADD R2,-404
	LOD R9,R2	;id: numbers
	CMP R8,R9	;LT comparison
	JNE _LT30
	MOV R8,0
	JMP _LT31
_LT30
	MOV R8,1
_LT31
	CMP R8,0
	JZR _F29
	MOV R2,R1
	ADD R2,-408
	LOD R10,R2	;id: num
	RDN R10		;read integer
	STR R2,R10
	MOV R2,R1
	ADD R2,-408
	LOD R11,R2	;id: num
	MOV R2,R1
	ADD R2,-416
	LOD R12,R2	;id: i
	MOV R2,R1
	ADD R2,-4	;l-value: list
	LOD R13,R2
	MUL R12,4	;multiply array offset by stack element size
	SUB R2,R12	;calculate array offset
	LOD R13,R2
	STR R2,R11	;assignment
	MOV R2,R1
	ADD R2,-416
	LOD R4,R2	;id: i
	MOV R3,R4
	ADD R4,1
	STR R2,R4	;post-increment
	JMP _F28
_F29
	JMP _STR32
_txt33
	DAT "Sorting...\n"
_STR32
	MOV R5,_txt33
	WRS R5		;output string
_F34
	MOV R2,R1
	ADD R2,-412
	LOD R6,R2	;id: sorted
	MOV R7,0	;constant
	CMP R6,R7	;equals comparison
	JZR _EQU36
	MOV R6,0
	JMP _EQU37
_EQU36
	MOV R6,1
_EQU37
	CMP R6,0
	JZR _F35
	MOV R8,1	;constant
	MOV R2,R1
	ADD R2,-412	;l-value: sorted
	LOD R9,R2
	STR R2,R8	;assignment
	MOV R10,0	;constant
	MOV R2,R1
	ADD R2,-416	;l-value: i
	LOD R11,R2
	STR R2,R10	;assignment
_F38
	MOV R2,R1
	ADD R2,-416
	LOD R12,R2	;id: i
	MOV R2,R1
	ADD R2,-404
	LOD R13,R2	;id: numbers
	MOV R3,1	;constant
	SUB R13,R3	;integer subtraction
	CMP R12,R13	;LT comparison
	JNE _LT40
	MOV R12,0
	JMP _LT41
_LT40
	MOV R12,1
_LT41
	CMP R12,0
	JZR _F39
	MOV R2,R1
	ADD R2,-416
	LOD R4,R2	;id: i
	MOV R2,R1
	ADD R2,-4	;l-value: list
	LOD R5,R2
	MUL R4,4	;multiply array offset by stack element size
	SUB R2,R4	;calculate array offset
	LOD R5,R2
	MOV R2,R1
	ADD R2,-408	;l-value: num
	LOD R6,R2
	STR R2,R5	;assignment
	MOV R2,R1
	ADD R2,-416
	LOD R7,R2	;id: i
	MOV R2,R1
	ADD R2,-4	;l-value: list
	LOD R8,R2
	MUL R7,4	;multiply array offset by stack element size
	SUB R2,R7	;calculate array offset
	LOD R8,R2
	MOV R2,R1
	ADD R2,-416
	LOD R9,R2	;id: i
	MOV R10,1	;constant
	ADD R9,R10	;integer addition
	MOV R2,R1
	ADD R2,-4	;l-value: list
	LOD R11,R2
	MUL R9,4	;multiply array offset by stack element size
	SUB R2,R9	;calculate array offset
	LOD R11,R2
	CMP R11,R8	;GT comparison
	JNE _GT44
	MOV R8,0
	JMP _GT45
_GT44
	MOV R8,1
_GT45
	CMP R8,0
	JZR _IF42
	MOV R12,0	;constant
	MOV R2,R1
	ADD R2,-412	;l-value: sorted
	LOD R13,R2
	STR R2,R12	;assignment
	MOV R2,R1
	ADD R2,-416
	LOD R3,R2	;id: i
	MOV R2,R1
	ADD R2,-4	;l-value: list
	LOD R4,R2
	MUL R3,4	;multiply array offset by stack element size
	SUB R2,R3	;calculate array offset
	LOD R4,R2
	MOV R2,R1
	ADD R2,-408	;l-value: num
	LOD R5,R2
	STR R2,R4	;assignment
	MOV R2,R1
	ADD R2,-416
	LOD R6,R2	;id: i
	MOV R7,1	;constant
	ADD R6,R7	;integer addition
	MOV R2,R1
	ADD R2,-4	;l-value: list
	LOD R8,R2
	MUL R6,4	;multiply array offset by stack element size
	SUB R2,R6	;calculate array offset
	LOD R8,R2
	MOV R2,R1
	ADD R2,-416
	LOD R9,R2	;id: i
	MOV R2,R1
	ADD R2,-4	;l-value: list
	LOD R10,R2
	MUL R9,4	;multiply array offset by stack element size
	SUB R2,R9	;calculate array offset
	LOD R10,R2
	STR R2,R8	;assignment
	MOV R2,R1
	ADD R2,-408
	LOD R11,R2	;id: num
	MOV R2,R1
	ADD R2,-416
	LOD R12,R2	;id: i
	MOV R13,1	;constant
	ADD R12,R13	;integer addition
	MOV R2,R1
	ADD R2,-4	;l-value: list
	LOD R3,R2
	MUL R12,4	;multiply array offset by stack element size
	SUB R2,R12	;calculate array offset
	LOD R3,R2
	STR R2,R11	;assignment
	JMP _IF43
_IF42
_IF43
	MOV R2,R1
	ADD R2,-416
	LOD R5,R2	;id: i
	MOV R4,R5
	ADD R5,1
	STR R2,R5	;post-increment
	JMP _F38
_F39
	JMP _F34
_F35
	JMP _STR46
_txt47
	DAT "sorted values:\n"
_STR46
	MOV R6,_txt47
	WRS R6		;output string
	MOV R7,0	;constant
	MOV R2,R1
	ADD R2,-416	;l-value: i
	LOD R8,R2
	STR R2,R7	;assignment
_F48
	MOV R2,R1
	ADD R2,-416
	LOD R9,R2	;id: i
	MOV R2,R1
	ADD R2,-404
	LOD R10,R2	;id: numbers
	CMP R9,R10	;LT comparison
	JNE _LT50
	MOV R9,0
	JMP _LT51
_LT50
	MOV R9,1
_LT51
	CMP R9,0
	JZR _F49
	MOV R2,R1
	ADD R2,-416
	LOD R11,R2	;id: i
	MOV R2,R1
	ADD R2,-4	;l-value: list
	LOD R12,R2
	MUL R11,4	;multiply array offset by stack element size
	SUB R2,R11	;calculate array offset
	LOD R12,R2
	MOV R2,R1
	ADD R2,-408	;l-value: num
	LOD R13,R2
	STR R2,R12	;assignment
	MOV R2,R1
	ADD R2,-408
	LOD R3,R2	;id: num
	WRN R3		;output integer
	JMP _STR52
_txt53
	DAT "\n"
_STR52
	MOV R4,_txt53
	WRS R4		;output string
	MOV R2,R1
	ADD R2,-416
	LOD R6,R2	;id: i
	MOV R5,R6
	ADD R6,1
	STR R2,R6	;post-increment
	JMP _F48
_F49
	MOV SP,R1	;free locals
	RET		;return to caller

_exit	HLT
