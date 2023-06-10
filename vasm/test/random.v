; random number generator

	JMP G_O

SOURCE DAT "/dev/urandom"

G_O
	CSB R1
	MOV R2, SOURCE
	ASS R1,R2        ; random number source
	MOV R3,R1
	OFD R3, 'R'      ; open input file in binary mode, file descriptor in R3
	SFD R3           ; select file descriptor in R3 for input
    MOV R4,10
LOOP
    RDN R5
    WRN R5
    WRC '\n'
    SUB R4,1
    JPO LOOP
    CFD R3
    HLT

