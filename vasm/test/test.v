; string testing program

    JMP G_O
HELLO    DAT "1234567890123456789012345678901234567890"

FILENAME DAT "test.out"

CMD DAT "/usr/bin/hd "

DONE DAT "Done"

G_O
    CSB R1
    MOV R2, FILENAME
    ASS R1,R2        ; output file in string buffer
    MOV R3,R1
    OFD R3, 'w'      ; open output file, file descriptor in R3
    SFD R3           ; select file descriptor in R3 for output
    MOV R4, HELLO
    WRS R4           ; output the data to the file
    WRC '\n'         ; output newline to the file
    CFD R3           ; close the output file
    MOV R5, DONE
    MOV R6, 2        ; file descriptor of standard output
    SFD R6           ; select standard output
    WRS R5           ; output "Done"
    WRC '\n'         ; output newline
    CSB R7           ; create a new stringbuffer for the command
    MOV R8, CMD
    ASS R7,R8
    ASS R7,R2        ; construct command "/usr/bin/hd test,out"
    WSB R7           ; write the command we will be executing
    WRC '\n'         ; followed by a newline to flush the output
    EXE R8,R7        ; execute the command
    WRC '\n'
    WRN R8           ; output the command result
    WRC '\n'
    HLT

