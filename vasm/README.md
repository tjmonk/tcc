# VASM Assembler

## Overview

The vasm utility is a an assembler for a virtual machine.  It assembles the
specified input file and writes the assembled program file to the specified
output file.  The assembled file may be executed using the
[vexe](https://github.com/tjmonk/tcc/blob/main/vexe/README.md) or
[vm](https://github.com/tjmonk/tcc/blob/main/vm/README.md) commands.

## Command Line Arguments

```
usage: vasm [-c core size]
            [-s stack size]
            [-h]
            [-o output filename]
            <assembly file>
```

| Argument | Description | Default Value |
| --- | --- | --- |
| -c | specify the size of the VM core in bytes | 65536 |
| -s | specify the size of the VM stack in bytes | 4096 |
| -h | display help for command usage | |
| -o | specify the output filename | a.out |

## Assemebly Instruction Set

See [libvmasm](https://github.com/tjmonk/tcc/blob/main/libvmasm/README.md) for
the assembly language instruction set.

## Sample Programs

The following sample programs are provided:

| Program | Description | Notes |
| --- | --- | --- |
| fact.v | Calculate factorials up to 5! | |
| gcd.v | Find the greatest common divisor between two numbers | |
| hw.v | Traditional Hello World! program | |
| name.v | Greet the operator using their name | |
| random.v | Generate some random numbers | |
| render.v | Generate variable rendering | This sample requires the VarServer to be running and the /SYS/TEST/C variable to exist. |
| rot.v | simple register operation test | |
| sort.v | Sort numbers provided by the operator | |
| stringmod.v | String Buffer modification example | |
| test.v | File output and System() test | |
| test2.v | String constant test | |
| test3.v | instruction test program | This program is not intended to be run, and it crashes if you run it |
| test4.v | Signed multiplication test | |
| test5.v | Delay test | |
| testf.v | Floating point number test | |
| timer.v | Dual timers test | |
| validate.v | VarServer variable validation test | This sample requires the VarServer to be running and the /SYS/TEST/A (int16) and /SYS/TEST/C (str) variables to exist |

## Build

```
./build.sh
```

## Assemble a sample program

```
vasm test/fact.v -o test/fact.bin
hd test/fact.bin
```

```
00000000  43 00 01 63 39 00 4d 00  a1 23 1e 85 0e 04 23 2e  |C..c9.M..#....#.|
00000010  85 0e 04 23 2e 85 0e 04  23 2e 23 21 84 02 0c 21  |...#....#.#!...!|
00000020  62 83 07 01 36 67 4e 00  2f 83 06 00 4d 00 32 83  |b...6gN./...M.2.|
00000030  06 01 96 06 00 4e 00 43  83 08 01 23 08 23 e1 15  |.....N.C...#.#..|
00000040  4d 00 43 23 21 84 02 0c  21 92 83 0a 01 25 9a 23  |M.C#!...!....%.#|
00000050  21 84 02 fc 21 b2 22 29  23 0e 23 21 84 02 fc 21  |!...!.")#.#!...!|
00000060  c2 39 0c 39 00 39 01 54  00 09 1a 01 1a 02 23 e2  |.9.9.9.T......#.|
00000070  23 21 84 02 f8 21 d2 22  20 23 21 84 02 0c 21 32  |#!...!." #!...!2|
00000080  23 21 84 02 f8 21 42 26  34 23 21 84 02 f4 21 52  |#!...!B&4#!...!R|
00000090  22 23 23 21 84 02 f4 21  62 23 06 23 e1 15 23 e1  |"##!...!b#.#..#.|
000000a0  15 23 1e 85 0e 04 23 2e  85 0e 04 23 2e 4d 00 c6  |.#....#....#.M..|
000000b0  66 61 63 74 6f 72 69 61  6c 20 67 65 6e 65 72 61  |factorial genera|
000000c0  74 69 6f 6e 00 00 43 09  00 b0 1f 22 09 1f 8f 0a  |tion..C...."....|
000000d0  1f 8f 0a 83 0a 01 23 21  84 02 fc 21 b2 22 2a 23  |......#!...!."*#|
000000e0  21 84 02 fc 21 c2 83 0d  06 36 cd 50 00 f4 83 0c  |!...!....6.P....|
000000f0  00 4d 00 f7 83 0c 01 96  0c 00 4e 01 60 23 0e 23  |.M........N.`#.#|
00000100  21 84 02 fc 21 32 39 03  39 00 39 01 54 00 09 1a  |!...!29.9.9.T...|
00000110  01 1a 02 23 e2 23 21 84  02 f8 21 42 22 20 23 21  |...#.#!...!B" #!|
00000120  84 02 fc 21 52 1f 2e 05  4d 01 3b 20 66 61 63 74  |...!R...M.; fact|
00000130  6f 72 69 61 6c 20 69 73  20 00 00 43 06 01 2b 1f  |orial is ..C..+.|
00000140  22 06 23 21 84 02 f8 21  72 1f 2e 07 1f 8f 0a 23  |".#!...!r......#|
00000150  21 84 02 fc 21 92 23 89  84 09 01 22 29 4d 00 df  |!...!.#....")M..|
00000160  23 e1 15 1b                                       |#...|
00000164
```

