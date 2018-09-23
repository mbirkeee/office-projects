//---------------------------------------------------------------------------
// File:    mbStrUtils.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    Oct. 31, 2002
//---------------------------------------------------------------------------
// Description:
//
// String utilities
//---------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <vcl.h>
#pragma hdrstop

#include "mbTypes.h"
#include "mbLock.h"
#include "mbMalloc.h"
#include "mbDebug.h"
#include "mbDlg.h"
#include "mbLog.h"

#define MB_INIT_GLOBALS
#include "mbStrUtils.h"
#undef  MB_INIT_GLOBALS

//---------------------------------------------------------------------------
// Function: mbStrToFile
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p  mbStrToFileName( Char_p str_p )
{
    Ints_t  len, i;
    Char_p  in_p, out_p;

    if( str_p == NIL ) goto exit;
    len = strlen( str_p );
    in_p = str_p;
    out_p = str_p;

    for( i = 0 ; i < len ; i++ )
    {
        if(    ( *in_p >= 'a' &&  *in_p <= 'z' )
            || ( *in_p >= 'A' &&  *in_p <= 'Z' )
            || ( *in_p >= '0' &&  *in_p <= '9' )
            ||   *in_p == '_'
            ||   *in_p == '-'
            ||   *in_p == '.'    )
        {
            *out_p++ = *in_p;
        }
        in_p++;
    }
    *out_p = 0;

exit:
    return str_p;
}

//---------------------------------------------------------------------------
// Function: mbStrToUpper
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p  mbStrToUpper( Char_p str_p )
{
   Ints_t  len, i;

    if( str_p == NIL ) goto exit;
    len = strlen( str_p );

    for( i = 0 ; i < len ; i++ )
    {
        if( *(str_p + i) >= 'a' && *(str_p + i) <= 'z' ) *(str_p + i ) -= 0x20;
    }
exit:
    return str_p;
}

//---------------------------------------------------------------------------
// Function: mbStrToLower
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p  mbStrToLower( Char_p str_p )
{
    Ints_t  len, i;

    if( str_p == NIL ) goto exit;
    len = strlen( str_p );

    for( i = 0 ; i < len ; i++ )
    {
        if( *(str_p + i) >= 'A' && *(str_p + i) <= 'Z' ) *(str_p + i ) += 0x20;
    }
exit:
    return str_p;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p mbStrAlphaNumericOnly
(
    Char_p      string_p
)
{
    Char_p      p1, p2;

    if( string_p == NULL ) goto exit;

    p1 = p2 = string_p;

    for( ; ; )
    {
        if( *p1 == NULL ) break;

        if( *p1 >= '0' && *p1 <= '9' )
        {
            *p2++ = *p1;
        }
        else if( *p1 >= 'a' && *p1 <= 'z' )
        {
            *p2++ = *p1;
        }
        else if( *p1 >= 'A' && *p1 <= 'Z' )
        {
            *p2++ = *p1;
        }
        p1++;
    }
    *p2 = (Char_t)NULL;

    nbDlgDebug(( "Cleaned number string '%s'", string_p ));

exit:
    return string_p;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p mbStrAlphaOnly
(
    Char_p      string_p
)
{
    Char_p      p1, p2;

    if( string_p == NULL ) goto exit;

    p1 = p2 = string_p;

    for( ; ; )
    {
        if( *p1 == NULL ) break;

         if( *p1 >= 'a' && *p1 <= 'z' )
        {
            *p2++ = *p1;
        }
        else if( *p1 >= 'A' && *p1 <= 'Z' )
        {
            *p2++ = *p1;
        }
        p1++;
    }
    *p2 = (Char_t)NULL;
exit:
    return string_p;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p mbStrDigitsOnly
(
    Char_p      string_p
)
{
    Char_p      p1, p2;

    if( string_p == NULL ) goto exit;

    p1 = p2 = string_p;

    for( ; ; )
    {
        if( *p1 == NULL ) break;

        if( *p1 >= '0' && *p1 <= '9' )
        {
            *p2++ = *p1;
        }

        p1++;
    }
    *p2 = (Char_t)NULL;

    nbDlgDebug(( "Cleaned number string '%s'", string_p ));

exit:
    return string_p;
}

//---------------------------------------------------------------------------
// Function: mbStrRemoveSlashTrailing
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

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
            if( *temp1_p == NULL ) break;
            temp1_p++;
        }
    }
exit:
    return str_p;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Ints_t mbStrPosLen
(
    Char_p  s1,
    Char_p  s2
)
{
    /* Test pointers */
    if( s1 == NIL || s2 == NIL ) return -1;

    if( strlen( s1 ) != strlen( s2 ) ) return -1;

    return mbStrPos( s1, s2 );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

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
    mbMalloc( s1, ( i1 + 1 ) );
    mbMalloc( s2, ( i2 + 1 ) );

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

Char_p mbStrToLatin( Char_p in_p )
{
    Int8u_p     p1, p2;
    Int8u_t     output;

    for( p1 = p2 = (Int8u_p)in_p ; *p1 != NULL ; )
    {
        output = 0;

        // ^0 outputs degrees
        if( *p1 == '^' )
        {
            if( *(p1 + 1) == '0' || *(p1 + 1) == 'o' || *(p1 + 1) == 'O')
            {
                output = MB_CHAR_DEGREES;
            }

            if( *(p1 + 1) == 'a' || *(p1 + 1) == 'A' )
            {
                output = MB_CHAR_ALPHA;
            }

            if( *(p1 + 1) == 'b' || *(p1 + 1) == 'B' )
            {
                output = MB_CHAR_BETA;
            }
        }

        if( output != 0 )
        {
            *p2++ = output;
            p1 += 1;
        }
        else
        {
            *p2++ = *p1;
        }
        p1++;
    }
    *p2 = 0;

    return in_p;
}

//---------------------------------------------------------------------------
// Function:    mbStrStrip
//---------------------------------------------------------------------------
// Description:
//
// Remove the leading and trailing white space in place
//---------------------------------------------------------------------------

Char_p          mbStrStrip( Char_p in_p )
{
    Char_p      p1, p2;
    Ints_t      len;
    Boolean_t   flag;

    if( in_p == NULL ) goto exit;

    // First, remove leading white space
    for( flag = TRUE, p1 = p2 = in_p ; *p1 != NULL ; )
    {
        if( flag )
        {
            if(    *p1 == MB_CHAR_SPACE
                || *p1 == MB_CHAR_TAB
                || *p1 == MB_CHAR_CR
                || *p1 == MB_CHAR_LF
                || *p1 == MB_CHAR_FF )
            {
                p1++;
                continue;
            }
            flag = FALSE;
        }
       *p2++ = *p1++;
    }
    *p2 = (Char_t)NULL;

    // Now remove trailing white space
    len = strlen( in_p );

    if( len == 0 ) goto exit;

    p1 = in_p + len - 1;

    for( ; ; p1-- )
    {
        if(    *p1 == MB_CHAR_SPACE
            || *p1 == MB_CHAR_LF
            || *p1 == MB_CHAR_CR
            || *p1 == MB_CHAR_TAB
            || *p1 == MB_CHAR_FF )
        {
            *p1 = 0;
        }
        else
        {
            break;
        }
        if( p1 == in_p ) break;
    }

exit:
    return in_p;
}

//---------------------------------------------------------------------------
// Function:    mbStrClean
//---------------------------------------------------------------------------
// Description:
//
// This function removes leading and trailing white space, and converts
// any double quoted to single quotes. The output is written to the
// specified buffer.  No length checking is done.
//---------------------------------------------------------------------------

Char_p          mbStrClean
(
    Char_p      in_p,
    Char_p      out_p,
    Boolean_t   backslashFlag
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
        mbMallocStr( out_p, in_p );
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

    nbDlgDebug(( "input:  '%s'\noutput: '%s'", in_p, out_p ));

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
// Function:    mbStrSlash
//---------------------------------------------------------------------------
// Description:
//
//
//---------------------------------------------------------------------------

Char_p          mbStrBackslash
(
    Char_p      in_p,
    Char_p      out_p,
    Int32u_t    flag
)
{
    Ints_t      len, i;
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
        mbMallocStr( out_p, in_p );
        copyToInput = TRUE;
    }
    else
    {
        strcpy( out_p, in_p );
    }

    len = strlen( out_p );
    for( i = 0 ; i < len ; i++ )
    {
        if( flag )
        {
            if( *( out_p + i ) == MB_CHAR_SLASH ) *( out_p + i ) = MB_CHAR_BACKSLASH;
        }
        else
        {
            if( *( out_p + i ) == MB_CHAR_BACKSLASH ) *( out_p + i ) = MB_CHAR_SLASH;
        }
    }

    nbDlgDebug(( "input:  '%s'\noutput: '%s'", in_p, out_p ));

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
// Function: mbStrUpdateRoot
//---------------------------------------------------------------------------
// This function updates replaces the "source_p" component of "string_p"
// with "target_p".
//
// If source_p == NIL, target_p is prepended.
// If target_p == NIL, source_p is removed.
//
// Function writes into the target_p without length checking.  If a NIL
// pointer is passed in as the result, space is allocated for it.
//---------------------------------------------------------------------------

Char_p          mbStrUpdateRoot
(
    Char_p      string_p,
    Char_p      source_p,
    Char_p      target_p,
    Char_p     *result_pp
)
{
    Char_p      return_p = NIL;
    Char_p      src_p = NIL;
    Char_p      tgt_p = NIL;
    Ints_t      srcLen = 0;
    Ints_t      tgtLen = 0;
    Char_p      buf_p;
    Char_p      result_p = NIL;

    if( string_p == NIL ) goto exit;

    if( source_p )
    {
        if( ( srcLen = strlen( source_p ) ) > 0 )
        {
            src_p = source_p;
        }
    }

    if( target_p )
    {
        if( ( tgtLen = strlen( target_p ) ) > 0 )
        {
            tgt_p = target_p;
        }
    }

    // Just allocate some space that should be big enough
    mbMalloc( buf_p, srcLen + tgtLen + strlen( string_p ) + 10 );

    if( src_p && tgt_p )
    {
        // Both source and target are defined.  The source must be located
        // at the root of the string
        if( mbStrPos( string_p, src_p ) != 0 ) goto exit;

        sprintf( buf_p, "%s%s", tgt_p, string_p + srcLen );
    }
    else if( src_p )
    {
        // The source is NIL, so prepend the target
        sprintf( buf_p, "%s%s", tgt_p, string_p );
    }
    else if( tgt_p )
    {
        // The target is NIL, so delete the source
        sprintf( buf_p, "%s", string_p + srcLen );
    }
    else
    {
        // Neither source nor target are defined, this is an error
        goto exit;
    }

    // Write the results to the appropriate location
    if( result_pp == NIL )
    {
        return_p = buf_p;
    }
    else if( *result_pp == NIL )
    {
        mbMallocStr( result_p, buf_p );
        *result_pp = result_p;
        return_p = *result_pp;
        mbFree( buf_p );
    }
    else
    {
        strcpy( *result_pp, buf_p );
        return_p = *result_pp;
        mbFree( buf_p );
    }

exit:

    return return_p;
}

//---------------------------------------------------------------------------
// Function: mbStrListAdd
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t            mbStrListTake
(
    qHead_p         list_q,
    Char_p          str_p
)
{
    return mbStrListAddInternal( list_q, str_p, NIL, 0, 0, TRUE );
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
    return mbStrListAddInternal( list_q, str_p, NIL, 0, 0, FALSE );
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
    return mbStrListAddInternal( list_q, str_p, str2_p, 0, 0 , FALSE );
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t            mbStrListAddNew
(
    qHead_p         list_q,
    Char_p          str_p,
    Char_p          str2_p,
    Int32u_t        handle,
    Int32u_t        handle2
)
{
    return mbStrListAddInternal( list_q, str_p, str2_p, handle, handle2, FALSE );
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t            mbStrListAddEntry
(
    qHead_p         list_q,
    mbStrList_p     entry_p
)
{
    return mbStrListAddInternal( list_q, entry_p->str_p, entry_p->str2_p, entry_p->handle, entry_p->handle2, FALSE );
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

mbStrList_p         mbStrListItemAllocInternal
(
    Char_p          str_p,
    Char_p          str2_p,
    Int32u_t        handle,
    Int32u_t        handle2,
    Boolean_t       takeFlag
)
{
    mbStrList_p     entry_p;

    mbCalloc( entry_p, sizeof( mbStrList_t ) );
    if( entry_p == NIL )
    {
        mbDlgError( "Memory allocation error." );
        goto exit;
    }

    if( takeFlag == FALSE )
    {
        mbMallocStr( entry_p->str_p, str_p );
        if( entry_p->str_p == NIL )
        {
            mbDlgError( "Memory allocation error." );
            mbFree( entry_p );
            goto exit;
        }
    }
    else
    {
        // This list takes ownership of the passed in pointer
        entry_p->str_p = str_p;
    }

    if( str2_p )
    {
        mbMallocStr( entry_p->str2_p, str2_p );
        if( entry_p->str2_p == NIL )
        {
            mbDlgError( "Memory allocation error." );
            mbFree( entry_p->str_p );
            mbFree( entry_p );
            goto exit;
        }
    }

    entry_p->handle = handle;
    entry_p->handle2 = handle2;

exit:
    return entry_p;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

mbStrList_p         mbStrListItemAlloc
(
    Char_p          str_p,
    Char_p          str2_p,
    Int32u_t        handle,
    Int32u_t        handle2
)
{
    return  mbStrListItemAllocInternal( str_p, str2_p, handle, handle2, FALSE );
}

//---------------------------------------------------------------------------
// Function: mbStrListAddInternal
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t            mbStrListAddInternal
(
    qHead_p         list_q,
    Char_p          str_p,
    Char_p          str2_p,
    Int32u_t        handle,
    Int32u_t        handle2,
    Boolean_t       takeFlag
)
{
    mbStrList_p     entry_p;
    Int32s_t        returnCode = MB_RET_ERR;

    if( list_q == NIL || str_p == NIL ) goto exit;

    if( ( entry_p = mbStrListItemAllocInternal( str_p, str2_p, handle, handle2, takeFlag ) ) == NIL )
    {
        goto exit;
    }

    // Add to queue
    qInsertLast( list_q, entry_p );

    returnCode = MB_RET_OK;
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
        mbStrListItemFree( entry_p );
        freeCount++;
    }

exit:
    return freeCount;
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t        mbStrListItemFree
(
    mbStrList_p entry_p
)
{
    Int32s_t    returnCode = MB_RET_ERR;

    if( entry_p == NIL ) goto exit;

    mbFree( entry_p->str2_p );
    mbFree( entry_p->str_p );
    mbFree( entry_p );

    returnCode = MB_RET_OK;
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
    Int32s_t    returnCode = MB_RET_ERR;
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
            returnCode = MB_RET_OK;
            break;
        }
    }

exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:    mbStrListLog
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t        mbStrListLog
(
    qHead_p     list_q
)
{
    Int32s_t    returnCode = MB_RET_ERR;
    mbStrList_p entry_p;
    Int32u_t    i = 0;

    if( list_q == NIL ) goto exit;

    qWalk( entry_p, list_q, mbStrList_p )
    {
        if( entry_p->str_p )
        {
            if( entry_p->str2_p )
            {
                mbLog( "strList entry %d: '%s' '%s'\n", i++, entry_p->str_p, entry_p->str2_p );
            }
            else
            {
                mbLog( "strList entry %d: '%s'\n", i++, entry_p->str_p );
            }
        }
        else
        {
            mbLog( "strList entry %d: NIL\n", i++ );
        }
    }

    returnCode = MB_RET_OK;
exit:
    return returnCode;
}

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
// Function: mbDollarStrInt32u
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Char_p          mbDollarStrInt32u
(
    Int32u_t    number,
    Char_p      result_p
)
{
    Int32u_t    dollars;
    Int32u_t    cents;
    Char_t      buf[64];

    if( result_p == NIL ) goto exit;

    dollars = number / 100;
    cents = number - dollars * 100;

    sprintf( result_p, "%s.%02ld", mbStrInt32u( dollars, buf ), cents );
exit:
    return result_p;
}
