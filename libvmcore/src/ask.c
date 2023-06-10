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
 * @defgroup ask ask
 * @brief Handle interactive queries to the operator
 * @{
 */

/*==============================================================================
        Includes
==============================================================================*/

#include <stdio.h>
#include "ask.h"

/*==============================================================================
        Public Function Definitions
==============================================================================*/

/*============================================================================*/
/*  ask                                                                       */
/*!
    Ask operator for a y/n response

    The ask function queries the operator with the specified query string
    and waits for a yes/no response.  Only the following values are
    accepted: y, n, Y, N.

    @param[in]
        ask
            pointer to the user query

    @retval non-zero - user responded yes
    @retval zero - user responded no

==============================================================================*/
int ask(char *s)
{
	char answer;

	do
	{
		printf("%s", s);
		answer = getchar();
		if (answer != '\n')
		{
			while (getchar() != '\n') { ; }
		}
	} while ((answer!='y') && (answer!='Y') &&
		     (answer!='n') && (answer!='N'));

	return((answer == 'y') || (answer == 'Y'));
}

/*! @}
 * end of ask group */