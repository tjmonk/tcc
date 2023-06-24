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
 * @defgroup externvars externvars
 * @brief Virtual Machine External Variables
 * @{
 */

/*==============================================================================
        Includes
==============================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <vmcore/externvars.h>

/*==============================================================================
        Private Definitions
==============================================================================*/

/*! The ExtVar object is used to represent a variable which is held
    externally to the Virtual Machine */
struct ExtVar
{
    /*! name of the external variable */
	char *name;

    /*! handle of the external variable */
	uint32_t handle;

    /*! 32-bit integer value of the external variable */
	uint32_t val;

    /*! character string pointer value of the external variable */
	char *sval;

    /*! floating point value of the external variable */
	float fval;

    /*! length of the external variable */
	size_t len;

    /*! pointer to the next external variable in the Virtual Machine */
	struct ExtVar *pNext;
};

/*==============================================================================
        Private Function Declarations
==============================================================================*/

static uint32_t extvar_fnGetHandle( void *pExt, char *name );
static void extvar_fnSet( void *pExt, uint32_t handle, uint32_t val );
static void extvar_fnSetFloat( void *pExt, uint32_t handle, float val );
static void extvar_fnSetString( void *pExt, uint32_t handle, char * val );
static uint32_t extvar_fnGet( void *pExt, uint32_t handle );
static float extvar_fnGetFloat( void *pExt, uint32_t handle );
static char *extvar_fnGetString( void *pExt, uint32_t handle );
static struct ExtVar *extvar_fnFindByName( void *pExt, char *name );
static struct ExtVar *extvar_fnFindByHandle( void *pExt, uint32_t handle );
static uint32_t extvar_fnNew( void *pExt, char *name );

/*==============================================================================
        File Scoped Variables
==============================================================================*/

tzEXTVARAPI defaultAPI = {
        extvar_fnGetHandle,
        extvar_fnSet,
        extvar_fnSetFloat,
        extvar_fnSetString,
        extvar_fnGet,
        extvar_fnGetFloat,
        extvar_fnGetString,
        NULL, /* extvar_fnNotify */
        NULL,  /* extvar_fnValidateStart */
        NULL, /* extvar_fnValidateEnd */
        NULL, /* extvar_fnOpenPrintSession */
        NULL /* extvar_fnClosePrintSession */
};

/*! default (local) API */
static tzEXTVARAPI *API = &defaultAPI;

/*! handle incremented for each created external variable */
static uint32_t handle = 0;

/*==============================================================================
        Public Function Definitions
==============================================================================*/

/*============================================================================*/
/*  EXTERNVAR_Init                                                            */
/*!
    Initialize the external vars API

    The EXTERNVAR_Init function initializes the external vars API set
    to the default API set as defined in this module.

    @retval pointer to the initialized ExtVar object

==============================================================================*/
void *EXTERNVAR_Init( void )
{
    static struct ExtVar *pExtVars = NULL;

    EXTERNVAR_fnSetAPI( &defaultAPI );

    return &pExtVars;
}

/*============================================================================*/
/*  EXTERNVAR_fnSetAPI                                                        */
/*!
    Set the external APIs

    The EXTERNVAR_fnSetAPI function initializes the external vars API set
    to the specified API set

==============================================================================*/
void EXTERNVAR_fnSetAPI( tzEXTVARAPI *pEXTVARAPI )
{
    API = pEXTVARAPI;
}

/*============================================================================*/
/*  EXTERNVAR_fnGetHandle                                                     */
/*!
    Get a handle to the external variable

    The EXTERNVAR_fnGetHandle function gets a handle to the specified
    external variable.

    @param[in]
        pExt
            opaque pointer to the external variable list

    @param[in]
        name
            name of the variable to get a handle for

    @retval handle to the specified variable

==============================================================================*/
uint32_t EXTERNVAR_fnGetHandle( void *pExt, char *name )
{
    uint32_t handle = 0L;
    if( API != NULL )
    {
        handle = API->pfnGetHandle( pExt, name );
    }

    return handle;
}

/*============================================================================*/
/*  EXTERNVAR_fnNotify                                                        */
/*!
    Request an external variable notification

    The EXTERNVAR_fnNotify function requests a notification for the
    specified external variable handle.

    @param[in]
        pExt
            opaque pointer to the external variable list

    @param[in]
        handle
            handle of the variable to request a notifcation for

    @param[in]
        request
            notification request type

    @retval result of ExtVar Notify function

==============================================================================*/
int EXTERNVAR_fnNotify( void *pExt, uint32_t handle, uint32_t request )
{
    int result = EINVAL;

    if( API != NULL )
    {
        result = ENOTSUP;

        if( API->pfnNotify != NULL )
        {
            result = API->pfnNotify( pExt, handle, request );
        }
    }

    return result;
}

/*============================================================================*/
/*  EXTERNVAR_fnValidateStart                                                 */
/*!
    Start an external variable validation

    The EXTERNVAR_fnValidateStart function starts an external variable
    validation for the specified external variable validation context.

    @param[in]
        pExt
            opaque pointer to the external variable list

    @param[in]
        handle
            handle of the variable validation context

    @param[in,out]
        hVar
            pointer to the location to store the variable handle

    @retval result of ExtVar ValidateStart function

==============================================================================*/
int EXTERNVAR_fnValidateStart( void *pExt, uint32_t handle, uint32_t *hVar )
{
    int result = EINVAL;

    if( API != NULL )
    {
        result = ENOTSUP;

        if( API->pfnValidateStart != NULL )
        {
            result = API->pfnValidateStart( pExt, handle, hVar );
        }
    }

    return result;
}

/*============================================================================*/
/*  EXTERNVAR_fnValidateEnd                                                   */
/*!
    End an external variable validation

    The EXTERNVAR_fnValidateEnd function ends an external variable
    validation for the specified external variable handle.

    @param[in]
        pExt
            opaque pointer to the external variable list

    @param[in]
        handle
            handle of the variable validation context

    @param[in]
        response
            validation response

    @retval result of ExtVar ValidateEnd function

==============================================================================*/
int EXTERNVAR_fnValidateEnd( void *pExt, uint32_t handle, int response )
{
    int result = EINVAL;

    if( API != NULL )
    {
        result = ENOTSUP;

        if( API->pfnValidateEnd != NULL )
        {
            result = API->pfnValidateEnd( pExt, handle, response );
        }
    }

    return result;
}

/*============================================================================*/
/*  EXTERNVAR_fnOpenPrintSession                                              */
/*!
    Open an external variable print session

    The EXTERNVAR_fnOpenPrintSession function starts an external variable
    print session for the specified external variable print session context.

    @param[in]
        pExt
            opaque pointer to the external variable list

    @param[in]
        handle
            handle of the variable print session context

    @param[in,out]
        hVar
            pointer to location to store the external variable handle

    @param[in,out]
        fd
            pointer to the location to store the output file descriptor

    @retval result of ExtVar OpenPrintSession function

==============================================================================*/
int EXTERNVAR_fnOpenPrintSession( void *pExt,
                                  uint32_t handle,
                                  uint32_t *hVar,
                                  int *fd )
{
    int result = EINVAL;

    if( ( API != NULL ) &&
        ( hVar != NULL ) &&
        ( fd != NULL ) )
    {
        result = ENOTSUP;

        if( API->pfnOpenPrintSession != NULL )
        {
            result = API->pfnOpenPrintSession( pExt, handle, hVar, fd );
        }
    }

    return result;
}

/*============================================================================*/
/*  EXTERNVAR_fnClosePrintSession                                             */
/*!
    Close an external variable print session

    The EXTERNVAR_fnClosePrintSession function ends an external variable
    print session for the specified external variable print session context.

    @param[in]
        pExt
            opaque pointer to the external variable list

    @param[in]
        handle
            handle of the variable print session context

    @param[in]
        fd
            output file descriptor

    @retval result of ExtVar ClosePrintSession function

==============================================================================*/
int EXTERNVAR_fnClosePrintSession( void *pExt, uint32_t handle, int fd )
{
    int result = EINVAL;

    if( API != NULL )
    {
        result = ENOTSUP;

        if( API->pfnClosePrintSession != NULL )
        {
            result = API->pfnClosePrintSession( pExt, handle, fd );
        }
    }

    return result;
}

/*============================================================================*/
/*  EXTERNVAR_fnSet                                                           */
/*!
    Set the value of an external variable

    The EXTERNVAR_fnSet function sets the value of an external variable

    @param[in]
        pExt
            opaque pointer to the external variable list

    @param[in]
        handle
            handle of the external variable

    @param[in]
        val
            32-bit integer value to set

    @retval result of ExtVar Set function

==============================================================================*/
void EXTERNVAR_fnSet( void *pExt, uint32_t handle, uint32_t val )
{
    if( API != NULL )
    {
        API->pfnSet( pExt, handle, val );
    }
}

/*============================================================================*/
/*  EXTERNVAR_fnSetFloat                                                      */
/*!
    Set the value of a floating point external variable

    The EXTERNVAR_fnSetFloat function sets the value of a 32-bit IEEE754
    floating point external variable

    @param[in]
        pExt
            opaque pointer to the external variable list

    @param[in]
        handle
            handle of the external variable

    @param[in]
        val
            32-bit IEEE754 floating point value to set

    @retval result of ExtVar SetFloat function

==============================================================================*/
void EXTERNVAR_fnSetFloat( void *pExt, uint32_t handle, float val )
{
    if( API != NULL )
    {
        API->pfnSetFloat( pExt, handle, val );
    }
}

/*============================================================================*/
/*  EXTERNVAR_fnSetString                                                     */
/*!
    Set the value of a string external variable

    The EXTERNVAR_fnSetString function sets the value of a string
    external variable

    @param[in]
        pExt
            opaque pointer to the external variable list

    @param[in]
        handle
            handle of the external variable

    @param[in]
        val
            pointer to the string value to set

    @retval result of ExtVar SetString function

==============================================================================*/
void EXTERNVAR_fnSetString( void *pExt, uint32_t handle, char * val )
{
    if( API != NULL )
    {
        API->pfnSetString( pExt, handle, val );
    }
}

/*============================================================================*/
/*  EXTERNVAR_fnGet                                                           */
/*!
    Get the value of a 32-bit integer external variable

    The EXTERNVAR_fnGet function gets the value of a 32-bit string
    external variable

    @param[in]
        pExt
            opaque pointer to the external variable list

    @param[in]
        handle
            handle of the external variable

    @retval result of ExtVar Get function

==============================================================================*/
uint32_t EXTERNVAR_fnGet( void *pExt, uint32_t handle )
{
    uint32_t result = 0L;

    if( API != NULL )
    {
        result = API->pfnGet( pExt, handle );
    }

    return result;
}

/*============================================================================*/
/*  EXTERNVAR_fnGetFloat                                                      */
/*!
    Get the value of a floating point external variable

    The EXTERNVAR_fnGetFloat function gets the value of a 32-bit IEEE754
    floating point external variable

    @param[in]
        pExt
            opaque pointer to the external variable list

    @param[in]
        handle
            handle of the external variable

    @retval result of ExtVar GetFloat function

==============================================================================*/
float EXTERNVAR_fnGetFloat( void *pExt, uint32_t handle )
{
    float result = 0.0;

    if( API != NULL )
    {
        result = API->pfnGetFloat( pExt, handle );
    }

    return result;
}

/*============================================================================*/
/*  EXTERNVAR_fnGetString                                                     */
/*!
    Get the value of a string external variable

    The EXTERNVAR_fnGetString function gets the value of a string
    external variable

    @param[in]
        pExt
            opaque pointer to the external variable list

    @param[in]
        handle
            handle of the external variable

    @retval result of ExtVar GetString function

==============================================================================*/
char *EXTERNVAR_fnGetString( void *pExt, uint32_t handle )
{
    char *pStr = "";

    if( API != NULL )
    {
        pStr = API->pfnGetString( pExt, handle );
    }

    return pStr;
}

/*==============================================================================
        Private Function Definitions
==============================================================================*/

/*============================================================================*/
/*  extvar_fnGetHandle                                                        */
/*!
    Get the handle of an external variable

    The extvar_fnGetHandle function gets the handle of an external variable
    given its name.  If the variable is not found, one will be allocated.

    @param[in]
        pExt
            opaque pointer to the external variable list

    @param[in]
        name
            name of the external variable

    @retval handle to the external variable

==============================================================================*/
static uint32_t extvar_fnGetHandle( void *pExt, char *name )
{
    struct ExtVar **ppExtVars = (struct ExtVar **)pExt;
    struct ExtVar *pExtVar;

    if( pExt != NULL )
    {
        if( *ppExtVars == NULL )
        {
            return extvar_fnNew( pExt, name );
        }
        else
        {
            pExtVar = extvar_fnFindByName( pExt, name );
            if( pExtVar == NULL )
            {
                return extvar_fnNew( pExt, name );
            }
            else
            {
                return pExtVar->handle;
            }
        }
    }

    return 0;
}

/*============================================================================*/
/*  extvar_fnSet                                                              */
/*!
    Set the value of an external variable

    The extvar_fnSet function sets the value of an external variable

    @param[in]
        pExt
            opaque pointer to the external variable list

    @param[in]
        handle
            external variable handle

    @param[in]
        val
            32-bit integer value to set

==============================================================================*/
static void extvar_fnSet( void *pExt, uint32_t handle, uint32_t val )
{
    struct ExtVar *pExtVar;

    pExtVar = extvar_fnFindByHandle( pExt, handle );
    if( pExtVar != NULL )
    {
        pExtVar->val = val;
    }
}

/*============================================================================*/
/*  extvar_fnSetFloat                                                         */
/*!
    Set the floating point value of an external variable

    The extvar_fnSetFloat function sets the 32-bit IEEE754 floating point
    value of an external variable

    @param[in]
        pExt
            opaque pointer to the external variable list

    @param[in]
        handle
            external variable handle

    @param[in]
        val
            32-bit IEEE743 floating point value to set

==============================================================================*/
static void extvar_fnSetFloat( void *pExt, uint32_t handle, float val )
{
    struct ExtVar *pExtVar;

    pExtVar = extvar_fnFindByHandle( pExt, handle );
    if( pExtVar != NULL )
    {
        pExtVar->fval = val;
    }
}

/*============================================================================*/
/*  extvar_fnSetString                                                        */
/*!
    Set the string value of an external variable

    The extvar_fnSetString function sets the string value of an
    external variable

    @param[in]
        pExt
            opaque pointer to the external variable list

    @param[in]
        handle
            external variable handle

    @param[in]
        val
            string value to set

==============================================================================*/
static void extvar_fnSetString( void *pExt, uint32_t handle, char * val )
{
    struct ExtVar *pExtVar;
    size_t len;

    if( val != NULL )
    {
	   	len = strlen( val );
	    pExtVar = extvar_fnFindByHandle( pExt, handle );
	    if( pExtVar != NULL )
	    {
	    	if( len > pExtVar->len )
	    	{
	    		if( pExtVar->sval != NULL )
	    		{
	    			pExtVar->sval = realloc( pExtVar->sval, len + 1 );
	    		}
	    	}

	    	if( pExtVar->sval == NULL )
	    	{
	    		pExtVar->sval = malloc( len + 1 );
	    	}

	    	if( pExtVar->sval != NULL )
	    	{
	    		strcpy(pExtVar->sval, val );
	    	}
	    }
	}
}

/*============================================================================*/
/*  extvar_fnGet                                                              */
/*!
    Get the value of an external variable

    The extvar_fnGet function gets the 32-bit integer value of an
    external variable

    @param[in]
        pExt
            opaque pointer to the external variable list

    @param[in]
        handle
            external variable handle

    @retval variable value.
    @retval 0 if no variable is found.

==============================================================================*/
static uint32_t extvar_fnGet( void *pExt, uint32_t handle )
{
    struct ExtVar *pExtVar;

    pExtVar = extvar_fnFindByHandle( pExt, handle );
    if( pExtVar != NULL )
    {
        return pExtVar->val;
    }

    return 0;
}

/*============================================================================*/
/*  extvar_fnGetFloat                                                         */
/*!
    Get the floating point value of an external variable

    The extvar_fnGetFloat function gets the 32-bit IEEE754 floating point
    value of an external variable

    @param[in]
        pExt
            opaque pointer to the external variable list

    @param[in]
        handle
            external variable handle

    @retval variable value.
    @retval 0 if no variable is found.

==============================================================================*/
static float extvar_fnGetFloat( void *pExt, uint32_t handle )
{
    struct ExtVar *pExtVar;

    pExtVar = extvar_fnFindByHandle( pExt, handle );
    if( pExtVar != NULL )
    {
        return pExtVar->fval;
    }

    return 0;
}

/*============================================================================*/
/*  extvar_fnGetString                                                        */
/*!
    Get the string value of an external variable

    The extvar_fnGetString function gets the string value of an external
    variable

    @param[in]
        pExt
            opaque pointer to the external variable list

    @param[in]
        handle
            external variable handle

    @retval variable value.
    @retval NULL if no variable is found.

==============================================================================*/
static char *extvar_fnGetString( void *pExt, uint32_t handle )
{
    struct ExtVar *pExtVar;

    pExtVar = extvar_fnFindByHandle( pExt, handle );
    if( pExtVar != NULL )
    {
        return pExtVar->sval;
    }

    return NULL;
}

/*============================================================================*/
/*  extvar_fnNew                                                              */
/*!
    Create a new external variable

    The extvar_fnNew function creates a new external variable.
    It allocates memory for the external variable and inserts it
    into the external variable list.

    @param[in]
        pExt
            opaque pointer to the external variable list

    @param[in]
        name
            name of the variable to create

    @retval variable handle.
    @retval 0 if no variable could be created.

==============================================================================*/
static uint32_t extvar_fnNew( void *pExt, char *name )
{
	struct ExtVar *pNew;
	size_t len;
    struct ExtVar **ppExtVars = (struct ExtVar **)pExt;

    if( ( pExt != NULL ) &&
        ( name != NULL ) )
    {
        /* allocate memory for the new ExtVar */
        pNew = malloc( sizeof( struct ExtVar ) );
        if( pNew != (struct ExtVar *)NULL )
        {
            /* allocate memory for the name */
            len = strlen(name);
            pNew->name = malloc( len + 1 );
            if( pNew->name == (char *)NULL )
            {
                free( pNew );
                return 0;
            }
            else
            {
                strcpy(pNew->name, name);
                pNew->name[len] = '\0';
                pNew->handle = ++handle;

                /* insert the extvar at the head of the list */
                pNew->pNext = *ppExtVars;
                *ppExtVars = pNew;

                /* return the handle of the new ExtVar */
                return pNew->handle;
            }
        }
    }

	return 0;
}

/*============================================================================*/
/*  extvar_fnFindByName                                                       */
/*!
    Find an external variable given its name

    The extvar_fnFindByName function searches for an external variable
    in the external variable list given its name.


    @param[in]
        pExt
            opaque pointer to the external variable list

    @param[in]
        name
            external variable name

    @retval variable value.
    @retval NULL if no variable is found.

==============================================================================*/
static struct ExtVar *extvar_fnFindByName( void *pExt, char *name )
{
    struct ExtVar **ppExtVars = (struct ExtVar **)pExt;
	struct ExtVar *pExtVar = NULL;

    if( ( pExt != NULL ) &&
        ( name != NULL ) )
    {
        pExtVar = *ppExtVars;
        while( pExtVar != NULL )
        {
            if( strcmp( name, pExtVar->name ) == 0 )
            {
                break;
            }

            pExtVar = pExtVar->pNext;
        }
    }

	return (struct ExtVar *)pExtVar;
}

/*============================================================================*/
/*  extvar_fnFindByHandle                                                     */
/*!
    Find an external variable given its handle

    The extvar_fnFindByHandle function searches for an external variable
    in the external variable list given its handle.

    @param[in]
        pExt
            opaque pointer to the external variable list

    @param[in]
        name
            external variable name

    @retval pointer to the ExtVar object for the specified variable
    @retval NULL if no variable is found.

==============================================================================*/
static struct ExtVar *extvar_fnFindByHandle( void *pExt, uint32_t handle )
{
    struct ExtVar **ppExtVars = (struct ExtVar **)pExt;
	struct ExtVar *pExtVar = NULL;

    if( pExt != NULL )
    {
        pExtVar = *ppExtVars;
        while( pExtVar != NULL )
        {
            if( pExtVar->handle == handle )
            {
                break;
            }

            pExtVar = pExtVar->pNext;
        }


    }
	return pExtVar;
}

/*! @}
 * end of externvars group */
