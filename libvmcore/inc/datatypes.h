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

#ifndef DATATYPES_H
#define DATATYPES_H

/*==============================================================================
        Public definitions
==============================================================================*/

/*! instruction operates on uint8_t data */
#define BYTE	0x80

/*! instruction operates on uint16_t data */
#define WORD    0x40

/*! instruction operates on float32 data */
#define FLOAT32 0xC0

/*! instruction operates on uint32_t data (default) */
#define LONG    0x00

/*! instruction operates on a handle to external data */
#define HANDLE  0x60

/*! instruction involves only registers */
#define MODE_REG	0x20

/*! instruction involves a variable length constant or memory addr */
#define MODE_CONST	0x00

#endif
