    ; "ticker" program for the virtual machine.
    ; increment /SYS/TEST/A every second
        JMP G_O
    intvar
        DAT "/SYS/TEST/A"
    G_O
        MOV R0,intvar
        EXT R0         ; get handle to the /SYS/TEST/A variable
        MOV R1,1       ; timer id
        MOV R2,1000    ; timer delay
        STM R1,R2      ; start timer
        MOV R3,0       ; initial value for /SYS/TEST/A
    WAIT
        WFS R4,R5      ; R4 is signal, R5 is id
        MOV R6,39      ; expected timer signal
        CMP R4,R6      ; compare received vs expected signal number
        JNZ WAIT       ; back to wait on unexpected signal
        CMP R5,R1      ; compare received vs expected timer id
        JNZ WAIT       ; back to wait on unexpected timer id
        SET R0,R3      ; update /SYS/TEST/A
        ADD R3,1       ; increment next value of /SYS/TEST/A
        JMP WAIT       ; wait for next signal
