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

#ifndef PARSEINFO_H
#define PARSEINFO_H

/*==============================================================================
        Includes
==============================================================================*/
#include <stdint.h>
#include <stddef.h>

/*==============================================================================
        Public definitions
==============================================================================*/

/*! numeric types to determine which kind of numeric encoding to perform */
typedef enum eNumType
{
    eINTEGER,
    eFLOAT,
    eHEXADECIMAL
} teNumType;

/*! supported datatypes */
typedef enum eParseType
{
    eUNKNOWN_PARSE_TYPE=0,
    eUINT8,
    eSINT8,
    eUINT16,
    eSINT16,
    eUINT32,
    eSINT32,
    eFLOAT32,
    eLABEL,
    eSTRING,
    eCHAR,
    eREGISTER,
    eOP
} teParseType;

/*! structure returned by the lexer */
typedef struct zParseInfo
{
    teParseType type;
    uint8_t n;
    uint8_t width;
    union
    {
        float fVal;
        uint32_t ulVal;
        int32_t slVal;
        uint16_t uiVal;
        int16_t siVal;
        uint8_t ucVal;
        int8_t scVal;
        char *pStrVal;
        uint8_t regnum;
        uint8_t op;
    } value;
} tzParseInfo;

/*==============================================================================
        Public Function declarations
==============================================================================*/

tzParseInfo AllocString( teParseType type,
                         char *label,
                         size_t length,
                         bool capitalize );

tzParseInfo EncodeOp( char *operator_name,
                      size_t length,
                      int lineno,
                      uint8_t operator_value );

tzParseInfo EncodeValue( char *valueText,
                         teNumType numType,
                         int lineno );

tzParseInfo EncodeChar( char *valueText,
                        size_t length ,
                        int lineno );

tzParseInfo GetRegister( char *regdef,
                         int line_number );

int copystring( tzParseInfo *pParseInfo,
                uint8_t *destination );

void CheckParseInfo( uint8_t *instptr,
                     tzParseInfo *pParseInfo1,
                     tzParseInfo *pParseInfo2,
                     int lineno );

void storeValue( tzParseInfo *pParseInfo,
                 uint8_t *memory,
                 uint16_t address,
                 int lineno );

#endif
