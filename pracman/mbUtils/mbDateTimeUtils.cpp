//---------------------------------------------------------------------------
// File:    mbDateTimeUtils.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    Feb. 9, 2002
//---------------------------------------------------------------------------
// Description:
//
// Various date and time utility functions
//---------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <vcl.h>
#pragma hdrstop

#include "mbTypes.h"
#include "mbDebug.h"
#include "mbLock.h"
#include "mbQueue.h"
#include "mbMalloc.h"
#include "mbLog.h"
#include "mbDlg.h"
#include "mbDateTime.h"
#include "mbStrUtils.h"

#define MB_INIT_GLOBALS
#include "mbDateTimeUtils.h"
#undef  MB_INIT_GLOBALS

//---------------------------------------------------------------------------
// Function:    mbUptime
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Char_p  mbSecToTimeStr
(
    Int32u_t    seconds,
    Char_p      str_p
)
{
    Int32u_t    days = 0;
    Int32u_t    hours = 0;
    Int32u_t    minutes = 0;
    Ints_t      l = 0;

    for( days    = 0 ; seconds >= 86400; days++    ) { seconds -= 86400; }
    for( hours   = 0 ; seconds >= 3600;  hours++   ) { seconds -= 3600; }
    for( minutes = 0 ; seconds >= 60;    minutes++ ) { seconds -= 60; }

    if( str_p )
    {
        Char_t  buf[32];
        *str_p = 0;
        if( days )
        {
            sprintf( buf, "%lu day%s ", days, ( days == 1 ) ? "" : "s" );
            strcat( str_p, buf );
        }
        if( hours )
        {

            sprintf( buf, "%lu hour%s ", hours, ( hours == 1 ) ? "" : "s" );
            strcat( str_p, buf );
        }
        if( minutes )
        {
            sprintf( buf, "%lu minute%s ", minutes, ( minutes == 1 ) ? "" : "s" );
            strcat( str_p, buf );
        }
        if( seconds )
        {
            sprintf( buf, "%lu second%s ", seconds, ( seconds == 1 ) ? "" : "s" );
            strcat( str_p, buf );
        }
        mbStrClean( str_p, NIL, FALSE );
    }
    return str_p;
}

//---------------------------------------------------------------------------
// Function:    mbTime
//---------------------------------------------------------------------------
// Description: Returns the time in an integer format.
//---------------------------------------------------------------------------

Int32u_t        mbTime( void )
{
    time_t      timer;
    struct tm  *tblock;
    Ints_t      timeint;

    timer  = time( NULL );
    tblock = localtime( &timer );

    timeint  = tblock->tm_hour * 10000;
    timeint += tblock->tm_min * 100;
    timeint += tblock->tm_sec;

    return timeint;
}

//---------------------------------------------------------------------------
// Function: mbMsec
//---------------------------------------------------------------------------
// High precision timer
//---------------------------------------------------------------------------

Int32u_t    mbMsec( Void_t )
{
    LARGE_INTEGER   largeInt;
    Int64u_t        currentTime;
    Int32u_t        result;

    if( !mbPerformanceInit )
    {
        // Initialize high resolution counter
        Int64u_t        frequency;

        if(  QueryPerformanceFrequency( &largeInt ) == 0 )
        {
            mbDlgError(( "This computer does not have a high resolution timer." ));
        }
        else
        {
            frequency = largeInt.QuadPart;
            mbPerformanceTicksPerMsec = frequency / 1000L;
            QueryPerformanceCounter( &largeInt );
            mbPerformanceStartTime = largeInt.QuadPart;
        }
        mbPerformanceInit = TRUE;
    }

    QueryPerformanceCounter( &largeInt );

    currentTime  = largeInt.QuadPart;
    currentTime -= mbPerformanceStartTime;
    currentTime /= mbPerformanceTicksPerMsec;

    result = (Int32u_t)currentTime;

    return result;
}

//---------------------------------------------------------------------------
// Function:    mbIsLeapYear
//---------------------------------------------------------------------------
// Description: Determines if the specified year is a leap year
//---------------------------------------------------------------------------

Boolean_t  mbIsLeapYear( Int32u_t year )
{
    if( ( ( year % 4 ) == 0  &&  ( year % 100 ) != 0 ) || ( ( year % 400 ) == 0 ) )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//---------------------------------------------------------------------------
// Function:    mbDaysInMonth
//---------------------------------------------------------------------------
// Description: Determines how many days in the specified month
//---------------------------------------------------------------------------

Int32u_t        mbDaysInMonth
(
    Int32u_t    year,
    Int32u_t    month
)
{
    Ints_t      result = 0;

    if( month > 12 || month < 1 ) goto exit;

    result = mbDaysInMonthArray[ month ];

    if( month == 2 )
    {
        if( mbIsLeapYear( year ) )
        {
            result = 29;
        }
    }
exit:
    return result;
}

//---------------------------------------------------------------------------
// Function:    mbDayOfWeekYMD
//---------------------------------------------------------------------------
// Description: Determines the day of the week
//---------------------------------------------------------------------------

Int32u_t        mbDayOfWeek
(
    Int32u_t    date
)
{
    Int32u_t    year, month, day;

    year = date / 10000;
    date = date - (year * 10000);
    month = date / 100;
    date = date - (month * 100);
    day = date;
    return mbDayOfWeekX( year, month, day );
}

//---------------------------------------------------------------------------
// Function:    mbDayOfWeek
//---------------------------------------------------------------------------
// Description: Determines the day of the week
//---------------------------------------------------------------------------

Int32u_t        mbDayOfWeekX
(
    Int32u_t    year,
    Int32u_t    month,
    Int32u_t    day
)
{
    TDateTime  *dateTime_p;
    Int32u_t    result;

    if( month < 1 || month > 12 ) return 0;

    dateTime_p = new TDateTime( (Int16u_t)year, (Int16u_t)month, (Int16u_t)day );
    result = (Int32u_t)dateTime_p->DayOfWeek( );
    delete dateTime_p;

    return result;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t        mbToday( void )
{
    TDateTime  *dateTime_p;
    Int32u_t    result;
    Int16u_t    year;
    Int16u_t    month;
    Int16u_t    day;

    dateTime_p = new TDateTime( Date( ) );
    dateTime_p->DecodeDate( &year, &month, &day );
    result = (Int32u_t)year * 10000 + (Int32u_t)month * 100 + (Int32u_t)day;
    delete dateTime_p;
    return result;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p  mbDayStringShort( Int32u_t day )
{
    if( day < 1 || day > 7 ) day = 0;
    return mbDayStringsArrayShort[day];
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p  mbDayStringLong( Int32u_t day )
{
    if( day < 1 || day > 7 ) day = 0;
    return mbDayStringsArrayLong[day];
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p  mbMonthStringShort( Int32u_t month )
{
    if( month < 1 || month > 12 ) month = 0;
    return mbMonthStringsArrayShort[month];
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p  mbMonthStringLong( Int32u_t month )
{
    if( month < 1 || month > 12 ) month = 0;
    return mbMonthStringsArrayLong[month];
}

//---------------------------------------------------------------------------
// Function: mbStrTimeRemaining
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p  mbSecToTimeRemaining
(
    Int32u_t    seconds,
    Char_p      buf_p
)
{
    Int32u_t    hours;
    Int32u_t    minutes;

    hours = seconds / 3600;
    seconds = seconds - ( hours * 3600 );
    minutes = seconds / 60;
    seconds = seconds - ( minutes * 60 );

    if( hours )
    {
        sprintf( buf_p, "%d hour%s %d minute%s",
                    hours,
                    hours == 1 ? "" : "s",
                    minutes,
                    minutes == 1 ? "" : "s" );
    }
    else if( minutes )
    {
        if( minutes > 30 )
        {
            sprintf( buf_p, "%d minute%s",
                    minutes,
                    minutes == 1 ? "" : "s" );
        }
        else
        {
            sprintf( buf_p, "%d minute%s %d second%s",
                    minutes,
                    minutes == 1 ? "" : "s",
                    seconds,
                    seconds == 1 ? "" : "s" );
        }

    }
    else
    {
        sprintf( buf_p, "%d second%s",
                    seconds,
                    seconds == 1 ? "" : "s" );
    }

exit:
    return buf_p;
}


