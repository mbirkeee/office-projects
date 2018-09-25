//---------------------------------------------------------------------------
// File:    pmcDateSelectForm.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    Feb. 15, 2001
//---------------------------------------------------------------------------
// Description:
//
// This file contains functions for updating the internal list of patients.
// The internal list is maintained to speed searching for patient records.
//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "mbUtils.h"

#include "pmcDateSelectForm.h"
#include "pmcGlobals.h"
#include "pmcUtils.h"

#pragma package(smart_init)
#pragma link "CCALENDR"
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
// Function: TDateSelectForm::TDateSelectForm( )
//---------------------------------------------------------------------------
// Description:
//
// Default constructor
//---------------------------------------------------------------------------

__fastcall TDateSelectForm::TDateSelectForm(TComponent* Owner)
    : TForm(Owner)
{
}


//---------------------------------------------------------------------------
// Function: TDateSelectForm::TDateSelectForm( )
//---------------------------------------------------------------------------
// Description:
//
// Constructor allowing starting date to be specified
//---------------------------------------------------------------------------

__fastcall TDateSelectForm::TDateSelectForm
(
    TComponent             *Owner,
    pmcDateSelectInf_p     dateSelectInfo_p
)
    : TForm(Owner)
{
    Int32u_t                mode;

    if( dateSelectInfo_p )
    {
        DateSelectInfo_p = dateSelectInfo_p;
        if( DateSelectInfo_p->caption_p )
        {
            Caption = DateSelectInfo_p->caption_p;
        }
    }
    else
    {
        mbDlgDebug(( "Called without info struct!" ));
        return;
    }

    ClearButton->Enabled = ( dateSelectInfo_p->clearEnabled == TRUE ) ? TRUE : FALSE;
    mode = DateSelectInfo_p->mode;
    ClearFlag = FALSE;

    if( mode == PMC_DATE_SELECT_TODAY )
    {
        Calendar->UseCurrentDate = TRUE;
    }
    else if( mode == PMC_DATE_SELECT_PARMS )
    {
        MbDateTime   dateTime;

        // Display today's date if a value of 0 is input
        if( DateSelectInfo_p->dateIn == 0 ) DateSelectInfo_p->dateIn = mbToday( );

        dateTime.SetDate( DateSelectInfo_p->dateIn );
        UpdateDate( dateTime.Year(), dateTime.Month(), dateTime.Day() );
    }
    else
    {
        // Invalid mode
    }

    SkipUpdate = FALSE;
    UpdateDateString( );

    OrigDate  = Calendar->Day;
    OrigMonth = Calendar->Month;
    OrigYear  = Calendar->Year;

    YearScrollBar->Position = Calendar->Year;
}

//---------------------------------------------------------------------------
// Function: TDateSelectForm::FormClose( )
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::FormClose(TObject *Sender, TCloseAction &Action)
{
    Action = caFree;
}

//---------------------------------------------------------------------------
// Function: TDateSelectForm::UpdateDate( )
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::UpdateDate
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

    SkipUpdate = TRUE;
    Calendar->Day = 1;
    Calendar->Month = month;
    Calendar->Day = day;
    Calendar->Year = year;

    SkipUpdate = FALSE;
    // Display string at top of form
    UpdateDateString( );
}

//---------------------------------------------------------------------------
// Function: TDateSelectForm::UpdateDateString( )
//---------------------------------------------------------------------------
// Description:
//
// Display date in label box at top of form
//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::UpdateDateString( )
{
    char    buf[64];
    int     monthIndex;

    monthIndex = Calendar->Month;
    if( monthIndex < 0 || monthIndex > 12 )
    {
        monthIndex = 0;
    }
    sprintf( buf, "%s %d, %d", mbMonthStringLong( monthIndex ), Calendar->Day, Calendar->Year );

    SelectedDate  = Calendar->Year * 10000;
    SelectedDate += Calendar->Month * 100;
    SelectedDate += Calendar->Day;

    YearScrollBar->Position = Calendar->Year;

    DateLabel->Caption = buf;
    ClearFlag = FALSE;
}

//---------------------------------------------------------------------------
// Function: TDateSelectForm::OkButtonClick( )
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::OkButtonClick(TObject *Sender)
{
    CalendarDone( );
}

//---------------------------------------------------------------------------
// Cancel button cicked, restore original date
//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::CancelButtonClick(TObject *Sender)
{
    MbDateTime dateTime = MbDateTime( OrigYear, OrigMonth, OrigDate, 0, 0, 0 );

    UpdateDate( OrigYear, OrigMonth, OrigDate );

    if( DateSelectInfo_p )
    {
        DateSelectInfo_p->dateOut = dateTime.Date();

        if( DateSelectInfo_p->string_p )
        {
            strcpy( DateSelectInfo_p->string_p, DateLabel->Caption.c_str() );
        }
        DateSelectInfo_p->returnCode = MB_BUTTON_CANCEL;
    }

    // Should have put a comment here
    ModalResult = 1;
    Release( );
}

//-------------------------------------------------------------------------
// Function: TDateSelectForm::YearScrollBarChange( )
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::YearScrollBarChange(TObject *Sender)
{
    Int32u_t   year;
    nbDlgDebug(( "ScrollBarChange called, position %d", YearScrollBar->Position));
    year = YearScrollBar->Position;
    UpdateDate( year, Calendar->Month, Calendar->Day );
}

//---------------------------------------------------------------------------
// Function: TDateSelectForm::TodayButtonClick( )
//---------------------------------------------------------------------------
// Description:
//
// Set selected date to current date (today).
//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::TodayButtonClick(TObject *Sender)
{
    Calendar->UseCurrentDate = TRUE;
    UpdateDateString(  );
}

//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::JanButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 1, Calendar->Day );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::FebButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 2, Calendar->Day );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::MarButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 3, Calendar->Day );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::AprButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 4, Calendar->Day );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::MayButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 5, Calendar->Day );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::JuneButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 6, Calendar->Day );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::JulyButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 7, Calendar->Day );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::AugButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 8, Calendar->Day );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::SeptButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 9, Calendar->Day );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::OctButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 10, Calendar->Day );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::NovButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 11, Calendar->Day );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::DecButtonClick(TObject *Sender)
{
    UpdateDate( Calendar->Year, 12, Calendar->Day );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::CalendarChange(TObject *Sender)
{
    if( !SkipUpdate ) UpdateDate( Calendar->Year, Calendar->Month, Calendar->Day );
}
//---------------------------------------------------------------------------


void __fastcall TDateSelectForm::CalendarKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{
    //pmcLog( "Key down, date %ld key %ld", SelectedDate, Key );
    KeyDownDate = SelectedDate;
    KeyDownKey = (Int32u_t)Key;
}
//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::CalendarKeyUp(TObject *Sender, WORD &Key,
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

void __fastcall TDateSelectForm::CalendarDblClick(TObject *Sender)
{
    CalendarDone( );
}
//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::CalendarDone( void )
{
    MbDateTime dateTime = MbDateTime( Calendar->Year, Calendar->Month, Calendar->Day, 0, 0, 0 );

    if( DateSelectInfo_p )
    {
        if( ClearFlag )
        {
            DateSelectInfo_p->dateOut = 0;
            if( DateSelectInfo_p->string_p )
            {
                strcpy( DateSelectInfo_p->string_p, "" );
            }
        }
        else
        {
            DateSelectInfo_p->dateOut = dateTime.Date();
            if( DateSelectInfo_p->string_p )
            {
                strcpy( DateSelectInfo_p->string_p, DateLabel->Caption.c_str() );
            }
        }
        DateSelectInfo_p->returnCode = MB_BUTTON_OK;
    }

    ModalResult = 1;
    Release();
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TDateSelectForm::ClearButtonClick(TObject *Sender)
{
    ClearFlag = TRUE;
    DateLabel->Caption = "";
}
//---------------------------------------------------------------------------

