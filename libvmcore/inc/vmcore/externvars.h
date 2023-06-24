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

#ifndef EXTERNVARS_H
#define EXTERNVARS_H

/*==============================================================================
        Includes
==============================================================================*/

#include <stdint.h>
#include <stdio.h>

/*==============================================================================
        Public definitions
==============================================================================*/

typedef struct zEXTVARAPI
{
    uint32_t (*pfnGetHandle)(void *pExt, char *name);
    void (*pfnSet)( void *pExt, uint32_t handle, uint32_t val );
    void (*pfnSetFloat)( void *pExt, uint32_t handle, float val );
    void (*pfnSetString)( void *pExt, uint32_t handle, char * val );
    uint32_t (*pfnGet)( void *pExt, uint32_t handle );
    float (*pfnGetFloat)( void *pExt, uint32_t handle );
    char * (*pfnGetString)( void *pExt, uint32_t handle );
    int (*pfnNotify)( void *pExt, uint32_t handle, uint32_t request );
    int (*pfnValidateStart)( void *pExt, uint32_t handle, uint32_t *hVar );
    int (*pfnValidateEnd)( void *pExt, uint32_t handle, int result );
    int (*pfnOpenPrintSession)( void *pExt,
                                uint32_t handle,
                                uint32_t *hVar,
                                int *fd );
    int (*pfnClosePrintSession)( void *pExt, uint32_t handle, int fd );
} tzEXTVARAPI;

/*==============================================================================
        Public function declarations
==============================================================================*/

void *EXTERNVAR_Init( void );
void EXTERNVAR_fnSetAPI( tzEXTVARAPI *pEXTVARAPI );
uint32_t EXTERNVAR_fnGetHandle( void *pExt, char *name );
void EXTERNVAR_fnSet( void *pExt, uint32_t handle, uint32_t val );
void EXTERNVAR_fnSetFloat( void *pExt, uint32_t handle, float val );
void EXTERNVAR_fnSetString( void *pExt, uint32_t handle, char * val );
uint32_t EXTERNVAR_fnGet( void *pExt, uint32_t handle );
float EXTERNVAR_fnGetFloat( void *pExt, uint32_t handle );
char *EXTERNVAR_fnGetString( void *pExt, uint32_t handle );
int EXTERNVAR_fnNotify( void *pExt, uint32_t handle, uint32_t request );
int EXTERNVAR_fnValidateStart( void *pExt,
                               uint32_t handle,
                               uint32_t *hVar );
int EXTERNVAR_fnValidateEnd( void *pExt, uint32_t handle, int result );
int EXTERNVAR_fnOpenPrintSession( void *pExt,
                                  uint32_t handle,
                                  uint32_t *hVar,
                                  int *fd );
int EXTERNVAR_fnClosePrintSession( void *pExt, uint32_t handle, int fd );

#endif
