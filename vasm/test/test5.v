; delay testing program

    JMP G_O
TICK    DAT "tick.."

G_O
    MOV R12, TICK
    MOV R4, 500
LOOP
    WRS R12
    WRC '\n'
    DLY R4
    JMP G_O

