//---------------------------------------------------------------------------
// File:    pmcCalendar.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 6, 2001
//---------------------------------------------------------------------------
// Description:
//
// Functions for managing the calendar on the main form.
//---------------------------------------------------------------------------

#include <stdio.h>
#include <vcl.h>
#pragma hdrstop

#include "mbUtils.h"

#include "pmcGlobals.h"
#include "pmcMainForm.h"
#include "pmcUtils.h"

#pragma package(smart_init)
#pragma link "CCALENDR"

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//
//---------------------------------------------------------------------------


void __fastcall TMainForm::UpdateDateString( )
{
    char        buf[64];
    int         monthIndex;
    Int32u_t    weekStart;
    Int32u_t    monthStart;

    if( CalendarSkipUpdate ) return;

    monthIndex = Calendar->Month;
    if( monthIndex < 0 || monthIndex > 12 )
    {
        monthIndex = 0;
    }
    sprintf( buf, "%s %\d, %d", mbMonthStringLong( monthIndex ), Calendar->Day, Calendar->Year );

    SelectedDate  = Calendar->Year * 10000;
    SelectedDate += Calendar->Month * 100;
    SelectedDate += Calendar->Day;

//    SelectedDayOfWeek = mbDayOfWeek( (Int32u_t)Calendar->Year, (Int32u_t)Calendar->Month, (Int32u_t)Calendar->Day );
    SelectedDayOfWeek = mbDayOfWeek( SelectedDate );

    weekStart = SelectedWeekViewDate[0];
    monthStart = SelectedMonthViewDate[0];

    SelectedWeekViewDateUpdate( );

    // Allow update of month to be skipped
    if( !CalendarMonthSkipUpdate ) SelectedMonthViewDateUpdate( );

    YearScrollBar->Position = Calendar->Year;
    CalendarTitleLabel->Caption = buf;

    if( !CalendarSkipRefresh )
    {
        // Refresh apointment views
        DayViewGrid->Invalidate( );
        ProviderViewGrid->Invalidate( );
        if( weekStart  != SelectedWeekViewDate[0] )  WeekViewGrid->Invalidate( );
        if( monthStart != SelectedMonthViewDate[0] ) MonthViewGrid->Invalidate( );
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::TodayButtonClick( TObject *Sender )
{
    Calendar->UseCurrentDate = TRUE;

    // I don't know why the UpdateDateString...
    // method is not getting invoked when the
    // calendar is changed to use the current date

    // For some reason this function does not appear to work if the
    // day of the month is the same as today;
    {
        Int32u_t    year;
        Int32u_t    month;
        Int32u_t    day;
        Int32u_t    temp;
        Int32u_t    today = mbToday();

        year = today/10000;
        temp = today - year * 10000;
        month = temp/100;
        day = temp - month * 100;

        CalendarSkipUpdate = TRUE;

        Calendar->Month = month;
        Calendar->Year = year;
        Calendar->Day= day;

        CalendarSkipUpdate = FALSE;
    }
    UpdateDateString(  );
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::JanButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 1, Calendar->Day );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FebButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 2, Calendar->Day );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MarButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 3, Calendar->Day );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::AprButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 4, Calendar->Day );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MayButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 5, Calendar->Day );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::JuneButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 6, Calendar->Day );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::JulyButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 7, Calendar->Day );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::AugButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 8, Calendar->Day );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SeptButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 9, Calendar->Day );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::OctButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 10, Calendar->Day );
    Calendar->SetFocus();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::NovButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 11, Calendar->Day );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::DecButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 12, Calendar->Day );
    Calendar->SetFocus();
}

//-------------------------------------------------------------------------
// Function: TDateSelectForm::YearScrollBarChange( )
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::YearScrollBarChange(TObject *Sender)
{
    Int32u_t   year;

    year = YearScrollBar->Position;
    UpdateDate( year, Calendar->Month, Calendar->Day );
}

//---------------------------------------------------------------------------
// Function: TMainForm::UpdateDate( )
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::UpdateDate
(
    Int32u_t    year,
    Int32u_t    month,
    Int32u_t    day
)
{
    Int32u_t    daysInMonth;

    if( month > 12 ) month = 12;

    daysInMonth = Calendar->DaysPerMonth( year, month );
    if( day > daysInMonth )
    {
        day = daysInMonth;
    }

    CalendarSkipUpdate = TRUE;

    Calendar->Year = year;
    Calendar->Day = 1;
    Calendar->Month = month;
    Calendar->Day = day;

    CalendarSkipUpdate = FALSE;
    // Display string at top of form
    UpdateDateString( );
}

//---------------------------------------------------------------------------
// Function: TMainForm::UpdateDateInt( )
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::UpdateDateInt
(
    Int32u_t    dateIn
)
{
    Int32u_t    daysInMonth;
    Int32u_t    year;
    Int32u_t    month;
    Int32u_t    day;
    Int32u_t    temp;

    year = dateIn/10000;
    temp = dateIn - year * 10000;
    month = temp/100;
    day = temp - month * 100;

    if( month > 12 ) month = 12;

    daysInMonth = Calendar->DaysPerMonth( year, month );
    if( day > daysInMonth )
    {
        day = daysInMonth;
    }

    CalendarSkipUpdate = TRUE;
    Calendar->Year = year;
    Calendar->Day = 1;
    Calendar->Month = month;
    Calendar->Day = day;
    CalendarSkipUpdate = FALSE;

    CalendarSkipRefresh = TRUE;
    UpdateDateString( );
    CalendarSkipRefresh = FALSE;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::CalendarChange(TObject *Sender)
{
    UpdateDateString( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::SelectedWeekViewDateUpdate( void )
{
    // Must compute the range of dates for the week view
    Int32u_t    year;
    Int32u_t    month;
    Int32u_t    day;
    Int32u_t    dayOfWeek;
    Int32u_t    startYear;
    Int32u_t    startMonth;
    Int32u_t    startDay;
    Int32u_t    daysInMonth;
    MbDateTime  dateTime;
    Int32u_t    i;

    dateTime.SetDate( SelectedDate );

    year  = dateTime.Year();
    month = dateTime.Month();
    day   = dateTime.Day();

    dayOfWeek   = mbDayOfWeek( SelectedDate );

    if( day >= dayOfWeek )
    {
        // The start of the week's range will be in the same month
        startYear = year;
        startMonth = month;
        startDay = (1 + day - dayOfWeek);
    }
    else
    {
        // The start of the week's range will be in the previous month
        if( --month == 0 )
        {
            year--;
            month = 12;
        }
        startDay = (1 + mbDaysInMonth( year, month ) - (dayOfWeek - day ) );
        startYear = year;
        startMonth = month;
    }

    daysInMonth = mbDaysInMonth( (Int32u_t)startYear, (Int32u_t)startMonth );

    //NDBG_MBOX(( "start date: Year: %d Month: %d Date: %d DayOfWeek: %d DaysInMonth: %d\n",
    //        startYear, startMonth, startDay, pmcDayOfWeek( startYear, startMonth, startDay ), daysInMonth ));

    // Compute all dates in the week's range
    for( i = 0 ; i < 7 ; i++ )
    {
        SelectedWeekViewYear[i]  = startYear;
        SelectedWeekViewMonth[i] = startMonth;
        SelectedWeekViewDay[i]   = startDay;

        SelectedWeekViewDate[i] = (Int32u_t)startYear * 10000 + (Int32u_t)startMonth * 100 + (Int32u_t)startDay;

        if( ++startDay > daysInMonth )
        {
            startDay = 1;
            if( ++startMonth > 12 )
            {
                startMonth = 1;
                startYear++;
            }
        }
    }
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::SelectedMonthViewDateUpdate( void )
{
    // Must compute the range of dates for the month view.
    // Start at beginning of week of selected day.

    Int32u_t    year;
    Int32u_t    month;
    Int32u_t    day;
    Int32u_t    dayOfWeek;
    Int32u_t    startYear;
    Int32u_t    startMonth;
    Int32u_t    startDay;
    Int32u_t    daysInMonth;
    MbDateTime  dateTime;
    Int32u_t    i;

    dateTime.SetDate( SelectedDate );

    year  = dateTime.Year();
    month = dateTime.Month();
    day   = dateTime.Day();

    dayOfWeek   = mbDayOfWeek( SelectedDate );

    //NDBG_MBOX(( "SelectedDate: %ld Year: %d Month: %d Date: %d DayOfWeek: %d\n",
    //        SelectedDate, year, month, day, dayOfWeek ));

    if( day >= dayOfWeek )
    {
        // The start of the range will be in the same month
        startYear = year;
        startMonth = month;
        startDay = (Int16u_t)(1 + day - dayOfWeek);
    }
    else
    {
        // The start of the range will be in the previous month
        if( --month == 0 )
        {
            year--;
            month = 12;
        }
        startDay = (Int16u_t)( 1 + mbDaysInMonth( year, month ) - ( dayOfWeek - day ) );
        startYear = year;
        startMonth = month;
    }

    daysInMonth = mbDaysInMonth( startYear, startMonth );

    //mbLog( "start date: Year: %lu Month: %lu Date: %lu DayOfWeek: %lu DaysInMonth: %lu\n",
    //        startYear, startMonth, startDay, dayOfWeek, daysInMonth );

    // Compute all dates in the range
    for( i = 0 ; i < 31 ; i++ )
    {
        SelectedMonthViewDay[i]  = startDay;
        SelectedMonthViewDate[i] = (Int32u_t)startYear * 10000 + (Int32u_t)startMonth * 100 + (Int32u_t)startDay;
        SelectedMonthViewDateEnd = SelectedMonthViewDate[i];

        if( ++startDay > daysInMonth )
        {
            startDay = 1;
            if( ++startMonth > 12 )
            {
                startMonth = 1;
                startYear++;
            }
            // MAB:20030208: Must recompute the days in the month...
            // looping through 31 days can span two month boundaries
            daysInMonth = mbDaysInMonth( startYear, startMonth );
        }
    }
    return;

#if 0
    // Must compute the range of dates for the month view
    // This code computes the range for the current month (i.e., from the
    // first to the last day of the month
    Int16u_t    year;
    Int16u_t    month;
    Int16u_t    daysInMonth;
    MDateTime  *dateTime_p;
    Ints_t      i;

    dateTime_p = new MDateTime( SelectedDate, 0 );
    year        = dateTime_p->Year();
    month       = dateTime_p->Month();
    delete dateTime_p;

    daysInMonth = pmcDaysInMonth( year, month );

    for( i = 0 ; i < 31 ; i++ )
    {
        if( i < daysInMonth )
        {
            SelectedMonthViewDate[i] = (Int32u_t)year * 10000 + (Int32u_t)month * 100 + (Int32u_t)( i + 1 );
            SelectedMonthViewDateEnd = SelectedMonthViewDate[i];
        }
        else
        {
            SelectedMonthViewDate[i] = 0;
        }
    }
    return;
#endif
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::CalendarKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
    // pmcLog( "Key down, date %ld key %ld", SelectedDate, Key );
    KeyDownDate = SelectedDate;
    KeyDownKey = (Int32u_t)Key;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::CalendarKeyUp(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
    Int32u_t    year, month, day;
    Int32u_t    daysInMonth;
    bool        update = FALSE;

    if( SelectedDate != KeyDownDate ) return;

    if( (Int32u_t)Key == KeyDownKey )
    {
        month = (Int32u_t)Calendar->Month;
        year =  (Int32u_t)Calendar->Year;
        day =   (Int32u_t)Calendar->Day;

        switch( Key )
        {
            case 40:
                // Want to add 7 days to display
                daysInMonth = mbDaysInMonth( year, month );

                if( ( daysInMonth - day ) < 7 )
                {
                    month++;
                    if( month > 12 )
                    {
                        year++;
                        month = 1;
                    }
                    day = (7 - ( daysInMonth - day ));
                    update = TRUE;
                }
                break;

            case 37:
                if( day == 1 )
                {
                    month--;
                    if( month == 0 )
                    {
                        month = 12;
                        year--;
                    }
                    day = mbDaysInMonth( year, month );
                    update = TRUE;
                }
                else
                {
                    day--;
                    update = TRUE;
                }

                break;

            case 39:
                // pmcLog( "Right, date %ld key %ld", SelectedDate, Key );
                if( day == mbDaysInMonth( year, month ) )
                {
                    month++;
                    day = 1;
                    if( month > 12 )
                    {
                        month = 1;
                        year++;
                    }
                    update = TRUE;
                }
                else
                {
                    day++;
                    update = TRUE;
                }
                break;

            case 38:
                // Want to subtract 7 days from date

                if( day <= 7 )
                {
                    month--;
                    if( month == 0 )
                    {
                        month = 12;
                        year--;
                    }
                    daysInMonth = mbDaysInMonth( year, month );
                    day = (daysInMonth - ( 7 - day ) );
                    update = TRUE;
                }
                break;
            case 33:
                // Page up - go to previous month
                month--;
                if( month == 0 )
                {
                    month = 12;
                    year--;
                }
                update = TRUE;
                break;
            case 34:
                month++;
                if( month > 12 )
                {
                    month = 1;
                    year++;
                }
                update = TRUE;
                break;
        }
        if( update ) UpdateDate( year, month, day );
    }
}
//---------------------------------------------------------------------------

