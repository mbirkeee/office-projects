/******************************************************************************
 * Copyright (c) 2007-2008, Michael A. Bree
 *-----------------------------------------------------------------------------
 * Description:
 *
 * Various utility functions
 *-----------------------------------------------------------------------------
 */

/* System include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Project include files */
#include "mbUtils.h"

//---------------------------------------------------------------------------
// Function: mbStrInt64u
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Char_p          mbStrInt64u
(
    Int64u_t    number,
    Char_p      result_p
)
{
    int     i, j, k, l;
    Char_t  b1[64];
    Char_t  b2[64];

    if( result_p == NIL ) goto exit;

    if( number == 0 )
    {
        sprintf( result_p, "0" );
        goto exit;
    }

    sprintf( b1, "%Ld", number );
    l = strlen( b1 );
    for( i = l - 1, j = 0, k = 0 ; i >= 0 ; i-- )
    {
        if( k == 3 )
        {
            b2[j++] = ',';
            k = 0;
        }
        b2[j++] = b1[i];
        k++;
    }
    b2[j] = 0;

    l = strlen( b2 );
    for( i = l - 1, j = 0 ; i >= 0 ; i-- )
    {
        *(result_p + j++ ) = b2[i];
    }
    *( result_p + j ) = 0;
exit:

    return result_p;
}

//---------------------------------------------------------------------------
// Function: mbStrInt32u
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Char_p          mbStrInt32u
(
    Int32u_t    number,
    Char_p      result_p
)
{
    int     i, j, k, l;
    Char_t  b1[64];
    Char_t  b2[64];

    if( result_p == NIL ) goto exit;

    if( number == 0 )
    {
        sprintf( result_p, "0" );
        goto exit;
    }

    sprintf( b1, "%ld", number );
    l = strlen( b1 );
    for( i = l - 1, j = 0, k = 0 ; i >= 0 ; i-- )
    {
        if( k == 3 )
        {
            b2[j++] = ',';
            k = 0;
        }
        b2[j++] = b1[i];
        k++;
    }
    b2[j] = 0;

    l = strlen( b2 );
    for( i = l - 1, j = 0 ; i >= 0 ; i-- )
    {
        *(result_p + j++ ) = b2[i];
    }
    *( result_p + j ) = 0;
exit:

    return result_p;
}

//---------------------------------------------------------------------------
// Function: mbDateStringFromInt
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p  mbDateStringFromInt( Int32u_t date, Char_p str_p )
{
    Int32u_t    temp;
    Int32u_t    iYear, iMonth, iDay;

    if( str_p == NULL ) goto exit;

    temp = date/10000;
    iYear = temp;

    date = date - ( temp * 10000 );
    temp = date/100;

    iMonth = temp;

    date = date - ( temp * 100 );

    iDay = date;

    sprintf( str_p, "%04lu-%02lu-%02lu", iYear, iMonth, iDay );

exit:
    return str_p;
}

/******************************************************************************
 * Function: mbDateIntFromString( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */
Int32u_t mbDateIntFromString( Char_p str_p )
{
    Int32u_t    result = 0;
    Char_p      temp_p;

    if( str_p == NIL ) goto exit;

    for( temp_p = str_p ; *temp_p != (Char_t)NULL ; temp_p++ )
    {
        if( *temp_p >= '0' && *temp_p <= '9' )
        {
            result *= 10;
            result += (Int32u_t)(*temp_p - '0');
        }
    }

exit:

    // fprintf( stdout, "str: '%s' val: %lu\n", str_p, result );
    return result;
}

/******************************************************************************
 * Function: mbDateBackOneDay( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * Subtract one day from the supplied date int.
 *-----------------------------------------------------------------------------
 */
Int32u_t mbDateDaysInMonth( Int32u_t year, Int32u_t month )
{
    if( month ==  1 ) return 31;
    if( month ==  2 ) return mbDateIsYearLeapYear( year ) == TRUE ? 29 : 28;
    if( month ==  3 ) return 31;
    if( month ==  4 ) return 30;
    if( month ==  5 ) return 31;
    if( month ==  6 ) return 30;
    if( month ==  7 ) return 31;
    if( month ==  8 ) return 31;
    if( month ==  9 ) return 30;
    if( month == 10 ) return 31;
    if( month == 11 ) return 30;
    if( month == 12 ) return 31;
    return 0;
}

/******************************************************************************
 * Function: mbDateIsYearLeapYear( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *
 *-----------------------------------------------------------------------------
 */

Int32u_t mbDateIsYearLeapYear( Int32u_t year )
{
    Int32u_t  result = FALSE;

    if( ( year % 400 ) == 0 )
    {
        result = TRUE;
    }
    else if( ( year % 100 ) == 0 )
    {
        result = FALSE;
    }
    else if( ( year % 4 ) == 0 )
    {
        result = TRUE;
    }

    return result;
}

/******************************************************************************
 * Function: mbDateBackOneDay( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * Subtract one day from the supplied date int.
 *-----------------------------------------------------------------------------
 */

Int32u_t mbDateBackOneDay( Int32u_t dateIn )
{
    Int32u_t    result = 0;
    Int32u_t    year;
    Int32u_t    month;
    Int32u_t    day;
    Int32u_t    remainder;

    year = dateIn / 10000;
    remainder = dateIn - ( year * 10000 );
    month = remainder / 100;
    remainder -= ( month * 100 );
    day = remainder;

    // printf( "Date in: %lu years: %lu month: %lu day %lu\n", dateIn, year, month, day );
    day = day - 1;

    if( day == 0 )
    {
        month = month - 1;
        if( month == 0 )
        {
            month = 12;
            year = year - 1;
        }
        day = mbDateDaysInMonth( year, month );
    }
    result = year * 10000 + month * 100 + day;

    return result;
}
/******************************************************************************
 * Function: mbStrClean( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */
Char_p          mbStrCleanWhite
(
    Char_p      in_p,
    Char_p      out_p
)
{
    Char_p      temp1_p, temp2_p;
    Ints_t      len, i;
    Boolean_t   nonWhiteFound;
    Boolean_t   copyToInput = FALSE;

    if( in_p == 0 )
    {
        if( out_p ) *out_p = 0;
        goto exit;
    }

    len = strlen( in_p );

    if( len == 0 )
    {
        if( out_p ) *out_p = 0;
        goto exit;
    }

    // Allocate space for the result... then copy back to input buffer
    if( out_p == NIL )
    {
        out_p = mbMallocStr( in_p );
        copyToInput = TRUE;
    }
    else
    {
        strcpy( out_p, in_p );
    }

    // Eliminate trailing white space
    temp1_p = out_p + len - 1;

    for( ; ; temp1_p-- )
    {
        if(    *temp1_p == MB_CHAR_SPACE
            || *temp1_p == MB_CHAR_LF
            || *temp1_p == MB_CHAR_CR
            || *temp1_p == MB_CHAR_TAB
            || *temp1_p == MB_CHAR_FF )
        {
            *temp1_p = 0;
        }
        else
        {
            break;
        }
        if( temp1_p == out_p ) break;
    }

    // Eliminate leading white space
    temp1_p = out_p;
    temp2_p = out_p;
    nonWhiteFound = FALSE;
    len = strlen( out_p );
    for( i = 0 ; i < len ; i++ )
    {
        if( nonWhiteFound )
        {
            *temp2_p++ = *temp1_p++;
        }
        else
        {
            if(    *temp1_p == MB_CHAR_SPACE
                || *temp1_p == MB_CHAR_TAB
                || *temp1_p == MB_CHAR_CR
                || *temp1_p == MB_CHAR_LF
                || *temp1_p == MB_CHAR_FF )
            {
                temp1_p++;
                continue;
            }
            nonWhiteFound = TRUE;
            *temp2_p++ = *temp1_p++;
        }
    }
    *temp2_p = 0;

#if 0
    // Convert double quotes to single quotes
    len = strlen( out_p );
    for( i = 0, temp1_p = out_p ; i < len  ; i++, temp1_p++ )
    {
        if( *temp1_p == MB_CHAR_QUOTE_DOUBLE )
        {
            *temp1_p = MB_CHAR_QUOTE_SINGLE;
        }
        else if( *temp1_p == MB_CHAR_BACKSLASH )
        {
            if( backslashFlag == TRUE )
            {
                *temp1_p = '|';
            }
        }
    }
#endif

exit:

    if( copyToInput )
    {
        strcpy( in_p, out_p );
        mbFree( out_p );
        return in_p;
    }
    else
    {
        return out_p;
    }
}

//---------------------------------------------------------------------------
// Function: mbStrRemoveSlashLeading
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p  mbStrRemoveSlashLeading( Char_p str_p )
{
    Ints_t      len;
    Char_p      temp1_p;
    Char_p      temp2_p;
    Ints_t      i;

    if( str_p == NIL ) goto exit;
    len = strlen( str_p );

    temp1_p = str_p;
    temp2_p = str_p;

    // Move temp1_p pointer past leading slashes
    for( i = 0 ; i < len ; i++ )
    {
        if( *temp1_p != MB_CHAR_SLASH && *temp1_p != MB_CHAR_BACKSLASH ) break;
        temp1_p++;
    }

    // Copy result to original string
    if( temp1_p != temp2_p )
    {
        for( ; ; )
        {
            *temp2_p++ = *temp1_p;
            if( *temp1_p == (Char_t)NULL ) break;
            temp1_p++;
        }
    }
exit:
    return str_p;
}
/******************************************************************************
 * Function: mbStrRemoveSlashTrailing( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Char_p  mbStrRemoveSlashTrailing( Char_p str_p )
{
    Ints_t  len;

    if( str_p == NIL ) goto exit;
    len = strlen( str_p );

    for( ; ; )
    {
        if( len == 0 ) break;

        if(    *( str_p + len - 1 ) == MB_CHAR_BACKSLASH
            || *( str_p + len - 1 ) == MB_CHAR_SLASH )
        {
            *( str_p + len - 1 ) = 0;
        }
        else
        {
            break;
        }
        len--;
    }

exit:
    return str_p;
}

/******************************************************************************
 * Function: strPos( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns the starting position of substring i2 within i1.
 *-----------------------------------------------------------------------------
 */

Ints_t mbStrPos
(
    Char_p  s1,
    Char_p  s2
)
{
    /* Check if s2 is anywhere within s1 */
    Ints_t i1, i2, i;

    /* Test pointers */
    if( s1 == NIL || s2 == NIL ) return -1;

    /* Get number of possible places to search */
    i1 = strlen( s1 );
    i2 = strlen( s2 );
    i1 = i1 - i2;

    /* return -1 if s1 < s2 */
    if( i1 < 0 )
    {
        return -1;
    }

    /* Search for the string */
    for( i=0 ; i <= i1 ; i++, s1++ )
    {
        if( strncmp( s1, s2, i2 ) == 0 )
        {
            /* string is found, return starting position */
            return i;
        }
    }
    /* Not Found */
    return -1;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
// Create a seprate function for strPosIgnoreCase so that we can avoid
// memory allocations in the regular strPos()
//---------------------------------------------------------------------------

Ints_t mbStrPosIgnoreCase
(
    Char_p          s1in,
    Char_p          s2in
)
{
    /* Check if s2 is anywhere within s1 */
    Int32s_t        i1, i2, i;
    Int32s_t        returnCode = -1;
    Int8u_p         s1;
    Int8u_p         s2;
    Int8u_p         s1free = NIL;
    Int8u_p         s2free = NIL;

    /* Test pointers */
    if( s1in == NIL || s2in == NIL ) goto exit;

     /* Get number of possible places to search */
    i1 = strlen( s1in );
    i2 = strlen( s2in );

    /* Copy strings so we can change case without affecting originals */
    s1 = mbMalloc( i1 + 1 );
    s2 = mbMalloc( i2 + 1 );

    s1free = s1;
    s2free = s2;

    strcpy( (Char_p)s1, s1in );
    strcpy( (Char_p)s2, s2in );

    for( i = 0 ; i < i1 ; i++ )
    {
        if( s1[i] >= (Int8u_t)'a' && s1[i] <= (Int8u_t)'z' ) s1[i] -= (Int8u_t)0x20;
    }
    for( i = 0 ; i < i2 ; i++ )
    {
        if( s2[i] >= (Int8u_t)'a' && s2[i] <= (Int8u_t)'z' ) s2[i] -= (Int8u_t)0x20;
    }

    i1 = i1 - i2;

    /* return -1 if s1 < s2 */
    if( i1 < 0 ) goto exit;

    /* Search for the string */
    for( i=0; i <= i1; i++, s1++ )
    {
        if( strncmp( (Char_p)s1, (Char_p)s2, i2 ) == 0 )
        {
            /* string is found, return starting position */
            returnCode = i;
            break;
        }
    }

exit:
    if( s1free ) mbFree( s1free );
    if( s2free ) mbFree( s2free );

    return returnCode;
}

//---------------------------------------------------------------------------
// Function: mbStrListAdd
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t            mbStrListAdd
(
    qHead_p         list_q,
    Char_p          str_p
)
{
    return mbStrListAddNew( list_q, str_p, NIL, 0 );
}

//---------------------------------------------------------------------------
// Function: mbStrListAdd
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t            mbStrListAddPair
(
    qHead_p         list_q,
    Char_p          str_p,
    Char_p          str2_p
)
{
    return mbStrListAddNew( list_q, str_p, str2_p, 0 );
}

//---------------------------------------------------------------------------
// Function: mbStrListAddNew
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t            mbStrListAddNew
(
    qHead_p         list_q,
    Char_p          str_p,
    Char_p          str2_p,
    Int32u_t        handle
)
{
    mbStrList_p     entry_p;
    Int32s_t        returnCode = FALSE;

    if( list_q == NIL || str_p == NIL ) goto exit;

    entry_p = (mbStrList_p)mbCalloc( sizeof( mbStrList_t ) );
    if( entry_p == NIL )
    {
        fprintf( stderr, "Memory allocation error.\n" );
        goto exit;
    }

    entry_p->str_p = mbMallocStr( str_p );
    if( entry_p->str_p == NIL )
    {
        fprintf( stderr, "Memory allocation error.\n" );
        mbFree( entry_p );
        goto exit;
    }

    if( str2_p )
    {
        entry_p->str2_p = mbMallocStr( str2_p );
        if( entry_p->str2_p == NIL )
        {
            fprintf( stderr, "Memory allocation error.\n" );
            mbFree( entry_p->str_p );
            mbFree( entry_p );
            goto exit;
        }
    }
    // Add to queue
    entry_p->handle = handle;
    qInsertLast( list_q, entry_p );

    returnCode = TRUE;
exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:    mbStrListFree
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32u_t        mbStrListFree
(
    qHead_p     list_q
)
{
    Int32u_t    freeCount = 0;
    mbStrList_p entry_p;

    if( list_q == NIL ) goto exit;

    while( !qEmpty( list_q ) )
    {
        entry_p = (mbStrList_p)qRemoveFirst( list_q );
        mbStrListFreeElement( entry_p );
        freeCount++;
    }

exit:
    return freeCount;
}

//---------------------------------------------------------------------------
// Function:    mbStrListFreeElement
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t        mbStrListFreeElement
(
    mbStrList_p entry_p
)
{
    Int32s_t    returnCode = FALSE;

    if( entry_p == NIL ) goto exit;

    mbFree( entry_p->str2_p );
    mbFree( entry_p->str_p );
    mbFree( entry_p );

    returnCode = TRUE;
exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:    mbStrListCheck
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t        mbStrListCheck
(
    qHead_p     list_q,
    Char_p      str_p
)
{
    return  mbStrListCheckCache( list_q, str_p, FALSE );
}

//---------------------------------------------------------------------------
// Function:    mbStrListCheckCache
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t        mbStrListCheckCache
(
    qHead_p     list_q,
    Char_p      str_p,
    Boolean_t   cacheFlag
)
{
    Int32s_t    returnCode = false;
    mbStrList_p entry_p;

    if( list_q == NIL || str_p == NIL ) goto exit;

    qWalk( entry_p, list_q, mbStrList_p )
    {
        if( strcmp( entry_p->str_p, str_p ) == 0 )
        {
            if( cacheFlag )
            {
                // Move hit to front of list
                qRemoveEntry( list_q, entry_p );
                qInsertFirst( list_q, entry_p );
            }
            returnCode = true;
            break;
        }
    }

exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:    mbDigitsOnly
//---------------------------------------------------------------------------
// Remove all but numeric (0-9) chars from input string
//---------------------------------------------------------------------------

Char_p mbDigitsOnly
(
    Char_p      string_p
)
{
    Char_p      p1, p2;

    if( string_p == (Char_p)NULL ) goto exit;

    p1 = p2 = string_p;

    for( ; ; )
    {
        if( *p1 == (Char_t)NULL ) break;

        if( *p1 >= '0' && *p1 <= '9' )
        {
            *p2++ = *p1;
        }

        p1++;
    }
    *p2 = (Char_t)NULL;

exit:
    return string_p;
}
