//---------------------------------------------------------------------------
// File:    pmcMonthView.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 6, 2001
//---------------------------------------------------------------------------
// Description:
//
// Functions for managing the main Practice Manager form.
//---------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <vcl.h>
#pragma hdrstop

#include "mbUtils.h"

#include "pmcTables.h"
#include "pmcColors.h"
#include "pmcGlobals.h"
#include "pmcUtils.h"
#include "pmcMainForm.h"
#include "pmcDateSelectForm.h"

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::MonthViewTimeGridDrawCell
(
    TObject    *Sender,
    int         ACol,
    int         ARow,
    TRect      &Rect,
    TGridDrawState State
)
{
    TimeGridDrawCell( Sender, ACol, ARow, Rect, State, MonthViewTimeGrid );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::MonthViewGridDrawCell
(
    TObject        *Sender,
    int             ACol,
    int             ARow,
    TRect          &Rect,
    TGridDrawState  State
)
{
    Char_t          buf[32];
    AnsiString      str = "";
    TColor          color;
    Ints_t          col, colStart, colStop;
    TRect           iRect;

    // Don't draw screen if patients list has not been read yet
    if( UpdatePatientListDone == FALSE || UpdateDoctorListDone == FALSE ) return;

    if( ARow == 0 && ACol == 0 && Active == TRUE )
    {
        // Now actually call function to read the appointment information
        if( MonthViewInfoUpdate( &MonthViewInfo ) )
        {
            MonthViewInfoCellConstruct( );
        }

        if( CheckTables == TRUE )
        {
            RefreshTable( PMC_TABLE_BIT_PATIENTS );
        }
        else
        {
            CheckTables = TRUE;
        }
    }

    // Display the day of the week in the first row
    if( ARow == 0 )
    {
        buf[0] = 0;
        if( SelectedMonthViewDay[ACol] )
        {
            // Compute display string
            sprintf( buf, "%d", SelectedMonthViewDay[ACol] );
        }
        str = buf;
        MonthViewGrid->Canvas->TextRect( Rect, Rect.Left + 3, Rect.Top, str );
    }

    if( ARow > 0 )
    {
        //If this is the selected row then draw the whole thing
        //if( ARow == DayViewGrid->Row )
        //{
        //    colStart = 0;
        //    colStop = PMC_DAY_VIEW_ARRAY_COLS;
        //}
        //else
        //{
           colStart = ACol;
           colStop = ACol + 1;
        //}

        for( col = colStart ; col < colStop ; col++ )
        {
            iRect = MonthViewGrid->CellRect( col, ARow );
            // Sanity check
            if( ARow >=  PMC_TIMESLOTS_PER_DAY || ACol >=  31 )
            {
                mbDlgDebug(( "Invalid row %ld col %ld  in month view", ARow, ACol ));
            }

            color = (TColor)MonthViewCellArray[col][ARow].color;

            // if( ARow == DayViewGrid->Row ) { PMC_COLOR_DARKEN( color ); }

            MonthViewGrid->Canvas->Brush->Color = color;
            MonthViewGrid->Canvas->FillRect( iRect );
            MonthViewGrid->Canvas->TextRect( iRect, iRect.Left + 2, iRect.Top, MonthViewCellArray[col][ARow].str );

            // Make color solid across multiple time slots
            //if( MonthViewGrid->Canvas->Brush->Color != pmcColor[ PMC_COLOR_AVAIL ] &&
            //    MonthViewGrid->Canvas->Brush->Color != pmcColor[ PMC_COLOR_BREAK ] &&
            //   !MonthViewCellArray[col][ARow].last)
            //{
            //    MonthViewGrid->Canvas->Pen->Color = MonthViewGrid->Canvas->Brush->Color;
            //    MonthViewGrid->Canvas->MoveTo( iRect.Left  , iRect.Bottom );
            //    MonthViewGrid->Canvas->LineTo( iRect.Right , iRect.Bottom );
            //}

            if( MonthViewGrid->Canvas->Brush->Color != pmcColor[ PMC_COLOR_AVAIL ] &&
                MonthViewGrid->Canvas->Brush->Color != pmcColor[ PMC_COLOR_BREAK ] )
            {
                Ints_t  subtract;
                color = clBlack;

                subtract = ( MonthViewCellArray[col][ARow].last ) ? 1 : 0 ;
                MonthViewGrid->Canvas->Pen->Color = color;

                MonthViewGrid->Canvas->MoveTo( iRect.Left,  iRect.Top  );
                MonthViewGrid->Canvas->LineTo( iRect.Left,  iRect.Bottom - subtract  );
                MonthViewGrid->Canvas->MoveTo( iRect.Right - 1 , iRect.Top  );
                MonthViewGrid->Canvas->LineTo( iRect.Right - 1, iRect.Bottom - subtract  );
            }

            if( MonthViewCellArray[col][ARow].last )
            {
                color = clBlack;
                MonthViewGrid->Canvas->Pen->Color = color;
                MonthViewGrid->Canvas->MoveTo( iRect.Left  , iRect.Bottom - 1 );
                MonthViewGrid->Canvas->LineTo( iRect.Right , iRect.Bottom - 1 );
            }

            if( MonthViewCellArray[col][ARow].first && ARow > 1 )
            {
                color = clBlack;
                MonthViewGrid->Canvas->Pen->Color = color;
                MonthViewGrid->Canvas->MoveTo( iRect.Left  , iRect.Top  );
                MonthViewGrid->Canvas->LineTo( iRect.Right , iRect.Top  );
            }
        }
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::MonthViewGridMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    Ints_t          row = 0, col = 0;
    TRect           cellRect;

    if( MonthViewInfo.notReady ) return;

    // Convert co-ordinates to cell
    MonthViewGrid->MouseToCell( X, Y, col, row );
    nbDlgDebug(( "Mouse Down in row %d col %d (X: %d Y: %d)\n", row, col, X, Y ));

    if( row < 1 || col < 0 || col >= MonthViewInfo.cols )
    {
        MouseInRow = 0;
        MouseInCol = 0;
        return;
    }

    PMC_INC_USE( MonthViewInfo.notReady  );

    cellRect = MonthViewGrid->CellRect( col, row );

    AppViewInfoUpdate( row, col, &MonthViewInfo, 0 );

    // Update the date
    CalendarMonthSkipUpdate = TRUE;
    UpdateDateInt( SelectedMonthViewDate[col] );
    CalendarMonthSkipUpdate = FALSE;

    //LastMonthDate = SelectedMonthViewDate[col];

    if( SelectedMonthViewDate[col] >= mbToday( ) )
    {
        if( Button == mbLeft )
        {
#if 0
            index = col * PMC_TIME_SLOTS_PER_DAY + row;
            // Only start a drag operation if click on an appointment
            count = MonthViewInfo.slot_p[index].count;
            if( count != 0 )
            {
                appointId = MonthViewInfo.slot_p[ index ].appointId[ count ];
                if( appointId != 0 )
                {
                    MonthViewGrid->Cursor = crDrag;
                    MonthViewGrid->BeginDrag( TRUE, 0 );
                }
            }
#else
            MonthViewGrid->Cursor = crDrag;
            MonthViewGrid->BeginDrag( TRUE, 0 );
#endif
        }
    }

    // Check if mouse click near top or bottom of cell
    if( Y > ( ( cellRect.bottom + cellRect.top ) / 2  ))
    {
        MouseInRowTop = FALSE;
    }
    else
    {
        MouseInRowTop = TRUE;
    }
    // Record what row and col the mouse is in
    MouseInRow = row;
    MouseInCol = col;

    PMC_DEC_USE( MonthViewInfo.notReady );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::MonthViewGridDragOver(TObject *Sender,
      TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
    Ints_t      col, row;

    Accept = true;
    MonthViewGrid->MouseToCell( X, Y, col, row );

    if( row > 0 && col >= 0 )
    {
        MonthViewGrid->Row = row;
        MonthViewGrid->Col = col;
    }
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::MonthViewGridEndDrag(TObject *Sender,
      TObject *Target, int X, int Y)
{
    // Restore cursor no matter what
    MonthViewGrid->Cursor = crDefault;

    if( MonthViewInfo.notReady ) return;

    // Set flag to false to prevent action on other mouse clicks until done
    PMC_INC_USE( MonthViewInfo.notReady  );
    MonthViewInfo.endDragProviderId = SelectedProviderId;
    AppViewGridEndDrag( MonthViewGrid, &MonthViewInfo, X, Y );
    PMC_DEC_USE( MonthViewInfo.notReady );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::MonthViewGridSelectCell(TObject *Sender,
      int ACol, int ARow, bool &CanSelect)
{
    if( ARow > 0 )
    {
        CanSelect = TRUE;
    }
    else
    {
        CanSelect = FALSE;
    }
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::MonthViewGridStartDrag(TObject *Sender,
      TDragObject *&DragObject)
{
    MonthViewGrid->Cursor = crDrag;
}




