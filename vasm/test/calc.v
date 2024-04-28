    ; "calc" program for the virtual machine.
    ; increment /SYS/TEST/B every time it is requested
        JMP G_O
    sig
        DAT "Received signal: "
    intvar
        DAT "/SYS/TEST/B"
    G_O
        MOV R0,intvar
        EXT R0         ; get handle to the /SYS/TEST/B variable
        MOV R1,2       ; select a calc notification
        NFY R0,R1      ; request calculation on /SYS/TEST/B variable
        MOV R2, 0      ; R2 will hold the value of /SYS/TEST/B
    WAIT
        WFS R3,R4      ; R3 is signal, R4 is variable handle
        MOV R5,sig
        WRS R5         ; output received signal notification
        WRN R3         ; output signal number
        WRC '\n'       ; output newline
        MOV R6,41      ; R5 (41) is code for the calc signal
        CMP R3,R6      ; see if we received the validation signal
        JNZ WAIT       ; we are not handling this signal
        SET R4, R2     ; write value to /SYS/TEST/B
        ADD R2, 1      ; increment R2
        RDUMP          ; register dump
        JMP WAIT       ; wait for next signal
