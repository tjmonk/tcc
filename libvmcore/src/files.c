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
 * @defgroup files File Descriptor Manager
 * @brief Manage File Descriptors for the Virtual Machine
 * @{
 */

/*============================================================================*/
/*!
@file files.c

    File Descriptor Manager

    The File Descriptor Manager manages open files for the
    virtual machine.  Special file descriptors are:

    0 - standard input
    1 - standard output
    2 - standard error

    Other file descriptors are allocated automatically for files
    as they are opened
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
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include "files.h"
#include "strbuf.h"

/*==============================================================================
        Definitions
==============================================================================*/

/*! Maximum number of open files in the virtual machine */
#define MAX_OPEN_FILES ( 20 )

/*! File Descriptor object to associated the file descriptor and its mode */
typedef struct _FileDescriptor
{
    /*! file descriptor id */
    uint32_t fd;

    /*! file descriptor mode 'r' or 'w' */
    char mode;

} FileDescriptor;

/*==============================================================================
        Private function declarations
==============================================================================*/

static int Findfd( int fd );
static int ScanNumber( void );
static char GetMode( int fd );
static int GetFreeFDIndex( void );

/*==============================================================================
        File scoped variables
==============================================================================*/

/*! the currently active read file descriptor */
static int active_read_fd = STDIN_FILENO;

/*! the currently active write file descriptor */
static int active_write_fd = STDOUT_FILENO;

/*! number of open files */
static int numOpenFiles = 3;

/*! file descriptor storage */
FileDescriptor FILES[MAX_OPEN_FILES];

/*==============================================================================
        Function definitions
==============================================================================*/

/*============================================================================*/
/*  InitFiles                                                                 */
/*!
    Initialize the standard files

    Set up the file descriptor associations to the standard files

    0 = stdin
    1 = stdout
    2 = stderr

==============================================================================*/
void InitFiles( void )
{
    int i;

    memset( FILES, 0, sizeof( FILES ) );

    FILES[STDIN_FILENO].fd = 0;
    FILES[STDIN_FILENO].mode = 'r';

    FILES[STDOUT_FILENO].fd = 1;
    FILES[STDOUT_FILENO].mode = 'w';

    FILES[STDERR_FILENO].fd = 2;
    FILES[STDERR_FILENO].mode = 'w';

    numOpenFiles = 3;

    for( i=numOpenFiles ; i < MAX_OPEN_FILES; i++ )
    {
        FILES[i].fd = -1;
    }
}

/*============================================================================*/
/*  GetFreeFDIndex                                                            */
/*!
    Get the index of an available slot in the file descriptor list

    The GetFreeFDIndex function scans the file descriptor list looking for the
    first available slot.  (where fd == -1 )

    @retval index of free file descriptor
    @retval -1 if there are no free indexes

==============================================================================*/
static int GetFreeFDIndex( void )
{
    int idx;
    int result = -1;

    for ( idx = 3; idx < MAX_OPEN_FILES ; idx++ )
    {
        if ( FILES[idx].fd == -1 )
        {
            result = idx;
            break;
        }
    }

    return result;
}

/*============================================================================*/
/*  SetExternWriteFileDescriptor                                              */
/*!
    Set up an external file descriptor for writing

    The SetExternFileDescriptor function inserts a new external write
    file descriptor in the file descriptor list.

    @param[in]
        fd
            the new external write file descriptor

    @param[in]
        mode
            file descriptor mode

    @retval EOK the external file descriptor was added
    @retval EBADF the file descriptor is invalid
    @retval ENOSPC there is no space to store the file descriptor
    @retval EXIST the file descriptor already exists

==============================================================================*/
int SetExternWriteFileDescriptor( int fd, char mode )
{
    int result = EBADF;
    int i;
    int idx;

    /* validate the file descriptor */
    if ( fd > 0 )
    {
        /* confirm it does not already exist */
        if (Findfd( fd ) == -1 )
        {
            idx = GetFreeFDIndex();
            if( idx != -1 )
            {
                /* insert the new file descriptor */
                FILES[idx].fd = fd;
                FILES[idx].mode = mode;

                /* increment the number of file descriptors we are tracking */
                numOpenFiles++;

                /* indicate success */
                result = EOK;
            }
            else
            {
                /* no space available for the new file descriptor */
                result = ENOSPC;
            }
        }
        else
        {
            /* we cannot add a file descriptor that already exists */
            result = EEXIST;
        }
    }

    return result;
}

/*============================================================================*/
/*  ClearExternFileDescriptor                                                 */
/*!
    Clean up an external file descriptor

    The ClearExternFileDescriptor function cleans up the specified
    file descriptor reservation in the file descriptor list

    @param[in]
        fd
            the file descriptor to clean up

    @retval EOK the specified file descriptor was cleaned up
    @retval EBADF the specified file descriptor is invalid
    @retval ENOENT the specified file descriptor does not exists

==============================================================================*/
int ClearExternFileDescriptor( int fd )
{
    int result = EBADF;
    int idx;

    if ( fd > 0 )
    {
        idx = Findfd( fd );
        if ( idx != -1 )
        {
            /* free up the file descriptor */
            FILES[idx].fd = -1;
            FILES[idx].mode = 0;

            /* decrement the number of file descriptors we are tracking */
            numOpenFiles--;

            /* indicate success */
            result = EOK;
        }
        else
        {
            result = ENOENT;
        }
    }

    return result;
}

/*============================================================================*/
/*  SetActiveFileDescriptor                                                   */
/*!
    Set the active file descriptor

    The SetActiveFileDescriptor function selects the currently active
    file that the I/O functions will target.

    @param[in]
        fd
            the new active file descriptor

    @retval EOK the active file descriptor was selected
    @retval EBADF the file descriptor is invalid
    @retval ENOTSUP the specified mode is not supported
    @retval EINVAL invalid arguments

==============================================================================*/
int SetActiveFileDescriptor( int fd )
{
    int result = EBADF;
    int idx;

    if( ( fd > 0 ) &&
        ( fd < MAX_OPEN_FILES ) )
    {
        idx = Findfd( fd );
        if( idx != -1 )
        {
            if( tolower( FILES[idx].mode ) == 'r' )
            {
                active_read_fd = fd;
                result = EOK;
            }
            else if ( tolower( FILES[idx].mode ) == 'w' )
            {
                active_write_fd = fd;
                result = EOK;
            }
        }
    }

    return result;
}

/*============================================================================*/
/*  OpenFileDescriptor                                                        */
/*!
    Open a file give the file name in a stringbuffer

    The Open File Descriptor function opens a file whose pathspec is
    specified in a stringbuffer.

    @param[in]
        stringID
            specifies the id of the stringbuffer which contains the
            path specification for the file to be opened

    @param[in]
        mode
            the file access mode. One of: r, w, R, W

    @param[in,out]
        fd
            pointer to the location to store the newly opened file descriptor

    @retval EOK the file was opened
    @retval ENOSPC not enough file descriptors available
    @retval ENOENT file does not exist
    @retval EINVAL invalid arguments

==============================================================================*/
int OpenFileDescriptor( int stringID, char mode, int *fd )
{
    char *pFileName;
    int result = EINVAL;
    FILE *fp;
    int idx;
    mode_t openMode;

    if( ( fd != NULL ) &&
        ( ( mode == 'r' ) ||
          ( mode == 'w' ) ||
          ( mode == 'R' ) ||
          ( mode == 'W' ) ) )
    {
        /* get the name of the file to open */
        pFileName = STRINGBUFFER_fnGet( stringID );
        if( pFileName != NULL )
        {
            mode = tolower( mode );
            if( mode == 'r' )
            {
                openMode = O_RDONLY;
            }
            else if ( mode == 'w' )
            {
                openMode = O_WRONLY;
            }

            /* get the index for the new file descriptor */
            idx = GetFreeFDIndex();
            if( idx != -1 )
            {
                /* open the file */
                *fd = open( pFileName, openMode );
                if( *fd != -1 )
                {
                    /* store the file descriptor and open mode */
                    FILES[idx].fd = *fd;
                    FILES[idx].mode = mode;

                    /* increment the number of open files */
                    numOpenFiles++;

                    /* indicate success */
                    result = EOK;
                }
                else
                {
                    result = ENOENT;
                }
            }
            else
            {
                result = ENOSPC;
            }
        }
    }

    return result;
}

/*============================================================================*/
/*  CloseFileDescriptor                                                       */
/*!
    Close a file given its file descriptor

    The Close File Descriptor function closes the file with the specified
    file descriptor

    @param[in]
        fd
            the file descriptor of the file to close

    @retval EOK the file was closed
    @retval ENOENT file does not exist
    @retval EBADF Bad file descriptor

==============================================================================*/
int CloseFileDescriptor( int fd )
{
    int result = EBADF;
    int i;

    if( fd > 3 )
    {
        /* assume we dont have the file descriptor until we find it */
        result = ENOENT;

        for ( i=3; i<MAX_OPEN_FILES; i++ )
        {
            if ( FILES[i].fd == fd )
            {
                close( fd );
                FILES[i].fd = -1;
                FILES[i].mode = 0;

                /* decrement the number of open files */
                numOpenFiles--;

                result = EOK;
            }
        }
    }

    return result;
}

/*============================================================================*/
/*  WriteString                                                               */
/*!
    Write a string to the active output file descriptor

    The WriteString function writes the specified string to the active
    output file descriptor

    @param[in]
        str
            pointer to the nul terminated string to write

    @retval EOK the string was successfully written
    @retval EBADF invalid file descriptor
    @retval EINVAL invalid arguments

==============================================================================*/
int WriteString( char *str )
{
    int result = EINVAL;
    char mode;
    FILE *fp;

    if( str != NULL )
    {
        if ( active_write_fd != -1 )
        {
            dprintf( active_write_fd, "%s", str );
            result = EOK;
        }
        else
        {
            result = EBADF;
        }
    }

    return result;
}

/*============================================================================*/
/*  WriteNum                                                                  */
/*!
    Write a number to the active output file descriptor

    The WriteNum function writes the specified integer to the active
    output file descriptor taking into account the mode of the file.
    If the file is opened as a binary file with mode = 'W', then
    the number will be written as a binary value.  If the file is opened
    as a text file with mode = 'w', then the number will be written as
    a text value.

    @param[in]
        n
            the number to be written

    @retval EOK the number was successfully written
    @retval ENOTSUP improper file mode
    @retval EINVAL invalid arguments

==============================================================================*/
int WriteNum( int n )
{
    int result = EBADF;
    char mode;

    if( active_write_fd != -1 )
    {
        mode = GetMode( active_write_fd );
        if( mode == 0 )
        {
            mode = 'w';
        }

        if( mode == 'W' )
        {
            write( active_write_fd, &n, sizeof(int) );
            result = EOK;
        }
        else if( mode == 'w' )
        {
            dprintf( active_write_fd, "%d", n );
            result = EOK;
        }
        else
        {
            result = ENOTSUP;
        }
    }

    return result;
}

/*============================================================================*/
/*  WriteFloat                                                                */
/*!
    Write a floating point number to the active output file descriptor

    The WriteNum function writes the specified floating point number
    to the active output file descriptor taking into account the mode
    of the file.

    If the file is opened as a binary file with mode = 'W', then
    the number will be written as a binary value.  If the file is opened
    as a text file with mode = 'w', then the number will be written as
    a text value.

    @param[in]
        f
            the floating point number to be written

    @retval EOK the floating point number was successfully written
    @retval ENOTSUP improper file mode
    @retval EBADF bad file descriptor

==============================================================================*/
int WriteFloat( float f )
{
    int result = EBADF;
    char mode;
    FILE *fp;

    if( active_write_fd != -1 )
    {
        mode = GetMode( active_write_fd );
        if( mode == 0 )
        {
            mode = 'w';
        }

        if( mode == 'W' )
        {
            write( active_write_fd, &f, sizeof(float) );
            result = EOK;
        }
        else if( mode == 'w' )
        {
            dprintf( active_write_fd, "%f", f );
            result = EOK;
        }
        else
        {
            result = ENOTSUP;
        }
    }

    return result;
}

/*============================================================================*/
/*  WriteChar                                                                 */
/*!
    Write a character to the active output file descriptor

    The WriteChar function writes the specified character
    to the active output file descriptor taking into account the mode
    of the file.

    If the file is opened as a binary file with mode = 'W', then
    the character will be written as a binary value.  If the file is opened
    as a text file with mode = 'w', then the character will be written as
    a text value.

    @param[in]
        c
            the character to be written

    @retval EOK the character was successfully written
    @retval ENOTSUP improper file mode
    @retval EINVAL invalid arguments

==============================================================================*/
int WriteChar( char c )
{
    int result = EINVAL;
    char mode;
    FILE *fp;

    mode = GetMode( active_write_fd );
    if( mode == 0 )
    {
        mode = 'w';
    }
    if( active_write_fd != -1 )
    {
        if( mode == 'W' )
        {
            write( active_write_fd, &c, 1 );
            result = EOK;
        }
        else if( mode == 'w' )
        {
            dprintf( active_write_fd, "%c", c );
            result = EOK;
        }
        else
        {
            result = ENOTSUP;
        }
    }

    return result;
}

/*============================================================================*/
/*  ReadNum                                                                   */
/*!
    Read a number from the active input file descriptor

    The ReadNum function reads a number from the active input
    file descriptor taking into account the mode of the file.

    If the file is opened as a binary file with mode = 'R', then
    the number will be read as a binary value.  If the file is opened
    as a text file with mode = 'r', then the number will be read as
    a text value.

    @param[in,out]
        n
            pointer to the location to store the number being read

    @retval EOK the number was successfully read
    @retval ENOTSUP improper file mode
    @retval EINVAL invalid arguments

==============================================================================*/
int ReadNum( int *n )
{
    int result = EINVAL;
    FILE *fp;
    char mode;

    if( n != NULL )
    {
        mode = GetMode(active_read_fd);
        if( mode == 0 )
        {
            mode = 'r';
        }

        result = ENOTSUP;
        if( mode == 'R' )
        {
            /* read an integer from the file descriptor */
            if( read( active_read_fd, n, sizeof(int) ) == sizeof(int) )
            {
                result = EOK;
            }
        }
        else if( mode == 'r' )
        {
            *n = ScanNumber();
            result = EOK;
        }
    }

    return result;
}

/*============================================================================*/
/*  ScanNumber                                                                */
/*!
    Scan a number in ASCII from the active input file descriptor

    The ScanNumber function scans a number in ASCII from the active input
    file descriptor and converts it to an integer using atol
    The last non numeric character in the input stream is lost

    @retval the scanned number

==============================================================================*/
static int ScanNumber( void )
{
    char ch;
    char buf[BUFSIZ];
    int idx = 0;
    bool done = false;
    int state = 0;
    int sign = 1;
    int result = 0;

    /* NUL terminate */
    buf[BUFSIZ-1] = 0;

    while( !done && ( idx < BUFSIZ-1 ) )
    {
        /* read the number one character at a time */
        if( read( active_read_fd, &ch, 1 ) == 1 )
        {
            switch( state )
            {
                default:
                case 0:
                    /* waiting for a '-' or a digit */
                    if( ( ch == ' ' ) || ( ch == '\t' ) )
                    {
                        break;
                    }
                    else if( ch == '-' )
                    {
                        sign = -1;
                    }
                    else if ( isdigit( ch ) )
                    {
                        buf[idx++] = ch;
                        state = 1;
                    }
                    else
                    {
                        /* nul terminate */
                        buf[idx++] = 0;
                        done = true;
                    }
                    break;

                case 1:
                    if ( isdigit( ch ) )
                    {
                        buf[idx++] = ch;
                    }
                    else
                    {
                        /* nul terminate */
                        buf[idx++] = 0;
                        done = true;
                    }
                    break;
            }
        }
    }

    result = atol( buf );

    return result * sign;
}

/*============================================================================*/
/*  ReadChar                                                                  */
/*!
    Read a character from the active input file descriptor

    The ReadChar function reads a character from the active input
    file descriptor taking into account the mode of the file.

    If the file is opened as a binary file with mode = 'R', then
    the character will be read as a binary value.  If the file is opened
    as a text file with mode = 'r', then the character will be read as
    a text value.

    @param[in,out]
        c
            pointer to the location to store the character being read

    @retval EOK the character was successfully read
    @retval EBADF no valid active read file descriptor
    @retval ENOTSUP improper file mode
    @retval EINVAL invalid arguments

==============================================================================*/
int ReadChar( char *c )
{
    int result = EINVAL;
    int n;
    int idx;
    char ch;

    if( c != NULL )
    {
        if( active_read_fd != -1 )
        {
            /* read the character from the file descriptor */
            n = read( active_read_fd, &ch, 1 );
            if( n == 1 )
            {
                *c = ch;
                result = EOK;
            }
            else
            {
                result = EIO;
            }
        }
        else
        {
            result = EBADF;
        }
    }

    return result;
}

/*============================================================================*/
/*  GetMode                                                                   */
/*!
    Get the i/o mode associated with the specified file descriptor

    The GetMode function looks through the open file list to determine
    the read or write mode associated with the specified file descriptor.
    If the file descriptor is not found, an invalid mode will be returned.

    @param[in]
       fd
            file descriptor to search for

    @retval 'r' ascii Read
    @retval 'R' binary Read
    @retval 'w' ascii Write
    @retval 'W' binary Write
    @retval -1 if the file descriptor is not found

==============================================================================*/
static char GetMode( int fd )
{
    int idx;
    char mode = '\0';

    idx = Findfd( fd );
    if( idx != -1 )
    {
        mode = FILES[idx].mode;
    }

    return mode;
}

/*============================================================================*/
/*  Findfd                                                                    */
/*!
    Find the index of the file descriptor in the open files array

    The Findfd function searches the open files array for the specified
    file descriptor and returns its index

    @param[in]
       fd
            file descriptor to search for

    @retval index of the file descriptor
    @retval -1 if the file descriptor is not found

==============================================================================*/
static int Findfd( int fd )
{
    int idx;
    int result = -1;

    if( fd >= 0 )
    {
        /* search for the file descriptor in the open files list */
        for ( idx=3; idx < MAX_OPEN_FILES; idx++ )
        {
            if( FILES[idx].fd == fd )
            {
                result = idx;
                break;
            }
        }
    }

    return result;
}

/*! @}
 * end of files group */
