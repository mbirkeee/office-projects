//---------------------------------------------------------------------------
// File:    mbLock.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    Feb. 9, 2002
//---------------------------------------------------------------------------
// Description:
//
// Debugging functions
//---------------------------------------------------------------------------

#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include <vcl.h>

#pragma hdrstop

#include "mbTypes.h"
#include "mbDebug.h"
#include "mbDlg.h"
#include "mbLog.h"

#define MB_INIT_GLOBALS
#include "mbLock.h"
#undef  MB_INIT_GLOBALS

Void_t  MB_LOCK_DESTROY( MB_LOCK_p lock_p )
{
    if( lock_p == NIL )
    {
        mbDlgError( "MB_LOCK_DESTROY: NIL pointer" );
        goto exit;
    }

    if( lock_p->magic != MB_LOCK_MAGIC )
    {
        mbDlgError( "MB_LOCK_DESTROY: Lock not initialized" );
        goto exit;
    }

    // Sanity Check... acquire lock before destroying it
    if( WaitForSingleObject( lock_p->handle, INFINITE ) == WAIT_FAILED )
    {
        mbDlgError( "MB_LOCK_DESTROY: WaitForSingleObject() failed" );
        goto exit;
    }

    if( ReleaseMutex( lock_p->handle ) == 0  )
    {
        mbDlgError( "MB_LOCK_DESTROY: ReleaseMutex() failed" );
        goto exit;
    }

    if( CloseHandle( lock_p->handle ) != TRUE )
    {
         mbDlgError( "MB_LOCK_DESTROY: CloseHandle() failed" );
         goto exit;
    }

    if( mbAllocatedLocks > 0 )
    {
        mbAllocatedLocksNew--;
    }
    else
    {
        mbDlgError( "MB_LOCK_DESTROY: mbAllocatedLocks == 0" );
    }

exit:

    lock_p->magic = 0;
    lock_p->handle = NIL;

    return;
}

//---------------------------------------------------------------------------
MB_LOCK_p MB_LOCK_INIT( MB_LOCK_p lock_p )
{
    if( lock_p == NIL )
    {
        mbDlgError( "MB_LOCK_INIT: NIL pointer" );
        goto exit;
    }

    if( lock_p->magic == MB_LOCK_MAGIC )
    {
        mbDlgError( "MB_LOCK_INIT: Lock already initialized" );
        goto exit;
    }

    if( ( lock_p->handle = CreateMutex( NIL, FALSE, NULL ) ) == NIL )
    {
        mbDlgError( "MB_LOCK_INIT: CreateMutex() failed" );
        lock_p->magic = 0;
        goto exit;
    }
    lock_p->magic = MB_LOCK_MAGIC;
    lock_p->count = 0;
    lock_p->threadId = 0;

     mbAllocatedLocksNew++;
exit:
    return lock_p;
}

//---------------------------------------------------------------------------
Void_t  MB_LOCK_ACQUIRE( MB_LOCK_p lock_p )
{
    Int64u_t    threadId;

    if( lock_p == NIL )
    {
        mbDlgError( "MB_LOCK_ACQUIRE: lock not initialized" );
        goto exit;
    }

    if( WaitForSingleObject( lock_p->handle, INFINITE ) == WAIT_FAILED )
    {
        mbDlgError( "MB_LOCK_ACQUIRE: WaitForSingleObject() failed" );
        goto exit;
    }

    threadId = _threadid;

    if( threadId == lock_p->threadId )
    {
        mbDlgInfo( "MB_LOCK_ACQUIRE: Already have this lock" );
        ReleaseMutex( lock_p->handle );
        goto exit;
    }

    lock_p->threadId = threadId;
    //mbDlgInfo( "Setting lock ID to %Ld", lock_p->threadId );
exit:
    return;
}

//---------------------------------------------------------------------------
Void_t  MB_LOCK_RELEASE( MB_LOCK_p lock_p )
{
    Int64u_t    threadId;

    if( lock_p == NIL )
    {
        mbDlgError( "MB_LOCK_RELEASE: NIL pointer" );
        goto exit;
    }

    threadId = _threadid;

    if( threadId != lock_p->threadId )
    {
        mbDlgError( "MB_LOCK_RELEASE: lock not held" );
        goto exit;
    }

    lock_p->threadId = 0;
    if( ReleaseMutex( lock_p->handle ) == 0  )
    {
        mbDlgError( "MB_LOCK_RELEASE: ReleaseMutex() failed" );
    }

exit:
    return;
}

//---------------------------------------------------------------------------

Int32s_t        mbLockAttemptFunc
(
    mbLock_p    lock_p,
    Char_p      file_p,
    Int32s_t    line
)
{
    Int32s_t    returnCode = FALSE;

#if MB_DEBUG_LOCK
    if( lock_p->count == 0 )
    {
        lock_p->count++;
        sprintf( lock_p->location, "%s:%ld", file_p, line );
        returnCode = TRUE;
    }
#else
    if( *lock_p == 0 )
    {
        *lock_p += 1;
        returnCode = TRUE;
    }
#endif

    return returnCode;
}

//---------------------------------------------------------------------------

Void_t        mbLockAcquireFunc
(
    mbLock_p    lock_p,
    Char_p      file_p,
    Int32s_t    line
)
{
    time_t      timer;
    struct tm  *tblock_p;

    timer    = time( NULL );
    tblock_p = localtime( &timer );

    if( ++lock_p->count != 1 )
    {
        mbLog( "Acquire Lock Error: %s:%ld - Previously locked at: %s (count: %d)",
            file_p, line, lock_p->location, lock_p->count );
        lock_p->count = 1;
    }
    sprintf( lock_p->location, "%02d:%02d:%02d:%s:%ld",
        tblock_p->tm_hour, tblock_p->tm_min, tblock_p->tm_sec, file_p, line );

    return;
}



