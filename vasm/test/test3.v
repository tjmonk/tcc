; instruction test program
; not intended to actually run

; write a null-terminated character string with address in R14.
; character will be loaded to R13.

	MOV R1, 1
	ADD R1, 16
	JMP START
	HLT

WRITE
	MOV R11, 0
WRITE1
	LOD.b R11, R12	; store a byte @R12 into R11
	ADD.b R11, 0	; test byte for null
	JNZ WRITE2
	RET		; if null character, return
WRITE2
	WRC R11		; write out character
	ADD R12, 1	; get next word
	JMP WRITE1

START
	MOV R12, FOO
	CAL WRITE

	DAT
	NOP
	CAL TEST
	HLT

FOO DAT "Hello World\r\n"

	DAT
	DAT
	NOP
TEST
	RDUMP
	WRN R1
	WRC '\n'
	STR FOO, R1
	RDUMP
	DAT 0xFE
	DAT 2.5
	DAT 'a'
	DAT 'b'
	DAT
	MOV R2,0x55AA
	MOV.b R1,R3
	MOV.w R1,R3
	MOV.l R1,R3
	MOV R1,R3
	CMP R1,R5
	WRN R3
	LOD R1,R3
	RET
	RET
	NOP
	WRN R4
	SHL R0,31
	SHR R1,12
	SHR R0, 12
	NOT R3
	RDUMP
	WRC 'A'
	WRC R12
	PSH R12
	POP R12
	JMP R6



	DAT
	DAT
	DAT
	DAT
HELLO	DAT "1234567890123456789012345678901234567890"

G_O


MOV.w R2, 0x2306
MOV.w R3, 0x46
mov.w R4, HELLO
;ret
;nop
hlt
;WRN R2
;NOT R1
;NOT R2
;NOT R3
;NOT R4
;NOT R5
;NOT R6
;NOT R7
;not r8
;not r9
;HELLO DAT "ABCDEFG"
;not r10
;not r11
;not r12
;not.b r13

