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

#ifndef LABELS_H
#define LABELS_H

/*==============================================================================
        Includes
==============================================================================*/

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*==============================================================================
        Public definitions
==============================================================================*/

#define NIL 0

/* structures for filling in label addresses */

/*! backpatch node */
typedef struct Backpatch
{
    /*! backpatch location */
	short location;

    /*! pointer to next backpatch node */
	struct Backpatch *next;
} backpatch,  *backpatchptr;

/*! backpatch record */
typedef struct BackpatchRec
{
    /*! pointer to label name */
	char *label;

    /*! backpatch address */
	short address;

    /*! pointer to backpatch list */
	backpatchptr locale;

    /*! pointer to next backpatch record */
	struct BackpatchRec *next;
} backpatchRec, *backpatchRecPtr;

/*==============================================================================
        Public function declarations
==============================================================================*/

backpatchRecPtr findLabelRec();
void enterLabel(char *label, uint16_t addr);
void setLabelAddr(char *label, uint16_t addr);
int LinkLabels(uint8_t *memory, bool verbose, bool showLabels);

#endif