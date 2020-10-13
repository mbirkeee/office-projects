/******************************************************************************
 * Copyright (c) 2007, Michael A. Bree
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 * $Header: /home/cvs/mbUtilsLinux/src/mbFileUtils.c,v 1.7 2008/01/02 01:04:37 mikeb Exp $
 *-----------------------------------------------------------------------------
 * $Log: mbFileUtils.c,v $
 * Revision 1.7  2008/01/02 01:04:37  mikeb
 * Add string formatting of Int64u_t and Int32u_t
 *
 * Revision 1.6  2008/01/01 17:32:43  mikeb
 * Add return code to mbFileListGet()
 *
 * Revision 1.5  2007/12/17 01:00:08  mikeb
 * Update file utilities
 *
 * Revision 1.4  2007/11/12 05:05:53  mikeb
 * Updates to mbFileListDisplay.  Display name_p if fileName_p is NULL
 *
 * Revision 1.3  2007/11/10 17:39:41  mikeb
 * Updates
 *
 * Revision 1.2  2007/07/16 04:16:21  mikeb
 * Add a few utility functions
 *
 * Revision 1.1.1.1  2007/07/10 05:06:02  mikeb
 * Initial Import
 *
 *-----------------------------------------------------------------------------
 */

/* System include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

/* Project include files */
#include "mbUtils.h"

/* Globals */
static Int32u_t     g_FileListLogValue = 0;

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
    return mbStrListFreeElement( entry_p );
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
    Int32u_t        nonSlashCount;

    // Sanity check
    if( name_p == NIL ) goto exit;

    temp_p = (Char_p)mbMalloc( MAX_PATH * 5 );

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
// Function:    mbFileNamePathStrip
//---------------------------------------------------------------------------
// Strips the path from a file name
//---------------------------------------------------------------------------

Char_p          mbFileNamePathStrip
(
    Char_p          name_p,
    Char_p          outIn_p
)
{
    qHead_t         queueHead;
    qHead_p         queue_p;
    mbNamePart_p    entry_p;
    Char_p          out_p = NIL;

    queue_p = qInitialize( &queueHead );
    if( outIn_p )
    {
        out_p = outIn_p;
        *out_p = 0;
    }
  
    if( name_p == NIL ) goto exit;

    if( mbFileNamePartsGet( name_p, queue_p ) == 0 ) goto exit;

    if( queue_p->size == 0 ) goto exit;

    entry_p = (mbNamePart_p)qRemoveLast( queue_p );
    qInsertLast( queue_p, entry_p );
    
    if( out_p )
    {
        strcpy( out_p, entry_p->str_p );
    }
    else
    {
        out_p = mbMallocStr( entry_p->str_p );
    }            

    mbStrRemoveSlashLeading( out_p );

exit:
    mbFileNamePartsFree( queue_p );
    return out_p;
}

/******************************************************************************
 * Function: mbFileListLogValueSet( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *
 *-----------------------------------------------------------------------------
 */
void  mbFileListLogValueSet( Int32u_t value )
{
   g_FileListLogValue = value;
   return;
}

/******************************************************************************
 * Function: mbFileExists( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *
 *-----------------------------------------------------------------------------
 */
Int32s_t mbFileExists( Char_p name_p )
{
    Int32s_t    returnCode = FALSE;

    if( name_p )
    {
        if( access( name_p, F_OK ) == 0 )
        {
            returnCode = TRUE;
        }
    }
    return returnCode;
}

/******************************************************************************
 * Function: mbFileNameCompare( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns the starting position of substring i2 within i1.
 *-----------------------------------------------------------------------------
 */
Int32s_t        mbFileNameCompare
(
    Char_p      fileName1_p,
    Char_p      fileName2_p
)
{
    if( fileName1_p == NIL || fileName2_p == NIL ) return -1;

    if( strlen( fileName1_p ) != strlen( fileName2_p ) ) return -1;

    return mbStrPos( fileName1_p, fileName2_p );
}

/******************************************************************************
 * Function: mbFileFree( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */
qHead_p mbFileListSort( qHead_p in_q, Boolean_t dateFlag )
{
    qHead_t     sortedQueue;
    qHead_p     sorted_q;
    MbFile_p    fileIn_p;
    MbFile_p    fileOut_p;
    Boolean_t   added;
    Int32u_t    sortTotal;

    if( in_q == NIL ) goto exit;

    sorted_q = qInitialize( &sortedQueue );

    sortTotal = in_q->size;

    while( !qEmpty( in_q ) )
    {
        fileIn_p = (MbFile_p)qRemoveFirst( in_q );

        added = FALSE;
        qWalk( fileOut_p, sorted_q, MbFile_p )
        {
            if( fileOut_p->pathName_p == NIL || fileIn_p->pathName_p == NIL ) break;
            
            if( dateFlag == TRUE )
            {
                if( fileOut_p->date < fileIn_p->date )
                {
                    qInsertBefore( sorted_q, fileOut_p, fileIn_p );
                    added = TRUE;
                    break;
                }
                if( fileOut_p->date > fileIn_p->date ) continue;
            }
            
            if( strcmp( fileIn_p->pathName_p, fileOut_p->pathName_p ) < 0 )
            {
                qInsertBefore( sorted_q, fileOut_p, fileIn_p );
                added = TRUE;
                break;
            }
        }

        if( added == FALSE )
        {
            qInsertLast( sorted_q, fileIn_p );
        }

        if( g_FileListLogValue )
        {
            if( ( sorted_q->size % g_FileListLogValue ) == 0 )
            {
                fprintf( stdout, "Sorted %lu of %lu files...\n", sorted_q->size, sortTotal );
                fflush( stdout );
            }
        }
    }

    while( !qEmpty( sorted_q ) )
    {
        fileIn_p = (MbFile_p)qRemoveFirst( sorted_q );
        qInsertLast( in_q, fileIn_p );
    }

exit:
    return in_q;
}

/******************************************************************************
 * Function: mbFileFree( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */
MbFile_p mbFileFree( MbFile_p file_p )
{
    if( file_p == NIL ) goto exit;

    mbFree( file_p->name_p );
    mbFree( file_p->path_p );
    mbFree( file_p->pathName_p );
    mbFree( file_p );

exit:
    return NIL;
}

/******************************************************************************
 * Function: freeFileEntry( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns the starting position of substring i2 within i1.
 *-----------------------------------------------------------------------------
 */
Int32s_t mbFileListDisplay( qHead_p q_p )
{
    Int32s_t    result = FALSE;
    MbFile_p    file_p;

    if( q_p == NULL ) goto exit;

    qWalk( file_p, q_p, MbFile_p )
    {
        if(  file_p->pathName_p )
        {
            fprintf( stdout, "File: '%s'\n", file_p->pathName_p );
        }
        else if( file_p->name_p )
        {
            fprintf( stdout, "File: '%s'\n", file_p->name_p );
        }
        else
        {
            fprintf( stdout, "File: 'NO NAME SPECIFIED'\n" );
        }
    }
    fflush( stdout );
    result = TRUE;
exit:
    return result;
}

/******************************************************************************
 * Function: mbFileNew( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns the starting position of substring i2 within i1.
 *-----------------------------------------------------------------------------
 */

MbFile_p mbFileMalloc
(
    Char_p  name_p,
    Char_p  path_p
)
{
    MbFile_p    file_p = NIL;
    Ints_t      pathLen;

    if( ( file_p = (MbFile_p)mbCalloc( sizeof( MbFile_t ) ) ) == NIL ) goto exit;

    if( name_p )
    {
        if( ( file_p->name_p = (Char_p)mbMalloc( strlen( name_p ) + 1 ) ) == NIL )
        {
            file_p = mbFileFree( file_p );
            goto exit;
        }
        strcpy( file_p->name_p, name_p );
    }

    if( path_p )
    {
        for( ; ; )
        {
            if( ( pathLen = strlen( path_p ) ) == 0 ) break;

            if( *( path_p + pathLen - 1 ) == '/' )
            {
                *( path_p + pathLen - 1 ) = 0;
            }
            else
            {
                break;
            }
        }

        if( ( file_p->path_p = (Char_p)mbMalloc( strlen( path_p ) + 1 ) ) == NIL )
        {
            file_p = mbFileFree( file_p );
            goto exit;
        }
        strcpy( file_p->path_p, path_p );
    }

    if( name_p && path_p )
    {
        if( ( file_p->pathName_p = (Char_p)mbMalloc( strlen( path_p ) + strlen( name_p ) + 2 ) ) == NIL )
        {
            file_p = mbFileFree( file_p );
            goto exit;
        }
        sprintf( file_p->pathName_p, "%s/%s", path_p, name_p );
    }
exit:
    return file_p;

}

/******************************************************************************
 * Function: mbFileListFree( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *
 *-----------------------------------------------------------------------------
 */
void mbFileListFree( qHead_p q_p )
{
    MbFile_p    file_p;

    if( q_p )
    {
        for( ; ; )
        {
            if( qEmpty( q_p ) ) break;

            file_p = (MbFile_p)qRemoveFirst( q_p );

            mbFileFree( file_p );
        }
    }
    return;
}

/******************************************************************************
 * Function: mbFileListGet( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * Get a list of fies in a directory
 *-----------------------------------------------------------------------------
 */
Int32s_t mbFileListGet
(
    Char_p      dir_p,
    qHead_p     q_p,
    Boolean_t   recurseFlag,
    Boolean_t   symFlag,
    Boolean_t   dirFlag
)
{
    struct dirent **namelist_pp = 0;
    int             n;
    struct stat     buf;
    Char_p          filename_p = NIL;
    MbFile_p        file_p = NIL;
    Boolean_t       ignore;
    int             result;
    Int32s_t        returnCode = FALSE;

#if 0
    if( q_p->size > 2000 )
    {
        printf( "quit scan early\n" );
        return 0;
    }
#endif
    filename_p = mbMalloc( 512 );

    //printf( "examining directory '%s'\n", dir_p );

    n = scandir( dir_p, &namelist_pp, 0, 0 );
    if( n < 0 )
    {
        fprintf( stderr, "scandir() error: %d on directory '%s'\n", n, dir_p );
        goto exit;
    }
    else
    {
        returnCode = TRUE;
        
        /* Step through all the entries */
        while( n-- )
        {
            /* debugging printf( "namelist[n]: 0x%08lX\n", (Int32u_t)&namelist[n] ); */

            sprintf( filename_p, "%s/%s", dir_p, namelist_pp[n]->d_name );

            if( ( result = lstat( filename_p, &buf ) ) != 0 )
            {
                /* Error */
                printf( "lstat() error: %d on file '%s'\n", result, filename_p );
            }
            else
            {
                ignore = FALSE;

#if 0
                qWalk( file_p, q_p, MbFile_p )
                {
                    if( file_p->inode == buf.st_ino )
                    {
                        //printf( "already have file '%s'... in a loop?\n", namelist_pp[n]->d_name );
                        ignore = TRUE;
                        break;
                    }
                }
#endif
                if( ignore ) continue;

                ignore = TRUE;

                if( S_ISLNK( buf.st_mode ) && symFlag == FALSE ) continue;

                if( S_ISREG( buf.st_mode ) )
                {
                    /* This is a regular file */
                    file_p = mbFileMalloc( namelist_pp[n]->d_name, dir_p );
                    file_p->inode = buf.st_ino;
                    file_p->size = buf.st_size;
                    file_p->type = MB_FILE_TYPE_FILE;
                    qInsertLast( q_p, file_p );
                    //printf( "added file: '%s'\n", filename_p );

                    if( g_FileListLogValue != 0 )
                    {
                        if( ( q_p->size % g_FileListLogValue ) == 0 )
                        {
                            fprintf( stdout, "Scanned %lu files...\n", q_p->size );
                            fflush( stdout );
                        }
                    }
                    // printf( "inode: 0x%08lX size: %d\n", buf.st_ino, sizeof( ino_t ) );
                    ignore = FALSE;
                }
                else
                {
                    /* Check to see if this is a directory */
                    if( S_ISDIR( buf.st_mode ) || S_ISLNK( buf.st_mode ) )
                    {
                        if( strcmp( namelist_pp[n]->d_name, ".." ) !=  0 )
                        {
                            if( strcmp( namelist_pp[n]->d_name, "." ) !=  0 )
                            {
                                /* This is a directory */
                                if( dirFlag )
                                {
                                    file_p = mbFileMalloc( namelist_pp[n]->d_name, dir_p );
                                    file_p->inode = buf.st_ino;
                                    file_p->type = MB_FILE_TYPE_DIR;
                                    qInsertLast( q_p, file_p );
                                    if( g_FileListLogValue != 0 )
                                    {
                                        if( ( q_p->size % g_FileListLogValue ) == 0 )
                                        {
                                            fprintf( stdout, "Scanned %lu files...\n", q_p->size );
                                            fflush( stdout );
                                        }
                                    }
                                }

                                if( recurseFlag == TRUE )
                                {
                                    mbFileListGet( filename_p, q_p, recurseFlag, symFlag, dirFlag );
                                }                                    
                                ignore = FALSE;
                            }
                        }
                    }
                    else
                    {
                        printf("not processing %s\n", filename_p );
                    }
                }
                //if( ignore )  printf( "ignoring file '%s'\n", filename_p );
            }

            if( namelist_pp[n] )
            {
                free( namelist_pp[n] );
            }
            else
            {
                printf( "Got NIL namelist[%d] on %s", n, dir_p );
            }
        }
    }

exit:

    if( namelist_pp ) free( namelist_pp );
    if( filename_p ) mbFree( filename_p );
    return returnCode;
}

