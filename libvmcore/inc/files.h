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
#ifndef FILES_H
#define FILES_H

/*==============================================================================
        Includes
==============================================================================*/

#include <stdint.h>

/*==============================================================================
        Public definitions
==============================================================================*/

#ifndef EOK
#define EOK 0
#endif

/*==============================================================================
        Public function declarations
==============================================================================*/

void InitFiles( void );
int SetExternWriteFileDescriptor( int fd, char mode );
int ClearExternFileDescriptor( int fd );
int SetActiveFileDescriptor( int fd );
int OpenFileDescriptor( int stringID, char mode, int *fd );
int CloseFileDescriptor( int fd );
int WriteString( char *str );
int WriteNum( int n );
int WriteFloat( float f );
int WriteChar( char c );
int ReadNum( int *n );
int ReadChar( char *c );

#endif
