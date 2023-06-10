/*==============================================================================
MIT License

Copyright (c) 2023 Trevor Monk

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
==============================================================================*/

/*==============================================================================
Registers

This module manages register allocation and deallocation.
R3-R13 are available for general purpose use by the compiler.
R0 contains return data for procedure calls.
R1 contains the current activation record pointer.
R2 reserved

==============================================================================*/

#ifndef REGISTERS_H
#define REGISTERS_H

/*==============================================================================
        Includes
==============================================================================*/

#include "SymbolTableManager.h"

/*==============================================================================
        Public Function Declarations
==============================================================================*/
int AllocReg(struct identEntry *idEntry, int regindex );
void UseReg( int reg );
void FreeReg( int reg );
void FreeTempReg( int reg );
bool HasIdentifier( int reg );

#endif