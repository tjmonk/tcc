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
 * @defgroup labels Label Manager
 * @brief VM Assembler Label Manager
 * @{
 */

/*============================================================================*/
/*!
@file labels.c

    Virtual Machine Assembler Label Manager

    The Virtual Machine Assembler Label Manager module manages all the
    assembler labels and their addresses.  It also handles updating the
    jump addresses for the labels.

*/
/*============================================================================*/

/*==============================================================================
        Includes
==============================================================================*/

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "labels.h"

/*==============================================================================
        File Scoped Variables
==============================================================================*/

/*! list of labels */
static backpatchRecPtr labels = NULL;

/*==============================================================================
        Public Function Definitions
==============================================================================*/

/*============================================================================*/
/*  enterLabel                                                                */
/*!
    Enter a location where a label value is to be plugged in

    The enterLabel function enters a location where a label value is
    to be plugged in by creating a new backpatch record associated
    with the specified label.

    @param[in]
        label
            pointer to the label name

    @param[in]
        addr
            the VM address which needs to be updated with the label address

==============================================================================*/
void enterLabel(char *label, uint16_t addr)
{
	backpatchRecPtr bp;	       /* list of known labels */
	backpatchptr labelLoc;     /* list of locations of label */

    /* find the label record */
	bp = findLabelRec(label);
    if ( bp != NULL )
    {
        if ( bp->locale == NULL )
        {
            /* create a new backpatch locale at the head of the locale list */
            bp->locale = (backpatchptr)malloc( sizeof( backpatch ) );
            if ( bp->locale != NULL )
            {
                bp->locale->next = NULL;
            }
        }
        else
        {
            /* create a new backpatch locale and insert it at the head
               of the locale list */
            labelLoc = bp->locale;
            bp->locale = (backpatchptr)malloc( sizeof(backpatch) );
            if ( bp->locale != NULL )
            {
                bp->locale->next = labelLoc;
            }
        }

        /* store the address in the backpatch locale */
        if ( bp->locale != NULL )
        {
            bp->locale->location = addr;
        }
    }
}

/*============================================================================*/
/*  setLabelAddr                                                              */
/*!
    Set the address of a label

    The setLabelAddr function associates a Virtual Machine address with
    a label.  If the label does not yet exist, it will be created when
    this function is called.

    @param[in]
        label
            pointer to the label name

    @param[in]
        addr
            the VM address associated with the label

==============================================================================*/
void setLabelAddr(char *label, uint16_t addr)
{
	backpatchRecPtr bp;

    /* find (or create) the label record */
	bp = findLabelRec(label);
	if (bp->address != -1)
	{
        /* multiple labels */
		fprintf(stderr, "multiple labels found:  %s\n", label);
	}
	else
	{
		bp->address = addr;
	}
}

/*============================================================================*/
/*  findLabelRec                                                              */
/*!
    Find a backpatch label record

    The findLabelRec function searches the label record list looking for
    the label record.  If the label record list is empty, a new label
    record will be placed at the start of the list.  If the list is
    searched and no record is found, one will be added at the end of the
    label record list.

    @param[in]
        label
            pointer to the label name

    @retval pointer to a backpatch record

==============================================================================*/
backpatchRecPtr findLabelRec(char *label)
{
	backpatchRecPtr bp;

	if ( labels == NULL )
	{
        /* create backpatch record at the head of the list */
        labels = (backpatchRecPtr)malloc( sizeof( backpatchRec ) );
		labels->next = NULL;
        labels->label = strdup( label );
		labels->address = -1;	/* a negative value implies error */
		labels->locale = NULL;

		return(labels);
	}
	else
	{
		bp = labels;
		do
		{
            /* check if label found */
         	if (strcmp(bp->label, label) != 0)
         	{
                /* not found */
			    if ( bp->next == NULL )
			    {
                    /* add backpatch record at the end of the list */
    				bp->next = (backpatchRecPtr)malloc( sizeof(backpatchRec) );
    				bp->next->next = NULL;
    				bp->next->label = strdup( label );
    				bp->next->locale = NULL;
    				bp->next->address = -1;
			    }

                /* move to next backpatch record */
			    bp = bp->next;
			}
		} while (strcmp(bp->label, label) != 0);

		return(bp);
	}
}

/*============================================================================*/
/*  LinkLabels                                                                */
/*!
    Link all the labels

    The LinkLabels function iterates through the label list, and for
    each label, iterates through the locale list populating the label
    address into the virtual machine memory wherever the label is used.

    @param[in]
        memory
            pointer to the virtual machine memory

    @param[in]
        verbose
            enable (true) or disable (false) diagnostic output

    @param[in]
        showLabels
            show the labels if true

    @retval 0 success
    @retval -1 an error occurred

==============================================================================*/
int LinkLabels(uint8_t *memory, bool verbose, bool showLabels)
{
	backpatchptr addr;
	backpatchRecPtr label;
    uint16_t loc;
    int status = 0;

	if (verbose)
	{
		printf("Now linking...\n");
	}

	label = labels;
	if (showLabels)
	{
		printf("assigning labels:\n");
	}

	while (label != NIL)
	{
		if (showLabels)
		{
			printf("%x  %s\n", label->address, label->label);
		}

		if (label->address == -1)
		{
			fprintf( stderr, "ERROR:  label %s not found.\n", label->label );
			status = -1;
		}
		else
		{
			addr = label->locale;
			while ( addr != NULL )
			{
                loc = addr->location;
                memory[loc] = ( ( label->address & 0xFF00 ) >> 8 );
                memory[loc+1] = ( label->address & 0x00FF );

                if ( showLabels )
                {
                	printf( "memory[%x]=%x\n", loc, memory[loc] );
                	printf( "memory[%x]=%x\n", loc+1, memory[loc+1] );
                	printf( "storing label address 0x%04X at address 0x%04X\n",
                            label->address, loc);
                }

				addr = addr->next;
			}
		}

		label = label->next;
	}

	return status;
}

/*! @}
 * end of labels group */