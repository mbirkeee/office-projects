/******************************************************************************
 * Copyright (c) 2007, Michael A. Bree
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 * $Header: /home/cvs/mbUtilsLinux/src/mbMalloc.c,v 1.1.1.1 2007/07/10 05:06:03 mikeb Exp $
 *-----------------------------------------------------------------------------
 * $Log: mbMalloc.c,v $
 * Revision 1.1.1.1  2007/07/10 05:06:03  mikeb
 * Initial Import
 *
 *-----------------------------------------------------------------------------
 */

/* System include files */
#include <stdio.h>
#include <string.h>

#include "mbTypes.h"
#include "mbMalloc.h"

static Int32u_t gTotalCount = 0;

/******************************************************************************
 * Function: mbMallocCount( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Int32u_t    mbMallocCount( Void_t )
{
    return gTotalCount;
}

/******************************************************************************
 * Function: mbFree( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Void_t  mbFree( Void_p ptr_p )
{
    if( ptr_p )
    {
        free( ptr_p );
        gTotalCount--;
    }
    return;
}

/******************************************************************************
 * Function: mbCalloc( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Void_p mbCalloc( int size )
{
    Void_p return_p = calloc( 1, size );
    if( return_p == NIL )
    {
        fprintf( stderr, "*** ERROR: mbMalloc failed\n" );
    }
    else
    {
        gTotalCount++;
    }
    return return_p;
}

/******************************************************************************
 * Function: mbMalloc( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Void_p mbMalloc( int size )
{
    Void_p return_p = malloc( size );
    if( return_p == NIL )
    {
        fprintf( stderr, "*** ERROR: mbMalloc failed\n" );
    }
    else
    {
        gTotalCount++;
    }
    return return_p;
}

/******************************************************************************
 * Function: mbMallocStr( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Void_p mbMallocStr( Char_p str_p )
{
    Void_p  return_p = NIL;

    size_t  len = 0;
    if( str_p != NIL )
    {
        len = strlen( str_p );
        return_p = mbMalloc( len + 1 );
        if( return_p )
        {
            strcpy( return_p, str_p );
        }
    }
    return return_p;
}


