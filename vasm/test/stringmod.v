; string modification program for the virtual machine.
; This program creates a sting buffer with the notice below
; and then reverses the string before truncating it and
; appending a string suffix

    JMP G_O
notice
    DAT "This is a test of the stringbuffer manipulation"

suffix
    DAT "...done"

length
    DAT "Length of string buffer: "

G_O
    MOV R0,notice
    CSB R1
    ASS R1,R0
    SBL R2,R1  ; get the length of the string buffer
    MOV R3,length
    WRS R3
    WRN R2
    WRC '\n'
    MOV R4,0
    MOV R13,R2
    SUB R13,1
reverse
    SBO R1,R4 ; R4 index at start of string
    GCO R5,R1 ; R5 char at start of string
    SBO R1,R13 ; R13 index from end of string
    GCO R6,R1 ; R6 char at end of string
    SBO R1,R4 ; R4 index from start of string
    SCO R1,R6 ; Store end char at start of string
    SBO R1,R13 ; R13 index from end of string
    SCO R1,R5 ; Store start char at end of string
    ADD R4,1 ; increment start index
    SUB R13,1 ; decrement end index
    CMP R13,R4
    JPO reverse
output
    WSB R1
    WRC '\n'
truncate
    MOV R6,14
    SBO R1,R6
    SCO R1,'\0'
    WSB R1
    WRC '\n'
append
    MOV R7,suffix
    ASS R1,R7
    WSB R1
    WRC '\n'
done
    HLT

