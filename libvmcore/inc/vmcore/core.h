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

#ifndef CORE_H
#define CORE_H

/*==============================================================================
        Includes
==============================================================================*/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/*==============================================================================
        Public definitions
==============================================================================*/

#define HNOP     0x00
#define HLOD     0x01
#define HSTR     0x02
#define HMOV     0x03
#define HADD     0x04
#define HSUB     0x05
#define HMUL     0x06
#define HDIV     0x07
#define HAND     0x08
#define HOR      0x09
#define HNOT     0x0A
#define HSHR	 0x0B
#define HSHL	 0x0C
#define HJMP     0x0D
#define HJZR     0x0E
#define HJNZ     0x0F
#define HJNE     0x10
#define HJPO     0x11
#define HJCA     0x12
#define HJNC     0x13
#define HCAL     0x14
#define HRET     0x15
#define HCMP	 0x16
#define HTOF     0x17
#define HTOI     0x18
#define HPSH     0x19
#define HPOP     0x1A
#define HHLT     0x1B
#define HEXT     0x1C
#define HGET     0x1D
#define HSET     0x1E
#define HNEXT     0x1F
#define HRMAXINST 0x01F

#define HOPS   0x00
#define HCPS   0x01
#define HWRS   0x02
#define HCSB   0x03
#define HZSB   0x04
#define HWSB   0x05
#define HASS   0x06
#define HASB   0x07
#define HASN   0x08
#define HASC   0x09
#define HASF   0x0A
#define HRDC   0x0B
#define HRDN   0x0C
#define HWRF   0x0D
#define HWRN   0x0E
#define HWRC   0x0F
#define HDLY   0x10
#define HSTM   0x11
#define HCTM   0x12
#define HNFY   0x13
#define HWFS   0x14
#define HEVS   0x15
#define HEVE   0x16
#define HSBL   0x17
#define HSBO   0x18
#define HSCO   0x19
#define HGCO   0x1A
#define HOFD   0x1B
#define HCFD   0x1C
#define HSFD   0x1D
#define HEXE   0x1E

#define HMDUMP 0x00
#define HRDUMP 0x01

#define HDAT   0xA4

typedef struct zCore tzCore;

/*==============================================================================
        Public function declarations
==============================================================================*/

tzCore *CORE_fnCreate( size_t max_program_size, size_t max_stack_size );
uint8_t *CORE_fnMemory( tzCore *pCore );
size_t CORE_fnSize( tzCore *pCore );
size_t CORE_fnStackSize( tzCore *pCore );
void CORE_fnDump( tzCore *pCore);
void CORE_fnDumpRegisters( tzCore *pCore, FILE *fp);
bool CORE_fnSave( tzCore *pCore, char *outputFile );
void CORE_fnDumpMemory( tzCore *pCore,
                        uint32_t address,
                        uint32_t length,
                        FILE *fp );
void CORE_fnDumpStack(tzCore *pCore, FILE *fp);
bool CORE_fnLoad( tzCore *pCore, char *programFile );
bool CORE_fnExecute( tzCore *pCore );
void CORE_fnSetProgramSize( tzCore *pCore, size_t programSize );
size_t CORE_fnGetProgramSize( tzCore *pCore );
int CORE_fnInitExternalsLib( tzCore *pCore, char *libname );
int CORE_fnShutdownExternalsLib( tzCore *pCore );

#endif
