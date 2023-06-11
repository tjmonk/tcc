# VM Core Library

## Overview

The VM Core library provides the core functionality for managing the
core functionity of a 32-bit Virtual Machine. It manages:

- core VM memory
- VM stack
- VM registers
- Operator interaction
- File management
- String buffer management
- External variable interface
- Timers

It is used by the following applications:

- [vasm](https://github.com/tjmonk/tcc/blob/main/vasm/README.md)
- [vexe](https://github.com/tjmonk/tcc/blob/main/vexe/README.md)
- [vm](https://github.com/tjmonk/tcc/blob/main/vm/README.md)

## Virtual Machine Memory

The virtual machine allocates a memory buffer for for use by the VM core.
The memory buffer is divided into two regions, core memory and stack memory.
The stack starts at the top of memory and grows downwards.
The program starts at the bottom of memory and grows upwards.
The caller can specify the core and stack memory sizes to meet their
application needs.  By default the memory is 65536 bytes, and the
stack size is 4096 bytes, leaving 61440 bytes for the application code.

## Virtual Machine Data Types

Since the Virtual Machine is only 32-bits, it does not natively support
data types which are larger than this.  It also stores signed unsigned
data in the same underlying 32-bit storage type.  Beware of these limitations
when interfacing with the [VarServer](https://github.com/tjmonk/varserver)
via External Variables using the
[libvarvm](https://github.com/tjmonk/tcc/blob/main/libvarvm/README.md) library.

## Virtual Machine Registers

The virtual machine provides 16 32-bit registers [R0..R15].

- R15 is the program counter (PC).
- R14 is the stack pointer (SP).
- R0..R13 are general purpose registers that can be used by user programs.

32-bit IEEE754 Floating point registers are overlayed onto the 32-bit integer
register space.  The type conversion operations can convert values between
integer and floating point values.

## Virtual Machine Instruction Set

The VM Core implements the following Virtual Machine instruction set:

### Register operations

The register operations can be used to move data between registers, or from
memory to register, or register to memory.

| Mnemonic | Description | Assembly Langage Example(s) |
| --- | --- | --- |
| NOP | No operation | NOP |
| LOD | Load Register from Memory | LOD Ra, Rb ; [out]Ra=value, Rb=memory location |
| STR | Store Register to Memory | STR Ra, Rb; Ra=memory location, Rb=value |
| MOV | Move Register or value to Register| MOV Ra, Rb ; [out]Ra=value, Rb=srcval |
| CMP | Compare Register with Register or Value | CMP Ra, Rb ; compare Ra with Rb and update flags |

### Math operations

Arithmetic and bitwise math operations are supported between 2 registers.
NOT, SHR, and SHL are unary operators that act on a single register.

| Mnemonic | Description | Assembly Langage Example(s) |
| --- | --- | --- |
| ADD | Add Register or Value to Register | ADD Ra, Rb ; [out]Ra=Ra+Rb |
| SUB | Subtract Register or Value from Register | SUB Ra,Rb ; [out]Ra=Ra-Rb |
| MUL | Multiply Register with Register or Value | MUL Ra,Rb ; [out]Ra=Ra*Rb |
| DIV | Divide Register with Register or Value | DIV Ra,Rb ; [out]Ra=Ra/Rb |
| AND | Bitwise AND Register with Register | AND Ra, Rb ; [out]Ra=Ra & Rb |
| OR  | Bitwise OR Register with Register or Value | OR Ra, Rb ; [out]Ra=Ra or Rb |
| NOT | Bitwise NOT of Register | NOT Ra ; [out]Ra=~Ra |
| SHR | Right Shift Register | SHR Ra, n ; [out]Ra = Ra >> n |
| SHL | Left Shift Register | SHL Ra, n ; [out]Ra = Ra << n |

### Program Counter manipulation

The program counter can be manipulated to create loops and function calls.

| Mnemonic | Description | Assembly Langage Example(s) |
| --- | --- | --- |
| JMP | Jump to location | JMP addr ; addr=address to jump to |
| JZR | Jump to location if Z flag is set | JZR addr ; addr=address to jump to |
| JNZ | Jump to location if Z flag is not set | JNZ addr ; addr=address to jump to |
| JNE | Jump to location if N flag is set | JNE addr ; addr=address to jump to |
| JPO | Jump to location if N flag is not set | JPO addr ; addr=address to jump to |
| JCA | Jump to location if C flag is set | JCA addr ; addr=address to jump to |
| JNC | Jump to location if C flag is not set | JNC addr ; addr=address to jump to |
| CAL | Call a subroutine and save Program Counter of next instruction on the stack | CAL addr ; addr=address to jump to |
| RET | Return from subroutine and restore Program Counter | RET |
| HLT | Halt the Virtual Machine | HLT |

### Type conversion

Type conversion functions are provided to convert numbers between integer
and floating point.

| Mnemonic | Description | Assembly Langage Example(s) |
| --- | --- | --- |
| TOF | Convert Register to floating point | TOF Ra ; [out]Ra=(float)Ra |
| TOI | Convert Register to integer | TOI Ra ; [out]Ra=(int)Ra |

### Input/Output and File operations

Input and Output and File operations are provided to allow programs
to manipulate files and streams outside of the virtual machine.

| Mnemonic | Description | Assembly Langage Example(s) |
| --- | --- | --- |
| RDN | Read 32-bit number from the active file descriptor | RDN Ra ; [out]Ra=read number |
| RDC | Read a character from the active file descriptor | RCD Ra ; [out]Ra=read char |
| WRN | Write a 32-bit number to the active file descriptor | WRN Ra; Ra=number to write |
| WRC | Write a character to the active file descriptor | WRC Ra ; Ra=char to write |
| WRF | Write a 32-bit IEEE754 floating point number to the active file descriptor | WRF Ra; Ra=float to write |
| WRS | Write the string to the active file descriptor | WRS Ra ; Ra=address of string |
| OFD | Open File Descriptor | OFD Ra, Rb ; Ra=id of string buffer containing file name, Rb=open mode: one of: 'r', 'w', 'R', 'W'. [out]Ra=File descriptor |
| CFD | Close File Descriptor | CFD Ra ; Ra=file descriptor id |
| SFD | Select the active File Descriptor | SFD Ra ; Ra=file descriptor to make active |

### Stack Manipulation

Stack manipulation is used to preserve data during subroutine calls.  Data
is pushed onto the stack prior to the subroutine call, and pulled back off
the stack after the subroutine call.

| Mnemonic | Description | Assembly Langage Example(s) |
| --- | --- | --- |
| PSH | Push a Register onto the stack | PSH Ra ; push Ra onto the stack |
| POP | Pop a Register off the stack | POP Ra; [out]Ra retrieved from stack |

### Diagnostic Operations

Some diagnostic operations are supported to help debug virtual machine
assembly programs at run-time.

| Mnemonic | Description | Assembly Langage Example(s) |
| --- | --- | --- |
| RDUMP | Diagnostic dump of the registers | RDUMP ; dump registers |
| MDUMP | Diagnostic dump of the memory | MDUMP Ra, n ; Ra=start address, n=number of bytes to dump |

### External Variable Manipulation

These functions can interface with external variables managed via an external
library. If no external library is specified, default external variable
functions provided by the virtual machine will be used.  For an example of
how an external variable library is used, see the
[libvarvm](https://github.com/tjmonk/tcc/blob/main/libvarvm/README.md) library.

| Mnemonic | Description | Assembly Langage Example(s) |
| --- | --- | --- |
| EXT | Get the handle of an external variable given its name | EXT Ra ; Ra=address of name, [out]Ra=variable handle |
| GET | Get the value of an external variable | GET Ra,Rb ; Ra=string buffer id (for string get), Rb=variable handle, [out]Ra=value |
| SET | Set the value of an external variable | SET Ra,Rb ; Ra=variable handle, Rb=value or string buffer id (for string type) |

### String Buffer Operations

String buffers are a mechanism implemented by the Virtual Machine to construct
and manipulate string variables without needing to worry about memory
management.  String buffers can be referenced by their numeric ID.

| Mnemonic | Description | Assembly Langage Example(s) |
| --- | --- | --- |
| CSB | Create a string buffer |  CSB Ra ; [out]Ra=string buffer id |
| ZSB | Zero (clear) a string buffer | ZSB Ra ; Ra=string buffer id |
| WSB | Write a string buffer to the active file descriptor | WSB Ra ; Ra=string buffer id |
| ASN | Append 32-bit integer to a string buffer | ASN Ra, Rb ; Ra=string buffer, Rb=integer value|
| ASS | Append a string to a string buffer | ASS R2, R3 ; Ra=string buffer, Rb=string address |
| ASB | Append a string buffer to a string buffer | ASS Ra, Rb  Ra=destination, Rb=source |
| ASC | Append a character to a string buffer | ASC Ra, Rb ; Ra=string buffer id, Rc=character |
| ASF | Append a 32-bit floating point value to a string buffer | ASF Ra, Rb; Ra=string buffer id, Rb=floating point value |
| SBL | String Buffer Length | SBL Ra, Rb ; [out]Ra=length, Rb=string buffer id |
| SBO | String Buffer Offset | SBO Ra, Rb ; Ra=string buffer id, Rb=read/write offset |
| SCO | Set string buffer Character at Offset | SCO Ra, Rb ; Ra=string buffer id, Rb=character to write |
| GCO | Get string buffer Character at Offset | GCO Ra, Rb ; [out]Ra=character, Rb=string buffer id |

### Execute System Call Operations

The System Call Operations allow a virtual machine program to execute a
system call to the host system.  Use caution, this could be a cybersecurity
risk.

| Mnemonic | Description | Assembly Langage Example(s) |
| --- | --- | --- |
| EXE | Execute String Buffer | EXE Ra, Rb ;[out]Ra=command result, Rb=string buffer identifier for command to execute |

### Timer Functions

The timer operation facilitate the creation and deletion of repeating timers
and delays.

| Mnemonic | Description | Assembly Langage Example(s) |
| --- | --- | --- |
| DLY | Delay a specified number of milliseconds | DLY Ra ; Ra=number of milliseconds to delay |
| STM | Setup a timer | STM Ra, Rb ; Ra=timer id, Rb=delay in milliseconds |
| CTM | Clear a timer | CTM Ra ; Ra=timer id |

### Signal Handling for External Variables

These functions require an appropriate external variable library to be loaded
when the virtual machine executes.  See the [libvarvm](https://github.com/tjmonk/tcc/blob/main/libvarvm/README.md) library for an appropriate example which interfaces with the [VarServer](https://github.com/tjmonk/varserver).

| Mnemonic | Description | Assembly Langage Example(s) |
| --- | --- | --- |
| NFY | Request an external variable notification | NFY Ra, Rb ; Ra=external variable handle,Rb=notification type|
| WFS | Wait for a signal | WFS Ra, Rb ; [out]Ra=signal number, [out]Rb=signal id |
| EVS | External variable validation start | EVS Ra, Rb ; [out]Ra=variable handle, Rb=validation notification reference id received from WFS |
| EVE | External variable validation end | EVE Ra, Rb ; Ra=validation notification reference id, Rb= validation result (0=ok, non-zero= errno) |
| OPS | Open Print Session | OPS Ra, Rb ; Ra=print notification handle, [out]Ra=output file descriptor, [out]Rb=external variable handle |
| CPS | Close Print Session | CPS Ra, Rb ;Ra=print notification handle, Rb=output file descriptor |

## Build

The build generates the libvmcore.so shared object.

```
./build.sh
```

## Examples

Refer to the [vasm](https://github.com/tjmonk/tcc/blob/main/vasm/README.md)
utility to see assembly program samples which work with the Virtual Machine.
