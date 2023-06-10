; "hello, world" program for the virtual machine.

	JMP G_O
HELLO	DAT "Hello World!\n"

G_O
	MOV R12, HELLO
	CAL WRITE
	HLT

; write a null-terminated character string with address in R12.
; character will be loaded to R11.

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

