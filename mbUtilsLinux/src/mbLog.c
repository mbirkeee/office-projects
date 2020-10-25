/******************************************************************************
 * Copyright (c) 2007, Michael A. Bree
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 * $Header: /home/cvs/mbUtilsLinux/src/mbLog.c,v 1.1.1.1 2007/07/10 05:06:03 mikeb Exp $
 *-----------------------------------------------------------------------------
 * $Log: mbLog.c,v $
 * Revision 1.1.1.1  2007/07/10 05:06:03  mikeb
 * Initial Import
 *
 *-----------------------------------------------------------------------------
 */

/* System include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Project include files */
#include "mbUtils.h"

/* Global variables */
Char_p   gProgName_p = NIL;
Char_p   gLogDir_p = NIL;
MbLock_t gLogLock;
MbLock_p gLogLock_p = NIL;
Int32u_t gPid = 0;

/******************************************************************************
 * Function: mbLogInit
 *-----------------------------------------------------------------------------
 * Description:
 *
 *
 *-----------------------------------------------------------------------------
 */

Ints_t mbLogInit( Char_p progName_p, Char_p logDir_p )
{
    mbFree( gProgName_p );

    if( progName_p )
    {
        gProgName_p = mbMallocStr( progName_p );
    }
    else
    {
        gProgName_p = mbMallocStr( "LOG" );
    }

    mbFree( gLogDir_p );

    if( logDir_p )
    {
        gLogDir_p = mbMallocStr( logDir_p );
    }
    else
    {
        Char_t  buf[MAX_NAME_LEN];
        gLogDir_p = mbMallocStr( getcwd( buf, MAX_NAME_LEN ) );
    }

    if( gLogLock_p == NIL )
    {
        gLogLock_p = MB_LOCK_INIT( gLogLock );
    }

    if( gPid == 0 )
    {
        gPid = (Int32u_t)getpid();
    }
    return TRUE;
}

/******************************************************************************
 * Function: mbLog
 *-----------------------------------------------------------------------------
 * Description:
 *
 *
 *-----------------------------------------------------------------------------
 */

Ints_t mbLog
(
    Char_p  string_p,
    ...
)
{
    va_list             arg;
    Char_p              str_p = NULL;
    Char_t              timeStr[64];
    Char_t              dateStr[64];
    Char_t              logFileName[MAX_NAME_LEN];
    FILE               *fp;
    Ints_t              returnCode = FALSE;

    if( gLogDir_p == NIL || gProgName_p == NIL )
    {
        fprintf( stdout, "ERROR: Logging system not inititalized\n" );
        goto exit;
    }

    if( ( str_p = (Char_p)mbMalloc( 1024 ) ) == 0 ) goto exit;

    MB_LOCK_ACQUIRE( gLogLock_p );

    va_start( arg, string_p );
    vsprintf( str_p, string_p, arg );
    va_end( arg );

    mbCharTime( timeStr );
    mbCharDate( dateStr );

    sprintf( logFileName, "%s/%s-%s.log", gLogDir_p, dateStr, gProgName_p );

    fp = fopen( logFileName, "a" );
    if( fp )
    {
        fprintf( fp, "%s: %ld: %s", timeStr, gPid, str_p );
        fclose( fp );

        fprintf( stdout, "%s: %ld: %s", timeStr, gPid, str_p );
        fflush( stdout );
    }

    mbFree( str_p );

    MB_LOCK_RELEASE( gLogLock_p );

    returnCode = TRUE;

exit:

    return returnCode;
}

/******************************************************************************
 * Function: mbCharTime
 *-----------------------------------------------------------------------------
 * Description:
 *
 *
 *-----------------------------------------------------------------------------
 */

Char_p mbCharTime( Char_p buf_p )
{
    time_t      tm;
    struct  tm *tm_p;

    tm = time( NIL );
    tm_p = localtime( &tm );

    if( buf_p )
    {
        sprintf( buf_p, "%02d:%02d:%02d", tm_p->tm_hour, tm_p->tm_min,tm_p->tm_sec );
    }
    return buf_p;
}

/******************************************************************************
 * Function: egCharDate
 *-----------------------------------------------------------------------------
 * Description:
 *
 *
 *-----------------------------------------------------------------------------
 */

Char_p mbCharDate( Char_p buf_p )
{
    time_t      tm;
    struct  tm *tm_p;

    tm = time( NIL );
    tm_p = localtime( &tm );

    if( buf_p )
    {
        sprintf( buf_p, "%04d-%02d-%02d", 1900 + tm_p->tm_year ,tm_p->tm_mon+1, tm_p->tm_mday );
    }
    return buf_p;
}

/******************************************************************************
 * Function: egToday
 *-----------------------------------------------------------------------------
 * Description:
 *
 *
 *-----------------------------------------------------------------------------
 */

Int32u_t mbToday( void )
{
    time_t              tm;
    struct tm          *tm_p;
    Int32u_t            today;

    tm = time( NIL );
    tm_p = localtime( &tm );

    today  = 1900 + tm_p->tm_year;
    today *= 100;
    today += tm_p->tm_mon+1;
    today *= 100;
    today += tm_p->tm_mday;

    return today;
}

/******************************************************************************
 * Function: egToday
 *-----------------------------------------------------------------------------
 * Description:
 *
 *
 *-----------------------------------------------------------------------------
 */

Ints_t mbSystem( Char_p cmd_p, Ints_p pid_p )
{
    Ints_t pid, status;

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

        execve( "/bin/sh", argv, 0 );
        exit(127);
    }

    if( pid_p ) *pid_p = pid;

    do
    {
        fprintf( stdout, "calling waitpid (%s)\n", cmd_p);
        fflush( stdout );
        if( waitpid( pid, &status, 0 ) == -1 )
        {
            if( errno != EINTR )
            {
                if( pid_p ) *pid_p = 0;

                fprintf(stdout, "returning -1\n");
                fflush(stdout);
                return -1;
            }
            fprintf( stdout, "errno: %d\n", errno );
            fflush( stdout );
            sleep( 1 );
        }
        else
        {
            if( pid_p ) *pid_p = 0;
            fprintf(stdout, "returning status: %d\n", status);
            fflush(stdout);
            return status;
        }
    }
    while(1);
}
