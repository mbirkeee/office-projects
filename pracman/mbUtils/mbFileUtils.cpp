//---------------------------------------------------------------------------
// File:    mbFileUtils.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    Oct. 31, 2002
//---------------------------------------------------------------------------
// Description:
//
// File utilities and functions
//---------------------------------------------------------------------------

#include <stdio.h>
#include <dos.h>
#include <dir.h>
#include <vcl.h>
#include <mmsystem.h>   // For call to mciSendCommand()
#pragma hdrstop

#include "mbTypes.h"
#include "mbLock.h"
#include "mbMalloc.h"
#include "mbDebug.h"
#include "mbDlg.h"
#include "mbLog.h"
#include "mbStrUtils.h"
#include "mbThermometer.h"
#include "mbCrc.h"
#include "mbUtils.h"

#define MB_INIT_GLOBALS
#include "mbFileUtils.h"
#undef  MB_INIT_GLOBALS

//---------------------------------------------------------------------------
// Function:    mbFileNameExtStrip
//---------------------------------------------------------------------------
// Strips the extension from a file name
//---------------------------------------------------------------------------

Char_p          mbFileNameExtStrip
(
    Char_p          name_p,
    Char_p          out_p
)
{
    Char_p          temp_p = NIL;
    Char_p          return_p = NIL;
    Ints_t          len, i;

    if( name_p == NIL ) goto exit;

    mbMalloc( temp_p, MAX_PATH );

    strcpy( temp_p, name_p );

    len = strlen( temp_p );
    if( len == 0 ) goto exit;

    for( i = len - 1 ; i >= 0 ; i-- )
    {
        if( *( temp_p + i ) == '.' )
        {
            *( temp_p + i ) = 0;
            break;
        }
    }

    if( out_p )
    {
        strcpy( out_p, temp_p );
        return_p = out_p;
    }
    else
    {
        strcpy( name_p, temp_p );
        return_p = name_p;
    }

exit:
    if( temp_p ) mbFree( temp_p );
    return return_p;
}

//---------------------------------------------------------------------------
// Function:    mbFileNamePathStrip
//---------------------------------------------------------------------------
// Strips the path from a file name
//---------------------------------------------------------------------------

Char_p          mbFileNamePathStrip
(
    Char_p          name_p,
    Char_p          out_p
)
{
    qHead_t         queueHead;
    qHead_p         queue_p;
    mbNamePart_p    entry_p;

    queue_p = qInitialize( &queueHead );
    if( out_p )
    {
        *out_p = NULL;
    }
    else
    {
        goto exit;
    }

    if( name_p == NIL ) goto exit;

    if( mbFileNamePartsGet( name_p, queue_p ) == 0 ) goto exit;

    if( queue_p->size == 0 ) goto exit;

    entry_p = (mbNamePart_p)qRemoveLast( queue_p );
    qInsertLast( queue_p, entry_p );
    strcpy( out_p, entry_p->str_p );

    mbStrRemoveSlashLeading( out_p );

exit:
    mbFileNamePartsFree( queue_p );
    return out_p;
}

//---------------------------------------------------------------------------
// Function:    mbFileNamePartsGet
//---------------------------------------------------------------------------
// Description:
//
// Returns part (component) count.
//---------------------------------------------------------------------------

Int32s_t        mbFileNamePartsGet
(
    Char_p          name_p,
    qHead_p         name_q
)
{
    Int32u_t        componentCount = 0;
    Char_p          temp_p;
    Int32u_t        j;
    mbStrList_p     entry_p;
    Int32u_t        nonSlashCount;

     // Sanity check
    if( name_p == NIL ) goto exit;

    mbMalloc( temp_p, MAX_PATH * 5 );

    // Get length of string
    if( strlen( name_p ) == 0 ) goto exit;

    nonSlashCount = 0;
    for( j = 0 ; ; )
    {
        if( *name_p == MB_CHAR_BACKSLASH || *name_p == MB_CHAR_SLASH || *name_p == 0 )
        {
            if( nonSlashCount > 0 )
            {
                *( temp_p + j ) = 0;
                mbStrListAdd( name_q, temp_p );
                j = 0;
                nonSlashCount = 0;
            }
            if( *name_p == MB_CHAR_BACKSLASH || *name_p == MB_CHAR_SLASH )
            {
                *( temp_p + j ) = *name_p;
                j++;
            }
            if( *name_p == 0 ) break;
        }
        else
        {
            *( temp_p + j ) = *name_p;
            j++;
            nonSlashCount++;
        }
        name_p++;
    }
    componentCount = name_q->size;
exit:
    if( temp_p ) mbFree( temp_p );
    return componentCount;
}

//---------------------------------------------------------------------------
// Function:    mbFileNamePartsDisplay
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t mbFileNamePartsDisplay
(
    qHead_p         name_q
)
{
    Int32s_t        returnCode = MB_RET_ERR;
    mbStrList_p     entry_p;

    if( name_q == NIL ) goto exit;

    qWalk( entry_p, name_q, mbStrList_p )
    {
        mbLog( "Name component '%s'\n", entry_p->str_p );
    }
    returnCode = MB_RET_OK;
exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:    mbFileNamePartsFree
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t mbFileNamePartsFree
(
    qHead_p         name_q
)
{
    return mbStrListFree( name_q );
}

//---------------------------------------------------------------------------
// Function:    mbFileNamePartsFreeElement
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t mbFileNamePartsFreeElement
(
    mbStrList_p     entry_p
)
{
    return mbStrListItemFree( entry_p );
}

//---------------------------------------------------------------------------
// Function: mbFileCopy
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t            mbFileCopy
(
    Char_p          srcFileName_p,
    Char_p          destFileName_p
)
{
    return mbFileCopyCall( srcFileName_p, destFileName_p, NIL, NIL, NIL, NIL, NIL );
}

//---------------------------------------------------------------------------
// Function: mbFileCopyCall
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t            mbFileCopyCall
(
    Char_p          srcFileName_p,
    Char_p          destFileName_p,
    Int32u_p        crc_p,
    Int32u_p        size_p,
    Void_p          this_p,
    Int32s_t        (*cancelFunc)( Void_p this_p ),
    Int32s_t        (*bytesFunc)( Void_p this_p, Int32u_t bytes_p )
)
{
    Int32s_t    returnCode = MB_RET_ERR;
    FILE       *srcFp = NIL;
    FILE       *destFp = NIL;
    Int8u_p     buf_p;
    size_t      bytes;
    size_t      bytesWritten;
    Boolean_t   result;
    Int32u_t    crc = 0;
    Int32u_t    size = 0;

    mbMalloc( buf_p, 4096 );

    if( srcFileName_p == NIL ) goto exit;
    if( destFileName_p == NIL ) goto exit;

    if( FileExists( destFileName_p ) )
    {
        if( mbDlgYesNo( "The file %s already exists.  Overwrite?", destFileName_p ) == MB_BUTTON_NO )
        {
            returnCode = MB_RET_CANCEL;
            goto exit;
        }
        unlink( destFileName_p );
    }

    if( ( srcFp = fopen( srcFileName_p, "rb" ) ) == NIL ) goto exit;
    if( ( destFp = fopen( destFileName_p, "wb" ) ) == NIL ) goto exit;

    for( ; ; )
    {
        // Check the cancel callback
        if( cancelFunc )
        {
            result = (*cancelFunc)( this_p );
            if( result == TRUE )
            {
                if( mbDlgYesNo( "Cancel file copy?" ) == MB_BUTTON_YES )
                {
                    returnCode = MB_RET_CANCEL;
                    goto exit;
                }
            }
        }

        bytes = fread( (Void_p)buf_p, 1, 4096, srcFp );
        size += bytes;

        // 20021229: Compute CRC during file copy if requested
        if( crc_p ) crc = mbCrcUpdate( crc, buf_p, bytes );

        if( bytes )
        {
            bytesWritten = fwrite( (Void_p)buf_p, 1, bytes, destFp );

            // Sanity check
            if( bytesWritten != bytes )
            {
                mbDlgDebug(( "Error, bytesWritten (%d) != bytesRead (%d)", bytesWritten, bytes ));
            }

            // Call the callback with the number of bytes written
            if( bytesFunc ) (*bytesFunc)( this_p, (Int32u_t)bytesWritten );
        }

        if( bytes < 4096 ) break;
    }

    if( crc_p )
    {
        bytes = size;
        while( bytes > 0 )
        {
            crc = (crc << 8) ^ mbCrcTable[((crc >> 24) ^ bytes) & 0xFF];
            bytes >>= 8;
        }
        crc = ~crc & 0xFFFFFFFF;
    }

    returnCode = MB_RET_OK;

exit:

    if( srcFp ) fclose( srcFp );
    if( destFp ) fclose( destFp );
    mbFree( buf_p );

    if( crc_p ) *crc_p = crc;
    if( size_p ) *size_p = size;

    return returnCode;
}

//---------------------------------------------------------------------------
// Function: mbFileDirCheckCreate
//---------------------------------------------------------------------------
// This function checks for the existance of the specified directory
// (which may or may not have a filename at the end).  If the directory
// does not exist it may be created.  A creation prompt may be requested
//---------------------------------------------------------------------------

Int32s_t            mbFileDirCheckCreate
(
    Char_p          name_p,
    qHead_p         dirCache_q,
    Boolean_t       fileFlag,
    Boolean_t       createFlag,
    Boolean_t       promptFlag
)
{
    Int32s_t        returnCode = MB_RET_ERR;
    qHead_t         partQueue;
    qHead_p         part_q;
    mbNamePart_p    part_p;
    Char_p          buf_p = NIL;

    mbMalloc( buf_p, 1024 );

    part_q = qInitialize( &partQueue );

    if( name_p == NIL ) goto exit;

    if( mbFileNamePartsGet( name_p, part_q ) == 0 ) goto exit;

    if( fileFlag && part_q->size == 1 )
    {
        mbDlgDebug(( "'%s' is in current directory", name_p ));
        returnCode = MB_RET_OK;
        goto exit;
    }

    // At this point we have some kind of path with at least two elements
    if( fileFlag )
    {
        // This is a file indicator... punt last part
        part_p = (mbNamePart_p)qRemoveLast( part_q );
        mbFileNamePartsFreeElement( part_p );
    }

    // Build complete path
    mbFileNamePartsAssemble( part_q, part_q->size, buf_p );

    if( mbStrListCheckCache( dirCache_q, buf_p, TRUE ) == MB_RET_OK )
    {
        // Got a cache hit... assume dir exists
        returnCode = MB_RET_OK;
        goto exit;
    }

    if( DirectoryExists( buf_p ) )
    {
        if( mbStrListCheckCache( dirCache_q, buf_p, TRUE ) != MB_RET_OK )
        {
            // Add directory to cache if its not there already
            mbStrListAdd( dirCache_q, buf_p );
        }
        returnCode = MB_RET_OK;
        goto exit;
    }

    // At this point, it is known that the directory does not exist
    if( createFlag == FALSE ) goto exit;

    if( promptFlag )
    {
        if( mbDlgYesNo( "The required directory\n\n%s\n\ndoes not exist. Create it now?", buf_p ) == MB_BUTTON_NO ) goto exit;
    }

    if( ForceDirectories( buf_p ) == TRUE )
    {
        mbStrListAdd( dirCache_q, buf_p );
        returnCode = MB_RET_OK;
        goto exit;
    }
    else
    {
        mbDlgError( "Failed to create required directory\n\n%s", buf_p );
    }

exit:
    if( buf_p ) mbFree( buf_p );
    mbFileNamePartsFree( part_q );
    return returnCode;
}

//---------------------------------------------------------------------------
// Function: mbFileNamePartsAssemble
//---------------------------------------------------------------------------
// This function assembles the requested number of name components into
// a string.  The result is placed into the supplied buffer (no length
// checking is done).
//---------------------------------------------------------------------------

Int32s_t            mbFileNamePartsAssemble
(
    qHead_p         part_q,
    Int32u_t        partCount,
    Char_p          result_p
)
{
    Int32s_t        returnCode = MB_RET_ERR;
    mbNamePart_p    part_p;
    Int32u_t        count;

    if( part_q == NIL ) goto exit;
    if( partCount == 0 ) goto exit;
    if( partCount > part_q->size ) goto exit;
    if( result_p == NIL ) goto exit;

    *result_p = 0;
    count = 0;

    qWalk( part_p, part_q, mbNamePart_p )
    {
        if( count < partCount )
        {
            strcat( result_p, part_p->str_p );
        }
    }
    returnCode = MB_RET_OK;
exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// Procedure:   mbDriveOpen
//---------------------------------------------------------------------------
// Description:
//
// This function attempts to open (eject) or close a drive (e.g., CD )
//-----------------------------------------------------------------------------

Int32s_t mbDriveOpen( Boolean_t openFlag, Char_p drive_p )
{
	MCI_OPEN_PARMS      op;
	MCI_STATUS_PARMS    st;
	Int32u_t            flags;
	Char_t              driveName[4];
    Int32s_t            returnCode = MB_RET_ERR;

    if( drive_p == NIL ) goto exit;

    // Attempt to extract the drive letter (first letter encountered)
    for( ; ; drive_p++ )
    {
        if( *drive_p == 0 ) goto exit;
        if( *drive_p >= 'a' && *drive_p <= 'z' ) break;
        if( *drive_p >= 'A' && *drive_p <= 'Z' ) break;
    }

    driveName[0] = *drive_p;
    driveName[1] = ':';
    driveName[2] = 0;

	::ZeroMemory( &op, sizeof(MCI_OPEN_PARMS) );
	op.lpstrDeviceType = (LPCSTR)MCI_DEVTYPE_CD_AUDIO;
    op.lpstrElementName = driveName;

	flags = MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT | MCI_OPEN_SHAREABLE;

	if( mciSendCommand( 0, MCI_OPEN, flags, (Int32u_t)&op ) ) goto exit;

	if( openFlag )
    {
        if( mciSendCommand( op.wDeviceID, MCI_SET, MCI_SET_DOOR_OPEN, 0 ) ) goto exit;
    }
    else
    {
        if( mciSendCommand( op.wDeviceID, MCI_SET, MCI_SET_DOOR_CLOSED, 0 ) ) goto exit;
    }
    mciSendCommand( op.wDeviceID, MCI_CLOSE, MCI_WAIT, 0 );

    returnCode = MB_RET_OK;
exit:
    return returnCode;
}


//---------------------------------------------------------------------------
// Procedure:   mbDriveLabel
//---------------------------------------------------------------------------
// Description:
//
// This function attempts to determine the drives label
//-----------------------------------------------------------------------------

Char_p            mbDriveLabel
(
    Char_p          name_p,
    Char_p          out_p
)
{
    Char_t          drive;
    Char_t          name[4];
    if( name_p == NIL || out_p == NIL ) goto exit;

     drive = *name_p;
    if( drive >= 'A' && drive <= 'Z' )
    {
        drive -= 'A';
    }
    else if( drive <= 'a' && drive >= 'z' )
    {
        goto exit;
    }

    name[0] = drive;
    name[1] = MB_CHAR_COLON;
    name[2] = MB_CHAR_BACKSLASH;
    name[3] = 0;

    if( GetVolumeInformation
        (
            name,
            out_p, 64,
            NIL, NIL, NIL, NIL,0
        ) == 0 )
    {
        *out_p = 0;
    }

exit:

    return out_p;
}

//---------------------------------------------------------------------------
// Procedure:   mbDriveFreeSpace
//---------------------------------------------------------------------------
// Description:
//
// This function attempts to determine the drive based on the passed in
// name, then determines the free space on that drive
//-----------------------------------------------------------------------------

Int32s_t            mbDriveFreeSpace
(
    Char_p          name_p,
    Int64u_p        freeSpace_p
)
{
    Int64u_t        totalSpace = 0;
    Int8u_t         drive;
    Boolean_t       found = FALSE;
    struct dfree    freespace;
    Int32s_t        returnCode = MB_RET_ERR;
    Int32u_t        mode;

    if( name_p == NIL ) goto exit;

    drive = *name_p;
    if( drive >= 'A' && drive <= 'Z' )
    {
        drive -= 'A';
        found = TRUE;
    }

    if( !found )
    {
        if( drive >= 'a' && drive <= 'z' )
        {
            drive -= 'a';
            found = TRUE;
        }
    }

    if( !found ) goto exit;
    if( *(++name_p) != MB_CHAR_COLON ) goto exit;

    drive++;

    // Suppress error dialog
    mode = SetErrorMode( SEM_FAILCRITICALERRORS	);

    // Ok, this looks like drive indicator - get space
    getdfree( drive, &freespace );

    // Restore error dialog mode
    SetErrorMode( mode );

    if( freespace.df_sclus == -1 ) goto exit;

    totalSpace =   (Int64u_t)freespace.df_avail
                 * (Int64u_t)freespace.df_bsec
                 * (Int64u_t)freespace.df_sclus;

    returnCode = MB_RET_OK;

exit:

    // 20031221: Automatically subtract 10 MB from the free space.  This should
    // ensure that echos will fit on direct-CD formated CDs.

    if( totalSpace > 10000000L ) totalSpace -= 10000000L;

    if( freeSpace_p ) *freeSpace_p = totalSpace;
    return returnCode;
}

//---------------------------------------------------------------------------
// Function: mbFileByteSum
//---------------------------------------------------------------------------
// Compute the sum of all the bytes in a file
//---------------------------------------------------------------------------

Int32u_t    mbFileByteSum
(
    Char_p  fileName_p
)
{
    FILE       *fp;
    Int8u_p     buf_p;
    size_t      bytes;
    Int32u_t    sum = 0;
    Int32u_t    i;

    mbMalloc( buf_p, 5000 );

    if( ( fp = fopen( fileName_p, "rb" ) ) == NIL ) goto exit;

    for( ; ; )
    {
        bytes = fread( (Void_p)buf_p, 1, 4096, fp );
        for( i = 0 ; i < bytes ; i++ )
        {
            sum += *( buf_p + i );
        }
        if( bytes < 4096 ) break;
    }
 exit:
    if( fp ) fclose( fp );
    mbFree( buf_p );

    return sum;
}

//---------------------------------------------------------------------------
// Function: mbProcessIdGet
//---------------------------------------------------------------------------
// This function attempts to get the process id of the named process
//---------------------------------------------------------------------------

Int32u_t    mbProcessIdGet( Char_p nameIn_p )
{
    Int32u_t    process[1024], needed, processes;
    Int32u_t    i;
    Int32u_t    pid = 0;
    HANDLE      process_h;
    HMODULE     module_h;
    Int32u_t    junk;
    Char_p      processName_p;
    Int32u_t    winVersion;

#if 0
    mbMalloc( processName_p, MAX_PATH );

    winVersion = mbWinVersion( );
    if( winVersion != MB_WINDOWS_2000XP ) goto exit;
    if( nameIn_p == NIL ) goto exit;

    if ( !EnumProcesses( &process[0], sizeof(process), &needed ) )
    {
        mbDlgError( "EnumProcesses failed" );
        goto exit;
    }

    // Calculate how many process identifiers were returned.
    processes = needed / sizeof(Int32u_t);

    // Loop thru the process list
    for ( i = 0 ; i < processes ; i++ )
    {
        // Get process handle
        process_h = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process[i] );

        // Failed to get handle, move onto next process in list
        if( process_h == NULL ) continue;

        if( !EnumProcessModules( process_h, &module_h, sizeof(module_h), &junk ) )
        {
            CloseHandle( process_h );
            continue;
        }

        GetModuleBaseName( process_h, module_h, processName_p, MAX_PATH );

        if( mbStrPosIgnoreCase( processName_p, nameIn_p ) >= 0 )
        {
            pid = process[i];
            CloseHandle( process_h );
            break;
        }

#if 0
                //BOOL GetProcessTimes(
                //    HANDLE hProcess,
                //    LPFILETIME lpCreationTime,
                //    LPFILETIME lpExitTime,
                //    LPFILETIME lpKernelTime,
                //    LPFILETIME lpUserTime
                //);

                FILETIME    creationTime;
                FILETIME    exitTime;
                FILETIME    kernelTime;
                FILETIME    userTime;

                for( int j = 0 ; j < 10 ; j++ )
                {
                    Sleep( 100 );
                if( GetProcessTimes( hProcess, &creationTime, &exitTime, &kernelTime, &userTime ) )
                {
                    Int64u_t    ktime, uTime;
                    mbLog( "KLow %lu KHigh %lu ULow %lu UHigh %lu\n",
                        kernelTime.dwLowDateTime, kernelTime.dwHighDateTime,
                        userTime.dwLowDateTime, userTime.dwHighDateTime  );
                }
                else
                {
                    mbLog( "GetProcessTimes failed\n" );
                }
                }

            }
        }
#endif
        CloseHandle( process_h );
    }

 exit:

    mbFree( processName_p );
 #endif
    return pid;
 }

//---------------------------------------------------------------------------
// Function: mbProcessIdWaitIdle
//---------------------------------------------------------------------------
// This function will delay until the specified process ID has been idle
// for the specified persiod of time.  This is an attempt to prevent access
// to the CD drive until directcd is ready.
//---------------------------------------------------------------------------

Int32s_t    mbProcessIdWaitIdle
(
    Int32u_t    pid,
    Int32u_t    idleTimeMsec,
    Int32u_t    timeoutMsec
)
{
    HANDLE      process_h;
    Int32u_t    winVersion;
    Int32s_t    returnCode = MB_RET_ERR;
    FILETIME    creationTime;
    FILETIME    exitTime;
    FILETIME    kernelTime;
    FILETIME    userTime;
    Int32u_t    startTime;

    Int32u_t    kTimeLOld = 0;
    Int32u_t    kTimeHOld = 0;
    Int32u_t    uTimeLOld = 0;
    Int32u_t    uTimeHOld = 0;

#if 0
    winVersion = mbWinVersion( );
    if( winVersion != MB_WINDOWS_2000XP ) goto exit;

    startTime = mbMsec( );
    // Get process handle
    process_h = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid );

    // Failed to get handle, move onto next process in list
    if( process_h == NULL ) goto exit;

    for( int j = 0 ; j < 10 ; j++ )
    {
        if( !GetProcessTimes( process_h, &creationTime, &exitTime, &kernelTime, &userTime ) ) break;

        if(    kernelTime.dwLowDateTime  == kTimeLOld
            && kernelTime.dwHighDateTime == kTimeHOld
            && userTime.dwLowDateTime    == uTimeLOld
            && userTime.dwHighDateTime   == uTimeHOld )
        {
            returnCode = MB_RET_OK;
            break;
        }
        else
        {
            kTimeLOld = kernelTime.dwLowDateTime;
            kTimeHOld = kernelTime.dwHighDateTime;
            uTimeLOld = userTime.dwLowDateTime;
            uTimeHOld = userTime.dwHighDateTime;
        }

        if( ( mbMsec() - startTime ) > timeoutMsec ) break;

        Sleep( idleTimeMsec );
   }

   CloseHandle( process_h );

exit:
#endif
    return returnCode;

}

