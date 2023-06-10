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

/*==============================================================================
Registers

This module manages register allocation and deallocation.
R3-R15 are available for general purpose use by the compiler.
R0 contains return data for procedure calls.
R1 contains the current activation record pointer.
R2 reserved
==============================================================================*/

/*!
 * @defgroup registers registers
 * @brief Register Management
 * @{
 */

/*==============================================================================
        Includes
==============================================================================*/

#include <stdio.h>
#include "SymbolTableManager.h"
#include "Registers.h"

/*==============================================================================
        Definitions
==============================================================================*/

#define TRUE 1
#define FALSE 0

/*! Register object */
struct Register
{
    /*! pointer to an identifier associated with the register */
	struct identEntry *idEntry;

    /*! flag to indicate if register object is used */
	char unused;
};

/*==============================================================================
        File Scoped Variables
==============================================================================*/

/*! array of 14 available registers */
static struct Register Registers[14] =
	{
		{ NULL,TRUE },
		{ NULL,TRUE },
		{ NULL,TRUE },
		{ NULL,TRUE },
		{ NULL,TRUE },
		{ NULL,TRUE },
		{ NULL,TRUE },
		{ NULL,TRUE },
		{ NULL,TRUE },
		{ NULL,TRUE },
		{ NULL,TRUE },
		{ NULL,TRUE },
		{ NULL,TRUE },
		{ NULL,TRUE }
	};

/*! track least recently accessed register */
static int lra = 3;

/*==============================================================================
        Public Function Definitions
==============================================================================*/

/*============================================================================*/
/*  AllocReg                                                                  */
/*!
    Allocate a register for use

	The AllocReg function allocates a register for use.  It returns the next
	available register in the list.  For example, if R6 was the last
	allocated, R7 will be the next.  If all registers are in use, the
	Least Recently Allocated (LRA) register will be deallocated and
	reallocated for the current request.  In this way, all allocations
	will succeed.

    @param[in]
        idEntry
			pointer to an identifier entry which wishes to allocate a register

    @param[in]
        regindex
			index of the register to use
				0 = identifier handle ( used for externs )
				1 = identifier value


    @retval number of the allocated register

==============================================================================*/
int AllocReg(struct identEntry *idEntry, int regindex )
{
	int reg = -1;

	/* search for and use a free register */
	for ( reg = 3; reg<14; reg++)
	{
		if (Registers[reg].unused == TRUE)
		{
			/* mark the register as used */
			Registers[reg].unused = FALSE;

			/* associate the identifier entry with this register */
			Registers[reg].idEntry = idEntry;

			/* store register in identifier entry structure */
			if ( ( idEntry != NULL ) &&
				 ( regindex < 2 ) )
			{
				idEntry->reg[regindex] = reg;
			}

			return( reg );
		}
	}

	/* no registers are free */
	/* free least recently allocated register */
	FreeReg( lra );

	reg = lra;

	/* mark register as used */
	Registers[reg].unused = FALSE;

	/* associate the idEntry with the register */
	Registers[reg].idEntry = idEntry;

	/* allocate least recently used register to the specified id */
	if ( ( idEntry != NULL ) &&
		 ( regindex < 2 ) )
	{
		idEntry->reg[regindex] = reg;
	}

	/* update least recently allocated register pointer */
	if (++lra == 14)
	{
		lra = 3;
	}

	/* return index of allocated register */
	return( reg );
}

/*============================================================================*/
/*  HasIdentifier                                                             */
/*!
    Determines if the register has an associated identifier

	The HasIdentifier function checks to see if the specified register is
	associated with an identifier.  i.e. the identifier is currently
	being represented by the specified register.

    @param[in]
        reg
			register number [3..14]

    @retval true register is associated with an identifier
    @retval false register is not associated with an identifier

==============================================================================*/
bool HasIdentifier( int reg )
{
	bool result = false;

	if( ( reg > 2 ) && ( reg < 14 ) )
	{
		if ( Registers[reg].idEntry != NULL )
		{
			result = true;
		}
	}

	return result;
}

/*============================================================================*/
/*  FreeReg                                                                   */
/*!
	Free a register

	The FreeReg function frees the specified register.  The register becomes
	available for allocation.  Any reference to it in an identifier
	entry structure is removed.

    @param[in]
        reg
			register number to free [3..14]

==============================================================================*/
void FreeReg( int reg )
{
	int i;
	struct identEntry *idEntry = NULL;

	/* check register bounds */
	if ( (reg > 2) && (reg < 14))
	{
		/* check if register is in use */
		if ( Registers[reg].unused == FALSE )
		{
			/* mark register as unused */
			Registers[reg].unused = TRUE;
			idEntry = Registers[reg].idEntry;

			if ( idEntry != NULL )
			{
				/* check the idEntry register associations */
				for( i=0; i<2; i++ )
				{
					if( idEntry->reg[i] == reg )
					{
						/* de-associated this register from the idEntry */
						idEntry->reg[i] = -1;
					}
				}

				/* de-associate the idEntry from this register */
				Registers[reg].idEntry = NULL;
			}
		}
	}
}

/*============================================================================*/
/*  FreeTempReg                                                               */
/*!
	Free a register not associated with an identifier

	The FreeTempReg function frees the specified register but only if it does
	not have an idEntry association.  The register becomes available for
    allocation.

    @param[in]
        reg
			register number to free [3..14]

==============================================================================*/
void FreeTempReg( int reg )
{
	/* check register bounds */
	if ( (reg > 2) && (reg < 14))
	{
		/* check that the register is not associated with an identEntry */
		if( Registers[reg].idEntry == NULL )
		{
			/* check if register is in use */
			if ( Registers[reg].unused == FALSE )
			{
				/* mark register as unused */
				Registers[reg].unused = TRUE;
			}
		}
	}
}

/*! @}
 * end of registers group */