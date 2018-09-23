//---------------------------------------------------------------------------      
// File:    pmcProviderView.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    September 29, 2001
//---------------------------------------------------------------------------
// Description:
//
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
#include "pmcInitialize.h"
#include "pmcMainForm.h"
#include "pmcDateSelectForm.h"
#include "pmcPollTableThread.h"
#include "pmcPromptForm.h"
#include "pmcPatientEditForm.h"
#include "pmcPatientListForm.h"
#include "pmcDoctorListForm.h"
#include "pmcDoctorEditForm.h"
#include "pmcAppLettersForm.h"
#include "pmcDayView.h"
#include "pmcAppList.h"
#include "pmcSeiko240.h"
#include "pmcProviderView.h"

//---------------------------------------------------------------------------

void __fastcall TMainForm::ProviderViewTimeGridDrawCell(TObject *Sender,
      int ACol, int ARow, TRect &Rect, TGridDrawState State)
{
    TimeGridDrawCell( Sender, ACol, ARow, Rect, State, ProviderViewTimeGrid );
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::ProviderViewGridDrawCell(TObject *Sender,
      int ACol, int ARow, TRect &Rect, TGridDrawState State)
{
    Char_t          buf[128];
    AnsiString      str = "";
    TColor          color;
    Ints_t          col, colStart, colStop;
    TRect           iRect;
    Ints_t          width, i;

    // Don't draw screen if patients list has not been read yet
    if( UpdatePatientListDone == FALSE || UpdateDoctorListDone == FALSE ) return;

    if( ARow == 0 && ACol == 0 && Active == TRUE )
    {
        // Now actually call function to read the appointment information
        if( ProviderViewInfoUpdate( &ProviderViewInfo ) )
        {
            ProviderViewInfoCellConstruct( );
        }

        if( CheckTables == TRUE )
        {
            RefreshTable( PMC_TABLE_BIT_PATIENTS );
        }
        else
        {
            CheckTables = TRUE;
        }

        // Compute the column widths
        col = pmcProviderView_q->size;
        if( col > 1 && col < ProviderViewGrid->ColCount )
        {
            width = ( ProviderViewGrid->Width - col * 2 ) / col;
            for( i = 0 ; i < col ; i++ )
            {
                ProviderViewGrid->ColWidths[i] = width;
            }
            for( i = col ; i < ProviderViewGrid->ColCount ; i++ )
            {
                ProviderViewGrid->ColWidths[i] = 1;
            }
        }
    }

    // Display the provider description in the first row
    if( ARow == 0 )
    {
        pmcProviderViewList_p   providerView_p;
        Int32u_t                i = 0;

        buf[0] = 0;
        mbLockAcquire( pmcProviderView_q->lock );

        qWalk( providerView_p, pmcProviderView_q, pmcProviderViewList_p )
        {
            if( (Ints_t)i == (Ints_t)ACol )
            {
                pmcProviderDescGet( providerView_p->providerId, &buf[0] );
                break;
            }
            i++;
        }
        mbLockRelease( pmcProviderView_q->lock );
        str = buf;
        ProviderViewGrid->Canvas->TextRect( Rect, Rect.Left + 3, Rect.Top, str );
    }

    if( ARow > 0 )
    {
        colStart = ACol;
        colStop = ACol + 1;

        for( col = colStart ; col < colStop ; col++ )
        {
            iRect = ProviderViewGrid->CellRect( col, ARow );

            // Sanity check
            if( ARow >=  PMC_TIMESLOTS_PER_DAY || ACol >=  PMC_PROVIDER_VIEW_COLS )
            {
                mbDlgDebug(( "Invalid row %ld col %ld  in provider view", ARow, ACol ));
            }

            color = (TColor)ProviderViewCellArray[col][ARow].color;

            ProviderViewGrid->Canvas->Brush->Color =  color;
            ProviderViewGrid->Canvas->FillRect(iRect);
            ProviderViewGrid->Canvas->TextRect( iRect, iRect.Left + 2, iRect.Top, ProviderViewCellArray[col][ARow].str );

            if( ProviderViewGrid->Canvas->Brush->Color != pmcColor[ PMC_COLOR_AVAIL ] &&
                ProviderViewGrid->Canvas->Brush->Color != pmcColor[ PMC_COLOR_BREAK ] )
            {
                Ints_t  subtract;
                color = clBlack;

                subtract = ( ProviderViewCellArray[col][ARow].last ) ? 1 : 0 ;
                ProviderViewGrid->Canvas->Pen->Color = color;

                ProviderViewGrid->Canvas->MoveTo( iRect.Left,  iRect.Top  );
                ProviderViewGrid->Canvas->LineTo( iRect.Left,  iRect.Bottom - subtract  );
                ProviderViewGrid->Canvas->MoveTo( iRect.Right - 1 , iRect.Top  );
                ProviderViewGrid->Canvas->LineTo( iRect.Right - 1, iRect.Bottom - subtract  );
            }

            if( ProviderViewCellArray[col][ARow].last )
            {
                color = clBlack;
                ProviderViewGrid->Canvas->Pen->Color = color;
                ProviderViewGrid->Canvas->MoveTo( iRect.Left  , iRect.Bottom - 1 );
                ProviderViewGrid->Canvas->LineTo( iRect.Right , iRect.Bottom - 1 );
            }

            if( ProviderViewCellArray[col][ARow].first && ARow > 1 )
            {
                color = clBlack;
                ProviderViewGrid->Canvas->Pen->Color = color;
                ProviderViewGrid->Canvas->MoveTo( iRect.Left  , iRect.Top  );
                ProviderViewGrid->Canvas->LineTo( iRect.Right , iRect.Top  );
            }
        }
    }
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::ProviderViewGridDragOver(TObject *Sender,
      TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
    Ints_t      col, row;

    Accept = true;
    ProviderViewGrid->MouseToCell( X, Y, col, row );

    if( row > 0 && col >= 0 )
    {
        ProviderViewGrid->Row = row;
        ProviderViewGrid->Col = col;
    }
    return;
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::ProviderViewGridEndDrag(TObject *Sender,
      TObject *Target, int X, int Y)
{
    Int32s_t                i = 0;
    pmcProviderViewList_p   providerView_p;

    // Restore cursor no matter what
    ProviderViewGrid->Cursor = crDefault;

    if( ProviderViewInfo.notReady ) return;

    ProviderViewInfo.endDragProviderId = 0;

    // Set flag to false to prevent action on other mouse clicks until done
    PMC_INC_USE( ProviderViewInfo.notReady  );

    // Must Determine the provider Id

    mbLockAcquire( pmcProviderView_q->lock );
    qWalk( providerView_p, pmcProviderView_q, pmcProviderViewList_p )
    {
        if( i == ProviderViewGrid->Col )
        {
            ProviderViewInfo.endDragProviderId = providerView_p->providerId;
            break;
        }
        i++;
    }
    mbLockRelease( pmcProviderView_q->lock );

    if( ProviderViewInfo.endDragProviderId != 0 )
    {
        AppViewGridEndDrag( ProviderViewGrid, &ProviderViewInfo, X, Y );
    }
    PMC_DEC_USE( ProviderViewInfo.notReady  );

    return;
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::ProviderViewGridMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    Ints_t          row = 0, col = 0;
    TRect           cellRect;

    if( ProviderViewInfo.notReady )
    {
        return;
    }

    // Convert co-ordinates to cell
    ProviderViewGrid->MouseToCell( X, Y, col, row );
    nbDlgDebug(( "Mouse Down in row %d col %d (X: %d Y: %d)\n", row, col, X, Y ));

    if( row < 1 || col < 0 || col >= ProviderViewInfo.cols )
    {
        MouseInRow = 0;
        MouseInCol = 0;
        return;
    }

    PMC_INC_USE( ProviderViewInfo.notReady );

    cellRect = ProviderViewGrid->CellRect( col, row );

    AppViewInfoUpdate( row, col, &ProviderViewInfo, 0 );

    if( Button == mbLeft )
    {
        ProviderViewGrid->Cursor = crDrag;
        ProviderViewGrid->BeginDrag( TRUE, 0 );
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

    PMC_DEC_USE( ProviderViewInfo.notReady );
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::ProviderViewGridSelectCell(TObject *Sender,
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

void __fastcall TMainForm::ProviderViewGridStartDrag(TObject *Sender,
      TDragObject *&DragObject)
{
    ProviderViewGrid->Cursor = crDrag;
}

//---------------------------------------------------------------------------

