//---------------------------------------------------------------------------
// File:    pmcMainForm.cpp
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
#include <dos.h>
#include <dir.h>
#include <process.h>
#include <Filectrl.hpp>
#pragma hdrstop

#include "mbUtils.h"
#include "mbSqlLib.h"
#include "pmcUtils.h"

#define PMC_INIT_GLOBALS
#include "pmcTables.h"
#include "pmcColors.h"
#include "pmcGlobals.h"
#include "pmcPatientForm.h"
#undef PMC_INIT_GLOBALS

#include "pmcInitialize.h"
#include "pmcMainForm.h"
#include "pmcPatientListForm.h"
#include "pmcDateSelectForm.h"
#include "pmcPollTableThread.h"
#include "pmcPromptForm.h"
#include "pmcPatientEditForm.h"
#include "pmcDoctorListForm.h"
#include "pmcDoctorEditForm.h"
#include "pmcAppLettersForm.h"
#include "pmcClaimEditForm.h"
#include "pmcDatabaseMaint.h"
#include "pmcSeiko240.h"
#include "pmcAboutForm.h"
#include "pmcClaimListForm.h"
#include "pmcMspClaimsin.h"
#include "pmcMspValidrpt.h"
#include "pmcMspReturns.h"
#include "pmcMspNotice.h"
#include "pmcReportForm.h"
#include "pmcDocumentEditForm.h"
#include "pmcBatchImportForm.h"
#include "pmcDocumentListForm.h"
#include "pmcWordCreateForm.h"
#include "pmcSelectForm.h"
#include "pmcProviderView.h"
#include "pmcPatientForm.h"
#include "pmcIcdForm.h"
#include "pmcAppHistoryForm.h"
#include "pmcEchoImportForm.h"
#include "pmcEchoCDContentsForm.h"
#include "pmcEchoListForm.h"
#include "pmcMedListForm.h"

#pragma package(smart_init)
#pragma link "CCALENDR"
#pragma resource "*.dfm"

TMainForm *MainForm;

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

__fastcall TMainForm::TMainForm(TComponent* Owner)
    : TForm(Owner)
{
    {
        Ints_t height, width, top, left;
        if( mbPropertyWinGet( PMC_WINID_MAIN, &height, &width, &top, &left ) == MB_RET_OK )
        {
            // MAB:20040919: There seems to be some kind of bug where the
            // window is getting created with a 0 size.  So do not set bounds
            // of main window unless the height and width look reasonable
            if( width > 50 && height > 50 && top > 0 && left > 0 && top < 1024 && left < 1024 )
            {
                SetBounds( left, top, width, height );
            }
        }
    }
    DayViewInfo.notReady = 1;
    WeekViewInfo.notReady = 1;
    MonthViewInfo.notReady = 1;
    ProviderViewInfo.notReady = 1;

    // Set the program caption
    {
        Char_p  buf_p;
        mbMalloc( buf_p, 512 );
        sprintf( buf_p, "%s - Database '%s'", PMC_NAME, pmcCfg[CFG_DATABASE_DESC].str_p );
        //sprintf( buf_p, "%s", PMC_NAME );
        Caption = buf_p;
        mbFree( buf_p );
    }

    SkipUpdateProviderLabelInvalidate = FALSE;

    DayViewInfo.updateForce      = FALSE;
    DayViewInfo.updateSkip       = FALSE;

    WeekViewInfo.updateForce     = FALSE;
    WeekViewInfo.updateSkip      = FALSE;

    MonthViewInfo.updateForce    = FALSE;
    MonthViewInfo.updateSkip     = FALSE;

    ProviderViewInfo.updateForce = FALSE;
    ProviderViewInfo.updateSkip  = FALSE;

    DayViewInfo.grid_p          = DayViewGrid;
    WeekViewInfo.grid_p         = WeekViewGrid;
    MonthViewInfo.grid_p        = MonthViewGrid;
    ProviderViewInfo.grid_p     = ProviderViewGrid;

    DayViewInfo.endDragProviderId       = 0;
    WeekViewInfo.endDragProviderId      = 0;
    MonthViewInfo.endDragProviderId     = 0;
    ProviderViewInfo.endDragProviderId  = 0;

    CalendarSkipUpdate      = FALSE;
    CalendarSkipRefresh     = FALSE;
    CalendarMonthSkipUpdate = FALSE;

    SkipTimer = FALSE;

    if( mbLog( "Initialization complete" ) == FALSE )
    {
        mbDlgError( "Log system not functioning!" );
    }

#if 0
    if( pmcCfg[CFG_USERNAME].str_p && pmcCfg[CFG_PASSWORD].str_p )
    {
        // Don't put up splash screen if no login prompt (can't get it to work)
        Splash = NIL;
    }
    else
    {
        // Put up a splash screen
        Splash = new TSplashScreen( NIL );
        Splash->Show( );
    }
#endif

    if( pmcCfg[CFG_NEW_PAT_FORM_FLAG].value == TRUE )
    {
        TestPatientFormModal->Visible = TRUE;
        TestPatientFormNonModal->Visible = TRUE;
    }

    PollThread_p = NIL;

    // Initialize the table status structures
    for( Ints_t i = 0 ; i < PMC_TABLE_COUNT ; i++ )
    {
        tableStatus[i].name = pmcTableNames_p[i];
        tableStatus[i].bit = pmcTableBits[i];
        tableStatus[i].curModifyTime = 0ui64;
        tableStatus[i].newModifyTime = 0ui64;
        tableStatus[i].lastReadTime = 0ui64;
        tableStatus[i].curDataSize = 0;
        tableStatus[i].newDataSize = 0;
        // mbDlgDebug(( "Got table name %s\n", VarToStr( tableStatus[i].name ) ));
    }

    SelectedProviderId = 0;
    SelectedDate = 0;

    for( Ints_t i = 0 ; i < PMC_WEEK_VIEW_COLS ; i++ )
    {
        SelectedWeekViewYear[i]  = 0;
        SelectedWeekViewMonth[i] = 0;
        SelectedWeekViewDay[i]   = 0;
        SelectedWeekViewDate[i]  = 0;
    }

    for( Ints_t i = 0 ; i < PMC_MONTH_VIEW_COLS ; i++ )
    {
        SelectedMonthViewDate[i] = 0;
    }

    DayViewInfo.providerId = 0;
    DayViewInfo.startDate = 0;
    DayViewInfo.cols = PMC_DAY_VIEW_COLS;
    DayViewInfo.type = PMC_APP_VIEW_TYPE_DAY;

    WeekViewInfo.startDate = 0;
    WeekViewInfo.providerId = 0;
    WeekViewInfo.cols = PMC_WEEK_VIEW_COLS;
    WeekViewInfo.type = PMC_APP_VIEW_TYPE_WEEK;

    MonthViewInfo.startDate = 0;
    MonthViewInfo.providerId = 0;
    MonthViewInfo.cols = PMC_MONTH_VIEW_COLS;
    MonthViewInfo.type = PMC_APP_VIEW_TYPE_MONTH;

    ProviderViewInfo.providerId = 0;
    ProviderViewInfo.startDate = 0;
    ProviderViewInfo.cols = PMC_PROVIDER_VIEW_COLS;
    ProviderViewInfo.type = PMC_APP_VIEW_TYPE_PROVIDER;

    // Signal that we should get the tables initial update times.
    // Can't do it here because the database has not been read yet.
    RefreshTableInitFlag = TRUE;

    // Delay initlializng the internal patient list
    UpdatePatientListDoInit = FALSE;
    UpdatePatientListDone = FALSE;

    // Delay initlializng the internal doctor list
    UpdateDoctorListDoInit = FALSE;
    UpdateDoctorListDone = FALSE;

    TestColor   = 0;
    MouseInRow  = 0;
    MouseInCol  = 0;
    CheckTables = TRUE;

    // Set the date over the calendar
    Calendar->UseCurrentDate = TRUE;
    UpdateDate( Calendar->Year, Calendar->Month, Calendar->Day );

    // Allocate space for appointment arrays
    mbCalloc( DayViewInfo.slot_p,      DayViewInfo.cols       * PMC_TIMESLOTS_PER_DAY * sizeof( pmcTimeSlotInfo_t ) );
    mbCalloc( WeekViewInfo.slot_p,     WeekViewInfo.cols      * PMC_TIMESLOTS_PER_DAY * sizeof( pmcTimeSlotInfo_t ) );
    mbCalloc( MonthViewInfo.slot_p,    MonthViewInfo.cols     * PMC_TIMESLOTS_PER_DAY * sizeof( pmcTimeSlotInfo_t ) );
    mbCalloc( ProviderViewInfo.slot_p, ProviderViewInfo.cols  * PMC_TIMESLOTS_PER_DAY * sizeof( pmcTimeSlotInfo_t ) );

    for( Ints_t i = 0 ; i < PMC_TIMESLOTS_PER_DAY ; i++ )
    {
        for( Ints_t j = 0 ; j < PMC_DAY_VIEW_ARRAY_COLS ; j++ )
        {
            DayViewCellArray[j][i].str = "";
            DayViewCellArray[j][i].color =  pmcColor[ PMC_COLOR_BREAK ];
        }
        for( Ints_t j = 0 ; j < WeekViewInfo.cols ; j++ )
        {
            WeekViewCellArray[j][i].str = "";
            WeekViewCellArray[j][i].color =  pmcColor[ PMC_COLOR_BREAK ];
        }
        for( Ints_t j = 0 ; j < MonthViewInfo.cols ; j++ )
        {
            MonthViewCellArray[j][i].str = "";
            MonthViewCellArray[j][i].color =  pmcColor[ PMC_COLOR_BREAK ];
        }

        for( Ints_t j = 0 ; j < ProviderViewInfo.cols ; j++ )
        {
            ProviderViewCellArray[j][i].str = "";
            ProviderViewCellArray[j][i].color =  pmcColor[ PMC_COLOR_BREAK ];
        }
    }

    // Initialize view arrays
    for( Ints_t i = 0 ; i < PMC_TIMESLOTS_PER_DAY ; i++ )
    {
        DayViewInfo.slot_p[i].startTime = pmcTimeSlotInts[i];

        for( Ints_t j = 0 ; j < WeekViewInfo.cols ; j++ )
        {
            WeekViewInfo.slot_p[j * PMC_TIMESLOTS_PER_DAY + i].startTime = pmcTimeSlotInts[i];
        }
        for( Ints_t j = 0 ; j < MonthViewInfo.cols ; j++ )
        {
            MonthViewInfo.slot_p[j * PMC_TIMESLOTS_PER_DAY + i].startTime = pmcTimeSlotInts[i];
        }
        for( Ints_t j = 0 ; j < ProviderViewInfo.cols ; j++ )
        {
            ProviderViewInfo.slot_p[j * PMC_TIMESLOTS_PER_DAY + i].startTime = pmcTimeSlotInts[i];
        }
    }
    CutBufferSizeLabel->Caption = "0";

    // Zero out the appointment info tab form
    AppViewInfoUpdate( 0, 0, NIL, 0 );

    if( pmcNewPatForm )
    {
        TestPatientFormNonModal->Visible = TRUE;
        TestPatientFormModal->Visible = TRUE;
    }
}


//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
// Main form destructor
//---------------------------------------------------------------------------

__fastcall TMainForm::~TMainForm( void )
{
    if( pmcMainFormCloseRan )
    {

    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
    pmcPatRecordStruct_p    patRecord_p;
    pmcDocRecordStruct_p    docRecord_p;
    pmcLinkageStruct_p      linkage_p;
    pmcProviderList_p       provider_p;
    pmcDocumentItem_p       document_p;
    
    Int32u_t                i;
    TThermometer           *thermometer_p;
    Int32u_t                records;
    Char_t                  buf[128];

    mbPropertyWinSave( PMC_WINID_MAIN, Height, Width, Top, Left );
    Action = caFree;
    pmcMainFormCloseRan = TRUE;

    mbSqlTerminateRequest( );

    mbLog( "Shutting down" );

    records  = pmcPatName_q->size;
    records += pmcDocName_q->size;
    records += pmcDocument_q->size;

    sprintf( buf, "Shutdown in progress... " );
    thermometer_p = new TThermometer( buf, 0, records, FALSE );

    // Attempt to kill polling thread after thermometer is up.  This way
    // user has an indication that shutdown is in progress
    if( PollThread_p )
    {
        PollThread_p->Terminate();

        for( i = 0 ; ; i++ )
        {
            if( pmcThreadTicks == 0 ) break;

            if( i == 5 )
            {
                mbLog( "Timeout waiting for PollThread to terminate." );
                break;
            }
            Sleep( 1000 );
        }
    }

    mbLockAcquire( pmcDocListLock );
    for( ; ; )
    {
        if( qEmpty( pmcDocDelete_q ) ) break;

        // get pointer by taking off queue then putting back on
        linkage_p = (pmcLinkageStruct_p)qRemoveFirst( pmcDocDelete_q );
        docRecord_p = (pmcDocRecordStruct_p)linkage_p->record_p;
        DoctorRecordFree( docRecord_p );
        thermometer_p->Increment();
    }

    // Delete patient list
    mbLockAcquire( pmcPatListLock );
    for( ; ; )
    {
        if( qEmpty( pmcPatDelete_q ) ) break;

        // get pointer by taking off queue then putting back on
        linkage_p = (pmcLinkageStruct_p)qRemoveFirst( pmcPatDelete_q );
        patRecord_p = (pmcPatRecordStruct_p)linkage_p->record_p;
        PatientRecordFree( patRecord_p );
        thermometer_p->Increment();
    }

    // Delete document list
    for( ; ; )
    {
        if( qEmpty( pmcDocument_q ) ) break;

        // get pointer by taking off queue then putting back on
        document_p = (pmcDocumentItem_p)qRemoveFirst( pmcDocument_q );
        pmcDocumentListFreeItem( document_p );
        thermometer_p->Increment();
    }

    // Delete list of available times
#if PMC_AVAIL_TIME
    {
        pmcAvailTimeStruct_p   availTime_p;

        for( ; ; )
        {
            if( qEmpty( pmcAvailTime_q ) ) break;

            availTime_p = (pmcAvailTimeStruct_p)qRemoveFirst( pmcAvailTime_q );
            PMC_FREE( availTime_p );
        }
    }
#endif

#if 0
    // Sanity check
    if( pmcPatName_q->size != 0 || pmcPatPhone_q->size != 0 || pmcPatPhn_q->size != 0 )
    {
        mbDlgDebug(( "Error freeing internal patient list" ));
    }

    // Sanity check
    if( pmcDocName_q->size != 0 || pmcDocPhone_q->size != 0 || pmcDocNum_q->size != 0 )
    {
        mbDlgDebug(( "Error freeing internal doctor list" ));
    }
#endif

    mbLockRelease( pmcPatListLock );
    mbLockRelease( pmcDocListLock );

    // Delete provider list
    for( ; ; )
    {
        if( qEmpty( pmcProvider_q ) ) break;
        provider_p = (pmcProviderList_p)qRemoveFirst( pmcProvider_q );

        mbFree( provider_p->description_p );
        mbFree( provider_p->docSearchString_p );
        mbFree( provider_p->lastName_p );
        mbFree( provider_p );
    }

    delete thermometer_p;

    // Must delete the created documents if they were not edited
    {
        mbFileListStruct_p document_p;
        Int32u_t    newCrc;
        Int32u_t    newSize;

        while( !qEmpty( pmcCreatedDocument_q ) )
        {
            document_p = (mbFileListStruct_p)qRemoveFirst( pmcCreatedDocument_q );

            newCrc = mbCrcFile( document_p->name_p, &newSize, NIL, NIL, NIL, NIL, NIL );

            if( newCrc == document_p->crc && newSize == (Int32u_t)document_p->size64 )
            {
                mbLog( "Not Delete unmodified document '%s'\n", document_p->name_p );
//                unlink( document_p->name_p );
            }
            else
            {
                //mbDlgDebug(( "document '%s' modified or not found\n", document_p->name_p ));
            }
            mbFileListFreeElement( document_p );
        }
    }
    mbFree( DayViewInfo.slot_p );
    mbFree( WeekViewInfo.slot_p );
    mbFree( MonthViewInfo.slot_p );
    mbFree( ProviderViewInfo.slot_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::PatientListButtonClick( TObject *Sender )
{
    TPatientListForm   *patListForm_p;
    PmcPatListInfo_t    patListInfo;

    patListInfo.patientId = 0;
    patListInfo.providerId = SelectedProviderId;
    patListInfo.mode = PMC_LIST_MODE_LIST;
    patListInfo.allowGoto = TRUE;

    patListForm_p = new TPatientListForm( this, &patListInfo );
    patListForm_p->ShowModal( );
    delete patListForm_p;

    if( patListInfo.gotoProviderId !=0 && patListInfo.gotoDate != 0 )
    {
        AppointmentGoto( patListInfo.gotoProviderId, patListInfo.gotoDate );
    }
    Calendar->SetFocus();
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::UpdateProviderListBox( TObject *Sender )
{
    pmcProviderList_p       provider_p;
    pmcProviderList_p       provider2_p;
    pmcProviderViewList_p   providerView_p;
    Int32u_t                i;
    bool                    inserted;
    MbSQL                   sql;

    if( sql.Query( "select * from providers where id > 0" ) == FALSE ) return;

    // Clear out any entries in the items list
    ProvidersListBox->Items->Clear();

    mbLockAcquire( pmcProvider_q->lock );
    while( !qEmpty( pmcProvider_q ) )
    {
        provider_p = (pmcProviderList_p)qRemoveFirst( pmcProvider_q );
        if( provider_p->docSearchString_p ) mbFree( provider_p->docSearchString_p );
        if( provider_p->description_p )     mbFree( provider_p->description_p );
        mbFree( provider_p );
    }

    while( sql.RowGet( ) )
    {
        // Add to provider list
        mbCalloc( provider_p, sizeof(pmcProviderList_t) );

        // Get the provider description
        mbMallocStr( provider_p->description_p, sql.String( 3 ) );

        // Get document search string
        mbMallocStr( provider_p->docSearchString_p, sql.String( 9 )  );

        mbMallocStr( provider_p->lastName_p, sql.String( 1 ) );
        provider_p->lastNameLen = strlen( provider_p->lastName_p );

        provider_p->id              = sql.Int32u( 0 );
        provider_p->providerNumber  = sql.Int32u( 10 );
        provider_p->picklistOrder   = sql.Int32u( 13 );

        provider_p->listIndex        = -1;
        provider_p->patientCount     = -1;
        provider_p->documentCount    = -1;
        provider_p->appointmentCount = -1;

        inserted = FALSE;

        // Loop through the list adding the entries in the desired order
        qWalk( provider2_p, pmcProvider_q, pmcProviderList_p )
        {
            if( provider_p->picklistOrder < provider2_p->picklistOrder )
            {
                qInsertBefore( pmcProvider_q, provider2_p, provider_p );
                inserted = TRUE;
                break;
            }
        }

        // Add to end of list
        if( !inserted ) qInsertLast( pmcProvider_q, provider_p );
    }

    // Clear out list of providers to show in provider list view
    mbLockAcquire( pmcProviderView_q->lock );

    while( !qEmpty( pmcProviderView_q ) )
    {
        providerView_p = (pmcProviderViewList_p)qRemoveFirst( pmcProviderView_q );
        mbFree( providerView_p );
    }

    // Next we must add the items to the pick list
    i = 0;

    qWalk( provider_p, pmcProvider_q, pmcProviderList_p )
    {
        if( provider_p->picklistOrder > 0 )
        {
            ProvidersListBox->Items->Add( provider_p->description_p );
            provider_p->listIndex = i++;

            // Temporary - build list of providers to display in provider view
            if( pmcProviderView_q->size < PMC_PROVIDER_VIEW_COLS )
            {
                mbMalloc( providerView_p, sizeof(pmcProviderViewList_t) );
                providerView_p->providerId = provider_p->id;
                qInsertLast( pmcProviderView_q, providerView_p );
            }
        }
    }
    mbLockRelease( pmcProviderView_q->lock );

    if( ProvidersListBox->Items->Count > 0 )
    {
        //sprintf( buf, "Provider items count: %d\n", ProviderListBox->Items->Count );
        //MessageBox( NULL, buf, "", MB_OK );
    }
    else
    {
        MessageBox( NULL, "No Service Providers were found in the database.\n", PMC_NAME,
            MB_OK | MB_ICONEXCLAMATION );

        // Add description to the list box
        ProvidersListBox->Items->Add( "Unknown" );
    }

    mbLockRelease( pmcProvider_q->lock );

    // Select the first item in the list
    ProvidersListBox->ItemIndex = 0;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void  TimeGridDrawCell
(
    TObject        *Sender,
    int             ACol,
    int             ARow,
    TRect           &Rect,
    TGridDrawState  State,
    TDrawGrid      *Grid_p
)
{
    AnsiString str = "";
    // Output the time of day in column 0
    if( ACol == 0 )
    {
        if( ARow >= 0 && ARow <= PMC_TIMESLOTS_PER_DAY - 2 )
        {
            str = pmcTimeSlotString[ARow + 1];
            Grid_p->Canvas->TextRect( Rect, Rect.Left, Rect.Top, str );
        }
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::ProvidersListBoxChange
(
    TObject *Sender
)
{
    UpdateProviderLabel( );
}

//---------------------------------------------------------------------------
// Function: UpdateProviderLabel( )
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::UpdateProviderLabel( void )
{
    if( AppointmentTabs->ActivePage == ProviderViewTab )
    {
        ProviderLabel->Caption = "Provider View";
        ProviderViewGrid->Invalidate( );
        ProvidersListBox->Visible = FALSE;
    }
    else
    {
        ProvidersListBox->Visible = TRUE;
        if( ProvidersListBox->ItemIndex == -1 )
        {
            // An invalid selection was made
        }
        else
        {
            ProviderLabel->Caption = ProvidersListBox->Items->Strings[ProvidersListBox->ItemIndex];
            SelectedProviderId    = pmcProviderIdGet( ProvidersListBox->Text.c_str() );

            if( SkipUpdateProviderLabelInvalidate == FALSE )
            {
                // Redraw day grid view
                DayViewGrid->Invalidate( );
                WeekViewGrid->Invalidate( );
                MonthViewGrid->Invalidate( );
            }

            PatientCountUpdate( PMC_COUNTER_READ );
            AppointmentCountUpdate( PMC_COUNTER_READ );
            DocumentCountUpdate( PMC_COUNTER_READ );
        }
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::RefreshTableQuery( void )
{
    Ints_t          i;

    for( i = 0 ; i < PMC_TABLE_COUNT ; i++ )
    {
        tableStatus[i].newModifyTime = pmcPollTableModifyTime[i];
        tableStatus[i].newDataSize   = pmcPollTableSize[i];
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t __fastcall TMainForm::RefreshTable( Int32u_t tableBitMask )
{
    Variant         name;
    Variant         Time;
    Ints_t          i;
    Int32u_t        returnCode = 0;

    // Sanity check... flag should not already be FALSE
    if( RefreshTableInitFlag == TRUE )
    {
        RefreshTableInit( );
    }

    // Get the current table settings
    RefreshTableQuery( );

    for( i = 0 ; i < PMC_TABLE_COUNT ; i++ )
    {
        if( tableStatus[i].curModifyTime != tableStatus[i].newModifyTime ||
            tableStatus[i].curDataSize   != tableStatus[i].newDataSize )
        {
            if( tableStatus[i].bit & tableBitMask )
            {
                //mbDlgDebug(( "Must Update table %s curTime: %Ld newTime: %Ld curLen %ld newLen %ld",
                //    VarToStr( tableStatus[i].name ),
                //    tableStatus[i].curModifyTime,
                //    tableStatus[i].newModifyTime,
                //    tableStatus[i].curDataLength,
                //    tableStatus[i].newDataLength ));

                if( tableStatus[i].bit & PMC_TABLE_BIT_PATIENTS )
                {
                    UpdatePatientList( &tableStatus[i], FALSE );
                }

                if( tableStatus[i].bit & PMC_TABLE_BIT_DOCTORS )
                {
                    UpdateDoctorList(  &tableStatus[i], FALSE );
                }

                if( tableStatus[i].bit & PMC_TABLE_BIT_PROVIDERS )
                {
                    // Get latest time
                    //mbDlgDebug(( "Must update table %s (cur: %Ld new: %Ld)",
                    //    tableStatus[i].name,
                    //    tableStatus[i].curModifyTime_p->Int64(),
                    //    tableStatus[i].newModifyTime_p->Int64() ));

                    tableStatus[i].curModifyTime = tableStatus[i].newModifyTime;
                    tableStatus[i].curDataSize   = tableStatus[i].newDataSize;
                }
                if( tableStatus[i].bit & PMC_TABLE_BIT_APPS )
                {
                    // Indicated updated time without accessing table.  This is generally
                    // not a good idea but should work if I am only calling table refresh
                    // from one spot
                    tableStatus[i].curModifyTime = tableStatus[i].newModifyTime;
                    tableStatus[i].curDataSize   = tableStatus[i].newDataSize;
                }
                if( tableStatus[i].bit & PMC_TABLE_BIT_TIMESLOTS )
                {
                    // Indicated updated time without accessing table.  This is generally
                    // not a good idea but should work if I am only calling table refresh
                    // from one spot
                    tableStatus[i].curModifyTime = tableStatus[i].newModifyTime;
                    tableStatus[i].curDataSize   = tableStatus[i].newDataSize;
                }

                // Set bit in return mask indicating table refresh
                returnCode |= tableStatus[i].bit;
            }
            else
            {
             //   mbDlgDebug(( "Detected update on non-requested table (bit 0x%08lX)", tableStatus[i].bit ));
            }
        }
    }
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t __fastcall TMainForm::RefreshTableForce( Int32u_t tableBitMask )
{
    Variant         name;
    Variant         Time;
    Ints_t          i;
    Int32u_t        returnCode = 0;

    // Sanity check... flag should not already be FALSE
    if( RefreshTableInitFlag == TRUE )
    {
        mbDlgDebug((  "ERROR: TMainForm::RefreshTable called with RefreshTableInitFlag == TRUE" ));
        goto exit;
    }

    for( i = 0 ; i < PMC_TABLE_COUNT ; i++ )
    {
            if( tableStatus[i].bit & tableBitMask )
            {
                //mbDlgDebug(( "Must Update table %s curTime: %Ld newTime: %Ld curLen %ld newLen %ld",
                //    VarToStr( tableStatus[i].name ),
                //    tableStatus[i].curModifyTime,
                //    tableStatus[i].newModifyTime,
                //    tableStatus[i].curDataLength,
                //    tableStatus[i].newDataLength ));

                if( tableStatus[i].bit & PMC_TABLE_BIT_PATIENTS )
                {
                    UpdatePatientList( &tableStatus[i], TRUE );
                }

                if( tableStatus[i].bit & PMC_TABLE_BIT_DOCTORS )
                {
                    UpdateDoctorList( &tableStatus[i], TRUE );
                }

                if( tableStatus[i].bit & PMC_TABLE_BIT_PROVIDERS )
                {
                    // Get latest time
                    //mbDlgDebug(( "Must update table %s (cur: %Ld new: %Ld)",
                    //    tableStatus[i].name,
                    //    tableStatus[i].curModifyTime_p->Int64(),
                    //    tableStatus[i].newModifyTime_p->Int64() ));

                    tableStatus[i].curModifyTime = tableStatus[i].newModifyTime;
                    tableStatus[i].curDataSize = tableStatus[i].newDataSize;
                }
                if( tableStatus[i].bit & PMC_TABLE_BIT_APPS )
                {
                    // Indicated updated time without accessing table.  This is generally
                    // not a good idea but shoud work if I am only calling table refresh
                    // from one spot
                    tableStatus[i].curModifyTime = tableStatus[i].newModifyTime;
                    tableStatus[i].curDataSize   = tableStatus[i].newDataSize;
                }
                if( tableStatus[i].bit & PMC_TABLE_BIT_TIMESLOTS )
                {
                    // Indicated updated time without accessing table.  This is generally
                    // not a good idea but should work if I am only calling table refresh
                    // from one spot
                    tableStatus[i].curModifyTime = tableStatus[i].newModifyTime;
                    tableStatus[i].curDataSize   = tableStatus[i].newDataSize;
                }

                // Set bit in return mask indicating table refresh
                returnCode |= tableStatus[i].bit;
            }
            else
            {
             //   mbDlgDebug(( "Detected update on non-requested table (bit 0x%08lX)", tableStatus[i].bit ));
            }
    }
exit:

    return returnCode;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::RefreshTableInit( void )
{
    Ints_t          i;

    //Start the table polling thread
    if( PollThread_p == NIL )
    {
        PollThread_p = new PollTableThread( FALSE );

        // Wait for the thread to be started by looking for non zero data
        while( pmcPollTableSize[0] == 0 )
        {
            Sleep( 1 );
        }
    }

    // Sanity check... flag should not already be FALSE
    if( RefreshTableInitFlag == FALSE )
    {
        mbDlgDebug((  "ERROR: TMainForm::RefreshTableInit called with RefreshTableInitFlag == FALSE" ));
        goto exit;
    }

    // Prevent this function from being called again.
    // Must set flag before query
    RefreshTableInitFlag = FALSE;

    // Get the new table settings
    RefreshTableQuery( );

    for( i = 0 ; i < PMC_TABLE_COUNT ; i++ )
    {
        switch( tableStatus[i].bit )
        {
            case PMC_TABLE_BIT_PROVIDERS:
                // Set each table's cur time to new time
                tableStatus[i].curModifyTime = tableStatus[i].newModifyTime;
                tableStatus[i].curDataSize   = tableStatus[i].newDataSize;
                break;

            case PMC_TABLE_BIT_APPS:
                tableStatus[i].curModifyTime = tableStatus[i].newModifyTime;
                tableStatus[i].curDataSize   = tableStatus[i].newDataSize;
                break;

            case PMC_TABLE_BIT_PATIENTS:
                break;

            case PMC_TABLE_BIT_DOCTORS:
                break;

            case PMC_TABLE_BIT_CLAIMS:
                break;

            case PMC_TABLE_BIT_CLAIM_HEADERS:
                break;

            case PMC_TABLE_BIT_TIMESLOTS:
               tableStatus[i].curModifyTime = tableStatus[i].newModifyTime;
               tableStatus[i].curDataSize   = tableStatus[i].newDataSize;
               break;

            case PMC_TABLE_BIT_ECHOS:
            case PMC_TABLE_BIT_MED_LIST:
            case PMC_TABLE_BIT_CONFIG:
            case PMC_TABLE_BIT_DOCUMENTS:
                break;

            default:
                mbDlgDebug(( "Unknown table encountered" ));
                break;
        }
    }

    // Get list of available times.  For now, this is only read on startup
#if PMC_AVAIL_TIME
    AvailTimeListInit( );
#endif

exit:
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::ProvidersListBoxDropDown( TObject *Sender )
{
    // User has clicked on providers drop down list.
    // See if the list of providers in the database has changed.
    if( RefreshTable( PMC_TABLE_BIT_PROVIDERS ) )
    {
        // List of providers has changed, update providers list box
        //mbDlgDebug(( "Detected table PMC_TABLE_BIT_PROVIDERS update" ));
        UpdateProviderListBox( Sender );
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DateButtonClick(TObject *Sender)
{
    TDateSelectForm     *dateSelectForm_p;
    pmcDateSelectInfo_t dateSelectInfo;
    Char_t              string[128];

    dateSelectInfo.mode = PMC_DATE_SELECT_TODAY;
    dateSelectInfo.string_p = string;
    dateSelectInfo.caption_p = NIL;

    dateSelectForm_p = new TDateSelectForm
    (
        NULL, &dateSelectInfo
    );
    dateSelectForm_p->ShowModal( );
    delete dateSelectForm_p;

    if( dateSelectInfo.returnCode == MB_BUTTON_OK )
    {
        mbDlgDebug(( "Got date '%s'", dateSelectInfo.string_p ));
    }
    else
    {
        mbDlgDebug(( "user must have pressed cancel button" ));
    }
}

//---------------------------------------------------------------------------
// Function: Timer1Tick
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::Timer1Tick(TObject *Sender)
{
    pmcTimerTicks++;
    if( SkipTimer == TRUE )
    {
        pmcTimerSkips++;
        return;
    }

    // MAB:20020711: Do not update if the form does not have focus
    if( MainForm->Active != TRUE ) return;

    if( DayViewInfo.notReady      ) return;
    if( WeekViewInfo.notReady     ) return;
    if( MonthViewInfo.notReady    ) return;
    if( ProviderViewInfo.notReady ) return;

    // mbLog( "timer1tick running\n" );
    
    PMC_INC_USE( DayViewInfo.notReady   );
    PMC_INC_USE( WeekViewInfo.notReady  );
    PMC_INC_USE( MonthViewInfo.notReady );
    PMC_INC_USE( ProviderViewInfo.notReady );

    if( pmcPollTableModifyTime[ PMC_TABLE_INDEX_APPS ]      > pmcDayViewAppLastReadTime ||
        pmcPollTableModifyTime[ PMC_TABLE_INDEX_TIMESLOTS ] > pmcDayViewSlotLastReadTime )
    {
        DayViewInfo.updateForce = TRUE;
        DayViewGrid->Invalidate( );
    }

    if( pmcPollTableModifyTime[ PMC_TABLE_INDEX_APPS ]      > pmcWeekViewAppLastReadTime ||
        pmcPollTableModifyTime[ PMC_TABLE_INDEX_TIMESLOTS ] > pmcWeekViewSlotLastReadTime )
    {
        WeekViewInfo.updateForce = TRUE;
        WeekViewGrid->Invalidate( );
    }

    if( pmcPollTableModifyTime[ PMC_TABLE_INDEX_APPS ]      > pmcMonthViewAppLastReadTime ||
        pmcPollTableModifyTime[ PMC_TABLE_INDEX_TIMESLOTS ] > pmcMonthViewSlotLastReadTime )
    {
        MonthViewInfo.updateForce = TRUE;
        MonthViewGrid->Invalidate( );
    }

    if( pmcPollTableModifyTime[ PMC_TABLE_INDEX_APPS ]      > pmcProviderViewAppLastReadTime ||
        pmcPollTableModifyTime[ PMC_TABLE_INDEX_TIMESLOTS ] > pmcProviderViewSlotLastReadTime )
    {
        ProviderViewInfo.updateForce = TRUE;
        ProviderViewGrid->Invalidate( );
    }

    PMC_DEC_USE( ProviderViewInfo.notReady  );
    PMC_DEC_USE( MonthViewInfo.notReady     );
    PMC_DEC_USE( WeekViewInfo.notReady      );
    PMC_DEC_USE( DayViewInfo.notReady       );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::DoctorListButtonClick(TObject *Sender)
{
    TDoctorListForm    *docListForm_p;
    pmcDocListInfo_t    docListInfo;

    docListInfo.doctorId = 0;
    docListInfo.mode = PMC_LIST_MODE_LIST;

    docListForm_p = new TDoctorListForm( this, &docListInfo );
    docListForm_p->ShowModal( );
    delete docListForm_p;
    Calendar->SetFocus();
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::AppointmentLetters1Click(TObject *Sender)
{
    TAppLettersForm    *appLettersForm_p;

    appLettersForm_p = new TAppLettersForm( this, SelectedDate, SelectedProviderId );
    appLettersForm_p->ShowModal( );
    delete appLettersForm_p;
    Calendar->SetFocus();
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::SystemStatusClick(TObject *Sender)
{
    Char_t      b[2048];
    Ints_t      l=0;
    Ints_t      i;
    Int32u_t    upTime;

    upTime = (Int32u_t)time( 0 ) - pmcStartTime;
    l=sprintf( b+l, "Up time: %s\n"
                    "Allocated memory chunks: %lu\n"
                    "Allocated Objects: %lu\n"
                    "Timer ticks: %lu (Skips: %lu)\n"
                    "Poll thread ticks: %lu (Skips: %lu)\n" ,
                     mbSecToTimeStr( upTime, &b[1000] ),
                     mbMallocChunks( ), mbObjectCountGet(),
                     pmcTimerTicks, pmcTimerSkips, pmcThreadTicks,
                     pmcThreadSkips );

    for( i = 0 ; i < PMC_TABLE_COUNT ; i++ )
    {
        l += sprintf( b+l, "SQL Table name '%s' size: %ld update time: %Ld\n",
             pmcTableNames_p[i], pmcPollTableSize[i], pmcPollTableModifyTime[i] );
    }
    mbDlgInfo( b );
    Calendar->SetFocus();
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::MainMenuDatabaseCleanFieldsClick(TObject *Sender)
{
    pmcDatabaseCleanFields( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::QuitClick(TObject *Sender)
{
    Close();
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormCloseQuery(TObject *Sender, bool &CanClose)
{
    CanClose = FALSE;

    pmcPatientFormListClean( );
    if( pmcPatientForm_q->size == 1 )
    {
        mbDlgInfo( "There is a patient form open.  "
                   "It must be closed before you can quit %s.", PMC_NAME );
        return;
    }

    if( pmcPatientForm_q->size > 1 )
    {
        mbDlgInfo( "There are %d patient forms open.  "
                   "They must be closed before you can quit %s.", pmcPatientForm_q->size, PMC_NAME );
        return;
    }

    if( pmcAppointCutBuf_q->size > 0 )
    {
        mbDlgInfo( "%s cannot shut down with appoitments in the cut buffer.", PMC_NAME );
        return;
    }

    if( mbDlgYesNo( "Quit %s?", PMC_NAME ) == MB_BUTTON_YES )
    {
        CanClose = TRUE;
        return;
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::TestLabelPrinterClick(TObject *Sender)
{
   /*  pmcTestLabelPrinter( ); */
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::HelpAboutClick(TObject *Sender)
{
    TAboutForm  *aboutForm_p;

    aboutForm_p = new TAboutForm( this );
    aboutForm_p->ShowModal( );
    delete aboutForm_p;
    Calendar->SetFocus();
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::ClaimListButtonClick(TObject *Sender)
{
    pmcClaimListInfo_t      claimListInfo;
    TClaimListForm         *claimListForm_p;

    if( DayViewInfo.notReady        ) return;
    if( WeekViewInfo.notReady       ) return;
    if( MonthViewInfo.notReady      ) return;
    if( ProviderViewInfo.notReady   ) return;

    PMC_INC_USE( DayViewInfo.notReady       );
    PMC_INC_USE( WeekViewInfo.notReady      );
    PMC_INC_USE( MonthViewInfo.notReady     );
    PMC_INC_USE( ProviderViewInfo.notReady  );

    claimListInfo.latestDate = mbToday( );

    MbDateTime dateTime = MbDateTime( claimListInfo.latestDate, 0 );

    // Go back to start of 6 months ago (brute force)
    claimListInfo.earliestDate = dateTime.StartPrevMonth( );
    dateTime.SetDate( claimListInfo.earliestDate );
    claimListInfo.earliestDate = dateTime.StartPrevMonth( );
    dateTime.SetDate( claimListInfo.earliestDate );
    claimListInfo.earliestDate = dateTime.StartPrevMonth( );
    dateTime.SetDate( claimListInfo.earliestDate );
    claimListInfo.earliestDate = dateTime.StartPrevMonth( );
    dateTime.SetDate( claimListInfo.earliestDate );
    claimListInfo.earliestDate = dateTime.StartPrevMonth( );
    dateTime.SetDate( claimListInfo.earliestDate );
    claimListInfo.earliestDate = dateTime.StartPrevMonth( );

    mbLog( "Claims list clicked, earliestDate: %ld latestDate: %ld\n",
        claimListInfo.earliestDate, claimListInfo.latestDate );

    claimListInfo.providerId = SelectedProviderId;
    claimListInfo.providerAllFlag = FALSE;  // By default, display only claims for selected provider
    claimListInfo.patientId = 0;
    claimListInfo.patientAllFlag = TRUE;    // By default, display claims for all patients

    claimListForm_p = new TClaimListForm( NULL, &claimListInfo );
    claimListForm_p->ShowModal();
    delete claimListForm_p;

    PMC_DEC_USE( ProviderViewInfo.notReady  );
    PMC_DEC_USE( DayViewInfo.notReady       );
    PMC_DEC_USE( WeekViewInfo.notReady      );
    PMC_DEC_USE( MonthViewInfo.notReady     );

    Calendar->SetFocus();
    return;
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::MainMenuImportSmaDrListClick(TObject *Sender)
{
    pmcSmaDoctorListImport( );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------


void __fastcall TMainForm::MainMenuCheckPatientRecordsClick(TObject *Sender)
{
    pmcCheckPatientRecords( );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::PrintArchivedMSPClaimsFileClick
(
    TObject *Sender
)
{
    OpenDialog->Title = "Select archived CLAIMSIN file...";
    OpenDialog->InitialDir = ( pmcCfg[CFG_MSP_ARCHIVE_DIR].str_p ) ? pmcCfg[CFG_MSP_ARCHIVE_DIR].str_p : "" ;
    OpenDialog->Filter = "CLAIMSIN Files (*.CLAIMSIN)|*.CLAIMSIN|All Files (*.*)|*.*";
    OpenDialog->FileName = "";

    if( OpenDialog->Execute() == TRUE )
    {
        if( pmcClaimsFilePrint( OpenDialog->FileName.c_str(), NIL, PMC_CLAIM_FILE_PRINT_MODE_ARCHIVE ) == TRUE )
        {
            mbDlgInfo( "Contents of file %s sent to printer.\n", OpenDialog->FileName.c_str() );
        }
        else
        {
            mbDlgInfo( "The file %s could not be printed.\n", OpenDialog->FileName.c_str() );
        }
    }
    Calendar->SetFocus();
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::MainMenuCheckAppointmentRecordsClick
(
      TObject *Sender
)
{
    pmcCheckAppointmentRecords( );
    Calendar->SetFocus();
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::ReportButtonClick(TObject *Sender)
{
    TReportForm         *reportForm_p;

    reportForm_p = new TReportForm( this );
    reportForm_p->ShowModal();
    delete reportForm_p;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::LegendDrawGridClick(TObject *Sender)
{
     ColorDialog1->Execute( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::LegendDrawGridDrawCell(TObject *Sender, int ACol,
      int ARow, TRect &Rect, TGridDrawState State)
{
    Int32s_t    color;
    AnsiString  str = "";
    static      int init = TRUE;

    color = pmcColor[ARow];
    str = pmcColorLegendStrings_p[ARow];

    LegendDrawGrid->Canvas->Brush->Color = (TColor)color;
    LegendDrawGrid->Canvas->FillRect( Rect );
    LegendDrawGrid->Canvas->TextRect( Rect, Rect.Left + 2, Rect.Top, str );

    if( init )
    {
        // It seems that somehow, by using a different thread, I can get the
        // form to draw before the patient and doctor lists are read.
        pmcCloseSplash( );
        init = FALSE;
        Timer2->Enabled = TRUE;
        UpdateProviderListBox( NIL );
        UpdateProviderLabel( );
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::Timer2Timer(TObject *Sender)
{
    static int init = TRUE;
    if( init )
    {
        init = FALSE;
        Timer2->Enabled = FALSE;
        UpdatePatientListDoInit = TRUE;
        UpdateDoctorListDoInit = TRUE;
        RefreshTable( PMC_TABLE_BIT_PATIENTS | PMC_TABLE_BIT_DOCTORS );
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::MainMenuImportMspDrListExcelClick
(
    TObject *Sender
)
{
    pmcImportMspDrListExcel( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::MainMenuCheckDoctorRecordsClick(TObject *Sender)
{
    mbDlgInfo( "Function not yet implemented" );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::AppointmentTabsChange(TObject *Sender)
{
    UpdateProviderLabel( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::PrintArchivedValidrptFileClick(TObject *Sender)
{
    OpenDialog->Title = "Select archived VALIDRPT file...";
    OpenDialog->InitialDir = ( pmcCfg[CFG_MSP_ARCHIVE_DIR].str_p ) ? pmcCfg[CFG_MSP_ARCHIVE_DIR].str_p : "" ;
    OpenDialog->Filter = "VALIDRPT Files (*.VALIDRPT)|*.VALIDRPT|All Files (*.*)|*.*";
    OpenDialog->FileName = "";

    if( OpenDialog->Execute() == TRUE )
    {
        if( pmcMspValidrptFileProcess( OpenDialog->FileName.c_str( ) ) == TRUE )
        {
            mbDlgInfo( "Contents of the file %s sent to printer.\n", OpenDialog->FileName.c_str() );
        }
        else
        {
            mbDlgInfo( "Errors were encountered printing file %s.\n", OpenDialog->FileName.c_str() );
        }
    }
    Calendar->SetFocus();
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::CheckClaimRecordsClick(TObject *Sender)
{
    pmcDatabaseCheckClaimRecords( );
    Calendar->SetFocus();
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::PrintArchivedReturnsFileClick(TObject *Sender)
{
    OpenDialog->Title = "Select archived RETURNS file...";
    OpenDialog->InitialDir = ( pmcCfg[CFG_MSP_ARCHIVE_DIR].str_p ) ? pmcCfg[CFG_MSP_ARCHIVE_DIR].str_p : "" ;
    OpenDialog->Filter = "RETURNS Files (*.RETURNS)|*.RETURNS|All Files (*.*)|*.*";
    OpenDialog->FileName = "";

    if( OpenDialog->Execute() == TRUE )
    {
        if( pmcMspReturnsFilePrint( OpenDialog->FileName.c_str( ) ) == TRUE )
        {
            //mbDlgInfo( "Printing of file %s complete\n", OpenDialog->FileName.c_str() );
        }
        else
        {
            mbDlgInfo( "Errors were encountered printing file %s.\n", OpenDialog->FileName.c_str() );
        }
    }
    Calendar->SetFocus();
}

//---------------------------------------------------------------------------
// Function: CheckArchivedRETURNSFileContents1Click
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::CheckArchivedRETURNSFileContents1Click(
      TObject *Sender)
{
    OpenDialog->Title = "Select archived RETURNS file...";
    OpenDialog->InitialDir = ( pmcCfg[CFG_MSP_ARCHIVE_DIR].str_p ) ? pmcCfg[CFG_MSP_ARCHIVE_DIR].str_p : "" ;
    OpenDialog->Filter = "RETURNS Files (*.RETURNS)|*.RETURNS|All Files (*.*)|*.*";
    OpenDialog->FileName = "";

    if( OpenDialog->Execute() == TRUE )
    {
        pmcMspReturnsFileContentsDialog( OpenDialog->FileName.c_str( ) );
    }
    Calendar->SetFocus();
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TMainForm::PrintArchivedNoticeFileClick(TObject *Sender)
{
    OpenDialog->Title = "Select archived NOTICE file...";
    OpenDialog->InitialDir = ( pmcCfg[CFG_MSP_ARCHIVE_DIR].str_p ) ? pmcCfg[CFG_MSP_ARCHIVE_DIR].str_p : "" ;
    OpenDialog->Filter = "NOTICE Files (*.NOTICE)|*.NOTICE|All Files (*.*)|*.*";
    OpenDialog->FileName = "";

    if( OpenDialog->Execute() == TRUE )
    {
        if( pmcMspNoticeFilePrint( OpenDialog->FileName.c_str( ) ) == TRUE )
        {
            mbDlgInfo( "Printing of file %s complete\n", OpenDialog->FileName.c_str() );
        }
        else
        {
            mbDlgInfo( "Errors were encountered printing file %s.\n", OpenDialog->FileName.c_str() );
        }
    }
    Calendar->SetFocus();
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::ImportDocumentsBatchClick(TObject *Sender)
{
    TBatchImportForm           *batchForm_p;
    pmcBatchImportInfoo_t        batchInfo;

    if( pmcCfg[CFG_DATABASE_LOCAL].value == FALSE )
    {
        mbDlgInfo( "Cannot import documents when connected to a remote database.\n" );
        goto exit;
    }
    batchInfo.providerId = SelectedProviderId;
    batchInfo.mode = PMC_IMPORT_MODE_DOCS;
    batchForm_p = new TBatchImportForm( this, &batchInfo );
    batchForm_p->ShowModal( );
    delete batchForm_p;

exit:
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::ImportDocumentsSingleClick(TObject *Sender)
{
    pmcDocImportSingle( 0 );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::TestCrcButtonClick(TObject *Sender)
{
    Int32u_t    crc;
    Int32u_t    size;

    OpenDialog->Title = "Select File..";
    OpenDialog->InitialDir = ( pmcCfg[CFG_DOC_IMPORT_FROM_DIR].str_p ) ? pmcCfg[CFG_DOC_IMPORT_FROM_DIR].str_p : "" ;
    OpenDialog->Filter = "All Files (*.*)|*.*";
    OpenDialog->FileName = "";

    if( OpenDialog->Execute() == TRUE )
    {
        crc = mbCrcFile( OpenDialog->FileName.c_str(), &size, NIL, NIL, NIL, NIL, NIL  );
        mbDlgDebug(( "CRC: %lu size: %ld\n", crc, size ));
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::DocumentListButtonClick(TObject *Sender)
{
    TDocumentListForm          *form_p;
    pmcDocumentListInfo_t       formInfo;

    formInfo.patientId = 0;
    form_p = new TDocumentListForm( this, &formInfo );
    form_p->ShowModal( );
    delete form_p;

    Calendar->SetFocus();
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::CreateWordClick(TObject *Sender)
{
    pmcWordCreate( 0, SelectedProviderId );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#define PMC_PROVIDER_COUNT_PATIENTS     0
#define PMC_PROVIDER_COUNT_APPS         1
#define PMC_PROVIDER_COUNT_DOCUMENTS    2

void __fastcall TMainForm::PatientCountUpdate( Int32u_t mode )
{
    Char_p      buf_p;
    Int32u_t    count;

    mbMalloc( buf_p, 256 );

    count = pmcProviderCounterGet( SelectedProviderId, PMC_PROVIDER_COUNT_PATIENTS, mode );
    mbStrInt32u( count, buf_p );
    PatientCountLbl->Caption = buf_p;
    PatientCountLbl->Invalidate( );
    mbFree( buf_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::AppointmentCountUpdate( Int32u_t mode )
{
    Char_p      buf_p;
    Int32u_t    count;

    mbMalloc( buf_p, 256 );
    count = pmcProviderCounterGet( SelectedProviderId, PMC_PROVIDER_COUNT_APPS, mode );
    mbStrInt32u( count, buf_p );
    AppointmentsCountLabel->Caption = buf_p;
    AppointmentsCountLabel->Invalidate( );
    mbFree( buf_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::DocumentCountUpdate( Int32u_t mode )
{
    Char_p      buf_p;
    Int32u_t    count;

    mbMalloc( buf_p, 256 );
    count = pmcProviderCounterGet( SelectedProviderId, PMC_PROVIDER_COUNT_DOCUMENTS, mode );
    mbStrInt32u( count, buf_p );
    DocumentsCountLabel->Caption = buf_p;
    DocumentsCountLabel->Invalidate( );
    mbFree( buf_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
void __fastcall TMainForm::TestSelectButtonClick(TObject *Sender)
{
    TSelectForm            *form_p;
    pmcSelectFormInfo_t     info;

    form_p = new TSelectForm( this, &info );
    form_p->ShowModal( );
    delete form_p;
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::ExtractDocumentsClick(TObject *Sender)
{
    if( pmcCfg[CFG_TEMP_EXTRACT_DOCUMENTS].value == TRUE )
    {
        // pmcExtractDocuments( );
        mbDlgInfo( "Feature compiled out.\n" );
    }
    else
    {
        mbDlgInfo( "Feature not enabled.\n" );
    }
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------


void __fastcall TMainForm::ScramblePatientNamesClick(TObject *Sender)
{
    if( pmcCfg[CFG_SCRAMBLE_NAMES_FLAG].value == TRUE )
    {
#if PMC_SCRAMBLE_PATIENT_NAMES
        pmcScrambleNames( );
#else
        mbDlgInfo( "Feature compiled out." );
#endif
    }
    else
    {
        mbDlgInfo( "Feature not enabled.\n" );
    }
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::CheckDocumentRecordsClick(TObject *Sender)
{
    pmcDatabaseCheckDocuments( );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::CheckDocumentDatabaseClick(TObject *Sender)
{
    pmcDocInDatabase( );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::AppointmentsClick(TObject *Sender)
{
    Int32u_t    providerId = 0;
    Int32u_t    date = 0;

    pmcViewAppointments( 0, TRUE, FALSE, TRUE, &providerId, &date, TRUE, PMC_LIST_MODE_LIST );
    AppointmentGoto( providerId, date );
    Calendar->SetFocus();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::AppointmentGoto
(
    Int32u_t    providerId,
    Int32u_t    date
)
{
    Int32u_t    providerIndex;

    if( providerId != 0 && date != 0 )
    {
        providerIndex = pmcProviderIndexGet( ProvidersListBox, providerId );
        ProvidersListBox->ItemIndex = providerIndex;

        //SkipUpdateProviderLabelInvalidate = TRUE;
        UpdateProviderLabel( );
        //SkipUpdateProviderLabelInvalidate = FALSE;

        UpdateDateInt( date );
        return;
    }
}
void __fastcall TMainForm::TestPatientFormModalClick(TObject *Sender)
{
    TPatientForm            *form_p;
    pmcPatientFormInfo_t     info;

    form_p = new TPatientForm( this, &info );
    form_p->ShowModal( );
    delete form_p;
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::TestPatientFormNonModalClick(TObject *Sender)
{
    pmcPatientFormNonModal( 0 );
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::LockTestButtonClick(TObject *Sender)
{
    mbLockAcquire( pmcTestLock );
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ImportMSPFilesClick(TObject *Sender)
{
    pmcImportMspFiles( );
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::DrFaxPhoneListClick(TObject *Sender)
{
    Char_p              buf_p;
    docMaintStruct_p    doc_p;
    qHead_t             docQueueHead;
    qHead_p             doc_q;
    Char_t              phoneNumber[16];
    Char_t              areaCode[8];
    Boolean_t           local;
    FILE               *fp;
    MbSQL               sql;

    doc_q = qInitialize( &docQueueHead );
    mbMalloc( buf_p, 1024 );

    SaveDialog->Title = "Save Dr. Fax/Phone List File";
    SaveDialog->InitialDir = ( pmcCfg[CFG_PHONE_LIST_DIR].str_p ) ? pmcCfg[CFG_PHONE_LIST_DIR].str_p : "" ;
    SaveDialog->Filter = "Text Files|*.txt";
    SaveDialog->FileName = "Dr fax.txt";

    if( SaveDialog->Execute() != TRUE )
    {
        goto exit;
    }

    sprintf( buf_p, "select %s,%s,%s,%s from %s where %s!=\"\" and %s=%ld",
        PMC_SQL_FIELD_FIRST_NAME,
        PMC_SQL_FIELD_LAST_NAME,
        PMC_SQL_DOCTORS_FIELD_WORK_PHONE,
        PMC_SQL_DOCTORS_FIELD_WORK_FAX,
        PMC_SQL_TABLE_DOCTORS,
        PMC_SQL_DOCTORS_FIELD_WORK_FAX,
        PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

    /* Query the database */
    sql.Query( buf_p );

    while( sql.RowGet( ) )
    {
        mbCalloc( doc_p, sizeof( docMaintStruct_t ) );

        // First Name
        mbMallocStr( doc_p->firstName_p, sql.String( 0 ) );

        // Last Name
        mbMallocStr( doc_p->lastName_p, sql.String( 1 ) );

        // Phone
        mbMallocStr( doc_p->phone_p, sql.String( 2 ) );

        // Fax
        mbMallocStr( doc_p->fax_p, sql.String( 3 ) );

        qInsertLast( doc_q, doc_p );
    }

    // Attempt to open the file
    if( ( fp = fopen( SaveDialog->FileName.c_str(), "w" ) ) == NIL )
    {
        mbDlgExclaim( "Error opening file %s\n", SaveDialog->FileName.c_str() );
        goto exit;
    }

    fprintf( fp, "\"LAST NAME\",\"FIRST NAME\",\"FAX\",\"PHONE\"\n" );

    qWalk( doc_p, doc_q, docMaintStruct_p )
    {
        fprintf( fp, "\"%s\",\"%s\",", doc_p->lastName_p, doc_p->firstName_p );

        pmcPhoneFormat( doc_p->fax_p, &areaCode[0], &phoneNumber[0], &local );
        if( local )
        {
            fprintf( fp, "\"%s\",", phoneNumber );
        }
        else
        {
            fprintf( fp, "\"(%s) %s\",", areaCode, phoneNumber );
        }
        pmcPhoneFormat( doc_p->phone_p, &areaCode[0], &phoneNumber[0], &local );
        if( local )
        {
            fprintf( fp, "\"%s\",", phoneNumber );
        }
        else
        {
            fprintf( fp, "\"(%s) %s\"", areaCode, phoneNumber );
        }
        fprintf( fp, "\n" );
    }

exit:

    if( fp ) fclose( fp );

    while( !qEmpty( doc_q ) )
    {
        doc_p = (docMaintStruct_p)qRemoveFirst( doc_q );

        if( doc_p->firstName_p ) mbFree( doc_p->firstName_p );
        if( doc_p->lastName_p )  mbFree( doc_p->lastName_p );
        if( doc_p->phone_p )     mbFree( doc_p->phone_p );
        if( doc_p->fax_p )       mbFree( doc_p->fax_p );

        mbFree( doc_p );
    }

    mbFree( buf_p );
    return;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::CreateAppHistoryClick(TObject *Sender)
{
#if PMC_APP_HISTORY_CREATE
    pmcAppHistoryCreate( );
#else
    mbDlgInfo( "Feature compiled out" );
#endif
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ImportEchosClick(TObject *Sender)
{
    pmcEchoImport( &Cursor );
    Calendar->SetFocus();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::CheckDoctorRecordsClick(TObject *Sender)
{
    mbDlgExclaim( "Check of doctor records not yet implemented." );
    Calendar->SetFocus();
    return;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::CDEjectClick(TObject *Sender)
{
    TCursor origCursor;

    origCursor = Screen->Cursor;
    Screen->Cursor = crHourGlass;
    mbDriveOpen( TRUE, pmcCfg[CFG_CD_BURNER].str_p );
    Screen->Cursor = origCursor;
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::CDLoadClick(TObject *Sender)
{
    TCursor origCursor;

    origCursor = Screen->Cursor;
    Screen->Cursor = crHourGlass;
    mbDriveOpen( FALSE, pmcCfg[CFG_CD_BURNER].str_p );
    Screen->Cursor = origCursor;
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::DisplayEchoCDContentsClick(TObject *Sender)
{
    pmcEchoCDContentsForm(  );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::EchosListButtonClick(TObject *Sender)
{
    pmcEchoListForm( 0 );
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MallocTest1Click(TObject *Sender)
{
    Void_p  test_p;

    if( mbDlgYesNo( "This test allocates a chunk of memory that is never freed.\n"
                    "This memory chunk should be detected on shutdown.\n\nContinue?" ) == MB_BUTTON_NO ) goto exit;

    if( mbMalloc( test_p, 1234 ) == NIL )
    {
        mbDlgInfo( "mbMalloc() failed" );
    }
    else
    {
        mbDlgInfo( "Allocated 1234 bytes" );
    }

exit:
    return;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::CheckPatientsUnreferencedClick(TObject *Sender)
{
    pmcCheckPatientsUnreferenced(  );
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::AssignPatientstoEchosClick(TObject *Sender)
{
    pmcDatabaseEchosAssignPatients( );
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::DatabaseCheckTimeslotsClick(TObject *Sender)
{
    pmcDatabaseCheckTimeslots( );
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ICDListClick(TObject *Sender)
{
    pmcIcdListInfo_t       info;
    TIcdForm              *form_p;

    info.codeIn[0] = 0;
    info.mode = PMC_LIST_MODE_LIST;
    form_p = new TIcdForm( this, &info );
    form_p->ShowModal( );
    delete form_p;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::MedListClick(TObject *Sender)
{
    TMedListForm              *form_p;

    form_p = new TMedListForm( this );
    form_p->ShowModal( );
    delete form_p;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormularyClick(TObject *Sender)
{
    OpenDialog->Title = "Select Formulary File..";
    OpenDialog->InitialDir = ( pmcCfg[CFG_DOC_IMPORT_FROM_DIR].str_p ) ? pmcCfg[CFG_DOC_IMPORT_FROM_DIR].str_p : "" ;
    OpenDialog->Filter = "All Files (*.*)|*.*";
    OpenDialog->FileName = "";

    if( OpenDialog->Execute() == TRUE )
    {
        pmcInitFormularyFileRead( OpenDialog->FileName.c_str() );
    }
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::TestClick(TObject *Sender)
{
    mbDlgInfo( "TestClick called" );
}


void __fastcall TMainForm::ImportScannedDocumentsClick(TObject *Sender)
{
    // NOTE: this function should be combined with batch import function
    TBatchImportForm           *batchForm_p;
    pmcBatchImportInfoo_t        batchInfo;

    if( pmcCfg[CFG_DATABASE_LOCAL].value == FALSE )
    {
        mbDlgInfo( "Cannot import documents when connected to a remote database.\n" );
        goto exit;
    }
    batchInfo.providerId = SelectedProviderId;
    batchInfo.mode = PMC_IMPORT_MODE_SCANS;
    batchForm_p = new TBatchImportForm( this, &batchInfo );
    batchForm_p->ShowModal( );
    delete batchForm_p;

exit:
    Calendar->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FileWriteTestClick(TObject *Sender)
{
    TCursor                 origCursor;
    Void_p                  handle_p = NIL;
    Char_p                  newDir_p;
    Char_p                  file_p;
    FILE                   *fp = NIL;
    Int8u_t                 buf[1024 * 64];
    Int32s_t                i, j, k;
    TThermometer           *thermometer_p = NIL;
    Int32u_t                startTime, endTime;
    Int32u_t                bytes_written;
    Float64_t               bytesPerSecond;
    Float64_t               seconds;

    if( mbDlgYesNo( "This utility will test throughput writing a large file. Continue?" ) != MB_BUTTON_YES ) return;

    mbMalloc( newDir_p, MAX_PATH );
    mbMalloc( file_p, MAX_PATH );

    origCursor = Screen->Cursor;
    Screen->Cursor = crHourGlass;

    handle_p = mbPathHandleGet( "f:\\" );

    if( mbDlgBrowseFolderNew( newDir_p, "Choose directory for write test...", &handle_p ) != MB_BUTTON_OK ) goto exit;

    pmcMakeFileName( newDir_p, file_p );

    strcat( file_p, "_WRITE_TEST" );

    if( mbDlgOkCancel( "The file '%s' will be created. Click OK to proceed.", file_p ) != MB_BUTTON_OK ) goto exit;

    startTime = mbMsec( );
    if( ( fp = fopen( file_p, "wb" ) ) == NIL ) goto exit;

    sprintf( newDir_p, "Writing %s..", file_p );
    thermometer_p = new TThermometer( newDir_p, 0, 100, TRUE );

    bytes_written = 0;
    for( k = 0 ; k < 100 ; k++ )
    {
        for( j = 0 ; j < 32 ; j++ )
        {
            for( i = 0 ; i < 1024 * 32 ; i++ )
            {
                buf[i] = (Int8u_t)( rand( ) & 0x000000FF );
            }
            bytes_written += fwrite( buf, 1, 1024 * 32, fp );
        }
        if( thermometer_p->Increment( ) == FALSE )
        {
            break;
        }
    }

    fclose( fp );
    fp = NIL;

    endTime = mbMsec();

    seconds = (Float64_t)(endTime - startTime)/1000.0;

    if( thermometer_p ) delete thermometer_p;

    bytesPerSecond = (Float64_t)bytes_written / (Float64_t)( seconds );

    Screen->Cursor = origCursor;

    mbDlgInfo( "Wrote %u byles in %.2f seconds for a speed of %.2f bytes/second.",
        bytes_written, seconds, bytesPerSecond );

exit:


    mbPathHandleFree( handle_p );

    if( fp ) fclose( fp );

    mbFree( newDir_p );
    mbFree( file_p );
    return;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FileReadTestClick(TObject *Sender)
{
    OpenDialog->Title = "Select Read Test File...";
    OpenDialog->InitialDir = "" ;
    OpenDialog->Filter = "All Files (*.*)|*.*";
    OpenDialog->FileName = "";
    Int8u_t     buf[1024 * 64];
    FILE        *fp = NIL;
    Int32u_t                startTime, endTime;
    Int32u_t                bytesRead, totalBytesRead;
    Float64_t               bytesPerSecond;
    Float64_t               seconds;
    TCursor                 origCursor;

    if( OpenDialog->Execute() != TRUE ) return;

    startTime = mbMsec( );

    if( ( fp = fopen( OpenDialog->FileName.c_str(), "rb" ) ) == NIL )
    {
        mbDlgInfo( "Failed to open file '%s'", OpenDialog->FileName.c_str() );
        return;
    }

    origCursor = Screen->Cursor;
    Screen->Cursor = crHourGlass;

    totalBytesRead = 0;
    for( ; ; )
    {
        bytesRead = (Int32u_t)fread( (void *)&buf[0], 1, 1024 * 32, fp );

        if( bytesRead == 0 ) break;

        totalBytesRead += bytesRead;
    }

    fclose( fp );

    endTime = mbMsec( );

    seconds = (Float64_t)(endTime - startTime)/1000.0;

    bytesPerSecond = (Float64_t)totalBytesRead / (Float64_t)( seconds );

    Screen->Cursor = origCursor;

    mbDlgInfo( "Read %u byles in %.2f seconds for a speed of %.2f bytes/second.",
        totalBytesRead, seconds, bytesPerSecond );

    return;
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::Write50FilesClick(TObject *Sender)
{
    TCursor                 origCursor;
    Void_p                  handle_p = NIL;
    Char_p                  newDir_p;
    Char_p                  file_p;
    Char_p                  file2_p;
    FILE                   *fp = NIL;
    Int8u_t                 buf[1024 * 64];
    Int32s_t                i, j, k;
    TThermometer           *thermometer_p = NIL;
    Int32u_t                startTime, endTime;
    Int32u_t                bytes_written;
    Float64_t               bytesPerSecond;
    Float64_t               seconds;
    Int32u_t                filecount;

    if( mbDlgYesNo( "This utility will test throughput writing 100 files. Continue?" ) != MB_BUTTON_YES ) return;

    mbMalloc( newDir_p, MAX_PATH );
    mbMalloc( file_p, MAX_PATH );
    mbMalloc( file2_p, MAX_PATH );

    origCursor = Screen->Cursor;
    Screen->Cursor = crHourGlass;

    handle_p = mbPathHandleGet( "f:\\" );

    if( mbDlgBrowseFolderNew( newDir_p, "Choose directory for write test...", &handle_p ) != MB_BUTTON_OK ) goto exit;

    sprintf( file_p, "%s\\WRITE_TEST_%08lX", newDir_p, mbMsec() );

    if( mbDlgOkCancel( "The files '%s_XX' will be created. Click OK to proceed.", file_p ) != MB_BUTTON_OK ) goto exit;

    sprintf( newDir_p, "Writing %s..", file_p );
    thermometer_p = new TThermometer( newDir_p, 0, 100, TRUE );

    startTime = mbMsec( );

    bytes_written = 0;

    for( filecount = 0 ; filecount < 100 ; filecount++ )
    {
        sprintf( file2_p, "%s_%02d", file_p, filecount );

        if( ( fp = fopen( file2_p, "wb" ) ) == NIL ) goto exit;

        for( k = 0 ; k < 1 ; k++ )
        {
            for( j = 0 ; j < 1024 ; j++ )
            {
                for( i = 0 ; i < 1024 ; i++ )
                {
                    buf[i] = (Int8u_t)( rand( ) & 0x000000FF );
                }
                bytes_written += fwrite( buf, 1, 1024, fp );
            }
        }

        fclose( fp );

        if( thermometer_p->Increment( ) == FALSE ) break;
    }
    fp = NIL;

    endTime = mbMsec();

    seconds = (Float64_t)(endTime - startTime)/1000.0;

    if( thermometer_p ) delete thermometer_p;

    bytesPerSecond = (Float64_t)bytes_written / (Float64_t)( seconds );

    Screen->Cursor = origCursor;

    mbDlgInfo( "Wrote %u byles in %.2f seconds for a speed of %.2f bytes/second.",
        bytes_written, seconds, bytesPerSecond );

exit:

    mbPathHandleFree( handle_p );

    if( fp ) fclose( fp );

    mbFree( newDir_p );
    mbFree( file_p );
    mbFree( file2_p );
    return;
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewTabShow(TObject *Sender)
{
    ButtonDaySheet->Visible = TRUE;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::WeekViewTabShow(TObject *Sender)
{
    ButtonDaySheet->Visible = FALSE;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::MonthViewTabShow(TObject *Sender)
{
    ButtonDaySheet->Visible = FALSE;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ProviderViewTabShow(TObject *Sender)
{
    ButtonDaySheet->Visible = FALSE;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ButtonDaySheetClick(TObject *Sender)
{
    this->DayViewPopupDaySheetClick( NIL );    
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::CheckActiveDocumentsClick(TObject *Sender)
{
    pmcDatabaseCheckActiveDocuments( );
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::IncrementClaimNumbersClick(TObject *Sender)
{
    pmcDatabaseIncrementClaimNumbers( );
}
//---------------------------------------------------------------------------

