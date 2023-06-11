# Virtual Machine Utility

## Overview

The Virtual Machine utility allows assembly and execution of
assembly language files or assembled VM binary files produced
by [vasm](https://github.com/tjmonk/tcc/tree/main/vasm).

## Command Line Arguments

```
usage: vm [-a] [-e] [-p] [-l] [-r] [-v] [-L <external variable handler library] [-c <core size (bytes)>] [-s <stack size (longwords)>] [-o <output file>] <input file>

 -a : assemble input file
 -e : execute input file
 -p : enable postmortem core dump
 -l : show labels
 -r : show registers
 -v : enable verbose operation
 -L : specify exernvars library
 -c : set core size
 -s : set stack size
 -o : write out program memory
```

| Argument | Description | Default Value |
| --- | --- | --- |
| -a | assemble an input file |  |
| -e | execute assembled file or binary input file |  |
| -p | dump registers and core after program execution | |
| -l | debug label assignment during link phase | |
| -r | show registers after program execution | |
| -v | enable verbose output | |
| -L | specify an external variable library ( e.g. libvarvm.so ) |  |
| -c | set the core size in bytes | 65536 |
| -s | set the stack size in bytes | 4096 |

## Assemebly Instruction Set

See [libvmasm](https://github.com/tjmonk/tcc/blob/main/libvmasm/README.md) for
the assembly language instruction set.

## Build

```
./build.sh
```

## Assemble and run a sample program

```
vm -a -e -p test/HelloWorld.v
cat vm.core
```

```

core:

00000000: 4D 00 11 48 65 6C 6C 6F 20 57 6F 72 6C 64 21 0A M..Hello World!.
00000010: 00 43 0C 00 03 54 00 19 1B 83 0B 00 A1 BC 84 0B .C...T..........
00000020: 00 4F 00 25 15 1F 2F 0B 84 0C 01 4D 00 1C 00 00 .O.%../....M....

registers:

R00: 0x00000000    R01: 0x00000000    R02: 0x00000000    R03: 0x00000000
R04: 0x00000000    R05: 0x00000000    R06: 0x00000000    R07: 0x00000000
R08: 0x00000000    R09: 0x00000000    R10: 0x00000000    R11: 0x00000000
R12: 0x00000010    R13: 0x00000000    R14: 0x00010000    R15: 0x00000019

STATUS = 1
 PC = 0x0019  SP = 0x10000

zero flag is set.
negative flag is cleared.
carry flag is cleared.

stack: empty
```

