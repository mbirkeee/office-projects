//---------------------------------------------------------------------------
// File:    pmcWeekView.cpp
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

void __fastcall TMainForm::WeekViewTimeGridDrawCell
(
    TObject *Sender,
    int ACol,
    int ARow,
    TRect &Rect,
    TGridDrawState State
)
{
    TimeGridDrawCell( Sender, ACol, ARow, Rect, State, WeekViewTimeGrid );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::WeekViewGridDrawCell
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
//        Calendar->SetFocus();

        // Now actually call function to read the appointment information
        if( WeekViewInfoUpdate( &WeekViewInfo ) )
        {
            WeekViewInfoCellConstruct( );
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
        // Compute display string
        sprintf( buf, "%s %s %d",
            mbDayStringShort( ACol+1 ),
            mbMonthStringShort( SelectedWeekViewMonth[ACol] ),
            SelectedWeekViewDay[ACol] );

        str = buf;
        WeekViewGrid->Canvas->TextRect( Rect, Rect.Left + 3, Rect.Top, str );
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
            iRect = WeekViewGrid->CellRect( col, ARow );
            // Sanity check
            if( ARow >=  PMC_TIMESLOTS_PER_DAY || ACol >=  7 )
            {
                mbDlgDebug(( "Invalid row %ld col %ld  in day view", ARow, ACol ));
            }

            color = (TColor)WeekViewCellArray[col][ARow].color;

            // if( ARow == DayViewGrid->Row ) { PMC_COLOR_DARKEN( color ); }

            WeekViewGrid->Canvas->Brush->Color =  color;
            WeekViewGrid->Canvas->FillRect(iRect);
            WeekViewGrid->Canvas->TextRect( iRect, iRect.Left + 2, iRect.Top, WeekViewCellArray[col][ARow].str );

            // Make color solid across multiple time slots
            //if( WeekViewGrid->Canvas->Brush->Color != pmcColor[ PMC_COLOR_AVAIL ] &&
            //    WeekViewGrid->Canvas->Brush->Color != pmcColor[ PMC_COLOR_BREAK ] &&
            //   !WeekViewCellArray[col][ARow].last)
            //{
            //    WeekViewGrid->Canvas->Pen->Color = WeekViewGrid->Canvas->Brush->Color;
            //    WeekViewGrid->Canvas->MoveTo( iRect.Left  , iRect.Bottom );
            //    WeekViewGrid->Canvas->LineTo( iRect.Right , iRect.Bottom );
            //}

            if( WeekViewGrid->Canvas->Brush->Color != pmcColor[ PMC_COLOR_AVAIL ] &&
                WeekViewGrid->Canvas->Brush->Color != pmcColor[ PMC_COLOR_BREAK ] )
            {
                Ints_t  subtract;
                color = clBlack;

                subtract = ( WeekViewCellArray[col][ARow].last ) ? 1 : 0 ;
                WeekViewGrid->Canvas->Pen->Color = color;

                WeekViewGrid->Canvas->MoveTo( iRect.Left,  iRect.Top  );
                WeekViewGrid->Canvas->LineTo( iRect.Left,  iRect.Bottom - subtract  );
                WeekViewGrid->Canvas->MoveTo( iRect.Right - 1 , iRect.Top  );
                WeekViewGrid->Canvas->LineTo( iRect.Right - 1, iRect.Bottom - subtract  );
            }

            if( WeekViewCellArray[col][ARow].last )
            {
                color = clBlack;
                WeekViewGrid->Canvas->Pen->Color = color;
                WeekViewGrid->Canvas->MoveTo( iRect.Left  , iRect.Bottom - 1 );
                WeekViewGrid->Canvas->LineTo( iRect.Right , iRect.Bottom - 1 );
            }

            if( WeekViewCellArray[col][ARow].first && ARow > 1 )
            {
                color = clBlack;
                WeekViewGrid->Canvas->Pen->Color = color;
                WeekViewGrid->Canvas->MoveTo( iRect.Left  , iRect.Top  );
                WeekViewGrid->Canvas->LineTo( iRect.Right , iRect.Top  );
            }
        }
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::WeekViewGridMouseDown
(
    TObject        *Sender,
    TMouseButton    Button,
    TShiftState     Shift,
    int             X,
    int             Y
)
{
    Ints_t          row = 0, col = 0;
    TRect           cellRect;

    if( WeekViewInfo.notReady ) return;

    // Convert co-ordinates to cell
    WeekViewGrid->MouseToCell( X, Y, col, row );
    nbDlgDebug(( "Mouse Down in row %d col %d (X: %d Y: %d)\n", row, col, X, Y ));

    if( row < 1 || col < 0 || col >= WeekViewInfo.cols )
    {
        MouseInRow = 0;
        MouseInCol = 0;
        return;
    }

    PMC_INC_USE( WeekViewInfo.notReady );

    cellRect = WeekViewGrid->CellRect( col, row );

    AppViewInfoUpdate( row, col, &WeekViewInfo, 0 );

    // Update the date
    UpdateDateInt( SelectedWeekViewDate[col] );

    if( SelectedWeekViewDate[col] >= mbToday( ) )
    {
        if( Button == mbLeft )
        {
#if 0
            index = col * PMC_TIME_SLOTS_PER_DAY + row;
            // Only start a drag operation if click on an appointment
            count = WeekViewInfo.slot_p[index].count;
            if( count != 0 )
            {
                appointId = WeekViewInfo.slot_p[ index ].appointId[ count ];
                if( appointId != 0 )
                {
                    WeekViewGrid->Cursor = crDrag;
                    WeekViewGrid->BeginDrag( TRUE, 0 );
                }
            }
#else
            WeekViewGrid->Cursor = crDrag;
            WeekViewGrid->BeginDrag( TRUE, 0 );
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

    PMC_DEC_USE( WeekViewInfo.notReady );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::WeekViewGridEndDrag(TObject *Sender,
      TObject *Target, int X, int Y)
{
    // Restore cursor no matter what
    WeekViewGrid->Cursor = crDefault;

    if( WeekViewInfo.notReady ) return;

    // Set flag to false to prevent action on other mouse clicks until done
    PMC_INC_USE( WeekViewInfo.notReady  );
    WeekViewInfo.endDragProviderId = SelectedProviderId;
    AppViewGridEndDrag( WeekViewGrid, &WeekViewInfo, X, Y );
    PMC_DEC_USE( WeekViewInfo.notReady  );

    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::WeekViewGridDragOver(TObject *Sender,
      TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
    Ints_t      col, row;

    Accept = true;
    WeekViewGrid->MouseToCell( X, Y, col, row );

    if( row > 0 && col >= 0 )
    {
        WeekViewGrid->Row = row;
        WeekViewGrid->Col = col;
    }
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::WeekViewGridSelectCell(TObject *Sender,
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
void __fastcall TMainForm::WeekViewGridStartDrag(TObject *Sender,
      TDragObject *&DragObject)
{
    WeekViewGrid->Cursor = crDrag;
}
//---------------------------------------------------------------------------

