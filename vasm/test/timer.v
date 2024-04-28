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
    MOV R7,10           ; set counter to 10
    MOV R0, notice      ; set up welcome message
    WRS R0              ; output welcome message
    MOV R0,2000         ; R0 = timer delay 2000 ms
    MOV R1,1            ; R1 = timer 1
    STM R1,R0           ; Start timer 1 with 2000ms timeout
    MOV R2, timer       ; point to status message
    WRS R2              ; output status message
    WRN R1              ; output timer number
    WRC '\n'            ; output newline
    MOV R0,500          ; R0 = timer delay 500 ms
    MOV R1,2            ; R1 = timer 2
    STM R1,R0           ; Start timer 2 with 500 ms timeout
    MOV R2, timer       ; point to status message
    WRS R2              ; output status message
    WRN R1              ; output timer number
    WRC '\n'            ; output newline
LOOP
    WFS R3, R4          ; wait for a signal
    MOV R0, sig         ; point to received signal msg
    WRS R0              ; output received signal msg
    WRN R3              ; output signal type
    WRC ','             ; output comma
    WRN R4              ; output signal source
    WRC '\n'            ; output newline
    MOV R5,2            ; R5 = 2
    CMP R4,R5           ; check if signal is for timer 2
    JZR count2          ; if yes, jump to count 2
    JMP LOOP            ; back to top of loop
count2
    ADD R6,1            ; R6 = R6 + 1
    CMP R6,R7           ; compare R6 and R7
    JNZ LOOP            ; if R6 <> R7 jump back to top
    CTM R4              ; cancel timer 2
    MOV R4, 1           ; R4 = 1
    CTM R4              ; cancel timer 1
    HLT                 ; terminate program
