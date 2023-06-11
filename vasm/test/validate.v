; "validate" program for the virtual machine.
; prevent /SYS/TEST/A from being set to a value larger than 10
; ensure /SYS/TEST/C max length is 8

    JMP G_O
notice
    DAT "External Variable Validation test on /SYS/TEST/A variable\n"

sig
    DAT "Received signal: "

intvar
    DAT "/SYS/TEST/A"

strvar
    DAT "/SYS/TEST/C"

G_O
    MOV R6,0
    MOV R7,10
    MOV R0,notice
    WRS R0
    MOV R0,intvar
    EXT R0         ; get handle to the /SYS/TEST/A variable
    MOV R1,3       ; select a validation notification
    NFY R0,R1      ; request validation on /SYS/TEST/A variable
    MOV R2,strvar
    EXT R2         ; get handle to the /SYS/TEST/C variable
    NFY R2,R1      ; request validation on /SYS/TEST/C variable
WAIT
    WFS R3,R4 ; R3 is signal, R4 is notification identifier
    MOV R13,sig
    WRS R13
    WRN R3
    WRC '\n'
    MOV R5,42      ; R5 is code for the validation signal
    CMP R3,R5      ; see if we received the validation signal
    JNZ WAIT       ; we are not handling this signal
    MOV R7,34      ; indicate validation is not ok to start with (ERANGE)
    EVS R6,R4      ; start external variable validation
    CMP R6,R0      ; check the handle for /SYS/TEST/A
    JZR validate_a ; validate /SYS/TEST/A
    CMP R6,R2      ; check the handle for /SYS/TEST/CMP
    JZR validate_c ; validate /SYS/TEST/C
validate_a
    MOV R8,10      ; comparison value is 10
    GET R9,R0      ; get the value of /SYS/TEST/A
    CMP R9,R8
    JPO ENDVAL
    MOV R7,0       ; indicate validation is ok
    JMP ENDVAL
validate_c
    MOV R9,1
    CSB R9
    GET.S R9,R2    ; get the variable into the string buffer using its handle
    MOV R8,8       ; max length of string
    SBL R10,R9     ; get the length of /SYS/TEST/C in R9
    CMP R10,R8     ; compare the length against max length in R8
    JPO ENDVAL     ; value is 8 or larger
    MOV R7,0       ; indicate validation is ok
ENDVAL
    EVE R4,R7      ; return validation result
    JMP WAIT       ; wait for next signal
    HLT

