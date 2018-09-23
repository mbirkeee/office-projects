//---------------------------------------------------------------------------
// Function: pmcDateTime.h
//---------------------------------------------------------------------------
// Date: Feb. 15, 2001
//---------------------------------------------------------------------------
// Description:
//
// Customized Date Time class for PMC application
//---------------------------------------------------------------------------

#ifndef mbDateTime_h
#define mbDateTime_h

#include "mbTypes.h"

#ifdef  External
#undef  External
#endif

#ifdef  MB_INIT_GLOBALS
#define External
#else
#define External extern
#endif

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------

class MbDateTime
{
public:
    // Constructors
    __fastcall  MbDateTime( void );
    __fastcall  MbDateTime( Int64u_t );
    __fastcall  MbDateTime( Int32u_t dateIn, Int32u_t timeIn );
    __fastcall  MbDateTime
    (
        Int32u_t    year,
        Int32u_t    month,
        Int32u_t    day,
        Int32u_t    hour,
        Int32u_t    min,
        Int32u_t    sec
    );

    // Destructor
    __fastcall  ~MbDateTime( void );

    Int32u_t    __fastcall Year()    { return (Int32u_t)iYear; }
    Int32u_t    __fastcall Month()   { return (Int32u_t)iMonth;}
    Int32u_t    __fastcall Day()     { return (Int32u_t)iDay;  }
    Int32u_t    __fastcall Hour()    { return (Int32u_t)iHour; }
    Int32u_t    __fastcall Min()     { return (Int32u_t)iMin;  }
    Int32u_t    __fastcall Sec()     { return (Int32u_t)iSec;  }
    Int32u_t    __fastcall Time()    { return iTimeInt; }
    Int32u_t    __fastcall Date()    { return iDateInt; }

    Char_p      __fastcall SqlString( ) { FormatSqlString(); return iSqlStr; }
    Int64u_t    __fastcall Int64( )     { FormatInt64(); return iSqlInt; }

    void        __fastcall MysqlStringSet( Char_p in_p );
    void        __fastcall Update( Variant *dateTime_p );
    void        __fastcall BackOneMinute( void );
    void        __fastcall AddMinutes( Int32u_t minutes );
    void        __fastcall Clone( MbDateTime *source );
    void        __fastcall SetTime( Int32u_t time );
    void        __fastcall SetDate( Int32u_t date );
    void        __fastcall SetDateTime( Int32u_t date, Int32u_t time );
    void        __fastcall SetDateTime64( Int64u_t dateTime );

    Char_p      __fastcall HM_TimeString( void );
    Char_p      __fastcall HMS_TimeString( void );
    Char_p      __fastcall DMY_DateStringWrong( void );
    Char_p      __fastcall DMDY_DateString( void );
    Char_p      __fastcall DMY_DateStringLong( void );
    Char_p      __fastcall MDY_DateString( void );
    Char_p      __fastcall MDY_DateStringLong( void );
    Char_p      __fastcall YMD_DateString( void );
    Char_p      __fastcall DigitsDateString( void );
    Char_p      __fastcall MD_DateString( void );
    Char_p      __fastcall Day_DateString( void );
    Char_p      __fastcall Year_DateString( void );
    Char_p      __fastcall MDYHM_DateStringLong( void );
    Char_p      __fastcall AgeString( Int32u_t date );

    Int32u_t    __fastcall StartPrevMonth( void );
    Int32u_t    __fastcall BackYears( Int32u_t count );
    Int32u_t    __fastcall BackMonths( Int32u_t count );

private:

    void        __fastcall FormatSqlString( void );
    void        __fastcall FormatInt64( void );
    void        __fastcall iDate( void );
    void        __fastcall iTime( void );

    Char_t      iSqlStr[32];
    Int32s_t    iYear;
    Int32s_t    iMonth;
    Int32s_t    iDay;
    Int32s_t    iHour;
    Int32s_t    iMin;
    Int32s_t    iSec;
    Int64u_t    iSqlInt;
    Int32u_t    iTimeInt;
    Int32u_t    iDateInt;

    // Queue for keeping track of allocated memory in this class
    qHead_t     memQueue;
    qHead_p     mem_q;
};

#endif

