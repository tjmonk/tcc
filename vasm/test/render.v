; "render" program for the virtual machine.
; render /SYS/TEST/C sysvar

    JMP G_O

message DAT "test "

strvar
    DAT "/SYS/TEST/C"

G_O
    MOV R0,strvar
    EXT R0         ; get handle to the /SYS/TEST/C variable
    MOV R1,4       ; select a print notification
    NFY R0,R1      ; request print notification on /SYS/TEST/C variable
    MOV R10,10     ; repeat 10 times before we stop
WAIT
    WFS R3,R4      ; R3 is signal, R4 is notification identifier
    MOV R5,43      ; code for the PRINT signal
    CMP R3,R5      ; see if we received the PRINT signal
    JNZ WAIT       ; we are not handling this signal
    MOV R6,R4      ; notification identifier
    OPS R6,R7      ; open print session. Outputs: R6 is file descriptor, R7 is var handle
    CMP R7,R0      ; check for var handle
    JNZ DONE
    MOV R8, message ; render the output
    WRS R8
    WRN R10
    WRC '\n'
    SUB R10,1      ; decrement counter by 1
DONE
    CPS R4,R6      ; close print session
    CMP R10,0      ; check for exit condition
    JPO WAIT       ; no exit yet, keep waiting for signals
    HLT

