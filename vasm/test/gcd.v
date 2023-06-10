; program to read in two numbers and calculate their gcd

	JMP GO
PROMPT	DAT "Enter a number:  \0\"
ANSWER1	DAT "The GCD of \0\"
ANSWER2	DAT " and \0\"
ANSWER3	DAT " is \0\"
ANSWER4	DAT ".\n\0\"

go
	MOV R12, PROMPT	; get first number
	WRS R12
	RDN R0
	PSH R0		; push first number as function argument
	MOV R12, PROMPT	; get second number
	WRS R12
	RDN R1
	PSH R1		; push second number as function argument
	PSH R0		; save R0
	PSH R1		; save R1
	MOV R2, 0
	PSH R2		; push empty space for return value
	CAL GCD
	POP R2		; get return value
	ADD SP, 8	; remove function parameters from stack
	POP R1		; restore R1
	POP R0		; restore R0
	MOV R12, ANSWER1
	WRS R12	; write the answer
	WRN R0
	MOV R12, ANSWER2
	WRS R12
	WRN R1
	MOV R12, ANSWER3
	WRS R12
	WRN R2
	MOV R12, ANSWER4
	WRS R12
	HLT


; GCD - calculate the GCD of two numbers.
; arguments:	SP+1 - return value
;		SP+2 - second argument
;		SP+3 - first argument

GCD
	MOV R12, SP
	ADD R12, 12	; get first argument
	LOD R0, R12
	SUB R12, 4	; get second argument
	LOD R1, R12
	SUB R12, 4	; R12 has address of return value
GCD1
	MOV R2, R0
	SUB R2, R1
	JNZ GCD2
	STR R12, R0	; write function value
	RET
GCD2
	JCA GCD3
	MOV R0, R2	; RO was greater
	JMP GCD1
GCD3
	MUL R2, -1	; change to positive number
	MOV R1, R2	; R1 was greater
	JMP GCD1

