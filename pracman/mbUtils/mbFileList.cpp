//---------------------------------------------------------------------------
// File:    mbFileListUtils.cpp
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
#include <dir.h>
#include <vcl.h>
#pragma hdrstop

#include "mbTypes.h"
#include "mbLock.h"
#include "mbMalloc.h"
#include "mbDebug.h"
#include "mbDlg.h"
#include "mbLog.h"
#include "mbStrUtils.h"
#include "mbFileUtils.h"

#define MB_INIT_GLOBALS
#include "mbFileList.h"
#undef  MB_INIT_GLOBALS

//---------------------------------------------------------------------------
// Function:    mbFileListSearch
//---------------------------------------------------------------------------
// This function searches the file list for the given string
//---------------------------------------------------------------------------

Int32s_t    mbFileListSearch
(
    qHead_p     file_q,
    Char_p      search_p
)
{
    Int32s_t            returnCode = MB_RET_ERR;
    mbFileListStruct_p  file_p;

    if( file_q == NIL || search_p == NIL ) goto exit;

    qWalk( file_p, file_q, mbFileListStruct_p )
    {
        if( strcmp( file_p->fullName_p, search_p ) == 0 )
        {
            returnCode = MB_RET_OK;
            break;
        }
        if( file_p->attrib & FA_DIREC )
        {
            if( file_p->sub_q )
            {
                returnCode = mbFileListSearch( file_p->sub_q, search_p );
                if( returnCode == MB_RET_OK ) break;
            }
        }
     }

exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:    mbFileListGet
//---------------------------------------------------------------------------
// This function (which may call itself recursively) returns the total size
// of all the files in the list.
//
// 20021101: This function cannot recurse without a filter of *.*  Fix later.
//---------------------------------------------------------------------------

Int64u_t                mbFileListGet
(
    Char_p              dir_p,
    Char_p              filter_p,
    qHead_p             q_p,
    Boolean_t           recurse
)
{
    static int          depthCount = 0;
    Int32s_t            pathLen;
    Char_p              path_p = NIL;
    Char_p              search_p = NIL;
    struct  ffblk       ffblk;
    Int32s_t            done;
    mbFileListStruct_p  file_p;
    Int32s_t            searchAttrib = 0;
    Int64u_t            totalSize = 0;

    // Sanity check
    if( recurse && strcmp( filter_p, "*.*" ) != 0 )
    {
        mbDlgDebug(( "Error: cannot recurse with filter_p != '*.*'\n" ));
        return totalSize;
    }

    // Sanity check
    if( depthCount > 25 )
    {
        mbDlgDebug(( "ERROR: depth count > 25; not executing\n" ));
        return totalSize;
    }
    depthCount++;

    // Copy the target directory
    mbMalloc( path_p, 1024 );
    mbMalloc( search_p, 1024 );

    if( dir_p )
    {
        strcpy( path_p, dir_p );
        mbStrRemoveSlashTrailing( path_p );
    }
    else
    {
        *path_p = 0;
    }

    // Get the length of the target directory
    pathLen = strlen( path_p );
    if( pathLen > 0 )
    {
        strcat(  path_p, "\\" );
        pathLen++;
    }

    if( pathLen )
    {
        sprintf( search_p, "%s%s", path_p, filter_p );
    }
    else
    {
        getcwd( search_p, 256 );
        sprintf( path_p, "%s\\", search_p );
        sprintf( search_p, "%s", filter_p );
    }

    if( recurse )
    {
        // If recursing or including directories, set appropriate search attributes
        searchAttrib = FA_DIREC;
    }

    // Now get list of all the files
    done = findfirst( search_p, &ffblk, searchAttrib );
    while( !done )
    {
        if( strcmp( ffblk.ff_name, "." ) != 0 && strcmp( ffblk.ff_name, ".." ) != 0 )
        {
            mbCalloc( file_p, sizeof( mbFileListStruct_t ) );
            mbMalloc( file_p->name_p, strlen( ffblk.ff_name ) + 1 );
            mbStrClean( ffblk.ff_name, file_p->name_p, FALSE );

            mbMalloc( file_p->fullName_p, strlen( ffblk.ff_name ) + strlen( path_p ) + 1 );
            sprintf( file_p->fullName_p, "%s%s", path_p, ffblk.ff_name );

            // Initialize queue of sub files
            file_p->sub_q = qInitialize( &file_p->subQueue );

            // Record attributes
            file_p->attrib = ffblk.ff_attrib;
            file_p->date = ffblk.ff_fdate;
            file_p->time = ffblk.ff_ftime;

            if( ( ffblk.ff_attrib & FA_DIREC ) && recurse )
            {
                // Include entry before recursing
                qInsertLast( q_p, file_p );

                // Get files in the sub directory
                file_p->size64 = mbFileListGet( file_p->fullName_p, filter_p, file_p->sub_q, recurse );
            }
            else
            {
                // Must be a file, insert into list
                qInsertLast( q_p, file_p );
                file_p->size64 = (Int64u_t)ffblk.ff_fsize;
            }
            // Increment total size, be this a file or directory
            totalSize += file_p->size64;
        }
        done = findnext( &ffblk );
    }
    findclose( &ffblk );

    mbFree( path_p );
    mbFree( search_p );

    depthCount--;

    return totalSize;
}

//---------------------------------------------------------------------------
// Function:    mbFileListGetNew
//---------------------------------------------------------------------------
// 20050108: Based on mbFileListGet, but I don't want to accidentally break
// pracman, which relies on this funtion.  Add ability to get directories
// only.
//---------------------------------------------------------------------------

Int64u_t                mbFileListGetNew
(
    Char_p              dir_p,
    Char_p              filter_p,
    qHead_p             q_p,
    Boolean_t           recurse,
    Boolean_t           dirOnlyFlag,
    Boolean_t           firstFlag,
    Int32u_t            depthMax
)
{
    static Int32u_t     depthCount = 0;
    Int32s_t            pathLen;
    Char_p              path_p = NIL;
    Char_p              search_p = NIL;
    struct  ffblk       ffblk;
    Int32s_t            done;
    mbFileListStruct_p  file_p;
    mbFileListStruct_p  first_p;
    Int32s_t            searchAttrib = 0;
    Int64u_t            totalSize = 0;
    Char_t              buf[256];

    // Sanity check
    if( recurse && strcmp( filter_p, "*.*" ) != 0 )
    {
        mbDlgDebug(( "Error: cannot recurse with filter_p != '*.*'\n" ));
        return totalSize;
    }

    if( depthCount > depthMax )
    {
        return totalSize;
    }

    depthCount++;

    // Copy the target directory
    mbMalloc( path_p, 1024 );
    mbMalloc( search_p, 1024 );

    if( dir_p )
    {
        strcpy( path_p, dir_p );
        mbStrRemoveSlashTrailing( path_p );
    }
    else
    {
        *path_p = 0;
    }

    // Get the length of the target directory
    pathLen = strlen( path_p );
    if( pathLen > 0 )
    {
        strcat(  path_p, "\\" );
        pathLen++;
    }

    if( pathLen )
    {
        sprintf( search_p, "%s%s", path_p, filter_p );
    }
    else
    {
        getcwd( search_p, 256 );
        sprintf( path_p, "%s\\", search_p );
        sprintf( search_p, "%s", filter_p );
    }

    first_p = NIL;
    if( depthCount == 1 && firstFlag == TRUE )
    {
        Char_t  temp[MAX_PATH];
        strcpy( temp, path_p );
        mbStrRemoveSlashTrailing( temp );
        mbCalloc( file_p, sizeof( mbFileListStruct_t ) );

        mbMallocStr( file_p->name_p, temp );
        mbMallocStr( file_p->fullName_p, temp );
        mbMallocStr( file_p->path_p, temp );

        // Record attributes
        file_p->attrib = ffblk.ff_attrib;
        file_p->date = ffblk.ff_fdate;
        file_p->time = ffblk.ff_ftime;

        mbMallocStr( file_p->timeStr_p, mbFFTimeToStr( file_p->time, buf ) );
        mbMallocStr( file_p->dateStr_p, mbFFTimeToStr( file_p->date, buf ) );

        file_p->sub_q = qInitialize( &file_p->subQueue );
        file_p->attrib |= FA_DIREC;
        qInsertLast( q_p, file_p );

        first_p = file_p;
    }

    if( recurse )
    {
        // If recursing or including directories, set appropriate search attributes
        searchAttrib = FA_DIREC;
    }

    // Now get list of all the files
    done = findfirst( search_p, &ffblk, searchAttrib );
    while( !done )
    {
        if( strcmp( ffblk.ff_name, "." ) != 0 && strcmp( ffblk.ff_name, ".." ) != 0 )
        {
            mbCalloc( file_p, sizeof( mbFileListStruct_t ) );
            mbMalloc( file_p->name_p, strlen( ffblk.ff_name ) + 1 );
            mbStrClean( ffblk.ff_name, file_p->name_p, FALSE );

            mbMalloc( file_p->fullName_p, strlen( ffblk.ff_name ) + strlen( path_p ) + 1 );
            sprintf( file_p->fullName_p, "%s%s", path_p, ffblk.ff_name );

            mbMallocStr( file_p->path_p, file_p->fullName_p );

            // This snippet trims the file name from the path
            {
                Ints_t  l1, l2;
                l1 = strlen( file_p->name_p );
                l2 = strlen( file_p->path_p );
                if( l2 > ( l1 + 1 ) )
                {
                    *(file_p->path_p + l2 - l1 - 1) = (Char_t)0;
                }
            }

            // Initialize queue of sub files
            file_p->sub_q = qInitialize( &file_p->subQueue );

            // Record attributes
            file_p->attrib = ffblk.ff_attrib;
            file_p->date = ffblk.ff_fdate;
            file_p->time = ffblk.ff_ftime;

            mbMallocStr( file_p->timeStr_p, mbFFTimeToStr( file_p->time, buf ) );
            mbMallocStr( file_p->dateStr_p, mbFFDateToStr( file_p->date, buf ) );

            if( ( ffblk.ff_attrib & FA_DIREC ) && recurse )
            {
                // Include entry before recursing
                qInsertLast( q_p, file_p );

                // Get files in the sub directory
                file_p->size64 = mbFileListGetNew( file_p->fullName_p,
                    filter_p, file_p->sub_q, recurse, dirOnlyFlag, FALSE, depthMax );
            }
            else
            {
                // Must be a file, insert into list unless doing dirs only
                if( dirOnlyFlag == FALSE )
                {
                    qInsertLast( q_p, file_p );
                    file_p->size64 = (Int64u_t)ffblk.ff_fsize;
                }
                else
                {
                    mbFileListFreeElement( file_p );
                    file_p = NIL;
                }
            }

            // Increment total size, be this a file or directory
            if( file_p )
            {
                totalSize += file_p->size64;
                mbMallocStr( file_p->sizeStr_p,  mbStrInt64u( file_p->size64, buf ) );
            }
        }
        done = findnext( &ffblk );
    }
    findclose( &ffblk );

    mbFree( path_p );
    mbFree( search_p );

    if( first_p )
    {
        first_p->size64 = totalSize;
        mbMallocStr( first_p->sizeStr_p,  mbStrInt64u( first_p->size64, buf ) );
    }

    depthCount--;

    return totalSize;
}

//---------------------------------------------------------------------------
// Function: mbFileListFlatten
//---------------------------------------------------------------------------
// Flatten the file list into a single queue
//---------------------------------------------------------------------------

Int32s_t    mbFileListFlatten( qHead_p file_q )
{
    qHead_t             tempQueue;
    qHead_p             temp_q;
    mbFileListStruct_p  file_p;

    if( file_q == NIL ) goto exit;

    temp_q = qInitialize( &tempQueue );

    mbFileListFlattenRecurse( file_q, temp_q );

    while( !qEmpty( temp_q ) )
    {
        file_p = (mbFileListStruct_p)qRemoveFirst( temp_q );
        qInsertLast( file_q, file_p );
    }

exit:
    return TRUE;
}

//---------------------------------------------------------------------------
// Function: mbFileListFlattenRecurse
//---------------------------------------------------------------------------
// Flatten the file list into a single queue
//---------------------------------------------------------------------------

Int32s_t    mbFileListFlattenRecurse( qHead_p file_q, qHead_p temp_q )
{
    mbFileListStruct_p  file_p;

    while( !qEmpty( file_q ) )
    {
        file_p = (mbFileListStruct_p)qRemoveFirst( file_q );
        qInsertLast( temp_q, file_p );
        mbFileListFlattenRecurse( file_p->sub_q, temp_q );
    }
    return TRUE;
}

//---------------------------------------------------------------------------
// Function: mbFileListLog
//---------------------------------------------------------------------------
// This function loops through all the echo files, and outputs to the log
// file the contents of the list.
//---------------------------------------------------------------------------

Int32s_t    mbFileListLog( qHead_p file_q )
{
    mbFileListStruct_p file_p;

    qWalk( file_p, file_q, mbFileListStruct_p )
    {
        if( file_p->attrib & FA_DIREC )
        {
            mbLog( "mbFileList: directory: '%s' Size: %Ld\n", file_p->fullName_p, file_p->size64 );
            if( file_p->sub_q ) mbFileListLog( file_p->sub_q );
        }
        else
        {
            mbLog( "mbFileList:      file: '%s' Size: %Ld\n", file_p->fullName_p, file_p->size64 );
        }
    }
    return TRUE;
}

//---------------------------------------------------------------------------
// Function: mbFileListPurgeDuplicates
//---------------------------------------------------------------------------
// Loop through list, purge dupicates (i.e., same name).
//---------------------------------------------------------------------------

Int32s_t    mbFileListPurgeDuplicates( qHead_p file_q )
{
    Int32s_t                entriesFreed = 0;
    Int32u_t                size;
    mbFileListStruct_p      file1_p;
    mbFileListStruct_p      file2_p;
    qHead_t                 tempQueue;
    qHead_p                 temp_q;

    if( file_q == NIL ) goto exit;

    temp_q = qInitialize( &tempQueue );

    while( !qEmpty( file_q ) )
    {
        file1_p = (mbFileListStruct_p)qRemoveFirst( file_q );
        qInsertLast( temp_q, file1_p );

        for( Int32u_t i = 0, size = file_q->size ; i < size ; i++ )
        {
            file2_p = (mbFileListStruct_p)qRemoveFirst( file_q );

            if( strcmp( file1_p->fullName_p, file2_p->fullName_p ) == 0 )
            {
                mbLog( "Delete duplicate entry '%s'\n", file2_p->fullName_p );
                mbFileListFreeElement( file2_p );
                entriesFreed++;
            }
            else
            {
                qInsertLast( file_q, file2_p );
            }
        }
    }

    // Done, now move files back into orig queue
    while( !qEmpty( temp_q ) )
    {
        file1_p = (mbFileListStruct_p)qRemoveFirst( temp_q );
        qInsertLast( file_q, file1_p );
    }

exit:
    return entriesFreed;
}

//---------------------------------------------------------------------------
// Function: mbFileListFreeElement
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
Int32s_t        mbFileListFreeElement
(
    mbFileListStruct_p file_p
)
{
    if( file_p == NIL ) return TRUE;

    // Recursively free subdirectories and files
    mbFileListFree( file_p->sub_q );
    mbFree( file_p->name_p );
    mbFree( file_p->fullName_p );
    mbFree( file_p->target_p );
    mbFree( file_p->path_p );
    mbFree( file_p->crcStr_p );
    mbFree( file_p->sizeStr_p );
    mbFree( file_p->dateStr_p );
    mbFree( file_p->timeStr_p );

    mbFree( file_p );

    return TRUE;
}

//---------------------------------------------------------------------------
// Function: mbFileListFree
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t        mbFileListFree
(
    qHead_p     q_p
)
{
    mbFileListStruct_p file_p;

    if( q_p == NULL ) return TRUE;

    for( ; ; )
    {
        if( qEmpty( q_p ) ) break;
        file_p = (mbFileListStruct_p)qRemoveFirst( q_p );

        mbFileListFreeElement( file_p );
    }
    return TRUE;
}

//---------------------------------------------------------------------------
// Function: mbFileListTargetMake
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t    mbFileListTargetMake
(
    Char_p              sourceIn_p,
    Char_p              targetIn_p,
    qHead_p             file_q
)
{
    mbFileListStruct_p  file_p;
    Int32s_t            returnCode = MB_RET_ERR;
    Char_p              source_p = NIL;
    Char_p              target_p = NIL;

    if( sourceIn_p == NIL || targetIn_p == NIL || file_q == NIL ) goto  exit;

    mbMallocStr( source_p, sourceIn_p );
    mbMallocStr( target_p, targetIn_p );

    mbStrRemoveSlashTrailing( target_p );
    mbStrRemoveSlashTrailing( source_p );

    qWalk( file_p, file_q, mbFileListStruct_p )
    {
        mbFileListTargetMakeElement( source_p, target_p, file_p );

        //mbLog( "mbFileList: %s: '%s' Size: %Ld\n", ( file_p->attrib & FA_DIREC ) ? "directory" : "file", file_p->fullName_p, file_p->size64 );
        //if( file_p->target_p ) mbLog( "mbFileList:    target: '%s' Size: %Ld\n", file_p->target_p, file_p->size64 );

        if( file_p->attrib & FA_DIREC )
        {
            // Recurse into sub directories
            if( file_p->sub_q ) mbFileListTargetMake( source_p, target_p, file_p->sub_q );
        }
    }

    returnCode = MB_RET_OK;
exit:
    if( target_p ) mbFree( target_p );
    if( source_p ) mbFree( source_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Function: mbFileListTargetMakeElement
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t mbFileListTargetMakeElement
(
    Char_p              source_p,
    Char_p              target_p,
    mbFileListStruct_p  file_p
)
{
    Int32s_t            returnCode = FALSE;

    mbStrRemoveSlashTrailing( target_p );
    mbStrRemoveSlashTrailing( source_p );

    // First lets clear out any string already in the target
    if( file_p->target_p )
    {
        mbFree( file_p->target_p );
        file_p->target_p = NIL;
    }

    if( mbStrUpdateRoot( file_p->fullName_p, source_p, target_p, &file_p->target_p ) ==  NIL )
    {
        mbDlgError( "Could not update file '%s'\n", file_p->fullName_p );
        goto exit;

    }
    returnCode = TRUE;
exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// Function: mbFileListCopy
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t        mbFileListCopy
(
    qHead_p     file_q,
    Boolean_t   thermometerFlag,
    Void_p      this_p,
    Int32s_t    (*cancelFunc)( Void_p this_p ),
    Int32s_t    (*fileFunc)( Void_p this_p, Char_p name_p ),
    Int32s_t    (*bytesFunc)( Void_p this_p, Int32u_t bytes_p )
)
{
    Int32s_t        returnCode;
    TThermometer    *thermometer_p = NIL;
    qHead_t         dirCacheQueue;
    qHead_p         dirCache_q;


    // For now, lets just set min/max to the number of files.
    // I would like to update this to bytes later on
    if( thermometerFlag ) thermometer_p = new TThermometer( "File Copy..." , 0, 10, TRUE );

    // List for existing directories... to speed up detection
    // of existing directories

    dirCache_q = qInitialize( &dirCacheQueue );

    returnCode = mbFileListCopyRecurse( file_q,
                                        dirCache_q,
                                        thermometer_p,
                                        this_p,
                                        cancelFunc,
                                        fileFunc,
                                        bytesFunc );

    if( thermometer_p ) delete thermometer_p;

    mbStrListLog( dirCache_q );
    mbStrListFree( dirCache_q );

    return returnCode;
}
//---------------------------------------------------------------------------
// Function: mbFileListCopyRecurse
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t            mbFileListCopyRecurse
(
    qHead_p         file_q,
    qHead_p         dirCache_q,
    TThermometer   *thermometer_p,
    Void_p          this_p,
    Int32s_t        (*cancelFunc)( Void_p this_p ),
    Int32s_t        (*fileFunc)( Void_p this_p, Char_p name_p ),
    Int32s_t        (*bytesFunc)( Void_p this_p, Int32u_t bytes_p )
)
{
    mbFileListStruct_p  file_p;
    Char_p              buf_p = NIL;
    Int32s_t            returnCode = MB_RET_ERROR;

    if( thermometer_p ) mbMalloc( buf_p, 1024 );

    qWalk( file_p, file_q, mbFileListStruct_p )
    {
        // Check for thermometer
        if( thermometer_p )
        {
            sprintf( buf_p, "Copying file '%s'", file_p->name_p );
            thermometer_p->SetCaption( buf_p );

            if( thermometer_p->Increment( ) == FALSE )
            {
                if( mbDlgYesNo( "Cancel file copy?" ) == MB_BUTTON_YES )
                {
                    returnCode = MB_RET_CANCEL;
                    goto exit;
                }
            }
        }

        if( cancelFunc )
        {
            // Call the cancel detect callback function
            Boolean_t result = (*cancelFunc)( this_p );
            if( result == TRUE )
            {
                if( mbDlgYesNo( "Cancel file copy?" ) == MB_BUTTON_YES )
                {
                    returnCode = MB_RET_CANCEL;
                    goto exit;
                }
            }
        }

        // Indicate to the calling code the file that is being copied
        if( fileFunc ) (*fileFunc)( this_p, file_p->name_p );

        if( file_p->attrib & FA_DIREC )
        {
            mbLog( "mbFileListCopy: descend directory: '%s' Size: %Ld\n", file_p->fullName_p, file_p->size64 );
            if( file_p->sub_q )
            {
                returnCode = mbFileListCopyRecurse( file_p->sub_q,
                                                    dirCache_q,
                                                    thermometer_p,
                                                    this_p,
                                                    cancelFunc,
                                                    fileFunc,
                                                    bytesFunc );

                if( returnCode != MB_RET_OK ) goto exit;
            }
        }
        else
        {
            returnCode = mbFileListCopyFile( file_p,
                                             dirCache_q,
                                             this_p,
                                             cancelFunc,
                                             bytesFunc );

            if( returnCode != MB_RET_OK ) goto exit;
        }
    }

    returnCode = MB_RET_OK;

exit:
    if( buf_p ) mbFree( buf_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Function: mbFileListCopyFile
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t    mbFileListCopyFile
(
    mbFileListStruct_p  file_p,
    qHead_p             dirCache_q,
    Void_p              this_p,
    Int32s_t            (*cancelFunc)( Void_p this_p ),
    Int32s_t            (*bytesFunc)( Void_p this_p, Int32u_t bytes_p )
)
{
    Int32s_t    returnCode = MB_RET_ERR;

    // Need to ensure that the target directory exists
    if( file_p == NIL ) goto exit;

    if( file_p->fullName_p == NIL ) goto exit;
    if( file_p->target_p == NIL ) goto exit;

    returnCode = mbFileDirCheckCreate( file_p->target_p,
                                       dirCache_q,          // Check cache for directories
                                       ( file_p->attrib & FA_DIREC ) ? FALSE : TRUE,
                                       TRUE,                // Create dir if required
                                       FALSE );             // Do not prompt

    if( returnCode != MB_RET_OK) goto exit;

    // At this point, it looks like the required directory exists
    // Proceed with the file copy.

    returnCode = mbFileCopyCall( file_p->fullName_p,
                                 file_p->target_p,
                                 NIL,
                                 NIL,
                                 this_p,
                                 cancelFunc,
                                 bytesFunc );
exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// Function: mbFileListDelete
//---------------------------------------------------------------------------
// This function will delete all the files in the list.
// It calls itself recursively
//---------------------------------------------------------------------------

Int32s_t        mbFileListDelete
(
    qHead_p     file_q,
    Boolean_t   deleteDirs
)
{
    mbFileListStruct_p  file_p;
    Int32s_t            returnCode = MB_RET_ERR;

    if( file_q == NIL )
    {
        returnCode = MB_RET_OK;
        goto exit;
    }

    qWalk( file_p, file_q, mbFileListStruct_p )
    {
        if( file_p->attrib & FA_DIREC )
        {
            // This is a diretory.  Must recursively delete contents
            // before directory can be deleted
            returnCode = mbFileListDelete( file_p->sub_q, deleteDirs );
            if( returnCode != MB_RET_OK )
            {
                goto exit;
            }

            if( deleteDirs )
            {
                // mbLog( "Delete dir '%s'\n", file_p->fullName_p );
                if( RemoveDirectory( file_p->fullName_p ) == 0 )
                {
                    mbLog( "Error deleting dir '%s'\n", file_p->fullName_p );
                    goto exit;
                }
            }
        }
        else
        {
            // This is a file... delete
            // mbLog( "Delete file '%s'\n", file_p->fullName_p );
            if( unlink( file_p->fullName_p ) != 0 )
            {
                mbLog( "Error deleting file '%s'\n", file_p->fullName_p );
                goto exit;
            }
        }
    }
    returnCode = MB_RET_OK;
exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// Function: mbFFTimeToStr
//---------------------------------------------------------------------------
// ff_ftime:
//
// Bits 0 to 4	    Seconds divided by 2 (e.g. 10 here means 20 sec)
// Bits 5 to 10	    Minutes
// Bits 11 to 15	Hours
//---------------------------------------------------------------------------

Char_p mbFFTimeToStr( Int16u_t time, Char_p str_p )
{
    Int16u_t    seconds;
    Int16u_t    minutes;
    Int16u_t    hours;

    if( str_p == NIL ) return "NIL";

    // 15 14 13 12    |    11 10 9  8    |   7  6  5  4   |   3  2  1  0
    // 1  1  1  1     |    1  1  1  1    |   1  1  1  1   |   1  1  1  1

    seconds = time & 0x001F;
    seconds *=2;

    minutes = time & 0x07E0;
    minutes >>= 5;

    hours = time & 0xF800;
    hours >>= 11;

    if( hours > 11 && hours != 24 )
    {
        if( hours > 12 ) hours -= 12;
        sprintf( str_p, "%d:%02d PM", hours, minutes );
    }
    else
    {
        if( hours == 24 ) hours = 12;
        sprintf( str_p, "%d:%02d AM", hours, minutes );
    }

    return str_p;
}

//---------------------------------------------------------------------------
// Function: mbFFDateToStr
//---------------------------------------------------------------------------
// ff_fdate:
//
// Bits 0-4     Day
// Bits 5-8     Month
// Bits 9-15    Years since 1980 (for example 9 here means 1989)
//---------------------------------------------------------------------------

Char_p mbFFDateToStr( Int16u_t date, Char_p str_p )
{
    Int16u_t    day;
    Int16u_t    month;
    Int16u_t    year;
    Char_p      monthStr_p[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                               "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

    if( str_p == NIL ) return "NIL";

    // 15 14 13 12    |    11 10 9  8    |   7  6  5  4   |   3  2  1  0
    // 1  1  1  1     |    1  1  1  1    |   1  1  1  1   |   1  1  1  1

    day = date & 0x001F;

    month = date & 0x01E0;
    month >>= 5;

    if( month > 0 ) month--;
    if( month > 11 ) month = 11;

    year = date & 0xFE00;
    year >>= 9;

    sprintf( str_p, "%d %s %d", day, monthStr_p[month], year + 1980 );
    return str_p;
}

