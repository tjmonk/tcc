; "on change" notification program for the virtual machine.
; watch for changes to /SYS/TEST/A
    JMP G_O
intvar
    DAT "/SYS/TEST/A"
G_O
    MOV R0,intvar
    EXT R0         ; get handle to the /SYS/TEST/A variable
    MOV R1,1       ; on change request
    NFY R0, R1     ; request change notifications
WAIT
    WFS R2,R3      ; R2 is signal, R3 is variable handle
    MOV R4,40      ; expected modified signal
    CMP R2,R4      ; compare received vs expected signal number
    JNZ WAIT       ; back to wait on unexpected signal
    CMP R3,R0      ; compare received vs expected timer variable handle
    JNZ WAIT       ; back to wait on unexpected timer id
    GET R5,R0      ; get value /SYS/TEST/A
    MOV R6,intvar
    WRS R6
    WRC '='
    WRN R5
    WRC '\n'
    JMP WAIT       ; wait for next signal
