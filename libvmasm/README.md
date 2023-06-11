# libvmasm

## Overview

The libvmasm library implements the assembly language parser for the
Virtual Machine.  The parser is implemented using flex/bison.
It works with the [libvmcore](https://github.com/tjmonk/tcc/tree/main/libvmcore)
library to assemble and link the assembly language program into the
Virtual Machine Core memory.  The VM core program memory can then be
written to disk to create the binary representation of the user program.

## Assembly Language Samples

See the [vasm](https://github.com/tjmonk/tcc/tree/main/vasm/test) test
directory for sample assembly language programs.

## Virtual Machine Instruction Set

See [libvmcore](https://github.com/tjmonk/tcc/blob/main/libvmcore/README.md)
for an overview of the Virtual Machine instruction set.

## Build

```
./build.sh
```

## Language Specification

The Virtual Machine assembly language specification is shown below:

```
program	: cmdlist
	;

cmdlist	: cmdlist command
	| command
	;

command	: label smplcmd EOLN
	| label EOLN
	| smplcmd EOLN
	| RDUMP EOLN
	| error EOLN
	| EOLN
    ;

label	: LABEL
	;

smplcmd	: DAT val
	| DAT STRING
	| DAT STRERR
	| DAT
	| DAT error
	| instr2 args1
	| STR args2
    | NOT REG
	| shift REG delim NUM
	| jump arg3
    | CAL arg3
    | CSB REG
    | EXE REG delim REG
    | WSB REG
    | ZSB REG
    | ASN REG delim REG
    | ASS REG delim REG
    | ASB REG delim REG
    | ASC REG delim REG
    | OFD REG delim REG
    | OFD REG delim CHAR
    | CFD REG
    | SFD REG
	| RET
	| NOP
	| HLT
    | MDUMP REG delim NUM
	| RDN REG
    | RDC REG
    | WRS REG
    | WRF REG
    | WRF float
    | WRN REG
	| WRN val
    | WRC REG
	| WRC CHAR
	| PSH REG
	| PSH val
	| POP REG
    | TOF REG
    | TOI REG
    | EXT REG
    | GET REG delim REG
    | SET REG delim REG
    | DLY REG
    | NFY REG delim REG
    | WFS REG delim REG
    | EVS REG delim REG
    | EVE REG delim REG
    | STM REG delim REG
    | CTM REG
    | SBL REG delim REG
    | SBO REG delim REG
    | OPS REG delim REG
    | CPS REG delim REG
    | SCO REG delim REG
	| SCO REG delim CHAR
    | GCO REG delim REG
	;

args1	: REG delim val
	| REG delim REG
	| val delim badarg
	;

args2	: val delim REG
    | REG delim REG
    | error val
    ;

arg3	: REG
	| LABEL
	;

badarg	: val
	| REG
	;

instr2	: LOD
	| MOV
	| ADD
	| SUB
	| MUL
	| DIV
	| AND
	| OR
	| CMP
	;

shift	: SHR
	| SHL
	;

jump	: JMP
	| JZR
	| JNZ
	| JNE
	| JPO
	| JCA
	| JNC
	;

delim	: COMMA
	|
	;

val	: LABEL
	| NUM
	| CHAR
    | FLOAT
	;

float : FLOAT
    ;
```
