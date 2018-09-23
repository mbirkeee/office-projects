//---------------------------------------------------------------------------
// File:    mbMalloc.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    September 7, 2002
//---------------------------------------------------------------------------
// Description:
//
// Utility functions
//---------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vcl.h>
#pragma hdrstop

#include "mbTypes.h"
#include "mbDebug.h"
#include "mbLock.h"
#include "mbQueue.h"
#include "mbDlg.h"

#define MB_INIT_GLOBALS
#include "mbMalloc.h"
#undef  MB_INIT_GLOBALS

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t    mbMallocChunks
(
    void
)
{
    return mbAllocatedMemoryChunks;
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32s_t    mbMallocInit
(
    Boolean_t   listFlag
)
{
#if MB_MALLOC_DEBUG
    if( mbMallocInitFlag == TRUE )
    {
        // Error.. already initialized
        return FALSE;
    }
    else
    {
        mbMallocListFlag = listFlag;
        mbMallocInitFlag = TRUE;
        mbMalloc_q = qInitialize( &mbMallocQueue );
        return TRUE;
    }
#else
    return TRUE;
#endif
}

#if MB_MALLOC_DEBUG
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Void_p          mbMallocFunc
(
    Void_p     *ptr_pp,
    Ints_t      size,
    Char_p      file_p,
    Int32u_t    line,
    Boolean_t   callocFlag
)
{
    size_t      mysize = size;
    Void_p      ptr_p = NIL;


    if( mbMallocInitFlag == FALSE )
    {
        mbMallocInit( FALSE );
    }

    if( mbMallocListFlag )
    {
        mbMemList_p     mem_p;

        if( mysize == 0 ) mysize = 8L;
        if( ( ptr_p = malloc( mysize ) ) == NIL )
        {
            mbDlgDebug(( "Memory Allocation error: bytes:%ld line: %ld file: '%s'", mysize, line, file_p ));
            goto exit;
        }

        mem_p = (mbMemList_p)malloc( sizeof( mbMemList_t ) );
        if( mem_p == NIL )
        {
            mbDlgDebug(( "Memory Allocation error: bytes:%ld line: %ld file: '%s'", mysize, line, file_p ));
            free( ptr_p );
            goto exit;
        }

        mem_p->size = size;
        mem_p->ptr = ptr_p;
        mem_p->line = line;
        mem_p->file_p = (Char_p)malloc( strlen( file_p ) + 1 );
        if( mem_p->file_p == NIL )
        {
            mbDlgDebug(( "Memory Allocation error: bytes:%ld line: %ld file: '%s'", mysize, line, file_p ));
            free( ptr_p );
            free( mem_p );
            goto exit;
        }

        strcpy( mem_p->file_p, file_p );
        qInsertFirst( mbMalloc_q, mem_p );
    }
    else
    {
        if( mysize == 0 ) mysize = 8L;
        if( ( ptr_p = malloc( mysize ) ) == NIL )
        {
            mbDlgDebug(( "Memory Allocation error (size:%d)", (Ints_t)mysize ));
            goto exit;
        }
    }

    if( callocFlag ) memset( ptr_p, 0, mysize );

    mbAllocatedMemoryChunks++;
    *ptr_pp = ptr_p;

exit:
    return ptr_p;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void                mbFreeFunc
(
    Void_p          ptr_p,
    Char_p          fileName_p,
    Int32u_t        line
)
{
    if( mbMallocListFlag )
    {
        mbMemList_p    mem_p;

        if( ptr_p != NIL )
        {
            qWalk( mem_p, mbMalloc_q, mbMemList_p )
            {
                if( mem_p->ptr == ptr_p )
                {
                    free( ptr_p );
                    ptr_p = NIL;
                    mbAllocatedMemoryChunks--;
                    qRemoveEntry( mbMalloc_q, mem_p );
                    if( mem_p->file_p ) free( mem_p->file_p );
                    free( mem_p );
                    break;
                }
            }
            if( ptr_p != NIL )
            {
                mbDlgDebug(( "%s %ld: Could not find memory in allocated list!!!", fileName_p, line ));
            }
        }
    }
    else
    {
        if( ptr_p != NIL )
        {
            free( ptr_p );
            mbAllocatedMemoryChunks--;
        }
    }
    return;
}
#else

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Void_p  mbMallocFunc
(
    Void_p     *ptr_pp,
    Ints_t      size,
    Boolean_t   callocFlag
)
{
    size_t      mysize = size;
    Void_p      ptr_p;

    if( mysize == 0 ) mysize = 8L;
    if( ( ptr_p = malloc( mysize ) ) == NIL )
    {
        mbDlgDebug(( "Memory Allocation error (size:%d)", (Ints_t)mysize ));
        goto exit;
    }
    if( callocFlag ) memset( ptr_p, 0, mysize );

    mbAllocatedMemoryChunks++;
    *ptr_pp = ptr_p;

exit:
    return ptr_p;
}
#endif

//---------------------------------------------------------------------------
// Function:    mbMallocDump
//---------------------------------------------------------------------------
// Description: Display list of allocated memory
//---------------------------------------------------------------------------

void                mbMallocDump( void )
{
#if MB_MALLOC_DEBUG
    mbMemList_p     mem_p;
    Char_p          buf_p;
    Char_p          buf2_p;
    Boolean_t       displayFlag = TRUE;

    buf_p  = (Char_p)malloc( 2048 );
    buf2_p = (Char_p)malloc( 512 );

    sprintf( buf2_p, "Display next chunk?" );

    while( !qEmpty( mbMalloc_q ) )
    {
        mem_p = (mbMemList_p)qRemoveFirst( mbMalloc_q );

        sprintf( buf_p, "Size: %d File: %s, Line: %d\n",
                mem_p->size,
                mem_p->file_p,
                mem_p->line );

        if( displayFlag == TRUE )
        {
            if( mbMalloc_q->size > 0 )
            {
                strcat( buf_p, buf2_p );

                if( mbDlgYesNo( buf_p ) == MB_BUTTON_NO ) displayFlag = FALSE;
            }
            else
            {
                mbDlgExclaim( buf_p );
            }
        }

        // Free the memory
        if( mem_p->ptr    ) free( mem_p->ptr    );
        if( mem_p->file_p ) free( mem_p->file_p );
        free( mem_p );
    }
    free( buf_p );
    free( buf2_p );
#endif
    return;
}

//---------------------------------------------------------------------------

Int32u_t mbObjectCountInc( void )
{
    mbObjectCount++;
    return mbObjectCount;
}

//---------------------------------------------------------------------------

Int32u_t mbObjectCountDec( void )
{
    if( mbObjectCount == 0 )
    {
        mbDlgError( "Object count unexpectedly 0" );
    }
    else
    {
        mbObjectCount--;
    }
    return mbObjectCount;
}

//---------------------------------------------------------------------------

Int32u_t mbObjectCountGet( void )
{
    return mbObjectCount;
}

