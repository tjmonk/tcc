JMP START
ADDITION
    DAT "addition\n"
SUBTRACTION
    DAT "subtraction\n"
MULTIPLICATION
    DAT "multiplication\n"
DIVISION
    DAT "division\n"
COMPARISON
    DAT "comparison\n"
MATCH
    DAT "numbers match!\n"
START
    MOV.F R0, 2.0
    WRF R0
    WRC '\n'
    MOV.F R1, 3.2
    WRF R1
    WRC '\n'

    ; addition
    MOV R8, ADDITION
    WRS R8
    ADD.F R1, R0
    WRF R1
    WRC '\n'

    ; subtraction
    MOV R8, SUBTRACTION
    WRS R8
    SUB.F R1, 1.3
    WRF R1
    WRC '\n'
    SUB.F R1, R0
    WRF R1
    WRC '\n'

    ; multiplication
    MOV R8, MULTIPLICATION
    WRS R8
    MUL.F R1, R0
    WRF R1
    WRC '\n'
    MUL.F R1, -2.15
    WRF R1
    WRC '\n'

    ; division
    MOV R8, DIVISION
    WRS R8
    MOV.F R0, 1.2
    DIV.F R1, R0
    WRF R1
    WRC '\n'
    DIV.F R1, 2.0
    WRF R1
    WRC '\n'
    MOV R2, 0
    MDUMP R2, 64

    ; comparison
    MOV R8, COMPARISON
    WRS R8
    MOV R0, 1.2
    MOV R1, 1.2
    CMP.F R0, R1
    RDUMP
    CMP.F R0, 1.2
    JNZ _end

_match
    MOV R8, MATCH
    WRS R8
_end
    HLT
