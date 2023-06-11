;-------------------------------------------------------------------------------
; random number generator
;
; Read 10 random numbers from /dev/urandom and write them to stdout
; one per line
;
;-------------------------------------------------------------------------------

    JMP G_O

SOURCE DAT "/dev/urandom"

G_O
    CSB R1           ; create a string buffer: id -> R1
    MOV R2, SOURCE   ; R2 points to random number source name
    ASS R1,R2        ; append name to string buffer
    MOV R3,R1        ; R3 <- string buffer identifier
    OFD R3, 'R'      ; open file in binary read mode, file descriptor in R3
    SFD R3           ; select file descriptor in R3 for input
    MOV R4,10        ; set loop counter to 10
LOOP
    RDN R5           ; read 32 bit integer from active file descriptor into R5
    WRN R5           ; write R5 to stdout
    WRC '\n'         ; write a newline
    SUB R4,1         ; decrement the loop counter
    JPO LOOP         ; keep going while loop counter is positive
    CFD R3           ; close the file descriptor in R3
    HLT              ; stop the virtual machine
