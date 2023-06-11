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

## External Variable VarServer interface

For any TCC script that needs to access the VarServer you will need to run
the virtual machine with the
[libvarvm.so](https://github.com/tjmonk/tcc/blob/main/libvarvm/README.md).

The example below shows how the
[exttest.c](https://github.com/tjmonk/tcc/blob/main/tcc/test/exttest.c) sample
can be run using the VarServer to provide the external variables.

Note: When accessing a VarServer variable containing a forward slash (e.g.
/sys/test/a), this is represented as __sys__test__a in the tcc script.  i.e.
replace forward slashes with two underscores.

### Start the VarServer

```
varserver &
```

### Create the variables we need for the sample

```
mkvar -t uint16 -n /sys/test/a
mkvar -t uint32 -n /sys/test/b
mkvar -t float -n /sys/test/f
mkvar -t uint16 -n /sys/test/delay
setvar /sys/test/a 10
setvar /sys/test/f 3.1415926535
setvar /sys/test/b 1024
setvar /sys/test/delay 1000
```

### Build the sample

```
mkdir -p build/samples
tcc test/exttest.c > build/samples/exttest.v
vasm build/samples/exttest.v -o build/samples/exttest
```

### Run the sample with the external variables library

```
vexe -L libvarvm.so build/samples/exttest
```

```
count=0
This is a test
delay=1000
This is another test
delay=1000
/sys/test/delay=1000
x = 10
/SYS/TEST/A=10
/SYS/TEST/A=11
/SYS/TEST/B=1024
/SYS/TEST/B=1025
/SYS/TEST/F=3.141593
/SYS/TEST/F=4.141593
count=1
This is a test
This is another test
delay=1000
/sys/test/delay=1000
x = 11
/SYS/TEST/A=11
/SYS/TEST/A=12
/SYS/TEST/B=1025
/SYS/TEST/B=1026
/SYS/TEST/F=4.141593
/SYS/TEST/F=5.141593
count=2
This is a test
This is another test
delay=1000
/sys/test/delay=1000
x = 12
/SYS/TEST/A=12
/SYS/TEST/A=13
/SYS/TEST/B=1026
/SYS/TEST/B=1027
/SYS/TEST/F=5.141593
/SYS/TEST/F=6.141593
count=3
This is a test
This is another test
delay=1000
/sys/test/delay=1000
x = 13
/SYS/TEST/A=13
/SYS/TEST/A=14
/SYS/TEST/B=1027
/SYS/TEST/B=1028
/SYS/TEST/F=6.141593
/SYS/TEST/F=7.141593
count=4
This is a test
This is another test
delay=1000
/sys/test/delay=1000
x = 14
/SYS/TEST/A=14
/SYS/TEST/A=15
/SYS/TEST/B=1028
/SYS/TEST/B=1029
/SYS/TEST/F=7.141593
/SYS/TEST/F=8.141593
count=5
This is a test
This is another test
delay=1000
/sys/test/delay=1000
x = 15
/SYS/TEST/A=15
/SYS/TEST/A=16
/SYS/TEST/B=1029
/SYS/TEST/B=1030
/SYS/TEST/F=8.141593
/SYS/TEST/F=9.141593
```






