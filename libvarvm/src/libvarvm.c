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
 * @defgroup libvarvm libvarvm
 * @brief Shared object library to link variables to the virtual machine
 * @{
 */

/*============================================================================*/
/*!
@file libvarvm.c

    Link variables to the Virtual Machine

    The Variable VM Link library provides a mechanism to allow
    the external virtual machine, which executes tiny-c applications, to
    set/get variables in the variable server.

    Variables in the tiny-c application which are declared "extern"
    will be linked to the variable server.

*/
/*============================================================================*/

/*==============================================================================
        Includes
==============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <unistd.h>
#include <dlfcn.h>
#include <varserver/var.h>
#include <varserver/varserver.h>
#include <vmcore/externvars.h>

/*==============================================================================
        Private definitions
==============================================================================*/

/*==============================================================================
        Type Definitions
==============================================================================*/
/*! The VarVM structure is an opaque structure which is shared with the
    VM (returned via the init() function) which allows the Var VM link
    to be re-entrant since the VarVM structure is passed to each function. */
typedef struct _VarVM
{
    /*! handle to the variable server */
    VARSERVER_HANDLE hVarServer;

    /*! pointer to the variable server library */
    void *pLib;

    /*! handle to the var server open function */
    VARSERVER_HANDLE (*open)(void);

    /*! handle to the var server get function */
    int (*get)( VARSERVER_HANDLE, VAR_HANDLE, VarObject *);

    /*! handle to the varserver set function */
    int (*set)( VARSERVER_HANDLE, VAR_HANDLE, VarObject *);

    /*! handle to the varserver find function */
    VAR_HANDLE (*find)( VARSERVER_HANDLE, char *);

    /*! storage location for handle of variable being validated */
    VAR_HANDLE hValidationVar;

    /*! storage location for validation data */
    VarObject validationData;

} VarVM;


/*==============================================================================
        Private function declarations
==============================================================================*/
tzEXTVARAPI *getapi(void);
void *init( void );
int shutdown( void *pExt );

static uint32_t varvm_fnGetHandle( void *pExt, char *name );
static void varvm_fnSet( void *pExt, uint32_t handle, uint32_t val );
static void varvm_fnSetFloat( void *pExt, uint32_t handle, float val );
static void varvm_fnSetString( void *pExt, uint32_t handle, char * val );
static uint32_t varvm_fnGet( void *pExt, uint32_t handle );
static float varvm_fnGetFloat( void *pExt, uint32_t handle );
static char *varvm_fnGetString( void *pExt, uint32_t handle );
static int varvm_fnNotify( void *pExt, uint32_t handle, uint32_t request );
static int varvm_fnValidateStart( void *pExt,
                                  uint32_t handle,
                                  uint32_t *hVar );
static int varvm_fnValidateEnd( void *pExt, uint32_t handle, int result );

static int varvm_fnOpenPrintSession( void *pExt,
                                     uint32_t handle,
                                     uint32_t *hVar,
                                     int *fd );

static int varvm_fnClosePrintSession( void *pExt, uint32_t handle, int fd );

/*==============================================================================
        Function definitions
==============================================================================*/

void __attribute__ ((constructor)) initLibrary(void) {
 //
 // Function that is called when the library is loaded
 //
}
void __attribute__ ((destructor)) cleanUpLibrary(void) {
 //
 // Function that is called when the library is »closed«.
 //
}

/*============================================================================*/
/* getapi                                                                     */
/*!
    Get a pointer to the VARVM API functions

    The getapi function is a public function which is expected to be
    present by the VM core.  The VM core will call this function
    to get a handle to the API functions for manipulating variables

    @return a pointer to the variable manipulation functions

==============================================================================*/
tzEXTVARAPI *getapi(void)
{
    static tzEXTVARAPI varvmAPI = {
            varvm_fnGetHandle,
            varvm_fnSet,
            varvm_fnSetFloat,
            varvm_fnSetString,
            varvm_fnGet,
            varvm_fnGetFloat,
            varvm_fnGetString,
            varvm_fnNotify,
            varvm_fnValidateStart,
            varvm_fnValidateEnd,
            varvm_fnOpenPrintSession,
            varvm_fnClosePrintSession
    };

    return &varvmAPI;
}

/*============================================================================*/
/*  init                                                                      */
/*!
    Initialize VARVM API library

    The init function is a public function which is expected to be
    present by the VM core.  The VM core will call this function
    to initialize the variable handling library.

    @retval pointer to the library instance (VarVM *)
    @retval NULL unable to initialize the library

==============================================================================*/
void *init( void )
{
    VarVM *pVarVM = NULL;

    pVarVM = calloc( 1, sizeof( VarVM ) );
    {
        pVarVM->pLib = dlopen("libvarserver.so", RTLD_NOW );
        if( pVarVM->pLib != NULL )
        {
            pVarVM->open = dlsym(pVarVM->pLib, "VARSERVER_Open" );
            if( pVarVM->open != NULL )
            {
                /* open the variable server */
                pVarVM->hVarServer = pVarVM->open();
            }

            pVarVM->set = dlsym(pVarVM->pLib, "VAR_Set" );
            pVarVM->get = dlsym(pVarVM->pLib, "VAR_Get" );
            pVarVM->find = dlsym(pVarVM->pLib, "VAR_FindByName" );
        }
        else
        {
            printf("Failed to open libvarserver.so\n");
            printf("%s\n",dlerror());
        }
    }

    return pVarVM;
}

/*============================================================================*/
/*  shutdown                                                                  */
/*!
    Shut Down VARVM API library

    The shutdown function is a public function which is expected to be
    present by the VM core.  The VM core will call this function
    to shut down the variable handling library.

    @param[in]
        pointer to the library instance obtained via the init() function call

    @retval EINVAL invalid arguments
    @retval EOK the library was successfully shut down

==============================================================================*/
int shutdown( void *pExt )
{
    int result = EINVAL;
    VarVM *pVarVM = (VarVM *)pExt;

    if( pVarVM != NULL )
    {
        VARSERVER_Close( pVarVM->hVarServer );
        pVarVM->hVarServer = NULL;
        result = EOK;
    }

    return result;
}

/*============================================================================*/
/*  varvm_fnGetHandle                                                         */
/*!
    Get a handle to a variable given its name

    The varvm_fnGetHandle function queries the variable server for
    a variable handle given its name.

    @param[in]
        pExt
            opaque pointer to the VarVM object which contains the handle
            to the Variable Server to query

    @param[in]
        name
            name of the variable to query

    @param[in]
        options
            pointer to the options used to modify the variable creation
            behavior.

    @retval EOK - variable query was successful
    @retval EINVAL - invalid arguments
    @retval ENOENT - variable does not exist

==============================================================================*/
static uint32_t varvm_fnGetHandle( void *pExt, char *name )
{
    VarVM *pVarVM = (VarVM *)pExt;
    VAR_HANDLE hVar = VAR_INVALID;

    if( ( pVarVM != NULL ) &&
        ( name != NULL ) &&
        ( pVarVM->find != NULL ) )
    {
        hVar = pVarVM->find( pVarVM->hVarServer, name );
        if( hVar == VAR_INVALID )
        {
            printf("Failed to get handle for %s\n", name);
        }
    }

    return (uint32_t)hVar;
}

/*============================================================================*/
/*  varvm_fnSet                                                               */
/*!
    Set a variable value given its handle

    The varvm_fnSet function requests the variable server to set a value
    for the variable specified by its handle

    @param[in]
        pExt
            opaque pointer to the VarVM object which contains the handle
            to the Variable Server

    @param[in]
        handle
            handle of the variable to set

    @param[in]
        val
            value (uint32_t) of the variable to set

==============================================================================*/
static void varvm_fnSet( void *pExt, uint32_t handle, uint32_t val )
{
    VarVM *pVarVM = (VarVM *)pExt;
    VarObject varObject;
    int result;

    if( ( pVarVM != NULL ) &&
        ( pVarVM->set != NULL ) )
    {
        varObject.type = VARTYPE_UINT32;
        varObject.len = sizeof( uint32_t );
        varObject.val.ul = val;

        result = pVarVM->set( pVarVM->hVarServer,
                              (VAR_HANDLE)handle,
                              &varObject );

        if( result != EOK )
        {
            printf( "%s Unable to set variable %d to %d: %s\n",
                    __func__,
                    handle,
                    val,
                    strerror( result ) );
        }
    }
}

/*============================================================================*/
/*  varvm_fnSetFloat                                                          */
/*!
    Set a variable value given its handle

    The varvm_fnSetFloat function requests the variable server to set a value
    for the variable specified by its handle

    @param[in]
        pExt
            opaque pointer to the VarVM object which contains the handle
            to the Variable Server

    @param[in]
        handle
            handle of the variable to set

    @param[in]
        val
            value (float) of the variable to set

==============================================================================*/
static void varvm_fnSetFloat( void *pExt, uint32_t handle, float val )
{
    VarVM *pVarVM = (VarVM *)pExt;
    VarObject varObject;

    if( ( pVarVM != NULL ) &&
        ( pVarVM->set != NULL ) )
    {
        varObject.type = VARTYPE_FLOAT;
        varObject.len = sizeof( float );
        varObject.val.f = val;

        pVarVM->set( pVarVM->hVarServer,
                     (VAR_HANDLE)handle,
                     &varObject );
    }
}

/*============================================================================*/
/*  varvm_fnSetString                                                         */
/*!
    Set a variable value given its handle

    The varvm_fnSetString function requests the variable server to set a value
    for the variable specified by its handle

    @param[in]
        pExt
            opaque pointer to the VarVM object which contains the handle
            to the Variable Server

    @param[in]
        handle
            handle of the variable to set

    @param[in]
        val
            pointer to a NUL terminated character string to set

==============================================================================*/
static void varvm_fnSetString( void *pExt, uint32_t handle, char *val )
{
    VarVM *pVarVM = (VarVM *)pExt;
    VarObject varObject;

    if( ( pVarVM != NULL ) &&
        ( pVarVM->set != NULL ) &&
        ( val != NULL ) )
    {
        varObject.type = VARTYPE_STR;
        varObject.len = strlen( val );
        varObject.val.str = val;

        pVarVM->set( pVarVM->hVarServer,
                     (VAR_HANDLE)handle,
                     &varObject );
    }
}

/*============================================================================*/
/*  varvm_fnGet                                                               */
/*!
    Get a variable value given its handle

    The varvm_fnGet function requests the variable server to get the value
    for the variable specified by its handle

    If the variable cannot be retrieved, zero is returned for its value

    @param[in]
        pExt
            opaque pointer to the VarVM object which contains the handle
            to the Variable Server

    @param[in]
        handle
            handle of the variable to get

    @return the value (cast to a uint32_t) of the variable

==============================================================================*/
static uint32_t varvm_fnGet( void *pExt, uint32_t handle )
{
    VarVM *pVarVM = (VarVM *)pExt;
    VarObject varObject = {0};
    uint32_t result = 0;
    VarObject *pVarObject = NULL;

    if( ( pVarVM != NULL ) &&
        ( pVarVM->get != NULL ) )
    {
        if( pVarVM->hValidationVar == handle )
        {
            /* get the variable data from the
               validation data if we are currently performing
               data validation on this variable*/
            pVarObject = &(pVarVM->validationData);
        }
        else
        {
            /* get the variable data from the variable server */
            if( pVarVM->get( pVarVM->hVarServer,
                            handle,
                            &varObject ) == EOK )
            {
                pVarObject = &varObject;
            }
            else
            {
                printf("%s failed to get variable\n", __func__ );
            }

        }

        if( pVarObject != NULL )
        {
            switch( pVarObject->type )
            {
                case VARTYPE_UINT32:
                    result = pVarObject->val.ul;
                    break;

                case VARTYPE_UINT16:
                    result = pVarObject->val.ui;
                    break;
            }
        }
    }

    return result;
}

/*============================================================================*/
/*  varvm_fnGetFloat                                                          */
/*!
    Get a variable value given its handle

    The varvm_fnGetFloat function requests the variable server to get
    the value for the variable specified by its handle

    If the variable cannot be retrieved, zero is returned for its value

    @param[in]
        pExt
            opaque pointer to the VarVM object which contains the handle
            to the Variable Server

    @param[in]
        handle
            handle of the variable to get

    @return the value (cast to a float) of the variable

==============================================================================*/
static float varvm_fnGetFloat( void *pExt, uint32_t handle )
{
    VarVM *pVarVM = (VarVM *)pExt;
    VarObject varObject = {0};
    float result = 0.0;
    VarObject *pVarObject = NULL;

    if( ( pVarVM != NULL ) &&
        ( pVarVM->get != NULL ) )
    {
        if( pVarVM->hValidationVar == handle )
        {
            /* get the variable data from the
               validation data if we are currently performing
               data validation on this variable*/
               pVarObject = &(pVarVM->validationData);
        }
        else
        {
            /* get the variable data from the variable server */
            if( pVarVM->get( pVarVM->hVarServer,
                            handle,
                            &varObject ) == EOK )
            {
                pVarObject = &varObject;
            }

        }

        if( pVarObject != NULL )
        {
            switch( varObject.type )
            {
                case VARTYPE_UINT32:
                    result = (float)(pVarObject->val.ul);
                    break;

                case VARTYPE_UINT16:
                    result = (float)(pVarObject->val.ui);
                    break;

                case VARTYPE_FLOAT:
                    result = pVarObject->val.f;
            }
        }
    }

    return result;
}

/*============================================================================*/
/*  varvm_fnGetString                                                         */
/*!
    Get a variable value given its handle

    The varvm_fnGetString function requests the variable server to get
    the value for the variable specified by its handle

    If the variable cannot be retrieved, NULL is returned for its value

    @param[in]
        pExt
            opaque pointer to the VarVM object which contains the handle
            to the Variable Server

    @param[in]
        handle
            handle of the variable to get

    @retval pointer to the NUL terminated variable string value
    @retval NULL if the variable cannot be retrieved

==============================================================================*/
static char *varvm_fnGetString( void *pExt, uint32_t handle )
{
    VarVM *pVarVM = (VarVM *)pExt;
    VarObject varObject = {0};
    char *result = NULL;
    VarObject *pVarObject = NULL;

    if( ( pVarVM != NULL ) &&
        ( pVarVM->get != NULL ) )
    {
        if( pVarVM->hValidationVar == handle )
        {
            /* get the variable data from the
               validation data if we are currently performing
               data validation on this variable*/
               pVarObject = &(pVarVM->validationData);
        }
        else
        {
            /* get the variable data from the variable server */
            if( pVarVM->get( pVarVM->hVarServer,
                            handle,
                            &varObject ) == EOK )
            {
                pVarObject = &varObject;
            }
        }

        if( pVarObject != NULL )
        {
            if( pVarObject->type == VARTYPE_STR )
            {
                result = pVarObject->val.str;
            }
        }
    }

    return result;
}

/*============================================================================*/
/*  varvm_fnNotify                                                            */
/*!
    Request notification on a variable given its handle

    The varvm_fnNotify function requests the variable server to
    send a notification to the client for the specified request.

    @param[in]
        pExt
            opaque pointer to the VarVM object which contains the handle
            to the Variable Server

    @param[in]
        handle
            handle of the variable to get

    @param[in]
        request
            type of notification being requested:
                - 1 = MODIFIED
                - 2 = CALC
                - 3 = VALIDATE
                - 4 = PRINT

    @retval EOK the notification request was successful
    @retval EINVAL if the notification request was not accepted

==============================================================================*/
static int varvm_fnNotify( void *pExt, uint32_t handle, uint32_t request )
{
    VarVM *pVarVM = (VarVM *)pExt;
    VarObject varObject = {0};
    int result = EINVAL;

    if( pVarVM != NULL )
    {
        result = VAR_Notify( pVarVM->hVarServer,
                             handle,
                             request );
    }

    return result;
}

/*============================================================================*/
/*  varvm_fnValidateStart                                                     */
/*!
    Start validation on a variable

    The varvm_fnValidateStart function is invoked at the start of the
    validation of a variable after receiption of a VALIDATE notification
    from the variable server.

    It returns a validation context from the variable server and
    populates the handle of the variable being validated and the data
    to be validated in the VarVM object context.

    @param[in]
        pExt
            opaque pointer to the VarVM object which contains the handle
            to the Variable Server

    @param[in]
        handle
            handle of the validation request as provided by the
            validation notification

    @param[out]
        hVar
            pointer to the location to store the handle of the variable
            being validated

    @retval EOK the validation information was retrieved successfully
    @retval EINVAL the validation start request could not be completed

==============================================================================*/
static int varvm_fnValidateStart( void *pExt, uint32_t handle, uint32_t *hVar )
{
    VarVM *pVarVM = (VarVM *)pExt;
    VarObject varObject = {0};
    int result = EINVAL;

    if( ( pVarVM != NULL ) &&
        ( hVar != NULL ) )
    {
        result = VAR_GetValidationRequest( pVarVM->hVarServer,
                                           handle,
                                           (VAR_HANDLE *)hVar,
                                           &pVarVM->validationData );
        if( result == EOK )
        {
            pVarVM->hValidationVar = (VAR_HANDLE)*hVar;
        }
    }

    return result;
}

/*============================================================================*/
/*  varvm_fnValidateEnd                                                       */
/*!
    Complete validation on a variable

    The varvm_fnValidateEnd function is invoked at the end of the
    validation of a variable to inform the client if the variable
    set action is allowed or denied.

    @param[in]
        pExt
            opaque pointer to the VarVM object which contains the handle
            to the Variable Server

    @param[in]
        handle
            handle of the validation request as provided by the
            validation notification

    @param[in]
        response
            EOK - the validation was successful and the variable can be set
            EINVAL - the validation was unsuccessful and the variable change
                     is denied

    @retval EOK the validation was completed successfully
    @retval EINVAL the validation could not be completed

==============================================================================*/
static int varvm_fnValidateEnd( void *pExt, uint32_t handle, int response )
{
    VarVM *pVarVM = (VarVM *)pExt;
    VarObject varObject = {0};
    int result = EINVAL;

    if( pVarVM != NULL )
    {
        result = VAR_SendValidationResponse( pVarVM->hVarServer,
                                             handle,
                                             response );

        pVarVM->hValidationVar = VAR_INVALID;

        if( ( pVarVM->validationData.type == VARTYPE_STR ) &&
            ( pVarVM->validationData.val.str != NULL ) )
        {
            /* free memory used by the string which was allocated during
               the VAR_Copy operation */
            free( pVarVM->validationData.val.str );
        }

        /* clear the validation data object */
        memset( &pVarVM->validationData, 0, sizeof( VarObject ) );
    }

    return result;
}

/*============================================================================*/
/*  varvm_fnOpenPrintSession                                                  */
/*!
    Start a print session for a variable

    The varvm_fnOpenPrintSession function is invoked at the start of the
    rendering of a variable, after receiption of a PRINT notification
    from the variable server.

    It sets up a printing context to render the variable data to an
    output stream on behalf of a requesting client.

    @param[in]
        pExt
            opaque pointer to the VarVM object which contains the handle
            to the Variable Server

    @param[in]
        handle
            handle of the print session request as provided by the
            print notification

    @param[in]
        hVar
            pointer to the location to store the handle of the variable
            that is being rendered.

    @param[in]
        fd
            pointer to the location to store the file descriptor for the
            client's output stream.

    @retval EOK - the print session was successfully created
    @retval EINVAL - invalid arguments

==============================================================================*/
static int varvm_fnOpenPrintSession( void *pExt,
                                     uint32_t handle,
                                     uint32_t *hVar,
                                     int *fd )
{
    VarVM *pVarVM = (VarVM *)pExt;
    int result = EINVAL;

    if( ( pVarVM != NULL ) &&
        ( fd != NULL ) )
    {
        result = VAR_OpenPrintSession( pVarVM->hVarServer,
                                       handle,
                                       hVar,
                                       fd );
    }

    return result;
}

/*============================================================================*/
/*  varvm_fnClosePrintSession                                                 */
/*!
    Complete a print session for a variable

    The varvm_fnClosePrintSession function is invoked at the end of the
    rendering of a variable, after the variable content has been
    rendered on behalf of the requesting client.

    It tears down the printing context used to render the variable data to the
    output stream on behalf of a requesting client and flushes and closes
    the output stream (via the VAR_ClosePrintSession) function

    @param[in]
        pExt
            opaque pointer to the VarVM object which contains the handle
            to the Variable Server

    @param[in]
        handle
            handle of the print session request as provided by the
            print notification

    @param[in]
        fp
            FILE pointer for the client's output stream.

    @retval EOK - the print session was successfully completed
    @retval EINVAL - invalid arguments

==============================================================================*/
static int varvm_fnClosePrintSession( void *pExt, uint32_t handle, int fd )
{
    VarVM *pVarVM = (VarVM *)pExt;
    int result = EINVAL;

    if( pVarVM != NULL )
    {
        result = VAR_ClosePrintSession( pVarVM->hVarServer,
                                        handle,
                                        fd );

    }

    return result;
}

/*! @}
 * end of libvarvm group */
