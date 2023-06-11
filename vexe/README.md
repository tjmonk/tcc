# VEXE Virtual Machine Executor

## Overview

The vexe utility is a an executor for the virtual machine.  It executes binary
images that have been assembled using the
[vasm](https://github.com/tjmonk/tcc/blob/main/vasm/README.md) or
[vm](https://github.com/tjmonk/tcc/blob/main/vm/README.md) commands.

## Command Line Arguments

```
usage: vexe [-c core size]
            [-s stack size]
            [-h]
            [-v]
            [-L externals lib name]
            <binary image>
```

| Argument | Description | Default Value |
| --- | --- | --- |
| -c | specify the size of the VM core in bytes | 65536 |
| -s | specify the size of the VM stack in bytes | 4096 |
| -h | display help for command usage | |
| -L | specify the external variables library (e.g. libvarvm.so) |

For more control of the execution and enhanced debugging support
see the [vm](https://github.com/tjmonk/tcc/blob/main/vm/README.md) command.

## Build

```
./build.sh
```

## Assemble and run a sample program

```
vasm test/hw.v -o test/hw.bin
hd test/hw.bin
vexe test/hw.bin
```

```
hd test/hw.bin
vexe test/hw.bin
inputFile = test/hw.v
assembly done
00000000  4d 00 11 68 65 6c 6c 6f  2c 20 77 6f 72 6c 64 0a  |M..hello, world.|
00000010  00 43 0c 00 03 54 00 19  1b 83 0b 00 a1 bc 84 0b  |.C...T..........|
00000020  00 4f 00 25 15 1f 2f 0b  84 0c 01 4d 00 1c        |.O.%../....M..|
0000002e
hello, world
```

