//---------------------------------------------------------------------------
// Function:    mbDateTimeUtils.h
//---------------------------------------------------------------------------
// Author:      Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:        Oct. 30 2002
//---------------------------------------------------------------------------
// Description:
//
// Date and time utilities
//---------------------------------------------------------------------------

#ifndef mbDateTimeUtils_h
#define mbDateTimeUtils_h

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

External Int32u_t  mbDaysInMonthArray[]
#ifdef MB_INIT_GLOBALS
= { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
#endif
;

enum
{
     MB_DAY_INVALID_MIN = 0
    ,MB_DAY_SUNDAY
    ,MB_DAY_MONDAY
    ,MB_DAY_TUESDAY
    ,MB_DAY_WEDNESDAY
    ,MB_DAY_THURSDAY
    ,MB_DAY_FRIDAY
    ,MB_DAY_SATURDAY
    ,MB_DAY_IMVALID_MAX
};

External Char_p mbDayStringsArrayLong[]
#ifdef MB_INIT_GLOBALS
=
{
     "Invalid"
    ,"Sunday"
    ,"Monday"
    ,"Tuesday"
    ,"Wednesday"
    ,"Thursday"
    ,"Friday"
    ,"Saturday"
}
#endif
;

External Char_p mbDayStringsArrayShort[]
#ifdef MB_INIT_GLOBALS
=
{
     "Invalid"
    ,"Sun"
    ,"Mon"
    ,"Tue"
    ,"Wed"
    ,"Thu"
    ,"Fri"
    ,"Sat"
}
#endif
;

External Char_p mbMonthStringsArrayLong[]
#ifdef MB_INIT_GLOBALS
=
{
     "Invalid"
    ,"January"
    ,"February"
    ,"March"
    ,"April"
    ,"May"
    ,"June"
    ,"July"
    ,"August"
    ,"September"
    ,"October"
    ,"November"
    ,"December"
}
#endif
;

External Char_p mbMonthStringsArrayShort[]
#ifdef MB_INIT_GLOBALS
=
{
     "Invalid"
    ,"Jan"
    ,"Feb"
    ,"Mar"
    ,"Apr"
    ,"May"
    ,"Jun"
    ,"Jul"
    ,"Aug"
    ,"Sept"
    ,"Oct"
    ,"Nov"
    ,"Dec"
}
#endif
;

External    Boolean_t   mbPerformanceInit
#ifdef MB_INIT_GLOBALS
= FALSE
#endif
;

External    Int64u_t    mbPerformanceStartTime
#ifdef MB_INIT_GLOBALS
= 0
#endif
;

External    Int64u_t    mbPerformanceTicksPerMsec
#ifdef MB_INIT_GLOBALS
= 0
#endif
;

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

Int32u_t    mbTime( void );
Boolean_t   mbIsLeapYear( Int32u_t year );
Int32u_t    mbDaysInMonth( Int32u_t year, Int32u_t month );
Int32u_t    mbDayOfWeekX( Int32u_t year, Int32u_t month, Int32u_t day );
Int32u_t    mbDayOfWeek( Int32u_t date );
Int32u_t    mbToday( void );

Char_p      mbDayStringShort( Int32u_t day );
Char_p      mbDayStringLong( Int32u_t day );
Char_p      mbMonthStringShort( Int32u_t month );
Char_p      mbMonthStringLong( Int32u_t month );

Int32u_t    mbMsec( Void_t );

Char_p      mbSecToTimeRemaining( Int32u_t sec, Char_p buf_p );
Char_p      mbSecToTimeStr
(
    Int32u_t    seconds,
    Char_p      str_p
);

#endif // mbDateTimeUtils_h

