    JMP go
    HLT

go
    MOV R0, 12
    MOV R1, -1
    WRN R0
    WRC '\n'
    WRN R1
    WRC '\n'
    MUL R0, R1
    WRN R0
    WRC '\n'
    MUL R0, -2
    WRN R0
    WRC '\n'
    HLT
