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

/*!
 * @defgroup strbuf String Buffer Manager
 * @brief Manage String Buffers for the Virtual Machine
 * @{
 */

/*============================================================================*/
/*!
@file strbuf.c

    String Buffer Manager

    The String Buffer Manager manages string buffers for the Virtual Machine.

    String buffers support creating, deleting, and appending content.

*/
/*============================================================================*/

/*==============================================================================
        Includes
==============================================================================*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "strbuf.h"
#include "files.h"

/*==============================================================================
        Private Definitions
==============================================================================*/

/*! initial starting size of the string buffer */
#define BUFSIZE 256

/*! The tzStringBuffer object defines the attributes of a string buffer */
typedef struct zStringBuffer
{
	/*! string buffer identifier */
	int id;

	/*! call stack level */
	int level;

	/*! string buffer write location (for append operations) */
	size_t offset;

	/*! string buffer read/write location for character operations */
	size_t rwOffset;

	/*! total size of the string buffer */
	size_t size;

	/*! pointer to the memory buffer */
	char *pBuffer;

	/*! pointer to the next string buffer in the list */
	struct zStringBuffer *pNext;

} tzStringBuffer;

/*==============================================================================
        File Scoped Variables
==============================================================================*/

/*! pointer to the first string buffer */
static tzStringBuffer *pFirst = NULL;

/*! pointer to the last string buffer */
static tzStringBuffer *pFreeList = NULL;

/* current call stack level */
static int level = 0;

/*==============================================================================
        Private Function Declarations
==============================================================================*/

tzStringBuffer *stringbuffer_fnFind( int id );
void stringbuffer_fnAppend( tzStringBuffer *p, char *str );

/*==============================================================================
        Public Function Definitions
==============================================================================*/

/*============================================================================*/
/*  STRINGBUFFER_fnSetLevel                                                   */
/*!
    Set the current call stack level for string buffer creation

    The STRINGBUFFER_fnSetLevel function sets the current call stack level
    which is used when creating string buffers.

    @param[in]
       l
            current call stack level

==============================================================================*/
void STRINGBUFFER_fnSetLevel(int l)
{
	level = l;
}

/*============================================================================*/
/*  STRINGBUFFER_fnCreate                                                     */
/*!
    Create a new string buffer

    The STRINGBUFFER_fnCreate function creates a new string buffer and
    populates the default string buffer object attributes.  It is then
    added to the string buffer list.

    @param[in]
       id
            string buffer identifier

    @retval true string buffer created ok
    @retval false string buffer could not be created

==============================================================================*/
bool STRINGBUFFER_fnCreate( int id )
{
	tzStringBuffer *p = NULL;

	/* search for an existing StringBuffer which was previously freed */
	if( pFreeList != NULL )
	{
		p = pFreeList;
		pFreeList = p->pNext;
		p->pNext = NULL;
	}

	if( p == NULL )
	{
		/* allocate memory for a new string buffer */
		p = malloc( sizeof( tzStringBuffer ));
		if( p != NULL )
		{
			/* allocate memory for the string content */
			p->pBuffer = malloc( BUFSIZE );
			if( p->pBuffer != NULL )
			{
				/* populate the new string buffer */
				p->id = id;
				p->offset = 0L;
				p->level = level;
				p->size = BUFSIZE;

				/* append the string buffer to the front of the list */
				p->pNext = pFirst;
				pFirst = p;
			}
			else
			{
				fprintf( stderr,
                         "Cannot allocate memory for the string buffer\n");

				free( p );
				p = NULL;
			}
		}
		else
		{
			fprintf(stderr, "Cannot allocate memory for the string buffer\n");
		}
	}

	return ( p != NULL ) ? true : false;
}

/*============================================================================*/
/*  STRINGBUFFER_fnAppendChar                                                 */
/*!
    Append a character to a string buffer

    The STRINGBUFFER_fnAppendChar function appends the specified character
    to the specified string buffer.

    @param[in]
       id
            string buffer identifier

    @param[in]
        c
            character to append

==============================================================================*/
void STRINGBUFFER_fnAppendChar( int id, char c )
{
	tzStringBuffer *p;
	char buf[2];

	buf[0] = c;
	buf[1] = 0;

	p = stringbuffer_fnFind( id );
	if( p != NULL )
	{
		stringbuffer_fnAppend(p, buf);
	}
}

/*============================================================================*/
/*  STRINGBUFFER_fnAppendNumber                                               */
/*!
    Append a 32-bit integer number to a string buffer

    The STRINGBUFFER_fnAppendNumber function appends the specified 32-bit
    integer number as a string to the specified string buffer.

    @param[in]
       id
            string buffer identifier

    @param[in]
        number
            32-bit integer to append to the string buffer

==============================================================================*/
void STRINGBUFFER_fnAppendNumber( int id, int32_t number )
{
	char numstring[20];
	tzStringBuffer *p;

	sprintf(numstring, "%d", number );

	p = stringbuffer_fnFind( id );
	if( p != NULL )
	{
		stringbuffer_fnAppend(p, numstring);
	}
}

/*============================================================================*/
/*  STRINGBUFFER_fnAppendFloat                                                */
/*!
    Append a 32-bit IEEE754 floating point number to a string buffer

    The STRINGBUFFER_fnAppendFloat function appends the specified 32-bit
    IEEE754 floating point number as a string to the specified string buffer.

    @param[in]
       id
            string buffer identifier

    @param[in]
        number
            32-bit integer to append to the string buffer

==============================================================================*/
void STRINGBUFFER_fnAppendFloat( int id, float number )
{
	char numstring[32];
	tzStringBuffer *p;

	sprintf(numstring, "%f", number );

	p = stringbuffer_fnFind( id );
	if( p != NULL )
	{
		stringbuffer_fnAppend(p, numstring);
	}
}

/*============================================================================*/
/*  STRINGBUFFER_fnAppendString                                               */
/*!
    Append a string to a string buffer

    The STRINGBUFFER_fnAppendString function appends the specified string
    to the specified string buffer.

    @param[in]
       id
            string buffer identifier

    @param[in]
        string
            pointer to the string to append

==============================================================================*/
void STRINGBUFFER_fnAppendString( int id, char *string )
{
	tzStringBuffer *p;

	p = stringbuffer_fnFind( id );
	if( p != NULL )
	{
		stringbuffer_fnAppend(p, string);
	}
}

/*============================================================================*/
/*  STRINGBUFFER_fnAppendBuffer                                               */
/*!
    Append a string buffer to a string buffer

    The STRINGBUFFER_fnAppendBuffer function appends the specified string
    buffer to the specified string buffer.

    @param[in]
       dest_id
            target string buffer identifier

    @param[in]
        src_id
            identifier of the source string buffer to append

==============================================================================*/
void STRINGBUFFER_fnAppendBuffer( int dest_id, int src_id )
{
	tzStringBuffer *pDst;
	tzStringBuffer *pSrc;

	pDst = stringbuffer_fnFind( dest_id );
	pSrc = stringbuffer_fnFind( src_id );
	if( ( pSrc != NULL ) && ( pDst != NULL ) )
	{
		stringbuffer_fnAppend( pDst, pSrc->pBuffer );
	}
}

/*============================================================================*/
/*  STRINGBUFFER_fnClear                                                      */
/*!
    Clear a string buffer

    The STRINGBUFFER_fnClear function clears the specified string
    buffer.  The string buffer still exists, but it has no content.

    @param[in]
       id
            string buffer identifier

==============================================================================*/
void STRINGBUFFER_fnClear( int id )
{
	tzStringBuffer *p;

	p = stringbuffer_fnFind( id );
	if( p != NULL )
	{
		if( p->pBuffer != NULL )
		{
			*(p->pBuffer) = '\0';
			p->offset = 0;
		}
	}
}

/*============================================================================*/
/*  STRINGBUFFER_fnWrite                                                      */
/*!
    Write a string buffer

    The STRINGBUFFER_fnWrite function writes the content of the specified
    string buffer to the specified FILE *

    @param[in]
       fp
            output FILE *

    @param[in]
        id
            string buffer identifier

==============================================================================*/
void STRINGBUFFER_fnWrite( FILE *fp, int id )
{
	tzStringBuffer *p;

	p = stringbuffer_fnFind( id );
	if( p != NULL )
	{
		if( p->pBuffer != NULL )
		{
            WriteString( p->pBuffer );
		}
	}
}

/*============================================================================*/
/*  STRINGBUFFER_fnGet                                                        */
/*!

    Get a pointer to a string buffer string

    The STRINGBUFFER_fnGet function gets a pointer to the string containined
    in the specified string buffer.

    @param[in]
        id
            string buffer identifier

    @retval pointer to the string buffer string
    @retval NULL if the string buffer is not found

==============================================================================*/
char *STRINGBUFFER_fnGet( int id )
{
	tzStringBuffer *p;
	char *buf = NULL;

	p = stringbuffer_fnFind( id );
	if( p != NULL )
	{
		buf = p->pBuffer;
	}
	return buf;
}

/*============================================================================*/
/*  STRINGBUFFER_fnFree                                                       */
/*!

    Free all string buffers at a specified scope

    The STRINGBUFFER_fnFree function frees all string buffers at the
    specified scope level.

    @param[in]
        level
            function scope level

==============================================================================*/
void STRINGBUFFER_fnFree( int level )
{
	/* clear all string buffers at the current level
	   and decrement the level */
	tzStringBuffer *p = pFirst;

	while( p != NULL )
	{
		if( p->level == level )
		{
			p->offset = 0;
			p->id = 0;

			/* pop the string buffer from the in-use list */
			pFirst = p->pNext;

			/* push the string buffer to the head of the free list */
			p->pNext = pFreeList;
			pFreeList = p;

			/* select the next String Buffer to process */
			p = pFirst;
		}
		else
		{
			/* do not process any other level */
			break;
		}
	}

	if( level > 0 )
	{
		level--;
	}
}

/*============================================================================*/
/*  STRINGBUFFER_fnGetLength                                                  */
/*!

    Get the string length of the specified string buffer

    The STRINGBUFFER_fnGetLength gets the length of the specified string
    buffer.

    @param[in]
        id
            string buffer identifier

    @retval length of the specified string buffer
    @retval 0 if the string buffer is not found

==============================================================================*/
size_t STRINGBUFFER_fnGetLength( int id )
{
	tzStringBuffer *pStringBuffer;
	size_t len = 0;

	pStringBuffer = stringbuffer_fnFind( id );
	if( pStringBuffer != NULL )
	{
		len = pStringBuffer->offset;
	}

	return len;
}

/*============================================================================*/
/*  STRINGBUFFER_fnSetRWOffset                                                */
/*!

    Set the read and write offset in the specified string buffer

    The STRINGBUFFER_fnSetRWOffset sets the read and write offset in the
    specified string buffer.

    @param[in]
        id
            string buffer identifier

    @param[id]
        offset
            offset into the string buffer (0-based)

==============================================================================*/
void STRINGBUFFER_fnSetRWOffset( int id, uint32_t offset )
{
	tzStringBuffer *pStringBuffer;

	pStringBuffer = stringbuffer_fnFind( id );
	if( pStringBuffer != NULL )
	{
		if( offset < pStringBuffer->offset )
		{
			pStringBuffer->rwOffset = offset;
		}
	}
}

/*============================================================================*/
/*  STRINGBUFFER_fnGetCharAtOffset                                            */
/*!

    Get the character at the current read/write offset in the string buffer

    The STRINGBUFFER_fnGetCharAtOffset gets the character at the current
    read/write offset in the string buffer.

    @param[in]
        id
            string buffer identifier

    @retval character at the offset in the string buffer
    @retval 0 if the offset is invalid or the string buffer doesn't exist

==============================================================================*/
char STRINGBUFFER_fnGetCharAtOffset( int id )
{
	tzStringBuffer *pStringBuffer;
	size_t offset = 0L;
	char c = '\0';

	pStringBuffer = stringbuffer_fnFind( id );
	if( pStringBuffer != NULL )
	{
		offset = pStringBuffer->rwOffset;
		if( offset < pStringBuffer->offset )
		{
			c = pStringBuffer->pBuffer[offset];
		}
	}

	return c;
}

/*============================================================================*/
/*  STRINGBUFFER_fnSetCharAtOffset                                            */
/*!

    Set the character at the current read/write offset in the string buffer

    The STRINGBUFFER_fnSetCharAtOffset sets the character at the current
    read/write offset in the string buffer to the specified value.
    If the string buffer is not found, or the offset is invalid, then
    no action is taken.

    @param[in]
        id
            string buffer identifier

    @param[in]
        c
            character to write

==============================================================================*/
void STRINGBUFFER_fnSetCharAtOffset( int id, char c )
{
	tzStringBuffer *pStringBuffer;
	size_t offset;

	pStringBuffer = stringbuffer_fnFind( id );
	if( pStringBuffer != NULL )
	{
		offset = pStringBuffer->rwOffset;
		if( offset < pStringBuffer->offset )
		{
			/* store the character into the string */
			pStringBuffer->pBuffer[offset] = c;

			/* check for NULL */
			if( c == 0 )
			{
				/* truncate the string */
				pStringBuffer->offset = offset;
			}
		}
	}
}

/*==============================================================================
        Public Function Definitions
==============================================================================*/

/*============================================================================*/
/*  stringbuffer_fnFind                                                       */
/*!

    Find the specified string buffer

    The stringbuffer_fnFind function searches through all the string buffers
    looking for the string buffer with the specified id.

    @param[in]
        id
            string buffer identifier

    @retval pointer to the tzStringBuffer that was found
    @retval NULL no tzStringBuffer object found.

==============================================================================*/
tzStringBuffer *stringbuffer_fnFind( int id )
{
	tzStringBuffer *p = pFirst;

	while( p != NULL )
	{
		if( p->id == id )
		{
			return p;
		}

		p = p->pNext;
	}

	return NULL;
}

/*============================================================================*/
/*  stringbuffer_fnAppend                                                     */
/*!

    Append a string to a string buffer

    The stringbuffer_fnAppend function appends the specified string to the
    specified string buffer.  No action is taken if the string buffer or
    source string are invalid.

    @param[in]
        p
            pointer to the tzStringBuffer object to append to

    @param[in]
        str
            pointer to the string to be appended

==============================================================================*/
void stringbuffer_fnAppend( tzStringBuffer *p, char *str )
{
	size_t len;
	size_t newsize;
	char *pBuffer;

	if( ( p == NULL ) || ( str == NULL ) )
	{
		return;

	}

	len = strlen( str );

	if( ( p->offset + len ) >= p->size )
	{
		/* we don't have enough space to append the string */
		/* we need to make the buffer bigger (in multiples of BUFSIZE ) */
		newsize = p->size;
		while( newsize <= ( p->offset + len ))
		{
			newsize += BUFSIZE;
		}

		p->pBuffer = realloc( p->pBuffer, newsize );
		if( p->pBuffer == NULL )
		{
			fprintf(stderr, "Unable to allocate buffer memory\n");
			return;
		}
	}

	/* append the string to the string buffer */
	strcpy( &(p->pBuffer[p->offset]), str );
	p->offset += len;

	/* null terminate the string buffer */
	p->pBuffer[p->offset] = '\0';
}

/*! @}
 * end of strbuf group */
