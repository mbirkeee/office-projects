/******************************************************************************
 * File: restore.c
 *-----------------------------------------------------------------------------
 * Copyright (c) 2007, Michael A. Bree
 *-----------------------------------------------------------------------------
 * Date: June 23, 2007
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 * $Header: /home/cvs/backup_new/src/restore.c,v 1.23 2008/05/17 15:01:32 mikeb Exp $
 *-----------------------------------------------------------------------------
 * $Log: restore.c,v $
 * Revision 1.23  2008/05/17 15:01:32  mikeb
 * Add ability to skip files/directories
 *
 * Revision 1.22  2008/04/07 03:37:24  mikeb
 * Fix typo
 *
 * Revision 1.21  2008/04/06 22:37:06  mikeb
 * Sort lists of added and deleted files
 *
 * Revision 1.20  2008/04/06 22:03:45  mikeb
 * Fixes to history feature
 *
 * Revision 1.19  2008/04/06 14:42:21  mikeb
 * Fix bug detecting removed files
 *
 * Revision 1.18  2008/04/06 14:15:44  mikeb
 * Add history output
 *
 * Revision 1.17  2008/01/02 01:03:56  mikeb
 * Initial, simple benchmark facility
 *
 * Revision 1.16  2007/12/17 00:58:32  mikeb
 * Add date to restore output.
 *
 * Revision 1.15  2007/11/12 15:38:14  mikeb
 * Update help
 *
 * Revision 1.14  2007/11/12 15:34:25  mikeb
 * Fix log, output message when database check done
 *
 * Revision 1.13  2007/11/12 15:05:28  mikeb
 * Mark orphan files with '0', and not backed up files with 'N'
 *
 * Revision 1.12  2007/11/12 15:01:49  mikeb
 * Fix close file bug
 *
 * Revision 1.11  2007/11/12 05:05:02  mikeb
 * Work on file delete
 *
 * Revision 1.10  2007/11/10 23:29:07  mikeb
 * More work on backup check
 *
 * Revision 1.9  2007/11/10 17:50:15  mikeb
 * Add fflush( stdout )
 *
 * Revision 1.8  2007/11/10 17:40:05  mikeb
 * Add check database feature
 *
 * Revision 1.7  2007/10/17 02:51:42  mikeb
 * Move delete of files to end of program, in case multiple files in restore reference deleted file.
 *
 * Revision 1.6  2007/08/19 23:00:32  mikeb
 * Add -h to allowable command line options
 *
 * Revision 1.5  2007/08/19 22:49:28  mikeb
 * add -x to help screen
 *
 * Revision 1.4  2007/07/23 13:42:47  mikeb
 * ESC the '`' character
 *
 * Revision 1.3  2007/07/22 20:29:15  mikeb
 * Update help
 *
 * Revision 1.2  2007/07/22 14:27:29  mikeb
 * Update to delete backed up files
 *
 * Revision 1.1  2007/07/22 01:04:56  mikeb
 * Initial import
 *
 *-----------------------------------------------------------------------------
 */

/* System include files */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>

/* Utility include files */
#include "mbUtils.h"

/* Project include files */
#include "backup.h"
#include "restore.h"

/* External references */
extern char *optarg;

/* Global variables */
static Int32u_t     g_Counter = 0;
static Boolean_t    gListFlag;
static Boolean_t    gRemoveFlag;
static Boolean_t    gBFilesFlag;

/******************************************************************************
 * Procedure: mbFileNameEsc( )
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

Char_p  mbFileNameEsc( Char_p in_p, Char_p out_p )
{
    Ints_t  i, j, len;

    len = strlen( in_p );
    for( i = 0, j = 0 ; i < len ; i++ )
    {
        if(  *(in_p + i) == MB_CHAR_QUOTE_SINGLE
         ||  *(in_p + i) == MB_CHAR_SPACE
         ||  *(in_p + i) == '&'
         ||  *(in_p + i) == '$'
         ||  *(in_p + i) == '~'
         ||  *(in_p + i) == '!'
         ||  *(in_p + i) == '`'
         ||  *(in_p + i) == '('
         ||  *(in_p + i) == ')' )
        {
            *(out_p + j) = MB_CHAR_BACKSLASH;
            j++;
        }
        *(out_p + j) = *(in_p + i);
        j++;
    }
    *(out_p + j ) = 0;

    return out_p;
}

/******************************************************************************
 * Procedure: benchmark( )
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

Int32s_t benchmark( Char_p dir_p, Boolean_t writeFlag )
{
    qHead_t         fileQueue;
    qHead_p         file_q;
    Char_p          target_p;
    Char_p          target2_p;
    Char_p          file_p;
    int             i, j, k, m;
    Int32u_t        startTime;
    Int32u_t        endTime;
    Int8u_p         buf_p;
    Int64u_t        totalBytes = 0;
    Int32u_t        bytesRead;
    FILE           *fp;
    Int32u_t        bps;
    MbFile_p        entry_p;

    buf_p = mbMalloc( 1024 );
    target_p = mbMalloc( RS_MAX_PATH );
    target2_p = mbMalloc( RS_MAX_PATH );
    file_p = mbMalloc( RS_MAX_PATH );

    file_q = qInitialize( &fileQueue );

    if( dir_p == NIL ) goto exit;

    /* Must test that directory exits... do this by getting dir listing */
    mbFileListLogValueSet( 1000 );
    if( mbFileListGet( dir_p, file_q, FALSE, FALSE, TRUE ) == FALSE )
    {
        printf( "Unable to access directory: %s\n", dir_p );
        goto exit;
    }

    if( writeFlag == TRUE )
    {
        printf( "proceed with write test; target directory: %s\n", dir_p );
        if( file_q->size != 0 )
        {
            printf( "target directory is not empty\n" );
            goto exit;
        }

        startTime = time( 0 );

        /* Seed random number generator */
        srand( startTime );

        for( i = 0 ; i < 256 ; i++ )
        {
            sprintf( target_p, "%s/%02x", dir_p, i );
            // printf( "create directory: %s\n", target_p );

            if( mkdir( target_p, 01777 ) != 0 )
            {
                printf( "Failed to create directory: %s\n", target_p );
                goto exit;
            }
            for( j = 0 ; j < 32 ; j++ )
            {
                sprintf( target2_p, "%s/%02x", target_p, j );
                // printf( "create directory: %s\n", target2_p );

                if( mkdir( target2_p, 01777 ) != 0 )
                {
                    printf( "Failed to create directory: %s\n", target2_p );
                    goto exit;
                }
                sprintf( file_p, "%s/file", target2_p );
                if( ( fp = fopen( file_p, "wb" ) ) == NIL )
                {
                    printf( "failed to open file: %s\n", file_p );
                }

                for( m = 0 ; m < 1024 ; m++ )
                {
                    for( k = 0 ; k < 512 ; k++ )
                    {
                        *( buf_p + k ) = (Int8u)rand();
                    }
                    fwrite( buf_p, 1024, 1, fp );

                    totalBytes += (Int64u_t)1024;
                }
                fclose( fp );
            }
        }

    }
    else
    {
        printf( "proceed with read test; target directory: %s\n", dir_p );
        if( file_q->size == 0 )
        {
            printf( "target directory is empty\n" );
            goto exit;
        }

         mbFileListFree( file_q );

         startTime = time( 0 );

         mbFileListGet( dir_p, file_q, TRUE, FALSE, FALSE );

         printf( "Reading %lu files...\n", file_q->size );

         qWalk( entry_p, file_q,  MbFile_p )
         {
            // printf( "Open file: %s\n", entry_p->pathName_p );
            if( ( fp = fopen( entry_p->pathName_p, "rb" ) ) == NIL )
            {
                printf( "failed to open file: %s\n", entry_p->pathName_p );
                goto exit;
            }

            for( ; ; )
            {
                if( ( bytesRead = fread( buf_p, 1, 1024, fp ) ) == 0 ) break;

                totalBytes += (Int64u_t) bytesRead;
            }
            fclose( fp );
         }
    }

    endTime = time( 0 );

    endTime = endTime - startTime;

    bps = (Int32u_t)( totalBytes / (Int64u_t)endTime );

    printf( "total bytes:    %s\n", mbStrInt64u( totalBytes, (Char_p)buf_p ) );
    printf( "total seconds:  %ld\n", endTime );
    printf( "bytes/sec:      %s\n", mbStrInt32u( bps, (Char_p)buf_p ) );

exit:

    mbFree( target_p );
    mbFree( target2_p );
    mbFree( file_p );
    mbFree( buf_p );

    mbFileListFree( file_q );
    return 0;

}


/******************************************************************************
 * Procedure: checkListAppend( )
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */
Int32s_t checkListAppend( Char_p fileName_p, qHead_p test_q, Int32u_t historyFlag, Int32u_t historyDate )
{
    Int32s_t            result = FALSE;
    Char_p              cmd_p;
    Char_p              target_p = NIL;
    Char_p              buf_p = NIL;
    Char_p              path_p = NIL;
    Char_t              md5str[128];
    FILE               *fp = NIL;
    struct timeval      curtime;
    MbFile_p            wantFile_p;
    MbFile_p            newFile_p;
    MbFile_p            file_p;
    Boolean_t           found;
    Char_t              fileName[ RS_MAX_PATH ];
    int                 i, j;
    Int32u_t            md5fast[4];
    Int8u_t             temp;
    Int32u_t            index;
    qHead_p             want_q;
    Int32u_t            fileDate;
    qHead_t             newQueue;
    qHead_p             new_q;
    Int32u_t            size;
    Char_t              buf[64];
    Int32u_t            prevDate;

    qHead_t             addQueue;
    qHead_p             add_q;

    qHead_t             delQueue;
    qHead_p             del_q;

    buf_p = mbMalloc( 4096 );
    cmd_p = mbMalloc( 3 * RS_MAX_PATH );
    target_p = mbMalloc( RS_MAX_PATH );

    gettimeofday( &curtime, NULL );
    fprintf( stdout, "Processing list file: %s\n", fileName_p );
    fflush( stdout );

    new_q = qInitialize( &newQueue );
    add_q = qInitialize( &addQueue );
    del_q = qInitialize( &delQueue );

    /* Extract date from filename */
    {
        Char_p  result_p;
        result_p = mbFileNamePathStrip( fileName_p, NIL );
        // fprintf( stdout, "stripped file name '%s'\n", result_p );
        fileDate = mbDateIntFromString( result_p );
        mbFree( result_p );
    }

    /* Make temporary file name */
    sprintf( target_p, "/tmp/restore-%lu-%lu-%lu", curtime.tv_sec, curtime.tv_usec, g_Counter++ );

    /* Format gunzip command */
    sprintf( cmd_p, "%s -c %s > %s", CMD_GUNZIP, fileName_p, target_p );

    if( mbSystem( cmd_p, NULL ) != 0 )
    {
        printf( "mbSystem( %s ) failed\n", cmd_p );
        goto exit;
    }

    /* Now read the temporary (unzipped) file */
    if( ( fp = fopen( target_p, "r" ) ) == NIL )
    {
        printf( "Failed to open file '%s'\n", target_p );
        goto exit;
    }

    while( fgets( buf_p, 4096, fp ) != NULL )
    {
        if( mbStrPos( buf_p, "P:" ) == 0 )
        {
            /* Remember the current path */
            mbFree( path_p );
            path_p = mbMallocStr( buf_p + 2 );
            mbStrCleanWhite( path_p, NIL );
        }
        else if( mbStrPos( buf_p, "F:" ) == 0 )
        {
            if( path_p == NIL )
            {
                printf( "Error: got file without path" );
                continue;
            }

            memcpy( md5str, buf_p + 2, 32 );
            md5str[32] = 0;

            temp = md5str[8];
            md5str[8] = 0;
            md5fast[0] = strtoul( (Char_p)&md5str[0], NULL , 16 );
            md5str[8] = temp;

            temp = md5str[16];
            md5str[16] = 0;
            md5fast[1] = strtoul( (Char_p)&md5str[8], NULL , 16 );
            md5str[16] = temp;

            temp = md5str[24];
            md5str[24] = 0;
            md5fast[2] = strtoul( (Char_p)&md5str[16], NULL , 16 );
            md5str[24] = temp;

            md5fast[3] = strtoul( (Char_p)&md5str[24], NULL , 16 );

            fileName[0] = 0;
            strcpy( fileName, buf_p + 35 );
            mbStrCleanWhite( fileName, NIL );

            /* Add file to new queue */

            wantFile_p = mbFileMalloc( fileName, path_p );
            strcpy( wantFile_p->md5str, md5str );

            wantFile_p->md5fast[0] = md5fast[0];
            wantFile_p->md5fast[1] = md5fast[1];
            wantFile_p->md5fast[2] = md5fast[2];
            wantFile_p->md5fast[3] = md5fast[3];

            wantFile_p->date = fileDate;
            qInsertLast( new_q, wantFile_p );
        }
    }

    if( historyDate == 0 ) historyDate = 20001212;
    prevDate = mbDateBackOneDay( historyDate );

    if( historyFlag && fileDate >= prevDate )
    {
        /* Check to see if known file has been deleted... but only after */
        /* specified date (this is a very, very time consuming operation */
        for( index = 0 ; index < 256 * 256 ; index++ )
        {
            want_q = test_q + index;
            size = want_q->size;
            for( j = 0 ; j < size ; j++ )
            {
                wantFile_p = (MbFile_p)qRemoveFirst( want_q );

                found = FALSE;
                qWalk( newFile_p, new_q, MbFile_p )
                {
                    for( i = 0 ; i < 4 ; i++ )
                    {
                        if( wantFile_p->md5fast[i] != newFile_p->md5fast[i] )
                        {
                            break;
                        }
                    }

                    if( i == 4 )
                    {
                        found = TRUE;
                        break;
                    }
                }

                if( found == FALSE )
                {
                     //printf( "dates: %lu %lu\n", fileDate, historyDate );
                    /* File must have been removed */
                    if( fileDate >= historyDate )
                    {
                        file_p = mbFileMalloc( wantFile_p->name_p, wantFile_p->path_p );
                        file_p->date = wantFile_p->date;
                        qInsertLast( del_q, file_p );
                    }
                    else
                    {
                       // printf( "skip initital dispaly %lu %lu\n", fileDate, historyDate );
                    }

                    mbFileFree( wantFile_p );
                }
                else
                {
                    qInsertLast( want_q, wantFile_p );
                }
            }
        }

        mbFileListSort( del_q, TRUE );

        while( !qEmpty( del_q ) )
        {
            file_p = (MbFile_p)qRemoveFirst( del_q );
            printf( "R: %s (file date: %s) %s\n",
               mbDateStringFromInt( fileDate, md5str ),
               mbDateStringFromInt( file_p->date, buf ),
               file_p->pathName_p );

            mbFileFree( file_p );
        }
    }

    /* Now loop through new files */
    while( !qEmpty( new_q ) )
    {
        newFile_p = (MbFile_p)qRemoveFirst( new_q );

        found = FALSE;

        /* Figure out which sub queue to look in */
        index = newFile_p->md5fast[0] & 0x0000FFFF;
        want_q = test_q + index;

        qWalk( wantFile_p, want_q, MbFile_p )
        {
            for( i = 0 ; i < 4 ; i++ )
            {
                if( wantFile_p->md5fast[i] != newFile_p->md5fast[i] )
                {
                    break;
                }
            }

            if( i == 4 )
            {
                found = TRUE;
                break;
            }
        }

        if( found == FALSE )
        {
            qInsertLast( want_q, newFile_p );

            if( historyFlag && newFile_p->date >= historyDate )
            {
                file_p = mbFileMalloc( newFile_p->name_p, newFile_p->path_p );
                file_p->date = newFile_p->date;
                qInsertLast( add_q, file_p );
            }
        }
        else
        {
            /* Already have this file... */
            mbFileFree( newFile_p );
        }
    }

    if( historyFlag )
    {
        mbFileListSort( add_q, TRUE );
        while( !qEmpty( add_q ) )
        {
            file_p = (MbFile_p)qRemoveFirst( add_q );
            printf( "A: %s %s\n", mbDateStringFromInt( file_p->date, buf ),
               file_p->pathName_p );

            mbFileFree( file_p );
        }
    }

exit:

    if( fp ) fclose( fp );

    if( target_p ) unlink( target_p );

    mbFree( buf_p );
    mbFree( cmd_p );
    mbFree( target_p );
    mbFree( path_p );

    mbFileListFree( add_q );
    mbFileListFree( del_q );

    return result;
}

/******************************************************************************
 * Procedure: check( )
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

Int32s_t check( Char_p backupDir_p, Int32u_t historyFlag, Int32u_t historyDate )
{
    Char_p          gotDir_p;
    Char_p          listDir_p;
    Char_p          tmp_p;
    Int32u_t        totalCompared = 0;
    Int32u_t        totalGot;
    Boolean_t       found;

    qHead_t         gotQueueHead;
    qHead_p         got_q;
    MbFile_p        gotFile_p;

    qHead_t         listQueueHead;
    qHead_p         list_q;
    MbFile_p        listFile_p;

    qHead_t         wantQueueHead;
    qHead_p         want_q;
    MbFile_p        wantFile_p;

    qHead_t         orphanQueueHead;
    qHead_p         orphan_q;

    qHead_p         test_q;
    qHead_p         junk_q;
    int             i;

    qHead_t         backQueue;
    qHead_p         back_q;

    back_q = qInitialize( &backQueue );
    orphan_q = qInitialize( &orphanQueueHead );

    got_q = qInitialize( &gotQueueHead );
    gotDir_p = mbMalloc( RS_MAX_PATH );

    list_q = qInitialize( &listQueueHead );
    listDir_p = mbMalloc( RS_MAX_PATH );

    tmp_p = mbMalloc( RS_MAX_PATH );

    want_q = qInitialize( &wantQueueHead );

    test_q = (qHead_p)mbMalloc( 256 * 256 * sizeof( qHead_t ) );

    for( i = 0 ; i < 256 * 256 ; i++ )
    {
        junk_q = &test_q[i];
        junk_q = qInitialize( junk_q );
    }

    sprintf( gotDir_p, "%s/files", backupDir_p );

    if( historyFlag == FALSE )
    {
        printf( "Scanning backed up files directory: %s\n", gotDir_p );
        mbFileListLogValueSet( 1000 );
        mbFileListGet( gotDir_p, got_q, TRUE, FALSE, FALSE );
    }

    // mbFileListDisplay( got_q );

    qWalk( gotFile_p, got_q, MbFile_p )
    {
        int len = strlen( gotFile_p->name_p );
        int i;

        /* Truncate .gz from file names */
        for( i = 0 ; i < len ; i++ )
        {
            if( *( gotFile_p->name_p + i ) == '.' )
            {
                *( gotFile_p->name_p + i ) = 0;
                break;
            }
        }
        // printf( "Got file '%s'\n", gotFile_p->name_p );
    }

    sprintf( listDir_p, "%s/done", backupDir_p );


    printf( "Scanning backup list directory: %s\n", listDir_p );
    mbFileListGet( listDir_p, list_q, TRUE, FALSE, FALSE );
    // mbFileListDisplay( list_q );

    printf( "Sorting backup list files...\n" );
    mbFileListSort( list_q, TRUE );


    qWalk( listFile_p, list_q, MbFile_p )
    {
        checkListAppend( listFile_p->pathName_p, test_q, historyFlag, historyDate );
/*
        if( want_q->size > 40000 )
        {
            printf( "stop examining files for testing purposes\n" );
            break;
        }
*/
    }

    /* Flatten queue */
    for( i = 0 ; i < 256 * 256 ; i++ )
    {
        junk_q = test_q + i;
        while( !qEmpty( junk_q ) )
        {
            wantFile_p = (MbFile_p)qRemoveFirst( junk_q );
            qInsertLast( want_q, wantFile_p );
        }
    }


    if( historyFlag == TRUE )
    {
        goto exit;
    }

    printf( "Comparing backed-up files to listed files...\n" );

    totalGot = got_q->size;

    while( !qEmpty( got_q ) )
    {
        totalCompared++;

        if( ( totalCompared % 1000 ) == 0 )
        {
            fprintf( stdout, "Compared %lu of %lu files...\n", totalCompared, totalGot );
            fflush( stdout );
        }

        gotFile_p = (MbFile_p)qRemoveFirst( got_q );

        found = FALSE;
        qWalk( wantFile_p, want_q, MbFile_p )
        {
            //printf( "Comparing '%s' tp '%s'\n", gotFile_p->name_p, wantFile_p->md5str );

            if( strcmp( gotFile_p->name_p, wantFile_p->md5str ) == 0 )
            {
                //printf( "found a match" );
                found = TRUE;
                break;
            }
        }

        if( found == TRUE )
        {
            mbFileFree( gotFile_p );
            qRemoveEntry( want_q, wantFile_p );
            qInsertLast( back_q, wantFile_p );
        }
        else
        {
            qInsertLast( orphan_q, gotFile_p );
        }
    }

    if( orphan_q->size == 0 )
    {
        printf( "No orphan backup files were detected\n" );
    }
    else
    {
        printf( "The following backed-up files are orphans (i.e., not listed):\n" );
        qWalk( wantFile_p, orphan_q, MbFile_p )
        {
            fprintf( stdout, "O:%s %s\n", wantFile_p->md5str, wantFile_p->pathName_p );

            if( gRemoveFlag )
            {
                if( unlink( mbFileNameEsc( wantFile_p->pathName_p, tmp_p ) ) == 0 )
                {
                    printf( "DELETED: '%s'\n", wantFile_p->pathName_p );
                }
                else
                {
                    printf( "FAILED: could not delete '%s'\n", wantFile_p->pathName_p );
                }
            }
        }
    }
    if( gListFlag )
    {
        mbFileListSort( back_q, TRUE );
        printf( "The following files are backed-up:\n" );
        qWalk( wantFile_p, back_q, MbFile_p )
        {
            fprintf( stdout, "B:%s %s %s\n", wantFile_p->md5str,
                mbDateStringFromInt( wantFile_p->date, tmp_p ), wantFile_p->pathName_p );
        }
    }

    mbFileListSort( want_q, TRUE );

    if( want_q->size == 0 )
    {
        printf( "No un-backed-up files were detected\n" );
    }
    else
    {
        printf( "The following files are not backed-up:\n" );

        qWalk( wantFile_p, want_q, MbFile_p )
        {
            fprintf( stdout, "N:%s %s %s\n", wantFile_p->md5str,
                mbDateStringFromInt( wantFile_p->date, tmp_p ), wantFile_p->pathName_p );
        }
    }

exit:

    mbFileListFree( orphan_q );
    mbFileListFree( got_q );
    mbFileListFree( list_q );
    mbFileListFree( want_q );
    mbFileListFree( back_q );

    mbFree( tmp_p );
    mbFree( gotDir_p );
    mbFree( listDir_p );
    mbFree( test_q );

    fprintf( stdout, "Done\n" );
    fflush( stdout );

    return TRUE;
}


/******************************************************************************
 * Procedure: fileListGet( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns a list of files in the specified date file
 *-----------------------------------------------------------------------------
 */
qHead_p fileListGet( Char_p file_p )
{
    Char_p          cmd_p;
    Char_p          target_p = NIL;
    Char_p          buf_p = NIL;
    Char_p          path_p = NIL;
    Char_t          md5str[128];
    FILE           *fp = NIL;
    struct timeval  curtime;
    MbFile_p        gotFile_p;
    Char_t          fileName[ RS_MAX_PATH ];
    Int32u_t        md5fast[4];
    Int8u_t         temp;
    qHead_t         listQueue;
    qHead_p         list_q = NIL;
    Int32u_t        date;

    buf_p = mbMalloc( 4096 );
    cmd_p = mbMalloc( 3 * RS_MAX_PATH );
    target_p = mbMalloc( RS_MAX_PATH );

    gettimeofday( &curtime, NULL );
    fprintf( stdout, "Processing list file: %s\n", file_p );
    fflush( stdout );

    /* Initialize the list to be returned */
    list_q = qInitialize( &listQueue );

    if( file_p == NIL ) goto exit;

    /* Extract date from filename */
    {
        Char_p  result_p;
        result_p = mbFileNamePathStrip( file_p, NIL );
        // fprintf( stdout, "stripped file name '%s'\n", result_p );
        date = mbDateIntFromString( result_p );
        mbFree( result_p );
    }

    /* Make temporary file name */
    sprintf( target_p, "/tmp/restore-%lu-%lu-%lu", curtime.tv_sec, curtime.tv_usec, g_Counter++ );

    /* Format gunzip command */
    sprintf( cmd_p, "%s -c %s > %s", CMD_GUNZIP, file_p, target_p );

    if( mbSystem( cmd_p, NULL ) != 0 )
    {
        printf( "mbSystem( %s ) failed\n", cmd_p );
        goto exit;
    }

    /* Now read the temporary (unzipped) file */
    if( ( fp = fopen( target_p, "r" ) ) == NIL )
    {
        printf( "Failed to open file '%s'\n", target_p );
        goto exit;
    }

    while( fgets( buf_p, 4096, fp ) != NULL )
    {
        if( mbStrPos( buf_p, "P:" ) == 0 )
        {
            mbFree( path_p );
            path_p = mbMallocStr( buf_p + 2 );
            mbStrCleanWhite( path_p, NIL );
        }
        else if( mbStrPos( buf_p, "F:" ) == 0 )
        {
            if( path_p == NIL )
            {
                printf( "Error: got file without path" );
                continue;
            }

            memcpy( md5str, buf_p + 2, 32 );
            md5str[32] = 0;

            temp = md5str[8];
            md5str[8] = 0;
            md5fast[0] = strtoul( (Char_p)&md5str[0], NULL , 16 );
            md5str[8] = temp;

            temp = md5str[16];
            md5str[16] = 0;
            md5fast[1] = strtoul( (Char_p)&md5str[8], NULL , 16 );
            md5str[16] = temp;

            temp = md5str[24];
            md5str[24] = 0;
            md5fast[2] = strtoul( (Char_p)&md5str[16], NULL , 16 );
            md5str[24] = temp;

            md5fast[3] = strtoul( (Char_p)&md5str[24], NULL , 16 );

            fileName[0] = 0;
            strcpy( fileName, buf_p + 35 );
            mbStrCleanWhite( fileName, NIL );

            gotFile_p = mbFileMalloc( fileName, path_p );

            strcpy( gotFile_p->md5str, md5str );

            gotFile_p->md5fast[0] = md5fast[0];
            gotFile_p->md5fast[1] = md5fast[1];
            gotFile_p->md5fast[2] = md5fast[2];
            gotFile_p->md5fast[3] = md5fast[3];

            gotFile_p->date = date;

            qInsertLast( list_q, gotFile_p );
        }
    }

exit:

    if( fp ) fclose( fp );

    if( target_p ) unlink( target_p );

    mbFree( buf_p );
    mbFree( cmd_p );
    mbFree( target_p );
    mbFree( path_p );

    return list_q;
}

/******************************************************************************
 * Procedure: check( )
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

Int32s_t filesDelete( Char_p backupDir_p, Char_p deleteFile_p )
{
    Int32s_t        result = FALSE;
    FILE           *fp = NIL;
    Char_p          buf_p = NIL;
    Char_t          md5str[64];
    Char_p          cmd_p = NIL;
    Char_p          target_p = NIL;
    Char_p          temp_p = NIL;
    struct timeval  curtime;
    qHead_t         listQueue;
    qHead_p         list_q;
    MbFile_p        listFile_p;
    qHead_t         delQueue;
    qHead_p         del_q;
    MbFile_p        delFile_p;
    Boolean_t       zipFlag = FALSE;
    Boolean_t       modified = FALSE;
    Boolean_t       skipLine = FALSE;
    FILE           *lp = NIL;
    Char_p          buf2_p = NIL;

    list_q = qInitialize( &listQueue );
    del_q = qInitialize( &delQueue );

    buf_p       = mbMalloc( 4096 );
    buf2_p      = mbMalloc( 4096 );
    cmd_p       = mbMalloc( 4 * RS_MAX_PATH );
    target_p    = mbMalloc( RS_MAX_PATH );
    temp_p      = mbMalloc( 4096 );

    if( backupDir_p == NIL ||  deleteFile_p == NIL ) goto exit;

    if( ( fp = fopen( deleteFile_p, "r" ) ) == NIL )
    {
        printf( "Failed to open '%s'\n", deleteFile_p );
        goto exit;
    }

    printf( "Uncompressing backup database...\n" );

    /* Backup all "done" files */
    gettimeofday( &curtime, NULL );

    /* Make backup directory name name */
    sprintf( target_p, "%s/done-backup-%lu-%lu-%lu", backupDir_p, curtime.tv_sec, curtime.tv_usec, g_Counter++ );
    sprintf( cmd_p, "%s %s", CMD_MKDIR, target_p );

    if( mbSystem( cmd_p, NULL ) != 0 )
    {
        printf( "mbSystem( %s ) failed\n", cmd_p );
        goto exit;
    }

    sprintf( cmd_p, "%s %s/done/*.gz %s/.", CMD_CP, backupDir_p, target_p );
    // printf( "command: '%s'\n", cmd_p );

    if( mbSystem( cmd_p, NULL ) != 0 )
    {
       printf( "mbSystem( %s ) failed\n", cmd_p );
       goto exit;
    }

    sprintf( cmd_p, "%s %s/done/*.gz", CMD_GUNZIP, backupDir_p );
    // printf( "command: '%s'\n", cmd_p );

    zipFlag = TRUE;
    if( mbSystem( cmd_p, NULL ) != 0 )
    {
        printf( "mbSystem( %s ) failed\n", cmd_p );
        goto exit;
    }

    // Get list of files
    sprintf( target_p, "%s/done", backupDir_p );
    printf( "Scanning backup list directory: '%s'\n", target_p );
    mbFileListGet( target_p, list_q, TRUE, FALSE, FALSE );
    mbFileListSort( list_q, FALSE );

    // This loop goes through the input file and finds all entried marked
    // 'N:' (not backed up) of 'B:' (backed up).  If the line also contains
    // what looks to be a valid md5sum, an entry for the file is created
    // in the delete list.
    while( fgets( buf_p, 4096, fp ) != NULL )
    {
        if(    mbStrPos( buf_p, "N:" ) == 0
            || mbStrPos( buf_p, "F:" ) == 0
            || mbStrPos( buf_p, "B:" ) == 0
          )
        {
            if( mbStrPos( buf_p, "F:" ) == 0 )
            {
                // 2008-10-30: Why the heck would I ever check for "F"???
                // I don't think we should ever expect to see anything 
                // but 'B:' (backed up) snd 'N:' (not backed up)
                fprintf( stdout, "Detected 'F:' ... what is going on here??" );
            }
            
            if( mbStrPos( buf_p, "B:" ) == 0 )
            {
                if( gBFilesFlag == FALSE )
                {
                    fprintf( stdout, "Not deleting 'B:' files without -B flag (%s)\n", buf_p );
                    fflush( stdout );
                    continue;
                }
            }

            mbStrCleanWhite( buf_p, NIL );
            memcpy( md5str, buf_p + 2, 32 );
            md5str[32] = 0;
            mbStrCleanWhite( md5str, NIL );

            if( strlen( md5str ) != 32 )
            {
                printf( "Invalid md5sum detected\n" );
                continue;
            }

            delFile_p = mbFileMalloc( buf_p, NIL );
            strcpy( delFile_p->md5str, md5str );

            qInsertLast( del_q, delFile_p );
        }
    }

    fclose( fp );
    fp = NIL;

    if( del_q->size == 0 )
    {
        fprintf( stdout, "No files to delete\n" );
        fflush( stdout );
        goto exit;
    }

    if( list_q->size == 0 )
    {
        fprintf( stdout, "No list files to update\n" );
        fflush( stdout );
        goto exit;
    }
    //mbFileListDisplay( del_q );
    //mbFileListDisplay( list_q );

    // Loop through all the list files.  If a file is to be deleted, 
    // references to it are removed from the list file.
    qWalk( listFile_p, list_q, MbFile_p )
    {
        fprintf( stdout, "Processing list file '%s'\n", listFile_p->pathName_p );
        fflush( stdout );

        modified = FALSE;

        if( ( lp = fopen( listFile_p->pathName_p, "r" ) ) == NIL )
        {
            printf( "Failed to open list file: '%s'\n", listFile_p->pathName_p );
            continue;
        }

        sprintf( target_p, "%s/done/%s-TEMP", backupDir_p, listFile_p->name_p );

        if( ( fp = fopen( target_p, "w" ) ) == NIL )
        {
            printf( "Failed to open list file: '%s'\n", listFile_p->pathName_p );
            fclose( lp );
            continue;
        }


        while( fgets( buf2_p, 4096, lp ) != NULL )
        {
            skipLine = FALSE;
            
            // Could speed things up a little bit by not even considering
            // lines that do not contain 'F:'...
            qWalk( delFile_p, del_q, MbFile_p )
            {
                // Instead of looking for position greater than zero,
                // we really should look for a specific position
                if( mbStrPos( buf2_p, delFile_p->md5str ) > 0 )
                {
                    strcpy( temp_p, buf2_p + 35 );
                    mbStrCleanWhite( temp_p, NIL );

                    fprintf( stdout, "Found: '%s' (%s)\n",
                            delFile_p->name_p, temp_p );

                    fflush( stdout );
                    modified = TRUE;
                    skipLine = TRUE;
                    break;
                }
            }

            if( skipLine == FALSE )
            {
               fprintf( fp, "%s", buf2_p );
            }
        }

        fclose( fp );
        fclose( lp );
        fp = NIL;

        if( modified )
        {
            sprintf( cmd_p, "%s -f %s %s", CMD_MV, target_p, listFile_p->pathName_p );
        }
        else
        {
            sprintf( cmd_p, "%s %s", CMD_RMDIR, target_p );
        }

        //fprintf( stdout, "command: '%s'\n", cmd_p );
        //fflush( stdout );

        if( mbSystem( cmd_p, NULL ) != 0 )
        {
            printf( "mbSystem( %s ) failed\n", cmd_p );
            continue;
        }
    }

    result  = TRUE;

exit:

    if( zipFlag )
    {
        printf( "Compressing backup database...\n" );

        sprintf( cmd_p, "%s %s/done/*.txt", CMD_GZIP, backupDir_p );
        //printf( "command: '%s'\n", cmd_p );

        if( mbSystem( cmd_p, NULL ) != 0 )
        {
            printf( "mbSystem( %s ) failed\n", cmd_p );
        }
    }
    printf( "Done file delete\n" );

    mbFileListFree( list_q );
    mbFileListFree( del_q );

    mbFree( buf_p );
    mbFree( cmd_p );
    mbFree( target_p );
    mbFree( buf2_p );
    mbFree( temp_p );

    if( fp ) fclose( fp );
    return result;
}

/******************************************************************************
 * Procedure: main( )
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

Ints_t main
(
    Ints_t          argc,
    Char_p          argv[]
)
{
    Int16s_t        option;
    qHead_t         dirQueueHead;
    qHead_p         dir_q;
    qHead_t         fileQueueHead;
    qHead_p         file_q;
    qHead_t         deleteQueue;
    qHead_p         delete_q;
    MbFile_p        file_p;
    MbFile_p        delFile_p;
    Boolean_t       helpFlag = TRUE;
    Char_p          backupDir_p = NIL;
    Char_p          dateStr_p = NIL;
    Char_p          source_p = NIL;
    Char_p          target_p = NIL;
    Char_p          target2_p = NIL;
    Char_p          cmd_p = NIL;
    Char_p          backupFile_p = NIL;
    Char_p          buf_p = NIL;
    Char_p          path_p = NIL;
    Char_p          fileName_p = NIL;
    Char_p          curDir_p;
    Char_p          dest_p;
    Char_p          tmp_p;
    Char_p          tmp2_p;
    Char_p          deleteFile_p = NIL;
    pid_t           myPid;
    time_t          runTime;
    FILE           *fp = NIL;
    Char_t          md5str[64];
    Boolean_t       found;
    Boolean_t       restoreFlag = FALSE;
    Boolean_t       checkFlag = FALSE;
    Boolean_t       historyFlag = FALSE;
    Boolean_t       zipped;
    Int32u_t        count = 0;
    Int32u_t        failed = 0;
    Char_p          benchmarkDir_p = NIL;
    Boolean_t       benchmarkWriteFlag = FALSE;

    myPid = getpid( );
    runTime = time( NIL );

    source_p        = mbMalloc( 10 * 1024 );
    target_p        = mbMalloc( 10 * 1024 );
    target2_p       = mbMalloc( 10 * 1024 );
    dest_p          = mbMalloc( 10 * 1024 );
    cmd_p           = mbMalloc( 10 * 1024 );
    backupFile_p    = mbCalloc( 1024 );
    fileName_p      = mbMalloc( 1024 );
    buf_p           = mbMalloc( 4096 );
    tmp_p           = mbMalloc( 4096 );
    tmp2_p          = mbMalloc( 4096 );
    curDir_p        = mbMalloc( MAX_NAME_LEN );

    gListFlag = FALSE;
    gRemoveFlag = FALSE;
    gBFilesFlag = FALSE;

    getcwd( curDir_p, MAX_NAME_LEN );

    dir_q = qInitialize( &dirQueueHead );
    file_q = qInitialize( &fileQueueHead );
    delete_q = qInitialize( &deleteQueue );

    while( ( option = getopt( argc, argv, "xrD:d:f:b:hv?clX:Bt:w" )) != -1 )
    {
        /* Directory to backup */
        switch( option )
        {
          case 'B':
            gBFilesFlag = TRUE;
            break;

          case 'l':
            gListFlag = TRUE;
            break;

          case 'c':
            checkFlag = TRUE;
            break;

          case 'h':
            historyFlag = TRUE;
            break;

          case 'b':
            backupDir_p =  mbMallocStr( optarg );
            break;

          case 'f':
            file_p = mbFileMalloc( NIL, optarg );
            qInsertLast( file_q, file_p );
            break;

          case 'd':
            dateStr_p = mbMallocStr( optarg );
            break;

          case 'D':
            file_p = mbFileMalloc( NIL, optarg );
            qInsertLast( dir_q, file_p );
            break;

          case 't':
            benchmarkDir_p = mbMallocStr( optarg );
            break;

          case 'w':
            benchmarkWriteFlag = TRUE;
            break;

          case 'r':
            restoreFlag = TRUE;
            break;

          case 'x':
            gRemoveFlag = TRUE;
            break;

          case 'X':
            deleteFile_p = mbMallocStr( optarg );
            break;

          case '?':
          case 'v':
            goto exit;

          default:
            printf( "%s: %c: unknown option.\n", RS_PROG_NAME, option );
            goto exit;
        }
    }

    mbStrRemoveSlashTrailing( backupDir_p );
    mbStrRemoveSlashTrailing( benchmarkDir_p );

    if( historyFlag == TRUE ) checkFlag = TRUE;

    if( benchmarkDir_p != NIL )
    {
        benchmark( benchmarkDir_p, benchmarkWriteFlag );
        helpFlag = FALSE;
        goto exit;
    }

    if( backupDir_p == NIL )
    {
        printf( "%s: must specify backup directory (-b)\n", RS_PROG_NAME );
        goto exit;
    }

    if( deleteFile_p != NIL )
    {
        if( checkFlag || restoreFlag )
        {
            printf( "%s: check (-c) restore (-r) and delete (-X) are mutually exclusive operations\n", RS_PROG_NAME );
            goto exit;
        }
        if( filesDelete( backupDir_p, deleteFile_p ) == TRUE )
        {
            helpFlag = FALSE;
        }
        goto exit;
    }


    if( checkFlag == TRUE )
    {
        Int32u_t historyDate = 0;
        if( dateStr_p ) historyDate = mbDateIntFromString( dateStr_p );

        if( restoreFlag == TRUE || deleteFile_p )
        {
            printf( "%s: check (-c) restore (-r) and delete (-X) are mutually exclusive operations\n", RS_PROG_NAME );
            goto exit;
        }
        if( check( backupDir_p, historyFlag, historyDate ) == TRUE )
        {
            helpFlag = FALSE;
        }
        goto exit;
    }

    if( dateStr_p == NIL )
    {
        printf( "%s: must specify backup date in format YYYY-MM-DD (-d)\n", RS_PROG_NAME );
        goto exit;
    }

    helpFlag = FALSE;

    sprintf( backupFile_p, "/tmp/restore-%lu-%lu-%s.txt", (Int32u_t)myPid, (Int32u_t)runTime, dateStr_p );

    sprintf( source_p, "%s/done/%s.txt.gz" , mbFileNameEsc( backupDir_p, tmp_p ), dateStr_p );
    // printf( "checking for file '%s'\n", source_p );
    sprintf( cmd_p, "%s -c %s > %s 2>/dev/null", CMD_GUNZIP,
        mbFileNameEsc( source_p, tmp_p ),
        mbFileNameEsc( backupFile_p, tmp2_p ) );

    if( mbSystem( cmd_p, NIL ) != 0 )
    {
        sprintf( source_p, "%s/new/%s.txt" , backupDir_p, dateStr_p );
        // printf( "checking for file '%s'\n", source_p );
        sprintf( cmd_p, "%s %s %s", CMD_CP,
            mbFileNameEsc( source_p, tmp_p ),
            mbFileNameEsc( backupFile_p, tmp2_p ) );

        if( mbSystem( cmd_p, NIL ) != 0 )
        {
            printf( "Failed to get file '%s'\n", source_p );
            goto exit;
        }
    }

    if( ( fp = fopen( backupFile_p, "r" ) ) == NIL )
    {
        printf( "Failed to open file '%s'\n", backupFile_p );
        goto exit;
    }

    while( fgets( buf_p, 4096, fp ) != NULL )
    {
        if( mbStrPos( buf_p, "P:" ) == 0 )
        {
            found = TRUE;
            mbFree( path_p );
            path_p = mbMallocStr( buf_p + 2 );
            mbStrCleanWhite( path_p, NIL );

            if( dir_q->size )
            {
                found = FALSE;
                qWalk( file_p, dir_q, MbFile_p )
                {
                    // printf( "consider dirs matching '%s'\n", file_p->path_p );
                    if( mbStrPos( path_p, file_p->path_p ) >= 0 )
                    {
                        found = TRUE;
                        break;
                    }
                }
            }

            if( found )
            {
                // printf( "DIRECTORY: '%s'\n", path_p );
            }
            else
            {
                mbFree( path_p );
                path_p = NIL;
            }
        }
        else if( mbStrPos( buf_p, "F:" ) == 0 )
        {
            if( path_p == NIL ) continue;

            memcpy( md5str, buf_p + 2, 32 );
            md5str[32] = 0;

            *fileName_p = 0;
            strcpy( fileName_p, buf_p + 35 );
            mbStrCleanWhite( fileName_p, NIL );

            sprintf( target_p, "%s/%s", path_p, fileName_p );

            found = TRUE;
            if( file_q->size )
            {
                found = FALSE;
                qWalk( file_p, file_q, MbFile_p )
                {
                    // printf( "consider dirs matching '%s'\n", file_p->path_p );
                    if( mbStrPos( target_p, file_p->path_p ) >= 0 )
                    {
                        found = TRUE;
                        break;
                    }
                }
            }

            if( found == FALSE ) continue;

            sprintf( source_p, "%s/files/%c%c/%c%c/%s", backupDir_p,
                md5str[0], md5str[1], md5str[2], md5str[3], md5str );

            found = FALSE;

            if( mbFileExists( mbFileNameEsc( source_p, tmp_p ) ) )
            {
                zipped = FALSE;
                found = TRUE;
            }
            else
            {
                sprintf( source_p, "%s/files/%c%c/%c%c/%s.gz", backupDir_p,
                    md5str[0], md5str[1], md5str[2], md5str[3], md5str );

                if( mbFileExists( mbFileNameEsc( source_p, tmp_p ) ) )
                {
                    zipped = TRUE;
                    found = TRUE;
                }
            }

            if( found )
            {
                if( restoreFlag == TRUE )
                {
                    sprintf( dest_p, "restored_files/%s%s", dateStr_p, path_p );
                    sprintf( cmd_p, "%s -p %s > /dev/null 2>/dev/null", CMD_MKDIR, mbFileNameEsc( dest_p, tmp_p ) );

                    if( mbSystem( cmd_p, NIL ) != 0 )
                    {
                       printf( "Failed to create directory '%s'\n", dest_p );
                    }
                    else
                    {
                        sprintf( target2_p, "%s/%s/%s", curDir_p, dest_p, fileName_p );
                        if( zipped == TRUE )
                        {
                            sprintf( cmd_p, "%s -c %s > %s", CMD_GUNZIP,
                                mbFileNameEsc( source_p, tmp_p ),
                                mbFileNameEsc( target2_p, tmp2_p ) );
                        }
                        else
                        {
                             sprintf( cmd_p, "/bin/cp %s %s",
                                mbFileNameEsc( source_p, tmp_p ),
                                mbFileNameEsc( target2_p, tmp2_p ) );
                        }

                        if( mbSystem( cmd_p, NIL ) != 0 )
                        {
                            printf( "FAILED: could not restore file '%s'\n", target_p );
                            printf( "command '%s'\n", cmd_p );
                            failed++;
                        }
                        else
                        {
                            printf( "RESTORED: '%s'\n", target_p );
                            if( gRemoveFlag )
                            {
                                delFile_p = mbFileMalloc( mbFileNameEsc( source_p, tmp_p ), NIL );
                                qInsertLast( delete_q, delFile_p );
                                //if( unlink( mbFileNameEsc( source_p, tmp_p ) ) == 0 )
                                //{
                                //    printf( "DELETED: '%s'\n", source_p );
                               // }
                                //else
                               // {
                                //    printf( "FAILED: could not delete '%s'\n", source_p );
                               // }
                            }
                            count++;
                        }
                    }
                }
                else
                {
                    count++;
                    printf( "AVAILABLE: '%s'\n", target_p );
                    // printf( "(source:   '%s'\n", source_p );

                }
            }
            else
            {
                printf( "NOT AVAIALBLE: '%s'\n", target_p );
            }
        }
    }

    while( !qEmpty( delete_q ) )
    {
        delFile_p = (MbFile_p)qRemoveFirst( delete_q );
        if( unlink( delFile_p->name_p ) == 0 )
        {
            printf( "DELETED: '%s'\n", delFile_p->name_p );
        }
        else
        {
            printf( "DELETED FAILED: '%s'\n", delFile_p->name_p );
        }
        mbFileFree( delFile_p );
    }

exit:

    if( count || failed )
    {
        if( restoreFlag == TRUE )
        {
            printf( "%lu files restored\n", count );
            printf( "%lu files failed restore\n", failed );
        }
        else
        {
            printf( "%lu file%s would be restored.  Use -r to actually restore files\n",
		    count, ( count == 1 ) ? "" : "s" );
        }
    }

    mbFileListFree( dir_q );
    mbFileListFree( file_q );

    if( helpFlag )
    {
        printf( "\n%s: restore backed up files (%s %s)\n", "restore" , __DATE__, __TIME__ );
        printf( "\noptions:\n\n" );
        printf( " -b    <backp_dir>         root of the backup directory\n" );
        printf( " -d    <date_str>          date string in format YYYY-MM-DD\n" );
        printf( " -D    <dir>               directory to restore\n" );
        printf( " -f    <file>              file to restore\n" );
        printf( " -h                        history\n" );
        printf( " -r                        restore (default: list only)\n" );
        printf( " -x                        delete restored files from backup; delete orphans during check (-c)\n" );
        printf( " -c                        check backup database (does not restore)\n" );
        printf( " -l                        list backed-up files during check\n" );
        printf( " -X     <list_file>        delete non-backed up files (marked 'N:' in <list_file>)\n" );
        printf( " -B                        delete backed up files (marked 'B:' in -X <list_file>)\n" );
        printf( " -t     <dir>              simple disk access benchmark - read (use with -w for write)\n" );
        printf( " -w                        simple disk access benchmark - write (use with -t)\n" );
        printf( " -v                        print version" );

        printf( "\n" );
        printf( "\n" );
    }

    if( fp ) fclose( fp );

    if( backupFile_p )
    {
        if( strlen( backupFile_p ) )
        {
            //printf( "skipping unlink of %s\n", backupFile_p );
            unlink( mbFileNameEsc( backupFile_p, tmp_p ) );
        }
    }

    mbFree( source_p );
    mbFree( target_p );
    mbFree( cmd_p );
    mbFree( buf_p );
    mbFree( path_p );
    mbFree( fileName_p );
    mbFree( curDir_p );
    mbFree( dest_p );
    mbFree( tmp_p );
    mbFree( target2_p );
    mbFree( deleteFile_p );

    {
        Int32u_t chunks = mbMallocCount();

        if( chunks != 0 )
        {
            // printf( "Memory chunks: %lu\n", chunks );
        }
    }
    return 0;
}
