; what is your name, anyway?
jmp GO
QUESTION DAT "Enter your name: "
greeting dat "Hello, "
BANG    DAT "!\n"

go

    mov r12, question
    cal WRITE
    MOV R12, HEAP
    CAL READ
    MOV R12, GREETING
    CAL WRITE
    MOV R12, HEAP
    CAL WRITE
    MOV R12, BANG
    CAL WRITE
    HLT

; write a null-terminated character string with address in R12.
; character will be loaded to R11.

WRITE
    MOV R11, 0
WRITE1
    LOD.b R11, R12 ; store a byte @R12 into R11
    ADD.b R11, 0   ; test byte for null
    JNZ WRITE2
    RET            ; if null character, return
WRITE2
    WRC R11        ; write out character
    ADD R12, 1     ; get next word
    JMP WRITE1

; read a character string to memory, storing at the address in R13.

READ
    MOV R13, R12   ; initialize R13 for testing
GETCHAR1
    RDC R12
    CMP R12, '\n'
    JNZ READ1
    MOV R0, 0
    STR.b R13, R0
    RET
READ1
    STR.b R13, R12
    ADD R13, 1
    JMP GETCHAR1

HEAP
