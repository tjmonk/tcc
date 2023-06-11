; "timer" program for the virtual machine.

    JMP G_O
notice
    DAT "Timer test\n"

timer
    DAT "Created timer "

sig
    DAT "Received signal: "

G_O
    MOV R6,0
    MOV R7,10
    MOV R0, notice
    WRS R0
    MOV R0,5000
    MOV R1,1
    STM R1,R0
    MOV R2, timer
    WRS R2
    WRN R1
    WRC '\n'
    MOV R0,500
    MOV R1,2
    STM R1,R0
    MOV R2, timer
    WRS R2
    WRN R1
    WRC '\n'
LOOP
    WFS R3, R4
    MOV R0, sig
    WRS R0
    WRN R3
    WRC ','
    WRN R4
    WRC '\n'
    MOV R5,2
    CMP R4,R5
    JZR count2
    JMP LOOP
count2
    ADD R6,1
    CMP R6,R7
    JNZ LOOP
    CTM R4
    JMP LOOP
    HLT

