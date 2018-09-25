//---------------------------------------------------------------------------
// File:    pmcDayView.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada        
//---------------------------------------------------------------------------
// Date:    May 12, 2001
//---------------------------------------------------------------------------
// Description:
//
//
//---------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <vcl.h>
#include <dir.h>
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
#include "pmcPatientRecord.h"
#include "pmcPatientEditForm.h"
#include "pmcPatientListForm.h"
#include "pmcDoctorListForm.h"
#include "pmcDoctorEditForm.h"
#include "pmcAppLettersForm.h"
#include "pmcDayView.h"
#include "pmcProviderView.h"
#include "pmcAppList.h"
#include "pmcSeiko240.h"
#include "pmcAppHistoryForm.h"
#include "pmcEchoListForm.h"
#include "pmcDocumentListForm.h"
#include "pmcClaimListForm.h"
#include "pmcPDF.h"

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewGridDrawCell
(
    TObject        *Sender,
    int             ACol,
    int             ARow,
    TRect          &Rect,
    TGridDrawState  State
)
{
    AnsiString      str;
    TColor          color;
    Ints_t          col, colStart, colStop;
    TRect           iRect;

    if( ARow == 0 )
    {
        str = pmcDayViewStrings[ACol];
        DayViewGrid->Canvas->TextRect( Rect, Rect.Left + 2, Rect.Top , str );
    }

    // Don't draw screen if patients list has not been read yet
    if( UpdatePatientListDone == FALSE || UpdateDoctorListDone == FALSE ) return;

    if( ARow == 0 && ACol == 0 && Active == TRUE )
    {
        if( DayViewInfoUpdate( &DayViewInfo ) )
        {
            DayViewInfoCellConstruct( );
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
           colStop  = ACol + 1;
        //}

        for( col = colStart ; col < colStop ; col++ )
        {
            iRect = DayViewGrid->CellRect( col, ARow );
            // Sanity check
            if( ARow >= PMC_TIMESLOTS_PER_DAY ||
                ACol >= PMC_DAY_VIEW_ARRAY_COLS )
            {
                mbDlgDebug(( "Invalid row %ld col %ld  in day view", ARow, ACol ));
            }

            color = (TColor)DayViewCellArray[col][ARow].color;

            // if( ARow == DayViewGrid->Row ) { PMC_COLOR_DARKEN( color ); }

            DayViewGrid->Canvas->Brush->Color =  color;
            DayViewGrid->Canvas->FillRect(iRect);
            DayViewGrid->Canvas->TextRect( iRect, iRect.Left + 2, iRect.Top , DayViewCellArray[col][ARow].str );

            // Make the color solid across multiple time slots
            //if( DayViewGrid->Canvas->Brush->Color != pmcColor[ PMC_COLOR_AVAIL ] &&
            //    DayViewGrid->Canvas->Brush->Color != pmcColor[ PMC_COLOR_BREAK ] &&
            //   !DayViewCellArray[ARow][col].last )
            //{
            //    DayViewGrid->Canvas->Pen->Color = DayViewGrid->Canvas->Brush->Color;
            //    DayViewGrid->Canvas->MoveTo( iRect.Left  , iRect.Bottom );
            //    DayViewGrid->Canvas->LineTo( iRect.Right , iRect.Bottom );
            //}

            if( DayViewCellArray[col][ARow].last )
            {
                color = clBlack;
                DayViewGrid->Canvas->Pen->Color = color;
                DayViewGrid->Canvas->MoveTo( iRect.Left  , iRect.Bottom -1 );
                DayViewGrid->Canvas->LineTo( iRect.Right , iRect.Bottom -1 );
            }

            if( DayViewCellArray[col][ARow].first && ARow > 1)
            {
                color = clBlack;
                DayViewGrid->Canvas->Pen->Color = color;
                DayViewGrid->Canvas->MoveTo( iRect.Left  , iRect.Top  );
                DayViewGrid->Canvas->LineTo( iRect.Right , iRect.Top  );
            }
        }
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewTimeGridDrawCell
(
    TObject *Sender,
    int ACol,
    int ARow,
    TRect
    &Rect,
    TGridDrawState State
)
{
    TimeGridDrawCell( Sender, ACol, ARow, Rect, State, DayViewTimeGrid );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewGridSelectCell(TObject *Sender, int ACol,
      int ARow, bool &CanSelect)
{
    CanSelect = ( ARow > 0 ) ? TRUE : FALSE;
    return;

#if 0
    // Draw highlighs on entire row
    for( Ints_t i = 0 ; i < PMC_DAY_VIEW_ARRAY_COLS ; i++ )
    {
        Rect = DayViewGrid->CellRect( i, ARow );

        DayViewGrid->Canvas->Pen->Color = clBlack;
        DayViewGrid->Canvas->MoveTo( Rect.Left  , Rect.Bottom - 1 );
        DayViewGrid->Canvas->LineTo( Rect.Right , Rect.Bottom - 1 );

        DayViewGrid->Canvas->MoveTo( Rect.Left  , Rect.Top  );
        DayViewGrid->Canvas->LineTo( Rect.Right , Rect.Top  );
    }
#endif
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewGridStartDrag(TObject *Sender,
      TDragObject *&DragObject)
{
    DayViewGrid->Cursor = crDrag;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
// Determine which appointment info view invoked the popup menu
//---------------------------------------------------------------------------

pmcAppViewInfo_p __fastcall TMainForm::AppViewPopupGet( void )
{
    pmcAppViewInfo_p    returnView_p = NIL;

    if( AppointmentTabs->ActivePage == DayViewTab )
    {
        DayViewInfo.providerId = SelectedProviderId;
        returnView_p = &DayViewInfo;
    }
    else if( AppointmentTabs->ActivePage == WeekViewTab )
    {
        WeekViewInfo.providerId = SelectedProviderId;
        returnView_p = &WeekViewInfo;
    }
    else if( AppointmentTabs->ActivePage == MonthViewTab )
    {
        MonthViewInfo.providerId = SelectedProviderId;
        returnView_p = &MonthViewInfo;
    }
    else if( AppointmentTabs->ActivePage == ProviderViewTab )
    {
        pmcProviderViewList_p   providerView_p;
        Int32s_t                i = 0;

        ProviderViewInfo.providerId = 0;

        mbLockAcquire( pmcProviderView_q->lock );

        qWalk( providerView_p, pmcProviderView_q, pmcProviderViewList_p )
        {
            if( i == MouseInCol )
            {
                ProviderViewInfo.providerId = providerView_p->providerId;
                break;
            }
            i++;
        }
        mbLockRelease( pmcProviderView_q->lock );

        // Sanity Check
        if( ProviderViewInfo.providerId == 0 )
        {
            mbDlgDebug(( "Did not find provider" ));
            goto exit;
        }
        returnView_p = &ProviderViewInfo;
    }

exit:

    if( returnView_p == NIL ) mbDlgDebug(( "Unknown app view" ));
    return returnView_p;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupPopup(TObject *Sender)
{
    Ints_t              appointId;
    Int32u_t            today;
    pmcAppViewInfo_p    appView_p;
    Int32u_t            appCol;

    appView_p = AppViewPopupGet( );

    pmcPopupItemEnableAll( DayViewPopup, FALSE );

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    // Check if screen should be redrawn
    today = mbToday( );

    // Allow printing of day sheet under any circumstance - day view only
    if( appView_p == &DayViewInfo )
    {
        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_PRINT_DAY_SHEET, TRUE );
        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_DAY_SHEET, TRUE );
    }

    // Determine if there is an appointment in this cell
    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId )
    {
        // These actions are allowed on future and past appointments
        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_PAT,            TRUE );
        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_DR,             TRUE );
        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_COMMENT,        TRUE );
        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_HISTORY,        TRUE );

        // Sub actions allowed on patient field in all circumstances
        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_PAT_VIEW,       TRUE );
        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_PAT_EDIT,       TRUE );
        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_PAT_APP_LIST,   TRUE );
        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_PAT_CLAIM_LIST, TRUE );
        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_PAT_DOC_LIST,   TRUE );
        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_PAT_ECHO_LIST,  TRUE );
        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_PAT_PR_REC,     TRUE );

        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_CONFIRM,            TRUE );
        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_CONFIRM_QUERY,      TRUE );

        if( pmcCfg[CFG_SLP_NAME].str_p )
        {
            pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_PAT_PR_LBL_REQ, TRUE );
            pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_PAT_PR_LBL_ADDR,TRUE );
        }

        // Sub actions allowed on doctor field
        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_DR_VIEW,        TRUE );
        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_DR_EDIT,        TRUE );

        if( pmcCfg[CFG_SLP_NAME].str_p )
        {
            pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_DR_PR_LBL_ADDR, TRUE );
        }
        //pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_REF_DR_PR_REC, TRUE );

        // Sub actions allowed on comment field - allow commments in past to be editied
        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_COMMENT_VIEW,   TRUE );
        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_COMMENT_EDIT,   TRUE );
    }

    if( SelectedDate < today && appointId )
    {
        // This is the past.  Disallow all actions except perhaps viewing
        // appointment records, and marking records as completed
        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_COMPLETED, TRUE );
        goto exit;
    }

    if( SelectedDate >= today )
    {
        if( AppViewExtractAvailable( MouseInRow, MouseInCol, appView_p ) == TRUE )
        {
            pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_UNAVAILABLE, TRUE );
        }
        else
        {
            pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_AVAILABLE, TRUE );
        }
    }

    if( SelectedDate == today && appointId )
    {
        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_COMPLETED, TRUE );
    }

    // Enable paste if there are entries in the paste buffer
    // Currently no checking as to time validity, etc.
    // Do not allow pasting to past days
    if( pmcAppointCutBuf_q->size > 0 && SelectedDate >= today )
    {
        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_PASTE, TRUE );
    }

    appCol = ( appView_p == &DayViewInfo ) ? 0 : MouseInCol;
    if( appView_p->slot_p[appCol * PMC_TIMESLOTS_PER_DAY + MouseInRow].countt == 0 && SelectedDate >= today )
    {
        // This slot is open for an appointment
        pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_NEW, TRUE );
    }
    else
    {
        // Check to see if there is an appointment in cell
        if( appointId != 0 )
        {
            // Sub actions allowed on patient field if appointment is not in past

           // if( appView_p == &DayViewInfo )
            {
                pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_CONFIRM_PHONE,      TRUE );
                pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_CONFIRM_LETTER,     TRUE );
                pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_CONFIRM_CANCEL_PHONE,  TRUE);
                pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_CONFIRM_CANCEL_LETTER, TRUE );
            }

            pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_DELETE,             TRUE );
            pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_CANCEL,             TRUE );
            pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_CUT,                TRUE );

            pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_PAT_CHANGE,         TRUE );
            pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_PAT_CLEAR,          TRUE );
            pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_PAT_PR_CONF_LET,    TRUE );

            pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_DR_CHANGE,          TRUE );
            pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_DR_CLEAR,           TRUE );

            pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_TYPE,               TRUE );
            pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_TYPE_NEW,           TRUE );
            pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_TYPE_NONE,          TRUE );
            pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_TYPE_REVIEW,        TRUE );
            pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_TYPE_URGENT,        TRUE );
            pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_TYPE_CLINIC,        TRUE );
            pmcPopupItemEnable( DayViewPopup, PMC_DAYVIEW_POPUP_TYPE_HOSPITAL,      TRUE );
        }
    }

exit:
    PMC_DEC_USE( appView_p->notReady );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupAvailableClick(TObject *Sender)
{
    DayViewPopupAvailableChange( Sender, TRUE, MouseInRow, MouseInRow, FALSE );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupUnavailableClick(TObject *Sender)
{
    DayViewPopupAvailableChange( Sender, FALSE, MouseInRow, MouseInRow, FALSE );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
// Create a new timeslot record.  There is a slight chance that a record
// could have been created in the time beteen this call and when we first
// realized that we needed to create a new record.  Thus the table must
// be locked.
//---------------------------------------------------------------------------

Int32u_t __fastcall TMainForm::TimeSlotRecordCreate
(
    Int32u_t    providerId,
    Int32u_t    date,
    Char_p      string_p
)
{
    Char_p          buf_p = NIL;
    Int32u_t        id = 0;
    Int32u_t        i = 0;
    Char_t          slotString[64];
    MbSQL           sql;

    mbMalloc( buf_p, 512 );

    // Lock the timeslots table
    pmcSuspendPollInc( );

    sprintf( buf_p, "lock tables %s write", PMC_SQL_TABLE_TIMESLOTS );
    sql.Update( buf_p );

    // Format command to read info from database
    sprintf( buf_p, "select %s,%s from %s where %s=%ld and %s=%ld",
        PMC_SQL_FIELD_ID,
        PMC_SQL_TIMESLOTS_FIELD_AVAILABLE,
        PMC_SQL_TABLE_TIMESLOTS,
        PMC_SQL_FIELD_PROVIDER_ID, providerId,
        PMC_SQL_FIELD_DATE, date );

    // Execute SQL command
    sql.Query( buf_p );

    while( sql.RowGet( ) )
    {
        id = sql.Int32u( 1 );
        strcpy( slotString, sql.String( 1 ) );
        i++;
    }

    // Sanity check
    if( i > 1 )
    {
        mbDlgDebug(( "Error: i (%d) > 1", i ))
        goto exit;

    }
    if( i == 1 )
    {
        // Someone else must have created the record in the interim.
        // Put the new string into the record.
        sprintf( buf_p, "update %s set %s='%s' where %s=%ld",
            PMC_SQL_TABLE_TIMESLOTS,
            PMC_SQL_TIMESLOTS_FIELD_AVAILABLE, string_p,
            PMC_SQL_FIELD_ID, id );

        sql.Update( buf_p );
    }
    else
    {
        sprintf( buf_p, "select max(%s) from %s where %s>0",
            PMC_SQL_FIELD_ID,
            PMC_SQL_TABLE_TIMESLOTS,
            PMC_SQL_FIELD_ID );

        // Get id from database
        sql.Query( buf_p );
        while( sql.RowGet( ) )
        {
            id = sql.Int32u( 0 );
        }

        // Increment id
        id++;

        // Now create the new record
        sprintf( buf_p, "insert into %s (%s,%s,%s,%s) values (%ld, %ld, %ld, '%s')",
            PMC_SQL_TABLE_TIMESLOTS,
            PMC_SQL_FIELD_ID,
            PMC_SQL_FIELD_DATE,
            PMC_SQL_FIELD_PROVIDER_ID,
            PMC_SQL_TIMESLOTS_FIELD_AVAILABLE,
            id, date, providerId, string_p );

        sql.Update( buf_p );
    }

exit:

    // Unlock the timeslots table
    sprintf( buf_p, "unlock tables" );
    sql.Update( buf_p );
    pmcSuspendPollDec( );

    mbFree( buf_p );

    return id;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupAvailableChange
(
    TObject    *Sender,
    Int32u_t    availFlag,
    Int32u_t    startSlot,
    Int32u_t    endSlot,
    bool        ignoreNotReadyFlag
)
{
    pmcAppViewInfo_p    appView_p;
    Char_p              buf_p;
    Int32u_t            id, i, j;
    Int8u_t             updateValue;
    Char_t              slotString[64];
    MbSQL               sql;

    nbDlgDebug(( "called, date %ld provider %ld flag: %ld", SelectedDate, SelectedProviderId, availFlag ));

    if( ( appView_p = AppViewPopupGet( ) ) == NIL )
    {
        mbDlgDebug(( "Error determined app view\n" ));
        return;
    }

    // MAB:20020711: Why ignore not ready flag??
    if( !ignoreNotReadyFlag )
    {
        PMC_CHECK_USE( appView_p->notReady );
    }

    PMC_INC_USE( appView_p->notReady );

    mbMalloc( buf_p, 1024 );

    // Format command to read info from database
    sprintf( buf_p, "select %s,%s from %s where %s=%ld and %s=%ld",
        PMC_SQL_FIELD_ID,
        PMC_SQL_TIMESLOTS_FIELD_AVAILABLE,
        PMC_SQL_TABLE_TIMESLOTS,
        PMC_SQL_FIELD_PROVIDER_ID, appView_p->providerId,
        PMC_SQL_FIELD_DATE, SelectedDate );

    nbDlgDebug(( buf_p ));

    i = 0;
    if( sql.Query( buf_p ) == FALSE ) goto exit;

    while( sql.RowGet( ) )
    {
        id = sql.Int32u( 0 );
        if( strlen( sql.String( 1 ) ) )
        {
            strcpy( slotString, sql.String( 1 ) );
        }
        i++;
    }

    // Create entry if there is not one already.
    if( i == 0 )
    {
        for( j = 0 ; j < PMC_TIMESLOTS_PER_DAY ; j++ ) slotString[j] = PMC_TIMESLOT_UNAVAILABLE;
        slotString[PMC_TIMESLOTS_PER_DAY] = 0;

        id = TimeSlotRecordCreate( appView_p->providerId, SelectedDate, &slotString[0] );
    }

    // 20040701: Deal with short strings.  If the string is ever modified then
    // it will get written to the database.
    for( ; ; )
    {
        if( strlen( slotString ) == PMC_TIMESLOTS_PER_DAY ) break;
        strcat( slotString, "0" );
    }

    if( i > 1 )
    {
        mbDlgDebug(( "Error: i = %d\n", i ));
    }
    else
    {
        nbDlgDebug(( "id: %ld slotString: '%s' MouseInRow: %ld\n", id, slotString, MouseInRow ));

        updateValue = ( availFlag == TRUE ) ? PMC_TIMESLOT_AVAILABLE : PMC_TIMESLOT_UNAVAILABLE;

        // reverse velue if required
        if( startSlot > endSlot )
        {
            Int32u_t    temp;

            temp        = startSlot;
            startSlot   = endSlot;
            endSlot     = temp;
        }

        // Update the slot string
        for( i = startSlot ; i <= endSlot ; i++ )
        {
            slotString[i] = updateValue;
        }

        // Write new slot string to the database
        sprintf( buf_p, "update %s set %s='%s' where %s=%ld",
            PMC_SQL_TABLE_TIMESLOTS,
            PMC_SQL_TIMESLOTS_FIELD_AVAILABLE, slotString,
            PMC_SQL_FIELD_ID, id );

        nbDlgDebug(( buf_p ));
        pmcSqlExec( buf_p );

        // Force screen update
        appView_p->updateForce = TRUE;
        appView_p->grid_p->Invalidate( );
    }

exit:

    PMC_DEC_USE( appView_p->notReady );
    mbFree( buf_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupPurgeClick(TObject *Sender)
{
    Int32u_t            appointId;
    pmcAppViewInfo_p    appView_p;

    appView_p = AppViewPopupGet( );

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );

    if( mbDlgOkCancel( "Purge record %d from table '%s'?", appointId, PMC_SQL_TABLE_APPS ) == MB_BUTTON_OK )
    {
        pmcSqlRecordPurge( PMC_SQL_TABLE_APPS, appointId );

        // Force update and redraw the screen
        appView_p->updateForce = TRUE;
        appView_p->grid_p->Invalidate();
        AppViewInfoUpdate( 0, 0, appView_p, 0 );
    }

    PMC_DEC_USE( appView_p->notReady );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupCompletedClick(TObject *Sender)
{
    Int32u_t            appointId;
    Char_p              cmd_p;
    Int32u_t            completedFlag;
    Int32u_t            action;
    pmcAppViewInfo_p    appView_p;
    bool                update = FALSE;

    appView_p = AppViewPopupGet( );

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );
    mbMalloc( cmd_p, 1024 );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId == 0 ) goto exit;

    sprintf( cmd_p, "select %s from %s where %s=%ld",
             PMC_SQL_APPS_FIELD_COMPLETED,
             PMC_SQL_TABLE_APPS,
             PMC_SQL_FIELD_ID,
             appointId );

    completedFlag = pmcSqlSelectInt( cmd_p, NIL );

    // MAB:20020708: Add switch statement to check the state of the appointment
    switch( completedFlag )
    {
        case PMC_APP_COMPLETED_STATE_NONE:
            completedFlag = PMC_APP_COMPLETED_STATE_COMPLETED;
            action = PMC_APP_ACTION_COMPLETE_SET;
            update = TRUE;
            break;

        case PMC_APP_COMPLETED_STATE_COMPLETED:
            completedFlag = PMC_APP_COMPLETED_STATE_NONE;
            action = PMC_APP_ACTION_COMPLETE_CLEAR;
            update = TRUE;
            break;

        default:
            mbDlgDebug(( "Invalid completed state: %d\n", completedFlag ));
    }

    if( update )
    {
        pmcSqlExecInt(  PMC_SQL_TABLE_APPS, PMC_SQL_APPS_FIELD_COMPLETED, completedFlag, appointId );
        mbLog( "Set appointment %d completed state to %d\n", appointId, completedFlag );
        pmcAppHistory( appointId, action, 0, 0, 0, 0, NIL );
     }

    appView_p->updateForce = TRUE;
    appView_p->grid_p->Invalidate( );

exit:
    mbFree( cmd_p );
    PMC_DEC_USE(  appView_p->notReady );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupPatClaimListClick(TObject *Sender)
{
    pmcClaimListInfo_t      claimListInfo;
    TClaimListForm         *claimListForm_p;
    MbDateTime              dateTime;
    pmcAppViewInfo_p        appView_p;
    Int32u_t                appointId;
    Int32u_t                patientId;

    appView_p = AppViewPopupGet( );

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId == 0 ) goto exit;

    patientId = pmcAppointPatientId( appointId );

    claimListInfo.patientId = patientId;
    dateTime.SetDate( mbToday( ) );
    claimListInfo.latestDate = mbToday( );
    claimListInfo.earliestDate = dateTime.BackYears( 10 );

    claimListInfo.providerId = 0;
    claimListInfo.providerAllFlag = TRUE;
    claimListInfo.patientAllFlag = FALSE;

    claimListForm_p = new TClaimListForm( NULL, &claimListInfo );
    claimListForm_p->ShowModal();
    delete claimListForm_p;

exit:
    PMC_DEC_USE( appView_p->notReady );

    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupPatEchoListClick(TObject *Sender)
{
    pmcAppViewInfo_p        appView_p;
    Int32u_t                appointId;
    Int32u_t                patientId;

    appView_p = AppViewPopupGet( );

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId == 0 ) goto exit;
    patientId = pmcAppointPatientId( appointId );
    pmcEchoListForm( patientId );

exit:
    PMC_DEC_USE( appView_p->notReady );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupPatDocListClick(TObject *Sender)
{
    TDocumentListForm      *form_p;
    pmcDocumentListInfo_t   formInfo;
    pmcAppViewInfo_p        appView_p;
    Int32u_t                appointId;
    Int32u_t                patientId;

    appView_p = AppViewPopupGet( );

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId == 0 ) goto exit;
    patientId = pmcAppointPatientId( appointId );

    formInfo.patientId = patientId;
    form_p = new TDocumentListForm( this, &formInfo );
    form_p->ShowModal( );
    delete form_p;

exit:
    PMC_DEC_USE( appView_p->notReady );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupPatAppListClick(TObject *Sender)
{
    Int32u_t            appointId;
    Int32u_t            patientId;
    pmcAppViewInfo_p    appView_p;
    Int32u_t            providerId = 0;
    Int32u_t            date = 0;

    appView_p = AppViewPopupGet( );

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId == 0 ) goto exit;

    patientId = pmcAppointPatientId( appointId );
    if( patientId )
    {
        pmcViewAppointments( patientId, TRUE, FALSE, TRUE, &providerId, &date, TRUE, PMC_LIST_MODE_LIST );
    }
    else
    {
        mbDlgExclaim( "Appointment has no patient." );
    }
exit:
    PMC_DEC_USE(  appView_p->notReady );

    AppointmentGoto( providerId, date );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupCancelClick(TObject *Sender)
{
    Int32u_t            appointId;
    Int32u_t            patientId;
    Char_p              buf_p;
    pmcAppViewInfo_p    appView_p;
    PmcSqlPatient_p     patient_p;

    appView_p = AppViewPopupGet( );

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    mbMalloc( buf_p, 256 );
    mbMalloc( patient_p, sizeof(PmcSqlPatient_t) );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );

    if( appointId == 0 ) goto exit;

    patientId = pmcAppointPatientId( appointId );
    pmcSqlPatientDetailsGet( patientId, patient_p );

    if( strlen( patient_p->lastName ) == 0 && strlen( patient_p->firstName ) == 0 )
    {
        sprintf( buf_p, "Cancel appointment?" );
    }
    else
    {
        sprintf( buf_p, "Cancel appointment for %s %s?", patient_p->firstName,
                                                         patient_p->lastName );
    }

    if( mbDlgYesNo( buf_p ) == MB_BUTTON_YES )
    {
        if( pmcSqlRecordDelete( PMC_SQL_TABLE_APPS, appointId ) == TRUE )
        {
            // MAB:20020708: Mark the appointment as cancelled
            pmcSqlExecInt( PMC_SQL_TABLE_APPS, PMC_SQL_APPS_FIELD_COMPLETED, PMC_APP_COMPLETED_STATE_CANCELLED, appointId );
            pmcAppHistory( appointId, PMC_APP_ACTION_CANCEL, 0, 0, 0, 0, NIL );

            // Force update and redraw the screen
            appView_p->updateForce = TRUE;
            appView_p->grid_p->Invalidate( );
            AppViewInfoUpdate( 0, 0, appView_p, 0 );
            mbLog( "Cancelled appointment %d (%s %s)", appointId, patient_p->firstName,
                                                       patient_p->lastName );
            AppointmentCountUpdate( PMC_COUNTER_UPDATE );
        }
        else
        {
            mbDlgDebug(( "Error cancelling appointment %d", appointId ));
        }
    }

exit:

    mbFree( patient_p );
    mbFree( buf_p );
    PMC_DEC_USE( appView_p->notReady );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupDeleteClick(TObject *Sender)
{
    Int32u_t            appointId;
    Int32u_t            patientId;
    Char_p              buf_p;
    pmcAppViewInfo_p    appView_p;
    PmcSqlPatient_p     patient_p;

    appView_p = AppViewPopupGet( );

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    mbMalloc( patient_p, sizeof(PmcSqlPatient_t) );
    mbMalloc( buf_p, 256 );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );

    if( appointId == 0 ) goto exit;

    patientId = pmcAppointPatientId( appointId );

    pmcSqlPatientDetailsGet( patientId, patient_p );

    if( strlen( patient_p->lastName ) == 0 && strlen( patient_p->firstName ) == 0 )
    {
        sprintf( buf_p, "Delete appointment?" );
    }
    else
    {
        sprintf( buf_p, "Delete appointment for %s %s?", patient_p->firstName, patient_p->lastName );
    }

    if( mbDlgYesNo( buf_p ) == MB_BUTTON_YES )
    {
        if( pmcSqlRecordDelete( PMC_SQL_TABLE_APPS, appointId ) == TRUE )
        {
            pmcAppHistory( appointId, PMC_APP_ACTION_DELETE, 0, 0, 0, 0, NIL );

            // Force update and redraw the screen
            appView_p->updateForce = TRUE;
            appView_p->grid_p->Invalidate( );
            AppViewInfoUpdate( 0, 0, appView_p, 0 );
            mbLog( "Deleted appointment %d (%s %s)", appointId, patient_p->firstName, patient_p->lastName );
            AppointmentCountUpdate( PMC_COUNTER_UPDATE );
        }
        else
        {
            mbDlgDebug(( "Error deleteing appointment %d", appointId ));
        }
    }

exit:
    mbFree( patient_p );
    mbFree( buf_p );
    PMC_DEC_USE( appView_p->notReady );
    return;
}

//---------------------------------------------------------------------------
// Generate a PDF version of the daysheet
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupDaySheetClick(TObject *Sender)
{
    MbPDF       doc = MbPDF( "day_sheet_", SelectedProviderId );
    MbCursor    cursor = MbCursor( crHourGlass );
    MbDateTime  dateTime = MbDateTime( mbToday( ), mbTime( ) );

    doc.subString( "_PRINT_DATE_", dateTime.MDYHM_DateStringLong( ), 0 );
    doc.subDate( "_DATE_", SelectedDate );
    doc.subCallback( "_CONTENTS_", DaySheetContents, (Int32u_t)this );
    doc.subCallback( "_NOTES_", DaySheetContents, (Int32u_t)this );
    doc.subCallback( "_IF_NOTE_COUNT_", DaySheetContents, (Int32u_t)this );
    doc.subCallback( "_ENDIF_NOTE_COUNT_", DaySheetContents, (Int32u_t)this );

    if( doc.templateToTempSub( ) == MB_RET_OK )
    {
        if( doc.generate( 15 ) == MB_RET_OK )
        {
            doc.view( );
        }
    }
    return;
}

//---------------------------------------------------------------------------
// Callback function for putting the contents into a PDF daysheet
// Services two keywords.  If _NOTES_, outputs notes as LATEX items.
// If not _NOTES_, outputs daysheet as entries in a 6-column table.
//---------------------------------------------------------------------------
Int32s_t TMainForm::DaySheetContents( Int32u_t handle, FILE *fp, Char_p key_p )
{
    Ints_t          i;
    Char_p          buf1_p = NIL;
    Char_p          buf2_p = NIL;
    Char_p          buf3_p = NIL;
    Char_p          tex_p = NIL;
    Boolean_t       conflictFlag;
    Int32u_t        appointId;
    Int32u_t        patientId;
    PmcSqlPatient_p pat_p = NIL;
    TMainForm      *form_p;
    mbStrList_p     entry_p;
    Int32s_t        result = TRUE;
    Int32s_t        color;

    static qHead_t   commentQueue;
    static qHead_p   comment_q = NIL;

    mbMalloc( tex_p, 1000000 );
    mbMalloc( buf1_p, 256 );
    mbMalloc( buf2_p, 256 );
    mbMalloc( buf3_p, 256 );

    // Initialize comment queue only if necessary
    if( comment_q == NIL )
    {
        comment_q = qInitialize( &commentQueue );
    }

    if( strcmp( key_p, "_NOTES_" ) == 0 )
    {
        while( !qEmpty( comment_q ) )
        {
            entry_p = (mbStrList_p)qRemoveFirst( comment_q );
            fprintf( fp, "\\item %s\n", mbStrTex( entry_p->str_p, tex_p, 0 ) );
            mbStrListItemFree( entry_p );
        }
        goto exit;
    }
    else if( strcmp( key_p, "_IF_NOTE_COUNT_" ) == 0 )
    {
        // Want to return TRUE/FALSE to this _IF_ statement.
        result = ( comment_q->size > 0 ) ? TRUE : FALSE;
        goto exit;
    }
    else if( strcmp( key_p, "_ENDIF_NOTE_COUNT_" ) == 0 )
    {
        result = TRUE;
        goto exit;
    }
    else
    {
        mbStrListFree( comment_q );
    }

    // The this pointer is sent back to this callback function via the handle
    form_p = static_cast<TMainForm *>((Void_p)handle);

    mbMalloc( pat_p, sizeof( PmcSqlPatient_t ) );

    for( i = 1 ; i < PMC_TIMESLOTS_PER_DAY ; i++ )
    {
        // Appointment time - col 1
        color = DayViewCellArray[0][i].color;

        // Note: my old method of detecting a conflict has probably been
        // broken for years... as I am shading the colors such that they
        // no longer match the exact values.
        conflictFlag = FALSE;
        if( abs( color - pmcColor[PMC_COLOR_CONFLICT] ) < 256 )
        {
            mbLog( "This is probably a conflict" );
            conflictFlag = TRUE;
        }
        //mbLog( "Color %08lX conflict %08lX\n", (Int32u_t)color, (Int32u_t)pmcColor[PMC_COLOR_CONFLICT] );
        //conflictFlag = ( color == pmcColor[PMC_COLOR_CONFLICT] ) ? '*' : ' ';
        fprintf( fp, "%s%s & ", conflictFlag ? "{*}" : "", pmcTimeSlotString[i] );

        // Check for patient name in slot
        sprintf( buf1_p, DayViewCellArray[PMC_DAY_VIEW_COL_LAST_NAME][i].str.c_str() );
        sprintf( buf2_p, DayViewCellArray[PMC_DAY_VIEW_COL_FIRST_NAME][i].str.c_str() );
        sprintf( buf3_p, "" );

        // Clear out the patient details structure
        memset( pat_p, NULL, sizeof( PmcSqlPatient_t ) );
        if( strlen( buf1_p ) || strlen( buf2_p ) )
        {
            // Patient's name is in buf3_p
            sprintf( buf3_p, "%s, %s  ", buf1_p, buf2_p );

            // This slot has a patient name... lets get the patient PHN too
            appointId = form_p->AppViewExtractAppointId( i, 0, &form_p->DayViewInfo );
            if( appointId != 0 )
            {
                patientId = pmcAppointPatientId( appointId );
                if( patientId != 0 )
                {
                    if( pmcSqlPatientDetailsGet( patientId, pat_p ) == FALSE )
                    {
                        // Error Reading patient info
                    }
                    pmcFormatPhnDisplay( pat_p->phn, pat_p->phnProv, buf2_p );
                    if( strlen( pat_p->phn ) == 0 ) sprintf( buf2_p, "--- --- ---" );
                    if( strlen( buf2_p ) > 11 )
                    {
                        *(buf2_p + 11) = 0;
                        *(buf2_p + 10) = '.';
                        *(buf2_p +  9) = '.';
                        *(buf2_p +  8) = '.';
                    }
                }
            }
        }

        // buf2_p contains the patient's phn - col 2
        // fprintf( fp, "%s & ", mbStrTex( buf2_p, tex_p, 0 ) );
        fprintf( fp, "%s & ", buf2_p );

        // buf3_p contains the patient name - limit to 25 chars - col 3
        fprintf( fp, "%s & ", mbStrTex( buf3_p, tex_p, 25 ) );

        // Print the patient's phone - col 4
        fprintf( fp, "%s & ", mbStrTex( &pat_p->formattedPhoneHome[0], tex_p, 0 ) );

        // Duration
        // fprintf( fp, "%4.4s ", DayViewCellArray[PMC_DAY_VIEW_COL_DURATION][i].str.c_str() );

        // Type
        // fprintf( fp, "%6.6s  ", DayViewCellArray[PMC_DAY_VIEW_COL_APP_TYPE][i].str.c_str() );

        // Indicate status of phone/letter confirmation - col 5
        if( strlen( DayViewCellArray[PMC_DAY_VIEW_COL_CONF_PHONE][i].str.c_str() ) )
        {
            if( DayViewCellArray[PMC_DAY_VIEW_COL_CONF_PHONE][i].color == pmcColor[PMC_COLOR_CONFLICT] )
            {
                fprintf( fp, "X" );
            }
            else
            {
                fprintf( fp, "P" );
            }
        }
        else
        {
            fprintf( fp, " " );
        }

        // Indicate status of letter confirmation
        if( strlen( DayViewCellArray[PMC_DAY_VIEW_COL_CONF_LETTER][i].str.c_str() ) )
        {
            if( DayViewCellArray[PMC_DAY_VIEW_COL_CONF_LETTER][i].color == pmcColor[PMC_COLOR_CONFLICT] )
            {
                fprintf( fp, "X" );
            }
            else
            {
                fprintf( fp, "L" );
            }
        }
        else
        {
            fprintf( fp, " " );
        }

        fprintf( fp , " & " );

        // Comment - col 6

        sprintf( buf1_p, "" );
        if( strlen( DayViewCellArray[PMC_DAY_VIEW_COL_APP_TYPE][i].str.c_str() ) )
        {
            sprintf( buf1_p, "%s", DayViewCellArray[PMC_DAY_VIEW_COL_APP_TYPE][i].str.c_str() );
        }
        if( strlen( DayViewCellArray[PMC_DAY_VIEW_COL_COMMENT][i].str.c_str() ) )
        {
            if( strlen( buf1_p ) > 0 ) strcat( buf1_p, ". " );
            strcat( buf1_p, DayViewCellArray[PMC_DAY_VIEW_COL_COMMENT][i].str.c_str() );
        }
        if( strlen( buf1_p ) > 30 )
        {
            mbStrListAdd( comment_q, buf1_p );
            fprintf( fp, "See note %d", comment_q->size );
        }
        else
        {
            // Combined type and comment
            fprintf( fp, " %s", mbStrTex( buf1_p, tex_p, 0 ) );
        }

        // End of line
        fprintf( fp, " \\\\\n" );
    }

exit:
    mbFree( buf1_p );
    mbFree( buf2_p );
    mbFree( buf3_p );
    mbFree( pat_p );
    mbFree( tex_p );

    return result;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupPrintDaySheetClick(TObject *Sender)
{
    FILE           *fp;
    MbDateTime      dateTime;
    Ints_t          i;
    Char_p          buf1_p;
    Char_p          buf2_p;
    Char_p          buf3_p;
    Char_p          spoolFileName_p;
    Char_p          flagFileName_p;
    Char_p          fileName_p;
    Char_t          conflictFlag;
    Int32u_t        appointId;
    Int32u_t        patientId;
    PmcSqlPatient_p pat_p;

    mbMalloc( spoolFileName_p, 256 );
    mbMalloc( flagFileName_p, 256 );
    mbMalloc( fileName_p, 256 );
    mbMalloc( buf1_p, 256 );
    mbMalloc( buf2_p, 256 );
    mbMalloc( buf3_p, 256 );
    mbMalloc( pat_p, sizeof( PmcSqlPatient_t ) );

    dateTime.SetDate( SelectedDate );

    sprintf( fileName_p, "%s.html", pmcMakeFileName( NIL, buf1_p ) );
    sprintf( spoolFileName_p, "%s\\spool\\%s", pmcCfg[CFG_PMCDESPL_DIR].str_p, fileName_p );
    sprintf( flagFileName_p,  "%s\\flags\\%s", pmcCfg[CFG_PMCDESPL_DIR].str_p, fileName_p );

    nbDlgDebug(( "spool '%s'\nflags: '%s'\n", spoolFileName_p, flagFileName_p ));

    fp = fopen( spoolFileName_p, "w" );
    if( fp == NIL )
    {
        mbDlgDebug(( "Error opening day sheet file '%s'", spoolFileName_p ));
        goto exit;
    }

    fprintf( fp, "<HTML><HEAD><Title>Day Sheet</TITLE></HEAD><BODY>\n" );
    fprintf( fp, "<H2><CENTER>%s - %s</CENTER></H2><HR>\n",
        pmcProviderDescGet( SelectedProviderId, buf1_p ),  dateTime.MDY_DateString(  ) );

    fprintf( fp, "<PRE WIDTH = 80>\n" );


 //   fprintf( fp, "\n" );
    fprintf( fp, "     Time      PHN      Name                     Phone Conf. Comment\n" );
    fprintf( fp, "________________________________________________________________________________\n" );

    for( i = 1 ; i < PMC_TIMESLOTS_PER_DAY ; i++ )
    {
        // Appointment time
        conflictFlag = ( DayViewCellArray[0][i].color == pmcColor[PMC_COLOR_CONFLICT] ) ? '*' : ' ';
        fprintf( fp, "%c%s  ", conflictFlag, pmcTimeSlotString[i] );

        // Check for patient name in slot
        sprintf( buf1_p, DayViewCellArray[PMC_DAY_VIEW_COL_LAST_NAME][i].str.c_str() );
        sprintf( buf2_p, DayViewCellArray[PMC_DAY_VIEW_COL_FIRST_NAME][i].str.c_str() );
        sprintf( buf3_p, "" );

        // Clear out the patient details structure
        memset( pat_p, NULL, sizeof( PmcSqlPatient_t ) );
        if( strlen( buf1_p ) || strlen( buf2_p ) )
        {
            // Patient's name is in buf3_p
            sprintf( buf3_p, "%s, %s  ", buf1_p, buf2_p );

            // This slot has a patient name... lets get the patient PHN too
            appointId = AppViewExtractAppointId( i, 0, &DayViewInfo );
            if( appointId != 0 )
            {
                patientId = pmcAppointPatientId( appointId );
                if( patientId != 0 )
                {
                    if( pmcSqlPatientDetailsGet( patientId, pat_p ) == FALSE )
                    {
                        // Error Reading patient info
                    }
                    pmcFormatPhnDisplay( pat_p->phn, pat_p->phnProv, buf2_p );
                    if( strlen( pat_p->phn ) == 0 ) sprintf( buf2_p, "--- --- ---" );
                    if( strlen( buf2_p ) > 11 )
                    {
                        *(buf2_p + 11) = 0;
                        *(buf2_p + 10) = '.';
                        *(buf2_p +  9) = '.';
                        *(buf2_p +  8) = '.';
                    }
                }
            }
        }

        // buf2_p contains the patient's phn
        fprintf( fp, "%-12.12s ", buf2_p );

        // buf3_p contains the patient name
        fprintf( fp, "%-18.18s ", buf3_p );

        // Print the patient's phone
        fprintf( fp, "%14.14s ", &pat_p->formattedPhoneHome[0] );

        // Duration
        // fprintf( fp, "%4.4s ", DayViewCellArray[PMC_DAY_VIEW_COL_DURATION][i].str.c_str() );

        // Type
        // fprintf( fp, "%6.6s  ", DayViewCellArray[PMC_DAY_VIEW_COL_APP_TYPE][i].str.c_str() );


        // Indicate status of phone confirmation
        if( strlen( DayViewCellArray[PMC_DAY_VIEW_COL_CONF_PHONE][i].str.c_str() ) )
        {
            if( DayViewCellArray[PMC_DAY_VIEW_COL_CONF_PHONE][i].color == pmcColor[PMC_COLOR_CONFLICT] )
            {
                fprintf( fp, "X" );
            }
            else
            {
                fprintf( fp, "P" );
            }
        }
        else
        {
            fprintf( fp, " " );
        }

        // Indicate status of letter confirmation
        if( strlen( DayViewCellArray[PMC_DAY_VIEW_COL_CONF_LETTER][i].str.c_str() ) )
        {
            if( DayViewCellArray[PMC_DAY_VIEW_COL_CONF_LETTER][i].color == pmcColor[PMC_COLOR_CONFLICT] )
            {
                fprintf( fp, "X" );
            }
            else
            {
                fprintf( fp, "L" );
            }
        }
        else
        {
            fprintf( fp, " " );
        }

        sprintf( buf1_p, "" );
        if( strlen( DayViewCellArray[PMC_DAY_VIEW_COL_APP_TYPE][i].str.c_str() ) )
        {
            sprintf( buf1_p, "%s ", DayViewCellArray[PMC_DAY_VIEW_COL_APP_TYPE][i].str.c_str() );
        }
        strcat( buf1_p, DayViewCellArray[PMC_DAY_VIEW_COL_COMMENT][i].str.c_str() );

        // Combined type and comment
        fprintf( fp, " %-18.18s", buf1_p );

        // End of line
        fprintf( fp, "\n" );
    }
    fprintf( fp, "________________________________________________________________________________\n" );
    fprintf( fp, "* Indicates conflict.                         Conf.  P: Phone L: Letter X: Wrong\n" );

    dateTime.SetDateTime( mbToday( ), mbTime( ) );
    fprintf( fp, "  Printed: %s %s\n", dateTime.MDY_DateStringLong( ), dateTime.HM_TimeString( ) );

    fprintf( fp, "</PRE>" );
    PMC_REPORT_FOOTER( fp );
    // End of document

    fprintf( fp, "</BODY></HTML>\n" );

    if( fp ) fclose( fp );

    mbDlgInfo( "Day sheet for %s sent to printer.", pmcProviderDescGet( SelectedProviderId, buf1_p ) );

    // Write flag file to trigger despooler
    fp = fopen( flagFileName_p, "w" );
    if( fp == NIL )
    {
        mbDlgDebug(( "Could not open flag file '%s'", flagFileName_p ));
    }
    else
    {
        fprintf( fp, "%s", flagFileName_p );
        fclose( fp );
        mbLog( "Printed day sheet for '%s'\n",  pmcProviderDescGet( SelectedProviderId, buf1_p ) );
    }

exit:
    mbFree( spoolFileName_p );
    mbFree( flagFileName_p );
    mbFree( fileName_p );
    mbFree( buf1_p );
    mbFree( buf2_p );
    mbFree( buf3_p );
    mbFree( pat_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupPatChangeClick(TObject *Sender)
{
    Int32u_t            appointId;
    pmcAppViewInfo_p    appView_p;

    appView_p = AppViewPopupGet( );

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId == 0 ) goto exit;

    DayViewPopupChangePat( appointId, appView_p );

exit:
    PMC_DEC_USE( appView_p->notReady );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupDrChangeClick(TObject *Sender)
{
    Int32u_t            appointId;
    Int32u_t            referringDrId;
    Int32u_t            patientId;
    TDoctorListForm    *docListForm_p;
    pmcDocListInfo_t    docListInfo;
    pmcAppViewInfo_p    appView_p;

    appView_p = AppViewPopupGet( );

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId == 0 ) goto exit;

    referringDrId = pmcAppointDoctorId( appointId );
    patientId = pmcAppointPatientId( appointId );

    // Now put up the doctor list form
    docListInfo.mode = PMC_LIST_MODE_SELECT;
    docListInfo.doctorId = referringDrId;
    docListForm_p = new TDoctorListForm( this, &docListInfo );
    docListForm_p->ShowModal( );
    delete docListForm_p;

    // Only update database if user actually changed the patient id
    if( docListInfo.returnCode == MB_BUTTON_OK && docListInfo.doctorId != referringDrId )
    {
        // Update the referring ID in this record
        pmcSqlExecInt( PMC_SQL_TABLE_APPS,
                       PMC_SQL_APPS_FIELD_REFERRING_DR_ID,
                       docListInfo.doctorId, appointId );

        pmcAppHistory( appointId, PMC_APP_ACTION_REF_DR_ID, docListInfo.doctorId, 0, 0, 0, NIL );

        if( appView_p == &DayViewInfo )
        {
            // Not necessary to update week of month views
            DayViewInfo.updateForce = TRUE;
            DayViewGrid->Invalidate( );
        }
        AppViewInfoUpdate( MouseInRow, MouseInCol, appView_p, appointId );

        // MAB:20020413:Check referring Dr against patient record, prompt for update
        pmcCheckReferringDr( patientId, docListInfo.doctorId );
    }

exit:
    PMC_DEC_USE( appView_p->notReady );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupPatEditClick(TObject *Sender)
{
    pmcPatEditInfo_t    patEditInfo;
    TPatientEditForm   *patEditForm;
    Int32u_t            appointId;
    Int32u_t            patientId;
    pmcAppViewInfo_p    appView_p;

    appView_p = AppViewPopupGet( );

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId == 0 ) goto exit;

    patientId = pmcAppointPatientId( appointId );

    if( patientId )
    {
        patEditInfo.patientId = patientId;
        patEditInfo.mode = PMC_EDIT_MODE_EDIT;
        sprintf( patEditInfo.caption, "Patient Details" );

        patEditForm = new TPatientEditForm( NULL, &patEditInfo );
        patEditForm->ShowModal();
        delete patEditForm;

        pmcSqlRecordUnlock( PMC_SQL_TABLE_PATIENTS, patEditInfo.patientId );
        if( patEditInfo.returnCode == MB_BUTTON_OK )
        {
            if( RefreshTableForce( PMC_TABLE_BIT_PATIENTS ) )
            {
                appView_p->updateForce = TRUE;
                appView_p->grid_p->Invalidate( );
                AppViewInfoUpdate( MouseInRow, MouseInCol, appView_p, appointId );
            }
        }
    }
    else
    {
        mbDlgExclaim( "Appointment has no patient." );
    }
exit:
    PMC_DEC_USE(  appView_p->notReady );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupDrEditClick(TObject *Sender)
{
    pmcDocEditInfo_t    docEditInfo;
    TDoctorEditForm    *docEditForm;
    Int32u_t            appointId;
    Int32u_t            doctorId;
    pmcAppViewInfo_p    appView_p;

    appView_p = AppViewPopupGet( );

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId == 0 ) goto exit;

    doctorId = pmcAppointDoctorId( appointId );

    if( doctorId )
    {
        docEditInfo.id = doctorId;
        docEditInfo.mode = PMC_EDIT_MODE_EDIT;
        sprintf( docEditInfo.caption, "Referring Doctor Details" );

        if( pmcSqlRecordLock( PMC_SQL_TABLE_DOCTORS, docEditInfo.id, TRUE ) )
        {
            docEditForm = new TDoctorEditForm( NULL, &docEditInfo );
            docEditForm->ShowModal();
            delete docEditForm;

            pmcSqlRecordUnlock( PMC_SQL_TABLE_DOCTORS, docEditInfo.id );
            if( docEditInfo.returnCode == MB_BUTTON_OK )
            {
                if( RefreshTableForce( PMC_TABLE_BIT_DOCTORS ) && appView_p == &DayViewInfo )
                {
                    DayViewInfo.updateForce = TRUE;
                    DayViewGrid->Invalidate( );
                    AppViewInfoUpdate( MouseInRow, MouseInCol, appView_p, appointId );
                }
            }
        }
        else
        {
            mbDlgExclaim( "Doctor record locked for editing by another user." );
        }
    }
    else
    {
        mbDlgExclaim( "Appointment has no referring doctor." );
    }
exit:
    PMC_DEC_USE(  appView_p->notReady );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupPatViewClick(TObject *Sender)
{
    pmcPatEditInfo_t    patEditInfo;
    TPatientEditForm   *patEditForm;
    Int32u_t            appointId;
    Int32u_t            patientId;
    pmcAppViewInfo_p    appView_p;

    appView_p = AppViewPopupGet( );

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId == 0 ) goto exit;

    patientId = pmcAppointPatientId( appointId );

    if( patientId == 0 )
    {
        mbDlgExclaim( "Appointment has no patient." );
        goto exit;
    }

    patEditInfo.patientId = patientId;
    sprintf( patEditInfo.caption, "Patient Details" );
    patEditInfo.mode = PMC_EDIT_MODE_VIEW;

    patEditForm = new TPatientEditForm( NULL, &patEditInfo );
    patEditForm->ShowModal();
    delete patEditForm;

exit:

    PMC_DEC_USE(  appView_p->notReady );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupPatClearClick(TObject *Sender)
{
    Int32u_t            appointId;
    Int32u_t            patientId;
    pmcAppViewInfo_p    appView_p;
    PmcSqlPatient_p     patient_p;

    appView_p = AppViewPopupGet( );

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    mbMalloc( patient_p, sizeof(PmcSqlPatient_t) );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId == 0 ) goto exit;

    patientId = pmcAppointPatientId( appointId );

    if( patientId == 0 )
    {
        mbDlgExclaim( "Appointment has no patient." );
        goto exit;
    }

    if( pmcSqlPatientDetailsGet( patientId, patient_p ) == FALSE )
    {
        mbDlgExclaim(( "Failed to get patient details\n" ));
        goto exit;
    }

    if( mbDlgOkCancel( "Remove patient %s %s from appointment?",
                        patient_p->firstName, patient_p->lastName ) == MB_BUTTON_OK )
    {
        patientId = 0;

        // Update the patient ID in this record
        pmcSqlExecInt( PMC_SQL_TABLE_APPS,
                       PMC_SQL_APPS_FIELD_REFERRING_DR_ID,
                       0, appointId );

        pmcSqlExecInt( PMC_SQL_TABLE_APPS,
                       PMC_SQL_FIELD_PATIENT_ID,
                       patientId,
                       appointId );

        pmcSqlExecString( PMC_SQL_TABLE_APPS,
                          PMC_SQL_APPS_FIELD_COMMENT_IN,
                          "",
                          appointId );

        pmcSqlExecString( PMC_SQL_TABLE_APPS,
                          PMC_SQL_APPS_FIELD_COMMENT_OUT,
                          "",
                          appointId );

        pmcAppHistory( appointId, PMC_APP_ACTION_PATIENT_ID,  0, 0, 0, 0, NIL );
        pmcAppHistory( appointId, PMC_APP_ACTION_REF_DR_ID,   0, 0, 0, 0, NIL );
        pmcAppHistory( appointId, PMC_APP_ACTION_COMMENT,  0, 0, 0, 0, NIL );

        appView_p->updateForce = TRUE;
        appView_p->grid_p->Invalidate( );
        AppViewInfoUpdate( MouseInRow, MouseInCol, appView_p, appointId );
    }

exit:
    mbFree( patient_p );
    PMC_DEC_USE( appView_p->notReady );
    return;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupAppTypeHospitalClick( TObject *Sender )
{
    DayViewPopupAppTypeSet( PMC_APP_TYPE_HOSPITAL );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupAppTypeClinicClick(TObject *Sender)
{
    DayViewPopupAppTypeSet( PMC_APP_TYPE_CLINIC );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupAppTypeClearClick(TObject *Sender)
{
    DayViewPopupAppTypeSet( PMC_APP_TYPE_NONE );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupAppTypeNewClick(TObject *Sender)
{
    DayViewPopupAppTypeSet( PMC_APP_TYPE_NEW );
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupAppTypeReviewClick(TObject *Sender)
{
    DayViewPopupAppTypeSet( PMC_APP_TYPE_REVIEW );
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupAppTypeUrgentClick(TObject *Sender)
{
    DayViewPopupAppTypeSet( PMC_APP_TYPE_URGENT );
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupAppTypeSet( Int32u_t appType )
{
    Int32u_t            appointId;
    pmcAppViewInfo_p    appView_p;

    appView_p = AppViewPopupGet( );

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId == 0 ) goto exit;

    pmcSqlExecInt(  PMC_SQL_TABLE_APPS, PMC_SQL_APPS_FIELD_TYPE, appType, appointId );
    pmcAppHistory( appointId, PMC_APP_ACTION_TYPE, appType, 0, 0, 0, NIL );

    if( appView_p == &DayViewInfo )
    {
        DayViewInfo.updateForce = TRUE;
        DayViewGrid->Invalidate( );
    }

exit:
    PMC_DEC_USE(  appView_p->notReady );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupDrViewClick(TObject *Sender)
{
    pmcDocEditInfo_t    docEditInfo;
    TDoctorEditForm    *docEditForm;
    Int32u_t            appointId;
    Int32u_t            doctorId;
    pmcAppViewInfo_p    appView_p;

    appView_p = AppViewPopupGet( );

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId == 0 ) goto exit;

    doctorId = pmcAppointDoctorId( appointId );
    if( doctorId == 0 )
    {
        mbDlgExclaim( "Appointment has no referring doctor." );
        goto exit;
    }

    docEditInfo.id = doctorId;
    sprintf( docEditInfo.caption, "Doctor Details" );
    docEditInfo.mode = PMC_EDIT_MODE_VIEW;

    docEditForm = new TDoctorEditForm( NULL, &docEditInfo );
    docEditForm->ShowModal();
    delete docEditForm;

exit:

    PMC_DEC_USE( appView_p->notReady );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupDrClearClick(TObject *Sender)
{
    Int32u_t            appointId;
    Int32u_t            referringDrId;
    pmcAppViewInfo_p    appView_p;
    PmcSqlDoctor_p      doctor_p;

    appView_p = AppViewPopupGet( );

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );
    mbMalloc( doctor_p, sizeof(PmcSqlDoctor_t) );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId == 0 ) goto exit;

    referringDrId = pmcAppointDoctorId( appointId );

    if( referringDrId == 0 )
    {
        mbDlgExclaim( "Appointment has no referring doctor." );
        goto exit;
    }

    pmcSqlDoctorDetailsGet( referringDrId, doctor_p );

    if( mbDlgOkCancel( "Remove referring doctor %s %s from appointment?",
                       doctor_p->firstName, doctor_p->lastName ) == MB_BUTTON_OK )
    {
        pmcSqlExecInt( PMC_SQL_TABLE_APPS,
                       PMC_SQL_APPS_FIELD_REFERRING_DR_ID, 0,
                       appointId );
        pmcAppHistory( appointId, PMC_APP_ACTION_REF_DR_ID, 0, 0, 0, 0, NIL );

        if( appView_p == &DayViewInfo )
        {
            DayViewInfo.updateForce = TRUE;
            DayViewGrid->Invalidate( );
        }
        AppViewInfoUpdate( MouseInRow, MouseInCol, appView_p, appointId );
    }

exit:
    mbFree( doctor_p );
    PMC_DEC_USE( appView_p->notReady );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupCommentClick( Int32u_t mode )
{
    TAppCommentForm    *form_p;
    pmcAppEditInfo_t    appEditInfo;
    Int32u_t            appointId;
    pmcAppViewInfo_p    appView_p;

    appView_p = AppViewPopupGet( );

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId == 0 ) goto exit;

    appEditInfo.appointId = appointId;
    appEditInfo.mode = mode;
    form_p = new TAppCommentForm( this, &appEditInfo );
    form_p->ShowModal( );
    delete form_p;

    if( appEditInfo.returnCode == MB_BUTTON_OK )
    {
        nbDlgDebug(( "User clicked OK button" ));

        if( appView_p == &DayViewInfo )
        {
            DayViewInfo.updateForce = TRUE;
            DayViewGrid->Invalidate( );
        }
        AppViewInfoUpdate( MouseInRow, MouseInCol, appView_p, appointId );
    }

exit:
    PMC_DEC_USE( appView_p->notReady );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupCommentViewClick(TObject *Sender)
{
    DayViewPopupCommentClick( PMC_EDIT_MODE_VIEW );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupCommentEditClick(TObject *Sender)
{
    DayViewPopupCommentClick( PMC_EDIT_MODE_EDIT );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupConfirmPhoneClick(TObject *Sender)
{
    Int32u_t            appointId;
    pmcAppViewInfo_p    appView_p;

    if( ( appView_p = AppViewPopupGet( ) ) == NIL ) return;

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId == 0 ) goto exit;

    pmcAppointmentConfirmPhone( appointId, appView_p->providerId, TRUE );

exit:
    PMC_DEC_USE( appView_p->notReady );

#if 0
    if( appView_p == &DayViewInfo )
    {
        DayViewInfo.updateForce = TRUE;
        DayViewGrid->Invalidate( );
    }
#endif
    
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupConfirmLetterClick(TObject *Sender)
{
    Int32u_t            appointId;
    pmcAppViewInfo_p    appView_p;

    if( ( appView_p = AppViewPopupGet( ) ) == NIL )
    {
        mbDlgDebug(( "Error getting app view\n" ));
        return;
    }

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId == 0 ) goto exit;

    pmcAppointmentConfirmLetter( appointId, TRUE );

    if( appView_p == &DayViewInfo )
    {
        DayViewInfo.updateForce = TRUE;
        DayViewGrid->Invalidate( );
    }

exit:

    PMC_DEC_USE( appView_p->notReady );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupPatPrintRecordClick(TObject *Sender)
{
    Int32u_t            appointId;
    Int32u_t            patientId;
    pmcAppViewInfo_p    appView_p;

    appView_p = AppViewPopupGet( );

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId == 0 ) goto exit;

    patientId = pmcAppointPatientId( appointId );

    if( patientId == 0 )
    {
        mbDlgExclaim( "Appointment has no patient." );
        goto exit;
    }
    else
    {
        pmcPatRecordHTML( patientId );
    }
exit:
    PMC_DEC_USE(  appView_p->notReady );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupPatPrintLabelReqClick(TObject *Sender)
{
    Int32u_t            appointId;
    pmcAppViewInfo_p    appView_p;

    if( ( appView_p = AppViewPopupGet( ) ) == NIL ) return;

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId == 0 ) goto exit;

    pmcLabelPrintPatReq( appointId, appView_p->providerId, 0 );

exit:

    PMC_DEC_USE(  appView_p->notReady );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupConfirmCancelPhoneClick(TObject *Sender)
{
    Int32u_t            appointId;
    pmcAppViewInfo_p    appView_p;

    if( ( appView_p = AppViewPopupGet( ) ) == NIL ) return;

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId == 0 ) goto exit;

    pmcAppointmentConfirmPhone( appointId, 0, FALSE );

    if( appView_p == &DayViewInfo )
    {
        DayViewInfo.updateForce = TRUE;
        DayViewGrid->Invalidate( );
    }

exit:

    PMC_DEC_USE(  appView_p->notReady );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupConfirmCancelLetterClick( TObject *Sender)
{
    Int32u_t            appointId;
    pmcAppViewInfo_p    appView_p;

    if( ( appView_p = AppViewPopupGet( ) ) == NIL ) return;

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );

    if( appointId == 0 ) goto exit;

    pmcAppointmentConfirmLetter( appointId, FALSE );

    if( appView_p == &DayViewInfo )
    {
        DayViewInfo.updateForce = TRUE;
        DayViewGrid->Invalidate( );
    }

exit:
    PMC_DEC_USE(  appView_p->notReady );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupConfirmQueryClick(TObject *Sender)
{
    Int32u_t            appointId;
    pmcAppViewInfo_p    appView_p;

    if( ( appView_p = AppViewPopupGet( ) ) == NIL ) return;

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    pmcViewAppointmentConfirmation( appointId );
    PMC_DEC_USE(  appView_p->notReady );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupPatPrintLabelAddrClick(TObject *Sender)
{
    Int32u_t            appointId;
    Int32u_t            patientId;
    pmcAppViewInfo_p    appView_p;

    if( ( appView_p = AppViewPopupGet( ) ) == NIL ) return;

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId == 0 ) goto exit;

    patientId = pmcAppointPatientId( appointId );

    if( patientId )
    {
        pmcLabelPrintPatAddress( patientId );
    }
    else
    {
        mbDlgExclaim( "Appointment has no patient." );
    }

exit:

    PMC_DEC_USE(  appView_p->notReady );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupDrPrintLabelAddrClick(TObject *Sender)
{
    Int32u_t            appointId;
    Int32u_t            doctorId;
    pmcAppViewInfo_p    appView_p;

    if( ( appView_p = AppViewPopupGet( ) ) == NIL ) return;

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    if( appointId == 0 ) goto exit;

    doctorId = pmcAppointDoctorId( appointId );
    pmcLabelPrintDrAddress( doctorId );

exit:
    PMC_DEC_USE(  appView_p->notReady );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupPasteClick(TObject *Sender)
{
    pmcAppointCutBuf_p  appoint_p = NIL;
    Int32u_t            appointId;
    Char_p              buf_p;
    Int32u_t            date;
    pmcAppViewInfo_p    appView_p;
    Int32u_t            dayOfWeek;
    Boolean_t           gotLock = FALSE;

    if( ( appView_p = AppViewPopupGet( ) ) == NIL ) return;

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    mbMalloc( buf_p, 512 );

    // Cannot paste into row 0
    if( MouseInRow == 0 ) goto exit;

    mbLockAcquire( pmcAppointCutBuf_q->lock  );
    gotLock = TRUE;

    if( pmcAppointCutBuf_q->size > 0 )
    {
        appoint_p = (pmcAppointCutBuf_p)qRemoveFirst( pmcAppointCutBuf_q );

        appointId = appoint_p->appointId;

        // There are three things that might have changed between the cut
        // and the paste: time, date, and provider.

        if( appView_p == &DayViewInfo ||
            appView_p == &ProviderViewInfo )
        {
            date = SelectedDate;
        }
        else if( appView_p == &WeekViewInfo )
        {
            date = SelectedWeekViewDate[MouseInCol];
        }
        else if( appView_p == &MonthViewInfo )
        {
            date = SelectedMonthViewDate[MouseInCol];
        }
        else
        {
            // Sanity check
            mbDlgDebug(( "Unknown appointment view!" ));
        }

        // MAB:20021208: Check the day of the week
        dayOfWeek = mbDayOfWeek( date );
        if( dayOfWeek == MB_DAY_SATURDAY || dayOfWeek == MB_DAY_SUNDAY )
        {
            if( mbDlgYesNo( "Paste appointment on a %s?",
                mbDayStringsArrayLong[ dayOfWeek ] ) == MB_BUTTON_NO )
            {
                qInsertFirst( pmcAppointCutBuf_q, appoint_p );
                appoint_p = NIL;
                goto exit;
            }
        }

        sprintf( buf_p, "update %s set %s=%d, %s=%d, %s=%d where %s=%d",
                PMC_SQL_TABLE_APPS,
                PMC_SQL_FIELD_PROVIDER_ID, appView_p->providerId,
                PMC_SQL_FIELD_DATE, date,
                PMC_SQL_APPS_FIELD_START_TIME, pmcTimeSlotInts[MouseInRow],
                PMC_SQL_FIELD_ID, appointId );

        pmcSqlExec( buf_p );

        pmcAppHistory( appointId, PMC_APP_ACTION_PASTE, date, pmcTimeSlotInts[MouseInRow], appView_p->providerId, 0, NIL );

        if( pmcSqlRecordUndelete( PMC_SQL_TABLE_APPS, appointId ) == TRUE )
        {
            mbLog( "Pasted appointment %ld to date %d time %d\n", appointId, date, pmcTimeSlotInts[MouseInRow] );
            // Force update and redraw the screen
            appView_p->updateForce = TRUE;
            appView_p->grid_p->Invalidate( );
        }

        sprintf( buf_p, "%ld", pmcAppointCutBuf_q->size );
        CutBufferSizeLabel->Caption = buf_p;

        // Update the info to the pasted appointment
        AppViewInfoUpdate( MouseInRow, MouseInCol, appView_p, appointId );

        // Check to see if there are still items in the cut buffer
        if( pmcAppointCutBuf_q->size > 0 )
        {
            appoint_p = (pmcAppointCutBuf_p)qRemoveFirst( pmcAppointCutBuf_q );
            qInsertFirst( pmcAppointCutBuf_q, appoint_p );
            AppointCutBufLabel->Caption = appoint_p->desc;
            appoint_p = NIL;
        }
        else
        {
            AppointCutBufLabel->Caption = "";
        }
    }
    else
    {
        // Nothing in the cut buffer
    }

exit:

    if( gotLock )  mbLockRelease( pmcAppointCutBuf_q->lock );
    mbFree( buf_p );
    if( appoint_p ) mbFree( appoint_p );

    PMC_DEC_USE( appView_p->notReady );
    Calendar->SetFocus();

    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t __fastcall TMainForm::AppViewExtractAppointId
(
    Int32s_t            row,
    Int32s_t            col,
    pmcAppViewInfo_p    appViewInfo_p
)
{
    Int32s_t            count, prevRow, prevCount, id, prevId, i, j;
    Int32s_t            offset;
    bool                found;

    // Special case - day view columns are all the same day
    if( appViewInfo_p == &DayViewInfo ) col = 0;

    // Sanity Check
    if( col >= appViewInfo_p->cols )
    {
        mbDlgDebug(( "Error: col; %ld >= appViewCols: %ld", col, appViewInfo_p->cols ));
        return 0;
    }

    offset = col * PMC_TIMESLOTS_PER_DAY;
    row += offset;

    count = appViewInfo_p->slot_p[row].countt;

    if( count == 0 ) return 0;

    if( ( row == ( offset + 1 ) ) || count == 1 )
    {
        i = count;
        goto exit;
    }

    for( found = FALSE, prevRow = row - 1 ; ; prevRow-- )
    {
        // Loop backward thru all ids
        for( found = FALSE, i = count ; i > 0 ; i-- )
        {
            if( prevRow == offset ) break;

            id = appViewInfo_p->slot_p[row].appointId[i];

            prevCount = appViewInfo_p->slot_p[prevRow].countt;

            for( found = FALSE, j = 0 ; j <= prevCount ; j++ )
            {
                prevId = appViewInfo_p->slot_p[prevRow].appointId[j];

                if( prevId == id )
                {
                    // Found a match, this appoint extends back to prev slot
                    found = TRUE;
                    break;
                }
            }
            if( !found ) break;
        }
        if( !found ) break;
    }
    // i should contain the count number of the most logical choice

exit:

    return  appViewInfo_p->slot_p[row].appointId[i];
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32s_t __fastcall TMainForm::AppViewExtractAvailable
(
    Int32s_t            row,
    Int32s_t            col,
    pmcAppViewInfo_p    appViewInfo_p
)
{
    Int32s_t            offset;

    // Special case - day view columns are all the same day
    if( appViewInfo_p == &DayViewInfo ) col = 0;

    // Sanity Check
    if( col >= appViewInfo_p->cols )
    {
        mbDlgDebug(( "Error: col; %ld >= appViewDays: %ld", col, appViewInfo_p->cols ));
        return FALSE;
    }

    offset = col * PMC_TIMESLOTS_PER_DAY;
    row += offset;

    return (Int32s_t)appViewInfo_p->slot_p[row].available;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::AppViewInfoUpdate
(
    Int32s_t            row,
    Int32s_t            col,
    pmcAppViewInfo_p    appViewInfo_p,
    Int32u_t            appointIn
)
{
    Int32s_t            returnCode = FALSE;
    Int32u_t            appointId;
    Int32u_t            patientId;
    MbSQL               sql;
    Int32u_t            recordCount = 0;
    Int32u_t            dateOfBirth;
    Int32u_t            referringNumber;
    Int32u_t            referringId;
    Int32u_t            type = 0;
    bool                localFlag;
    Char_t              areaCode[8];
    Char_p              buf_p = NIL;
    Char_p              buf2_p = NIL;
    Char_p              firstName_p = NIL;
    Char_p              lastName_p = NIL;
    Char_p              title_p = NIL;
    Char_p              phn_p = NIL;
    Char_p              phnProv_p = NIL;
    Char_p              phone_p = NIL;
    Char_p              patComment_p = NIL;
    Char_p              appComment_p = NIL;
    Char_p              refFirstName_p = NIL;
    Char_p              refLastName_p = NIL;

    if( mbMalloc( buf_p, 2048 ) == NIL ) goto exit;
    if( mbMalloc( buf2_p, 128 ) == NIL ) goto exit;

    // Get the appointment ID
    if( appointIn )
    {
         appointId = appointIn;
    }
    else
    {
        if( appViewInfo_p )
        {
            appointId = AppViewExtractAppointId( row, col, appViewInfo_p );
        }
        else
        {
            appointId = 0;
        }
    }

    // If there is no appointment, do nothing
    if( appointId == 0 )
    {
        if( row == 0 && col == 0 )
        {
            AppInfoNameLabel->Caption       = "";
            AppInfoPhnLabel->Caption        = "";
            AppInfoPhoneLabel->Caption      = "";
            AppInfoDobLabel->Caption        = "";
            AppInfoPatCommentLabel->Caption = "";
            AppInfoAppCommentLabel->Caption = "";
            AppInfoRefLabel->Caption        = "";
            AppInfoTypeLabel->Caption       = "";
        }
        goto exit;
    }
    // It is not safe to get the info from the app view arrays, because there
    // could be conflicts.  Only safe way is to read database.

    //                          0     1     2     3     4     5     6     7     8     9
    sprintf( buf_p, "select %s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,"
                           "%s.%s,%s.%s,%s.%s,%s.%s,%s.%s"
                    " from %s,%s,%s where %s.%s=%s.%s and %s.%s=%s.%s and %s.%s=%ld",
        PMC_SQL_TABLE_PATIENTS, PMC_SQL_FIELD_FIRST_NAME,               //  0
        PMC_SQL_TABLE_PATIENTS, PMC_SQL_FIELD_LAST_NAME,                //  1
        PMC_SQL_TABLE_PATIENTS, PMC_SQL_FIELD_TITLE,                    //  2
        PMC_SQL_TABLE_PATIENTS, PMC_SQL_PATIENTS_FIELD_PHN,             //  3
        PMC_SQL_TABLE_PATIENTS, PMC_SQL_PATIENTS_FIELD_PHN_PROVINCE,    //  4
        PMC_SQL_TABLE_PATIENTS, PMC_SQL_PATIENTS_FIELD_HOME_PHONE,      //  5
        PMC_SQL_TABLE_PATIENTS, PMC_SQL_PATIENTS_FIELD_DATE_OF_BIRTH,   //  6
        PMC_SQL_TABLE_PATIENTS, PMC_SQL_PATIENTS_FIELD_COMMENT,         //  7

        PMC_SQL_TABLE_DOCTORS,  PMC_SQL_FIELD_FIRST_NAME,               //  8
        PMC_SQL_TABLE_DOCTORS,  PMC_SQL_FIELD_LAST_NAME,                //  9
        PMC_SQL_TABLE_DOCTORS,  PMC_SQL_DOCTORS_FIELD_DOCTOR_NUMBER,    //  10

        PMC_SQL_TABLE_APPS,     PMC_SQL_FIELD_PATIENT_ID,               //  11
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_REFERRING_DR_ID,     //  12
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_COMMENT_IN,          //  13
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_TYPE,                //  14

        PMC_SQL_TABLE_PATIENTS,
        PMC_SQL_TABLE_DOCTORS,
        PMC_SQL_TABLE_APPS,

        PMC_SQL_TABLE_PATIENTS, PMC_SQL_FIELD_ID, PMC_SQL_TABLE_APPS, PMC_SQL_FIELD_PATIENT_ID,
        PMC_SQL_TABLE_DOCTORS,  PMC_SQL_FIELD_ID, PMC_SQL_TABLE_APPS, PMC_SQL_APPS_FIELD_REFERRING_DR_ID,
        PMC_SQL_TABLE_APPS, PMC_SQL_FIELD_ID, appointId );

    if( sql.Query( buf_p ) == FALSE ) goto exit;

    while( sql.RowGet( ) )
    {
        // First Name
        mbMallocStr( firstName_p, sql.String( 0 ) );

        // Last Name
        mbMallocStr( lastName_p, sql.String( 1 ) );

        // Title
        mbMallocStr( title_p, sql.String( 2 ) );

        // Phn
        mbMallocStr( phn_p, sql.String( 3 ) );

        // PhnProv
        mbMallocStr( phnProv_p, sql.String( 4 ) );

        // Phone
        mbMallocStr( phone_p, sql.String( 5 ) );

        dateOfBirth = sql.Int32u( 6 );

        // Patient Comment
        mbMallocStr( patComment_p, sql.String( 7 ) );

        // Referring First Name
        mbMallocStr( refFirstName_p, sql.String( 8 ) );

        // Referring Last Name
        mbMallocStr( refLastName_p, sql.String( 9 ) );

        referringNumber     = sql.Int32u( 10 );
        patientId           = sql.Int32u( 11 );
        referringId         = sql.Int32u( 12 );

        // Appointment  Comment
        mbMallocStr( appComment_p, sql.String( 13 ) );

        type =  sql.Int32u( 14 ); //pmcDataSet_p->Fields->Fields[14]->AsInteger;

        recordCount++;
    }

    // Sanity Check
    if( recordCount != 1 ) mbDlgDebug(( "Error: recordCount: %ld", recordCount ));

    AppInfo->ActivePage = AppInfoInfo;

    // Format name for display
    sprintf( buf_p, "" );
    if( patientId )
    {
        pmcFormatName( title_p, firstName_p, NIL, lastName_p, buf_p, PMC_FORMAT_NAME_STYLE_FMLT );
    }
    AppInfoNameLabel->Caption = buf_p;

    // Format the Phn for display
    sprintf( buf_p, "" );
    if( patientId )
    {
        pmcFormatPhnDisplay( phn_p, phnProv_p, buf_p );
    }
    AppInfoPhnLabel->Caption = buf_p;

    // Format phone for display
    sprintf( buf_p, "" );
    if( patientId )
    {
        pmcPhoneFormat( phone_p, areaCode, buf2_p, &localFlag );
        if( strlen( buf2_p ) )
        {
            if( localFlag )
            {
                sprintf( buf_p, "%s", buf2_p );
            }
            else
            {
                sprintf( buf_p, "(%s) %s", areaCode, buf2_p );
            }
        }
    }
    AppInfoPhoneLabel->Caption = buf_p;

    // Format phone for display
    if( patientId && dateOfBirth )
    {
        MbDateTime dateTime = MbDateTime( dateOfBirth, 0 );
        AppInfoDobLabel->Caption = dateTime.MDY_DateString( );
    }
    else
    {
        AppInfoDobLabel->Caption = buf_p;
    }

    // Format patient comment
    sprintf( buf_p, "" );
    if( patientId )
    {
        sprintf( buf_p, "%s", patComment_p );
    }
    AppInfoPatCommentLabel->Caption = buf_p;

    // Format appointment comment
    sprintf( buf_p, "%s", appComment_p );
    AppInfoAppCommentLabel->Caption = buf_p;

    // Format referring Dr name
    sprintf( buf_p, "" );
    if( referringId )
    {
        sprintf( buf_p, "%s %s", refFirstName_p, refLastName_p );
        if( referringNumber )
        {
            sprintf( buf2_p, " (%ld)", referringNumber );
            strcat( buf_p, buf2_p );
        }
    }
    AppInfoRefLabel->Caption = buf_p;

    sprintf( buf_p, pmcAppTypeStrings[type] );
    AppInfoTypeLabel->Caption = buf_p;

    returnCode = TRUE;

exit:

    mbFree( buf_p );
    mbFree( buf2_p );

    mbFree( firstName_p );
    mbFree( lastName_p );
    mbFree( title_p );
    mbFree( phone_p );
    mbFree( phn_p );
    mbFree( phnProv_p );
    mbFree( patComment_p );
    mbFree( appComment_p );
    mbFree( refFirstName_p );
    mbFree( refLastName_p );

    if( returnCode == FALSE && appointId != 0 )
    {
        mbDlgDebug(( "Failed" ));
    }
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupHistoryClick(TObject *Sender)
{
    pmcAppViewInfo_p    appView_p;
    Int32u_t            appointId;

    appView_p = AppViewPopupGet( );

    if( appView_p == NIL ) return;

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );
    pmcAppHistoryForm( appointId );
    PMC_DEC_USE( appView_p->notReady );

    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupCutClick(TObject *Sender)
{
    Ints_t              count;
    Ints_t              i, j;
    bool                foundMatch;
    Int32u_t            offset, col;
    Int32u_t            appointId;
    pmcAppointCutBuf_p  appoint_p;
    Char_t              buf[8];
    pmcAppViewInfo_p    appView_p;

    if( ( appView_p = AppViewPopupGet( ) ) == NIL ) return;
    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    appointId = AppViewExtractAppointId( MouseInRow, MouseInCol, appView_p );

    if( appointId == 0 ) goto exit;

    nbDlgDebug(( "Cut appointment called id: %d", appointId ));

    mbLockAcquire( pmcAppointCutBuf_q->lock );

    mbMalloc( appoint_p, sizeof(pmcAppointCutBuf_t) );

    col = ( appView_p == &DayViewInfo ) ? 0 : MouseInCol;

    offset = col * PMC_TIMESLOTS_PER_DAY;

    // Must find first slot of appointment
    for( i = 0 ; i < PMC_TIMESLOTS_PER_DAY ; i++ )
    {
        // How appointments in this slot?
        count = appView_p->slot_p[offset+i].countt;
        for( foundMatch = FALSE, j = 0 ; j <= count ; j++ )
        {
            if( appView_p->slot_p[offset+i].appointId[j] == appointId )
            {
                foundMatch = TRUE;
                break;
            }
        }
        if( foundMatch ) break;
    }

    if( foundMatch )
    {
        // Mark the record as deleted
        if( pmcSqlRecordDelete( PMC_SQL_TABLE_APPS, appointId ) == TRUE )
        {
            mbLog( "Cut appointment id %ld", appointId );
            pmcAppHistory( appointId, PMC_APP_ACTION_CUT, 0, 0, 0, 0, NIL );

            appoint_p->appointId = appointId;

            // The variable i should now be the slot index of the first slot in the appointment
            sprintf( appoint_p->desc, "%s, %s",
                appView_p->slot_p[offset+i].lastName,
                appView_p->slot_p[offset+i].firstName );

            // Force update and redraw the screen
            appView_p->updateForce = TRUE;
            appView_p->grid_p->Invalidate( );

            qInsertFirst( pmcAppointCutBuf_q, appoint_p );

            sprintf( buf, "%ld", pmcAppointCutBuf_q->size );
            CutBufferSizeLabel->Caption = buf;
            AppointCutBufLabel->Caption = appoint_p->desc;
       }
        else
        {
            mbDlgDebug(( "Error cutting appointment %d", appointId ));
            // Error deleting appointment
            mbFree( appoint_p );
        }
    }
    else
    {
        mbDlgDebug(( "Error cutting appointment %d", appointId ));
        mbFree( appoint_p );
        sprintf( appoint_p->desc, "Unknown name" );
    }

    mbLockRelease( pmcAppointCutBuf_q->lock );

exit:
    PMC_DEC_USE( appView_p->notReady );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewGridDragOver(TObject *Sender,
      TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
    Ints_t      col, row;

    Accept = true;
    DayViewGrid->MouseToCell( X, Y, col, row );

    if( row > 0 && col >= 0 )
    {
        DayViewGrid->Row = row;
        DayViewGrid->Col = col;
    }
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewGridEndDrag
(
    TObject *Sender,
    TObject *Target,
    int X,  int Y
)
{
    pmcAppViewInfo_p    appView_p;

    // Restore cursor no matter what
    DayViewGrid->Cursor = crDefault;

    if( ( appView_p = AppViewPopupGet( ) ) == NIL ) return;

    PMC_CHECK_USE( appView_p->notReady );

    // Set flag to false to prevent action on other mouse clicks until done
    PMC_INC_USE( appView_p->notReady );
    DayViewInfo.endDragProviderId = SelectedProviderId;
    AppViewGridEndDrag( DayViewGrid, &DayViewInfo, X, Y );
    PMC_DEC_USE( appView_p->notReady );

    return;
}

//---------------------------------------------------------------------------
// Function: AppViewGridEndDrag
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::AppViewGridEndDrag
(
    TDrawGrid          *appViewGrid,
    pmcAppViewInfo_p    appViewInfo_p,
    int                 X,
    int                 Y
)
{
    Ints_t      row = 0, col = 0;
    Ints_t      startCol;
    Ints_t      index;
    Int32u_t    appointId;
    Int32u_t    i, j, count;
    Int32u_t    offset;
    Int32u_t    startDate, endDate;
    Int32u_t    today;
    Ints_t      firstRow = -1;
    Ints_t      lastRow = -1;
    bool        foundMatch = FALSE;
    bool        resize = FALSE;
    bool        move = FALSE;
    bool        found = FALSE;
    Char_t      buf[256];

    // Convert co-ordinates to cell
    appViewGrid->MouseToCell( X, Y, col, row );

    if( appViewGrid == DayViewGrid )
    {
        if( row == MouseInRow ) return;
    }
    else
    {
        if( row == MouseInRow && col == MouseInCol ) return;
    }

    if( row < 1 || col < 0 ) return;

    // If this is the day view we want to use column 0
    startCol = ( appViewGrid == DayViewGrid ) ? 0 : MouseInCol;

    nbDlgDebug(( "Mouse up in row %d col %d (X:%d Y:%d\n", row, col, X, Y ));
    nbDlgDebug(( "User must have dragged from r,c %d,%d to r.c %d,%d", MouseInRow, startCol, row, col ));

    // Determine starting date and end date... this depends on the view
    if( appViewGrid == DayViewGrid ||
        appViewGrid == ProviderViewGrid )
    {
        startDate = endDate = SelectedDate;
    }
    else if( appViewGrid == WeekViewGrid )
    {
        startDate = SelectedWeekViewDate[startCol];
        endDate = SelectedWeekViewDate[col];
    }
    else if( appViewGrid == MonthViewGrid )
    {
        startDate = SelectedMonthViewDate[startCol];
        endDate = SelectedMonthViewDate[col];
    }
    else
    {
        // Sanity check
        mbDlgDebug(( "Unknown appointment view!" ));
    }

    // Do not allow any kind of manipulation in the past
    today = mbToday( );
    if( startDate < today || endDate < today ) goto exit;


    // Examine starting position
    index  = startCol * PMC_TIMESLOTS_PER_DAY + MouseInRow;
    offset = startCol * PMC_TIMESLOTS_PER_DAY;

    // Check that there is actually an appointment at the start time
    count = appViewInfo_p->slot_p[index].countt;

    appointId = AppViewExtractAppointId( MouseInRow, startCol, appViewInfo_p );
    if( appointId != 0 )
    {
        if( endDate != startDate )
        {
            Int32u_t dayOfWeek = mbDayOfWeek( endDate );
            if( dayOfWeek == MB_DAY_SATURDAY || dayOfWeek == MB_DAY_SUNDAY )
            {
                if( mbDlgYesNo( "Move appointment to a %s?\n",
                    mbDayStringsArrayLong[ dayOfWeek ] ) == MB_BUTTON_NO )
                {
                    goto exit;
                }
            }
        }

        // Must search for first and last row of appointment
        for( i = 0 ; i < PMC_TIMESLOTS_PER_DAY ; i++ )
        {
            lastRow = PMC_TIMESLOTS_PER_DAY - 1;
            count = appViewInfo_p->slot_p[i+offset].countt;
            for( foundMatch = FALSE, j = 0 ; j <= count ; j++ )
            {
                if( appViewInfo_p->slot_p[i+offset].appointId[j] == appointId )
                {
                    foundMatch = TRUE;
                    break;
                }
            }

            if( foundMatch )
            {
                if( firstRow == -1 ) { firstRow = i; }
            }
            else
            {
                if( firstRow != -1 )
                {
                    lastRow = i - 1;
                    break;
                }
            }
        }
    }
    else // if( appointId != 0 )
    {
        // User must have dragged available time...
        Int32u_t    startRow = MouseInRow;
        Int32u_t    stopRow = row;
        Int16u_t    availStart;
        Int16u_t    availDrag;

        // Nothing to do
        if( startRow == stopRow ) goto exit;

        // Get starting position
        index  = startCol * PMC_TIMESLOTS_PER_DAY + startRow;
        offset = startCol * PMC_TIMESLOTS_PER_DAY;

        // Get availability of the starting cell
        availStart = appViewInfo_p->slot_p[index].available;

        // Determine if we are on a boundary.  If so, the direction on the
        // drag will determine the "color" we are dragging
        availDrag = availStart;
        if( stopRow > startRow )
        {
            // Dragging down
            if( startRow > 1 )
            {
                if( availStart != appViewInfo_p->slot_p[index-1].available )
                {
                    if( startRow < PMC_TIMESLOTS_PER_DAY - 1 )
                    {
                        if( availStart == appViewInfo_p->slot_p[index+1].available )
                        {
                            availDrag = appViewInfo_p->slot_p[index-1].available;
                            stopRow--;
                        }
                    }
                }
            }
        }
        else
        {
            // Dragging up
            if( startRow < PMC_TIMESLOTS_PER_DAY - 1 )
            {
                if( availStart != appViewInfo_p->slot_p[index+1].available )
                {
                    if( startRow > 1 )
                    {
                        if( availStart == appViewInfo_p->slot_p[index-1].available )
                        {
                            availDrag = appViewInfo_p->slot_p[index+1].available;
                            stopRow++;
                        }
                    }
                }
            }
        }

        nbDlgDebug(( "Drag avail %s start: %ld stop %ld date %ld provider %ld",
            ( availDrag ) ? "AVAIL" : "UNAVAIL", startRow, stopRow, SelectedDate, SelectedProviderId ));

        // Next we must check to see if the drag will actually change something,
        // otherwise do nothing.

        if( startRow > stopRow )
        {
            Int32u_t    temp;
            temp = startRow;
            startRow = stopRow;
            stopRow = temp;
        }

        found = FALSE;
        for( i = startRow ; i <= stopRow ; i++ )
        {
            if( availDrag != appViewInfo_p->slot_p[offset + i].available )
            {
                found = TRUE;
                break;
            }
        }

        if( found )
        {
            DayViewPopupAvailableChange( NIL, availDrag, startRow, stopRow, TRUE );
        }
        else
        {
            nbDlgDebug(( "Nothing to change\n" ));
        }
        goto exit;

    } // if( appointId != 0 )

    // Sanity check
    if( firstRow == -1 || lastRow == -1 )
    {
        mbDlgDebug(( "Could not find dragged appointment" ));
        goto exit;
    }

    nbDlgDebug(( "Found appoint id %d firstRow:%d lastRow: %d", appointId, firstRow, lastRow ));

    // Determine requested action
    if( firstRow == lastRow )
    {
        // A 1 cell appointment
        if( MouseInRowTop )
        {
            move = TRUE;
        }
        else
        {
            if( startDate == endDate )
            {
                resize = TRUE;
            }
            else
            {
                move = TRUE;
            }
        }
    }
    else if( MouseInRow == firstRow )
    {
        // First row dragged
        move = TRUE;
    }
    else if( MouseInRow == lastRow )
    {
        if( startDate == endDate )
        {
            // Last row dragged
            resize = TRUE;
        }
        else
        {
            move = TRUE;
        }
    }
    else
    {
        // middle dragged
        move = TRUE;
        nbDlgDebug(( "in middle of appoint, move" ));
    }

    // Next step is to actually carry out request
    if( move )
    {
        // For now, preform almost no error checking on requested action.
        // Allow appointment to be put anywhere execpt past end of space

        if( row + ( 1 + lastRow - firstRow ) > PMC_TIMESLOTS_PER_DAY )
        {
            mbDlgExclaim( "Cannot move appointment end time past end of day." );
            goto exit;
        }

        // Proceed with move
        nbDlgDebug(( "New start time will be %ld",  TimeSlotInts[row] ));

        sprintf( buf,  "update %s set %s=%ld,%s=%ld,%s=%ld where %s=%ld and %s=1",
                        PMC_SQL_TABLE_APPS,
                        PMC_SQL_APPS_FIELD_START_TIME, pmcTimeSlotInts[row],
                        PMC_SQL_FIELD_DATE, endDate,
                        PMC_SQL_FIELD_PROVIDER_ID, appViewInfo_p->endDragProviderId,
                        PMC_SQL_FIELD_ID, appointId,
                        PMC_SQL_FIELD_NOT_DELETED );

        pmcSqlExec( buf );

        mbLog( "Dragged appointment %d to date %d time %d\n", appointId, endDate, pmcTimeSlotInts[row] );

        pmcAppHistory( appointId, PMC_APP_ACTION_MOVE, endDate, pmcTimeSlotInts[row], appViewInfo_p->endDragProviderId, 0, NIL );

        appViewInfo_p->updateForce = TRUE;
        appViewGrid->Invalidate( );
    }

    if( resize )
    {
        Ints_t    duration;

        // Again almost no error checking
        if( row < firstRow )
        {
            mbDlgExclaim( "Cannot set appointment end time before start time." );
            goto exit;
        }

        duration = 15 * ( 1 + row - firstRow );
        if( pmcSqlExecInt( PMC_SQL_TABLE_APPS,
                           PMC_SQL_APPS_FIELD_DURATION,
                           duration, appointId ) == TRUE )
        {
            pmcAppHistory( appointId, PMC_APP_ACTION_DURATION, duration, 0, 0, 0, NIL );
            appViewInfo_p->updateForce = TRUE;
            appViewGrid->Invalidate( );
        }
    }

exit:

    Calendar->SetFocus();

    return;
}

//---------------------------------------------------------------------------
// Function: TMainForm::DayViewGridMouseDown()
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewGridMouseDown
(
    TObject            *Sender,
    TMouseButton        Button,
    TShiftState         Shift,
    int                 X,
    int                 Y
)
{
    Ints_t              row = 0, col = 0;
    TRect               cellRect;
    pmcAppViewInfo_p    appView_p;

    appView_p = AppViewPopupGet( );

    PMC_CHECK_USE( appView_p->notReady );

    // Convert co-ordinates to cell
    DayViewGrid->MouseToCell( X, Y, col, row );
    nbDlgDebug(( "Mouse Down in row %d col %d (X: %d Y: %d)\n", row, col, X, Y ));

    if( row < 1 || col < 0 || col >= PMC_DAY_VIEW_ARRAY_COLS )
    {
        MouseInRow = 0;
        MouseInCol = 0;
        return;
    }

    PMC_INC_USE( appView_p->notReady );

    cellRect = DayViewGrid->CellRect( col, row );

    AppViewInfoUpdate( row, 0, &DayViewInfo, 0 );

    if( SelectedDate >= mbToday( ) )
    {
        if( Button == mbLeft )
        {
#if 0
            // Only start a drag operation if click on an appointment
            count = DayViewInfo.slot_p[row].count;
            if( count != 0 )
            {
                appointId = DayViewInfo.slot_p[ row ].appointId[ count ];
                if( appointId != 0 )
                {
                    DayViewGrid->Cursor = crDrag;
                    DayViewGrid->BeginDrag( TRUE, 0 );
                }
            }
#else
            // Allow drag start with no app... user could be changing available time
            DayViewGrid->Cursor = crDrag;
            DayViewGrid->BeginDrag( TRUE, 0 );
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

    PMC_DEC_USE( appView_p->notReady );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewPopupNewClick(TObject *Sender)
{
    Int32u_t            appointId;
    pmcAppViewInfo_p    appView_p;
    Int32u_t            date;
    Int32u_t            dayOfWeek;
    Int32u_t            length;
    Char_p              buf_p = NIL;

    // Figure out which view we are looking at
    if( ( appView_p = AppViewPopupGet( ) ) == NIL )
    {
        mbDlgDebug(( "Error determining app view\n" ));
        return;
    }

    PMC_CHECK_USE( appView_p->notReady );
    PMC_INC_USE( appView_p->notReady );

    mbMalloc( buf_p, 1024 );

    // Determine starting date ... this depends on the view
    if( appView_p == &DayViewInfo ||
        appView_p == &ProviderViewInfo )
    {
        date = SelectedDate;
    }
    else if( appView_p == &WeekViewInfo )
    {
        date = SelectedWeekViewDate[MouseInCol];
    }
    else if( appView_p == &MonthViewInfo )
    {
        date = SelectedMonthViewDate[MouseInCol];
    }
    else
    {
        date = 0;
        // Sanity check
        mbDlgDebug(( "Unknown appointment view!" ));
    }

    // MAB:20021208: Check the day of the week
    dayOfWeek = mbDayOfWeek( date );
    if( dayOfWeek == MB_DAY_SATURDAY || dayOfWeek == MB_DAY_SUNDAY )
    {
        if( mbDlgYesNo( "Create appointment on a %s?",
            mbDayStringsArrayLong[ dayOfWeek ] ) == MB_BUTTON_NO )
        {
            goto exit;
        }
    }

    // MAB:20020407: Get the default appointment length for this provider
    sprintf( buf_p, "select %s from %s where %s=%ld",
        PMC_SQL_PROVIDERS_FIELD_APP_LENGTH,
        PMC_SQL_TABLE_PROVIDERS,
        PMC_SQL_FIELD_ID, appView_p->providerId );

    if( ( length = pmcSqlSelectInt( buf_p, NIL ) ) < 15 ) length = 15;

    appointId = pmcSqlRecordCreate( PMC_SQL_TABLE_APPS, NIL );
    if( appointId )
    {
        sprintf( buf_p, "update %s set %s=%d,%s=%d,%s=%d,%s=%d,%s=%d where %s=%d",
                 PMC_SQL_TABLE_APPS,
                 PMC_SQL_FIELD_PROVIDER_ID, appView_p->providerId,
                 PMC_SQL_FIELD_DATE, date,
                 PMC_SQL_APPS_FIELD_START_TIME, pmcTimeSlotInts[MouseInRow],
                 PMC_SQL_FIELD_PATIENT_ID, 0,
                 PMC_SQL_APPS_FIELD_DURATION, length,
                 PMC_SQL_FIELD_ID, appointId );

        pmcSqlExec( buf_p );

        pmcSqlRecordUndelete( PMC_SQL_TABLE_APPS, appointId );

        // Record appointment creation in the database
        pmcAppHistory( appointId, PMC_APP_ACTION_CREATE, date, pmcTimeSlotInts[MouseInRow], appView_p->providerId, length, NIL );

        mbLog( "Created new appointment %d date %d time %d\n", appointId, date, pmcTimeSlotInts[MouseInRow] );

        if( DayViewPopupChangePat( appointId, appView_p ) == FALSE )
        {
            AppViewInfoUpdate( MouseInRow, MouseInCol, appView_p, appointId );
            appView_p->updateForce = TRUE;
            appView_p->grid_p->Invalidate( );
        }
        AppointmentCountUpdate( PMC_COUNTER_UPDATE );
    }
    else
    {
        mbDlgDebug(( "Error creating new appointment" ));
    }

exit:

    PMC_DEC_USE( appView_p->notReady );
    if( buf_p ) mbFree( buf_p );
}
//---------------------------------------------------------------------------

Int32s_t __fastcall TMainForm::DayViewPopupChangePat
(
    Int32u_t            appointId,
    pmcAppViewInfo_p    appView_p
)
{
    Int32u_t            patientId;
    Int32u_t            referringDr;
    Int32u_t            deceasedDate;
    bool                deceasedPrompt = FALSE;
    bool                redraw = FALSE;
    Int32u_t            count;
    Char_p              cmd_p;
    TPatientListForm   *patListForm_p;
    PmcPatListInfo_t    patListInfo;
    PmcSqlPatient_p     pat_p;
    Int32s_t            returnCode = FALSE;
    Int32u_t            today;
    Int32u_t            appCount = 0;
    Int32u_t            result;

    today = mbToday( );

    mbMalloc( cmd_p, 1024 );
    mbMalloc( pat_p, sizeof( PmcSqlPatient_t ) );

    patientId = pmcAppointPatientId( appointId );

    // Now put up the patient list form
    patListInfo.patientId    = patientId;
    patListInfo.providerId   = SelectedProviderId;
    patListInfo.mode         = PMC_LIST_MODE_SELECT;
    patListForm_p = new TPatientListForm( this, &patListInfo );
    patListForm_p->ShowModal( );
    delete patListForm_p;

    // Only update database if user actually changed the patient id
    if( patListInfo.returnCode == MB_BUTTON_OK && patListInfo.patientId != patientId )
    {
        redraw = TRUE;

        // Get the patient's deceased date
        sprintf( cmd_p, "select %s from %s where %s=%ld and %s=%d",
                 PMC_SQL_PATIENTS_FIELD_DATE_OF_DEATH,
                 PMC_SQL_TABLE_PATIENTS,
                 PMC_SQL_FIELD_ID,
                 patListInfo.patientId,
                 PMC_SQL_FIELD_NOT_DELETED,
                 PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

        deceasedDate = pmcSqlSelectInt( cmd_p, &count );

        // Sanity check
        if( count != 1 ) mbDlgDebug(( "error patient id: %ld count: %ld\n",  patListInfo.patientId, count ));

        if( deceasedDate > 0 )
        {
            deceasedPrompt = TRUE;
            goto exit;
        }

        // Get the patient's default referring Dr. ID
        sprintf( cmd_p, "select %s from %s where %s=%ld and %s=%d",
                 PMC_SQL_PATIENTS_FIELD_REFERRING_DR_ID,
                 PMC_SQL_TABLE_PATIENTS,
                 PMC_SQL_FIELD_ID,
                 patListInfo.patientId,
                 PMC_SQL_FIELD_NOT_DELETED,
                 PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

        referringDr = pmcSqlSelectInt( cmd_p, NIL );

        sprintf( cmd_p, "update %s set %s=%ld,%s=%ld,%s=\"\",%s=\"\" where %s=%ld",
            PMC_SQL_TABLE_APPS,
            PMC_SQL_APPS_FIELD_REFERRING_DR_ID, referringDr,
            PMC_SQL_FIELD_PATIENT_ID, patListInfo.patientId,
            PMC_SQL_APPS_FIELD_COMMENT_IN,
            PMC_SQL_APPS_FIELD_COMMENT_OUT,
            PMC_SQL_FIELD_ID, appointId );

        pmcSqlExec( cmd_p );
        mbLog( "Updated appointment %d patient id to %d\n", appointId, patListInfo.patientId );

        pmcAppHistory( appointId, PMC_APP_ACTION_PATIENT_ID,  patListInfo.patientId, 0, 0, 0, NIL );
        pmcAppHistory( appointId, PMC_APP_ACTION_REF_DR_ID,   referringDr, 0, 0, 0, NIL );
        pmcAppHistory( appointId, PMC_APP_ACTION_COMMENT,  0, 0, 0, 0, NIL );
        // pmcAppHistory( appointId, PMC_APP_ACTION_COMMENT_2, 0, 0, 0, 0, NIL );

        returnCode = TRUE;

        // MAB:20020504: Check to see if this patient has any other appointments
        sprintf( cmd_p, "select %s from %s where %s=%ld and %s>=%ld and %s=%ld",
            PMC_SQL_FIELD_ID,
            PMC_SQL_TABLE_APPS,
            PMC_SQL_FIELD_PATIENT_ID, patListInfo.patientId,
            PMC_SQL_FIELD_DATE, today,
            PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

        pmcSqlSelectInt( cmd_p, &appCount );
    }
    else
    {
        nbDlgDebug(( "User must have pressed cancel button "));
    }

exit:

    if( redraw )
    {
        appView_p->updateForce = TRUE;
        appView_p->grid_p->Invalidate( );
        AppViewInfoUpdate( MouseInRow, MouseInCol, appView_p, appointId );
    }

    if( deceasedPrompt || appCount > 1 )
    {
        if( pmcSqlPatientDetailsGet( patListInfo.patientId, pat_p ) != TRUE )
        {
            mbDlgDebug(( "Error getting patient id %d details\n", patientId ));
            goto exit2;
        }
    }

    if( deceasedPrompt )
    {
        mbDlgInfo( "%s %s has been marked as deceased.\n"
                       "New appointments cannot be created for deceased patients.\n",
                       pat_p->firstName, pat_p->lastName );
    }

    if( appCount > 1 )
    {
        Int32u_t    providerId;
        Int32u_t    date;

        if( appCount == 2 )
        {
            result = mbDlgYesNo( "%s %s has 1 other appointment booked.\n"
                                 "Would you like to see the appointment list?",
                                  pat_p->firstName, pat_p->lastName );
        }
        else
        {
            result = mbDlgYesNo( "%s %s has %ld other appointments booked.\n"
                                 "Would you like to see the appointment list?",
                                  pat_p->firstName, pat_p->lastName, appCount - 1 );
        }

        if( result == MB_BUTTON_YES )
        {
            pmcViewAppointments( patListInfo.patientId, TRUE, FALSE, TRUE,
                &providerId, &date, FALSE, PMC_LIST_MODE_LIST );

            AppointmentGoto( providerId, date );
        }
    }

exit2:

    mbFree( cmd_p );
    mbFree( pat_p );
    return returnCode;
}


