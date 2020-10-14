/******************************************************************************
 * File: backup.c
 *-----------------------------------------------------------------------------
 * Copyright (c) 2007-2008, Michael A. Bree
 *-----------------------------------------------------------------------------
 * Retreives file list and listed files from remote machine.
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

/* Utility include files */
#include "mbUtils.h"

/* Project include files */
#include "backup.h"

/* External references */
extern char *optarg;

/* Global variables */
Char_p      gBackupDir_p = NIL;
pthread_t   gWorkerThread = 0;
Int32u_t    gWatchdogCounter = 0;
Boolean_t   gThreadStopped = FALSE;
Ints_t      gSystemPid = 0;
Ints_t      gSkipCount = 0;

/* Function prototypes */
Int32s_t backup( qHead_p dir_q, qHead_p skip_q, Boolean_t recurseFlag,
                 Boolean_t followSymLinkFlag );

Int32s_t snapshot( qHead_p      q_p,
                   qHead_p      skip_q,
                   FILE        *fp,
                   Int32u_p     fileCount_p,
                   Int64u_p     fileLength_p );

Void_p  workerThreadEntry( Void_p args_p );

Int32s_t backup_remote( Char_p host_p, Char_p remoteDir_p, Char_p localDir_p );
Int32s_t scpFile( Char_p host_p, Char_p source_p, Char_p target_p );

#define BK_MODE_BACKUP  1
#define BK_MODE_RESTORE 2
#define BK_MODE_GET     3

typedef struct ThreadArgs_s
{
    Int32u_t    mode;
    qHead_p     dir_q;
    qHead_p     skip_q;
    Boolean_t   recurseFlag;
    Boolean_t   followSymLinkFlag;
    Char_p      host_p;
    Char_p      remoteDir_p;
    Char_p      localDir_p;
} ThreadArgs_t, *ThreadArgs_p;

/******************************************************************************
 * Procedure: main( )
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

int main
(
    int             argc,
    Char_p          argv[]
)
{
    Int16s_t        option;
    qHead_t         dirQueueHead;
    qHead_p         dir_q;
    MbFile_p        file_p;

    qHead_t         skipQueue;
    qHead_p         skip_q;

    Boolean_t       helpFlag = TRUE;
    Boolean_t       recurseFlag = TRUE;
    Char_t          curDir[MAX_NAME_LEN];

    Char_p          logDir_p = NIL;
    Char_p          host_p = NIL;
    Char_p          remoteDir_p = NIL;
    Char_t          lockFile[MAX_NAME_LEN];

    Boolean_t       gotLock = FALSE;
    Boolean_t       backupFlag = FALSE;
    Boolean_t       restoreFlag = FALSE;
    Boolean_t       getFlag = FALSE;
    Boolean_t       followSymLinkFlag = FALSE;
    Ints_t          modeCount = 0;
    Ints_t          i;
    Int32u_t        ticks = 0;
    ThreadArgs_t    threadArgs;

    dir_q = qInitialize( &dirQueueHead );
    skip_q = qInitialize( &skipQueue );

    getcwd( curDir, MAX_NAME_LEN );

    while( ( option = getopt( argc, argv, "r:d:l:b:h:v?fs:" )) != -1 )
    {
        switch( option )
        {
          case 's':
            printf("we want to skip '%s'\n", (Char_p)optarg );
            //file_p = mbFileMalloc( NIL, optarg );

            mbStrListAdd( skip_q, optarg );

            //qInsertLast( skip_q, file_p );
            break;

          case 'd':
            file_p = mbFileMalloc( NIL, optarg );
            qInsertLast( dir_q, file_p );
            break;

          case 'l':
            logDir_p = (Char_p)mbMallocStr( optarg );
            break;

          case 'b':
            gBackupDir_p = mbMallocStr( optarg );
            backupFlag = TRUE;
            break;

          case 'h':
            host_p = mbMallocStr( optarg );
            getFlag = TRUE;
            break;

          case 'f':
            followSymLinkFlag = TRUE;
            printf("Follow symlink = True\n");

          case 'r':
            remoteDir_p = mbMallocStr( optarg );
            break;

          case '?':
          case 'v':
            goto exit;

          default:
            printf( "%s: %c: unknown option.\n", BK_PROG_NAME, option );
            goto exit;
        }
    }

    // exit( 0 );

    mbStrRemoveSlashTrailing( logDir_p );
    mbStrRemoveSlashTrailing( gBackupDir_p );
    mbStrRemoveSlashTrailing( remoteDir_p );

    if( backupFlag == FALSE && restoreFlag == FALSE && getFlag == FALSE )
    {
        /* No actions specified... output help */
        goto exit;
    }

    if( logDir_p == NIL ) logDir_p = mbMallocStr( "/tmp" );

    mbLogInit( "backup", logDir_p );

    mbLog( "Started\n" );

    helpFlag = FALSE;

    if( backupFlag == TRUE )    modeCount++;
    if( getFlag == TRUE )       modeCount++;
    if( restoreFlag == TRUE )   modeCount++;

    if( modeCount > 1 )
    {
        mbLog( "Multiple modes selected, terminating\n" );
        helpFlag = TRUE;
        goto exit;
    }

    if( getFlag == TRUE )
    {
        sprintf( lockFile, "%s/%s", logDir_p, BK_LOCK_FILE_GET );
    }
    else
    {
        sprintf( lockFile, "%s/%s", logDir_p, BK_LOCK_FILE );
    }

    if( !mbLockFileGet( lockFile ) )
    {
        mbLog( "Could not acquire lock file; not running\n" );
        goto exit;
    }

    gotLock = TRUE;

    if( backupFlag == TRUE )
    {
        if( dir_q->size == 0 )
        {
            mbLog( "No directories specified for backup\n" );
            goto exit;
        }
        threadArgs.mode = BK_MODE_BACKUP;
        threadArgs.dir_q = dir_q;
        threadArgs.skip_q = skip_q;
        threadArgs.recurseFlag = recurseFlag;
        threadArgs.followSymLinkFlag = followSymLinkFlag;
    }
    else if( getFlag == TRUE )
    {
        if( remoteDir_p == NIL )
        {
            mbLog( "Remote directory node specified\n" );
            goto exit;
        }
        if( dir_q->size != 1 )
        {
            mbLog( "Local directory not specified\n" );
            goto exit;
        }
        file_p = (MbFile_p)qRemoveFirst( dir_q );
        qInsertLast( dir_q, file_p );

        mbStrRemoveSlashTrailing( file_p->path_p );

        threadArgs.mode = BK_MODE_GET;
        threadArgs.host_p = host_p;
        threadArgs.remoteDir_p = remoteDir_p;
        threadArgs.localDir_p = file_p->path_p;
    }
    else if( restoreFlag == TRUE )
    {
    }
    else
    {
        mbLog( "No action specified\n" );
        goto exit;
    }

    /* Start the worker thread */
    pthread_create( &gWorkerThread, 0, (Void_p)workerThreadEntry, (Void_p)&threadArgs );

    /* Watchdog loop */
    for( i = 0 ; ; i++ )
    {
        sleep( 1 );

        if( gThreadStopped == TRUE )
        {
            gWatchdogCounter = 1;
            break;
        }
        if( ++ticks > 3600 )
        {
            if( gWatchdogCounter == 0 )
            {
                break;
            }
            ticks = 0;
            gWatchdogCounter = 0;
        }
    }

    if( gWatchdogCounter ==  0 )
    {
        mbLog( "Watchdog counter failed\n" );
        if( gSystemPid )
        {
            mbLog( "must kill pid: %d\n", gSystemPid );
            kill( gSystemPid, SIGKILL );
        }
    }
    else
    {
        mbLog( "Normal stop detected\n" );
    }

exit:

    if( gotLock ) unlink( lockFile );

    mbFileListFree( dir_q );
    // mbFileListFree( skip_q );
    mbStrListFree( skip_q );

    if( helpFlag )
    {
        printf( "\n%s: backup files (%s %s)\n", BK_PROG_NAME, __DATE__, __TIME__ );
        printf( "\n" );
        printf( "This program operates in two modes:\n\n"
                "-h option is specified: Files are backed up from a remote host.\n\n"
                "-d option is specified: A 'snapshot' of local files and their checksums \n"
                "   is created.  This is used by a remote host to backup the local host.\n" );
        printf( "\n" );
        printf( "options:\n\n" );
        printf( " -b <backup_dir>       Local backup directory results\n" );
        printf( " -d <dir>              Local directory to backup (multiple -d allowed)\n" );
        printf( " -l <log_dir>          Log (and lock file) directory\n" );
        printf( " -h <remote_host>      Remote host to backup\n" );
        printf( " -r <remote_dir>       Remote directory (i.e., remote file list location)\n" );
        printf( " -s <skip_string>      In -d, skip any /path/file containing <skip_string>\n" );
        printf( " -f                    Follow SymLinks\n" );
        printf( "\n" );
    }
    else
    {
        mbLog( "Done\n" );
    }

    mbFree( host_p );
    mbFree( remoteDir_p );
    mbFree( gBackupDir_p );
    mbFree( logDir_p );

    {
        Int32u_t chunks = mbMallocCount();

        if( chunks != 0 )
        {
            printf( "Memory chunks: %lu\n", chunks );
        }
    }

    return 0;
}

/******************************************************************************
 * Function: workerThreadEntry( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */
Void_p              workerThreadEntry
(
    Void_p          args_p
)
{
    ThreadArgs_p    threadArgs_p;

    threadArgs_p = (ThreadArgs_p)args_p;

    mbLog( "Worker thread started\n" );

    if( threadArgs_p == NIL ) goto exit;

    if( threadArgs_p->mode == BK_MODE_BACKUP )
    {
        backup( threadArgs_p->dir_q, threadArgs_p->skip_q,
                threadArgs_p->recurseFlag, threadArgs_p->followSymLinkFlag );
    }
    else if( threadArgs_p->mode == BK_MODE_GET )
    {
        backup_remote( threadArgs_p->host_p,
                       threadArgs_p->remoteDir_p,
                       threadArgs_p->localDir_p );
    }

exit:
    gThreadStopped = TRUE;

    return NIL;
}

/******************************************************************************
 * Function: backup_remote( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * Coming back and looking at this in 202 I can't beleive I wrote code like
 * this.  This function does way too much and is difficult to test.
 *-----------------------------------------------------------------------------
 */

Int32s_t backup_remote( Char_p host_p, Char_p remoteDir_p, Char_p localDir_p )
{
    Char_p          cmd_p= NIL;
    Char_p          source_p = NIL;
    Char_p          target_p = NIL;
    Char_p          buf_p = NIL;
    Char_p          name_p = NIL;
    Char_p          path_p = NIL;
    Char_t          dateStr[64];
    Char_t          md5str[64];
    Char_t          md5str2[64];
    Boolean_t       success = FALSE;
    Boolean_t       found;
    FILE           *fp = NIL;
    Int32u_t        fileCount = 0;
    Int32u_t        filesFailed = 0;
    Int32u_t        filesPrev = 0;
    Int32u_t        size1, size2;
    qHead_t         fileQueue;
    qHead_p         file_q;
    qHead_t         tempQueue;
    qHead_p         temp_q;
    MbFile_p        file_p;

    file_q      = qInitialize( &fileQueue );
    temp_q      = qInitialize( &tempQueue );

    cmd_p       = mbMalloc( 20000 );
    source_p    = mbMalloc( 2000 );
    target_p    = mbMalloc( 2000 );
    buf_p       = mbMalloc( 4096 );
    name_p      = mbMalloc( 1024 );

    mbLog( "Want to get files from '%s'\n", host_p );
    mbLog( "remote dir '%s'\n", remoteDir_p );
    mbLog( "local dir '%s'\n", localDir_p );

    mbCharDate( dateStr );

    sprintf( source_p, "%s/%s.txt.gz", remoteDir_p, dateStr );
    sprintf( target_p, "%s/done/%s.txt.gz", localDir_p, dateStr );

    mbLog( "source '%s'\n", source_p );
    mbLog( "target '%s'\n", target_p );

    if( mbFileExists( target_p ) )
    {
        mbLog( "File '%s' aleady processed\n", target_p );
        goto exit;
    }

    sprintf( target_p, "%s/temp", localDir_p );
    sprintf( cmd_p, "%s %s\n", CMD_RMDIR, target_p );

    mbLog( cmd_p );
    if( mbSystem( cmd_p, &gSystemPid ) != 0 )
    {
        mbLog( "Failed to remove directory: %s\n", target_p );
        goto exit;
    }

    sprintf( cmd_p, "%s %s\n", CMD_MKDIR, target_p );
    mbLog( cmd_p );
    if( mbSystem( cmd_p, &gSystemPid ) != 0 )
    {
        mbLog( "Failed to create directory: %s\n", target_p );
        goto exit;
    }

    sprintf( target_p, "%s/new/%s.txt", localDir_p, dateStr );

    if( mbFileExists( target_p ) )
    {
        mbLog( "File '%s' already exists\n", target_p );
    }
    else
    {
        sprintf( target_p, "%s/temp/%s.txt.gz", localDir_p, dateStr );
        mbLog( "Attempt to get file: %s\n", source_p );
        if( scpFile( host_p, source_p, target_p ) == FALSE )
        {
            mbLog( "Failed to get file: %s\n", source_p );
            goto exit;
        }

        sprintf( cmd_p, "%s %s\n", CMD_GUNZIP, target_p );

        mbLog( cmd_p );
        if( mbSystem( cmd_p, &gSystemPid ) != 0 )
        {
            mbLog( "Failed to unzip file: %s\n", target_p );
            goto exit;
        }

        sprintf( source_p, "%s/temp/%s.txt", localDir_p, dateStr );
        sprintf( target_p, "%s/new/%s.txt", localDir_p, dateStr );
        sprintf( cmd_p, "%s %s %s\n", CMD_MV, source_p, target_p );

        mbLog( cmd_p );
        if( mbSystem( cmd_p, &gSystemPid ) != 0 )
        {
            mbLog( "Failed to move file: %s\n", target_p );
            goto exit;
        }
    }

    if( ( fp = fopen( target_p, "r" ) ) == NIL )
    {
        mbLog( "Failed to open file: %s\n", target_p );
        goto exit;
    }

    mbLog( "Scanning local files\n" );
    sprintf( target_p, "%s/files", localDir_p );

    mbFileListGet( target_p, file_q, TRUE, FALSE, FALSE );

    while( !qEmpty( file_q ) )
    {
        file_p = (MbFile_p)qRemoveFirst( file_q );
        if( file_p->type == MB_FILE_TYPE_FILE )
        {
            qInsertLast( temp_q, file_p );
        }
        else
        {
            mbFileFree( file_p );
        }
    }

    file_q = temp_q;

#if 0
    qWalk( file_p, file_q, MbFile_p )
    {
        mbLog( "Local file '%s' (%s)\n", file_p->pathName_p, file_p->name_p );
    }
#endif

    mbLog( "Detected %lu local files\n", file_q->size );

    while( fgets( buf_p, 4096, fp ) != NULL )
    {
        gWatchdogCounter++;

        //mbLog( buf_p );
        if( mbStrPos( buf_p, "P:" ) == 0 )
        {
            mbLog( "Found path '%s'\n",  buf_p + 2 );
            mbFree( path_p );
            path_p = mbMallocStr( buf_p + 2 );
            mbStrCleanWhite( path_p, NIL );
        }
        else if( mbStrPos( buf_p, "F:" ) == 0 )
        {
            memcpy( md5str, buf_p + 2, 32 );
            md5str[32] = 0;

            strcpy( name_p, buf_p + 35 );
            mbStrCleanWhite( name_p, NIL );
            mbLog( "Found file '%s' md5 '%s'PATH '%s'\n", name_p, md5str, path_p );

            /* Check to see if we already have this file */
            found = FALSE;
            qWalk( file_p, file_q, MbFile_p )
            {
                //mbLog( "Local file '%s' (%s)\n", file_p->pathName_p, file_p->name_p );
                if( mbStrPos( file_p->name_p, md5str ) == 0 )
                {
                    // mbLog( "Already have file %s '%s' \n", md5str, name_p );
                    found = TRUE;
                    filesPrev++;
                    break;
                }
            }

            if( found ) continue;

            sprintf( source_p, "%s/%s", path_p, name_p );
            sprintf( target_p, "%s/temp/%s", localDir_p, md5str );

            filesFailed++;

            mbLog( "Calling scp %s -> %s\n", source_p, target_p );
            if( scpFile( host_p, source_p, target_p ) == FALSE )
            {
                mbLog( "Failed to get file: '%s'\n", source_p );
            }
            else
            {
                /* Ensure file matches expected checksum */
                mbMD5File( target_p, NIL, md5str2, &size1 );

                mbLog( "Got file %d: '%s'  -- %u\n", (fileCount + 1), source_p, size1 );

                if( strcmp( md5str, md5str2 ) == 0 )
                {
                    sprintf( cmd_p, "%s %s", CMD_GZIP, target_p );
                    if( mbSystem( cmd_p, &gSystemPid ) != 0 )
                    {
                        mbLog( "Failed to zip file: '%s'\n", target_p );
                    }
                    else
                    {
                        sprintf( target_p, "%s/files/%c%c", localDir_p, md5str[0], md5str[1] );
                        sprintf( cmd_p, "%s %s > /dev/null 2>/dev/null", CMD_MKDIR, target_p );
                        mbSystem( cmd_p, &gSystemPid );

                        sprintf( target_p, "%s/files/%c%c/%c%c", localDir_p, md5str[0], md5str[1], md5str[2], md5str[3] );
                        sprintf( cmd_p, "%s %s > /dev/null 2>/dev/null", CMD_MKDIR, target_p );
                        mbSystem( cmd_p, &gSystemPid );

                        sprintf( source_p, "%s/temp/%s.gz", localDir_p, md5str );
                        mbMD5File( source_p, NIL, md5str2, &size2 );
                        if( size2 < size1 )
                        {
                            //mbLog( "size2 is less that size1, store gzipped version (%lu %lu)\n", size1, size2 );
                        }
                        else
                        {
                            //mbLog( "size2 is more size1, store ungzipped version (%lu %lu)\n", size1, size2 );
                            sprintf( cmd_p, "%s %s", CMD_GUNZIP, source_p );
                            if( mbSystem( cmd_p, &gSystemPid ) != 0 )
                            {
                                mbLog( "Failed to unzip file: '%s'\n", source_p );
                            }
                            sprintf( source_p, "%s/temp/%s", localDir_p, md5str );
                        }

                        sprintf( cmd_p, "%s %s %s/.", CMD_MV, source_p, target_p );
                        if( mbSystem( cmd_p, &gSystemPid ) != 0 )
                        {
                            mbLog( "Failed to move file: '%s'\n", source_p );
                            unlink( source_p );
                        }
                        else
                        {
                            file_p = mbFileMalloc( md5str, NIL );
                            qInsertLast( file_q, file_p );
                            fileCount++;
                            filesFailed--;
                        }
                    }

                }
                else
                {
                    mbLog( "Checksum error: '%s'\n", source_p );
                    unlink( target_p );
                }
            }
        }
        else if( mbStrPos( buf_p, "E:END" ) == 0 )
        {
            success = TRUE;
            break;
        }
    }

    fclose( fp );

    mbLog( "Got files: %lu\n", fileCount );
    mbLog( "Previous files: %lu\n", filesPrev );
    mbLog( "Failed files: %lu files\n", filesFailed );

    if( success )
    {
        /* gzip the file and move it to the done directory */
        sprintf( target_p, "%s/new/%s.txt", localDir_p, dateStr );
        sprintf( cmd_p, "%s %s\n", CMD_GZIP, target_p );

        mbLog( cmd_p );
        if( mbSystem( cmd_p, &gSystemPid ) != 0 )
        {
            mbLog( "Failed to zip file: %s\n", target_p );
            goto exit;
        }

        sprintf( cmd_p, "%s %s.gz %s/done/.\n", CMD_MV, target_p, localDir_p );

        if( mbSystem( cmd_p, &gSystemPid ) != 0 )
        {
            mbLog( "Failed to move file: %s.gz\n", target_p );
            goto exit;
        }
    }


exit:

    mbFileListFree( file_q );
    mbFree( cmd_p );
    mbFree( source_p );
    mbFree( target_p );
    mbFree( buf_p );
    mbFree( name_p );
    mbFree( path_p );
    return TRUE;
}

/******************************************************************************
 * Function: scpFile( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns the starting position of substring i2 within i1.
 *-----------------------------------------------------------------------------
 */

Int32s_t scpFile( Char_p host_p, Char_p source_p, Char_p target_p )
{
    Int32s_t    result = FALSE;
    Char_p      cmd_p = NIL;
    Char_p      tempSrc_p = NIL;
    Ints_t      len;
    Ints_t      i,j;

    if( ( cmd_p = mbMalloc( 20000 ) ) == NIL ) goto exit;

    if( ( tempSrc_p = mbMalloc( 20000 ) ) == NIL ) goto exit;

    len = strlen( source_p );
    for( i = 0, j = 0 ; i < len ; i++ )
    {
        if(  *(source_p + i) == MB_CHAR_QUOTE_SINGLE
         ||  *(source_p + i) == MB_CHAR_SPACE
         ||  *(source_p + i) == '&'
         ||  *(source_p + i) == '~'
         ||  *(source_p + i) == '$'
         ||  *(source_p + i) == '!'
         ||  *(source_p + i) == '`'
         ||  *(source_p + i) == '('
         ||  *(source_p + i) == ')' )
        {
            *(tempSrc_p + j) = MB_CHAR_BACKSLASH;
            j++;
        }
        *(tempSrc_p + j) = *(source_p + i);
        j++;
    }
    *(tempSrc_p + j ) = 0;

    sprintf( cmd_p, "%s %s:\"%s\" %s\n",
                CMD_SCP, host_p, tempSrc_p, target_p );

    //mbLog( cmd_p );
    if( mbSystem( cmd_p, &gSystemPid ) == 0 )
    {
        result = TRUE;
    }

exit:
    gSystemPid = 0;
    mbFree( cmd_p );
    mbFree( tempSrc_p );

    return result;
}

/******************************************************************************
 * Function: snapshot( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns the starting position of substring i2 within i1.
 *-----------------------------------------------------------------------------
 */

Int32s_t backup(
    qHead_p         dir_q,
    qHead_p         skip_q,
    Boolean_t       recurseFlag,
    Boolean_t       followSymLinkFlag
)
{

    qHead_t         fileQueueHead;
    qHead_p         file_q;
    MbFile_p        file_p;
    FILE           *fp;
    Char_t          fileName[MAX_NAME_LEN];
    Char_t          tmpName[MAX_NAME_LEN];
    Char_t          dateStr[64];
    Int64u_t        length = 0;
    Int32u_t        fileCount = 0;
    Int64u_t        totalLength = 0;
    Int32u_t        totalCount = 0;
    Char_t          cmd[2048];

    file_q = qInitialize( &fileQueueHead );

    mbCharDate( dateStr );

    sprintf( tmpName, "%s/tmp-%s.txt", gBackupDir_p, dateStr );
    sprintf( fileName, "%s/%s.txt", gBackupDir_p, dateStr );

    if( ( fp = fopen( tmpName, "w" ) ) == NIL )
    {
        mbLog( "Failed to open output file '%s'\n", tmpName );
        goto exit;
    }

    /* Loop through the specified directories, getting the list of files in each */
    qWalk( file_p, dir_q, MbFile_p )
    {
        mbLog( "Examining directory '%s'...\n", file_p->path_p );

        printf("follow symlink flag: %d\n", (Ints_t)followSymLinkFlag);

        mbFileListGet( file_p->path_p, file_q, recurseFlag,
                       followSymLinkFlag, FALSE );

        snapshot( file_q, skip_q, fp, &fileCount, &length );
        mbFileListFree( file_q );

        mbLog( "Files: %lu\n", fileCount );
        mbLog( "Length: %Lu\n", length );

        totalLength += length;
        totalCount += fileCount;
    }

    mbLog( "Total Files: %lu\n", totalCount );
    mbLog( "Total Length: %Lu\n", totalLength );

exit:

    if( fp )
    {
        fprintf( fp, "E:END\n" );
        fclose( fp );
        sprintf( cmd, "%s %s", CMD_GZIP, tmpName );
        mbSystem( cmd, NIL );

        sprintf( cmd, "%s %s.gz %s.gz", CMD_MV, tmpName, fileName );
        mbSystem( cmd, NIL );
    }
    return TRUE;
}

/******************************************************************************
 * Function: snapshot( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns the starting position of substring i2 within i1.
 *-----------------------------------------------------------------------------
 */

Int32s_t snapshot
(
    qHead_p         q_p,
    qHead_p         skip_q,
    FILE           *fp,
    Int32u_p        fileCount_p,
    Int64u_p        fileLength_p
)
{
    Char_p          lastPath_p = NIL;
    Boolean_t       newPath;
    Boolean_t       pathEmitted = FALSE;
    MbFile_p        file_p;
    mbStrList_p     skipFile_p;
    Int32u_t        length;
    Int64u_t        totalLength = 0;
    Int32u_t        fileCount = 0;

    if( q_p == NIL || fp == NIL ) goto exit;

    qWalk( file_p, q_p, MbFile_p )
    {
        newPath = FALSE;
        if( file_p->type == MB_FILE_TYPE_FILE )
        {
            if( lastPath_p == NIL )
            {
                newPath = TRUE;
            }
            else
            {
                if( strcmp( lastPath_p, file_p->path_p ) != 0 )
                {
                    newPath = TRUE;
                }
            }

            if( newPath )
            {
                pathEmitted = FALSE;
                // fprintf( fp, "P:%s\n", file_p->path_p );
                lastPath_p = file_p->path_p;
            }

            {
                Ints_t      len;
                Boolean_t   skip = FALSE;

                // This section skips files that end in .o
                len = strlen( file_p->name_p );
                if( len > 1 )
                {
                    if( *(file_p->name_p + len - 1 ) == 'o' )
                    {
                        if( *(file_p->name_p + len - 2 ) == '.' ) skip = TRUE;
                    }
                }

                // See if the file contains the skip string
                if( skip_q && skip == FALSE )
                {
                    qWalk( skipFile_p, skip_q, mbStrList_p )
                    {
                        // printf("searching %s for %s\n", file_p->pathName_p, skipFile_p->str_p);
                        if( mbStrPos( file_p->pathName_p, skipFile_p->str_p ) >= 0 )
                        {
                            skip = TRUE;
                            break;
                        }
                    }
                }

                if( skip == FALSE )
                {
                    if( pathEmitted == FALSE )
                    {
                        fprintf( fp, "P:%s\n", file_p->path_p );
                        pathEmitted = TRUE;
                    }
                    mbMD5File( file_p->pathName_p, file_p->md5, file_p->md5str, &length );
                    fprintf( fp, "F:%s %s\n",  file_p->md5str, file_p->name_p );

                    totalLength += (Int64u_t)length;
                    fileCount++;
                }
                else
                {
                    gSkipCount++;
                    mbLog( "Skipping file: %d %s\n", gSkipCount, file_p->pathName_p );
                }
            }
            gWatchdogCounter++;
            //printf( "file '%s' len %lu total: %Lu\n", file_p->name_p, length, totalLength );
        }
    }

    if( fileCount_p ) *fileCount_p = fileCount;

    if( fileLength_p ) *fileLength_p = totalLength;

exit:

    return TRUE;
}


