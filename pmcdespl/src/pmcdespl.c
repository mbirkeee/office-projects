/******************************************************************************
 * File: pmcdespl.c
 *-----------------------------------------------------------------------------
 * Copyright (c) 2001, Michael A. Bree
 *-----------------------------------------------------------------------------
 * Date: March 6, 2001
 *-----------------------------------------------------------------------------
 * Description:
 *
 * NOTES:
 *
 * At some point, this program could be updated to use the utils library.
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
#include <sys/wait.h>

/* Project include files */
#include "mbTypes.h"
#include "mbQueue.h"
#include "mbLock.h"
#include "pmcdesplMain.h"

#define INIT_GLOBALS
#include "pmcdesplGlob.h"
#undef  INIT_GLOBALS

extern char *optarg;

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
    Int32u_t        optCount;
    Int16s_t        option;
    Boolean_t       gotLock = FALSE;
    int             i, ticks = 0;

    /* Initialize globals */
    gMyPid = getpid();
    gMallocCount = 0;
    gWatchdogCounter = 0;
    gSkipPrintFlag = FALSE;
    gSkipConvertFlag = FALSE;
    gArchiveFlag = FALSE;
    gPrintCommand_p      = DS_PRINT_COMMAND;
    gConvertCommand_p    = DS_CONVERT_COMMAND;
    gArchiveCommand_p    = DS_ARCHIVE_COMMAND;

    while( ( option = getopt( argc, argv, "d:snh:p:va" )) != -1 )
    {
        optCount++;

        switch( option )
        {
          case 'a':
            gArchiveFlag = TRUE;
            break;

          case 'd':
            gBaseDir_p = optarg;
            break;

          case 's':
            gSkipPrintFlag = TRUE;
            break;

          case 'n':
            gSkipConvertFlag = TRUE;
            break;

          case 'p':
            gPrintCommand_p = optarg;
            break;

          case 'h':
            gConvertCommand_p = optarg;
            break;

          case 'v':
            printf( "pmcdespl: (%s %s) - Convert spooled files and send to printer\n", __DATE__, __TIME__ );
            printf( "options:\n\n" );
            printf( "  -d base directory (e.g., /tmp/pmcdespl)\n" );
            printf( "  -p print command (currently %s)\n", gPrintCommand_p );
            printf( "  -c conversion command  (currently %s)\n", gConvertCommand_p );
            printf( "  -s skip print command\n" );
            printf( "  -n skip conversion command\n" );
            printf( "  -a archive spooled file (default is skip)\n\n" );
            printf( "Default commands:\n\n" );
            printf( "  convert: %s\n",   DS_CONVERT_COMMAND );
            printf( "  print:   %s\n", DS_PRINT_COMMAND );
            printf( "  archive: %s\n", DS_ARCHIVE_COMMAND );
            printf( "  latex:   %s\n", DS_LATEX_COMMAND );
            printf( "  dvipdf:  %s\n\n", DS_DVIPDF_COMMAND );
            goto exit;

          default:
            printf( "%s: %c: unknown option\n", DS_PROG_NAME, option );
            break;
        }
    }

    signal( SIGHUP,  dsHandleSignals );
    signal( SIGINT,  dsHandleSignals );
    signal( SIGQUIT, dsHandleSignals );
    signal( SIGILL,  dsHandleSignals );
    signal( SIGTRAP, dsHandleSignals );
    signal( SIGABRT, dsHandleSignals );
    signal( SIGFPE,  dsHandleSignals );
    signal( SIGSEGV, dsHandleSignals );
    signal( SIGPIPE, dsHandleSignals );
    signal( SIGALRM, dsHandleSignals );
    signal( SIGTERM, dsHandleSignals );
    signal( SIGURG,  dsHandleSignals );
    signal( SIGTTIN, dsHandleSignals );
    signal( SIGTTOU, dsHandleSignals );
    signal( SIGIO,   dsHandleSignals );
    signal( SIGBUS,  dsHandleSignals );
    signal( SIGTSTP, dsHandleSignals );

    gStopDetected = FALSE;

    if( gBaseDir_p == NIL )
    {
        dsLog( "No working directory specified\n" );
        goto exit;
    }

    if( chdir( gBaseDir_p ) != 0 )
    {
        dsLog( "Could not change to directory '%s'\n", gBaseDir_p );
        goto exit;
    }

    if( !dsLockFileGet( ) ) goto exit;

    gotLock = TRUE;

    dsLog( "Starting\n" );
    dsLog( "Base directory '%s'\n",     gBaseDir_p );
    dsLog( "Print command '%s'\n",      gPrintCommand_p );
    dsLog( "Convert command '%s'\n",    gConvertCommand_p );
    dsLog( "Archive command '%s'\n",    gArchiveCommand_p );


    /* Start the worker thread */
    pthread_create( &gWorkerThread, 0, (Void_p)dsWorkerThreadEntry, (Void_p)NIL );

    /* Watchdog loop */
    for( i = 0 ; ; i++ )
    {
        sleep( 1 );
        if( gStopDetected == TRUE )
        {
            gWatchdogCounter = 1;
            break;
        }

        if( ++ticks > 60 )
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
        dsLog( "Watchdog counter failed\n" );
        if( gSystemPid )
        {
            dsLog( "must kill pid: %d\n", gSystemPid );
            kill( gSystemPid, SIGKILL );
        }
    }
    else
    {
        dsLog( "Normal stop detected\n" );
    }

exit:

    if( gotLock ) unlink( DS_LOCKFILE_NAME );


    if( gMallocCount )
    {
        dsLog( "Done: gMallocCount: %ld\n", gMallocCount );
    }
    else
    {
        dsLog( "Done\n" );
    }

    return 0;
}

/******************************************************************************
 * Function: dsWorkerThreadEntry( )
 *-----------------------------------------------------------------------------
 * Description:
 *

 *-----------------------------------------------------------------------------
 */

Void_p              dsWorkerThreadEntry
(
    Void_p          args_p
)
{
    Char_t          cmd[MAX_PATH];
    Char_t          buf1[MAX_PATH];
    qHead_t         fileFlagQueueHead;
    qHead_t         fileSpoolQueueHead;
    qHead_p         flag_q;
    qHead_p         spool_q;
    Char_t          flagDir[MAX_PATH];
    Char_t          spoolDir[MAX_PATH];
    Char_t          failedDir[MAX_PATH];
    Char_t          tempDir[MAX_PATH];
    Char_t          debugDir[MAX_PATH];
    Char_t          archiveDir[MAX_PATH];
    Char_t          psDir[MAX_PATH];
    Char_t          pdfDir[MAX_PATH];
    DsFile_p        fileFlag_p;
    DsFile_p        fileSpool_p;
    Boolean_t       match;

    sprintf( flagDir,   "%s/flags",     gBaseDir_p );
    sprintf( spoolDir,  "%s/spool",     gBaseDir_p );
    sprintf( failedDir, "%s/failed",    gBaseDir_p );
    sprintf( tempDir,   "%s/temp",      gBaseDir_p );
    sprintf( debugDir,  "%s/debug",     gBaseDir_p );
    sprintf( archiveDir,"%s/archive",   gBaseDir_p );
    sprintf( psDir,     "%s/ps",        gBaseDir_p );
    sprintf( pdfDir,    "%s/pdf",       gBaseDir_p );

    flag_q  = qInitialize( &fileFlagQueueHead );
    spool_q = qInitialize( &fileSpoolQueueHead );

#if 0
    dsLog( "Calling stuck\n" );
    /* Test watchdog timer */
    sprintf( cmd, "/home/mikeb/projects/pmcdespl/mak/stuck" );
    dsSystem( cmd, TRUE );
    dsLog( "Back from stuck\n" );
#endif

    while( !gStopDetected )
    {
        dsGetFileList( flagDir,  flag_q,  DS_FILE_MODE_REG );
        dsGetFileList( spoolDir, spool_q, DS_FILE_MODE_REG );

        /* Look at the files in the flag directory */
        qWalk( fileFlag_p, flag_q, DsFile_p )
        {
            dsLog( "Found flag file '%s'\n", fileFlag_p->name );
            match = FALSE;

            // Look to see if a corresponding spool file can be found
	        qWalk( fileSpool_p, spool_q, DsFile_p )
            {
                if( dsFileCompare( fileFlag_p->name, fileSpool_p->name ) == 0 )
                {
                    match = TRUE;
                    break;
                }
            }

            if( match == TRUE )
            {
                // Attempt to convert the file to postscript
                dsLog( "Found spooled file '%s'\n", fileFlag_p->name );

                if( gArchiveFlag )
                {
                    sprintf( cmd, "%s %s > %s/%s.gz 2>/dev/null ", gArchiveCommand_p,
                        fileSpool_p->pathName, archiveDir, fileSpool_p->name );
                    dsLog( "Archive command '%s'\n", cmd );
                    dsSystem( cmd, TRUE );
                }

                if( strPos( fileSpool_p->name, ".html" ) >= 0 )
                {
                    dsLog( "Found an HTML file\n" );
                    dsProcessHtml( fileSpool_p, fileFlag_p, tempDir );
                }
                else if( strPos( fileSpool_p->name, ".tex" ) >= 0 )
                {
                    dsLog( "Found a LATEX file\n" );
                    dsProcessLatex(  fileSpool_p, fileFlag_p, pdfDir );
                }
                else if( strPos( fileSpool_p->name, ".ps" ) >= 0  )
                {
                    dsLog( "Found a PostScript file\n" );
                }
                else
                {
                    dsLog( "Found unknown file: '%s'\n", fileSpool_p->name );
                }
            }
            else
            {
                dsLog( "Could not find spool file '%s'\n", fileFlag_p->name );
                sprintf( buf1, "%s/%s", failedDir, fileFlag_p->name );
                if( rename( fileFlag_p->pathName, buf1 ) != 0 )
                {
                    dsLog( "Failed to move flag file '%s' to failed directory, deleting\n", fileFlag_p->name );
                    unlink( fileFlag_p->pathName );
                }
            }
            /* Look for temp files as we process each spooled file so that */
            /* printing will start immediately */
            dsProcessTempFiles( tempDir, debugDir, gPrintCommand_p, gSkipPrintFlag );
        }

        /* Also loolk for temp files here. Do not want to rely upon */
        /* spooled files in order to handle temp files */
        dsProcessTempFiles( tempDir, debugDir, gPrintCommand_p, gSkipPrintFlag );

        gWatchdogCounter++;

        dsFileListFree( flag_q );
        dsFileListFree( spool_q );

        sleep( 1 );
    }

    gStopDetected = TRUE;

    return NIL;
}

/******************************************************************************
 * Function: dsProcessLatex( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Int32s_t dsProcessLatex
(
    DsFile_p    fileSpool_p,
    DsFile_p    fileFlag_p,
    Char_p      pdfDir_p
)
{
    Char_t          cmd[MAX_PATH];
    Char_t          fileTex[MAX_PATH];
    Char_t          fileDvi[MAX_PATH];
    Char_t          filePdf[MAX_PATH];
    Int32s_t        pos;

    qHead_t         fileQueueHead;
    qHead_p         file_q;
    DsFile_p        file_p;

    file_q = qInitialize( &fileQueueHead );

    /* Going to work in the ps directory */
    if( chdir( pdfDir_p ) != 0 )
    {
        dsLog( "Could not change to directory '%s'\n", pdfDir_p );
        goto exit;
    }

    dsGetFileList( pdfDir_p, file_q, DS_FILE_MODE_REG );

    /* This loop cleans out all cruft from the directory */
    while( !qEmpty( file_q ) )
    {
        file_p = (DsFile_p)qRemoveFirst( file_q );

        /* dsLog( "Found file: '%s'\n", file_p->pathName ); */

        if( strPos( file_p->name, ".pdf" ) < 0 )
        {
            /* dsLog( "Want to delete file '%s'\n", file_p->pathName ); */
            unlink( file_p->pathName );
        }
        else
        {
            /* dsLog( "Want to keep file '%s'\n", file_p->pathName ); */
        }

        dsFileListFreeEntry( file_p );
    }

    sprintf( fileTex, "%s/%s", pdfDir_p, fileFlag_p->name );

    strcpy( fileDvi, fileTex );
    strcpy( filePdf, fileTex );

    pos = strPos( fileDvi, ".tex" );
    if( pos < 0 )
    {
        dsLog( "Failed to find .tex extension\n" );
    }

    fileDvi[pos] = 0;
    filePdf[pos] = 0;
    strcat( fileDvi, ".dvi" );
    strcat( filePdf, ".pdf" );

    /* dsLog( "DVI file name: '%s'\n", fileDvi ); */
    /* dsLog( "PDF file name: '%s'\n", filePdf ); */

    /* Move file from spool dir to pdf dir */
    rename( fileSpool_p->pathName, fileTex );

    sprintf( cmd, "%s %s > latex.log 2> latex2.log", DS_LATEX_COMMAND, fileTex );
    dsLog( "%s\n", cmd );

    dsSystem( cmd, TRUE );
    dsSystem( cmd, TRUE );

    /* For dvipdf */
    // sprintf( cmd, "%s %s %s.TEMP > dvipdf.log 2> dvipdf2.log", DS_DVIPDF_COMMAND, fileDvi, filePdf );

    /* For dvipdfm */
    sprintf( cmd, "%s -o %s.TEMP %s > dvipdf.log 2> dvipdf2.log", DS_DVIPDF_COMMAND, filePdf, fileDvi );
    dsLog( "%s\n", cmd );
    dsSystem( cmd, TRUE );

    sprintf( cmd, "%s.TEMP", filePdf );
    rename( cmd, filePdf );

    dsLog( "Latex -> PDF conversion complete\n" );

    /* Get rid of the input files */
    unlink( fileFlag_p->pathName );

exit:

    if( chdir( gBaseDir_p ) != 0 )
    {
        dsLog( "Could not change to directory '%s'\n", gBaseDir_p );
    }

    return TRUE;
}


/******************************************************************************
 * Function: dsProcessTempFiles( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Int32s_t dsProcessHtml
(
    DsFile_p    fileSpool_p,
    DsFile_p    fileFlag_p,
    Char_p      tempDir_p
)
{
    Char_t          buf1[MAX_PATH];
    Char_t          buf2[MAX_PATH];

    /* Get all files in the conversion directory, and delete */


    sprintf( buf2, "%s/%s.cnv", tempDir_p, fileFlag_p->name );

    if( gSkipConvertFlag )
    {
        dsLog( "Skipping conversion, move file to '%s'\n", buf2 );
        //Move spooled file to the temp directory
        rename( fileSpool_p->pathName, buf2 );
    }
    else
    {
        sprintf( buf2, "%s/%s.cnv", tempDir_p, fileFlag_p->name );
        sprintf( buf1, "%s -o %s %s\n", gConvertCommand_p, buf2, fileSpool_p->pathName );

        dsLog( "Convert command: '%s'", buf1 );

        dsSystem( buf1, TRUE );
        unlink( fileSpool_p->pathName );
    }

    /* Get rid of the input files */
    unlink( fileFlag_p->pathName );

    return TRUE;
 }

/******************************************************************************
 * Function: dsProcessTempFiles( )
 *-----------------------------------------------------------------------------
 * Description:
 *

 *-----------------------------------------------------------------------------
 */

Void_t dsProcessTempFiles
(
    Char_p    tempDir_p,
    Char_p    debugDir_p,
    Char_p    printCommand_p,
    Int32u_t  skipPrint
)
{
    Char_t          buf[MAX_PATH];
    Char_t          cmd[MAX_PATH];
    DsFile_p        fileTemp_p;
    qHead_t         fileTempQueueHead;
    qHead_p         fileTempQueueHead_p;

    fileTempQueueHead_p =  qInitialize( &fileTempQueueHead );

    dsGetFileList( tempDir_p, fileTempQueueHead_p, DS_FILE_MODE_REG );

    qWalk( fileTemp_p, fileTempQueueHead_p, DsFile_p )
    {
        dsLog( "Found temp file '%s'\n", fileTemp_p->name );
        if( skipPrint )
        {
            /* move postscript file to a debug directory */
            sprintf( buf, "%s/%s", debugDir_p, fileTemp_p->name );

            dsLog( "Skipping print, move '%s' to debug dir\n", fileTemp_p->name );
            if( rename( fileTemp_p->pathName, buf ) != 0 )
            {
                dsLog( "Failed to move temp file '%s' to debug directory, deleting\n", fileTemp_p->name );
                unlink( fileTemp_p->pathName );
            }
        }
        else
        {
            sprintf( cmd, "%s %s\n", printCommand_p, fileTemp_p->pathName );
            dsLog( "Print command: '%s'", cmd );
            dsSystem( cmd, TRUE );
            unlink( fileTemp_p->pathName );
        }
    } /* End of for loop looking for temp files */

    /* Empty queue before returning */

    for( ; ; )
    {
        if( qEmpty( fileTempQueueHead_p ) ) break;

        fileTemp_p = (DsFile_p)qRemoveFirst( fileTempQueueHead_p );
        free( fileTemp_p );
    }

    return;
}

/******************************************************************************
 * Function: dsFileCompare( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns the starting position of substring i2 within i1.
 *-----------------------------------------------------------------------------
 */

Int32s_t        dsFileCompare
(
    Char_p      fileName1_p,
    Char_p      fileName2_p
)
{
    if( strlen( fileName1_p ) != strlen( fileName2_p ) ) return -1;

    return strPos( fileName1_p, fileName2_p );
}

/******************************************************************************
 * Function: strGetFileList( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns the starting position of substring i2 within i1.
 *-----------------------------------------------------------------------------
 */

Int32s_t dsGetFileList
(
    Char_p      dir_p,
    qHead_p     q_p,
    Ints_t      mode
)
{
    struct dirent **namelist_pp = 0;
    int             n;
    struct stat     buf;
    char            filename[MAX_PATH];
    DsFile_p        file_p;

#undef  MN
#define MN "dsGetFileList"

    /* First, clear out any files that may be in the list */
    for( ; ; )
    {
        if( qEmpty( q_p ) ) break;
        file_p = (DsFile_p)qRemoveFirst( q_p );
        free( file_p );
    }

    n = scandir( dir_p, &namelist_pp, 0, 0 );
    if( n < 0 )
    {
        dsLog( "scandir( %s ) failed: %d\n", dir_p, n );
        goto exit;
    }
    else
    {
        /* Step through all the entries */
        while( n-- )
        {
            /* debugging printf( "namelist[n]: 0x%08lX\n", (Int32u_t)&namelist[n] ); */

            sprintf( filename, "%s/%s", dir_p, namelist_pp[n]->d_name );

            if( stat( filename, &buf ) != 0 )
            {
                /* Error */
                dsLog( "stat( %s ) returned error\n", filename );
            }
            else
            {
                if( ( mode == DS_FILE_MODE_ALL ) ||
                    ( mode == DS_FILE_MODE_REG && S_ISREG( buf.st_mode ) ) )
                {
                    file_p = mbMalloc( sizeof( DsFile_t ) );
                    /* Perhaps the following two lines should be copies, not sprintfs */
                    /* sprintf( file_p->name, namelist_pp[n]->d_name ); */
                    strncpy( file_p->name, namelist_pp[n]->d_name, MAX_PATH - 1);
                    /* sprintf( file_p->path, dir_p ); */
                    strncpy( file_p->path, dir_p, MAX_PATH -1 );
                    sprintf( file_p->pathName, "%s/%s", dir_p, namelist_pp[n]->d_name );
                    qInsertFirst( q_p, file_p );
                }
            }

            if( namelist_pp[n] )
            {
                free( namelist_pp[n] );
            }
            else
            {
                dsLog( "Got NIL namelist[%d]", n );
            }
        }
    }

    if( namelist_pp )
    {
        free( namelist_pp );
    }
    else
    {
        dsLog( "Got NIL namelist\n" );
    }

exit:
    return 0;
}


/******************************************************************************
 * Function: strPos( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns the starting position of substring i2 within i1.
 *-----------------------------------------------------------------------------
 */

Int32s_t strPos
(
    Char_p  s1,
    Char_p  s2
)
{
    /* Check if s2 is anywhere within s1 */
    Int32s_t i1, i2, i;

#undef  MN
#define MN "strPos"

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

    for( i=0; i <= i1; i++, s1++ )
    {
        if( strncmp( s1, s2, (size_t)i2 ) == 0 )
        {
            /* string is found, return starting position */
            return i;
        }
    }
    /* Not Found */

    return -1;
}

/******************************************************************************
 * Function: dsHandleSignals( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns the starting position of substring i2 within i1.
 *-----------------------------------------------------------------------------
 */


Void_t      dsHandleSignals
(
    Ints_t  sig
)
{
    if( sig == SIGHUP     ) dsLog( "Got SIGHUP    \n" );
    if( sig == SIGINT     ) dsLog( "Got SIGINT    \n" );
    if( sig == SIGQUIT    ) dsLog( "Got SIGQUIT   \n" );
    if( sig == SIGILL     ) dsLog( "Got SIGILL    \n" );
    if( sig == SIGTRAP    ) dsLog( "Got SIGTRAP   \n" );
    if( sig == SIGABRT    ) dsLog( "Got SIGABRT   \n" );
    if( sig == SIGFPE     ) dsLog( "Got SIGFPE    \n" );
    if( sig == SIGSEGV    ) dsLog( "Got SIGSEGV   \n" );
    if( sig == SIGPIPE    ) dsLog( "Got SIGPIPE   \n" );
    if( sig == SIGALRM    ) dsLog( "Got SIGALRM   \n" );
    if( sig == SIGTERM    ) dsLog( "Got SIGTERM   \n" );
    if( sig == SIGURG     ) dsLog( "Got SIGURG    \n" );
    if( sig == SIGTTIN    ) dsLog( "Got SIGTTIN   \n" );
    if( sig == SIGTTOU    ) dsLog( "Got SIGTTOU   \n" );
    if( sig == SIGIO      ) dsLog( "Got SIGIO     \n" );
    if( sig == SIGBUS     ) dsLog( "Got SIGBUS    \n" );
    if( sig == SIGTSTP    ) dsLog( "Got SIGTSTP   \n" );

    gStopDetected = TRUE;

    return;
}

/******************************************************************************
 * Function: dsLog
 *-----------------------------------------------------------------------------
 * Description:
 *
 *
 *-----------------------------------------------------------------------------
 */

Ints_t dsLog
(
    Char_p  string_p,
    ...
)
{
    va_list             arg;
    Char_p              str_p = NULL;
    Char_t              timeStr[64];
    Char_t              dateStr[64];
    time_t              ptime;
    Char_t              logFileName[MAX_PATH];
    FILE               *fp;

    str_p = (Char_p)mbMalloc( 1024 );
    if( str_p == NULL ) return 0;

    va_start( arg, string_p );
    vsprintf( str_p, string_p, arg );
    va_end( arg );

    ptime = time( (long *) 0 );
    strcpy( timeStr, dsCharTime( &ptime ) );
    strcpy( dateStr, dsCharDate( &ptime ) );

    sprintf( logFileName, "%s/log/%s-%s.log", gBaseDir_p, dateStr, DS_PROG_NAME );
    fp = fopen( logFileName, "a" );
    if( fp )
    {
        fprintf( fp, "%s: %lu: %s", timeStr, gMyPid, str_p );
        fclose( fp );
    }

    fprintf( stdout, "%s: %lu: %s", timeStr, gMyPid, str_p );

    mbFree( str_p );
    return 0;
}

/******************************************************************************
 * Function: dsCharTime
 *-----------------------------------------------------------------------------
 * Description:
 *
 *
 *-----------------------------------------------------------------------------
 */

Char_p dsCharTime( time_t *tm )
{
    static Char_t   buf[100];
    struct tm *tm_p; tm_p = localtime( tm );

    sprintf( buf, "%02d:%02d:%02d", tm_p->tm_hour, tm_p->tm_min,tm_p->tm_sec );
    return &buf[0];
}

/******************************************************************************
 * Function: dsCharDate
 *-----------------------------------------------------------------------------
 * Description:
 *
 *
 *-----------------------------------------------------------------------------
 */

Char_p dsCharDate( time_t *tm )
{
    static Char_t   buf[100];
    struct tm *tm_p; tm_p = localtime( tm );

    sprintf( buf, "%04d-%02d-%02d", 1900 + tm_p->tm_year ,tm_p->tm_mon+1, tm_p->tm_mday );

    return &buf[0];
}

/******************************************************************************
 * Function: egLockFileGet
 *-----------------------------------------------------------------------------
 * Description:
 *
 *
 *-----------------------------------------------------------------------------
 */

Int32s_t    dsLockFileGet( void )
{
    Int32s_t        returnCode = FALSE;
    Boolean_t       foundPidFile = FALSE;
    Boolean_t       foundPid = FALSE;
    FILE           *fp;
    FILE           *fp2;
    qHead_t         fileQueue;
    qHead_p         file_q;
    Char_p          buf1_p = mbMalloc( 1024 );
    Char_p          buf2_p = mbMalloc( 1024 );
    DsFile_p        file_p;

    file_q = qInitialize( &fileQueue );

     /* Get pid from lockfile  */
    fp = fopen( DS_LOCKFILE_NAME, "r" );
    if( fp )
    {
        fgets( buf1_p, 512, fp );

        /* There is a lockfile, get pid from it */
        dsGetFileList( "/proc", file_q, DS_FILE_MODE_ALL );

        qWalk( file_p, file_q, DsFile_p )
        {
            {
                if( dsFileCompare( buf1_p, file_p->name ) == 0 )
                {
                    foundPidFile = TRUE;
                    sprintf( buf2_p, "/proc/%s/cmdline", buf1_p );
                    fp2 = fopen( buf2_p, "r" );
                    if( fp2 )
                    {
                        fgets( buf2_p, 512, fp2 );
                        if( strPos( buf2_p, DS_PROG_NAME ) >= 0 )
                        {
                            foundPid = TRUE;
                            break;
                        }
                        fclose( fp2 );
                    }
                    else
                    {
                        dsLog( "Failed to open '%s' for reading\n", buf2_p );
                    }
                    if( foundPidFile == TRUE ) break;
                }
            }
        }

        fclose( fp );
        dsFileListFree( file_q );

        if( foundPidFile == FALSE )
        {
            /* Stale lock file? */
        }
        else
        {
            if( foundPid == FALSE )
            {
                dsLog( "Found /proc/pid file but not pid '%s', terminating\n", buf1_p );
                goto exit;
            }
        }

        if( foundPid == TRUE )
        {
            dsLog( "Found process running with pid '%s', terminating\n", buf1_p );
            goto exit;
        }
        else
        {
            dsLog( "Deleting stale lockfile with pid '%s' and running\n", buf1_p );
            unlink( DS_LOCKFILE_NAME );
        }
    }

    /* At this point we are going to run and should create a pid file */
    fp = fopen( DS_LOCKFILE_NAME, "w" );
    if( fp == NIL )
    {
        dsLog( "Failed to create lockfile\n" );
        goto exit;
    }
    fprintf( fp, "%ld", gMyPid );
    fclose( fp );

    returnCode = TRUE;

exit:

    mbFree( buf1_p );
    mbFree( buf2_p );

    return returnCode;
}

/******************************************************************************
 * Function: ( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */
Char_p      mbMallocStr( Char_p string_p )
{
    Char_p  return_p = NIL;
    Ints_t  len;

    if( string_p )
    {
        len = strlen( string_p );
        if( ( return_p = malloc( len + 1 ) ) )
        {
            strcpy( return_p, string_p );
            gMallocCount++;
        }
    }
    return return_p;
}

Void_p mbMalloc( int size )
{
    Void_p  return_p = malloc( size );

    if( return_p ) gMallocCount++;

    return return_p;
}

Void_p mbCalloc( int size )
{
    Void_p  return_p = calloc( size, 1 );

    if( return_p ) gMallocCount++;

    return return_p;
}

Void_t  mbFreeFunc( Void_p ptr_p )
{
    if( ptr_p != NIL )
    {
        free( ptr_p );
        gMallocCount--;
    }
}

/******************************************************************************
 * Function: dsFileListFree( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns the starting position of substring i2 within i1.
 *-----------------------------------------------------------------------------
 */

Int32s_t dsFileListFree( qHead_p file_q )
{
    DsFile_p    file_p;
    Int32s_t    returnCode = FALSE;

    if( file_q == NIL ) goto exit;
    while( !qEmpty( file_q ) )
    {
        file_p = (DsFile_p)qRemoveFirst( file_q );
        dsFileListFreeEntry( file_p );
    }
    returnCode = TRUE;
exit:
    return returnCode;
}

/******************************************************************************
 * Function: egFileListFreeEntry( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Int32s_t dsFileListFreeEntry( DsFile_p file_p )
{
    Int32s_t    returnCode = FALSE;

    if( file_p == NIL ) goto exit;

//    mbFree( file_p->name_p );
//    mbFree( file_p->path_p );
//    mbFree( file_p->pathName_p );
//    mbFree( file_p->origName_p );
    mbFree( file_p );

    returnCode = TRUE;

exit:
    return returnCode;
}

/******************************************************************************
 * Function: ( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

int dsSystem( char *cmd_p, int pidFlag )
{
    int pid, status;

    if( cmd_p == NIL ) return 1;

    pid = fork();

    if( pid == -1 ) return -1;

    /* egLog( "Got PID: %d\n", pid ); */

    if( pid == 0 )
    {
        char *argv[4];

        argv[0] = "sh";
        argv[1] = "-c";
        argv[2] = cmd_p;
        argv[3] = 0;
        //execve("/bin/sh", argv, environ );

        execve( "/bin/sh", argv, 0 );
        exit(127);
    }

    if( pidFlag )
    {
        gSystemPid = pid;
        /* egLog( "gSystemPid: %d\n", gSystemPid ); */
    }

    do
    {
        if( waitpid( pid, &status, 0 ) == -1 )
        {
            if( errno != EINTR )
            {
                gSystemPid = 0;
                return -1;
            }
        }
        else
        {
            gSystemPid = 0;
            return status;
        }
    }
    while(1);
}
