//---------------------------------------------------------------------------
// File:    mbLog.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    Oct. 31, 2002
//---------------------------------------------------------------------------
// Description:
//
// Simple logging function
//---------------------------------------------------------------------------

#include <stdio.h>
#include <time.h>
#include <vcl.h>    // needed for TDateTime
#pragma hdrstop

#include "mbTypes.h"
#include "mbMalloc.h"
#include "mbStrUtils.h"

#define MB_INIT_GLOBALS
#include "mbLog.h"
#undef  MB_INIT_GLOBALS

//---------------------------------------------------------------------------
// Function:    mbLogInit
//---------------------------------------------------------------------------
// Description: simple logging utility
//---------------------------------------------------------------------------

Ints_t mbLogInit
(
    Char_p      name_p,
    Char_p      directory_p,
    Int32u_t    pid
)
{
    Ints_t      returnCode = FALSE;

    if( mbLogInitFlag != FALSE ) goto exit;

    if( name_p == NIL ) goto exit;

    mbLogName_p = (Char_p)malloc( strlen(  name_p ) + 1 );
    strcpy( mbLogName_p, name_p );

    if( directory_p )
    {
        mbLogDir_p = (Char_p)malloc( strlen( directory_p ) + 1 );
        strcpy( mbLogDir_p, directory_p );
    }
    else
    {
        mbLogDir_p = NIL;
    }

    mbLogYear = 0;
    mbLogMonth = 0;
    mbLogDay = 0;
    mbLogPid = pid;
    mbLogInitFlag = TRUE;

exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:    mbLogTerminate
//---------------------------------------------------------------------------
// Description: simple logging utility
//---------------------------------------------------------------------------

Ints_t mbLogTerminate( void )
{
    free( mbLogName_p );
    free( mbLogDir_p );
    free( mbLogFileName_p );
    mbLogInitFlag = FALSE;
    return TRUE;
}

//---------------------------------------------------------------------------
// Function:    mbLog
//---------------------------------------------------------------------------
// Description: simple logging utility
//---------------------------------------------------------------------------

Ints_t mbLog
(
    Char_p  string_p,
    ...
)
{
    va_list             arg;
    Char_p              str_p = NIL;
    FILE               *fp = NIL;
    time_t              timer;
    struct tm          *tblock;
    Ints_t              len;
    TDateTime          *dateTime_p;
    Int16u_t            year = 0;
    Int16u_t            month = 0;
    Int16u_t            day = 0;
    Ints_t              returnCode = FALSE;

    if( mbLogName_p == NIL ) goto exit;

    // Allocate big buffer
    str_p = (Char_p)malloc( 50000 );
    if( str_p == NULL ) goto exit;

    // Get the date
    dateTime_p = new TDateTime( Date( ) );
    dateTime_p->DecodeDate( &year, &month, &day );
    delete dateTime_p;

    // Check to see if the filename should be regenerated
    if(    year  != (Int16u_t)mbLogYear
        || month != (Int16u_t)mbLogMonth
        || day   != (Int16u_t)mbLogDay
        || mbLogFileName_p == NIL )
    {
        Ints_t  len;

        mbLogYear  = (Int32u_t)year;
        mbLogMonth = (Int32u_t)month;
        mbLogDay   = (Int32u_t)day;

        *str_p = 0;
        if( mbLogDir_p )
        {
            mbStrRemoveSlashTrailing( mbLogDir_p );
   
            sprintf( str_p, "%s\\", mbLogDir_p );
        }
        len  = strlen( str_p );
        len += sprintf( str_p + len, "%s_%04d%02d%02d.log", mbLogName_p, year, month, day );

        if( mbLogFileName_p ) free( mbLogFileName_p );
        mbLogFileName_p = (Char_p)malloc( len + 1 );
        strcpy( mbLogFileName_p, str_p );
    }

    if( mbLogFileName_p == 0 ) goto exit;

    timer  = time( NULL );
    tblock = localtime( &timer );

    va_start( arg, string_p );
    vsprintf( str_p, string_p, arg );
    va_end( arg );

    fp = fopen( mbLogFileName_p, "a" );
    if( fp == NIL ) goto exit;

    len = strlen( str_p );
    if( len == 0 ) goto exit;

    // Add a carriage return to the end of the line if there isn't one already
    if( *(str_p + len - 1 ) != '\n' ) strcat( str_p, "\n" );

    fprintf( fp, "%02d:%02d:%02d %lu %s", tblock->tm_hour, tblock->tm_min, tblock->tm_sec, mbLogPid, str_p );

    returnCode = TRUE;

exit:
    if( fp )            fclose( fp );
    if( str_p )         free( str_p );
    return returnCode;
}

