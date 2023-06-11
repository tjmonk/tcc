# libvarvm

## Overview

The libvarvm library implements an external variable interface between
the [libvmcore](https://github.com/tjmonk/tcc/blob/main/libvmcore/README.md)
library and the
[VarServer](https://github.com/tjmonk/varserver/blob/main/README.md).

This allows assembly language programs written for the Virtual Machine
to access and manipulate VarServer variables.

The libvarvm library provides external variable interfaces for the
following instructions:

| Operation | Description |
|---|---|
| EXT | Get the handle of an external variable given its name |
| GET | Get the value of an external variable |
| SET | Set the value of an external variable |
| NFY | Request an external variable notification |
| WFS | Wait for a signal associated with an external variable |
| EVS | External Variable Validation Start |
| EVE | External Variable Validation End |
| OPS | Open Print Session |
| CPS | Close Print Session |

## Build

```
./build.sh
```

## Examples

Refer to the [vasm](https://github.com/tjmonk/tcc/tree/main/vasm/test) samples
to see how the external variable interface is used. The following examples
are relevant:

- [validate.v](https://github.com/tjmonk/tcc/blob/main/vasm/test/validate.v)
- [render.v](https://github.com/tjmonk/tcc/blob/main/vasm/test/render.v)

