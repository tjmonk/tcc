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

#ifndef STRBUF_H
#define STRBUF_H

/*==============================================================================
        Includes
==============================================================================*/

#include <stdint.h>
#include <stdio.h>

/*==============================================================================
        Public function declarations
==============================================================================*/

void STRINGBUFFER_fnSetLevel( int level );
bool STRINGBUFFER_fnCreate( int id );
void STRINGBUFFER_fnClear( int id );
void STRINGBUFFER_fnAppendChar( int id, char c );
void STRINGBUFFER_fnAppendNumber( int id, int32_t number );
void STRINGBUFFER_fnAppendFloat( int id, float number );
void STRINGBUFFER_fnAppendString( int id, char *string );
void STRINGBUFFER_fnAppendBuffer( int dest_id, int src_id );
void STRINGBUFFER_fnWrite( FILE *fp, int id );
char *STRINGBUFFER_fnGet( int id );
void STRINGBUFFER_fnFree( int level );
size_t STRINGBUFFER_fnGetLength( int id );
void STRINGBUFFER_fnSetRWOffset( int id, uint32_t offset );
char STRINGBUFFER_fnGetCharAtOffset( int id );
void STRINGBUFFER_fnSetCharAtOffset( int id, char c );

#endif