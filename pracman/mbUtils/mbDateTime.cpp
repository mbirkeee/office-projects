//---------------------------------------------------------------------------
// mbDateTime.cpp
//---------------------------------------------------------------------------
// (c) 2001 Michael A. Bree
//---------------------------------------------------------------------------
// Object for manipulating date/time and returning formatted strings
//---------------------------------------------------------------------------

#include <stdio.h>
#include <vcl.h>
#pragma hdrstop

#include "mbUtils.h"

#define MB_INIT_GLOBALS
#include "mbDateTime.h"
#undef  MB_INIT_GLOBALS

//---------------------------------------------------------------------------
// Constructor: Set time according to input 64-bit value
//---------------------------------------------------------------------------

__fastcall MbDateTime::MbDateTime( Int64u_t in )
{
    this->mem_q = qInitialize( &this->memQueue );
    this->SetDateTime64( in );
    return;
}

//---------------------------------------------------------------------------
// Constructor: Set time according to input 32-bit date and 32-bit time ints
//---------------------------------------------------------------------------

__fastcall MbDateTime::MbDateTime( Int32u_t dateIn, Int32u_t timeIn )
{
    Int32u_t    temp;

    this->mem_q = qInitialize( &this->memQueue );

    temp = dateIn/10000;

    iYear = (Int32s_t)temp;

    dateIn = dateIn - ( temp * 10000 );

    temp = dateIn/100;

    iMonth = (Int32s_t)temp;

    dateIn = dateIn - ( temp * 100 );

    iDay = (Int32s_t)dateIn;

    temp = timeIn/10000;

    iHour = (Int32s_t) temp;

    timeIn = timeIn - ( temp * 10000 );

    temp = timeIn/100;

    iMin = (Int32s_t)temp;

    timeIn = timeIn - (temp * 100 );

    iSec = (Int32s_t)timeIn;

    sprintf( iSqlStr, "0" );
    iSqlInt = 0i64;

    iDateInt = (Int32u_t)( (Int32u_t)iYear * (Int32u_t)10000 ) + ( (Int32u_t)iMonth * (Int32u_t)100 ) + ( (Int32u_t)iDay );
    iTimeInt = (Int32u_t)( (Int32u_t)iHour * (Int32u_t)10000 ) + ( (Int32u_t)iMin   * (Int32u_t)100 ) + ( (Int32u_t)iSec );

    return;
}

//---------------------------------------------------------------------------
// Constructor: Initialize object to all values 0
//---------------------------------------------------------------------------

__fastcall MbDateTime::MbDateTime( void )
{
    this->mem_q = qInitialize( &this->memQueue );

    iYear = 0;
    iMonth = 0;
    iDay = 0;
    iHour = 0;
    iMin = 0;
    iSec = 0;
    sprintf( iSqlStr, "0" );
    iSqlInt = 0i64;
    iDateInt = 0;
    iTimeInt = 0;
    return;
}

//---------------------------------------------------------------------------
// Compute iDate
//---------------------------------------------------------------------------

void __fastcall MbDateTime::iDate( void )
{
    iDateInt = (Int32u_t)( (Int32u_t)iYear * (Int32u_t)10000 ) + ( (Int32u_t)iMonth * (Int32u_t)100 ) + ( (Int32u_t)iDay );
}

//---------------------------------------------------------------------------
// Compute iTime
//---------------------------------------------------------------------------

void __fastcall MbDateTime::iTime( void )
{
    iTimeInt = (Int32u_t)( (Int32u_t)iHour * (Int32u_t)10000 ) + ( (Int32u_t)iMin   * (Int32u_t)100 ) + ( (Int32u_t)iSec );
}

//---------------------------------------------------------------------------
// Constructor: Set time according to year, month, etc.
//---------------------------------------------------------------------------

__fastcall MbDateTime::MbDateTime
(
    Int32u_t    year,
    Int32u_t    month,
    Int32u_t    day,
    Int32u_t    hour,
    Int32u_t    min,
    Int32u_t    sec
)
{
    this->mem_q = qInitialize( &this->memQueue );

    iYear  = (Int32s_t)year;
    iMonth = (Int32s_t)month;
    iDay   = (Int32s_t)day;
    iHour  = (Int32s_t)hour;
    iMin   = (Int32s_t)min;
    iSec   = (Int32s_t)sec;

    this->iDate();
    this->iTime();

    return;
}

//---------------------------------------------------------------------------
// Set time according to input 64-bit int
//---------------------------------------------------------------------------

void __fastcall MbDateTime::SetDateTime64( Int64u_t in )
{
    Int64u_t temp;
    // __int64 temp1 = in;
    temp = in/10000000000ui64;
    iYear = (Int32s_t)temp;
    in = in - ( temp * 10000000000ui64 );
    temp = in/100000000ui64;
    iMonth = (Int32s_t)temp;
    in = in - ( temp * 100000000ui64 );
    temp = in/1000000ui64;
    iDay = (Int32s_t)temp;
    in = in - (  temp * 1000000ui64 );
    temp = in/10000ui64;
    iHour = (Int32s_t) temp;
    in = in - ( temp * 10000ui64 );
    temp = in/100ui64;
    iMin = (Int32s_t)temp;
    in = in - (temp * 100ui64 );
    iSec = (Int32s_t)in;

    this->iDate();
    this->iTime();

    return;
}

//---------------------------------------------------------------------------
// Set the time back one minute. Date can change.
//---------------------------------------------------------------------------

void __fastcall MbDateTime::BackOneMinute( void )
{
    if( iMonth == 0 )
    {
        // Has not yet been initialized;
    }
    else
    {
        iMin--;
        if( iMin < 0 )
        {
            iMin = 59;
            iHour--;
            if( iHour < 0 )
            {
                iHour = 23;
                iDay--;
                if( iDay == 0 )
                {
                    iMonth --;
                    if( iMonth == 0 )
                    {
                        iMonth = 12;
                        iYear--;
                    }
                    iDay = (Int32s_t)mbDaysInMonth( (Int32u_t)iYear, (Int32u_t)iMonth );
                }
            }
        }

        this->iDate();
        this->iTime();
    }
}

//---------------------------------------------------------------------------
// Set the time forward the specified number of minutes. Date can change.
//---------------------------------------------------------------------------

void __fastcall MbDateTime::AddMinutes( Int32u_t minutes )
{
    iMin += (Int32s_t)minutes;

    while( iMin >= 60 )
    {
        iMin -= (Int16s_t)60;
        iHour++;
    }

    while( iHour >= 24 )
    {
        iHour -= (Int32s_t)24;
        iDay++;
        if( iDay > (Int32s_t)mbDaysInMonth( (Int32u_t)iYear, (Int32u_t)iMonth ) )
        {
            iDay = 1;
            iMonth++;

            if( iMonth > 12 )
            {
                iMonth = 1;
                iYear++;
            }
        }
    }

    this->iDate();
    this->iTime();
}

//---------------------------------------------------------------------------
// Return date in format November 14, 2008  7:42 PM
//---------------------------------------------------------------------------

Char_p __fastcall MbDateTime::MDYHM_DateStringLong( void )
{
    Char_p  buf_p;
    mbMalloc( buf_p, 256 );

    sprintf( buf_p, "%s %s", this->MDY_DateStringLong( ), this->HM_TimeString(  ) );

    // Store this item for later freeing
    mbStrListTake( this->mem_q, buf_p );
    return buf_p;
}

//---------------------------------------------------------------------------
// Return time string formatted "HH:MM AM/PM"
//---------------------------------------------------------------------------

Char_p __fastcall MbDateTime::HM_TimeString( void )
{
    Char_t      am[] = "AM";
    Char_t      pm[] = "PM";
    Char_p      ampm;
    Int16s_t    hours;
    Char_p      buf_p;

    mbMalloc( buf_p, 256 );

    ampm = ( iHour >= 12 ) ? pm : am;

    hours = ( (Int32s_t)iHour > (Int32s_t)12 ) ? (Int32s_t)(iHour - 12) : iHour;

    if( hours == 0 ) hours = 12;

    sprintf( buf_p, "%d:%02d %s", hours, iMin, ampm );

    // Store this item for later freeing
    mbStrListTake( this->mem_q, buf_p );

exit:
    return buf_p;
}

//---------------------------------------------------------------------------
// Return time string formatted "HH:MM:SS"
//---------------------------------------------------------------------------

Char_p __fastcall MbDateTime::HMS_TimeString( void )
{
    Char_p  buf_p;
    mbMalloc( buf_p, 256 );
    sprintf( buf_p, "%02d:%02d:%02d", iHour, iMin, iSec );

    // Store this item for later freeing
    mbStrListTake( this->mem_q, buf_p );

    return buf_p;
}
//---------------------------------------------------------------------------
// Is this method obsolete?
//---------------------------------------------------------------------------

Char_p __fastcall MbDateTime::DMY_DateStringWrong( void )
{
    Char_p  buf_p;
    mbMalloc( buf_p, 256 );

    sprintf( buf_p, "%s%s %d, %04d",
        mbMonthStringsArrayShort[ iMonth ],
        ( iMonth == 5 ) ? "" : ".",
        iDay, iYear );

    // Store this item for later freeing
    mbStrListTake( this->mem_q, buf_p );

    return buf_p;
}

//---------------------------------------------------------------------------
// Return string formatted "Sat. Nov. 15, 2008"
//---------------------------------------------------------------------------

Char_p __fastcall MbDateTime::DMDY_DateString( void )
{
    Char_p  buf_p;
    mbMalloc( buf_p, 256 );

    sprintf( buf_p, "%s. %s%s %d, %04d", mbDayStringsArrayShort[ mbDayOfWeekX( iYear, iMonth, iDay ) ],
        mbMonthStringsArrayShort[ iMonth ],
        ( iMonth == 5 ) ? "" : ".",
        iDay, iYear );

    // Store this item for later freeing
    mbStrListTake( this->mem_q, buf_p );
    return buf_p;
}

//---------------------------------------------------------------------------
// Return string formatted "2008/11/15"
//---------------------------------------------------------------------------

Char_p __fastcall MbDateTime::DigitsDateString( void )
{
    Char_p  buf_p;
    mbMalloc( buf_p, 256 );

    sprintf( buf_p, "%04d/%02d/%02d", iYear, iMonth, iDay );

    // Store this item for later freeing
    mbStrListTake( this->mem_q, buf_p );
    return buf_p;
}

//---------------------------------------------------------------------------
// Resturn string formatted "Saturday, November 15, 2008"
//---------------------------------------------------------------------------

Char_p __fastcall MbDateTime::DMY_DateStringLong( void )
{
    Char_p  buf_p;
    mbMalloc( buf_p, 256 );

    sprintf( buf_p, "%s, %s %d, %04d", mbDayStringsArrayLong[ mbDayOfWeekX( iYear, iMonth, iDay ) ],
        mbMonthStringsArrayLong[ iMonth ], iDay, iYear );

    // Store this item for later freeing
    mbStrListTake( this->mem_q, buf_p );
    return buf_p;
}

//---------------------------------------------------------------------------
// Return string formatted "Saturday"
//---------------------------------------------------------------------------

Char_p __fastcall MbDateTime::Day_DateString( void )
{
    Char_p  buf_p;
    mbMalloc( buf_p, 256 );

    sprintf( buf_p, "%s", mbDayStringsArrayLong[ mbDayOfWeekX( iYear, iMonth, iDay ) ] );

    // Store this item for later freeing
    mbStrListTake( this->mem_q, buf_p );
    return buf_p;
}

//---------------------------------------------------------------------------
// Return string formatted "2008"
//---------------------------------------------------------------------------

Char_p __fastcall MbDateTime::Year_DateString( void )
{
    Char_p  buf_p;
    mbMalloc( buf_p, 256 );

    sprintf( buf_p, "%04d", iYear );

    // Store this item for later freeing
    mbStrListTake( this->mem_q, buf_p );
    return buf_p;
}

//---------------------------------------------------------------------------
// Return string formatted "Nov. 15"
//---------------------------------------------------------------------------

Char_p __fastcall MbDateTime::MD_DateString( void )
{
    Char_p  buf_p;
    mbMalloc( buf_p, 256 );

    *buf_p = NULL;

    if( iMonth >= 1 && iMonth <= 12 )
    {
        sprintf( buf_p, "%s%s %d",
            mbMonthStringsArrayShort[ iMonth ],
            (iMonth == 5 ) ? "" : ".",
            iDay );
    }

    // Store this item for later freeing
    mbStrListTake( this->mem_q, buf_p );
    return buf_p;
}

//---------------------------------------------------------------------------
// Return string formatted "Nov. 15, 2008"
//---------------------------------------------------------------------------

Char_p __fastcall MbDateTime::MDY_DateString( void )
{
    Char_p  buf_p;
    mbMalloc( buf_p, 256 );

    *buf_p = NULL;

    if( iMonth >= 1 && iMonth <= 12 )
    {
        sprintf( buf_p, "%s%s %d, %04d",
            mbMonthStringsArrayShort[ iMonth ],
            (iMonth == 5 ) ? "" : ".",
            iDay, iYear );
    }

    // Store this item for later freeing
    mbStrListTake( this->mem_q, buf_p );
    return buf_p;
}

//---------------------------------------------------------------------------
// Return string formatted "November 15, 2008"
//---------------------------------------------------------------------------

Char_p __fastcall MbDateTime::MDY_DateStringLong( void )
{
    Char_p  buf_p;
    mbMalloc( buf_p, 256 );

    sprintf( buf_p, "%s %d, %04d",
        mbMonthStringsArrayLong[ iMonth ],
        iDay, iYear );

    // Store this item for later freeing
    mbStrListTake( this->mem_q, buf_p );
    return buf_p;
}

//---------------------------------------------------------------------------
// Return string formatted "2008-Nov-15"
//---------------------------------------------------------------------------

Char_p __fastcall MbDateTime::YMD_DateString( void )
{
    Char_p  buf_p;
    mbMalloc( buf_p, 256 );

    sprintf( buf_p, "%04d-%s-%02d", iYear, mbMonthStringsArrayShort[ iMonth ], iDay, iYear );

    // Store this item for later freeing
    mbStrListTake( this->mem_q, buf_p );
    return buf_p;
}

//---------------------------------------------------------------------------
// Set date/time according top specified inputs
//---------------------------------------------------------------------------

void __fastcall MbDateTime::SetDateTime( Int32u_t date, Int32u_t time )
{
    this->SetDate( date );
    this->SetTime( time );
}

//---------------------------------------------------------------------------
// Set time according to 32-bit int (sec since midnight). Date not affected
//---------------------------------------------------------------------------

void __fastcall MbDateTime::SetTime( Int32u_t time )
{
    Int32u_t    hours;
    Int32u_t    minutes;
    Int32u_t    seconds;

    hours = time / 10000;

    time = time - (hours * 10000);

    minutes = time / 100;
    time = time - (minutes * 100);

    seconds = time;

    nbDlgDebug(( "hours minutes seconds: %d %d %d", hours, minutes,seconds ));

    iHour = (Int32s_t)hours;
    iMin  = (Int32s_t)minutes;
    iSec  = (Int32s_t)seconds;

    this->iDate();
    this->iTime();
}

//---------------------------------------------------------------------------
// Compute age (i.e., years back) from today
//---------------------------------------------------------------------------

Char_p __fastcall MbDateTime::AgeString( Int32u_t date )
{
    Int32s_t    today = (Int32s_t)date;
    Int32s_t    year;
    Int32s_t    month;
    Int32s_t    day;
    Int32s_t    age = 0;
    Char_p      buf_p;

    year  = today / 10000;
    today = today - (year * 10000);
    month = today / 100;
    today = today - (month * 100);
    day   = today;

    mbMalloc( buf_p, 16 );

    // This algorithm probably does not handle leap years.
    age = year - iYear;

    if( month <= iMonth )
    {
        if( month == iMonth )
        {
            if( day < iDay )
            {
                age--;
            }
        }
        else
        {
            age--;
        }
    }

    if( age >= 0 )
    {
        sprintf( buf_p, "%d", age );
    }
    else
    {
        sprintf( buf_p, "" );
    }

    // Store this item for later freeing
    mbStrListTake( this->mem_q, buf_p );
    return buf_p;
}

//---------------------------------------------------------------------------
// Set date according to int; e.g.: 20081115.  Time is not affected
//---------------------------------------------------------------------------

void __fastcall MbDateTime::SetDate( Int32u_t date )
{
    Int32u_t    year;
    Int32u_t    month;
    Int32u_t    day;

    year = date / 10000;
    date = date - (year * 10000);
    month = date / 100;
    date = date - (month * 100);
    day = date;

    if( month > 12 )
    {
        // mbDlgDebug(( "invalid month: %d", month ));
    }

    if( day > (Int32u_t)mbDaysInMonth( (Int16u_t)year, (Int16u_t)month ) )
    {
        // mbDlgDebug(( "invalid day (year: %d month: %d day: %d)", year, month, day ));
    }

    iYear  = (Int32s_t)year;
    iMonth = (Int32s_t)month;
    iDay   = (Int32s_t)day;

    this->iDate();
    this->iTime();
}

//---------------------------------------------------------------------------
// This function sets the time according to the passed in MySQL time string
//---------------------------------------------------------------------------
void __fastcall MbDateTime::MysqlStringSet( Char_p in_p )
{
    int     i = 0;
    int     count = 0;
    int     val;
    Char_p  p;

    if( in_p == NIL ) goto exit;

    iYear   = 0;
    iMonth  = 0;
    iDay    = 0;
    iHour   = 0;
    iMin    = 0;
    iSec    = 0;

    for( p = in_p ; *p != NULL ; p++ )
    {
        if( *p < '0' || *p > '9' ) continue;

        val = *p - '0';

        if( count < 4 ) // Year
        {
            iYear *= 10;
            iYear += val;
        }
        else if( count < 6 ) // Month
        {
            iMonth *= 10;
            iMonth += val;

        }
        else if( count < 8 ) // Day
        {
            iDay *= 10;
            iDay += val;
        }
        else if( count < 10 ) // Hour
        {
            iHour *= 10;
            iHour += val;
        }
        else if( count < 12 ) // Min
        {
            iMin *= 10;
            iMin += val;
        }
        else if( count < 14 )
        {
            iSec *= 10;
            iSec += val;
        }
        else
        {
            // Error
        }
        count++;
    }

    this->iDate();
    this->iTime();

exit:
    return;
}
//---------------------------------------------------------------------------
// Set date/time according to Variant varTime.  Convert to known format
// using DateTime.decode
//---------------------------------------------------------------------------

void __fastcall MbDateTime::Update( Variant *varTime_p )
{
    TDateTime   dateTime;
    Int16u_t    year = 0;
    Int16u_t    month = 0;
    Int16u_t    day = 0;
    Int16u_t    hour = 0;
    Int16u_t    minute = 0;
    Int16u_t    second = 0;
    Int16u_t    msec = 0;

    if( varTime_p->Type() == 7 && !varTime_p->IsNull() && !varTime_p->IsEmpty() )
    {
        // Convert Variant to TDateTime object
        dateTime = VarToDateTime( varTime_p );

        // Must call Decode Date 'cause format may change according to system settings
        dateTime.DecodeDate( &year, &month, &day );

        dateTime.DecodeTime( &hour, &minute, &second, &msec );

        nbDlgDebug(( "update called year %d month %d date %d hour %d min %d sec %d",
            year, month, day, hour, minute, second  ));

        if( year == 1899 && month == 12 && day == 30 )
        {
            year = 0;
            month = 0;
            day = 0;
        }
    }

    iYear    = (Int32s_t)year;
    iMonth   = (Int32s_t)month;
    iDay     = (Int32s_t)day;
    iHour    = (Int32s_t)hour;
    iMin     = (Int32s_t)minute;
    iSec     = (Int32s_t)second;

    this->iDate();
    this->iTime();

    return;
}

//---------------------------------------------------------------------------
// Format string for use in sql statments
//---------------------------------------------------------------------------

void __fastcall MbDateTime::FormatSqlString( void )
{
    sprintf( iSqlStr, "%04d%02d%02d%02d%02d%02d",
        iYear, iMonth, iDay, iHour, iMin, iSec );

    return;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall MbDateTime::FormatInt64( void )
{
    __int64 temp;

    iSqlInt = iYear;
    iSqlInt *= 10000000000i64;

    temp = iMonth;
    temp *= 100000000i64;
    iSqlInt += temp;

    temp = iDay;
    temp *= 1000000i64;
    iSqlInt += temp;

    temp = iHour;
    temp *= 10000i64;
    iSqlInt += temp;

    temp = iMin;
    temp *= 100i64;
    iSqlInt += temp;

    temp = iSec;
    iSqlInt += temp;

    return;
}

//---------------------------------------------------------------------------
// Copy time from another MbDateTime object
//---------------------------------------------------------------------------

void __fastcall MbDateTime::Clone( MbDateTime *in_p )
{
    iYear    = in_p->iYear;
    iMonth   = in_p->iMonth;
    iDay     = in_p->iDay;
    iHour    = in_p->iHour;
    iMin     = in_p->iMin;
    iSec     = in_p->iSec;

    this->iDate();
    this->iTime();

    return;
}

//---------------------------------------------------------------------------
// Set date to the 1st of the previous month
//---------------------------------------------------------------------------

Int32u_t __fastcall MbDateTime::StartPrevMonth( void )
{
    iMonth--;
    if( iMonth == 0 )
    {
        iYear--;
        iMonth = 12;
    }
    iDay = 1;

    this->iDate();
    this->iTime();

    return iDateInt;
}

//---------------------------------------------------------------------------
// Set date back specified number of years
//---------------------------------------------------------------------------

Int32u_t __fastcall MbDateTime::BackYears( Int32u_t count )
{
    for( Int32u_t i = 0 ; i != count ; i++ )
    {
        if( iYear ) iYear--;
    }

    this->iDate();
    this->iTime();

    return iDateInt;
}

//---------------------------------------------------------------------------
// Set date back specified number of months
//---------------------------------------------------------------------------

Int32u_t __fastcall MbDateTime::BackMonths( Int32u_t count )
{
    for( Int32u_t i = 0 ; i != count ; i++ )
    {
        if( iMonth > 1 )
        {
            iMonth--;
        }
        else
        {
            iMonth = 12;
            if( iYear ) iYear--;
        }
    }

    this->iDate();
    this->iTime();

    return iDateInt;
}

//---------------------------------------------------------------------------
// Destructor... free any allocated memory
//---------------------------------------------------------------------------

__fastcall MbDateTime::~MbDateTime( )
{
    mbStrListFree( this->mem_q );
    return;
}


