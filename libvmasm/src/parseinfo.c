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
 * @defgroup parseinfo Parser Support Functions
 * @brief Virtual Machine Assembler Parser Support Functions
 * @{
 */

/*==============================================================================
        Includes
==============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <vmcore/datatypes.h>
#include "parseinfo.h"

/*==============================================================================
        Private Function Declarations
==============================================================================*/

static void allCAPS( char *str);
static uint32_t xtol( char *hexstring );
static uint16_t hexdigit( char digit, uint16_t *val );
static unsigned char ParseChar( char *input, int *length );

/*==============================================================================
        Public Function Definitions
==============================================================================*/

/*============================================================================*/
/*  AllocString                                                               */
/*!
    Allocate a string

    The AllocString function allocates a string, and returns the
    string in a tzParseInfo object.

    @param[in]
        type
            specify the type of the tzParseInfo object

    @param[in]
        label
            pointer to the label string

    @param[in]
        length
            length of the label string

    @param[in]
        capitalize
            true - capitalize the label


    @retval a tzParseInfo object containing the label string

==============================================================================*/
tzParseInfo AllocString( teParseType type,
                         char *label,
                         size_t length,
                         bool capitalize )
{
	char *dest;
	tzParseInfo parseInfo;

	memset(&parseInfo, 0, sizeof( parseInfo ));

	dest = (char *)malloc( length+1 );
	if( dest == NULL )
	{
		printf("Failed to allocate memory for '%s'\n", label );
		exit(1);
	}

	/* clear the string */
	memset( dest, 0, length+1 );
	strncpy(dest, label, length);

	if( capitalize == true )
	{
		allCAPS( dest );
	}

	parseInfo.type = type;
	if ( type == eLABEL )
	{
		parseInfo.n = 2;  /* labels take up 2 bytes */
		parseInfo.width = 2;
	}

	parseInfo.value.pStrVal = dest;

	return parseInfo;
}

/*============================================================================*/
/*  GetRegister                                                               */
/*!
    Parse a register into a tzParseInfo object

    The GetRegister function parses a register definition and
    creates a tzParseInfo object containing the register information.

    @param[in]
        regdef
            register definition string

    @param[in]
        line_number
            the line number of the register definition

    @retval a tzParseInfo object containing the register information

==============================================================================*/
tzParseInfo GetRegister( char *regdef, int line_number )
{
	char *reg_digit;
	long reg = 0;
	tzParseInfo parseInfo;

	memset(&parseInfo, 0, sizeof( parseInfo ));

	if( regdef == NULL )
	{
		return parseInfo;
	}

	if( toupper( regdef[0] ) == 'R' )
	{
		reg_digit = &regdef[1];
		while( ( *reg_digit >= '0' ) && ( *reg_digit <= '9' ) )
		{
			reg = reg * 10 + ( *reg_digit - '0' );
			reg_digit++;
		}

		if( ( reg < 0 ) || ( reg > 13 ) )
		{
			printf("invalid register (R%ld) at line %d\n", reg, line_number);
			exit(1);
		}
	}
	else if( ( toupper( regdef[0] ) == 'S' ) &&
		     ( toupper( regdef[1] ) == 'P' ) )
	{
		reg = 14;
	}
	else if( ( toupper( regdef[0] ) == 'P' ) &&
		     ( toupper( regdef[1] ) == 'C' ) )
	{
		reg = 15;
	}

	parseInfo.type = eREGISTER;
	parseInfo.n = 1; /* register definition uses 1 byte */
	parseInfo.width = 4; /* registers are 4 bytes wide */
	parseInfo.value.regnum = reg;

	return parseInfo;
}

/*============================================================================*/
/*  EncodeOp                                                                  */
/*!
    Encode an operator into a tzParseInfo Object

    The EncodeOp function parses an operator into a tzParseInfo object.

    @param[in]
        operator_name
            name of the operator

    @param[in]
        length
            length of the operator name

    @param[in]
        lineno
            the line number of the operator name

    @param[in]
        operator_value
            the operator value

    @retval a tzParseInfo object containing the operator information

==============================================================================*/
tzParseInfo EncodeOp( char *operator_name,
                      size_t length,
                      int lineno,
                      uint8_t operator_value )
{
	char length_spec = 'L';
	tzParseInfo parseInfo;
	size_t i;

	memset(&parseInfo, 0, sizeof( parseInfo ));

	parseInfo.type = eOP;
	parseInfo.n = 1;  /* an operator takes 1 byte */
	parseInfo.value.op = operator_value;

	for( i = 0; i < length ; i++ )
	{
		if( operator_name[i] == '.' )
		{
			length_spec = toupper(operator_name[i+1]);
		}
	}

	/* encode the operation width into the operator code */
	switch( length_spec )
	{
		case 'H':
			/* special case indicates the value is a handle to external data */
			parseInfo.value.op |= 0x60;
			parseInfo.width = 1;
			break;

		case 'S':
		case 'B':
			parseInfo.value.op |= 0x80;
			parseInfo.width = 1;
			break;

		case 'W':
			parseInfo.value.op |= 0x40;
			parseInfo.width = 2;
			break;

		case 'F':
			parseInfo.value.op |= 0xC0;
			parseInfo.width = 4;
			break;

		case 'L':
			parseInfo.width = 4;
			break;

		default:
			printf("Line: %d: Invalid length specifier\n", lineno );
			break;
	}

	return parseInfo;
}

/*============================================================================*/
/*  EncodeValue                                                               */
/*!
    Encode a value into a tzParseInfo Object

    The EncodeValue function parses a value into a tzParseInfo object.

    @param[in]
        valueText
            pointer to the value text

    @param[in]
        numType
            the type of the number

    @param[in]
        lineno
            the line number of the value

    @retval a tzParseInfo object containing the value information

==============================================================================*/
tzParseInfo EncodeValue( char *valueText, teNumType numType, int lineno )
{
	tzParseInfo parseInfo;
	int32_t stemp;
	uint32_t utemp;

	memset(&parseInfo, 0, sizeof( parseInfo ));

	switch( numType )
	{
	    case eINTEGER:
	    	if( valueText[0] == '-' )
	    	{
	    		stemp = atol( valueText );
	    		if( stemp >= -128 && stemp <= 127 )
	    		{
	    			parseInfo.type = eSINT8;
	    			parseInfo.n = 1;
	    			parseInfo.width = 1;
	    			parseInfo.value.scVal = (int8_t)stemp;
	    		}
	    		else if( stemp >= -32768 && stemp <= 32767 )
	    		{
	    			parseInfo.type = eSINT16;
	    			parseInfo.n = 2;
	    			parseInfo.width = 2;
	    			parseInfo.value.siVal = (int16_t)stemp;
	    		}
	    		else
	    		{
	    			parseInfo.type = eSINT32;
	    			parseInfo.n = 4;
    				parseInfo.width = 4;
	    			parseInfo.value.slVal = stemp;
	    		}
	    	}
	    	else
	    	{
	    		utemp = atol( valueText );
	    		if( utemp <= 0x7F )
	    		{
	    			parseInfo.type = eUINT8;
	    			parseInfo.n = 1;
	    			parseInfo.width = 1;
	    			parseInfo.value.ucVal = (uint8_t)(utemp & 0xFF);
	    		}
	    		else if( utemp <= 0x7FFF)
	    		{
	    			parseInfo.type = eUINT16;
	    			parseInfo.n = 2;
    				parseInfo.width = 2;
	    			parseInfo.value.uiVal = (uint16_t)(utemp & 0xFFFF);
	    		}
	    		else
	    		{
	    			parseInfo.type = eUINT32;
	    			parseInfo.n = 4;
	    			parseInfo.width = 4;
	    			parseInfo.value.ulVal = utemp;
	    		}
	    	}
	    	break;

    	case eFLOAT:
    		parseInfo.type = eFLOAT32;
    		parseInfo.n = 4;
			parseInfo.width = 4;
    		parseInfo.value.fVal = atof(valueText);
    		break;

    	case eHEXADECIMAL:
    		utemp = xtol( valueText );
    		if( utemp <= 0xFF )
    		{
    			parseInfo.type = eUINT8;
    			parseInfo.n = 1;
    			parseInfo.width = 1;
    			parseInfo.value.ucVal = (uint8_t)(utemp & 0xFF);
    		}
    		else if( utemp <= 0xFFFF)
    		{
    			parseInfo.type = eUINT16;
    			parseInfo.n  = 2;
    			parseInfo.width = 2;
    			parseInfo.value.uiVal = (uint16_t)(utemp & 0xFFFF);
    		}
    		else
    		{
    			parseInfo.type = eUINT32;
    			parseInfo.n = 4;
    			parseInfo.width = 4;
    			parseInfo.value.ulVal = utemp;
    		}
    		break;

    	default:
    		printf("unknown numeric type at line: %d", lineno );
    		break;

	}

	return parseInfo;
}

/*============================================================================*/
/*  EncodeChar                                                                */
/*!
    Encode a character into a tzParseInfo Object

    The EncodeValue function parses a value into a tzParseInfo object.

    @param[in]
        valueText
            pointer to the value text

    @param[in]
        length
            length of the value text

    @param[in]
        lineno
            the line number of the value

    @retval a tzParseInfo object containing the character value information

==============================================================================*/
tzParseInfo EncodeChar( char *valueText, size_t length, int lineno)
{
	int n = 0;
	unsigned int charVal = 0;
	tzParseInfo parseInfo;

	if (*valueText == '\\')
	{
		charVal = ParseChar(&valueText[1], &n);
	}
  	else
  	{
        charVal = *valueText;
	}

	if (charVal > 0xFF)
	{
		printf("line: %d: char constant too big", lineno);
		charVal = 0;
	}

	parseInfo.type = eUINT8;
	parseInfo.n = 1;
	parseInfo.width = 1;
	parseInfo.value.ucVal = charVal;

	return parseInfo;
}

/*============================================================================*/
/*  copystring                                                                */
/*!
    Copy a string constant to memory

    The copystring function copies a string constant to memory while
    performing character translation using the ParseChar function.

    @param[in]
        pParseInfo
            pointer to the tzParseInfo object containing the source string

    @param[in]
        destination
            pointer to the location to store the copied (parsed) string

    @retval number of bytes copied to the destination
    @retval 0 an error occurred

==============================================================================*/
int copystring(tzParseInfo *pParseInfo, uint8_t *destination )
{
	int n = 0;
	int len = 0;
	char * source;
	int escape=0;  /* escape sequence flag */

    if( pParseInfo == NULL )
    {
        return 0;
    }

    if( pParseInfo->type != eSTRING )
    {
        fprintf(stderr, "invalid type for copystring operation\n");
        return 0;
    }

    source = pParseInfo->value.pStrVal;
	while (*source != '\0')
	{
		if( *source == '\\' )
		{
			escape = 1;
			source++;
		}
		else if( escape )
		{
			*destination++ = ParseChar( source, &n );
			len++;
			source += n;
			escape = 0;
		}
		else
		{
			*destination++ = *source++;
			len++;
		}
	}

    /* null terminate the string */
    *destination++ = '\0';
    len++;

	return len;
}

/*==============================================================================
        Private Function Definitions
==============================================================================*/

/*============================================================================*/
/*  ParseChar                                                                 */
/*!
    Escape the input character

    The ParseChar function parses a character constant which follows a
    backslash in a quoted string.  For example:

    '\n \r \t \0 \0xFE \123'

    @param[in]
        input
            pointer to the input character to be parsed.

    @param[in]
        length
            pointer to the location to store the number of bytes processed

    @retval the parsed character value

==============================================================================*/
static unsigned char ParseChar( char *input, int *length )
{
	unsigned char charVal = 0;
	int n = 1;
	uint16_t val;

	switch (*input)
	{
        case 'n':
        	charVal = '\n';
			break;

        case 't':
            charVal = '\t';
            break;

        case 'b':
            charVal = '\b';
            break;

        case 'r':
            charVal = '\r';
            break;

        case 'f':
            charVal = '\f';
            break;

        case '\\':
        	charVal = '\\';
        	break;

        case '\'':
        	charVal = '\'';
        	break;

        case '0':
			if ( ( input[1] == 'x') || ( input[1] == 'X' ) )
			{
				input += 2;
				n++;
				while (hexdigit(*input, &val))
				{
					charVal = charVal << 4 | val;
					n++;
					input++;
				}
				break;
			}
			else
			{
				charVal = 0;
			}
			break;

        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
			while ( ( *input >= '0' ) && ( *input <= '9' ) )
			{
				charVal = charVal*10 + *input - '0';
				n++;
				input++;
			}
            break;

        default:
        	printf("invalid escape sequence in character string: %x\n", *input);
        	break;
    }

    /* store the number of bytes processed */
    *length = n;
    return charVal;
}

/*
	allCAPS -- translate a string to all capital lettrz
 */

/*============================================================================*/
/*  allCAPS                                                                   */
/*!
    Convert a string to all caps

    The allCAPS function iterates through the characters in the specified
    character string and converts each character to upper case.

    @param[in]
        ch
            pointer to the input string to be converted

==============================================================================*/
static void allCAPS(char *ch)
{
	while (*ch != '\0')
	{
		if ((*ch >= 'a') && (*ch <= 'z'))
		{
			*ch = *ch - (char)32;
		}

		ch++;
	}
}

/*============================================================================*/
/*  xtol                                                                      */
/*!
    Convert a hexadecimal string to a number

    The xtol function converts a hexadecimal string to a 32-bit unsigned
    integer.

    @param[in]
        hexstring
            pointer to the hexadecimal string

    @retval numeric value of the hexadecimal string
    @retval 0 an error occurred

==============================================================================*/
static uint32_t xtol(char *hexstring )
{
	uint32_t value = 0;
	uint16_t digit;

	if( hexstring == NULL )
	{
		return 0;
	}

	while (hexdigit(*hexstring++, &digit))
	{
		value = (value<<4) | digit;
	}

	return value;
}

/*============================================================================*/
/*  hexdigit                                                                  */
/*!
    Convert a hexadecimal digit to an unsigned integer

    The hexdigit function converts a hexadecimal digit into a 16-bit
    unsigned value.

    @param[in]
        digit
            hex digit to process

    @param[in]
        val
            pointer to a location to store the converted hex digit

    @retval 1 digit processed ok
    @retval 0 not a hex digit

==============================================================================*/
static uint16_t hexdigit(char digit, uint16_t *val)
{
	if ((digit >= '0') && (digit <= '9'))
	{
		*val = digit - '0';
		return 1;
	}
	else if ((digit >= 'a') && (digit <= 'f'))
	{
		*val = digit - 'a' + 10;
		return 1;
	}
	else if ((digit >= 'A') && (digit <= 'F'))
	{
		*val = digit - 'A' + 10;
		return 1;
	}
	else
	{
		return 0;
	}
}

/*============================================================================*/
/*  CheckParseInfo                                                            */
/*!
    Update the Parse Information

    update the flags on the instruction based on the attributes of the
    two specified parser information structures

    @param[in]
        instptr
            pointer to the instruction to be updated

    @param[in]
        pParseInfo1
            pointer to parse info of left hand side

    @param[in]
        pParseInfo2
            pointer to parse info of right hand side

    @param[in]
        lineno
            line number

==============================================================================*/
void CheckParseInfo( uint8_t *instptr,
                     tzParseInfo *pParseInfo1,
                     tzParseInfo *pParseInfo2,
                     int lineno )
{
	if( pParseInfo2->type == eREGISTER )
	{
		/* ignore check of registers on RHS of operation */
		return;
	}

	if( pParseInfo2->width > pParseInfo1->width)
	{
		printf( "Line: %d: Parse info width error!"
                " left side = %d, "
                " right side = %d\n",
                lineno,
                pParseInfo1->width,
                pParseInfo2->width );
		exit(1);
	}

	if( pParseInfo1->width > pParseInfo2->width )
	{
        /* adjust width of left hand side to match width of right hand side */
		pParseInfo1->width = pParseInfo2->width;
	}

    if( pParseInfo2->type == eREGISTER )
    {
        instptr[0] |= MODE_REG;
    }

    if( pParseInfo1->width == 2 )
    {
        instptr[0] |= WORD;
    }
    else if( pParseInfo1->width == 1 )
    {
        instptr[0] |= BYTE;
    }

    if( ( pParseInfo1->width == 4 ) && ( pParseInfo2->type == eFLOAT32 ) )
    {
    	instptr[0] |= FLOAT32;
    }
}

/*============================================================================*/
/*  storeValue                                                                */
/*!
    Store a value in VM memory

    The storeValue function stores a value from the tzParseInfo object
    into the Virtual Machine memory at the specified location.
    The value is stored in Big Endian format (MSB first).

    @param[in]
        pParseInfo
            pointer to the tzParseInfo object containing the value to store

    @param[in]
        memory
            pointer to the VM memory location to store the value into

    @param[in]
        address
            unused

    @param[in]
        lineno
            line number

==============================================================================*/
void storeValue( tzParseInfo *pParseInfo,
                 uint8_t *memory,
                 uint16_t address,
                 int lineno )
{
	uint8_t b0;
	uint8_t b1;
    uint8_t b2;
    uint8_t b3;
    uint32_t ulVal;
    uint16_t uiVal;

    union
    {
        float fVal;
        uint8_t uVal[4];
    } data;

    if( pParseInfo == NULL )
    {
        fprintf(stderr, "ProcessVal: Invalid parser info structure!\n");
        exit(1);
    }

    switch( pParseInfo->type )
    {
        case eUINT8:
        	//printf("UINT8\n");
            *memory = pParseInfo->value.ucVal;
            break;

        case eSINT8:
        	//printf("SINT8\n");
            *memory = pParseInfo->value.scVal;
            break;

        case eUINT16:
            uiVal = pParseInfo->value.uiVal;
            b1 = (uint8_t)((uiVal & 0xFF00 ) >> 8);
            b0 = (uint8_t)(uiVal & 0x00FF);
            *memory++ = b1;
            *memory = b0;
            break;

        case eSINT16:
            uiVal = (uint16_t)(pParseInfo->value.siVal);
            b1 = (uint8_t)((uiVal & 0xFF00) >> 8);
            b0 = (uint8_t)(uiVal & 0x00FF);
            *memory++ = b1;
            *memory = b0;
            break;

        case eUINT32:
            ulVal = pParseInfo->value.ulVal;
            b3 = (uint8_t)(( ulVal & 0xFF000000 ) >> 24);
            b2 = (uint8_t)(( ulVal & 0x00FF0000 ) >> 16);
            b1 = (uint8_t)(( ulVal & 0x0000FF00 ) >> 8);
            b0 = (uint8_t)( ulVal & 0x000000FF );
            *memory++ = b3;
            *memory++ = b2;
            *memory++ = b1;
            *memory = b0;
            break;

        case eSINT32:
            ulVal = (uint32_t)(pParseInfo->value.slVal);
            b3 = (uint8_t)(( ulVal & 0xFF000000 ) >> 24);
            b2 = (uint8_t)(( ulVal & 0x00FF0000 ) >> 16);
            b1 = (uint8_t)(( ulVal & 0x0000FF00 ) >> 8);
            b0 = (uint8_t)( ulVal & 0x000000FF );
            *memory++ = b3;
            *memory++ = b2;
            *memory++ = b1;
            *memory = b0;
            break;

        case eFLOAT32:
			data.fVal = pParseInfo->value.fVal;
			memory[0] = data.uVal[3];
			memory[1] = data.uVal[2];
			memory[2] = data.uVal[1];
			memory[3] = data.uVal[0];
            break;

        default:
            printf( "Line: %d: unsupported type: %d\n",
                    lineno,
                    pParseInfo->type );
            exit(1);
            break;
    }
}

/*! @}
 * end of parseinfo group */
