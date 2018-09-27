/******************************************************************************
 * File: echoGetMain.c
 *-----------------------------------------------------------------------------
 * Copyright (c) 2003, Michael A. Bree
 *-----------------------------------------------------------------------------
 * Date: January 27, 2003
 *-----------------------------------------------------------------------------
 * Description:
 *
 * Get echo files from MySQL database.
 *-----------------------------------------------------------------------------
 * $Header: /home/cvs/echoget/src/echoGetMain.c,v 1.25 2007/07/14 14:53:03 mikeb Exp $
 *-----------------------------------------------------------------------------
 * $Log: echoGetMain.c,v $
 * Revision 1.25  2007/07/14 14:53:03  mikeb
 * Add Log to header
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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <mysql/mysql.h>

/* Project include files */
#include "mbTypes.h"
#include "mbQueue.h"
#include "mbLock.h"
#include "echoGetMain.h"

#define INIT_GLOBALS
#include "echoGetGlobals.h"
#undef  INIT_GLOBALS

extern char *optarg;

Int32s_t    gMallocCount;
pthread_t   gWorkerThread;

int egSystem( char *cmd_p, Boolean_t pidFlag );

Int32s_t    egDocumentsGetRemote( qHead_p remote_q );
Int32s_t    egDocumentsGet( Boolean_t  verifyFlag );
Boolean_t   egEchoDirNameValid( Char_p name_p );

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
    Char_p          dirName_p = NIL;
    Char_p          port_p = "3306";
    Ints_t          gotLock = FALSE;
    Ints_t          helpFlag = FALSE;
    Ints_t          i;
    Int32u_t        ticks;

    /* Initialize global variables */
    gMallocCount = 0;
    gLogLock_p = MB_LOCK_INIT( gLogLock );
    gBuf_p  = mbMalloc( 2048 );
    gLocalDir_p = &gLocalDir[0];
    strcpy( gLocalDir_p, EG_LOCAL_DIR );
    sprintf( gRemoteMachine, "%s", EG_REMOTE_MACHINE_DEFAULT );

    gHost_p = "127.0.0.1";
    gPort = 3306;

    gMySQL_p = NIL;
    gSilentNew = FALSE;
    gMaxAge = 0;
    gStopDetected = FALSE;
    gCheckLocalEchos = FALSE;
    gMoveBadFlag    = FALSE;
    gMoveEmptyFlag  = FALSE;
    gSetBackupFlag  = FALSE;
    gSetOnlineFlag  = FALSE;
    gGetDocs = FALSE;
    gCheckDocs = FALSE;
    gEchoID = 0;
    gWatchdogCounter = 0;
    gSystemPid = 0;

    /* Get my pid */
    gMyPid = getpid();

    while( ( option = getopt( argc, argv, "a:l:Bm:becd:h:i:sp:v?r:DCo" )) != -1 )
    {
        optCount++;

        switch( option )
        {
          case 'D':
            gGetDocs = TRUE;
            break;

          case 'C':
            gCheckDocs = TRUE;
            break;

          case 'a':
            gMaxAge = atoi( optarg );
            break;

          case 'l':
            strcpy( gLocalDir_p, optarg );
            break;

          case  'r':
            strcpy( gRemoteMachine, optarg );
            break;

          case 'B':
            gSetBackupFlag = TRUE;
            break;

          case 'o':
            gSetOnlineFlag = TRUE;
            break;

          case 'm':
            gMaxEchosToGet = atoi( optarg );
            break;

          case 'c':
            gCheckLocalEchos = TRUE;
            break;

          case 'b':
            gMoveBadFlag = TRUE;
            break;

          case 'e':
            gMoveEmptyFlag = TRUE;
            break;

          case 'd':
            dirName_p = optarg;
            break;

          case 'h':
            gHost_p = optarg;
            break;

          case 'i':
            gEchoID = atoi( optarg );
            break;

          case 's':
            gSilentNew = TRUE;
            break;

          case 'p':
            port_p = optarg;
            break;

          case '?':
          case 'v':
            helpFlag = TRUE;
            break;

          default:
            printf( "%s: %c: unknown option\n", EG_PROG_NAME, option );
            helpFlag = TRUE;
            break;
        }
    }

    if( helpFlag )
    {
        printf( "\nechoget: (%s %s) - Get echo files from MySQL database\n", __DATE__, __TIME__ );
        printf( "\noptions:\n\n" );
        printf( "  -a               max get age (format: 20001231)\n" );
        printf( "  -b               move bad echos\n" );
        printf( "  -B               mark verifed echos backed up\n" );
        printf( "  -o               mark verifed echos online\n" );
        printf( "  -c               check local echos\n" );
        printf( "  -d dir_name      log and lockfile directory\n" );
        printf( "  -e               move empty echos\n" );
        printf( "  -h host          MySQL host name\n" );
        printf( "  -i ID            ID of echo to get/verify (default: most recent)\n" );
        printf( "  -l dir_name      local echo directory\n" );
        printf( "  -m               max echos to get\n" );
        printf( "  -p port          MySQL port\n" );
        printf( "  -r host          remote (scp) machine\n" );
        printf( "  -D               get documents as well as echos\n" );
        printf( "  -C               check documents\n" );
        printf( "  -s               silent operation\n" );

        printf( "\nCompiled in options and defaults:\n\n" );
        printf( "   remote machine  %s\n", gRemoteMachine );
        printf( "   remote dir      %s\n", EG_REMOTE_DIR     );
        printf( "   local dir       %s\n", gLocalDir_p       );
        printf( "   remote doc dir  %s\n", EG_REMOTE_DOC_DIR );
        printf( "   local doc dir   %s\n", EG_LOCAL_DOC_DIR  );
        printf( "   scp cmd         %s\n", EG_SCP_CMD        );
        printf( "   rm cmd          %s\n", EG_RM_CMD         );
        printf( "   mv cmd          %s\n", EG_MV_CMD         );
        printf( "   mkdir cmd       %s\n", EG_MKDIR_CMD      );
        printf( "\n" );
        printf( "MySQL database table and field names are also hardcoded into this program\n\n" );
        goto exit;
    }

    mbStrTrailingSlashRemove( gLocalDir_p );
    strcat( gLocalDir_p, "/" );

    if( gCheckDocs == TRUE )
    {
        gGetDocs = TRUE;
    }

    if( dirName_p  )
    {
        gBaseDir_p = dirName_p;

        if( chdir( dirName_p ) != 0 )
        {
            printf( "Could not change to directory '%s'\n", dirName_p );
            goto exit;
        }
    }
    else
    {
        gBaseDir_p = NIL;
    }

    if( gHost_p == NIL )
    {
        egLog( "MySQL host not specified.\n" );
        goto exit;
    }

    if( port_p == NIL )
    {
        egLog( "MySQL port not specified.\n" );
        goto exit;
    }
    gPort = atoi( port_p );

    if( egStopCheck() == TRUE ) goto exit;

    if( !egLockFileGet( ) ) goto exit;

    gotLock = TRUE;

#if 0
    signal( SIGHUP,  egHandleSignals );
    signal( SIGINT,  egHandleSignals );
    signal( SIGQUIT, egHandleSignals );
    signal( SIGILL,  egHandleSignals );
    signal( SIGTRAP, egHandleSignals );
    signal( SIGABRT, egHandleSignals );
    signal( SIGFPE,  egHandleSignals );
    signal( SIGSEGV, egHandleSignals );
    signal( SIGPIPE, egHandleSignals );
    signal( SIGALRM, egHandleSignals );
    signal( SIGTERM, egHandleSignals );
    signal( SIGURG,  egHandleSignals );
    signal( SIGTTIN, egHandleSignals );
    signal( SIGTTOU, egHandleSignals );
    signal( SIGIO,   egHandleSignals );
    signal( SIGBUS,  egHandleSignals );
    signal( SIGTSTP, egHandleSignals );

    dsTerminate = FALSE;
#endif

    egCrcInit( );
    egPurgeTemp( );



    /* Start the worker thread */
    pthread_create( &gWorkerThread, 0, (Void_p)egWorkerThreadEntry, (Void_p)NIL );

    /* Watchdog loop */
    for( i = 0 ; ; i++ )
    {
        sleep( 1 );

        // egLog( "Watchdog thread awake: watchdog counter: %u\n", gWatchdogCounter );
        if( gStopDetected == TRUE )
        {
            gWatchdogCounter = 1;
            break;
        }
        if( ++ticks > 300 )
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
        egLog( "Watchdog counter failed\n" );
        if( gSystemPid )
        {
            egLog( "must kill pid: %d\n", gSystemPid );
            kill( gSystemPid, SIGKILL );
        }
    }
    else
    {
        egLog( "Normal stop detected\n" );
    }

//    pthread_kill_other_threads_np( );

exit:

    if( gMySQL_p ) mysql_close( gMySQL_p );

    mbFree( gBuf_p );

    if( gotLock ) unlink( EG_LOCKFILE_NAME );

    if( gMallocCount )
    {
        egLog( "Done: gMallocCount: %ld\n", gMallocCount );
    }
    else
    {
        egLog( "Done\n" );
    }

    return 0;
}

/******************************************************************************
 * Function: egWorkerThreadEntry( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */
Void_p              egWorkerThreadEntry
(
    Void_p          args_p
)
{
    qHead_t         remoteQueue;
    qHead_p         remote_q;
    qHead_t         localQueue;
    qHead_p         local_q;
    qHead_t         tempQueue;
    qHead_p         temp_q;
    Int32u_t        i, size;
    EgEcho_p        remoteEcho_p;
    EgEcho_p        localEcho_p;
    Boolean_t       found;
    Int32u_t        gotEchosCount = 0;


    /* Connect to mysql database */
    if( ( gMySQL_p = mysql_init( NULL ) ) == NIL )
    {
        egLog( "MySQL initialization failed\n" );
        goto exit;
    }

    // egLog( "mysqlPort: %u\n", mysqlPort );

    if( mysql_real_connect( gMySQL_p,
                            gHost_p,
                            "mikeb",
                            "mikebsql",
                            "clinic_active",
                            gPort, NULL, 0 ) == NIL )
    {
        egLog( "Connection to mysql database failed\n" );
        goto exit;
    }


    remote_q  = qInitialize( &remoteQueue );
    local_q   = qInitialize( &localQueue );
    temp_q    = qInitialize( &tempQueue );

    if( gGetDocs )
    {
        egDocumentsGet( gCheckLocalEchos );
    }

    if( egStopCheck() == TRUE ) goto exit;

    if( gCheckDocs == TRUE ) goto exit;

    egLog( "Scanning local echos...\n" );

    /* Get local echos */
    if( egEchoQueueLocalGet( local_q ) != TRUE )
    {
        egLog( "Error getting 'my' echo list\n" );
        goto exit;
    }

    egLog( "Local echos detected: %u\n", local_q->size );

#if 0
    /* Test... see if I can kill this process from watchdog */
    {
        Char_t  buf[512];

        sprintf( buf, "./stuck" );
        egLog( "calling '%s'\n", buf );
        egSystem( buf, TRUE );
        egLog( "back from stuck\n" );
    }
#endif


    if( gCheckLocalEchos == TRUE )
    {
        EgEcho_p    echo_p;

        found = FALSE;
        while( !qEmpty( local_q ) )
        {
            echo_p = (EgEcho_p)qRemoveLast( local_q );

            if( gEchoID == 0 || gEchoID == echo_p->ID )
            {
                found = TRUE;
                egEchoVerify( echo_p, TRUE );
            }
            egEchoQueueFreeEntry( echo_p );

            if( egStopCheck() == TRUE ) break;
        }

        if( gEchoID != 0 && found == FALSE )
        {
            egLog( "Echo ID %u: Not found on local disk\n", gEchoID );
        }

        goto exit;
    }

    if( egEchoQueueRemoteGet( remote_q ) != TRUE )
    {
        egLog( "Error getting remote echo list\n" );
        goto exit;
    }

    egLog( "Remote echos detected: %u\n", remote_q->size );

    /* Loop looking for new unread echos */
    size = remote_q->size;

    for( i = 0 ; i < size ; i++ )
    {
        remoteEcho_p = (EgEcho_p)qRemoveLast( remote_q );

        found = FALSE;

        qWalk( localEcho_p, local_q, EgEcho_p )
        {
 //           egLog( "compare remote ID %u to local ID %u\n", remoteEcho_p->ID, localEcho_p->ID );
            if( remoteEcho_p->ID == localEcho_p->ID )
            {
                found = TRUE;
                break;
            }
        }

        if( found )
        {
            egEchoQueueFreeEntry( remoteEcho_p );
            qRemoveEntry( local_q, localEcho_p );
            qInsertLast( temp_q, localEcho_p );
        }
        else
        {
            // egLog( "Did not match ID %u\n", remoteEcho_p->ID );
            qInsertFirst( remote_q, remoteEcho_p );
        }
        if( egStopCheck() == TRUE ) break;
    }

    if( egStopCheck() == TRUE ) goto exit;

    qWalk( localEcho_p, local_q, EgEcho_p )
    {
        egLog( "Local echo ID %u: Did not detect remote echo\n", localEcho_p->ID );
    }

    while( !qEmpty( temp_q ) )
    {
       localEcho_p = (EgEcho_p)qRemoveLast( temp_q );
       qInsertFirst( local_q, localEcho_p );
    }

    egLog( "Remote echos to process: %u\n", remote_q->size );

    /* Loop looking for new unread echos */
    for( ; ; )
    {
        if( qEmpty( remote_q ) )
        {
            egLog( "No more echos to get\n" );
            break;
        }

        remoteEcho_p = (EgEcho_p)qRemoveLast( remote_q );

        found = FALSE;

        qWalk( localEcho_p, local_q, EgEcho_p )
        {
            if( remoteEcho_p->ID == localEcho_p->ID )
            {
                found = TRUE;
                break;
            }
        }

        if( !found )
        {
            if( gEchoID == 0 || gEchoID == remoteEcho_p->ID )
            {
                if( egEchoGet( remoteEcho_p ) != TRUE )
                {
                    /*Failed to get this echo... move on to next echo */
                    egLog( "Echo ID %u: Get of remote echo failed\n", remoteEcho_p->ID );

                }
                else
                {
                    gotEchosCount++;
                }

                if( --gMaxEchosToGet <= 0 )
                {
                    egLog( "Got maximum allowable echos\n" );
                    egEchoQueueFreeEntry( remoteEcho_p );
                    break;
                }
            }
        }

        egEchoQueueFreeEntry( remoteEcho_p );
        if( egStopCheck() == TRUE ) break;
    }


exit:

    if( gotEchosCount > 0 )
    {
        egLog( "Got %lu echos\n", gotEchosCount );
    }

    egEchoQueueFree( remote_q );
    egEchoQueueFree( local_q );

    gStopDetected = TRUE;

    return NIL;
}

/******************************************************************************
 * Function:
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */
Char_p  egDocumentDirFromName( Char_p path_p, Char_p name_p, Char_p subDir_p  )
{
    static Char_t   dir[1024];
    Char_t          total[1024];

    if( name_p == NIL ) return "";

    strcpy( dir, name_p );

    if( dir[8] != '-' ) return "";

    dir[6] = 0;

    strcpy( subDir_p, &dir[0] );

    sprintf( total, "%s/%s", path_p, &dir[0] );

    /* Blindly attempt to create directory */
    sprintf( gBuf_p, "%s %s >/dev/null 2>/dev/null\n", EG_MKDIR_CMD, total );
    egSystem( gBuf_p, TRUE );

    sprintf( gBuf_p, "%s %s >/dev/null 2>/dev/null\n", EG_CHMOD_CMD, total );
    egSystem( gBuf_p, TRUE );

    return subDir_p;
}

/******************************************************************************
 * Function:
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */
Int32s_t egDocumentsGet( verifyFlag )
{
    qHead_p     remote_q;
    qHead_t     remoteQueue;
    qHead_t     localQueue;
    qHead_p     local_q;
    qHead_t     localDirQueue;
    qHead_p     localDir_q;
    qHead_t     localTempQueue;
    qHead_p     localTemp_q;
    EgFile_p    localFile_p;
    EgFile_p    remoteFile_p;
    Char_t      source[1024];
    Char_t      temp[1024];
    Char_t      target[1024];
    Char_t      subDir[64];
    Boolean_t   found;
    Boolean_t   alreadyCountedBad;
    Int32u_t    crc;
    Int32u_t    verifyGoodCount = 0;
    Int32u_t    verifyBadCount = 0;

    remote_q    = qInitialize( &remoteQueue );
    local_q     = qInitialize( &localQueue );
    localDir_q  = qInitialize( &localDirQueue );
    localTemp_q = qInitialize( &localTempQueue );

    egLog( "Scanning remote documents...\n" );
    egDocumentsGetRemote( remote_q );
    egLog( "Detected %lu remote documents\n", remote_q->size );


    egLog( "Scanning local documents...\n" );
    egFileListGet( EG_LOCAL_DOC_DIR, localDir_q, EG_FILE_MODE_ALL );

    qWalk( localFile_p, localDir_q, EgFile_p )
    {
        if( !egEchoDirNameValid( localFile_p->name_p ) )
        {
            // egLog( "document DIR not valid: '%s'\n", localFile_p->pathName_p );
            continue;
        }
        // egLog( "Got document DIR: '%s'\n", localFile_p->pathName_p );

        egFileListGet( localFile_p->pathName_p, localTemp_q, EG_FILE_MODE_REG );

        while(!qEmpty( localTemp_q ) )
        {
            remoteFile_p = (EgFile_p)qRemoveFirst( localTemp_q );
            qInsertLast( local_q, remoteFile_p );
        }
    }

    egLog( "Detected %lu local documents\n", local_q->size );

    qWalk( remoteFile_p, remote_q, EgFile_p )
    {
        found = FALSE;
        qWalk( localFile_p, local_q, EgFile_p )
        {
            if( strcmp( localFile_p->name_p, remoteFile_p->name_p ) == 0 )
            {
                found = TRUE;
                localFile_p->found = TRUE;
                break;
            }
        }

        if( gCheckDocs )
        {
            alreadyCountedBad = FALSE;
            if( found == TRUE )
            {
                crc = egCrcFile( localFile_p->pathName_p, NIL );
                if( crc == remoteFile_p->crc )
                {
                    verifyGoodCount++;
                    /* egLog( "Existing File '%s': verified OK\n", localFile_p->pathName_p ); */
                }
                else
                {
                    egLog( "Existing File '%s': verified FAILED\n", localFile_p->pathName_p );
                    unlink( localFile_p->pathName_p );
                    found = FALSE;
                    verifyBadCount++;
                    alreadyCountedBad = TRUE;
                }
            }
            else
            {
               egLog( "Don't have document: '%s'\n", remoteFile_p->name_p );
            }
        }

        if( found == FALSE && gCheckDocs == FALSE )
        {
             egLog( "Must get file '%s'\n", remoteFile_p->name_p );

            egDocumentDirFromName( EG_LOCAL_DOC_DIR, remoteFile_p->name_p, subDir );

            sprintf( source, "%s/%s/%s",      EG_REMOTE_DOC_DIR, subDir, remoteFile_p->name_p );
            sprintf( temp,   "%s/%s/%s-temp", EG_LOCAL_DOC_DIR,  subDir, remoteFile_p->name_p );
            sprintf( target, "%s/%s/%s",      EG_LOCAL_DOC_DIR,  subDir, remoteFile_p->name_p );

            sprintf( gBuf_p, "%s %s:%s %s > /dev/null 2>/dev/null\n",
                EG_SCP_CMD, gRemoteMachine, source, temp );

            egLog( gBuf_p );
            egSystem( gBuf_p, TRUE );

            /* Check file */
            crc = egCrcFile( temp, NIL );

            //mbLog( "local %X remote %x\n", crc, remoteFile_p->crc );
            if( crc == remoteFile_p->crc )
            {
                egLog( "File '%s': verified OK\n", target );
                sprintf( gBuf_p, "%s %s %s > /dev/null 2>/dev/null\n",
                    EG_MV_CMD, temp, target );

                egLog( gBuf_p );
                egSystem( gBuf_p, TRUE );
                verifyGoodCount++;
            }
            else
            {
	        egLog("ERROR: File '%s' CRC failed; want: %x, got: %x\n", target, remoteFile_p->crc, crc );
                if( alreadyCountedBad == FALSE ) verifyBadCount++;
                unlink( temp );
            }
        }
        if( egStopCheck() == TRUE ) break;
    }

//exit:
    if( gCheckDocs )
    {
        qWalk( localFile_p, local_q, EgFile_p )
        {
            if( localFile_p->found != TRUE )
            {
                egLog( "Unknown local file: '%s'\n", localFile_p->pathName_p );
            }
        }
        egLog( "Documents verified Good: %lu Bad: %lu\n", verifyGoodCount, verifyBadCount );
    }
    egFileListFree( localDir_q );
    egFileListFree( localTemp_q );
    egFileListFree( remote_q );
    egFileListFree( local_q );

    return TRUE;
}

/******************************************************************************
 * Function:
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */
Int32s_t egDocumentsGetRemote( qHead_p remote_q )
{
    EgFile_p        file_p;
    Int32s_t        i;
    MYSQL_RES      *resultSet_p = NIL;
    MYSQL_ROW       row;

    sprintf( gBuf_p, "select name,crc from documents where id > 0 and not_deleted > 0 and imported=1" );

     /* Read all the echos */
    if( mysql_query( gMySQL_p, gBuf_p ) != 0 ) goto exit;

    if( ( resultSet_p = mysql_store_result( gMySQL_p ) ) == NULL ) goto exit;

    while( ( row = mysql_fetch_row( resultSet_p ) ) != NULL )
    {
        /* Allocate space for the echo */
        file_p = mbCalloc( sizeof( EgFile_t ) );

        for( i = 0 ; i < mysql_num_fields( resultSet_p ); i++ )
        {
            switch( i )
            {
                case 0:
                    if( ( file_p->name_p = mbMallocStr( row[i] ) ) == NIL ) goto exit;
                    break;

                case 1:
                    file_p->crc = strtoul( row[i], NIL, 10 );
                    break;

                default:
                    break;

            }
        }
        qInsertLast( remote_q, file_p );
    }

exit:
    return TRUE;
}

/******************************************************************************
 * Function: egEchoVerify( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */
Int32s_t egEchoVerify( EgEcho_p echo_p, Boolean_t localFlag )
{
    EgFile_p    localFile_p;
    EgFile_p    remoteFile_p;
    Int32s_t    returnCode = FALSE;
    Int32u_t    compareCount;
    Int32u_t    verifyCount = 0;
    Boolean_t   match;
    Boolean_t   found;
    Boolean_t   noFiles = FALSE;
    Boolean_t   crcError = FALSE;
    Boolean_t   missing = FALSE;

    if( echo_p == NIL ) goto exit;

    /* egLog( "Verify echo '%s' %u\n", echo_p->path_p, echo_p->ID ); */

    /* Get the list of files for this echo */
    if( echo_p->localFile_q == NIL )
    {
        echo_p->localFile_q = qInitialize( &echo_p->localFileQueue );
    }
    else
    {
        egFileListFree( echo_p->localFile_q );
    }

    if( localFlag )
    {
        egFileListGet( echo_p->path_p, echo_p->localFile_q, EG_FILE_MODE_REG );
    }
    else
    {
        Char_t  buf[512];
        sprintf( buf, "%s%s", gLocalDir_p, echo_p->path_p );
        egLog( "Echo ID %lu: %s verifiy...\n", echo_p->ID, buf );
        egFileListGet( buf, echo_p->localFile_q, EG_FILE_MODE_REG );
    }

    if( echo_p->localFile_q->size == 0 )
    {
        egLog( "Echo ID %u: No local files detected\n", echo_p->ID );
        noFiles = TRUE;
        goto exit;
    }

    /* Get list of remote files belonging to this echo */
    if( egEchoRemoteFileQueueGet( echo_p ) == FALSE ) goto exit;

    compareCount = echo_p->remoteFile_q->size;

    /* Now must compare files */
    while( !qEmpty( echo_p->remoteFile_q ) )
    {
        match = FALSE;
        found = FALSE;

        remoteFile_p = (EgFile_p)qRemoveFirst( echo_p->remoteFile_q );
        qWalk( localFile_p, echo_p->localFile_q, EgFile_p )
        {
            if( strcmp( localFile_p->name_p, remoteFile_p->name_p ) == 0 )
            {
                found = TRUE;

                if( localFile_p->crc == 0 )
                {
                    localFile_p->crc = egCrcFile( localFile_p->pathName_p, NIL );
                }

                if( localFile_p->crc == remoteFile_p->crc )
                {
                    match = TRUE;
                    verifyCount++;
                }
                else
                {
                    egLog( "Echo ID %u: CRC failure on file '%s'\n", echo_p->ID,
                        localFile_p->name_p );
                    crcError = TRUE;
                }

                qRemoveEntry( echo_p->localFile_q, localFile_p );
                egFileListFreeEntry( localFile_p );

                break;
            }
        }

        if( found == FALSE )
        {
            egLog( "Echo ID %u: File not found: %s\n", echo_p->ID, remoteFile_p->name_p );
            missing = TRUE;
        }

        egFileListFreeEntry( remoteFile_p );
    }

    /* Now both queues should be empty */
    if( echo_p->remoteFile_q->size != 0 )
    {
        egLog( "Echo ID %u: verify error: The following remote files were not matched\n", echo_p->ID );
        egEchoFileQueueDisplay( echo_p->remoteFile_q );
        goto exit;
    }

    if( echo_p->localFile_q->size != 0 )
    {
        egLog( "Echo ID %u: Warning: Extraneous local files detected\n", echo_p->ID );
        egEchoFileQueueDisplay( echo_p->localFile_q );
    }

    if( verifyCount == compareCount )
    {
        egLog( "Echo ID %u: %d files verified OK\n", echo_p->ID, verifyCount );
    }
    else
    {
        egLog( "Echo ID %d: %d of %d files verified\n", echo_p->ID, verifyCount, compareCount );
    }

    returnCode = TRUE;
exit:

    if( echo_p )
    {
        if( returnCode == FALSE || missing == TRUE || crcError == TRUE )
        {
            Char_t  badDir[512];

            sprintf( badDir, "%s%s", gLocalDir_p, EG_LOCAL_BAD_DIR );
            if( noFiles == FALSE )
            {
                egLog( "Echo ID %u: verify FAILED\n", echo_p->ID );
            }

            if( noFiles == TRUE && gMoveEmptyFlag == TRUE )
            {
                egMoveBadEcho( echo_p, badDir );
            }
            else if( noFiles == FALSE && gMoveBadFlag == TRUE )
            {
                egMoveBadEcho( echo_p, badDir );
            }

            if( gSetBackupFlag )
            {
                sprintf( gBuf_p, "update echos set backup=0 where id=%lu", echo_p->ID );

                egLog( "Echo ID %u: %s\n", echo_p->ID, gBuf_p );
                egSqlExec( gBuf_p );
            }

            if( gSetOnlineFlag )
            {
                sprintf( gBuf_p, "update echos set online=0 where id=%lu", echo_p->ID );

                egLog( "Echo ID %u: %s\n", echo_p->ID, gBuf_p );
                egSqlExec( gBuf_p );
            }
        }
        else
        {
            /* Attempt to mark echo backed up */
            if( gSetBackupFlag )
            {
                sprintf( gBuf_p, "update echos set backup=1 where id=%lu", echo_p->ID );

                egLog( "Echo ID %u: %s\n", echo_p->ID, gBuf_p );
                egSqlExec( gBuf_p );
            }

            if( gSetOnlineFlag )
            {
                sprintf( gBuf_p, "update echos set online=1 where id=%lu", echo_p->ID );

                egLog( "Echo ID %u: %s\n", echo_p->ID, gBuf_p );
                egSqlExec( gBuf_p );
            }
        }

        /* Should clean the files queues, becase a failure could leave them */
        /* half drained */

        egFileListFree( echo_p->remoteFile_q );
        egFileListFree( echo_p->localFile_q );
    }
    return returnCode;
}

/******************************************************************************
 * Function: egMoveBadEcho( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Int32s_t    egMoveBadEcho( EgEcho_p echo_p, Char_p badDir_p )
{
    Int32s_t    returnCode = FALSE;
    Char_t      buf[1024];

    if( echo_p == NIL || badDir_p == NIL ) goto exit;

    sprintf( gBuf_p, "%s %s %s%06lu >/dev/null 2>/dev/null", EG_MV_CMD, echo_p->path_p, badDir_p, echo_p->ID );

    sprintf( buf, "%s\n", gBuf_p );
    egLog( buf );

    if( egSystem( gBuf_p, TRUE ) != 0 )
    {
        egLog( "Echo ID %lu: Move failed\n", echo_p->ID );
    }

exit:
    return returnCode;
}

/******************************************************************************
 * Function: egEchoQueueFreeEntry( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */
Int32s_t egEchoQueueFreeEntry( EgEcho_p echo_p )
{
    if( echo_p )
    {
        egFileListFree( echo_p->remoteFile_q );
        egFileListFree( echo_p->localFile_q );
        mbFree( echo_p->name_p );
        mbFree( echo_p->path_p );
        mbFree( echo_p );
    }
    return TRUE;
}

/******************************************************************************
 * Function: egEchoGet( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Int32s_t egEchoGet( EgEcho_p echo_p )
{
    Int32s_t        returnCode = FALSE;
    EgFile_p        file_p;
    Char_p          source_p = NIL;
    Char_p          target_p = NIL;
    Char_p          temp_p = NIL;
    Char_p          date_p = NIL;
    Char_p          dest_p = NIL;
    Ints_t          len;
    Ints_t          result;
    Int32u_t        crc;

    source_p        = mbMalloc( 1024 );
    target_p        = mbMalloc( 1024 );

    egLog( "Echo ID %lu: Get echo name '%s'\n", echo_p->ID, echo_p->name_p );

    /* Get a list of the files in this echo */
    if( egEchoRemoteFileQueueGet( echo_p ) != TRUE ) goto exit;

    /* Display the files */
    /* egEchoFileQueueDisplay( file_q ); */

    /* Blindly attempt to create dir.  It might already be there from   */
    /* a failed previous attempt                                        */
    sprintf( gBuf_p, "%s %s.temp-%lu >/dev/null 2>/dev/null\n", EG_MKDIR_CMD, gLocalDir_p, echo_p->ID );
    egSystem( gBuf_p, TRUE );

    qWalk( file_p, echo_p->remoteFile_q, EgFile_p )
    {
        if( egStopCheck() == TRUE ) goto exit;

        /* Make the intermediate directory (may already exist) */
        if( date_p == NIL )
        {
            int i, len;
            date_p = mbMalloc( 256 );
            strcpy( date_p, file_p->path_p );
            len = strlen( date_p );
            for( i = 0 ; i < len ; i++ )
            {
                if( *(date_p + i) == MB_CHAR_SLASH )
                {
                    *(date_p + i) = 0;
                }
            }
            sprintf( gBuf_p, "%s %s%s >/dev/null 2>/dev/null\n", EG_MKDIR_CMD, gLocalDir_p, date_p );
            egLog( gBuf_p );
            egSystem( gBuf_p, TRUE );
        }

        if( dest_p == NIL )
        {
            dest_p = mbMalloc( 256 );
            strcpy( dest_p, file_p->path_p );
        }

        /* Format the SCP command */
        sprintf( source_p, "%s%s/%s", gLocalDir_p, file_p->path_p, file_p->name_p );

        strcpy( gBuf_p, file_p->name_p );
        len = strlen( gBuf_p );
        for( temp_p = gBuf_p + len -1 ;  ; temp_p-- )
        {
            if( *temp_p == '/' )
            {
                temp_p++;
                break;
            }
            if( temp_p == gBuf_p ) break;
        }

        sprintf( target_p, "%s.temp-%lu/%s", gLocalDir_p, echo_p->ID, temp_p );

        if( egCrcFile( target_p, NIL ) == file_p->crc )
        {
            egLog( "Echo ID %lu: Already have file '%s'\n", echo_p->ID, target_p );
        }
        else
        {
            /* Blindly attempt to remove target file in case its already partially there */
            unlink( target_p );
            sprintf( gBuf_p, "%s %s:%s %s > /dev/null 2>/dev/null", EG_SCP_CMD, gRemoteMachine, source_p, target_p );
            egLog( "%s\n", gBuf_p );
            if( ( result = egSystem( gBuf_p, TRUE ) ) != 0 )
            {
                egLog( "System() returned %d\n", result );
                goto exit;
            }
            crc = egCrcFile( target_p, NIL );
            if( crc != file_p->crc )
            {
                /* Bail out */
                egLog( "Echo ID %lu: CRC failed: got %lu expected %lu\n", echo_p->ID, crc, file_p->crc );
                unlink( target_p );
                goto exit;
            }
        }
    }

    if( dest_p )
    {
        if( strlen( dest_p ) )
        {
            sprintf( gBuf_p, "%s '%s%s' '/home/mikeb/moved_echos/%lu' >/dev/null 2>/dev/null\n",
                EG_MV_CMD, gLocalDir_p, dest_p, echo_p->ID );

            egLog( gBuf_p );
            egSystem( gBuf_p, TRUE );
        }
    }

    sprintf( gBuf_p, "%s '%s.temp-%lu' '%s%s' >/dev/null 2>/dev/null\n", EG_MV_CMD, gLocalDir_p, echo_p->ID, gLocalDir_p, dest_p );
    egLog( gBuf_p );
    if( egSystem( gBuf_p, TRUE ) != 0 )
    {
        egLog( "Echo ID %lu: Move failed\n", echo_p->ID );
        goto exit;
    }

    if( echo_p->path_p ) mbFree( echo_p->path_p );
    echo_p->path_p = mbMallocStr( dest_p );

    sprintf( gBuf_p, "%s %s%s >/dev/null 2>/dev/null\n", EG_CHMOD_CMD, gLocalDir_p, dest_p );
    egLog( gBuf_p );
    if( egSystem( gBuf_p, TRUE ) == 0 )
    {
        if( gSetBackupFlag )
        {
            egEchoVerify( echo_p, FALSE );
        }
    }

#if 0
    /* Create a new record for this echo */
    if( egSqlExec(  "lock tables echos_get write" ) != TRUE ) goto exit;
    gotLock = TRUE;

    if( egSqlExecInt( "select max( id ) from echos_get", &recordId ) != TRUE ) goto exit;
    recordId++;
    sprintf( gBuf_p, "insert into echos_get ( id, created ) values ( %lu, now() )", recordId );
    if( egSqlExec( gBuf_p ) != TRUE ) goto exit;

    sprintf( gBuf_p, "update echos_get set echo_id=%lu,my_id=%lu,not_deleted=1,name=\"%s\" where id=%lu",
        echo_p->ID, egMyId, name_p, recordId );
    if( egSqlExec( gBuf_p ) != TRUE ) goto exit;
#endif

    returnCode = TRUE;
exit:

    mbFree( target_p );
    mbFree( source_p );
    mbFree( date_p );
    mbFree( dest_p );

    if( returnCode == FALSE ) egLog( "Get failed\n" );
    return returnCode;
}

/******************************************************************************
 * Function: egEchoFileQueueDisplay( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Int32s_t egEchoFileQueueDisplay( qHead_p file_q )
{
    EgFile_p        file_p;
    Int32s_t        returnCode = FALSE;

    if( file_q == NIL ) goto exit;

    qWalk( file_p, file_q, EgFile_p )
    {
        egLog( "File: '%s' path '%s' orig '%s' CRC: %lu\n",
            file_p->name_p      ? file_p->name_p     : "UNKNOWN",
            file_p->path_p      ? file_p->path_p     : "UNKNOWN",
            file_p->origName_p  ? file_p->origName_p : "UNKNOWN",
            file_p->crc );
    }

    returnCode = TRUE;
exit:
    return returnCode;
}

/******************************************************************************
 * Function: egEchoRemoteFileQueueGet( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */
Int32s_t egEchoRemoteFileQueueGet( EgEcho_p echo_p )
{
    MYSQL_RES      *resultSet_p = NIL;
    MYSQL_ROW       row;
    Int32s_t        returnCode = FALSE;
    Int32s_t        i;
    EgFile_p        file_p;


    if( echo_p == NIL ) goto exit;

    if( echo_p->remoteFile_q == NIL )
    {
      echo_p->remoteFile_q = qInitialize( &echo_p->remoteFileQueue );
    }
    else
    {
      egFileListFree( echo_p->remoteFile_q );
    }

    sprintf( gBuf_p, "select name,orig,path,crc from echo_files where echo_id=%lu and not_deleted=1\n", echo_p->ID );

    if( mysql_query( gMySQL_p, gBuf_p ) != 0 )
    {
        egLog( "Error: mysql_query( %s ) != 0\n", gBuf_p );
        goto exit;
    }

    if( ( resultSet_p = mysql_store_result( gMySQL_p ) ) == NIL )
    {
        egLog( "Error: resultSet_p = NIL\n" );
        goto exit;
    }

    while( ( row = mysql_fetch_row( resultSet_p ) ) != NIL )
    {
        // Allocate space for the file
        if( ( file_p = mbCalloc( sizeof( EgFile_t ) ) ) == NIL ) goto exit;

        for( i = 0 ; i < mysql_num_fields( resultSet_p ); i++ )
        {
            switch( i )
            {
                case 0:
                    if( ( file_p->name_p = mbMallocStr( row[i] ) ) == NIL ) goto exit;
                    break;

                case 1:
                    if( ( file_p->origName_p = mbMallocStr( row[i] ) ) == NIL ) goto exit;
                    break;

                case 2:
                    if( ( file_p->path_p = mbMallocStr( row[i] ) ) == NIL ) goto exit;
                    break;

                case 3:
                    file_p->crc = strtoul( row[i], NIL, 10 );
                    break;

                default:
                    break;
            }
        }
        qInsertLast( echo_p->remoteFile_q, file_p );
    }

    returnCode = TRUE;
exit:
    if( resultSet_p ) mysql_free_result( resultSet_p );
    return returnCode;
}

/******************************************************************************
 * Function: egSqlExec( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Int32s_t    egSqlExec( Char_p cmd_p )
{
    Int32s_t    returnCode = FALSE;
    if( mysql_query( gMySQL_p, cmd_p ) != 0 )
    {
        goto exit;
    }
    returnCode = TRUE;
exit:
    if( returnCode == FALSE && cmd_p )
    {
        egLog( "SQL command '%s' failed\n", cmd_p );
    }
    return returnCode;
}

/******************************************************************************
 * Function: egSqlExecInt( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Int32s_t    egSqlExecInt( Char_p cmd_p, Int32u_p result_p )
{
    MYSQL_RES   *resultSet_p = NIL;
    MYSQL_ROW   row;
    Int32s_t    returnCode = FALSE;
    Int32u_t    resultInt = 0;
    Int32u_t    count = 0;

    if( cmd_p == NIL ) goto exit;

    if( mysql_query( gMySQL_p, cmd_p ) != 0 )
    {
        goto exit;
    }

    if( ( resultSet_p = mysql_store_result( gMySQL_p ) ) == NULL )
    {
        goto exit;
    }

    if( mysql_num_fields( resultSet_p ) != 1 )
    {
        goto exit;
    }

    while( ( row = mysql_fetch_row( resultSet_p ) ) != NULL )
    {
        resultInt = atoi( row[0] );
        count++;
    }

    if( count != 1 ) goto exit;

    returnCode = TRUE;
exit:

    if( resultSet_p ) mysql_free_result( resultSet_p );
    if( result_p ) *result_p = resultInt;

    if( returnCode == FALSE && cmd_p )
    {
        egLog( "SQL command '%s' failed\n", cmd_p );
    }
    return returnCode;
}

/******************************************************************************
 * Function: egSqlExecInt( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Char_p    egSqlExecString( Char_p cmd_p, Char_p string_p )
{
    MYSQL_RES   *resultSet_p = NIL;
    MYSQL_ROW   row;
    Int32u_t    count = 0;

    if( string_p == NIL || cmd_p == NIL ) goto exit;

    if( mysql_query( gMySQL_p, cmd_p ) != 0 )
    {
        goto exit;
    }

    if( ( resultSet_p = mysql_store_result( gMySQL_p ) ) == NULL )
    {
        goto exit;
    }

    if( mysql_num_fields( resultSet_p ) != 1 )
    {
        goto exit;
    }

    while( ( row = mysql_fetch_row( resultSet_p ) ) != NULL )
    {
        strcpy( string_p, row[0] );
        count++;
    }

    if( count != 1 ) goto exit;

exit:

    if( resultSet_p ) mysql_free_result( resultSet_p );
    return string_p;
}

/******************************************************************************
 * Function: egEchoQueueRemoteGet( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Int32s_t egEchoQueueRemoteGet( qHead_p echo_q )
{
    MYSQL_RES   *resultSet_p = NIL;
    MYSQL_ROW   row;
    Int32u_t    i;
    Int32s_t    returnCode = FALSE;
    EgEcho_p    echo_p;
    EgEcho_p    echo2_p;
    Boolean_t   added;

    if( gMaxAge > 0 )
    {
        egLog( "Only consider remote echos newer than %lu\n", gMaxAge );
    }

    sprintf( gBuf_p,  "select name,id,date from echos "
                      "where not_deleted = 1 "
                      "and online > 0 and date > %lu and read_date != 3", gMaxAge );
    /* Read all the echos */
    if( mysql_query( gMySQL_p, gBuf_p ) != 0 ) goto exit;

    if( ( resultSet_p = mysql_store_result( gMySQL_p ) ) == NULL ) goto exit;

    while( ( row = mysql_fetch_row( resultSet_p ) ) != NULL )
    {
        /* Allocate space for the echo */
        echo_p = mbCalloc( sizeof( EgEcho_t ) );
        if( echo_p == NIL )
        {
            goto exit;
        }

        for( i = 0 ; i < mysql_num_fields( resultSet_p ); i++ )
        {
            switch( i )
            {
                case 0:
                    /* This is the echo name */
                    echo_p->name_p = mbMallocStr( row[i] );
                    break;

                case 1:
                    /* This is the echo id */
                    echo_p->ID = atoi( row[i] );
                    break;

                case 2:
                    /* This is the echo id */
                    echo_p->date = atoi( row[i] );
                    break;

                default:
                    break;
            }
        }

        /* Sort by ID */
        added = FALSE;
        qWalk( echo2_p, echo_q, EgEcho_p )
        {
            if( echo_p->ID < echo2_p->ID )
            {
                qInsertBefore( echo_q, echo2_p, echo_p );
                added = TRUE;
                break;
            }
        }
        if( !added )
        {
            qInsertLast( echo_q, echo_p );
        }
    }

    returnCode = TRUE;
exit:

    if( resultSet_p ) mysql_free_result( resultSet_p );
    return returnCode;
}

/******************************************************************************
 * Function: egEchoQueueLocalGet( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Boolean_t   egEchoDirNameValid( Char_p name_p )
{
    Boolean_t   returnCode = FALSE;
    Ints_t      i, len;

    if( name_p == NIL ) goto exit;

    if( ( len = strlen( name_p ) ) != 6 ) goto exit;

    for( i = 0 ; i < len ; i++ )
    {
        if( *( name_p + i ) < '0' || *( name_p + i ) > '9' ) goto exit;
    }

    returnCode = TRUE;
exit:
    return returnCode;
}

/******************************************************************************
 * Function: egEchoQueueLocalGet( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Int32s_t egEchoQueueLocalGet( qHead_p echo_q )
{
//    MYSQL_RES   *resultSet_p = NIL;
//    MYSQL_ROW   row;
//    Int32u_t    i;

//    Char_p      cmd_p;

    Int32s_t        returnCode = FALSE;
    qHead_t         fileQueue;
    qHead_p         file_q;
    EgFile_p        file_p;
    EgFile_p        file2_p;
    qHead_t         file2Queue;
    qHead_p         file2_q;
    EgEcho_p        echo_p;
    EgEcho_p        echo2_p;
    Boolean_t       added;

    file_q  = qInitialize( &fileQueue );
    file2_q = qInitialize( &file2Queue );

    egFileListGet( gLocalDir_p, file_q, EG_FILE_MODE_ALL );

    /* egLog( "file_q->size: %u\n", file_q->size ); */
    qWalk( file_p, file_q, EgFile_p )
    {
        /* egLog( "Got file '%s'\n", file_p->pathName_p ); */

        egFileListGet( file_p->pathName_p, file2_q, EG_FILE_MODE_ALL );

        qWalk( file2_p, file2_q, EgFile_p )
        {
            /* egLog( "Got file '%s'\n", file2_p->pathName_p ); */

            if( egEchoDirNameValid( file2_p->name_p ) )
            {
                if( ( echo_p = mbCalloc( sizeof(EgEcho_t) ) ) == NIL )
                {
                    egLog( "Error allocating memory for echo\n" );
                }
                else
                {
                    /* Assume that the echo id is embedded in the dir name */
                    echo_p->ID = atoi( file2_p->name_p );
                    echo_p->path_p = mbMallocStr( file2_p->pathName_p );

                    echo_p->localFile_q = qInitialize( &echo_p->localFileQueue );

                    if( gCheckLocalEchos == FALSE )
                    {
                        egFileListGet( echo_p->path_p, echo_p->localFile_q, EG_FILE_MODE_REG );
                    }
                    if( echo_p->localFile_q->size > 0 || gCheckLocalEchos == TRUE )
                    {
                        egFileListFree( echo_p->localFile_q );
                        added = FALSE;
                        qWalk( echo2_p, echo_q, EgEcho_p )
                        {
                            if( echo_p->ID < echo2_p->ID )
                            {
                                qInsertBefore( echo_q, echo2_p, echo_p );
                                added = TRUE;
                                break;
                            }
                        }

                        if( !added )
                        {
                            qInsertLast( echo_q, echo_p );
                        }
                    }
                    else
                    {

                        egLog( "Echo ID %lu: No local files\n", echo_p->ID );
                        egEchoQueueFreeEntry( echo_p );
                    }
                }
            }
            else
            {
                /* egLog( "%s: NOT VALID\n", file2_p->name ); */
            }
        }
        egFileListFree( file2_q );
    }
    egFileListFree( file_q );
    returnCode = TRUE;

#if 0
    cmd_p = mbMalloc( 1024 );

    sprintf( cmd_p, "select name,id,echo_id,status from echos_get where my_id = %ld", egMyId );

    if( mysql_query( gMySQL_p, cmd_p ) != 0 )
    {
        goto exit;
    }

    if( ( resultSet_p = mysql_store_result( gMySQL_p ) ) == NULL )
    {
        goto exit;
    }

    while( ( row = mysql_fetch_row( resultSet_p ) ) != NULL )
    {
        /* Allocate space for the echo */
        echo_p = mbCalloc( sizeof( EgEcho_t ) );
        if( echo_p == NIL )
        {
            goto exit;
        }

        for( i = 0 ; i < mysql_num_fields( resultSet_p ); i++ )
        {

            switch( i )
            {
                case 0:
                    /* This is the echo name */
                    echo_p->name_p = mbCalloc( strlen( row[0] ) + 1 );
                    if( echo_p->name_p == NIL )
                    {
                        mbFree( echo_p );
                        goto exit;
                    }
                    strcpy( echo_p->name_p, row[0] );
                    break;

                case 1:
                    /* This is the record id */
                    echo_p->recordId = atoi( row[1] );
                    break;

                case 2:
                    /* This is the echo id */
                    echo_p->ID = atoi( row[2] );
                    break;

                case 3:
                    /* This is the echo status */
                    echo_p->status = atoi( row[3] );
                    break;

                default:
                    break;
            }
        }
        qInsertLast( echo_q, echo_p );
    }

    returnCode = TRUE;
exit:

    if( resultSet_p ) mysql_free_result( resultSet_p );
    mbFree( cmd_p );
#endif

    return returnCode;
}

/******************************************************************************
 * Function: egEchoQueueDisplay( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Void_t egEchoQueueDisplay( qHead_p echo_q )
{
    EgEcho_p echo_p;

    qWalk( echo_p, echo_q, EgEcho_p )
    {
        egLog( "Name '%s' ID %u\n",
            echo_p->name_p ? echo_p->name_p : "UNKNOWN",
            echo_p->ID );
    }
    return;
}

/******************************************************************************
 * Function: egEchoQueueFree( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Void_t  egEchoQueueFree( qHead_p echo_q )
{
    EgEcho_p    echo_p;

    if( echo_q == NIL ) goto exit;
    while( !qEmpty( echo_q ) )
    {
        echo_p = (EgEcho_p)qRemoveFirst( echo_q );
        egEchoQueueFreeEntry( echo_p );
    }
exit:
    return;
}

/******************************************************************************
 * Function: egFileCompare( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns the starting position of substring i2 within i1.
 *-----------------------------------------------------------------------------
 */

Int32s_t        egFileCompare
(
    Char_p      fileName1_p,
    Char_p      fileName2_p
)
{
    if( strlen( fileName1_p ) != strlen( fileName2_p ) ) return -1;

    return strPos( fileName1_p, fileName2_p );
}

/******************************************************************************
 * Function: egFileListGet( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *
 *-----------------------------------------------------------------------------
 */

Int32s_t egFileListGet
(
    Char_p      dirIn_p,
    qHead_p     q_p,
    Ints_t      mode
)
{
    struct dirent **namelist_pp = 0;
    int             n;
    struct stat     buf;
    char            filename[512];
    char            dir[512];
    EgFile_p        file_p;

    if( dirIn_p == NIL ) goto exit;
    strcpy( dir, dirIn_p );
    mbStrTrailingSlashRemove( dir );

    /* egLog( "egFileListGet( %s )\n", dir ); */

    /* First, clear out any files that may be in the list */
    egFileListFree( q_p );

    n = scandir( dir, &namelist_pp, 0, 0 );
    if( n < 0 )
    {
        egLog( "Error: scandir( %s ) < 0: %d\n", dir, n );
        goto exit;
    }
    else
    {
        /* Step through all the entries */
        while( n-- )
        {
            sprintf( filename, "%s/%s", dir, namelist_pp[n]->d_name );

            /* printf( "Consider filename '%s'\n", filename ); */

            if( stat( filename, &buf ) != 0 )
            {
                /* Error */
                egLog( "Error: stat( %s ) != 0\n", filename );
            }
            else
            {
                if( ( mode == EG_FILE_MODE_ALL ) ||
                    ( mode == EG_FILE_MODE_REG && S_ISREG( buf.st_mode ) ) )
                {
                    file_p = mbCalloc( sizeof( EgFile_t ) );

                    file_p->name_p = mbMallocStr( namelist_pp[n]->d_name );
                    file_p->path_p = mbMallocStr( dir );
                    file_p->pathName_p = mbMallocStr( filename );

                    qInsertFirst( q_p, file_p );
                }
            }

            if( namelist_pp[n] )
            {
                /* must use free() here */
                free( namelist_pp[n] );
            }
            else
            {
                egLog( "Got NIL namelist[%d]", n );
            }
        }
    }

    if( namelist_pp )
    {
        free( namelist_pp );
    }
    else
    {
        egLog( "Got NIL namelist\n" );
    }

exit:
    return 0;
}

/******************************************************************************
 * Function: egFileListFreeEntry( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Int32s_t egFileListFreeEntry( EgFile_p file_p )
{
    Int32s_t    returnCode = FALSE;

    if( file_p == NIL ) goto exit;

    mbFree( file_p->name_p );
    mbFree( file_p->path_p );
    mbFree( file_p->pathName_p );
    mbFree( file_p->origName_p );
    mbFree( file_p );

    returnCode = TRUE;

exit:
    return returnCode;
}
/******************************************************************************
 * Function: egFileListFree( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns the starting position of substring i2 within i1.
 *-----------------------------------------------------------------------------
 */

Int32s_t egFileListFree( qHead_p file_q )
{
    EgFile_p    file_p;
    Int32s_t    returnCode = FALSE;

    if( file_q == NIL ) goto exit;
    while( !qEmpty( file_q ) )
    {
        file_p = (EgFile_p)qRemoveFirst( file_q );
        egFileListFreeEntry( file_p );
    }
    returnCode = TRUE;
exit:
    return returnCode;
}

/******************************************************************************
 * Function: egCrcFile( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns the starting position of substring i2 within i1.
 *-----------------------------------------------------------------------------
 */

Int32u_t egCrcFile( Char_p fileName_p, Int32u_p size_p )
{
    FILE       *fp;
    Int8u_p     buf_p;
    size_t      bytes;
    Int32u_t    crc = 0;
    Int32u_t    size = 0;
    Int64u_t    testSize = 0;

    buf_p = mbMalloc( 5000 );

    if( ( fp = fopen( fileName_p, "rb" ) ) == NIL ) goto exit;

    for( ; ; )
    {
        bytes = fread( (Void_p)buf_p, 1, 4096, fp );
        size += bytes;
        testSize += (Int64u_t)bytes;

        if( testSize > 0x00000000FFFFFFFF )
        {
            break;
        }
        crc = egCrcUpdate( crc, buf_p, bytes );
        if( bytes < 4096 ) break;
    }

    bytes = size;

    while( bytes > 0 )
    {
        crc = (crc << 8) ^ gCrcTable[((crc >> 24) ^ bytes) & 0xFF];
        bytes >>= 8;
    }

    crc = ~crc & 0xFFFFFFFF;

exit:

    if( size_p ) *size_p = size;

    if( fp ) fclose( fp );
    mbFree( buf_p );

    return crc;
}

/******************************************************************************
 * Function: egCrcUpdate( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns the starting position of substring i2 within i1.
 *-----------------------------------------------------------------------------
 */

Int32u_t        egCrcUpdate
(
    Int32u_t    crc,
    Int8u_p     data_p,
    Int32u_t    dataLen
)
{
    Int32u_t    j;

    for( j = 0 ;  j < dataLen ;  j++ )
    {
        crc = ( crc << 8 ) ^ gCrcTable[ ( ( crc >> 24 ) ^ *data_p++ ) & 0xFF ];
    }
    return crc;
}

/*-----------------------------------------------------------------------------
 * Procedure: egCrcInit( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * Generates a 256-word table containing all CRC remainders for every
 * possible 8-bit byte.
 *
 * Polynomial: (MB_CRC_POLYNOMIAL)
 * x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x^1+x^0
 *-----------------------------------------------------------------------------
 */

void egCrcInit( void )
{
    Int32u_t    i, j, crcTableEntry;

    for( i = 0 ; i < 256 ; i++ )
    {
        crcTableEntry = (Int32u_t)( i << 24 );
        for( j = 0 ; j < 8 ; j++ )
        {
            if ( crcTableEntry & 0x80000000L )
            {
                crcTableEntry = ( crcTableEntry << 1 ) ^ EG_CRC_POLYNOMIAL;
            }
            else
            {
                crcTableEntry = ( crcTableEntry << 1 );
            }
        }
        gCrcTable[i] = crcTableEntry;
    }
    return;
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
 * Function: egHandleSignals( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns the starting position of substring i2 within i1.
 *-----------------------------------------------------------------------------
 */

Void_t      egHandleSignals
(
    Ints_t  sig
)
{
    if( sig == SIGHUP     ) egLog( "got SIGHUP    \n" );
    if( sig == SIGINT     ) egLog( "got SIGINT    \n" );
    if( sig == SIGQUIT    ) egLog( "got SIGQUIT   \n" );
    if( sig == SIGILL     ) egLog( "got SIGILL    \n" );
    if( sig == SIGTRAP    ) egLog( "got SIGTRAP   \n" );
    if( sig == SIGABRT    ) egLog( "got SIGABRT   \n" );
    if( sig == SIGFPE     ) egLog( "got SIGFPE    \n" );
    if( sig == SIGSEGV    ) egLog( "got SIGSEGV   \n" );
    if( sig == SIGPIPE    ) egLog( "got SIGPIPE   \n" );
    if( sig == SIGALRM    ) egLog( "got SIGALRM   \n" );
    if( sig == SIGTERM    ) egLog( "got SIGTERM   \n" );
    if( sig == SIGURG     ) egLog( "got SIGURG    \n" );
    if( sig == SIGTTIN    ) egLog( "got SIGTTIN   \n" );
    if( sig == SIGTTOU    ) egLog( "got SIGTTOU   \n" );
    if( sig == SIGIO      ) egLog( "got SIGIO     \n" );
    if( sig == SIGBUS     ) egLog( "got SIGBUS    \n" );
    if( sig == SIGTSTP    ) egLog( "got SIGTSTP   \n" );

    return;
}

/******************************************************************************
 * Function: egLog
 *-----------------------------------------------------------------------------
 * Description:
 *
 *
 *-----------------------------------------------------------------------------
 */

Ints_t egLog
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
    Char_t              logFileName[128];
    FILE               *fp;

    if( ( str_p = (Char_p)malloc( 1024 ) ) == 0 ) return 0;

    MB_LOCK_ACQUIRE( gLogLock_p );

    va_start( arg, string_p );
    vsprintf( str_p, string_p, arg );
    va_end( arg );

    ptime = time( (long *) 0 );
    strcpy( timeStr, egCharTime( &ptime ) );
    strcpy( dateStr, egCharDate( &ptime ) );

    if( gBaseDir_p )
    {
        sprintf( logFileName, "%s/%s-%s.log", gBaseDir_p, dateStr, EG_PROG_NAME );
    }
    else
    {
        sprintf( logFileName, "%s-%s.log", dateStr, EG_PROG_NAME );
    }

    fp = fopen( logFileName, "a" );
    if( fp )
    {
        fprintf( fp, "%s: %ld: %s", timeStr, gMyPid, str_p );
        fclose( fp );
    }

    if( !gSilentNew )
    {
        fprintf( stdout, "%s: %s", timeStr, str_p );
    }

    free( str_p );

    MB_LOCK_RELEASE( gLogLock_p );

    return 0;
}

/******************************************************************************
 * Function: egCharTime
 *-----------------------------------------------------------------------------
 * Description:
 *
 *
 *-----------------------------------------------------------------------------
 */

Char_p egCharTime( time_t *tm )
{
    static Char_t   buf[100];
    struct tm *tm_p; tm_p = localtime( tm );

    sprintf( buf, "%02d:%02d:%02d", tm_p->tm_hour, tm_p->tm_min,tm_p->tm_sec );
    return &buf[0];
}

/******************************************************************************
 * Function: egCharDate
 *-----------------------------------------------------------------------------
 * Description:
 *
 *
 *-----------------------------------------------------------------------------
 */

Char_p egCharDate( time_t *tm )
{
    static Char_t   buf[100];
    struct tm *tm_p; tm_p = localtime( tm );

    sprintf( buf, "%04d-%02d-%02d", 1900 + tm_p->tm_year ,tm_p->tm_mon+1, tm_p->tm_mday );
    return &buf[0];
}

/******************************************************************************
 * Function: egToday
 *-----------------------------------------------------------------------------
 * Description:
 *
 *
 *-----------------------------------------------------------------------------
 */

Int32u_t egToday( void )
{
    time_t              tm;
    struct tm          *tm_p;
    Int32u_t            today;
    tm = time( NIL );
    tm_p = localtime( &tm );

    today = 1900 + tm_p->tm_year;
    today *= 100;
    today += tm_p->tm_mon+1;
    today *= 100;
    today += tm_p->tm_mday;

    return today;
}

/*---------------------------------------------------------------------------
 * Function:    egStrClean
 *---------------------------------------------------------------------------
 * Description:
 *
 * this function removes leading and trailing white space, and converts
 * any double quoted to single quotes. The output is written to the
 * specified buffer.  No length checking is done.
 *---------------------------------------------------------------------------
 */

Char_p          egStrClean
(
    Char_p      in_p,
    Char_p      out_p
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
        out_p = mbMalloc( strlen( in_p ) + 1 );
        strcpy( out_p, in_p );
        copyToInput = TRUE;
    }
    else
    {
        strcpy( out_p, in_p );
    }

    // Eliminate trailing white space
    temp1_p = out_p + len - 1;
    for( ; temp1_p >= out_p ; temp1_p-- )
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
       if( *temp1_p == MB_CHAR_QUOTE_DOUBLE )  *temp1_p = MB_CHAR_QUOTE_SINGLE;
    }

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

/******************************************************************************
 * Function: egPurgeTemp
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

void egPurgeTemp( void )
{
    qHead_t         fileQueue;
    qHead_p         file_q;
    EgFile_p        file_p;
    struct stat     fileStat;
    Int32u_t        fileSec;
    Int32u_t        nowSec;
    Int32u_t        age;

    file_q = qInitialize( &fileQueue );

    egFileListGet( gLocalDir_p, file_q, EG_FILE_MODE_ALL );

    qWalk( file_p, file_q, EgFile_p )
    {
        if( strPos( file_p->name_p, ".temp" ) == 0 )
        {
            stat( file_p->pathName_p, &fileStat );

            fileSec = (Int32u_t)fileStat.st_atime;
            nowSec  = (Int32u_t)time( 0 );

            age = 60 * 60 * 24 * 7;

            if( nowSec - fileSec > age )
            {
                sprintf( gBuf_p, "%s '%s'\n", EG_RM_CMD, file_p->pathName_p );
                egLog( gBuf_p );
                if( egSystem( gBuf_p, TRUE ) != 0 )
                {
                    egLog( "Failed to purge temp directory\n" );
                }
                else
                {
                    egLog( "Purged '%s' (time: %lu now %lu)\n", file_p->pathName_p, fileSec, nowSec );
                }
            }
        }
    }
    egFileListFree( file_q );
    return;
}

/******************************************************************************
 * Function: egStopCheck
 *-----------------------------------------------------------------------------
 * Description:
 *
 *
 *-----------------------------------------------------------------------------
 */
Int32s_t    egStopCheck( void )
{

    FILE           *fp;

    gWatchdogCounter++;

    if( gStopDetected == TRUE ) return TRUE;

    if( ( fp = fopen( EG_STOP_NAME, "r" ) ) != NIL )
    {
        fclose( fp );
        egLog( "Detected stop file...\n" );
        gStopDetected = TRUE;
        return TRUE;
    }
    return FALSE;
}

/******************************************************************************
 * Function: egLockFileGet
 *-----------------------------------------------------------------------------
 * Description:
 *
 *
 *-----------------------------------------------------------------------------
 */

Int32s_t    egLockFileGet( )
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
    EgFile_p        file_p;

    file_q = qInitialize( &fileQueue );

     /* Get pid from lockfile  */
    fp = fopen( EG_LOCKFILE_NAME, "r" );
    if( fp )
    {
        fgets( buf1_p, 512, fp );

        /* There is a lockfile, get pid from it */
        egFileListGet( "/proc", file_q, EG_FILE_MODE_ALL );

        qWalk( file_p, file_q, EgFile_p )
        {
            {
                if( egFileCompare( buf1_p, file_p->name_p ) == 0 )
                {
                    foundPidFile = TRUE;
                    sprintf( buf2_p, "/proc/%s/cmdline", buf1_p );
                    fp2 = fopen( buf2_p, "r" );
                    if( fp2 )
                    {
                        fgets( buf2_p, 512, fp2 );
                        if( strPos( buf2_p, EG_PROG_NAME ) >= 0 )
                        {
                            foundPid = TRUE;
                            break;
                        }
                        fclose( fp2 );
                    }
                    else
                    {
                        egLog( "Failed to open '%s' for reading\n", buf2_p );
                    }
                    if( foundPidFile == TRUE ) break;
                }
            }
        }

        fclose( fp );
        egFileListFree( file_q );

        if( foundPidFile == FALSE )
        {
            /* Stale lock file? */
        }
        else
        {
            if( foundPid == FALSE )
            {
                egLog( "Found /proc/pid file but not pid '%s', terminating\n", buf1_p );
                goto exit;
            }
        }

        if( foundPid == TRUE )
        {
            egLog( "Found process running with pid '%s', terminating\n", buf1_p );
            goto exit;
        }
        else
        {
            egLog( "Deleting stale lockfile with pid '%s' and running\n", buf1_p );
            unlink( EG_LOCKFILE_NAME );
        }
    }

    /* At this point we are going to run and should create a pid file */
    fp = fopen( EG_LOCKFILE_NAME, "w" );
    if( fp == NIL )
    {
        egLog( "Failed to create lockfile\n" );
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

//---------------------------------------------------------------------------
// Function: mbStrRemoveSlashTrailing
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p  mbStrTrailingSlashRemove( Char_p str_p )
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

#if 0
/******************************************************************************
 * Function: egEchoDelete( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Int32s_t egEchoDelete( EgEcho_p echo_p )
{
    Int32s_t    returnCode = FALSE;
    Char_p      name_p;
    Int32u_t    readDate = 0;
    Int32u_t    today = 0;

    name_p = mbMalloc( 1024 );

    /* Must delete echo here */
    sprintf( gBuf_p, "select name from echos_get where id = %lu", echo_p->recordId );

    if( egSqlExecString( gBuf_p, name_p ) == NIL )
    {
        egLog( "Failed to get echo name\n" );
        goto exit;
    }

    sprintf( gBuf_p, "select read_date from echos where id = %lu", echo_p->ID );

    if( egSqlExecInt( gBuf_p, &readDate ) != TRUE ) goto exit;

    today = egToday();

    if( readDate < today )
    {
        egLog( "Delete '%s', read date %lu, today %lu\n", name_p, readDate, today );

        /* Blindly try to delete target directory */
        sprintf( gBuf_p, "%s '%s%s'\n", EG_RM_CMD, EG_TARGET_DIR, name_p );
        egLog( gBuf_p );
        if( egSystem( gBuf_p, TRUE ) != 0 )
        {
            egLog( "Failed to delete echo files\n" );
            goto exit;
        }

        /* Once echo successfully deleted, punt the record.  I cannot see a     */
        /* problem with this, even though the recordIds could get reused        */
        sprintf( gBuf_p, "delete from echos_get where id=%lu", echo_p->recordId );
        if( egSqlExec( gBuf_p ) != TRUE ) goto exit;
    }

    returnCode = TRUE;
exit:
    mbFree( name_p );
    if( returnCode == FALSE ) egLog( "Delete failed\n" );
    return returnCode;
}

#endif

int egSystem( char *cmd_p, int pidFlag )
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
