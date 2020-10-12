/******************************************************************************
 * Copyright (c) 2007, Michael A. Bree
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 * $Header: /home/cvs/mbUtilsLinux/src/mbLock.c,v 1.2 2007/07/18 03:55:42 mikeb Exp $
 *-----------------------------------------------------------------------------
 * $Log: mbLock.c,v $
 * Revision 1.2  2007/07/18 03:55:42  mikeb
 * Fix lock file
 *
 * Revision 1.1.1.1  2007/07/10 05:06:03  mikeb
 * Initial Import
 *
 *-----------------------------------------------------------------------------
 */

/* System include files */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "mbUtils.h"

/******************************************************************************
 * Function: mbLockFileGet
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Int32s_t    mbLockFileGet( Char_p lockFile_p )
{
    Int32s_t        returnCode = FALSE;
    Boolean_t       foundPid = FALSE;
    FILE           *fp = NIL;
    FILE           *fp2 = NIL;
    Char_p          buf1_p = mbMalloc( 1024 );
    Char_p          buf2_p = mbMalloc( 1024 );
    Int32u_t        checkPid;

    fp = fopen( lockFile_p, "r" );
    if( fp )
    {
        /* Get pid from lockfile  */
        fgets( buf1_p, 1020, fp );
        checkPid = atol( buf1_p );
        fgets( buf1_p, 1020, fp );
        mbStrCleanWhite( buf1_p, NIL );

        mbLog( "Found existing lock file PID: %lu command: '%s'\n", checkPid, buf1_p );

        sprintf( buf2_p, "/proc/%lu/cmdline", checkPid );

        fp2 = fopen( buf2_p, "r" );
        if( fp2 )
        {
            fgets( buf2_p, 1020, fp2 );
            mbStrCleanWhite( buf2_p, NIL );

            if( strcmp( buf2_p, buf1_p ) == 0 )
            {
                foundPid = TRUE;
            }
            fclose( fp2 );
        }

        fclose( fp );

        if( foundPid == TRUE )
        {
            mbLog( "Found existing process with PID: %lu\n", checkPid );
            goto exit;
        }
        else
        {
            mbLog( "Deleting stale lockfile with PID: %lu\n", checkPid );
            unlink( lockFile_p );
        }
    }

    /* At this point we are going to run and should create a pid file */
    fp = fopen( lockFile_p, "w" );
    if( fp == NIL )
    {
        mbLog( "Failed to create lockfile\n" );
        goto exit;
    }
    else
    {
        Int32u_t    myPid = (Int32u_t)getpid();

        sprintf( buf2_p, "/proc/%lu/cmdline", myPid );
        if( ( fp2 = fopen( buf2_p, "r" ) ) == NIL )
        {
            mbLog( "Failed to create lockfile\n" );
            fclose( fp );
            unlink( lockFile_p );
            goto exit;
        }

        fgets( buf2_p, 1020, fp2 );
        mbStrCleanWhite( buf2_p, NIL );
        fclose( fp2 );

        fprintf( fp, "%lu\n", myPid );
        fprintf( fp, "%s\n", buf2_p );
        fclose( fp );
    }
    returnCode = TRUE;

exit:

    mbFree( buf1_p );
    mbFree( buf2_p );

    return returnCode;
}



