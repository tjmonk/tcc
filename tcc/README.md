# tcc

Tiny C Compiler

## Overview

The Tiny C Compiler compiles a simple c-like scripting language into
an assembly program which is suitable for assembling into a binary
which can be run on the Virtual Machine.

## Build

```
./build.sh
```

## Sample TCC Scripts

The table below shows some sample TCC scripts that you can build and run.

| Script | Description |
|---|---|
| [chartest.c](https://github.com/tjmonk/tcc/blob/main/tcc/test/chartest.c) | Character and String manipulation |
| [comptest.c](https://github.com/tjmonk/tcc/blob/main/tcc/test/comptest.c) | Floating Point variable comparison |
| [externs.c](https://github.com/tjmonk/tcc/blob/main/tcc/test/externs.c) | External Variable Referencing |
| [extstr.c](https://github.com/tjmonk/tcc/blob/main/tcc/test/extstr.c) | External Variable Referencing |
| [exttest.c](https://github.com/tjmonk/tcc/blob/main/tcc/test/exttest.c) | External Variable Referencing |
| [fact.c](https://github.com/tjmonk/tcc/blob/main/tcc/test/fact.c) | Factorial Generation |
| [floattest.c](https://github.com/tjmonk/tcc/blob/main/tcc/test/floattest.c) | Floating Point variable manipulation |
| [floattest2.c](https://github.com/tjmonk/tcc/blob/main/tcc/test/floattest2.c) | Floating Point expressions |
| [fread.c](https://github.com/tjmonk/tcc/blob/main/tcc/test/fread.c) | File Reading |
| [fwrite.c](https://github.com/tjmonk/tcc/blob/main/tcc/test/fwrite.c) | File Writing |
| [notify.c](https://github.com/tjmonk/tcc/blob/main/tcc/test/notify.c) | External Variable Notifications |
| [or_equals.c](https://github.com/tjmonk/tcc/blob/main/tcc/test/or_equals.c) | Or-Equals operator testing |
| [primes.c](https://github.com/tjmonk/tcc/blob/main/tcc/test/primes.c) | Prime Number Generator |
| [sort.c](https://github.com/tjmonk/tcc/blob/main/tcc/test/sort.c) | Arrays and Number sorting |
| [strtest.c](https://github.com/tjmonk/tcc/blob/main/tcc/test/strtest.c) | String Testing |
| [switchtest.c](https://github.com/tjmonk/tcc/blob/main/tcc/test/switchtest.c) | Switch Testing |
| [system.c](https://github.com/tjmonk/tcc/blob/main/tcc/test/system.c) | System() command execution |
| [test1.c](https://github.com/tjmonk/tcc/blob/main/tcc/test/test1.c) | Looping and basic string output |
| [test2.c](https://github.com/tjmonk/tcc/blob/main/tcc/test/test2.c) | Simple Variable arithmetic |
| [timer.c](https://github.com/tjmonk/tcc/blob/main/tcc/test/timer.c) | Timer Manipulation |

## Compiling and Running a script

The instructions below show how to compile, assemble, and run
a basic script.

### Compile to VM Assembly Language

```
tcc test/fact.c > fact.v
```

### Assemble to binary program
```
vasm fact.v -o fact.bin
```

```
inputFile = fact.v
assembly done
```

### Run the binary using the Virtual Machine

```
vexe fact.bin
```

```
factorial generation

1 factorial is 1
2 factorial is 2
3 factorial is 6
4 factorial is 24
5 factorial is 120
```

