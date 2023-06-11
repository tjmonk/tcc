# tcc
Tiny C Compiler and Virtual Machine

## Overview

The TCC component provides a Virtual Machine, Assembler, and a C-like
scripting language which are capable of running micro-services which
interface with the
[VarServer](https://github.com/tjmonk/varserver/blob/main/README.md).

Programs can be written in the Virtual Machine assembly language and
then assembled and run on the Virtual Machine using the
[vm](https://github.com/tjmonk/tcc/blob/main/vm/README.md),
[vasm](https://github.com/tjmonk/tcc/blob/main/vasm/README.md),
and [vexe](https://github.com/tjmonk/tcc/blob/main/vexe/README.md) utilities.

Alternatively, micro-service scripts can be written in the C-like scripting
language and compiled to assembly using the
[tcc](https://github.com/tjmonk/tcc/blob/main/tcc/README.md) compiler.

## Build

The build script compiles and installs the following components:

- [libvarvm](https://github.com/tjmonk/tcc/blob/main/libvarvm/README.md) : varserver external variable interface library
- [libvmasm](https://github.com/tjmonk/tcc/blob/main/libvmasm/README.md) : virtual machine assembler library
- [libvmcore](https://github.com/tjmonk/tcc/blob/main/libvmcore/README.md) : virtual machine core library
- [vasm](https://github.com/tjmonk/tcc/blob/main/vasm/README.md) : virtual machine assembler
- [vexe](https://github.com/tjmonk/tcc/blob/main/vexe/README.md) : virtial machine binary executor
- [vm](https://github.com/tjmonk/tcc/blob/main/vm/README.md) : virtual machine utility
- [tcc](https://github.com/tjmonk/tcc/blob/main/tcc/README.md) : tiny C compiler

Build all the subcomponents with this build script.

```
./build.sh
```

## Examples

This repository contains [VM assembly language samples](https://github.com/tjmonk/tcc/tree/main/vasm/test) and [C script samples](https://github.com/tjmonk/tcc/tree/main/tcc/test).

