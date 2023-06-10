	MOV R0,_exit	;program startup code
	PSH R0		;program end
	JMP _main	;jump to program start
_fact
	MOV R1,SP	;stack frame pointer
	SUB SP,4
	MOV R2,SP	;decl: a
	SUB SP,4
	MOV R2,SP	;decl: b
	SUB SP,4
	MOV R2,SP	;decl: c
	MOV R2,R1
	ADD R2,12
	LOD R6,R2	;id: n
	MOV R7,1	;constant
	CMP R6,R7	;equals comparison
	JZR _EQU2
	MOV R6,0
	JMP _EQU3
_EQU2	MOV R6,1
_EQU3
	CMP R6,0
	JZR _IF0
	MOV R8,1	;constant
	MOV R0,R8	;return result
	;TODO: we do not seem to return when we exit a function early
	MOV SP,R1	;free locals
	RET		;return to caller
	JMP _IF1
_IF0
_IF1
	MOV R2,R1
	ADD R2,12
	LOD R9,R2	;id: n
	MOV R10,1	;constant
	SUB R9,R10	;integer subtraction
	MOV R2,R1
	ADD R2,-4	;l-value: a
	LOD R11,R2
	STR R2,R9	;assignment
	MOV R0,SP	;save stack pointer
	MOV R2,R1
	ADD R2,-4
	LOD R12,R2	;id: a
	PSH R12		;push argument
	PSH R0		;procedure invokation
	PSH R1
	CAL _fact
	POP R1
	POP R2
	MOV SP,R2
	MOV R2,R1
	ADD R2,-8	;l-value: b
	LOD R13,R2
	STR R2,R0	;assignment
	MOV R2,R1
	ADD R2,12
	LOD R3,R2	;id: n
	MOV R2,R1
	ADD R2,-8
	LOD R4,R2	;id: b
	MUL R3,R4	;integer multiplication
	MOV R2,R1
	ADD R2,-12	;l-value: c
	LOD R5,R2
	STR R2,R3	;assignment
	MOV R2,R1
	ADD R2,-12
	LOD R6,R2	;id: c
	MOV R0,R6	;return result
	;TODO: we do not seem to return when we exit a function early
	MOV SP,R1	;free locals
	RET		;return to caller
	MOV SP,R1	;free locals
	RET		;return to caller
_main
	MOV R1,SP	;stack frame pointer
	SUB SP,4
	MOV R2,SP	;decl: i
	SUB SP,4
	MOV R2,SP	;decl: n
	JMP _STR4
_txt5	DAT "factorial generation\0\"
_STR4	MOV R9,_txt5
	WRS R9	;output string
	WRC '\n'	;newline
	WRC '\n'	;newline
	MOV R10,1	;constant
	MOV R2,R1
	ADD R2,-4	;l-value: i
	LOD R11,R2
	STR R2,R10	;assignment
_F6	MOV R2,R1
	ADD R2,-4
	LOD R12,R2	;id: i
	MOV R13,6	;constant
	CMP R12,R13	;LT comparison
	JNE _LT8
	MOV R12,0
	JMP _LT9
_LT8	MOV R12,1
_LT9
	CMP R12,0
	JZR _F7
	MOV R0,SP	;save stack pointer
	MOV R2,R1
	ADD R2,-4
	LOD R3,R2	;id: i
	PSH R3		;push argument
	PSH R0		;procedure invokation
	PSH R1
	CAL _fact
	POP R1
	POP R2
	MOV SP,R2
	MOV R2,R1
	ADD R2,-8	;l-value: n
	LOD R4,R2
	STR R2,R0	;assignment
	MOV R2,R1
	ADD R2,-4
	LOD R5,R2	;id: i
	WRN R5		;output integer
	JMP _STR10
_txt11	DAT " factorial is \0\"
_STR10	MOV R6,_txt11
	WRS R6	;output string
	MOV R2,R1
	ADD R2,-8
	LOD R7,R2	;id: n
	WRN R7		;output integer
	WRC '\n'	;newline
	MOV R2,R1
	ADD R2,-4
	LOD R9,R2	;id: i
	MOV R8,R9
	ADD R9,1
	STR R2,R9	;post-increment
	JMP _F6
_F7
	MOV SP,R1	;free locals
	RET		;return to caller

_exit	HLT
