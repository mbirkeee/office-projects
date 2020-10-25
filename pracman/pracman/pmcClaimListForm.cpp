//---------------------------------------------------------------------------
// File:    pmcClaimListForm.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 26, 2001
//---------------------------------------------------------------------------
// Description:
//
// Functions for managing the claim list form
//---------------------------------------------------------------------------

#include <stdio.h>
#include <vcl.h>
#include <io.h>
#pragma hdrstop

#include "mbUtils.h"

#include "pmcTables.h"
#include "pmcGlobals.h"
#include "pmcUtils.h"
#include "pmcColors.h"
#include "pmcMsp.h"

#include "pmcMainForm.h"
#include "pmcClaimListForm.h"
#include "pmcDateSelectForm.h"
#include "pmcPatientListForm.h"
#include "pmcDoctorListForm.h"
#include "pmcClaimEditForm.h"
#include "pmcMspClaimsin.h"
#include "pmcMspValidrpt.h"
#include "pmcMspNotice.h"
#include "pmcMspReturns.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

__fastcall TClaimListForm::TClaimListForm(TComponent* Owner)
    : TForm(Owner)
{
    mbDlgDebug(( "Default constructor called" ));
    Close( );
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------


__fastcall TClaimListForm::TClaimListForm
(
    TComponent         *Owner,
    pmcClaimListInfo_p  claimListInfo_p
)
: TForm(Owner)
{
    MbDateTime      dateTime;
    Char_p          buf_p;

    mbMalloc( buf_p, 128 );
    Active = FALSE;

    OkButton->Visible = FALSE;
    CancelButton->Caption = "Close";

    SubmitDate = mbToday( );
    ReplyDate = mbToday( );

    StatusLabel[0] = StatusLabel0;
    StatusLabel[1] = StatusLabel1;
    StatusLabel[2] = StatusLabel2;
    StatusLabel[3] = StatusLabel3;
    StatusLabel[4] = StatusLabel4;
    StatusLabel[5] = StatusLabel5;
    StatusLabel[6] = StatusLabel6;
    StatusLabel[7] = StatusLabel7;
    StatusLabel[8] = StatusLabel8;
    StatusLabel[9] = StatusLabel9;
    StatusLabel[10] = StatusLabel10;
    StatusLabel[11] = StatusLabel11;
    StatusLabel[12] = StatusLabel12;
    StatusLabel[13] = StatusLabel13;
    StatusLabel[14] = StatusLabel14;
    StatusLabel[15] = StatusLabel15;
    StatusLabel[16] = StatusLabel16;
    StatusLabel[17] = StatusLabel17;

    MouseDownExp = FALSE;

    SelectedClaimId = 0;
    SearchClaimNumber = 0;

    // Prevent database reads until we're ready
    SkipClaimListGet = TRUE;

    mbLockInit( ClaimQueueLock );

    ClaimListInfo_p = claimListInfo_p;


    // Force claim list form to start from today
    LatestDate = mbToday( );

    if( claimListInfo_p->earliestDate == 0 )
    {
        EarliestDate = LatestDate;

        // Set earliest date back 6 months.
        dateTime.SetDate( EarliestDate );
        EarliestDate = dateTime.StartPrevMonth( );
        dateTime.SetDate( EarliestDate );
        EarliestDate = dateTime.StartPrevMonth( );
        dateTime.SetDate( EarliestDate );
        EarliestDate = dateTime.StartPrevMonth( );
        dateTime.SetDate( EarliestDate );
        EarliestDate = dateTime.StartPrevMonth( );
        dateTime.SetDate( EarliestDate );
        EarliestDate = dateTime.StartPrevMonth( );
        dateTime.SetDate( EarliestDate );
        EarliestDate = dateTime.StartPrevMonth( );
    }
    else
    {
        EarliestDate = claimListInfo_p->earliestDate;
    }

    // Initiaize the strings in the edit boxes
    dateTime.SetDate( EarliestDate );
    EarliestDateEdit->Text = dateTime.MDY_DateString( );

    dateTime.SetDate( LatestDate );
    LatestDateEdit->Text = dateTime.MDY_DateString( );

    dateTime.SetDate( ReplyDate );
    ReplyDateEdit->Text = dateTime.MDY_DateString( );

    dateTime.SetDate( SubmitDate );
    SubmitDateEdit->Text = dateTime.MDY_DateString( );

    // Initialize the provider information
    ProviderId = pmcProviderListBuild( ProviderComboBox, claimListInfo_p->providerId, TRUE, TRUE );
    ProviderIndex = ProviderComboBox->ItemIndex;
    ProviderComboBoxSkipUpdate = FALSE;

    PatientId = claimListInfo_p->patientId;
    PatientAllFlag = claimListInfo_p->patientAllFlag;
    PatientRadioGroup->ItemIndex = ( PatientAllFlag == TRUE ) ? 0 : 1;
    PatientEditUpdate( PatientId );

    ProviderAllFlag = claimListInfo_p->providerAllFlag;
    ProviderRadioGroup->ItemIndex = ( ProviderAllFlag == TRUE ) ? 0 : 1;

    // Initialize linked list of claim records
    Claim_q = qInitialize( &ClaimQueueHead );

    UpdateTotals( );

    if(  PatientRadioGroup->ItemIndex == 1 )
    {
        PaidCheckBox->Checked               = TRUE;
        RejectedCheckBox->Checked           = TRUE;
        SubmittedCheckBox->Checked          = TRUE;
        ReadyCheckBox->Checked              = TRUE;
        NotReadyCheckBox->Checked           = TRUE;
        ReducedCheckBox->Checked            = TRUE;
        ReducedAcceptCheckBox->Checked      = TRUE;
        RejectedAcceptCheckBox->Checked     = TRUE;
    }
    else
    {
        // Read state of check boxes from global variables
        PaidCheckBox->Checked               = pmcClaimListPaidCheck;
        RejectedCheckBox->Checked           = pmcClaimListRejectedCheck;
        SubmittedCheckBox->Checked          = pmcClaimListSubmittedCheck;
        ReadyCheckBox->Checked              = pmcClaimListReadyCheck;
        NotReadyCheckBox->Checked           = pmcClaimListNotReadyCheck;
        ReducedCheckBox->Checked            = pmcClaimListReducedCheck;
        ReducedAcceptCheckBox->Checked      = pmcClaimListReducedAcceptCheck;
        RejectedAcceptCheckBox->Checked     = pmcClaimListRejectedAcceptCheck;
    }

    PaidCheckBoxStore               = PaidCheckBox->Checked;
    RejectedCheckBoxStore           = RejectedCheckBox->Checked;
    SubmittedCheckBoxStore          = SubmittedCheckBox->Checked;
    ReadyCheckBoxStore              = ReadyCheckBox->Checked;
    NotReadyCheckBoxStore           = NotReadyCheckBox->Checked;
    ReducedCheckBoxStore            = ReducedCheckBox->Checked;
    RejectedAcceptCheckBoxStore     = RejectedAcceptCheckBox->Checked;
    ReducedAcceptCheckBoxStore      = ReducedAcceptCheckBox->Checked;

    // Read database for the first time
    SkipClaimListGet = FALSE;
    ClaimArray_pp = NIL;
    ClaimArraySize = 0;
    ClaimDrawGrid->RowCount = 2;
    ClaimDrawGrid->TopRow = 1;
    ClaimDrawGrid->Row = 1;

    // 2017_11_18:  Change the default sort order to ID from claim number.
    // This was done becase claim numbers are now wrapping
    // SortCode = PMC_SORT_NUMBER_ASCENDING;
    SortCode = PMC_SORT_ID_ASCENDING;
    ClaimListGet( NIL );

    Active = TRUE;
    mbFree( buf_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::OkButtonClick(TObject *Sender)
{
    ClaimListInfo_p->returnCode = MB_BUTTON_OK;
    Close( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::CancelButtonClick(TObject *Sender)
{
    ClaimListInfo_p->returnCode = MB_BUTTON_CANCEL;
    Close( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
    ClaimListFree( );
    pmcProviderListFree( ProviderComboBox );

    pmcClaimListPaidCheck               = PaidCheckBox->Checked;
    pmcClaimListRejectedCheck           = RejectedCheckBox->Checked;
    pmcClaimListSubmittedCheck          = SubmittedCheckBox->Checked;
    pmcClaimListReadyCheck              = ReadyCheckBox->Checked;
    pmcClaimListNotReadyCheck           = NotReadyCheckBox->Checked;
    pmcClaimListReducedCheck            = ReducedCheckBox->Checked;
    pmcClaimListReducedAcceptCheck      = ReducedAcceptCheckBox->Checked;
    pmcClaimListRejectedAcceptCheck     = RejectedAcceptCheckBox->Checked;
    
    Action = caFree;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ReplyDateButtonClick(TObject *Sender)
{
    TDateSelectForm    *dateSelectForm_p;
    pmcDateSelectInfo_t dateSelectInfo;

    dateSelectInfo.mode = PMC_DATE_SELECT_PARMS;
    dateSelectInfo.string_p = NIL;
    dateSelectInfo.dateIn = ReplyDate;
    dateSelectInfo.caption_p = "Select Reply Date";

    dateSelectForm_p = new TDateSelectForm
    (
        NULL,
        &dateSelectInfo
    );

    dateSelectForm_p->ShowModal( );
    delete dateSelectForm_p;

    if( dateSelectInfo.returnCode == MB_BUTTON_OK )
    {
        ReplyDate = dateSelectInfo.dateOut;
        MbDateTime dateTime = MbDateTime( ReplyDate, 0 );

        ReplyDateEdit->Text = dateTime.MDY_DateString( );
        ClaimListGet( NIL );
    }
    if( Active ) ClaimDrawGrid->SetFocus( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::SubmitDateButtonClick(TObject *Sender)
{
    TDateSelectForm    *dateSelectForm_p;
    pmcDateSelectInfo_t dateSelectInfo;

    dateSelectInfo.mode = PMC_DATE_SELECT_PARMS;
    dateSelectInfo.string_p = NIL;
    dateSelectInfo.dateIn = SubmitDate;
    dateSelectInfo.caption_p = "Select Submit Date";

    dateSelectForm_p = new TDateSelectForm
    (
        NULL,
        &dateSelectInfo
    );

    dateSelectForm_p->ShowModal( );
    delete dateSelectForm_p;

    if( dateSelectInfo.returnCode == MB_BUTTON_OK )
    {
        SubmitDate = dateSelectInfo.dateOut;
        MbDateTime dateTime = MbDateTime( SubmitDate, 0 );

        SubmitDateEdit->Text = dateTime.MDY_DateString( );
        ClaimListGet( NIL );
    }
    if( Active ) ClaimDrawGrid->SetFocus( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::LatestDateButtonClick(TObject *Sender)
{
    TDateSelectForm    *dateSelectForm_p;
    pmcDateSelectInfo_t dateSelectInfo;

    dateSelectInfo.mode = PMC_DATE_SELECT_PARMS;
    dateSelectInfo.string_p = NIL;
    dateSelectInfo.dateIn = LatestDate;
    dateSelectInfo.caption_p = "Select Latest Date";

    dateSelectForm_p = new TDateSelectForm
    (
        NULL,
        &dateSelectInfo
    );

    dateSelectForm_p->ShowModal( );
    delete dateSelectForm_p;

    if( dateSelectInfo.returnCode == MB_BUTTON_OK )
    {
        LatestDate = dateSelectInfo.dateOut;
        MbDateTime dateTime = MbDateTime( LatestDate, 0 );

        LatestDateEdit->Text = dateTime.MDY_DateString( );
        ClaimListGet( NIL );
    }
    if( Active ) ClaimDrawGrid->SetFocus( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::EarliestDateButtonClick(TObject *Sender)
{
    TDateSelectForm    *dateSelectForm_p;
    pmcDateSelectInfo_t dateSelectInfo;

    dateSelectInfo.mode = PMC_DATE_SELECT_PARMS;
    dateSelectInfo.string_p = NIL;
    dateSelectInfo.dateIn = EarliestDate;
    dateSelectInfo.caption_p = "Select Earliest Date";

    dateSelectForm_p = new TDateSelectForm
    (
        NULL,
        &dateSelectInfo
    );
    dateSelectForm_p->ShowModal( );
    delete dateSelectForm_p;

    if( dateSelectInfo.returnCode == MB_BUTTON_OK )
    {
        EarliestDate = dateSelectInfo.dateOut;
        MbDateTime dateTime = MbDateTime( EarliestDate, 0 );
        EarliestDateEdit->Text = dateTime.MDY_DateString( );
        ClaimListGet( NIL );
    }
    if( Active ) ClaimDrawGrid->SetFocus( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::SubmitDateEditChange(TObject *Sender)
{
    if( !Active ) return;

    MbDateTime dateTime = MbDateTime( SubmitDate, 0 );
    SubmitDateEdit->Text = dateTime.MDY_DateString( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ReplyDateEditChange(TObject *Sender)
{
     if( !Active ) return;

    MbDateTime dateTime = MbDateTime( ReplyDate, 0 );
    ReplyDateEdit->Text = dateTime.MDY_DateString( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::EarliestDateEditChange(TObject *Sender)
{
    if( !Active ) return;

    MbDateTime dateTime = MbDateTime( EarliestDate, 0 );
    EarliestDateEdit->Text = dateTime.MDY_DateString( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::LatestDateEditChange(TObject *Sender)
{
    MbDateTime dateTime = MbDateTime( LatestDate, 0 );
    LatestDateEdit->Text = dateTime.MDY_DateString( );
    if( Active ) ClaimDrawGrid->SetFocus( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ClaimListGet( MbSQL *sqlIn_p )
{
    Char_p              cmd_p;
    Char_p              whereClause_p;
    Char_p              buf_p;
    pmcClaimStruct_p    claim_p;
    Ints_t              i;
    Int32u_t            count;
    TThermometer       *thermometer_p = NIL;
    MbSQL              *sql_p;
    TCursor             cursorOrig;

    if( SkipClaimListGet == TRUE ) return;

    cursorOrig = Screen->Cursor;
    Screen->Cursor = crHourGlass;

    if( sqlIn_p == NIL )
    {
        sql_p = new MbSQL( );
    }
    else
    {
        sql_p = sqlIn_p;
    }

    mbMalloc( cmd_p, 2048 );
    mbMalloc( whereClause_p, 1024 );
    mbMalloc( buf_p, 256 );

    ClaimListFree( );

    // Construct where clause
    sprintf( whereClause_p, "where " );
    sprintf( buf_p, "%s.%s=%s.%s",
        PMC_SQL_TABLE_PATIENTS, PMC_SQL_FIELD_ID,
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_FIELD_PATIENT_ID );
    strcat( whereClause_p, buf_p ); strcat( whereClause_p, " and " );

    sprintf( buf_p, "%s.%s=%s.%s",
        PMC_SQL_TABLE_PROVIDERS, PMC_SQL_FIELD_ID,
        PMC_SQL_TABLE_CLAIMS,    PMC_SQL_FIELD_PROVIDER_ID );
    strcat( whereClause_p, buf_p ); strcat( whereClause_p, " and " );

    // Add provider ID
    if( ProviderRadioGroup->ItemIndex == 1 )
    {
        sprintf( buf_p, "%s.%s=%ld", PMC_SQL_TABLE_CLAIMS, PMC_SQL_FIELD_PROVIDER_ID, ProviderId );
        strcat( whereClause_p, buf_p ); strcat( whereClause_p, " and " );
    }

    // If searching for a particular claim number, consider all patients
    if( ClaimSearchPageControl->ActivePage == ClaimNumberPage )
    {
        SkipClaimListGet = TRUE;
        PatientRadioGroup->ItemIndex = 0;

        ReadyCheckBox->Checked          = TRUE;
        NotReadyCheckBox->Checked       = TRUE;
        SubmittedCheckBox->Checked      = TRUE;
        RejectedCheckBox->Checked       = TRUE;
        ReducedCheckBox->Checked        = TRUE;
        PaidCheckBox->Checked           = TRUE;
        ReducedAcceptCheckBox->Checked  = TRUE;
        RejectedAcceptCheckBox->Checked = TRUE;

        SkipClaimListGet = FALSE;
    }

    // Add patient ID
    if( PatientRadioGroup->ItemIndex == 1 )
    {
        sprintf( buf_p, "%s.%s=%ld", PMC_SQL_TABLE_CLAIMS, PMC_SQL_FIELD_PATIENT_ID, PatientId );
        strcat( whereClause_p, buf_p ); strcat( whereClause_p, " and " );
    }

    // Add to where clause based on the search method
    if( ClaimSearchPageControl->ActivePage == ServiceDatesPage )
    {
        sprintf( buf_p, "( ( claims.%s>=%ld and claims.%s<=%ld ) or ( claims.%s>=%ld and claims.%s<=%ld ) ) and claims.%s=%ld",
            PMC_SQL_CLAIMS_FIELD_SERVICE_DATE_EARLIEST, EarliestDate,
            PMC_SQL_CLAIMS_FIELD_SERVICE_DATE_EARLIEST, LatestDate,
            PMC_SQL_CLAIMS_FIELD_SERVICE_DATE_LATEST,   EarliestDate,
            PMC_SQL_CLAIMS_FIELD_SERVICE_DATE_LATEST,   LatestDate,
            PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );
        strcat( whereClause_p, buf_p );
    }
    else if( ClaimSearchPageControl->ActivePage == ClaimNumberPage )
    {
        sprintf( buf_p, "claims.%s=%ld and claims.%s=%ld",
            PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER, SearchClaimNumber,
            PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );
        strcat( whereClause_p, buf_p );
    }
    else if( ClaimSearchPageControl->ActivePage == SubmitDatePage )
    {
        sprintf( buf_p, "claims.%s=%ld and claims.%s=%ld",
            PMC_SQL_CLAIMS_FIELD_SUBMIT_DATE, SubmitDate,
            PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );
        strcat( whereClause_p, buf_p );
    }
    else if( ClaimSearchPageControl->ActivePage == ReplyDatePage )
    {
        sprintf( buf_p, "claims.%s=%ld and claims.%s=%ld",
            PMC_SQL_CLAIMS_FIELD_REPLY_DATE, ReplyDate,
            PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );
        strcat( whereClause_p, buf_p );
    }
    else
    {
        mbDlgDebug(( "Got invalid search method\n" ));
    }

    sprintf( cmd_p, "select sum(%s.%s) from %s,%s,%s %s",
        PMC_SQL_TABLE_CLAIMS, PMC_SQL_FIELD_NOT_DELETED,
        PMC_SQL_TABLE_PATIENTS, PMC_SQL_TABLE_PROVIDERS, PMC_SQL_TABLE_CLAIMS,
        whereClause_p );

    // Figure out (approximately) how many records meet the criteria
    sql_p->Query( cmd_p );
    sql_p->RowGet( );
    count = sql_p->Int32u( 0 );
    
    // Throw up a thermometer if the count is large
    if( count > 2500 )
    {
        Char_t  tmp[32];
        sprintf( buf_p, "Retrieving %s claims from database...", mbStrInt32u( count, tmp ) );
        thermometer_p = new TThermometer( buf_p, 0, count, FALSE );
    }

    // Format SQL command
    //                          0      1      2      3      4      5      6      7      8      9
    sprintf( cmd_p, "select %s.%s, %s.%s, %s.%s, %s.%s, %s.%s, %s.%s, %s.%s, %s.%s, %s.%s, %s.%s, "
                           "%s.%s, %s.%s, %s.%s, %s.%s, %s.%s, %s.%s, %s.%s, %s.%s, %s.%s, %s.%s, "
                           "%s.%s, %s.%s, %s.%s, %s.%s, %s.%s, %s.%s, %s.%s, %s.%s, %s.%s, %s.%s, "
                           "%s.%s, %s.%s, %s.%s, %s.%s, %s.%s, %s.%s from %s,%s,%s %s",
        PMC_SQL_TABLE_PATIENTS, PMC_SQL_FIELD_LAST_NAME,                // 0
        PMC_SQL_TABLE_PATIENTS, PMC_SQL_FIELD_FIRST_NAME,               // 1
        PMC_SQL_TABLE_PATIENTS, PMC_SQL_PATIENTS_FIELD_PHN_PROV,        // 2
        PMC_SQL_TABLE_PATIENTS, PMC_SQL_PATIENTS_FIELD_PHN,             // 3
        PMC_SQL_TABLE_PATIENTS, PMC_SQL_FIELD_ID,                       // 4
        PMC_SQL_TABLE_PATIENTS, PMC_SQL_PATIENTS_FIELD_GENDER,          // 5
        PMC_SQL_TABLE_PATIENTS, PMC_SQL_PATIENTS_FIELD_DATE_OF_BIRTH,   // 6

        PMC_SQL_TABLE_PROVIDERS,PMC_SQL_FIELD_LAST_NAME,                // 7
        PMC_SQL_TABLE_PROVIDERS,PMC_SQL_PROVIDERS_FIELD_PROVIDER_NUMBER,// 8

        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_SERVICE_DATE,      // 9
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_SUBMIT_DATE,       // 10
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER,      // 11
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_CLAIM_SEQ,         // 12
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_FEE_SUBMITTED,     // 13
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_FEE_PAID,          // 14
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_REPLY_DATE,        // 15
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_FEE_CODE,          // 16
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_UNITS,             // 17
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_EXP_CODE,          // 18
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_CLAIM_INDEX,       // 19
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_FIELD_PROVIDER_ID,              // 20
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_CLAIM_HEADER_ID,   // 21
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_REFERRING_DR_NUM,  // 22
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_FIELD_ID,                       // 23
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_COMMENT_TO_MSP,    // 24
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_APPOINTMENT_TYPE,  // 25
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_ICD_CODE,          // 26
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_SERVICE_DATE_LAST, // 27
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_SERVICE_LOCATION,  // 28
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_REFERRING_DR_TYPE, // 29
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_REFERRING_DR_ID,   // 30
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_STATUS,            // 31
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_FEE_CODE_APPROVED, // 32
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_RUN_CODE,          // 33
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_SUB_COUNT,         // 34
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_CLAIMS_FIELD_FEE_PREMIUM,       // 35

        PMC_SQL_TABLE_PATIENTS, PMC_SQL_TABLE_PROVIDERS, PMC_SQL_TABLE_CLAIMS,
        whereClause_p   );

    i = 0;

    mbLockAcquire( ClaimQueueLock );

    if( sql_p->Query( cmd_p ) == FALSE )
    {
        mbDlgInfo( "failed to read claims\n" );
    }

    while( sql_p->RowGet( ) )
    {
#if 0
        {
            int j;
            mbLog( "----- Claim: %d ---------\n", i );
            for( j = 0 ; j < 36 ; j++ )
            {
                mbLog( "Field: %d %s\n",  j, sql_p->String( j ) );
            }
        }
#endif
        mbCalloc( claim_p, sizeof( pmcClaimStruct_t ) );

        mbMallocStr( claim_p->lastName_p,  sql_p->String( 0 ) );
        mbMallocStr( claim_p->firstName_p, sql_p->String( 1 ) );

        // PHN Province
//        pmcFormatPhnDisplay( sql_p->String( 2 ) , NIL, buf_p );
        mbStrClean( sql_p->String( 2 ), buf_p, TRUE );
        mbStrToUpper( buf_p );
        mbMallocStr( claim_p->phnProv_p, buf_p );

        // PHN
        pmcFormatPhnDisplay( sql_p->String( 3 ), NIL, buf_p );
        mbMallocStr( claim_p->phn_p, buf_p );

        claim_p->patientId      = sql_p->Int32u( 4 );
        claim_p->gender         = sql_p->Int32u( 5 );
        claim_p->dob            = sql_p->Int32u( 6 );

        // Provider Name
        mbMallocStr( claim_p->providerName_p, sql_p->String( 7 ) );

        claim_p->billingNumber  = (Int16u_t)sql_p->Int32u(  8 );
        claim_p->serviceDay     =           sql_p->Int32u(  9 );
        claim_p->submitDay      =           sql_p->Int32u( 10 );
        claim_p->claimNumber    =           sql_p->Int32u( 11 );

        // If the claim number has not yet been assigned use the INVALID value.
        if( claim_p->claimNumber == 0 ) claim_p->claimNumber = PMC_CLAIM_NUMBER_NOT_ASSIGNED;

        claim_p->claimSequence  = (Int16u_t)sql_p->Int32u( 12 );
        claim_p->feeSubmitted   =           sql_p->Int32u( 13 );
        claim_p->feePaid        =           sql_p->Int32u( 14 );
        claim_p->replyDay       =           sql_p->Int32u( 15 );

        // Fee code
        mbMallocStr( claim_p->feeCode_p, sql_p->String( 16 ) );
        claim_p->units = (Int16u_t)sql_p->Int32u( 17 );

        // Reply Code
        mbMallocStr( claim_p->expCode_p, sql_p->String( 18 ) );

        claim_p->claimIndex     = (Int16u_t)sql_p->Int32u( 19 );
        claim_p->providerId     =           sql_p->Int32u( 20 );
        claim_p->claimHeaderId  =           sql_p->Int32u( 21 );
        claim_p->referringNumber= (Int16u_t)sql_p->Int32u( 22 );
        claim_p->id             =           sql_p->Int32u( 23 );

        // Comment
        mbMallocStr( claim_p->comment_p, sql_p->String( 24 ) );
        claim_p->appointmentType = (Int16u_t)sql_p->Int32u( 25 );
        mbMallocStr( claim_p->icdCode_p, sql_p->String( 26 ) );
        claim_p->lastDay        =           sql_p->Int32u( 27 );
        claim_p->locationCode   = (Int16u_t)sql_p->Int32u( 28 );
        claim_p->refDrTypeIndex = (Int16u_t)sql_p->Int32u( 29 );
        claim_p->referringId    =           sql_p->Int32u( 30 );
        claim_p->status         = (Int16u_t)sql_p->Int32u( 31 );

        // Fee code approved
        mbMallocStr( claim_p->feeCodeApproved_p, sql_p->String( 32 )  );

        // Fee code approved
        mbMallocStr( claim_p->runCode_p, sql_p->String( 33 ) );

        // Submission count
        claim_p->subCount = (Int16u_t)sql_p->Int32u( 34 );

        // Premium fee paid
        claim_p->feePremium = (Int16u_t)sql_p->Int32u( 35 );

        // Sanity check
        if( ProviderRadioGroup->ItemIndex == 1 )
        {
            if( claim_p->providerId != ProviderId )
            {
                mbDlgDebug(( "god bad provider ID: %ld != %ld", claim_p->providerId, ProviderId ));
            }
        }

        nbDlgDebug(( "Name '%s %s patid: %ld claimid: %ld'",
            claim_p->firstName_p, claim_p->lastName_p, claim_p->patientId, claim_p->claimId ));

        ValidateClaim( claim_p );

        qInsertFirst( Claim_q, claim_p );
        if( thermometer_p ) thermometer_p->Set( i++ );
    }

    mbLockRelease( ClaimQueueLock );

    if( thermometer_p ) delete thermometer_p;

    // If just some portions of a claim are not ready, make them all not ready
    ClaimCheckNotReady(  );

    // Force a sort
    PreviousSortCode = 0;
    ClaimArrayGet( );

    mbFree( cmd_p );
    mbFree( whereClause_p );
    mbFree( buf_p );

    // Must delete sql object if allocated herein
    if( sqlIn_p == NIL ) delete sql_p;

    Screen->Cursor = cursorOrig;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ClaimCheckNotReady(  )
{
    // July 1 - Can't mark all claims as not ready because some
    // could be paid or reduced etc.

#if 0
    pmcClaimStruct_p        claim1_p, claim2_p;

    mbLockAcquire( ClaimQueueLock );


    // Two nested loops through entire list
    for( claim1_p  = (pmcClaimStruct_p)( Claim_q->linkage.flink );
         claim1_p != (pmcClaimStruct_p)( Claim_q );
         claim1_p  = (pmcClaimStruct_p)( claim1_p->linkage.flink ) )
    {
        for( claim2_p  = (pmcClaimStruct_p)( Claim_q->linkage.flink );
             claim2_p != (pmcClaimStruct_p)( Claim_q );
             claim2_p  = (pmcClaimStruct_p)( claim2_p->linkage.flink ) )
        {
            /* Ensure we are not comparing the entry to itself */
            if( claim1_p != claim2_p )
            {
                if( claim1_p->status == PMC_CLAIM_STATUS_NOT_READY )
                {
                    if( claim2_p->claimNumber == claim1_p->claimNumber &&
                        claim2_p->providerId == claim1_p->providerId )
                    {
                        claim2_p->status = PMC_CLAIM_STATUS_NOT_READY;
                    }
                }
            }
        }
    }

    mbLockRelease( ClaimQueueLock );
#endif
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ClaimArrayGet(  )
{
    Int32u_t            i;
    Int32u_t            row;
    pmcClaimStruct_p    claim_p, claim2_p;
    Boolean_t           include;
    Boolean_t           added;
    qHead_t             claim2Queue;
    qHead_p             claim2_q;
    TCursor             cursorOrig;

    // Do nothing if not yet ready
    if( SkipClaimListGet == TRUE ) return;

    mbLockAcquire( ClaimQueueLock );

    cursorOrig = Screen->Cursor;
    Screen->Cursor = crHourGlass;

    if( SortCode != PreviousSortCode )
    {
        PreviousSortCode = SortCode;
        claim2_q = qInitialize( &claim2Queue );

        // First, move all entries into a backup queue
        for( ; ; )
        {
            if( qEmpty( Claim_q ) ) break;
            claim_p = (pmcClaimStruct_p)qRemoveFirst( Claim_q );
            qInsertFirst( claim2_q, claim_p );
        }

        // Move all entries back into main queue in desired order
        for( ; ; )
        {
            if( qEmpty( claim2_q ) ) break;
            claim_p = (pmcClaimStruct_p)qRemoveFirst( claim2_q );

            added = FALSE;
            // Loop through existing entries in the main queue
            qWalk( claim2_p, Claim_q, pmcClaimStruct_p )
            {
                if( SortCode == PMC_SORT_ID_ASCENDING )
                {
                    if( ( added = ClaimSortIdAscending( claim_p, claim2_p ) ) == TRUE ) break;
                }
                else if( SortCode == PMC_SORT_ID_DESCENDING )
                {
                    if( ( added = ClaimSortIdDescending( claim_p, claim2_p ) ) == TRUE ) break;
                }
                else if( SortCode == PMC_SORT_NUMBER_ASCENDING )
                {
                    if( ( added = ClaimSortClaimNumberAscending( claim_p, claim2_p ) ) == TRUE ) break;
                }
                else if( SortCode == PMC_SORT_NUMBER_DESCENDING )
                {
                    if( ( added = ClaimSortClaimNumberDescending( claim_p, claim2_p ) ) == TRUE ) break;
                }
                else if( SortCode == PMC_SORT_DATE_ASCENDING )
                {
                    if( ( added = ClaimSortServiceDateAscending( claim_p, claim2_p ) ) == TRUE ) break;
                }
                else if( SortCode == PMC_SORT_DATE_DESCENDING )
                {
                    if( ( added = ClaimSortServiceDateDescending( claim_p, claim2_p ) ) == TRUE ) break;
                }
                else if( SortCode == PMC_SORT_NAME_ASCENDING )
                {
                    if( ( added = ClaimSortNameAscending( claim_p, claim2_p ) ) == TRUE ) break;
                }
                else if( SortCode == PMC_SORT_NAME_DESCENDING )
                {
                    if( ( added = ClaimSortNameDescending( claim_p, claim2_p ) ) == TRUE ) break;
                }
                else if( SortCode == PMC_SORT_PHN_ASCENDING )
                {
                    if( ( added = ClaimSortPhnAscending( claim_p, claim2_p ) ) == TRUE ) break;
                }
                else if( SortCode == PMC_SORT_PHN_DESCENDING )
                {
                    if( ( added = ClaimSortPhnDescending( claim_p, claim2_p ) ) == TRUE ) break;
                }
                else if( SortCode == PMC_SORT_EXP_ASCENDING )
                {
                    if( ( added = ClaimSortExpAscending( claim_p, claim2_p ) ) == TRUE ) break;
                }
                else if( SortCode == PMC_SORT_EXP_DESCENDING )
                {
                    if( ( added = ClaimSortExpDescending( claim_p, claim2_p ) ) == TRUE ) break;
                }
                else if( SortCode == PMC_SORT_CODE_ASCENDING )
                {
                    if( ( added = ClaimSortFeeCodeAscending( claim_p, claim2_p ) ) == TRUE ) break;
                }
                else if( SortCode == PMC_SORT_CODE_DESCENDING )
                {
                    if( ( added = ClaimSortFeeCodeDescending( claim_p, claim2_p ) ) == TRUE ) break;
                }
            }
            if( added == FALSE ) qInsertLast( Claim_q, claim_p );
        }
    } // End of sort function

    if( ClaimArray_pp ) mbFree( ClaimArray_pp )

    nbDlgDebug(( "Allocating %ld bytes (list size: %ld)",  sizeof(pmcClaimStruct_p) * Claim_q->size, Claim_q->size ));

    mbMalloc( ClaimArray_pp, sizeof(pmcClaimStruct_p) * Claim_q->size );

    i = 0;
    NotReadyCount = 0;
    ReadyCount = 0;
    SubmittedCount = 0;
    PaidCount = 0;
    RejectedCount = 0;
    ReducedCount = 0;
    ReducedAcceptCount = 0;
    RejectedAcceptCount = 0;

    NotReadyFeeClaimed = 0;
    ReadyFeeClaimed = 0;
    SubmittedFeeClaimed = 0;
    PaidFeeClaimed = 0;
    RejectedFeeClaimed = 0;
    RejectedAcceptFeeClaimed = 0;
    ReducedFeeClaimed = 0;
    ReducedAcceptFeeClaimed = 0;

    PaidFeePaid = 0;
    ReducedFeePaid = 0;
    ReducedAcceptFeePaid = 0;

    // Set default row to 1 (could change when building list)
    row = 1;

    qWalk( claim_p, Claim_q, pmcClaimStruct_p )
    {
        include = FALSE;

        // Check to see if this claim should be included in the array
        if( claim_p->status == PMC_CLAIM_STATUS_NOT_READY && NotReadyCheckBox->Checked == TRUE )
        {
            NotReadyCount++;
            include = TRUE;
            NotReadyFeeClaimed += claim_p->feeSubmitted;
        }
        else if( claim_p->status == PMC_CLAIM_STATUS_READY       && ReadyCheckBox->Checked == TRUE )
        {
            ReadyCount++;
            ReadyFeeClaimed += claim_p->feeSubmitted;
            include = TRUE;
        }
        else if( claim_p->status == PMC_CLAIM_STATUS_SUBMITTED && SubmittedCheckBox->Checked == TRUE )
        {
            SubmittedCount++;
            SubmittedFeeClaimed += claim_p->feeSubmitted;
            include = TRUE;
        }
        else if( claim_p->status == PMC_CLAIM_STATUS_PAID && PaidCheckBox->Checked == TRUE )
        {
            PaidCount++;
            PaidFeeClaimed += claim_p->feeSubmitted;
            PaidFeePaid += claim_p->feePaid;
            include = TRUE;
        }
        else if( claim_p->status == PMC_CLAIM_STATUS_REJECTED && RejectedCheckBox->Checked == TRUE )
        {
            RejectedCount++;
            RejectedFeeClaimed += claim_p->feeSubmitted;
            include = TRUE;
        }
        else if( claim_p->status == PMC_CLAIM_STATUS_REJECTED_ACCEPT && RejectedAcceptCheckBox->Checked == TRUE )
        {
            RejectedAcceptCount++;
            RejectedAcceptFeeClaimed += claim_p->feeSubmitted;
            include = TRUE;
        }
        else if( claim_p->status == PMC_CLAIM_STATUS_REDUCED && ReducedCheckBox->Checked == TRUE )
        {
            ReducedCount++;
            ReducedFeeClaimed += claim_p->feeSubmitted;
            ReducedFeePaid += claim_p->feePaid;
            include = TRUE;
        }
        else if( claim_p->status == PMC_CLAIM_STATUS_REDUCED_ACCEPT && ReducedAcceptCheckBox->Checked == TRUE )
        {
            ReducedAcceptCount++;
            ReducedAcceptFeeClaimed += claim_p->feeSubmitted;
            ReducedAcceptFeePaid += claim_p->feePaid;
            include = TRUE;
        }

        if( include )
        {
            ClaimArray_pp[i] = claim_p;

            // Check to see if this should be the highlighed row
            if( claim_p->id == SelectedClaimId )
            {
                row = i+1;
            }
            i++;
        }
    }

    TotalClaimsCount =    ReadyCount + NotReadyCount + SubmittedCount
                        + PaidCount + RejectedCount + ReducedCount
                        + ReducedAcceptCount + RejectedAcceptCount;

    ClaimArraySize = i;

    // Must ensure always two rows
    SkipClaimListDraw = TRUE;
    ClaimDrawGrid->RowCount  = (( i + 1 ) == 1 ) ? 2 : i + 1;

    if( i > PMC_CLAIM_LIST_LINES )
    {
        if( ( i - row ) >= PMC_CLAIM_LIST_LINES )
        {
            ClaimDrawGrid->TopRow    = row;
            ClaimDrawGrid->Row       = row;
        }
        else
        {
            ClaimDrawGrid->TopRow    = i - PMC_CLAIM_LIST_LINES;
            ClaimDrawGrid->Row       = row;
        }
    }
    else
    {
        // There is less than a full screen worth
        ClaimDrawGrid->TopRow    = 1;
        ClaimDrawGrid->Row       = row;
    }

    mbLockRelease( ClaimQueueLock );
    Screen->Cursor = cursorOrig;

    SkipClaimListDraw = FALSE;
    ClaimDrawGrid->Invalidate( );
    UpdateTotals();
    UpdateStatusLabels( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ClaimListFree( void )
{
    pmcClaimStruct_p   claim_p;

    mbLockAcquire( ClaimQueueLock );

    for( ; ; )
    {
        if( qEmpty( Claim_q ) )  break;

        claim_p = (pmcClaimStruct_p)qRemoveLast( Claim_q );

        mbFree( claim_p->firstName_p );
        mbFree( claim_p->lastName_p );
        mbFree( claim_p->phn_p );
        mbFree( claim_p->phnProv_p );
        mbFree( claim_p->feeCode_p );
        mbFree( claim_p->feeCodeApproved_p );
        mbFree( claim_p->expCode_p );
        mbFree( claim_p->providerName_p );
        mbFree( claim_p->comment_p );
        mbFree( claim_p->icdCode_p );
        mbFree( claim_p->runCode_p );
        mbFree( claim_p );
    }

    mbLockRelease( ClaimQueueLock );

    if( ClaimArray_pp )
    {
        mbFree( ClaimArray_pp );
        ClaimArray_pp = NIL;
        ClaimArraySize = 0;
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ValidateClaim( pmcClaimStruct_p claim_p )
{
    Char_p      buf_p;
    Int32u_t    status;
    Int32u_t    multiple;
    Int32u_t    fee;
    Int32u_t    debugReason = 0;

    // First look for statuses that cannot change dynamically
    if(    claim_p->status == PMC_CLAIM_STATUS_SUBMITTED
        || claim_p->status == PMC_CLAIM_STATUS_PAID
        || claim_p->status == PMC_CLAIM_STATUS_REJECTED
        || claim_p->status == PMC_CLAIM_STATUS_REDUCED
        || claim_p->status == PMC_CLAIM_STATUS_REDUCED_ACCEPT
        || claim_p->status == PMC_CLAIM_STATUS_REJECTED_ACCEPT
        || claim_p->status == PMC_CLAIM_STATUS_NOT_READY )
    {
        return;
    }

    // Sanity check remaining possible statuses
    if(    claim_p->status != PMC_CLAIM_STATUS_EDIT
        && claim_p->status != PMC_CLAIM_STATUS_READY
        && claim_p->status != PMC_CLAIM_STATUS_NOT_READY )
    {
        mbDlgExclaim( "Got bad claim status %d id %ld", claim_p->status, claim_p->id );
        debugReason = 1;
        return;
    }

    mbMalloc( buf_p, 128 );
    status = PMC_CLAIM_STATUS_NOT_READY;

    // Check patient first name
    if( strlen( claim_p->firstName_p ) == 0 )
    {
        debugReason = 2;
        goto exit;
    }

    // Check patient last name
    if( strlen( claim_p->lastName_p ) == 0 )
    {
        debugReason = 3;
        goto exit;
    }

    // Check patient PHN
    if( strcmp( claim_p->phnProv_p, PMC_PHN_DEFAULT_PROVINCE ) == 0 )
    {
        strcpy( buf_p, claim_p->phn_p );
        mbStrAlphaNumericOnly( buf_p );
        if( pmcPhnVerifyString( buf_p ) == FALSE )
        {
            debugReason = 4;
            goto exit;
        }
    }

    // Check patient gender
    if( claim_p->gender != PMC_SQL_GENDER_MALE && claim_p->gender != PMC_SQL_GENDER_FEMALE )
    {
        debugReason = 5;
        goto exit;
    }


    // Check patient date of birth
    if( claim_p->dob == 0 )
    {
        debugReason = 6;
        goto exit;
    }

    // Check appointment type
    if( claim_p->appointmentType != PMC_CLAIM_TYPE_SERVICE &&
        claim_p->appointmentType != PMC_CLAIM_TYPE_HOSPITAL )
    {
        debugReason = 7;
        goto exit;
    }

    // Check fee code
    mbStrClean( claim_p->feeCode_p, buf_p, TRUE );
    if( strlen( buf_p ) == 0 )
    {
        debugReason = 8;
        goto exit;
    }
    else
    {
        if( pmcFeeCodeVerify( buf_p, claim_p->serviceDay,
                              &fee, NIL, &multiple, NIL, NIL,
                              claim_p->appointmentType ) == FALSE )
        {
            debugReason = 9;
            goto exit;
        }
        if( fee == 0 )
        {
            goto exit;
        }
    }

    // Ensure billing and referring Drs are not the same
    if( claim_p->billingNumber != 0 )
    {
        if( claim_p->billingNumber == claim_p->referringNumber )
        {
            debugReason = 10;
            goto exit;
        }
    }

    // Check the referring doctor number
    if( claim_p->referringId != 0 && claim_p->referringNumber == 0 )
    {
        goto exit;
    }

    // Check that fee code supports multiple units
    if( claim_p->units > 1 && multiple == FALSE )
    {
        debugReason = 11;
        goto exit;
    }

    // Fee submitted cannot be 0
    if( claim_p->feeSubmitted == 0 )
    {
        debugReason = 12;
        goto exit;
    }

    if( claim_p->feeSubmitted > PMC_MAX_FEE )
    {
        debugReason = 13;
        goto exit;
    }
    // Ensure there is a service day
    if( claim_p->serviceDay == 0 )
    {
        debugReason = 14;
        goto exit;
    }

    // Ensure hospital claims have end day specified as well
    if( claim_p->appointmentType == PMC_CLAIM_TYPE_HOSPITAL )
    {
        if( claim_p->lastDay == 0 )
        {
            debugReason = 15;
            goto exit;
        }
    }

    // Check ICD code
    mbStrClean( claim_p->icdCode_p, buf_p, TRUE );
    if( pmcIcdVerify( buf_p, NIL, FALSE ) != TRUE )
    {
        debugReason = 16;
        goto exit;
    }

    // Verify location code
    if( pmcLocationCodeVerify( claim_p->locationCode ) == FALSE )
    {
        debugReason = 17;
        goto exit;
    }

    status = PMC_CLAIM_STATUS_READY;

exit:
    if( status != PMC_CLAIM_STATUS_READY )
    {
        mbLog( "Claim id %ld not ready, reason: %ld\n", claim_p->id, debugReason );
    }

    claim_p->status = status;
    mbFree( buf_p );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ProviderComboBoxChange(TObject *Sender)
{
    Boolean_t   update = FALSE;
    Int32u_t    newProviderId;
    Char_p      buf_p;

    if( !Active ) return;

    if( ProviderComboBoxSkipUpdate )
    {
        mbDlgDebug(( "called recursively" ));
        return;
    }

    mbMalloc( buf_p, 64 );

    ProviderComboBoxSkipUpdate = TRUE;

    newProviderId = pmcProviderIdGet( ProviderComboBox->Text.c_str() );
    nbDlgDebug(( "id: %d index: %d", newProviderId, ProviderComboBox->ItemIndex ));

    // If the provider id is unknown, set it back to what it was before
    if( newProviderId == 0 )
    {
        ProviderComboBox->ItemIndex = ProviderIndex;
    }
    else
    {
        ProviderIndex = ProviderComboBox->ItemIndex;

        if( newProviderId != ProviderId )
        {
            ProviderId = newProviderId;
            ClaimListInfo_p->providerId = ProviderId;

            update = TRUE;

            if( ProviderRadioGroup->ItemIndex == 0 )
            {
                if( update ) SkipClaimListGet = TRUE;
                ProviderRadioGroup->ItemIndex = 1;
                SkipClaimListGet = FALSE;
            }
        }
    }
    ProviderComboBox->Text = pmcProviderDescGet( ProviderId, buf_p );

    if( update ) ClaimListGet( NIL );
    ProviderComboBoxSkipUpdate = FALSE;

    ClaimDrawGrid->SetFocus( );

    mbFree( buf_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ProviderRadioGroupClick(TObject *Sender)
{
    if( !Active ) return;

    ClaimListGet( NIL );
    ClaimDrawGrid->SetFocus( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::PatientEditUpdate( Int32u_t patientId )
{
    Char_t              buf[128];
    PmcSqlPatient_p     patient_p = NIL;

    if( mbMalloc( patient_p, sizeof(PmcSqlPatient_t) ) == NIL ) goto exit;

    sprintf( buf, "" );

    if( patientId != 0 )
    {
        if( pmcSqlPatientDetailsGet( patientId, patient_p ) )
        {
            sprintf( buf, "%s, %s (%s)", patient_p->lastName,
                                         patient_p->firstName,
                                         patient_p->title );

            if( PatientRadioGroup->ItemIndex == 0 )
            {
                SkipClaimListGet = TRUE;
                PatientRadioGroup->ItemIndex = 1;
                SkipClaimListGet = FALSE;
            }
        }
    }
    PatientEdit->Text = buf;
    if( Active ) ClaimDrawGrid->SetFocus( );
exit:
    mbFree( patient_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::PatientSelectButtonClick(TObject *Sender)
{
    PatientSelect( 0 );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::PatientSelect( Int32u_t key )
{
    TPatientListForm   *patListForm_p;
    PmcPatListInfo_t    patListInfo;

    patListInfo.patientId = 0;
    patListInfo.providerId = ProviderId;
    patListInfo.mode = PMC_LIST_MODE_SELECT;
    patListInfo.character = key;
    patListInfo.allowGoto = FALSE;

    patListForm_p = new TPatientListForm( this, &patListInfo );
    patListForm_p->ShowModal( );
    delete patListForm_p;

    if( patListInfo.returnCode == MB_BUTTON_OK )
    {
        if( patListInfo.patientId != PatientId )
        {
            SelectedClaimId = 0;
            PatientId = patListInfo.patientId;
            PatientEditUpdate( patListInfo.patientId );
            ClaimListGet( NIL );
        }
    }
    else
    {
        // User pressed cancel button - do nothing
    }
    if( Active ) ClaimDrawGrid->SetFocus( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::PatientRadioGroupClick(TObject *Sender)
{
    Int32u_t    claimListSkipSetting;


    if( !Active ) return;

    claimListSkipSetting = SkipClaimListGet;
    SkipClaimListGet = TRUE;

    if( PatientRadioGroup->ItemIndex == 0 )
    {
        // Want to look at all patients
        PaidCheckBox->Checked           = PaidCheckBoxStore;
        RejectedCheckBox->Checked       = RejectedCheckBoxStore;
        SubmittedCheckBox->Checked      = SubmittedCheckBoxStore;
        ReadyCheckBox->Checked          = ReadyCheckBoxStore;
        NotReadyCheckBox->Checked       = NotReadyCheckBoxStore;
        ReducedCheckBox->Checked        = ReducedCheckBoxStore;
        ReducedAcceptCheckBox->Checked  = ReducedAcceptCheckBoxStore;
        RejectedAcceptCheckBox->Checked = RejectedAcceptCheckBoxStore;

    }
    else
    {
        // Want to look at a single patient
        PaidCheckBoxStore           = PaidCheckBox->Checked;
        RejectedCheckBoxStore       = RejectedCheckBox->Checked;
        SubmittedCheckBoxStore      = SubmittedCheckBox->Checked;
        ReadyCheckBoxStore          = ReadyCheckBox->Checked;
        NotReadyCheckBoxStore       = NotReadyCheckBox->Checked;
        ReducedCheckBoxStore        = ReducedCheckBox->Checked;
        ReducedAcceptCheckBoxStore  = ReducedAcceptCheckBox->Checked;
        RejectedAcceptCheckBoxStore = RejectedAcceptCheckBox->Checked;

        PaidCheckBox->Checked           = TRUE;
        RejectedCheckBox->Checked       = TRUE;
        SubmittedCheckBox->Checked      = TRUE;
        ReadyCheckBox->Checked          = TRUE;
        NotReadyCheckBox->Checked       = TRUE;
        ReducedCheckBox->Checked        = TRUE;
        ReducedAcceptCheckBox->Checked  = TRUE;
        RejectedAcceptCheckBox->Checked = TRUE;
    }

    SkipClaimListGet = claimListSkipSetting;

    ClaimListGet( NIL );
    ClaimDrawGrid->SetFocus( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::FormActivate(TObject *Sender)
{
    ClaimDrawGrid->SetFocus( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//    PMC_CLAIM_LIST_COL_LAST_NAME = 0
//    PMC_CLAIM_LIST_COL_FIRST_NAME
//    PMC_CLAIM_LIST_COL_PHN
//    PMC_CLAIM_LIST_COL_DATE
//    PMC_CLAIM_LIST_COL_CLAIM
//    PMC_CLAIM_LIST_COL_PROVIDER
//    PMC_CLAIM_LIST_COL_DATE_SUBMIT
//    PMC_CLAIM_LIST_COL_FEE_CODE
//    PMC_CLAIM_LIST_COL_UNITS
//    PMC_CLAIM_LIST_COL_CLAIMED
//    PMC_CLAIM_LIST_COL_PAID
//    PMC_CLAIM_LIST_COL_FEE_CODE_APPROVED
//    PMC_CLAIM_LIST_COL_EXP_CODE
//    PMC_CLAIM_LIST_COL_DATE_REPLY
//    PMC_CLAIM_LIST_COL_RUN_CODE
//    PMC_CLAIM_LIST_COL_COUNT
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ClaimDrawGridDrawCell
(
    TObject        *Sender,
    int             ACol,
    int             ARow,
    TRect          &Rect,
    TGridDrawState  State
)
{
    AnsiString          str = "";
    pmcClaimStruct_p    claim_p;
    MbDateTime          dateTime;
    Char_p              buf_p;
    Char_t              buf2[8];
    Ints_t              cellWidth;
    Ints_t              textWidth;
    Ints_t              offset = 2;
    Boolean_t           rightJustify = FALSE;

    if( SkipClaimListDraw ) return;
    mbMalloc( buf_p, 128 );

    if( ARow == 1 && ACol == 0 )
    {
        UpdateStatusLabels( );
    }

    rightJustify = FALSE;

    if( ARow == 0 )
    {
        // This is the title row
        str = pmcClaimListStrings[ACol];
        ClaimDrawGrid->Canvas->TextRect( Rect, Rect.Left + 2, Rect.Top, str );
    }
    else if( ARow <= (Ints_t)ClaimArraySize )
    {
        str = "";

        // Claim array size could be 0 if there are no entries to display
        if( ClaimArraySize > 0 )
        {
            // Look for selected Row to determined SelectedClaimId
            if( ARow == ClaimDrawGrid->Row )
            {
                claim_p = ClaimArray_pp[ARow - 1];
                SelectedClaimId = claim_p->id;
                sprintf( buf_p, "SelectedClaimId: %ld", SelectedClaimId );
                DebugEdit->Text = buf_p;
            }

            claim_p = ClaimArray_pp[ARow-1];

            sprintf( buf_p, "" );
            if( ACol == PMC_CLAIM_LIST_COL_LAST_NAME  ) str = claim_p->lastName_p;
            if( ACol == PMC_CLAIM_LIST_COL_FIRST_NAME ) str = claim_p->firstName_p;
            if( ACol == PMC_CLAIM_LIST_COL_PHN        ) str = claim_p->phn_p;

            if( ACol == PMC_CLAIM_LIST_COL_DATE && claim_p->serviceDay )
            {
                dateTime.SetDate( claim_p->serviceDay );
                str = dateTime.YMD_DateString( );
            }
            if( ACol == PMC_CLAIM_LIST_COL_CLAIM )
            {
                if( claim_p->claimNumber != PMC_CLAIM_NUMBER_NOT_ASSIGNED )
                {
                    sprintf( buf_p, "%ld", claim_p->claimNumber );
                    if( claim_p->submitDay )
                    {
                        sprintf( buf2, "-%ld", claim_p->claimSequence );
                        strcat( buf_p, buf2 );
                    }
                }
                str = buf_p;
            }

            if( ACol == PMC_CLAIM_LIST_COL_PROVIDER )
            {
                if( claim_p->providerName_p ) sprintf( buf_p, "%s", claim_p->providerName_p );
                str = buf_p;
            }

            if( ACol == PMC_CLAIM_LIST_COL_DATE_SUBMIT && claim_p->submitDay )
            {
                dateTime.SetDate( claim_p->submitDay );
                str = dateTime.YMD_DateString( );
            }
            if( ACol == PMC_CLAIM_LIST_COL_FEE_CODE && claim_p->feeCode_p )
            {
                // Strip leading zeros from the fee code
                str = pmcFeeCodeFormatDisplay( claim_p->feeCode_p, buf_p );
                rightJustify = TRUE;
            }
            if( ACol == PMC_CLAIM_LIST_COL_FEE_CODE_APPROVED && claim_p->feeCodeApproved_p )
            {
                // Strip leading zeros from the fee code
                if( strcmp( claim_p->feeCode_p, claim_p->feeCodeApproved_p ) != 0 )
                {
                    str = pmcFeeCodeFormatDisplay( claim_p->feeCodeApproved_p, buf_p );
                    rightJustify = TRUE;
                }
            }
            if( ACol == PMC_CLAIM_LIST_COL_UNITS && claim_p->units )
            {
                sprintf( buf_p, "%ld", claim_p->units );
                str = buf_p;
                rightJustify = TRUE;
            }
            if( ACol == PMC_CLAIM_LIST_COL_CLAIMED && claim_p->feeSubmitted )
            {
//                sprintf( buf_p, "%6.2f", (float)claim_p->feeSubmitted / 100.0 );
                mbDollarStrInt32u( claim_p->feeSubmitted, buf_p );
                str = buf_p;
                rightJustify = TRUE;
            }
            if( ACol == PMC_CLAIM_LIST_COL_PAID &&
                ( claim_p->feePaid || claim_p->status == PMC_CLAIM_STATUS_REDUCED || claim_p->status == PMC_CLAIM_STATUS_REDUCED_ACCEPT ) )
            {
//                sprintf( buf_p, "%6.2f", (float)claim_p->feePaid / 100.0 );
                mbDollarStrInt32u( claim_p->feePaid, buf_p );
                str = buf_p;
                rightJustify = TRUE;
            }
            if( ACol == PMC_CLAIM_LIST_COL_EXP_CODE && claim_p->expCode_p )
            {
                str = claim_p->expCode_p;
                rightJustify = TRUE;
            }
            if( ACol == PMC_CLAIM_LIST_COL_DATE_REPLY && claim_p->replyDay )
            {
                dateTime.SetDate( claim_p->replyDay );
                str = dateTime.YMD_DateString( );
            }
            if( ACol == PMC_CLAIM_LIST_COL_PREMIUM )
            {
                if( claim_p->locationCode >= 'B' && claim_p->locationCode <= 'E')
                {
                    sprintf( buf_p, "1" );
                    if( claim_p->feePremium > 0 ) { strcat( buf_p, "-P" ); }
                }
                if( claim_p->locationCode >= 'K' && claim_p->locationCode <= 'T')
                {
                    sprintf( buf_p, "2" );
                    if( claim_p->feePremium > 0 ) { strcat( buf_p, "-P" ); }
                }
                str = buf_p;
            }
            if( ACol == PMC_CLAIM_LIST_COL_ID )
            {
                sprintf( buf_p, "%ld", claim_p->claimHeaderId );
                str = buf_p;
                rightJustify = TRUE;
            }
            if( ACol == PMC_CLAIM_LIST_COL_RUN_CODE && claim_p->runCode_p )
            {
                str = claim_p->runCode_p;
            }
            if( ACol == PMC_CLAIM_LIST_COL_COUNT )
            {
                sprintf( buf_p, "%d", ARow );
                str = buf_p;
                rightJustify = TRUE;
            }
            if( ACol == PMC_CLAIM_LIST_COL_SUB_COUNT )
            {
                sprintf( buf_p, "%ld", claim_p->subCount );
                str = buf_p;
                rightJustify = TRUE;
            }
        }

        // Output the string... right justify if requested
        if( rightJustify )
        {
            cellWidth = Rect.Right - Rect.Left;
            textWidth = ClaimDrawGrid->Canvas->TextWidth( str );
            offset = cellWidth - textWidth - 3;
            if( offset < 3 ) offset = 3;
        }
        ClaimDrawGrid->Canvas->TextRect( Rect, Rect.Left + offset, Rect.Top, str);
    }
    else
    {
        nbDlgDebug(( "Got invalid Row: %ld, ClaimArraySize: %ld", ARow, ClaimArraySize ));
    }
    mbFree( buf_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------
void __fastcall TClaimListForm::RejectedAcceptCheckBoxClick(
      TObject *Sender)
{
    SelectedClaimId = 0;
    ClaimArrayGet( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------
void __fastcall TClaimListForm::ReducedAcceptCheckBoxClick(TObject *Sender)
{
    SelectedClaimId = 0;
    ClaimArrayGet( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::PaidCheckBoxClick(TObject *Sender)
{
    nbDlgDebug(( "called" ));
    SelectedClaimId = 0;
    ClaimArrayGet( );
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::SubmittedCheckBoxClick(TObject *Sender)
{
    nbDlgDebug(( "called" ));
    SelectedClaimId = 0;
    ClaimArrayGet( );
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::NotReadyCheckBoxClick(TObject *Sender)
{
    nbDlgDebug(( "called" ));
    SelectedClaimId = 0;
    ClaimArrayGet( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ReadyCheckBoxClick(TObject *Sender)
{
    SelectedClaimId = 0;
    ClaimArrayGet( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ReducedCheckBoxClick(TObject *Sender)
{
    SelectedClaimId = 0;
    ClaimArrayGet( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::RejectedCheckBoxClick(TObject *Sender)
{
    nbDlgDebug(( "called" ));
    SelectedClaimId = 0;
    ClaimArrayGet( );
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::UpdateTotals( )
{
    Char_t      buf[32];
    Int32u_t    totalFee;

    mbDollarStrInt32u( ReadyFeeClaimed, buf );
    ReadyFeeClaimedLabel->Caption = buf;

    mbDollarStrInt32u( NotReadyFeeClaimed, buf );
    NotReadyFeeClaimedLabel->Caption = buf;

    mbDollarStrInt32u( SubmittedFeeClaimed, buf );
    SubmittedFeeClaimedLabel->Caption = buf;

    mbDollarStrInt32u( RejectedFeeClaimed, buf );
    RejectedFeeClaimedLabel->Caption = buf;

    mbDollarStrInt32u( ReducedFeeClaimed, buf );
    ReducedFeeClaimedLabel->Caption = buf;

    mbDollarStrInt32u( PaidFeeClaimed, buf );
    PaidFeeClaimedLabel->Caption = buf;

    mbDollarStrInt32u( ReducedAcceptFeeClaimed, buf );
    ReducedAcceptFeeClaimedLabel->Caption = buf;

    mbDollarStrInt32u( RejectedAcceptFeeClaimed, buf );
    RejectedAcceptFeeClaimedLabel->Caption = buf;

    mbDollarStrInt32u( ReducedAcceptFeePaid, buf );
    ReducedAcceptFeePaidLabel->Caption = buf;

    mbDollarStrInt32u( ReducedFeePaid, buf );
    ReducedFeePaidLabel->Caption = buf;

    mbDollarStrInt32u( PaidFeePaid, buf );
    PaidFeePaidLabel->Caption = buf;

    mbStrInt32u( ReadyCount, buf );
    ReadyLabel->Caption = buf;

    mbStrInt32u( NotReadyCount, buf );
    NotReadyLabel->Caption = buf;

    mbStrInt32u( SubmittedCount, buf );
    SubmittedLabel->Caption = buf;

    mbStrInt32u( PaidCount, buf );
    PaidLabel->Caption = buf;

    mbStrInt32u( RejectedCount, buf );
    RejectedLabel->Caption = buf;

    mbStrInt32u( RejectedAcceptCount, buf );
    RejectedAcceptLabel->Caption = buf;

    mbStrInt32u( ReducedAcceptCount, buf );
    ReducedAcceptLabel->Caption = buf;

    mbStrInt32u( ReducedCount, buf );
    ReducedLabel->Caption = buf;

    mbStrInt32u( TotalClaimsCount, buf );
    TotalClaimsLabel->Caption = buf;

    totalFee = ReadyFeeClaimed + NotReadyFeeClaimed + SubmittedFeeClaimed +
               RejectedFeeClaimed + PaidFeeClaimed + ReducedFeeClaimed +
               ReducedAcceptFeeClaimed + RejectedAcceptFeeClaimed;

    mbDollarStrInt32u( totalFee, buf );
    TotalFeeClaimedLabel->Caption = buf;

    totalFee = PaidFeePaid + ReducedFeePaid + ReducedAcceptFeePaid;

    mbDollarStrInt32u( totalFee, buf );
    TotalFeePaidLabel->Caption = buf;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::NewButtonClick(TObject *Sender)
{
    pmcClaimEditInfo_t      claimEditInfo;
    TClaimEditForm         *claimEditForm;
    Int32s_t                newProviderIndex;
    Int32u_t                newProviderId;

    claimEditInfo.providerId = ProviderId;

    claimEditInfo.patientId = 0;
    if( PatientRadioGroup->ItemIndex == 1 )
    {
        // If this list is for a certain patient, preload that patient into new claim form
        claimEditInfo.patientId = PatientId;
    }

    claimEditInfo.mode = PMC_CLAIM_EDIT_MODE_NEW;

    claimEditForm = new TClaimEditForm( this, &claimEditInfo );
    claimEditForm->ShowModal();
    delete claimEditForm;

    if( claimEditInfo.returnCode == MB_BUTTON_CANCEL )
    {
        nbDlgDebug(( "User pressed cancel button" ));
    }
    if( claimEditInfo.returnCode == MB_BUTTON_OK )
    {
        SortCode = PMC_SORT_NUMBER_ASCENDING;
        SelectedClaimId = claimEditInfo.claimId;
        newProviderId = claimEditInfo.providerId;

        // Check to see if user changed provider
        if( newProviderId != ProviderId )
        {
            if( ( newProviderIndex = pmcProviderIndexGet( ProviderComboBox, newProviderId ) ) > -1 )
            {
                Char_t  buf[64];
                ProviderId = newProviderId;
                ProviderComboBox->ItemIndex = newProviderIndex;
                ProviderIndex = ProviderComboBox->ItemIndex;
                ProviderComboBox->Text = pmcProviderDescGet( ProviderId, buf );
            }
        }

        // SelectedClaimId = 0;
        // Reread the claim list after used cliked OK on new form
        ClaimListGet( NIL );
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ClaimDrawGridMouseDown
(
    TObject        *Sender,
    TMouseButton    Button,
    TShiftState     Shift,
    int             X,
    int             Y
)
{
    Ints_t    row = 0, col = 0;
    TRect           rect;
    Char_p          str_p;

    ClaimDrawGrid->MouseToCell( X, Y, col, row );
    rect = ClaimDrawGrid->CellRect( col, row );

    MouseInRow = ( row >= 0 && row <= (Ints_t)ClaimArraySize )   ? row : -1;
    MouseInCol = ( col >= 0 && col <= PMC_CLAIM_LIST_COL_COUNT ) ? col : -1;

    if( row == 0 )
    {
        str_p = pmcClaimListStrings[col];
        pmcButtonDown( ClaimDrawGrid->Canvas, &rect, str_p );
    }

    // There seems to be a problem in which activities other than mouse clicks
    // are causing the exp code to be displays.  Therefore, add following flag
    MouseDownExp = TRUE;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ClaimDrawGridMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    pmcButtonUp( );

    if( MouseInRow == 0 )
    {
        // Figure out if we should resort the list
        if( MouseInCol == PMC_CLAIM_LIST_COL_LAST_NAME || MouseInCol == PMC_CLAIM_LIST_COL_FIRST_NAME )
        {
            SelectedClaimId = 0;
            // Sort by name
            SortCode = ( SortCode == PMC_SORT_NAME_ASCENDING ) ? PMC_SORT_NAME_DESCENDING :  PMC_SORT_NAME_ASCENDING;
            ClaimArrayGet( );
        }
        else if( MouseInCol == PMC_CLAIM_LIST_COL_PHN )
        {
            SelectedClaimId = 0;
            // Sort by name
            SortCode = ( SortCode == PMC_SORT_PHN_ASCENDING ) ? PMC_SORT_PHN_DESCENDING :  PMC_SORT_PHN_ASCENDING;
            ClaimArrayGet( );
        }
        else if( MouseInCol == PMC_CLAIM_LIST_COL_DATE )
        {
            SelectedClaimId = 0;
            // Sort by date of service
            SortCode = ( SortCode == PMC_SORT_DATE_ASCENDING ) ?
                PMC_SORT_DATE_DESCENDING :  PMC_SORT_DATE_ASCENDING;
            ClaimArrayGet( );
        }
        else if( MouseInCol == PMC_CLAIM_LIST_COL_CLAIM )
        {
            SelectedClaimId = 0;
            // Sort by claim number
            SortCode = ( SortCode == PMC_SORT_NUMBER_ASCENDING ) ?
                PMC_SORT_NUMBER_DESCENDING :  PMC_SORT_NUMBER_ASCENDING;
            ClaimArrayGet( );
        }
        else if( MouseInCol == PMC_CLAIM_LIST_COL_EXP_CODE )
        {
            SelectedClaimId = 0;
            // Sort by claim number
            SortCode = ( SortCode == PMC_SORT_EXP_ASCENDING ) ?
                PMC_SORT_EXP_DESCENDING :  PMC_SORT_EXP_ASCENDING;
            ClaimArrayGet( );
        }
        else if( MouseInCol == PMC_CLAIM_LIST_COL_FEE_CODE )
        {
            SelectedClaimId = 0;
            // Sort by claim number
            SortCode = ( SortCode == PMC_SORT_CODE_ASCENDING ) ?
                PMC_SORT_CODE_DESCENDING :  PMC_SORT_CODE_ASCENDING;
            ClaimArrayGet( );
        }
        else if( MouseInCol == PMC_CLAIM_LIST_COL_ID )
        {
            SelectedClaimId = 0;
            // Sort by claim ID
            SortCode = ( SortCode == PMC_SORT_ID_ASCENDING ) ?
                PMC_SORT_ID_DESCENDING :  PMC_SORT_ID_ASCENDING;
            ClaimArrayGet( );
        }
    }
    ClaimDrawGrid->Invalidate( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Boolean_t __fastcall TClaimListForm::ClaimSortClaimNumberAscending
(
    pmcClaimStruct_p    claim1_p,
    pmcClaimStruct_p    claim2_p
)
{
    Boolean_t           added = FALSE;

    if( claim1_p->claimNumber > claim2_p->claimNumber )
    {
        qInsertBefore(  Claim_q, claim2_p, claim1_p );
        added = TRUE;
        goto exit;
    }

    if( claim1_p->claimNumber < claim2_p->claimNumber ) goto exit;

    // At this point it is known that the claim numbers are the same.  Next sort
    // by claim header index
    if( claim1_p->claimHeaderId > claim2_p->claimHeaderId )
    {
        qInsertBefore(  Claim_q, claim2_p, claim1_p );
        added = TRUE;
        goto exit;
    }

    if( claim1_p->claimHeaderId < claim2_p->claimHeaderId ) goto exit;

    // Next check the claimIndex - make them decending always
    if( claim1_p->claimIndex < claim2_p->claimIndex )
    {
        qInsertBefore(  Claim_q, claim2_p, claim1_p );
        added = TRUE;
        goto exit;
    }

    if( claim1_p->claimIndex > claim2_p->claimIndex ) goto exit;

    mbDlgExclaim( "Duplicate entires detected.\n" );

exit:
    return added;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Boolean_t __fastcall TClaimListForm::ClaimSortClaimNumberDescending
(
    pmcClaimStruct_p    claim1_p,
    pmcClaimStruct_p    claim2_p
)
{
    Boolean_t           added = FALSE;

    if( claim1_p->claimNumber < claim2_p->claimNumber )
    {
        qInsertBefore(  Claim_q, claim2_p, claim1_p );
        added = TRUE;
        goto exit;
    }

    if( claim1_p->claimNumber > claim2_p->claimNumber ) goto exit;

    // At this point it is known that the claim numbers are the same.  Next sort
    // by claim header index
    if( claim1_p->claimHeaderId < claim2_p->claimHeaderId )
    {
        qInsertBefore(  Claim_q, claim2_p, claim1_p );
        added = TRUE;
        goto exit;
    }

    if( claim1_p->claimHeaderId > claim2_p->claimHeaderId ) goto exit;

    // Next check the claimIndex
    if( claim1_p->claimIndex < claim2_p->claimIndex )
    {
        qInsertBefore(  Claim_q, claim2_p, claim1_p );
        added = TRUE;
        goto exit;
    }

    if( claim1_p->claimIndex > claim2_p->claimIndex ) goto exit;

    mbDlgExclaim( "Duplicate entires detected.\n" );

exit:

    return added;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Boolean_t __fastcall TClaimListForm::ClaimSortIdAscending
(
    pmcClaimStruct_p    claim1_p,
    pmcClaimStruct_p    claim2_p
)
{
    Boolean_t           added = FALSE;

    if( claim1_p->id > claim2_p->id )
    {
        qInsertBefore( Claim_q, claim2_p, claim1_p );
        added = TRUE;
    }
    if( claim1_p->id == claim2_p->id )
    {
        // Same ID... sub sort by claim number
        added = ClaimSortClaimNumberAscending( claim1_p, claim2_p );
    }
    return added;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Boolean_t __fastcall TClaimListForm::ClaimSortIdDescending
(
    pmcClaimStruct_p    claim1_p,
    pmcClaimStruct_p    claim2_p
)
{
    Boolean_t           added = FALSE;
      if( claim1_p->id < claim2_p->id )
    {
        qInsertBefore( Claim_q, claim2_p, claim1_p );
        added = TRUE;
    }
    if( claim1_p->id == claim2_p->id )
    {
        // Same ID... sub sort by claim number
        added = ClaimSortClaimNumberDescending( claim1_p, claim2_p );
    }
    return added;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Boolean_t __fastcall TClaimListForm::ClaimSortPhnAscending
(
    pmcClaimStruct_p    claim1_p,
    pmcClaimStruct_p    claim2_p
)
{
    Boolean_t           added = FALSE;
    Ints_t              result;

    result = strcmp( claim1_p->phn_p, claim2_p->phn_p );

    if( result < 0 )
    {
        qInsertBefore(  Claim_q, claim2_p, claim1_p );
        added = TRUE;
    }
    if( result == 0 )
    {
        added = ClaimSortClaimNumberAscending( claim1_p, claim2_p );
    }
    return added;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Boolean_t __fastcall TClaimListForm::ClaimSortPhnDescending
(
    pmcClaimStruct_p    claim1_p,
    pmcClaimStruct_p    claim2_p
)
{
    Boolean_t           added = FALSE;
    Ints_t              result;

    result = strcmp( claim1_p->phn_p, claim2_p->phn_p );

    if( result > 0 )
    {
        qInsertBefore(  Claim_q, claim2_p, claim1_p );
        added = TRUE;
    }
    if( result == 0 )
    {
        added = ClaimSortClaimNumberAscending( claim1_p, claim2_p );
    }
    return added;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Boolean_t __fastcall TClaimListForm::ClaimSortFeeCodeAscending
(
    pmcClaimStruct_p    claim1_p,
    pmcClaimStruct_p    claim2_p
)
{
    Boolean_t           added = FALSE;
    Ints_t              result;

    result = strcmp( claim1_p->feeCode_p, claim2_p->feeCode_p );

    if( result < 0 )
    {
        qInsertBefore(  Claim_q, claim2_p, claim1_p );
        added = TRUE;
    }
    if( result == 0 )
    {
        added = ClaimSortClaimNumberAscending( claim1_p, claim2_p );
    }
    return added;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Boolean_t __fastcall TClaimListForm::ClaimSortFeeCodeDescending
(
    pmcClaimStruct_p    claim1_p,
    pmcClaimStruct_p    claim2_p
)
{
    Boolean_t           added = FALSE;
    Ints_t              result;

    result = strcmp( claim1_p->feeCode_p, claim2_p->feeCode_p );

    if( result > 0 )
    {
        qInsertBefore(  Claim_q, claim2_p, claim1_p );
        added = TRUE;
    }
    if( result == 0 )
    {
        added = ClaimSortClaimNumberAscending( claim1_p, claim2_p );
    }
    return added;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Boolean_t __fastcall TClaimListForm::ClaimSortExpAscending
(
    pmcClaimStruct_p    claim1_p,
    pmcClaimStruct_p    claim2_p
)
{
    Boolean_t           added = FALSE;
    Ints_t              result;

    if( strlen( claim1_p->expCode_p ) == 0 )
    {
        goto exit;
    }
    else
    {
        if( strlen( claim2_p->expCode_p ) == 0 )
        {
            result = -1;
        }
        else
        {
            result = strcmp( claim1_p->expCode_p, claim2_p->expCode_p );
        }
    }

    if( result < 0 )
    {
        qInsertBefore(  Claim_q, claim2_p, claim1_p );
        added = TRUE;
    }
    if( result == 0 )
    {
        added = ClaimSortClaimNumberAscending( claim1_p, claim2_p );
    }
exit:
    return added;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Boolean_t __fastcall TClaimListForm::ClaimSortExpDescending
(
    pmcClaimStruct_p    claim1_p,
    pmcClaimStruct_p    claim2_p
)
{
    Boolean_t           added = FALSE;
    Ints_t              result;

    if( strlen( claim1_p->expCode_p ) == 0 )
    {
        goto exit;
    }
    else
    {
        if( strlen( claim2_p->expCode_p ) == 0 )
        {
            result = 1;
        }
        else
        {
            result = strcmp( claim1_p->expCode_p, claim2_p->expCode_p );
        }
    }

    if( result > 0 )
    {
        qInsertBefore(  Claim_q, claim2_p, claim1_p );
        added = TRUE;
    }
    if( result == 0 )
    {
        added = ClaimSortClaimNumberAscending( claim1_p, claim2_p );
    }
exit:
    return added;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Boolean_t __fastcall TClaimListForm::ClaimSortServiceDateAscending
(
    pmcClaimStruct_p    claim1_p,
    pmcClaimStruct_p    claim2_p
)
{
    Boolean_t           added = FALSE;

    if( claim1_p->serviceDay > claim2_p->serviceDay )
    {
        qInsertBefore( Claim_q, claim2_p, claim1_p );
        added = TRUE;
    }
    if( claim1_p->serviceDay == claim2_p->serviceDay )
    {
        // Same date... sub sort by claim number
        added = ClaimSortClaimNumberAscending( claim1_p, claim2_p );
    }
    return added;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Boolean_t __fastcall TClaimListForm::ClaimSortServiceDateDescending
(
    pmcClaimStruct_p    claim1_p,
    pmcClaimStruct_p    claim2_p
)
{
    Boolean_t           added = FALSE;

    if( claim1_p->serviceDay < claim2_p->serviceDay )
    {
        qInsertBefore( Claim_q, claim2_p, claim1_p );
        added = TRUE;
    }
    if( claim1_p->serviceDay == claim2_p->serviceDay )
    {
        // Same date... sub sort by claim number
        added = ClaimSortClaimNumberAscending( claim1_p, claim2_p );
    }
    return added;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Boolean_t __fastcall TClaimListForm::ClaimSortNameAscending
(
    pmcClaimStruct_p    claim1_p,
    pmcClaimStruct_p    claim2_p
)
{
    Boolean_t           added = FALSE;
    Ints_t              result;

    result = strcmp( claim1_p->lastName_p, claim2_p->lastName_p );

    if( result < 0 )
    {
        qInsertBefore( Claim_q, claim2_p, claim1_p );
        added = TRUE;
    }
    if( result == 0 )
    {
        result = strcmp( claim1_p->firstName_p, claim2_p->firstName_p );
        if( result < 0 )
        {
            qInsertBefore(  Claim_q, claim2_p, claim1_p );
            added = TRUE;
        }
        else if( result == 0 )
        {
            added = ClaimSortClaimNumberAscending( claim1_p, claim2_p );
        }
    }
    return added;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Boolean_t __fastcall TClaimListForm::ClaimSortNameDescending
(
    pmcClaimStruct_p    claim1_p,
    pmcClaimStruct_p    claim2_p
)
{
    Boolean_t           added = FALSE;
    Ints_t              result;

    result = strcmp( claim1_p->lastName_p, claim2_p->lastName_p );

    if( result > 0 )
    {
        qInsertBefore( Claim_q, claim2_p, claim1_p );
        added = TRUE;
    }
    if( result == 0 )
    {
        result = strcmp( claim1_p->firstName_p, claim2_p->firstName_p );
        if( result > 0 )
        {
            qInsertBefore( Claim_q, claim2_p, claim1_p );
            added = TRUE;
        }
        else if( result == 0 )
        {
            added = ClaimSortClaimNumberAscending( claim1_p, claim2_p );
        }
    }
    return added;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ClaimGridPopupPopup(TObject *Sender)
{
    pmcClaimStruct_p    claim_p;

    pmcPopupItemEnableAll( ClaimGridPopup, FALSE );

    // Get doctor over which mouse is located
    if( MouseInRow <= ClaimArraySize && MouseInRow > 0 )
    {
        ClaimDrawGrid->Row = MouseInRow;
        pmcPopupItemEnable( ClaimGridPopup, PMC_CLAIM_LIST_POPUP_EDIT,          TRUE );
        pmcPopupItemEnable( ClaimGridPopup, PMC_CLAIM_LIST_POPUP_NEW,           TRUE );
        pmcPopupItemEnable( ClaimGridPopup, PMC_CLAIM_LIST_POPUP_VIEW,          TRUE );
        pmcPopupItemEnable( ClaimGridPopup, PMC_CLAIM_LIST_POPUP_LIST_PATIENT,  TRUE );
        pmcPopupItemEnable( ClaimGridPopup, PMC_CLAIM_LIST_POPUP_LIST_APPOINTMENTS,  TRUE );

        // See if EXP option should be enabled
        claim_p = ClaimArray_pp[MouseInRow - 1];
        if( strlen( claim_p->expCode_p ) > 0 )
        {
            pmcPopupItemEnable( ClaimGridPopup, PMC_CLAIM_LIST_POPUP_VIEW_EXP,  TRUE );
        }
    }
    else
    {
        nbDlgDebug(( "Mouse in row: %d ClaimArraySize: %d", MouseInRow, ClaimArraySize ));
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ClaimGridPopupEditClick(TObject *Sender)
{
    ClaimEdit( );
    return;
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ClaimGridPopupNewClick(TObject *Sender)
{
    pmcClaimStruct_p    claim_p;
    pmcClaimEditInfo_t  claimEditInfo;
    TClaimEditForm     *claimEditForm;

    // Sanity Check
    if( MouseInRow > ClaimArraySize )
    {
        mbDlgDebug(( "Error, MouseInRow %ld  > ClaimArraySize %ld", MouseInRow, ClaimArraySize ));
        return;
    }

    nbDlgDebug(( "Mouse in row: %ld (%ld(, claimNumber: %ld, providerId: %ld",
        MouseInRow, ClaimDrawGrid->Row, claim_p->claimNumber, claim_p->providerId ));

    claim_p = ClaimArray_pp[MouseInRow - 1];

    claimEditInfo.providerId = claim_p->providerId;
    claimEditInfo.patientId = claim_p->patientId;
    claimEditInfo.mode = PMC_CLAIM_EDIT_MODE_NEW;
    claimEditInfo.claimNumber = claim_p->claimNumber;

    claimEditForm = new TClaimEditForm( this, &claimEditInfo );
    claimEditForm->ShowModal();
    delete claimEditForm;

    if( claimEditInfo.returnCode == MB_BUTTON_CANCEL )
    {
        nbDlgDebug(( "User pressed cancel button" ));
    }

    if( claimEditInfo.returnCode == MB_BUTTON_OK )
    {
        SortCode = PMC_SORT_NUMBER_ASCENDING;
        SelectedClaimId = claimEditInfo.claimId;
        // Reread the claim list after used cliked OK on new form
        ClaimListGet( NIL );
    }
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ClaimDrawGridRowMoved(TObject *Sender,
      int FromIndex, int ToIndex)
{
    mbDlgDebug(( "called, top row: %ld", ClaimDrawGrid->TopRow ));
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ClaimDrawGridTopLeftChanged
(
      TObject *Sender
)
{
    UpdateStatusLabels( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::UpdateStatusLabels
(
    void
)
{
    pmcClaimStruct_p    claim_p;
    Ints_t              i, claimIndex;

    // Set the status according to the corresponding claim
    for( i = 0 ; i < PMC_CLAIM_LIST_LINES ; i++ )
    {
        claimIndex = ClaimDrawGrid->TopRow - 1 + i;
        if( claimIndex < ClaimArraySize )
        {
            claim_p = ClaimArray_pp[claimIndex];

            StatusLabel[i]->Caption = pmcClaimStatusStrings[claim_p->status];
            StatusLabel[i]->Color = (TColor)pmcClaimStatusColors[ claim_p->status ];
        }
        else
        {
            StatusLabel[i]->Caption = "";
            StatusLabel[i]->Color = clBtnFace;
        }
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ClaimGridPopupViewClick(TObject *Sender)
{
    pmcClaimStruct_p    claim_p;
    pmcClaimEditInfo_t  claimEditInfo;
    TClaimEditForm     *claimEditForm;

    // Sanity Check
    if( MouseInRow > ClaimArraySize )
    {
        mbDlgDebug(( "Error, MouseInRow %ld  > ClaimArraySize %ld", MouseInRow, ClaimArraySize ));
        return;
    }

    nbDlgDebug(( "Mouse in row: %ld (%ld(, claimNumber: %ld, providerId: %ld",
        MouseInRow, ClaimDrawGrid->Row, claim_p->claimNumber, claim_p->providerId ));

    claim_p = ClaimArray_pp[MouseInRow - 1];

    claimEditInfo.providerId = claim_p->providerId;
    claimEditInfo.patientId = claim_p->patientId;
    claimEditInfo.mode = PMC_CLAIM_EDIT_MODE_VIEW;
    claimEditInfo.claimNumber = claim_p->claimNumber;
    claimEditInfo.claimHeaderId = claim_p->claimHeaderId;

    claimEditForm = new TClaimEditForm( this, &claimEditInfo );
    claimEditForm->ShowModal();
    delete claimEditForm;

    if( claimEditInfo.updateClaims == TRUE ) ClaimListGet( NIL );

    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ClaimGridPopupListPatientClick
(
      TObject *Sender
)
{
    pmcClaimStruct_p    claim_p;

    // Sanity Check
    if( MouseInRow > ClaimArraySize )
    {
        mbDlgDebug(( "Error, MouseInRow %ld  > ClaimArraySize %ld", MouseInRow, ClaimArraySize ));
        return;
    }
    claim_p = ClaimArray_pp[MouseInRow - 1];

    if( claim_p->patientId )
    {
        Int32u_t    skipSetting;

        nbDlgDebug(( "Mouse in row: %ld (%ld(, claimNumber: %ld, patientId: %ld",
            MouseInRow, ClaimDrawGrid->Row, claim_p->claimNumber, claim_p->patientId ));

        skipSetting = SkipClaimListGet;
        SkipClaimListGet = TRUE;

        PatientId = claim_p->patientId;
        PatientEditUpdate( PatientId );
        SelectedClaimId = 0;

        SkipClaimListGet = skipSetting;
        ClaimListGet( NIL );
    }
}

//---------------------------------------------------------------------------
// Function: SubmitButtonClick
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

typedef struct  providerStruct_s
{
    qLinkage_t          linkage;
    Int32u_t            id;
    Int32u_t            claimNumber;
} providerStruct_t, *providerStruct_p;

void __fastcall TClaimListForm::SubmitButtonClick(TObject *Sender)
{
    pmcClaimStruct_p    claim_p, claim2_p, claimCopy_p;
    Boolean_t           add;
    qHead_t             claim2Queue;
    qHead_p             claim2_q;
    qHead_t             providerQueue;
    qHead_p             provider_q;
    providerStruct_p    provider_p;
    Int32u_t            notReadyCount = 0;
    Int32u_t            readyCount = 0;
    Char_p              buf_p;
    Char_p              errorString_p;
    Char_p              archiveFileName_p;
    Char_p              archiveBaseName_p;
    Int32u_t            currentProviderId = 0;
    Int32u_t            errorCode = FALSE;
    Int32u_t            today;
    Boolean_t           tablesLocked = FALSE;
    Boolean_t           run = FALSE;
    Boolean_t           prompted = FALSE;
    FILE               *fp = NIL;
    Boolean_t           foundClaimsin = FALSE;
    Boolean_t           foundValidrpt = FALSE;
    Boolean_t           foundNotice = FALSE;
    Boolean_t           foundReturns = FALSE;
    Boolean_t           foundInfo = FALSE;
    Boolean_t           found = FALSE;
    Boolean_t           terminateFlag = FALSE;
    TThermometer       *thermometer_p = NIL;
    Int32u_t            clinicNumber = 0;
    MbSQL               sql;
    TCursor             cursorOrig;

    cursorOrig = Screen->Cursor;
    Screen->Cursor = crHourGlass;

    mbLog( "Claims submit start\n" );

    mbMalloc( buf_p, 512 );
    mbMalloc( errorString_p, 128 );
    mbMalloc( archiveFileName_p, 128 );
    mbMalloc( archiveBaseName_p, 128 );

    pmcMspFilesLocate( &foundClaimsin, &foundInfo, &foundValidrpt, &foundReturns, &foundNotice, TRUE );

    today = mbToday( );

    // Initialize temporary queue
    claim2_q = qInitialize( &claim2Queue );
    provider_q = qInitialize( &providerQueue );

    terminateFlag = TRUE;


    if( !pmcCfg[CFG_CAN_SUBMIT_FLAG].value )
    {
        prompted = TRUE;
        if( mbDlgOkCancel( "This computer is not configured to submit MSP files.\n"
                           "However, a test of the submission file generation process is allowed.\nProceed with test?" ) == MB_BUTTON_CANCEL )
        {
            goto exit;
        }
    }
    else
    {
        // Check for the existance of MSP files
        if( foundValidrpt )
        {
            if( mbDlgOkCancel( "The MSP response file %s was detected.\n"
                               "This file must be processed before more claims can be submitted.\n"
                               "Would you like to process this file now?", pmcValidrpt_p ) == MB_BUTTON_CANCEL )
            {
                goto exit;
            }
            else
            {
                if( pmcMspValidrptFileHandle( FALSE ) == FALSE ) goto exit;
            }
        }

        if( foundReturns )
        {
            if( mbDlgOkCancel( "The MSP returns file %s was detected.\n"
                               "This file must be processed before more claims can be submitted.\n"
                               "Would you like to process this file before proceeding with the submission?", pmcReturns_p ) == MB_BUTTON_CANCEL )
            {
                goto exit;
            }
            else
            {
                if( pmcMspReturnsFileHandle( ) == FALSE ) goto exit;

                // Update the display
                ClaimListGet( NIL );
                UpdateStatusLabels( );

                if( mbDlgOkCancel( "The MPS returns file %s was successfully processed.\n"
                                   "Proceed with claims submission?\n", pmcReturns_p ) == MB_BUTTON_CANCEL )
                {
                    goto exit;
                }
            }
        }

        if( foundNotice )
        {
            if( pmcMspNoticeFileHandle( ) == FALSE ) goto exit;
        }

        if( foundClaimsin )
        {
            // Sanity Check
            if( !foundInfo )
            {
                mbDlgExclaim( "Error: found %s but not %s.\nContact system administrator", pmcClaimsin_p, pmcInfo_p );
                goto exit;
            }

            if( mbDlgOkCancel( "An existing MSP claims file, %s, was detected.\n"
                               "You must send this file to the MSP before a new claims file can be prepared for submission.\n"
                               "Would you like to send the existing file to the MSP now?", pmcClaimsin_p ) == MB_BUTTON_OK )
            {
                if( pmcSubmit( ) == FALSE ) goto exit;

                ClaimListGet( NIL );
                UpdateStatusLabels( );

                if( mbDlgOkCancel( "The existing MSP claims file %s was successfully submitted to the MSP.\n"
                                   "Proceed with new submission?", pmcClaimsin_p ) == MB_BUTTON_CANCEL )
                {
                    terminateFlag = FALSE;
                    goto exit;
                }
            }
            else
            {
                goto exit;
            }
        }
        else
        {
            if( foundInfo )
            {
                mbDlgExclaim( "Error: found %s but not %s.\nContact system administrator",  pmcInfo_p, pmcClaimsin_p );
                goto exit;
            }
        }
    }

    // MAB:20030301: Temporary code... check to see if the submission
    // interval straddles Feb. 1, 2003 (the clinic number changed from
    // 543 to 0 on this day.
    if( LatestDate >= 20030201 && EarliestDate < 20030201 )
    {
        mbDlgInfo( "Claims before and after Feb. 1, 2003 cannot be submitted\n"
                   "in the same batch (due to differing clinic numbers).\n\n"
                   "You must set the Latest Date to Jan. 31, 2003 (or earlier),\n"
                   "or the Earliest Date to Feb. 1, 2003 (or later)." );
        goto exit;
    }

    if( LatestDate < 20030201 && EarliestDate < 20030201 )
    {
        clinicNumber = 543;
    }
    else if( LatestDate >= 20030201 && EarliestDate >= 20030201 )
    {
        clinicNumber = 0;
    }
    else
    {
        // Sanity check
        mbDlgError( "Submission date error\n" );
        goto exit;
    }

    // Sanity Check... lets double check file statuses after they have supposedly been procesed
    pmcMspFilesLocate( &foundClaimsin, &foundInfo, &foundValidrpt, &foundReturns, &foundNotice, TRUE );
    if( foundClaimsin || foundInfo || foundReturns || foundValidrpt )
    {
        mbDlgExclaim( "Error: existing MSP files detected.\nContact system administrator." );
        goto exit;
    }

    // Make the archive file name ahead of time (so we can print it)
    sprintf( archiveBaseName_p, "%s",    pmcMakeFileName( pmcCfg[CFG_MSP_ARCHIVE_DIR].str_p, buf_p ) );
    sprintf( archiveFileName_p, "%s.%s", archiveBaseName_p, PMC_MSP_FILENAME_CLAIMSIN );

    // Check that "All Providers" are selected
    if( ProviderRadioGroup->ItemIndex != 0 )
    {
        prompted = TRUE;
        if( mbDlgOkCancel( "Proceed with submission without 'Show Providers - All' checked?\n\n"
                           "(All providers should be submitted in one file.)  " ) == MB_BUTTON_CANCEL )
        goto exit;
    }

    if( ReadyCheckBox->Checked == FALSE )
    {
        mbDlgInfo( "Cannot proceed with submission without 'Show Claims - Ready' checked." );
        goto exit;
    }

    // Check that "All Patients" are selected
    if( PatientRadioGroup->ItemIndex != 0 )
    {
        prompted = TRUE;
        if( mbDlgOkCancel( "Proceed with submission without 'Show Patients - All' checked?" ) == MB_BUTTON_CANCEL )
        {
            goto exit;
        }
    }

    if( ReadyCount == 0 )
    {
        prompted = TRUE;
        mbDlgInfo( "No claims with status 'Ready' were found." );
        goto exit;
    }

    // Query if we should proceed even though there are "not ready" claims
    if( NotReadyCount )
    {
        prompted = TRUE;
        if( mbDlgOkCancel( "Some 'Not Ready' claims were detected.  Proceed with submission?" ) == MB_BUTTON_CANCEL )
        {
            goto exit;
        }
    }

    // Give user one last chance to back out
    if( !prompted )
    {
        if( mbDlgOkCancel( "Submission conditions met.  Proceed with submission?" ) == MB_BUTTON_CANCEL )
        {
            goto exit;
        }
    }

    terminateFlag = FALSE;

    // Suspend polling before lock.  Must ensure polling is stopped or deadlock can ensue
    pmcSuspendPollInc( );

    // Lock all of the tables we are going to use
    sprintf( buf_p, "lock tables %s write, %s write, %s write, %s write",
        PMC_SQL_TABLE_PATIENTS,
        PMC_SQL_TABLE_PROVIDERS,
        PMC_SQL_TABLE_CLAIMS,
        PMC_SQL_TABLE_CLAIM_HEADERS );
//    pmcSqlExec( buf_p );
    sql.Update( buf_p );

    tablesLocked = TRUE;

    // Re-read the claims list after the tables have been locked.  There
    // seems to be some kind of bug if the claims are in an unexpected
    // order.  Set the order here.
    SortCode = PMC_SORT_NUMBER_ASCENDING;
    ClaimListGet( &sql );

    mbLockAcquire( ClaimQueueLock );

    // First, identify all the providers in the list

    qWalk( claim_p, Claim_q, pmcClaimStruct_p )
    {
        found = FALSE;
        qWalk( provider_p, provider_q, providerStruct_p )
        {
            if( claim_p->providerId == provider_p->id )
            {
                found = TRUE;
                break;
            }
        }

        if( !found )
        {
            mbCalloc( provider_p, sizeof( providerStruct_t ) );
            qInsertLast( provider_q, provider_p );
            provider_p->id = claim_p->providerId;
            {
//                Int32u_t    count;
                sprintf( buf_p, "select %s from %s where %s = %ld",
                        PMC_SQL_PROVIDERS_FIELD_CLAIM_NUMBER,
                        PMC_SQL_TABLE_PROVIDERS,
                        PMC_SQL_FIELD_ID, provider_p->id );

                // Get the claim number from the provider record
//                provider_p->claimNumber = pmcSqlSelectInt( buf_p, &count );

                sql.Query( buf_p );
                sql.RowGet( );
                provider_p->claimNumber = sql.Int32u( 0 );

                mbLog( "Read provider %d claimNumber %d\n", provider_p->id, provider_p->claimNumber );
                if( sql.RowCount( ) != 1 )
                {
                    mbDlgExclaim( "Error reading provider %ld claim number\n", provider_p->id );
                }
            }
        }
    }

    // Make our own copy of the linked list.  We must sort this list into the
    // ultimate order in which the claims will be generated.  However, it is possible
    // some of the claims may not yet have claim numbers.

    qWalk( claim_p, Claim_q, pmcClaimStruct_p )
    {
        if( claim_p->status == PMC_CLAIM_STATUS_READY )
        {
            // Allocate space for a copy of this claim
            mbCalloc( claimCopy_p, sizeof( pmcClaimStruct_t ) );

            // Copy the contents of the claim into new structure.  The pointers
            // to the allocated memory (e.g., names, etc. should point to the same
            // areas as the original clim structures.
            memcpy( claimCopy_p, claim_p, sizeof( pmcClaimStruct_t ) );

            claimCopy_p->submitStatus = FALSE;

            // Now loop through claim copy queue.
            add = FALSE;

            qWalk( claim2_p, claim2_q, pmcClaimStruct_p )
            {
                if( claimCopy_p->providerId < claim2_p->providerId )
                {
                    add = TRUE;
                    break;
                }

                if( claimCopy_p->providerId > claim2_p->providerId ) continue;

                // At this point it is known that the provider IDs match... sort by claim number
                if( claimCopy_p->claimNumber < claim2_p->claimNumber )
                {
                    add = TRUE;
                    break;
                }

                if( claimCopy_p->claimNumber > claim2_p->claimNumber ) continue;

                // The claim numbers match too
                if( claimCopy_p->claimHeaderId < claim2_p->claimHeaderId )
                {
                    add = TRUE;
                    break;
                }

                // Sanity Check
                if( claimCopy_p->claimHeaderId != claim2_p->claimHeaderId )
                {
                    mbDlgDebug(( "Claim header ids do not match" ));
                }

                // Next check the claimIndex
                if( claimCopy_p->claimIndex < claim2_p->claimIndex )
                {
                    add = TRUE;
                    break;
                }

                if( claimCopy_p->claimIndex > claim2_p->claimIndex ) continue;

                mbDlgExclaim( "Duplicate entires detected.\n" );
            }

            if( add )
            {
                qInsertBefore(  claim2_q, claim2_p, claimCopy_p );
            }
            else
            {
                qInsertLast( claim2_q, claimCopy_p );
            }
            readyCount++;
        }
        else if( claim_p->status == PMC_CLAIM_STATUS_NOT_READY )
        {
            notReadyCount++;
        }
    }
    mbLockRelease( ClaimQueueLock );

    // Next we must assign the claim numbers to the sorted claims

    qWalk( claim_p, claim2_q, pmcClaimStruct_p )
    {
        if( claim_p->claimNumber == PMC_CLAIM_NUMBER_NOT_ASSIGNED )
        {
            Boolean_t    found = FALSE;
            // if there another claim with the same header?? If so and its claim
            // number is set, we must use that claim number, else we must assign
            // a new claim number
            qWalk( claim2_p, claim2_q, pmcClaimStruct_p )
            {
                if( claim2_p != claim_p )
                {
                    if( claim2_p->claimHeaderId == claim_p->claimHeaderId )
                    {
                        if( claim2_p->claimNumber != PMC_CLAIM_NUMBER_NOT_ASSIGNED )
                        {
                            found = TRUE;
                            break;
                        }
                    }
                }
            }

            if( found == TRUE )
            {
                claim_p->claimNumber = claim2_p->claimNumber;
                mbLog( "Assigning existing claim number %d to claim %d\n", claim_p->claimNumber, claim_p->id );
            }
            else
            {
                // OK, don't see any claims with the same claim header.  Must
                // loop through the list of providers and find the appropriate
                // provider

                found = FALSE;

                qWalk( provider_p, provider_q, providerStruct_p )
                {
                    if( claim_p->providerId == provider_p->id )
                    {
                        found = TRUE;
                        break;
                    }
                }

                if( found )
                {
                    provider_p->claimNumber++;

                    // if( provider_p->claimNumber > 15000 && provider_p->claimNumber < 20000 )     // 2013_12_27
                    // if( provider_p->claimNumber > 45000 && provider_p->claimNumber < 50000 )     // 2014_12_29
                    // if( provider_p->claimNumber > 55000 && provider_p->claimNumber < 60000 )     // 2019_02_05
                    if( provider_p->claimNumber > 65000 && provider_p->claimNumber < 70000 )        // 2020_09_20
                    {
                        mbDlgExclaim("Get Mike to increment claim numbers (%ld)!", provider_p->claimNumber );
                    }

                    if( provider_p->claimNumber >= PMC_CLAIM_NUMBER_MAX )
                    {
                        // Do not use the actual max claim number (99999) as it
                        // is used for things like returned message records that
                        // do not belong to any claim
                        mbDlgExclaim( "Claim number wrapped." );
                        provider_p->claimNumber = 10000;
                    }
                    claim_p->claimNumber = provider_p->claimNumber;
                    mbLog( "Assigning new claim number %d to claim %d\n", claim_p->claimNumber, claim_p->id );
                }
                else
                {
                    mbDlgExclaim( "Error locating provider id %d\n", claim_p->providerId );
                }
            }
        }
        mbLog( "Provider %d claim %d index %d claimId %d\n", claim_p->providerId, claim_p->claimNumber, claim_p->claimIndex, claim_p->providerId );
    }

    fp = fopen( pmcClaimsin_p, "w" );
    if( fp == NIL )
    {
        mbDlgExclaim( "Could not open output claims file %s.  Terminating submission.", pmcClaimsin_p );
        goto exit;
    }
    run = TRUE;

    {
        Int32u_t maxThermometerValue;
        if( pmcCfg[CFG_CAN_SUBMIT_FLAG].value )
        {
            maxThermometerValue = (Int32u_t)( claim2_q->size * 2 );
        }
        else
        {
            maxThermometerValue = (Int32u_t)( claim2_q->size );
        }

        sprintf( buf_p, "Processing %ld claims...", claim2_q->size );
        thermometer_p = new TThermometer( buf_p, 0, maxThermometerValue, FALSE );
    }

    // Loop through list of claims
    {
        Int32s_t                doctorNumber = 0;
        Int32u_t                totalRecordCount = 0;
        Int32u_t                feeClaimed = 0;
        Int32u_t                totalFeeClaimed = 0;
        Int32u_t                serviceRecordCount = 0;
        Int32u_t                sequenceNumber = 0;
        Int32u_t                recordsWritten = 0;

        ReciprocalRequired = FALSE;
        ReciprocalClaim_p = NIL;

        qWalk( claim_p, claim2_q, pmcClaimStruct_p )
        {
            if( claim_p->providerId != currentProviderId )
            {
                // A new provider has been detected
                if( currentProviderId )
                {
                    // Previous provider was not 0; must terminate it
                    if( ClaimTrailerRecordWrite( fp, doctorNumber, totalFeeClaimed, serviceRecordCount, totalRecordCount ) != TRUE )
                    {
                        errorCode = TRUE;
                        sprintf( errorString_p, "Error writing trailer record." );
                        break;
                    }
                }

                // Output header for new provider
                currentProviderId = claim_p->providerId;
                doctorNumber = ClaimHeaderWrite( fp, currentProviderId, &recordsWritten, clinicNumber, &sql );
                if( doctorNumber < 0 )
                {
                    errorCode = TRUE;
                    sprintf( errorString_p, "Error writing header record." );
                    break;
                }
                PreviousClaimNumber = 0;
                PreviousSequenceNumber = 0;
                totalRecordCount = recordsWritten;
                serviceRecordCount = 0;
                totalFeeClaimed = 0;
            }

            // Must output a service record
            if( ClaimServiceRecordWrite( fp, claim_p, (Int32u_t)doctorNumber, &sequenceNumber, &recordsWritten, &feeClaimed ) != TRUE )
            {
                errorCode = TRUE;
                sprintf( errorString_p, "Error writing service record." );
                break;
            }
            serviceRecordCount++;
            totalRecordCount += recordsWritten;
            totalFeeClaimed += feeClaimed;
            claim_p->claimSequence = (Int16u_t)sequenceNumber;
            claim_p->submitStatus = TRUE;
            if( totalFeeClaimed > 9999999 )
            {
                errorCode = TRUE;
                sprintf( errorString_p, "Error: total fee exceeds $99999.99" );
                break;
            }
            thermometer_p->Increment( );
        }
        // Finished going through the list

        if( currentProviderId )
        {
            if( ClaimTrailerRecordWrite( fp, doctorNumber, totalFeeClaimed, serviceRecordCount, totalRecordCount ) != TRUE )
            {
                errorCode = TRUE;
                sprintf( errorString_p, "Error writing trailer record." );
            }
        }

        // Close the file
        if( fp )
        {
            fclose( fp );
            fp = NIL;
            if( errorCode == FALSE )
            {
                Int32u_t    mode;

                mode = ( pmcCfg[CFG_CAN_SUBMIT_FLAG].value == TRUE ) ? PMC_CLAIM_FILE_PRINT_MODE_SUBMIT : PMC_CLAIM_FILE_PRINT_MODE_TEST;
                if( pmcClaimsFilePrint( pmcClaimsin_p, archiveFileName_p, mode ) == FALSE )
                {
                    sprintf( errorString_p, "Error printing claims record." );
                    errorCode = TRUE;
                }
            }
        }

        // OK, now update the submit days and sequence numbers if successful
        if( pmcCfg[CFG_CAN_SUBMIT_FLAG].value )
        {
            if( errorCode == FALSE )
            {
                // Everything seems to have gone OK.
                for( ; ; )
                {
                    if( qEmpty( claim2_q ) ) break;

                    claim_p = (pmcClaimStruct_p)qRemoveFirst( claim2_q );

                    // Sanity check
                    if( claim_p->submitStatus != TRUE )
                    {
                        mbDlgDebug(( "Got submitStatus != TRUE" ));
                    }

                    // MAB:20010827:  Add units paid to the claim.  It appears that
                    // MSP will divide a claim with multiple units into various
                    // responses.  For example, a claim with 9 units can be replied
                    // to as 5 units paid and 4 units not paid.

                    //                              today  stat  units num    seq     count
                    sprintf( buf_p, "update %s set %s=%ld,%s=%ld,%s=0,%s=%ld,%s=%ld,%s=%s+1 where %s=%ld",
                        PMC_SQL_TABLE_CLAIMS,
                        PMC_SQL_CLAIMS_FIELD_SUBMIT_DATE, today,
                        PMC_SQL_CLAIMS_FIELD_STATUS, PMC_CLAIM_STATUS_SUBMITTED,
                        PMC_SQL_CLAIMS_FIELD_UNITS_PAID,
                        PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER, claim_p->claimNumber,
                        PMC_SQL_CLAIMS_FIELD_CLAIM_SEQ, claim_p->claimSequence,
                        PMC_SQL_CLAIMS_FIELD_SUB_COUNT, PMC_SQL_CLAIMS_FIELD_SUB_COUNT,
                        PMC_SQL_FIELD_ID, claim_p->id );

                    sql.Update( buf_p );

                    mbLog( "Marking claim %ld-%d as submitted (id: %ld)\n",
                        claim_p->claimNumber, claim_p->claimSequence, claim_p->id );

                    thermometer_p->Increment( );
                    mbFree( claim_p );
                }
                // Now we must update the claim numbers for the providers
                for( ; ; )
                {
                    Int32u_t    test;

                    if( qEmpty( provider_q ) ) break;
                    provider_p = (providerStruct_p)qRemoveFirst( provider_q );

                    // Update the claim number in the provider record
                    sprintf( buf_p, "update %s set %s=%lu where %s=%lu",
                                        PMC_SQL_TABLE_PROVIDERS,
                                        PMC_SQL_PROVIDERS_FIELD_CLAIM_NUMBER, provider_p->claimNumber,
                                        PMC_SQL_FIELD_ID, provider_p->id );

                    sql.Update( buf_p );

                    // Sanity Check
                    sprintf( buf_p, "select %s from %s where %s=%ld",
                                    PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER,
                                    PMC_SQL_TABLE_PROVIDERS,
                                    PMC_SQL_FIELD_ID, provider_p->id );

                    sql.Query( buf_p );
                    sql.RowGet( );
                    test = sql.Int32u( 0 );

                    if( sql.RowCount( ) != 1 )
                    {
                        mbDlgDebug(( "Error: count != 1 in update provider %ld claim number to %ld\n", provider_p->id, provider_p->claimNumber ));
                    }

                    if( test != provider_p->claimNumber )
                    {
                        mbDlgDebug(( "Error: claim number read back (%ld) does not match expected (%ld)\n", test, provider_p->id ));
                    }
                    mbLog( "Setting provider %d claim_number to %d\n", provider_p->id, provider_p->claimNumber );
                    mbFree( provider_p );
                }
            }
        }
    }

    delete thermometer_p;

exit:

    if( tablesLocked )
    {
        sprintf( buf_p, "unlock tables" );
        sql.Update( buf_p );
        pmcSuspendPollDec( );
    }

    if( terminateFlag == TRUE ) mbDlgInfo( "Terminating submission.\n" );

    if( run )
    {
        if( fp ) fclose( fp );

        if( pmcCfg[CFG_CAN_SUBMIT_FLAG].value )
        {
            if( errorCode == TRUE )
            {
                sprintf( buf_p,  "Deleting claims file %s due to the following error:\n\n%s\n",
                    pmcClaimsin_p, errorString_p );
                mbDlgExclaim( buf_p );
                mbLog( buf_p );

                if( !pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value ) unlink( pmcClaimsin_p );
            }
            else
            {
                // Update the claims list
                SubmittedCheckBox->Checked = TRUE;
                ClaimListGet( NIL );
                UpdateStatusLabels( );

                if( mbFileCopy( pmcClaimsin_p, archiveFileName_p ) != MB_RET_OK )
                {
                    sprintf( buf_p, "Claims file %s created.\nError creating archive file.  Contact system administrator.\n", pmcClaimsin_p );
                    mbDlgInfo( buf_p );
                    mbLog( buf_p );
                }
                else
                {
                    if( !pmcMspFileDatabaseAdd( archiveFileName_p, PMC_MSP_FILE_TYPE_CLAIMSIN ) )
                    {
                        mbDlgExclaim( "Error adding file %s to database.\nContact system administrator.", archiveFileName_p );
                    }

                    // Put the archive name into the info file
                    if( ( fp = fopen( pmcInfo_p, "w" ) ) == NIL )
                    {
                        mbDlgExclaim( "Error creating file %s.  Contact system administrator", pmcInfo_p );
                    }
                    else
                    {
                        fprintf( fp, "%s\n", archiveBaseName_p );
                        fclose( fp );

                        mbLog( "Claims file %s created.\n(Archived to %s.)\n", pmcClaimsin_p, archiveFileName_p );

                        if( mbDlgOkCancel(  "The claims file %s succesfully was created.\n"
                                               "Would you like to submit this file to the MSP now?", pmcClaimsin_p ) == MB_BUTTON_OK )
                        {
                            // Attempt to submit the file
                            if( pmcSubmit( ) == FALSE )
                            {
                                mbDlgInfo( "The newly created claims file was not successfully submitted to the MSP.\n"
                                           "You can submit it at any time in the future by clicking the 'Submit' button.\n" );
                            }
                            else
                            {
                                ClaimListGet( NIL );
                                UpdateStatusLabels( );

                                mbDlgInfo( "The newly created claims file was successfully submitted to the MSP.\n" );
                            }
                        }
                        else
                        {
                            mbDlgInfo( "You have chosen not to submit the newly created claims file to the MSP at this time.\n"
                                       "You can submit it at any time in the future by clicking the 'Submit' button.\n" );
                        }
                    }
                }
            }
        }
        else
        {
            unlink( pmcClaimsin_p );
            if( errorCode == TRUE )
            {
                mbDlgExclaim( "The claim submisson test failed due to the following error:\n\n%s\n",  errorString_p );
            }
            else
            {
                mbDlgInfo( "The claim submission test was successful.\n" );
            }
        }
    }

    // Get rid of claim copies
    for( ; ; )
    {
        if( qEmpty( claim2_q ) ) break;
        claim_p = (pmcClaimStruct_p)qRemoveFirst( claim2_q );

        nbDlgDebug(( "Claim number: %ld-%ld-%ld (%s %s)", claim_p->providerId,
            claim_p->claimNumber, claim_p->claimIndex, claim_p->firstName_p, claim_p->lastName_p ));

        mbFree( claim_p );
    }

    for( ; ; )
    {
        if( qEmpty( provider_q ) ) break;
        provider_p = (providerStruct_p)qRemoveFirst( provider_q );
        mbFree( provider_p );
    }

    mbFree( buf_p );
    mbFree( archiveFileName_p );
    mbFree( archiveBaseName_p );
    mbFree( errorString_p );

    Screen->Cursor = cursorOrig;

    return;
}

//---------------------------------------------------------------------------
// Function:  PickupButtonClick()
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::PickupButtonClick(TObject *Sender)
{
    Boolean_t        foundClaimsin = FALSE;
    Boolean_t        foundValidrpt = FALSE;
    Boolean_t        foundNotice = FALSE;
    Boolean_t        foundReturns = FALSE;
    Boolean_t        foundInfo = FALSE;
    Boolean_t        terminateFlag = FALSE;
    Int32s_t    resultCode;

    mbLog( "Claims pickup start\n" );

    pmcMspFilesLocate( &foundClaimsin, &foundInfo, &foundValidrpt, &foundReturns, &foundNotice, TRUE );

    terminateFlag = TRUE;
    if( !pmcCfg[CFG_CAN_SUBMIT_FLAG].value )
    {
        mbDlgInfo( "This computer is not configured to pickup claims files." );
        goto exit;
    }
    else
    {
        // Check for the existance of MSP files
        if( foundValidrpt )
        {
            if( mbDlgOkCancel( "The claims response file %s was detected.\n"
                               "This file must be processed before the pickup can proceed.\n"
                               "Would you like to process this file now?", pmcValidrpt_p ) == MB_BUTTON_CANCEL )
            {
                goto exit;
            }
            else
            {
                if( pmcMspValidrptFileHandle( FALSE ) == FALSE ) goto exit;
            }
        }

        if( foundReturns )
        {
            if( mbDlgOkCancel( "An existing claims returns file %s was detected.\n"
                               "Would you like to process this file before proceeding with the pickup?", pmcReturns_p ) == MB_BUTTON_CANCEL )
            {
                goto exit;
            }
            else
            {
                if( pmcMspReturnsFileHandle( ) == FALSE ) goto exit;

                // Update the display
                ClaimListGet( NIL );
                UpdateStatusLabels( );

                if( mbDlgOkCancel( "The claims returns file %s was successfully processed.\n"
                                   "Proceed with pickup?\n", pmcReturns_p ) == MB_BUTTON_CANCEL )
                {
                    goto exit;
                }
            }
        }

        if( foundNotice )
        {
            if( pmcMspNoticeFileHandle( ) == FALSE ) goto exit;
        }

        if( foundClaimsin )
        {
            // Sanity Check
            if( !foundInfo )
            {
                mbDlgExclaim( "Error: found %s but not %s.\nContact system administrator", pmcClaimsin_p, pmcInfo_p );
                goto exit;
            }

            mbDlgInfo( "The claims submission file %s was detected.\n"
                       "Before proceeding with the pickup, you must submit\n"
                       "this file by clicking the Submit button.", pmcClaimsin_p );
            goto exit;
        }
    }

    // Sanity Check... lets double check file statuses after they have supposedly been procesed
    pmcMspFilesLocate( &foundClaimsin, &foundInfo, &foundValidrpt, &foundReturns, &foundNotice, FALSE );
    if( foundValidrpt || foundReturns )
    {
        mbDlgExclaim( "Existing MSP files detected." );
        goto exit;
    }

#if PMC_MODEM_SUBMIT
    if( mbDlgOkCancel( "About to launch the MSP pickup program.\n"
                        "Ensure the phone line is connected to the modem,\n"
                        "then click OK to proceed, or Cancel to terminate." ) == MB_BUTTON_CANCEL ) goto exit;
#endif

    for( ; ; )
    {

        // Check for these files again within the loop
        if( foundClaimsin )
        {
            mbDlgExclaim( "Error: found %s\nContact system administrator.\n", pmcClaimsin_p );
            goto exit;
        }

        if( foundInfo )
        {
            mbDlgExclaim( "Error: found %s\nContact system administrator.\n", pmcInfo_p );
            goto exit;
        }

        if( foundValidrpt )
        {
            mbDlgExclaim( "Error: found %s\nContact system administrator.\n", pmcValidrpt_p );
            goto exit;
        }

#if PMC_MODEM_SUBMIT
        if( foundReturns )
        {
            mbDlgExclaim( "Error: found %s\nContact system administrator.\n", pmcReturns_p );
            goto exit;
        }

        if( system( "msp.bat" ) == -1 )
        {
            mbDlgDebug(( "system() error %d", errno ));
        }
#else
        if( mbDlgOkCancel( "You must pickup the claims response file via the Internet, and save it in the '%s' directory.\n\n"
           "When this has been done, click OK.\n\nOtherwise, click Cancel.", pmcCfg[CFG_MSP_DIR].str_p ) == MB_BUTTON_CANCEL ) goto exit;
#endif
        // Recheck file statuses
        pmcMspFilesLocate( &foundClaimsin, &foundInfo, &foundValidrpt, &foundReturns, &foundNotice, FALSE );

        if( foundNotice )
        {
            pmcMspNoticeFileHandle( );
        }

        if( foundValidrpt )
        {
            // User could have done a pickup
            if( mbDlgOkCancel( "The claims response file %s was detected.\n"
                               "This file must be processed before proceeding with the submission.\n"
                               "Would you like to process this file now?", pmcValidrpt_p ) == MB_BUTTON_OK )
            {
                resultCode = pmcMspValidrptFileHandle( FALSE );
                if( resultCode == FALSE ) goto exit;

                mbDlgInfo( "The claims file %s was successfully processed.\n", pmcValidrpt_p );
            }
            else
            {
                goto exit;
            }
        }

        if( foundReturns )
        {
            if( mbDlgYesNo( "The claims returns file %s was detected.\n"
                               "Would you like to process this file now?", pmcReturns_p ) != MB_BUTTON_YES )
            {
                goto exit;
            }

            resultCode = pmcMspReturnsFileHandle( );
            ClaimListGet( NIL );
            UpdateStatusLabels( );
            if( resultCode == TRUE )
            {
                mbDlgInfo( "The claims returns file %s was successfully processed.\n", pmcReturns_p );
                terminateFlag = FALSE;
            }
            goto exit;
        }

        // Returns was not detected
        if( mbDlgYesNo( "The claims returns file %s was not detected.\nRetry pickup?", pmcReturns_p ) == MB_BUTTON_NO ) goto exit;
    }

exit:

    if( terminateFlag ) mbDlgInfo( "Terminating pickup." );

    return;
}

//---------------------------------------------------------------------------
// Function:  PrintButtonClick()
//---------------------------------------------------------------------------
// Description:
//
//
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ClaimGridPopupListAppointmentsClick
(
      TObject *Sender
)
{
    pmcClaimStruct_p    claim_p;

    // Sanity Check
    if( MouseInRow > ClaimArraySize )
    {
        mbDlgDebug(( "Error, MouseInRow %ld  > ClaimArraySize %ld", MouseInRow, ClaimArraySize ));
        return;
    }
    claim_p = ClaimArray_pp[MouseInRow - 1];
    pmcViewAppointments( claim_p->patientId, TRUE, FALSE, 0, 0, 0, 0, PMC_LIST_MODE_LIST );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ClaimGridPopupViewExpClick(TObject *Sender)
{
    pmcClaimStruct_p    claim_p;

    claim_p = ClaimArray_pp[MouseInRow - 1];
    pmcExpBox( claim_p->expCode_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ClaimDrawGridClick(TObject *Sender)
{
    pmcClaimStruct_p    claim_p;

    if( MouseInCol == PMC_CLAIM_LIST_COL_EXP_CODE && MouseInRow > 0 && MouseInRow <= ClaimArraySize )
    {
        claim_p = ClaimArray_pp[MouseInRow - 1];
        if( MouseDownExp == TRUE )
        {
            pmcExpBox( claim_p->expCode_p );
            MouseDownExp = FALSE;
        }
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::PrintShortButtonClick(TObject *Sender)
{
    if( ProviderRadioGroup->ItemIndex != 1  )
    {
        mbDlgInfo( "Cannot print claims with 'Providers - All' selected" );
    }
    else
    {
        PrintClaims( FALSE );
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::PrintLongButtonClick(TObject *Sender)
{
    if( ProviderRadioGroup->ItemIndex != 1  )
    {
        mbDlgInfo( "Cannot print claims with 'Providers - All' selected" );
    }
    else
    {
        PrintClaims( TRUE );
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::PrintClaims( Int32u_t longFlag )
{
    Char_p              spoolFileName_p;
    Char_p              flagFileName_p;
    Char_p              fileName_p;
    Char_p              buf1_p, buf2_p, buf3_p;
    FILE               *fp = NIL;
    MbDateTime          dateTime;
    pmcClaimStruct_p    claim_p;
    PmcSqlDoctor_p      doctor_p;
    Boolean_t           statusCount = 0;
    Boolean_t           displayStatus = FALSE;
    Boolean_t           displayProvider = FALSE;
    Boolean_t           displayName = FALSE;
    TThermometer       *thermometer_p = NIL;

    mbMalloc( fileName_p, 256 );
    mbMalloc( spoolFileName_p, 256 );
    mbMalloc( flagFileName_p, 256 );
    mbMalloc( buf1_p, 256 );
    mbMalloc( buf2_p, 256 );
    mbMalloc( buf3_p, 256 );
    mbMalloc( doctor_p, sizeof(PmcSqlDoctor_t) );

    if( ClaimArraySize == 0 )
    {
        mbDlgExclaim( "No claims to print." );
        goto exit;
    }

    if( longFlag && ClaimArraySize > 100 )
    {
        if( mbDlgOkCancel( "Are you sure you want to print %ld claims in long format?\n"
                           "(Could be many pages)", ClaimArraySize ) == MB_BUTTON_CANCEL )
        {
            goto exit;
        }
    }
    else
    {
        if( mbDlgOkCancel( "Print %ld claim%s?", ClaimArraySize, ( ClaimArraySize == 1 ) ? "" : "s" ) == MB_BUTTON_CANCEL )
        {
            goto exit;
        }
    }

    if( ClaimArraySize > 50 )
    {
        sprintf( buf1_p, "Processing %ld claims...", ClaimArraySize );
        thermometer_p = new TThermometer( buf1_p, 0, ClaimArraySize, FALSE );
    }

    mbLockAcquire( ClaimQueueLock );

    sprintf( fileName_p, "%s.html", pmcMakeFileName( NIL, buf1_p ) );
    sprintf( spoolFileName_p, "%s\\spool\\%s", pmcCfg[CFG_PMCDESPL_DIR].str_p, fileName_p );
    sprintf( flagFileName_p,  "%s\\flags\\%s", pmcCfg[CFG_PMCDESPL_DIR].str_p, fileName_p );

    fp = fopen( spoolFileName_p, "w" );
    if( fp == NIL )
    {
        mbDlgExclaim( "Error opening file %s", spoolFileName_p );
        goto exit;
    }

    fprintf( fp, "<HTML><HEAD><Title>Claim List</TITLE></HEAD><BODY>\n" );
    fprintf( fp, "<H2><CENTER>Claim List Report</CENTER></H2><HR>\n" );
    fprintf( fp, "<PRE WIDTH = 80>\n" );

#define FMTSTR "%-35.35s %s\n"

    if( ClaimSearchPageControl->ActivePage == ServiceDatesPage )
    {
        // End date
        dateTime.SetDate( LatestDate );
        fprintf( fp, FMTSTR, "Latest Date:", dateTime.MDY_DateString( ) );

        // Start date
        dateTime.SetDate( EarliestDate );
        fprintf( fp, FMTSTR, "Earliest Date:", dateTime.MDY_DateString( ) );
    }
    else if( ClaimSearchPageControl->ActivePage == SubmitDatePage )
    {
        dateTime.SetDate( SubmitDate );
        fprintf( fp, FMTSTR, "Submit Date:", dateTime.MDY_DateString( ) );
    }
    else if( ClaimSearchPageControl->ActivePage == ReplyDatePage )
    {
        dateTime.SetDate( ReplyDate );
        fprintf( fp, FMTSTR, "Reply Date:", dateTime.MDY_DateString( ) );
    }
    else if( ClaimSearchPageControl->ActivePage == ClaimNumberPage )
    {
        sprintf( buf2_p, "%ld", SearchClaimNumber );
        fprintf( fp, FMTSTR, "Claim Number:  %s", buf2_p );
    }
    else
    {
        fprintf( fp, "ClaimSearchPageControl error\n" );
    }

    // Provider
    if( ProviderRadioGroup->ItemIndex == 1 )
    {
        pmcProviderDescGet( ProviderId, buf1_p );
        fprintf( fp, FMTSTR, "Provider:", buf1_p );
    }
    else
    {
        fprintf( fp, FMTSTR, "Providers:", "All" );
        displayProvider = TRUE;
    }

    // Patient
    if( PatientRadioGroup->ItemIndex == 1 )
    {
        fprintf( fp, FMTSTR, "Patient:", PatientEdit->Text.c_str() );
    }
    else
    {
        fprintf( fp, FMTSTR, "Patients:", "All" );
        displayName = TRUE;
    }

    fprintf( fp, FMTSTR, "Show Claims 'Not Ready':",            ( NotReadyCheckBox->Checked         == TRUE ) ? "Yes" : "No" );
    fprintf( fp, FMTSTR, "Show Claims 'Ready':",                ( ReadyCheckBox->Checked            == TRUE ) ? "Yes" : "No" );
    fprintf( fp, FMTSTR, "Show Claims 'Submitted':",            ( SubmittedCheckBox->Checked        == TRUE ) ? "Yes" : "No" );
    fprintf( fp, FMTSTR, "Show Claims 'Rejected':",             ( RejectedCheckBox->Checked         == TRUE ) ? "Yes" : "No" );
    fprintf( fp, FMTSTR, "Show Claims 'Rejected' (Accepted):",  ( RejectedAcceptCheckBox->Checked   == TRUE ) ? "Yes" : "No" );
    fprintf( fp, FMTSTR, "Show Claims 'Reduced':",              ( ReducedCheckBox->Checked          == TRUE ) ? "Yes" : "No" );
    fprintf( fp, FMTSTR, "Show Claims 'Reduced'  (Accepted):",  ( ReducedAcceptCheckBox->Checked    == TRUE ) ? "Yes" : "No" );
    fprintf( fp, FMTSTR, "Show Claims 'Paid':",                 ( PaidCheckBox->Checked             == TRUE ) ? "Yes" : "No" );

    if( NotReadyCheckBox->Checked       == TRUE ) statusCount++;
    if( ReadyCheckBox->Checked          == TRUE ) statusCount++;
    if( SubmittedCheckBox->Checked      == TRUE ) statusCount++;
    if( RejectedCheckBox->Checked       == TRUE ) statusCount++;
    if( PaidCheckBox->Checked           == TRUE ) statusCount++;
    if( ReducedCheckBox->Checked        == TRUE ) statusCount++;
    if( ReducedAcceptCheckBox->Checked  == TRUE ) statusCount++;
    if( RejectedAcceptCheckBox->Checked == TRUE ) statusCount++;

    if( statusCount > 1 ) displayStatus = TRUE;

    dateTime.SetDateTime( mbToday( ), mbTime( ) );
    sprintf( buf1_p, "%s %s", dateTime.HM_TimeString( ), dateTime.MDY_DateString( ) );
    fprintf( fp, FMTSTR, "Printed:", buf1_p );

    fprintf( fp, "\n\n" );
    if( !longFlag )
    {
        fprintf( fp, "Status Legend:  NR  -  Not Ready\n" );
        fprintf( fp, "                RE  -  Ready\n" );
        fprintf( fp, "                RER -  Ready    (Resubmission)\n" );
        fprintf( fp, "                SB  -  Submitted\n" );
        fprintf( fp, "                RJ  -  Rejected\n" );
        fprintf( fp, "                RJA -  Rejected (Accepted)\n" );
        fprintf( fp, "                RD  -  Reduced\n" );
        fprintf( fp, "                RDA -  Reduced  (Accepted)\n" );
        fprintf( fp, "                PD  -  Paid     (In Full)\n\n\n" );
    }

    if( longFlag )
    {
    fprintf( fp, "   #  Claim    PHN            Claim Details\n" );
    }
    else
    {
    fprintf( fp, "   #  Claim   PHN       YYYY/MM/DD Name Status  IDC Code  # Claimed    Paid Exp\n" );
    }

    fprintf( fp, "________________________________________________________________________________\n\n" );

    for( Ints_t i = 0 ; i < ClaimArraySize ; i++ )
    {
        claim_p = ClaimArray_pp[i];

        // List number
        fprintf( fp, "%4ld: ", i+1 );

        // Claim number
        sprintf( buf1_p, "" );

        if( claim_p->claimNumber != 0 && claim_p->claimNumber != PMC_CLAIM_NUMBER_NOT_ASSIGNED )
        {
            sprintf( buf1_p, "%ld", claim_p->claimNumber );
            if( claim_p->submitDay )
            {
                sprintf( buf2_p, "-%ld", claim_p->claimSequence );
                strcat( buf1_p, buf2_p );
            }
        }
        if( longFlag )
        {
            fprintf( fp, "%-7.7s  ", buf1_p );
        }
        else
        {
            fprintf( fp, "%-7.7s ", buf1_p );
        }

        // PHN
        if( strlen( claim_p->phn_p ) != 0 )
        {
            sprintf( buf1_p, "%s", claim_p->phn_p );
        }
        else
        {
            sprintf( buf1_p, "--- --- ---" );
        }
        if( longFlag )
        {
            fprintf( fp, "%-12.12s  ", buf1_p );
        }
        else
        {
             fprintf( fp, "%-9.9s ", mbStrDigitsOnly( buf1_p ) );
        }

        // Date of service
        dateTime.SetDate( claim_p->serviceDay );
        if( longFlag )
        {
        fprintf( fp, " Date:       %-12.12s", dateTime.YMD_DateString( ) );
        }
        else
        {
        fprintf( fp, "%s ", dateTime.DigitsDateString( ) );
        }

        if( longFlag )
        {
            // Name
            if( displayName )
            {
                sprintf( buf1_p, "%s, %s", claim_p->lastName_p, claim_p->firstName_p );
                fprintf( fp, "%26.26s", buf1_p );
            }
        }
        else
        {
            fprintf( fp, "%-8.8s ", claim_p->lastName_p );
        }

        if( longFlag ) fprintf( fp, "\n" );

        if( longFlag )
        {
            if( displayStatus )
            {
            switch( claim_p->status )
            {
                case PMC_CLAIM_STATUS_NOT_READY:        { sprintf( buf1_p, "Not Ready" );           break; }
                case PMC_CLAIM_STATUS_READY:
                    if( claim_p->replyDay )
                    {
                        sprintf( buf1_p, "Ready (Resub)" );
                    }
                    else
                    {
                        sprintf( buf1_p, "Ready" );
                    }
                    break;
                case PMC_CLAIM_STATUS_SUBMITTED:        { sprintf( buf1_p, "Submitted" );           break; }
                case PMC_CLAIM_STATUS_PAID:             { sprintf( buf1_p, "Paid (In Full)" );      break; }
                case PMC_CLAIM_STATUS_REJECTED:         { sprintf( buf1_p, "Rejected (Active)" );   break; }
                case PMC_CLAIM_STATUS_REDUCED:          { sprintf( buf1_p, "Reduced (Active)" );    break; }
                case PMC_CLAIM_STATUS_REJECTED_ACCEPT:  { sprintf( buf1_p, "Rejected (Accepted)" ); break; }
                case PMC_CLAIM_STATUS_REDUCED_ACCEPT:   { sprintf( buf1_p, "Reduced (Accepted)" );   break; }
                case PMC_CLAIM_STATUS_EDIT:             { sprintf( buf1_p, "Edit" );                break; }
                default:                                { sprintf( buf1_p, "Invalid" );             break; }
            }
            fprintf( fp, "                              Status:     %s\n", buf1_p );
            }
        }
        else
        {
            // Short form
            switch( claim_p->status )
            {
                case PMC_CLAIM_STATUS_NOT_READY:        { sprintf( buf1_p, "NR " ); break; }
                case PMC_CLAIM_STATUS_READY:
                    if( claim_p->replyDay )
                    {
                        sprintf( buf1_p, "RER" );
                    }
                    else
                    {
                        sprintf( buf1_p, "RE " );
                    }
                    break;
                case PMC_CLAIM_STATUS_SUBMITTED:        { sprintf( buf1_p, "SB " ); break; }
                case PMC_CLAIM_STATUS_PAID:             { sprintf( buf1_p, "PD " ); break; }
                case PMC_CLAIM_STATUS_REJECTED:         { sprintf( buf1_p, "RJ " ); break; }
                case PMC_CLAIM_STATUS_REDUCED:          { sprintf( buf1_p, "RD " ); break; }
                case PMC_CLAIM_STATUS_REJECTED_ACCEPT:  { sprintf( buf1_p, "RJA" ); break; }
                case PMC_CLAIM_STATUS_REDUCED_ACCEPT:   { sprintf( buf1_p, "RDA" ); break; }
                case PMC_CLAIM_STATUS_EDIT:             { sprintf( buf1_p, "ED " ); break; }
                default:                                { sprintf( buf1_p, "IN " ); break; }
            }
            fprintf( fp, "%s ", buf1_p );
        }

        if( longFlag )
        {
            if( claim_p->submitDay )
            {
            dateTime.SetDate( claim_p->submitDay );
            fprintf( fp, "                              Submitted:  %-12.12s\n", dateTime.YMD_DateString( ) );
            }
        }

        if( longFlag )
        {
            Boolean_t failed = TRUE;
            if( claim_p->referringNumber )
            {
                if( pmcSqlDoctorDetailsGet( claim_p->referringId, doctor_p ) )
                {
            fprintf( fp, "                              Ref. Dr:    %s %s (%ld)\n",  doctor_p->firstName, doctor_p->lastName, claim_p->referringNumber );
            failed = FALSE;
                }
            }
            if( failed )
            {
            fprintf( fp, "                              Ref. Dr:    %ld\n",  claim_p->referringNumber );
            }
        }

        if( longFlag )
        {
            if( displayProvider )
            {
            fprintf( fp, "                              Provider:   %s\n", claim_p->providerName_p );
            }
        }

        if( longFlag )
        {
            if( strlen( claim_p->comment_p ) > 0 )
            {
            fprintf( fp, "                              Comment:    %-77.77s\n", claim_p->comment_p );
            }
        }

        if( longFlag )
        {
        fprintf( fp, "                              ICD Code:   %3.3s\n", claim_p->icdCode_p  );
        }
        else
        {
        fprintf( fp, "%3.3s ", claim_p->icdCode_p );
        }

        // Amount claimed
        mbDollarStrInt32u( claim_p->feeSubmitted, buf1_p );

        pmcFeeCodeFormatDisplay( claim_p->feeCode_p, buf2_p );
        if( longFlag )
        {
        fprintf( fp, "                              Fee Code:   %-4.4s        Units: %2ld Claimed: %7.7s\n",
            buf2_p, claim_p->units, buf1_p );
        }
        else
        {
        fprintf( fp, "%4.4s %2ld %7.7s ", buf2_p, claim_p->units, buf1_p  );
        }

        if( claim_p->replyDay )
        {
            if( longFlag )
            {
                dateTime.SetDate( claim_p->replyDay );
                sprintf( buf2_p, "%s",  ( claim_p->expCode_p ) ? claim_p->expCode_p : "" );
                sprintf( buf3_p, "%6.2f", (float)claim_p->feePaid / 100.0 );
                fprintf( fp, "                              Reply:      %-12.12s  Exp: %2.2s    Paid: %7.7s\n",
                    dateTime.YMD_DateString( ),  buf2_p, buf3_p );
            }
            else
            {
                sprintf( buf2_p, "%s",  ( claim_p->expCode_p ) ? claim_p->expCode_p : "" );
                sprintf( buf3_p, "%6.2f", (float)claim_p->feePaid / 100.0 );
                fprintf( fp, "%7.7s %2.2s", buf3_p, buf2_p );
            }
        }

        // End of line
        fprintf( fp, "\n" );

        if( thermometer_p ) thermometer_p->Set( i );
    }

    if( thermometer_p ) delete thermometer_p;

    fprintf( fp, "________________________________________________________________________________\n\n" );

    fprintf( fp, "Claim Type         Number     Claimed         Paid \n\n" );
    fprintf( fp, "Not Ready:         %6ld  %10.2f\n",  NotReadyCount, (double)NotReadyFeeClaimed/100.0 );
    fprintf( fp, "Ready:             %6ld  %10.2f\n",  ReadyCount,    (double)ReadyFeeClaimed/100.0 );
    fprintf( fp, "Submitted:         %6ld  %10.2f\n",  SubmittedCount,(double)SubmittedFeeClaimed/100.0 );
    fprintf( fp, "Rejected:          %6ld  %10.2f\n",  RejectedCount, (double)RejectedFeeClaimed/100.0 );
    fprintf( fp, "Rejected Accepted: %6ld  %10.2f\n",  RejectedAcceptCount, (double)RejectedAcceptFeeClaimed/100.0 );
    fprintf( fp, "Reduced:           %6ld  %10.2f   %10.2f\n",  ReducedCount,  (double)ReducedFeeClaimed/100.0, (double)ReducedFeePaid/100.0 );
    fprintf( fp, "Reduced Accepted:  %6ld  %10.2f   %10.2f\n",  ReducedAcceptCount,  (double)ReducedAcceptFeeClaimed/100.0, (double)ReducedAcceptFeePaid/100.0 );
    fprintf( fp, "Paid:              %6ld  %10.2f   %10.2f\n",  PaidCount,     (double)PaidFeeClaimed/100.0,    (double)PaidFeePaid/100.0    );

    {
        Int32u_t    total;

        total = NotReadyCount + ReadyCount + SubmittedCount +
                RejectedCount + ReducedCount + PaidCount +
                RejectedAcceptCount + ReducedAcceptCount;

        fprintf( fp, "\nTotals:        %10ld",  total );

        total = ReadyFeeClaimed + NotReadyFeeClaimed + SubmittedFeeClaimed +
                RejectedFeeClaimed + PaidFeeClaimed + ReducedFeeClaimed +
                ReducedAcceptFeeClaimed + RejectedAcceptFeeClaimed;

        fprintf( fp, "  %10.2f", (double)total / 100.0 );

        total = PaidFeePaid + ReducedFeePaid + ReducedAcceptFeePaid;

        fprintf( fp, "   %10.2f\n", (double)total / 100.0 );
    }
    fprintf( fp, "\n" );
    fprintf( fp, "</PRE><HR>\n\n" );
    PMC_REPORT_FOOTER( fp );

    // End of document
    fprintf( fp, "</BODY></HTML>\n" );

    if( fp ) fclose( fp );

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
        mbLog( "Printed claim list" );
    }

    mbLockRelease( ClaimQueueLock );

    mbDlgInfo( "Claim list sent to printer." );

exit:
    mbFree( spoolFileName_p );
    mbFree( flagFileName_p );
    mbFree( fileName_p );
    mbFree( doctor_p );
    mbFree( buf1_p );
    mbFree( buf2_p );
    mbFree( buf3_p );
    return;
}

//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ClaimDrawGridDblClick(TObject *Sender)
{
    ClaimEdit( );
}

//---------------------------------------------------------------------------
// Function:  ClaimEdit
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ClaimEdit( void )
{
    pmcClaimStruct_p    claim_p;
    pmcClaimEditInfo_t  claimEditInfo;
    TClaimEditForm     *claimEditForm;

    // Sanity Check
    if( MouseInRow > ClaimArraySize )
    {
        mbDlgDebug(( "Error, MouseInRow %ld  > ClaimArraySize %ld", MouseInRow, ClaimArraySize ));
        return;
    }

    if( MouseInRow < 1 ) return;

    nbDlgDebug(( "Mouse in row: %ld (%ld(, claimNumber: %ld, providerId: %ld",
        MouseInRow, ClaimDrawGrid->Row, claim_p->claimNumber, claim_p->providerId ));

    claim_p = ClaimArray_pp[MouseInRow - 1];

    claimEditInfo.providerId = claim_p->providerId;
    claimEditInfo.patientId = claim_p->patientId;
    claimEditInfo.mode = PMC_CLAIM_EDIT_MODE_EDIT;
    claimEditInfo.claimNumber = claim_p->claimNumber;
    claimEditInfo.claimHeaderId = claim_p->claimHeaderId;

    claimEditForm = new TClaimEditForm( this, &claimEditInfo );
    claimEditForm->ShowModal();
    delete claimEditForm;

    if( claimEditInfo.returnCode == MB_BUTTON_CANCEL )
    {
        if( claimEditInfo.updateClaims == TRUE ) ClaimListGet( NIL );
    }

    if( claimEditInfo.returnCode == MB_BUTTON_OK )
    {
        // Reread the claim list after used cliked OK on new form
        ClaimListGet( NIL );
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ClaimNumberEditExit(TObject *Sender)
{
    SearchClaimNumber = atol( ClaimNumberEdit->Text.c_str() );
    if( SearchClaimNumber != 0 )
    {
        ClaimListGet( NIL );
        ClaimDrawGrid->SetFocus( );
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ClaimNumberEditKeyPress(TObject *Sender, char &Key)
{
    if( Key == 0x0D )
    {
        SearchClaimNumber = atol( ClaimNumberEdit->Text.c_str() );
        if( SearchClaimNumber != 0 )
        {
            ClaimListGet( NIL );
            ClaimDrawGrid->SetFocus( );
        }
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ClaimSearchPageControlChange(TObject *Sender)
{
    SearchClaimNumber = atol( ClaimNumberEdit->Text.c_str() );
    Boolean_t    search = FALSE;

    if( ClaimSearchPageControl->ActivePage == ServiceDatesPage ) search = TRUE;
    if( ClaimSearchPageControl->ActivePage == SubmitDatePage )   search = TRUE;
    if( ClaimSearchPageControl->ActivePage == ReplyDatePage )    search = TRUE;

    if( ClaimSearchPageControl->ActivePage == ClaimNumberPage )
    {
        SearchClaimNumber = atol( ClaimNumberEdit->Text.c_str() );
        if( SearchClaimNumber != 0 )
        {
            search = TRUE;
        }
        else
        {
            // Get cursor into this window
            ClaimNumberEdit->SetFocus( );
        }
    }

    if( search )
    {
        ClaimListGet( NIL );
        ClaimDrawGrid->SetFocus( );
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::StatusLabel0Click (TObject *Sender){ StatusLabelClick(  0 ); }
void __fastcall TClaimListForm::StatusLabel1Click (TObject *Sender){ StatusLabelClick(  1 ); }
void __fastcall TClaimListForm::StatusLabel2Click (TObject *Sender){ StatusLabelClick(  2 ); }
void __fastcall TClaimListForm::StatusLabel3Click (TObject *Sender){ StatusLabelClick(  3 ); }
void __fastcall TClaimListForm::StatusLabel4Click (TObject *Sender){ StatusLabelClick(  4 ); }
void __fastcall TClaimListForm::StatusLabel5Click (TObject *Sender){ StatusLabelClick(  5 ); }
void __fastcall TClaimListForm::StatusLabel6Click (TObject *Sender){ StatusLabelClick(  6 ); }
void __fastcall TClaimListForm::StatusLabel7Click (TObject *Sender){ StatusLabelClick(  7 ); }
void __fastcall TClaimListForm::StatusLabel8Click (TObject *Sender){ StatusLabelClick(  8 ); }
void __fastcall TClaimListForm::StatusLabel9Click (TObject *Sender){ StatusLabelClick(  9 ); }
void __fastcall TClaimListForm::StatusLabel10Click(TObject *Sender){ StatusLabelClick( 10 ); }
void __fastcall TClaimListForm::StatusLabel11Click(TObject *Sender){ StatusLabelClick( 11 ); }
void __fastcall TClaimListForm::StatusLabel12Click(TObject *Sender){ StatusLabelClick( 12 ); }
void __fastcall TClaimListForm::StatusLabel13Click(TObject *Sender){ StatusLabelClick( 13 ); }
void __fastcall TClaimListForm::StatusLabel14Click(TObject *Sender){ StatusLabelClick( 14 ); }
void __fastcall TClaimListForm::StatusLabel15Click(TObject *Sender){ StatusLabelClick( 15 ); }
void __fastcall TClaimListForm::StatusLabel16Click(TObject *Sender){ StatusLabelClick( 16 ); }
void __fastcall TClaimListForm::StatusLabel17Click(TObject *Sender){ StatusLabelClick( 17 ); }

void __fastcall TClaimListForm::StatusLabelClick( Ints_t index )
{
    // mbDlgDebug(( "click index %d\n", index ));
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::StatusLabel0DblClick( TObject *Sender){ StatusLabelDblClick(  0 ); }
void __fastcall TClaimListForm::StatusLabel1DblClick( TObject *Sender){ StatusLabelDblClick(  1 ); }
void __fastcall TClaimListForm::StatusLabel2DblClick( TObject *Sender){ StatusLabelDblClick(  2 ); }
void __fastcall TClaimListForm::StatusLabel3DblClick( TObject *Sender){ StatusLabelDblClick(  3 ); }
void __fastcall TClaimListForm::StatusLabel4DblClick( TObject *Sender){ StatusLabelDblClick(  4 ); }
void __fastcall TClaimListForm::StatusLabel5DblClick( TObject *Sender){ StatusLabelDblClick(  5 ); }
void __fastcall TClaimListForm::StatusLabel6DblClick( TObject *Sender){ StatusLabelDblClick(  6 ); }
void __fastcall TClaimListForm::StatusLabel7DblClick( TObject *Sender){ StatusLabelDblClick(  7 ); }
void __fastcall TClaimListForm::StatusLabel8DblClick( TObject *Sender){ StatusLabelDblClick(  8 ); }
void __fastcall TClaimListForm::StatusLabel9DblClick( TObject *Sender){ StatusLabelDblClick(  9 ); }
void __fastcall TClaimListForm::StatusLabel10DblClick(TObject *Sender){ StatusLabelDblClick( 10 ); }
void __fastcall TClaimListForm::StatusLabel11DblClick(TObject *Sender){ StatusLabelDblClick( 11 ); }
void __fastcall TClaimListForm::StatusLabel12DblClick(TObject *Sender){ StatusLabelDblClick( 12 ); }
void __fastcall TClaimListForm::StatusLabel13DblClick(TObject *Sender){ StatusLabelDblClick( 13 ); }
void __fastcall TClaimListForm::StatusLabel14DblClick(TObject *Sender){ StatusLabelDblClick( 14 ); }
void __fastcall TClaimListForm::StatusLabel15DblClick(TObject *Sender){ StatusLabelDblClick( 15 ); }
void __fastcall TClaimListForm::StatusLabel16DblClick(TObject *Sender){ StatusLabelDblClick( 16 ); }
void __fastcall TClaimListForm::StatusLabel17DblClick(TObject *Sender){ StatusLabelDblClick( 17 ); }

void __fastcall TClaimListForm::StatusLabelDblClick( Ints_t   index )
{
    Ints_t    claimIndex;

    claimIndex = ClaimDrawGrid->TopRow - 1 + index;
    if( claimIndex < ClaimArraySize )
    {
        ClaimEdit( );
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::StatusLabel0MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y){ StatusLabelMouseDown(  0 ); }
void __fastcall TClaimListForm::StatusLabel1MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y){ StatusLabelMouseDown(  1 ); }
void __fastcall TClaimListForm::StatusLabel2MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y){ StatusLabelMouseDown(  2 ); }
void __fastcall TClaimListForm::StatusLabel3MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y){ StatusLabelMouseDown(  3 ); }
void __fastcall TClaimListForm::StatusLabel4MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y){ StatusLabelMouseDown(  4 ); }
void __fastcall TClaimListForm::StatusLabel5MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y){ StatusLabelMouseDown(  5 ); }
void __fastcall TClaimListForm::StatusLabel6MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y){ StatusLabelMouseDown(  6 ); }
void __fastcall TClaimListForm::StatusLabel7MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y){ StatusLabelMouseDown(  7 ); }
void __fastcall TClaimListForm::StatusLabel8MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y){ StatusLabelMouseDown(  8 ); }
void __fastcall TClaimListForm::StatusLabel9MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y){ StatusLabelMouseDown(  9 ); }
void __fastcall TClaimListForm::StatusLabel10MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y){ StatusLabelMouseDown( 10 ); }
void __fastcall TClaimListForm::StatusLabel11MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y){ StatusLabelMouseDown( 11 ); }
void __fastcall TClaimListForm::StatusLabel12MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y){ StatusLabelMouseDown( 12 ); }
void __fastcall TClaimListForm::StatusLabel13MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y){ StatusLabelMouseDown( 13 ); }
void __fastcall TClaimListForm::StatusLabel14MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y){ StatusLabelMouseDown( 14 ); }
void __fastcall TClaimListForm::StatusLabel15MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y){ StatusLabelMouseDown( 15 ); }
void __fastcall TClaimListForm::StatusLabel16MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y){ StatusLabelMouseDown( 16 ); }
void __fastcall TClaimListForm::StatusLabel17MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y){ StatusLabelMouseDown( 17 ); }

void __fastcall TClaimListForm::StatusLabelMouseDown( Ints_t index )
{
    Ints_t    topRow, newRow;

    topRow = ClaimDrawGrid->TopRow;
    newRow = topRow + index;
    if( newRow < ClaimDrawGrid->RowCount )
    {
        ClaimDrawGrid->Row = newRow;
        MouseInRow = newRow;
    }
    else
    {
        MouseInRow = 0;
    }
    nbDlgDebug(( "Top Row: %ld Row: %ld Mouse in Row %ld\n", ClaimDrawGrid->TopRow, ClaimDrawGrid->Row , MouseInRow ));
}

//---------------------------------------------------------------------------
// Function: pmcMspFilesLocate
//---------------------------------------------------------------------------
// Description:
//
// This function checks for the existance of MSP files
//---------------------------------------------------------------------------

Int32s_t    pmcMspFilesLocate
(
    Boolean_t    *claimsin_p,
    Boolean_t    *info_p,
    Boolean_t    *validrpt_p,
    Boolean_t    *returns_p,
    Boolean_t    *notice_p,
    Boolean_t    logFlag
)
{
    Char_p       dir_p;
    Boolean_t    foundValidrpt;
    Boolean_t    foundClaimsin;
    Boolean_t    foundReturns;
    Boolean_t    foundInfo;
    Boolean_t    foundNotice;

    mbMalloc( dir_p, 128 );

    // Format the directory
    sprintf( dir_p, "%s", ( pmcCfg[CFG_MSP_DIR].str_p ) ? pmcCfg[CFG_MSP_DIR].str_p : "" );
    if( strlen( dir_p ) ) strcat( dir_p, "\\" );

    // Format the actual filenames
    sprintf( pmcInfo_p,     "%s%s", dir_p, PMC_MSP_FILENAME_INFO     );
    sprintf( pmcClaimsin_p, "%s%s", dir_p, PMC_MSP_FILENAME_CLAIMSIN );
    sprintf( pmcValidrpt_p, "%s%s", dir_p, PMC_MSP_FILENAME_VALIDRPT );
    sprintf( pmcReturns_p,  "%s%s", dir_p, PMC_MSP_FILENAME_RETURNS  );
    sprintf( pmcNotice_p,   "%s%s", dir_p, PMC_MSP_FILENAME_NOTICE   );

    // Figure out what files are present
    foundValidrpt  = ( access( pmcValidrpt_p, 0 ) == 0 ) ? TRUE : FALSE;
    foundClaimsin  = ( access( pmcClaimsin_p, 0 ) == 0 ) ? TRUE : FALSE;
    foundReturns   = ( access( pmcReturns_p, 0 )  == 0 ) ? TRUE : FALSE;
    foundNotice    = ( access( pmcNotice_p, 0 )   == 0 ) ? TRUE : FALSE;
    foundInfo      = ( access( pmcInfo_p, 0 )     == 0 ) ? TRUE : FALSE;

    // It looks like if a submission is done, and there is nothing to submit,
    // MSP will return a VALIDRPT file.  If this file is present, and
    // CLAIMSIN_INFO is not (i.e., the VALIDRPT does not appear to be in
    // response to a submision) then check if the VALIDRPT file can be
    // tossed.

    if( foundValidrpt == TRUE && foundInfo == FALSE )
    {
        if( pmcMspValidrptFileCheckEmpty( pmcValidrpt_p ) == TRUE )
        {
            mbLog( "deleteing empty VALIDRPT file\n" );
            unlink( pmcValidrpt_p );
            foundValidrpt = FALSE;
        }
    }

    // Log present files
    if( logFlag )
    {
        mbLog( "%s:%s\n", pmcClaimsin_p, foundClaimsin ? "TRUE" : "FALSE" );
        mbLog( "%s:%s\n", pmcInfo_p,     foundInfo     ? "TRUE" : "FALSE" );
        mbLog( "%s:%s\n", pmcValidrpt_p, foundValidrpt ? "TRUE" : "FALSE" );
        mbLog( "%s:%s\n", pmcReturns_p,  foundReturns  ? "TRUE" : "FALSE" );
        mbLog( "%s:%s\n", pmcNotice_p,   foundNotice   ? "TRUE" : "FALSE" );
    }

    if( validrpt_p  ) *validrpt_p   =  foundValidrpt;
    if( claimsin_p  ) *claimsin_p   =  foundClaimsin;
    if( returns_p   ) *returns_p    =  foundReturns;
    if( notice_p    ) *notice_p     =  foundNotice;
    if( info_p      ) *info_p       =  foundInfo;

    mbFree( dir_p );
    return TRUE;
}

//---------------------------------------------------------------------------
// Function: pmcLocateMspFiles
//---------------------------------------------------------------------------
// Description:
//
// This function checks for the existance of MSP files
//---------------------------------------------------------------------------

Int32s_t pmcSubmit( void )
{
    Boolean_t    foundClaimsin = FALSE;
    Boolean_t    foundValidrpt = FALSE;
    Boolean_t    foundNotice = FALSE;
    Boolean_t    foundReturns = FALSE;
    Boolean_t    foundInfo = FALSE;
    Int32s_t     returnCode = FALSE;

    pmcMspFilesLocate( &foundClaimsin, &foundInfo, &foundValidrpt, &foundReturns, &foundNotice, FALSE );

    // Sanity checks
    if( !foundInfo )
    {
        mbDlgExclaim( "Error: could not find %s\nContact system administrator.", pmcInfo_p );
        goto exit;
    }

    if( foundNotice )
    {
        pmcMspNoticeFileHandle( );
    }

    if( foundValidrpt )
    {
        mbDlgExclaim( "Error: found %s\nContact system administrator.", pmcValidrpt_p );
        goto exit;
    }

    if( foundReturns )
    {
        mbDlgExclaim( "Error: found %s\nContact system administrator.", pmcReturns_p );
        goto exit;
    }

    if( !foundClaimsin )
    {
        mbDlgExclaim( "Error: cound not find %s\nContact system administrator.\n", pmcClaimsin_p );
        goto exit;
    }

#if PMC_MODEM_SUBMIT
    if( mbDlgOkCancel( "About to launch the MSP submission program.\n"
                       "Ensure the phone line is connected to the modem,\n"
                       "then click OK to proceed, or Cancel to terminate." ) == MB_BUTTON_CANCEL ) goto exit;
#else
    {
        Char_t      buf[4096];

        buf[0] = 0;
        pmcClaimsFileInfoGetString( pmcClaimsin_p, buf );
        //mbDlgInfo( buf );

        if( mbDlgOkCancel( "The file '%s' must be submitted via the Internet. File Details:\n\n%s\n\n"
                       "Once the file has successfully been submitted, click OK.\n"
                       "The submission website must confirm the details shown above.\n\n"
                       "If the file was not submitted, click Cancel. (It can be submitted later.)\n\n"
                       "IMPORTANT: If OK is clicked and the file was not submitted, the claims will be lost!",
                       pmcClaimsin_p, buf ) == MB_BUTTON_CANCEL )
        {
            mbDlgInfo( "You can attempt to submit the CLAIMSIN file at a later time." );
            goto exit;
        }

        // At this point the user clicked OK... must assume the file was successfully summitted"
        // mbDlgInfo( " At this point the user clicked OK... must assume the file was successfully summitted" );
        returnCode = pmcMspValidrptFileHandle( TRUE );
    }
#endif

#if PMC_MODEM_SUBMIT
    for( ; ; )
    {
        // Check for these files again within the loop
        if( !foundClaimsin )
        {
            mbDlgExclaim( "Error: cound not find %s\nContact system administrator.\n", pmcClaimsin_p );
            goto exit;
        }

        if( !foundInfo )
        {
            mbDlgExclaim( "Error: cound not find %s\nContact system administrator.\n", pmcInfo_p );
            goto exit;
        }


        if( system( "msp.bat" ) == -1 )
        {
            mbDlgDebug(( "system() error %d", errno ));
        }

        // Recheck file statuses
        pmcMspFilesLocate( &foundClaimsin, &foundInfo, &foundValidrpt, &foundReturns, &foundNotice, FALSE );

        if( foundNotice )
        {
            pmcMspNoticeFileHandle( );
        }

        if( foundReturns )
        {
            // User could have done a pickup
            if( mbDlgOkCancel( "The MSP response file %s was detected.\n"
                               "This file must be processed before proceeding with the submission.\n"
                               "Would you like to process this file now?", pmcReturns_p ) == MB_BUTTON_OK )
            {
                returnCode = pmcMspReturnsFileHandle( );
                if( returnCode == FALSE ) goto exit;

                mbDlgInfo( "The MSP response file %s was successfully processed.\n", pmcReturns_p );
                returnCode = FALSE;
            }
            else
            {
                goto exit;
            }
        }

        if( foundValidrpt )
        {
            mbDlgInfo( "The MSP response file %s was detected. Click OK to begin processing this file.", pmcValidrpt_p );
            returnCode = pmcMspValidrptFileHandle( );

            goto exit;
        }

        // Validrpt was not detected
        if( mbDlgOkCancel( "The MSP response file %s was not detected.\nRetry submission?", pmcValidrpt_p ) == MB_BUTTON_CANCEL ) goto exit;
    }
#endif

exit:

   return returnCode;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::PatientListButtonClick(TObject *Sender)
{
    TPatientListForm   *patListForm_p;
    PmcPatListInfo_t    patListInfo;

    patListInfo.patientId = 0;
    patListInfo.providerId = 0;
    patListInfo.mode = PMC_LIST_MODE_LIST;
    patListInfo.allowGoto = FALSE;

    patListForm_p = new TPatientListForm( this, &patListInfo );
    patListForm_p->ShowModal( );
    delete patListForm_p;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::DoctorListButtonClick(TObject *Sender)
{
    TDoctorListForm    *docListForm_p;
    pmcDocListInfo_t    docListInfo;

    docListInfo.doctorId = 0;
    docListInfo.mode = PMC_LIST_MODE_LIST;

    docListForm_p = new TDoctorListForm( this, &docListInfo );
    docListForm_p->ShowModal( );
    delete docListForm_p;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::PatientEditChange(TObject *Sender)
{
    PatientEditUpdate( PatientId );
}
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::PatientEditMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    PatientSelect( 0 );
}
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::LatestDateEditMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    LatestDateButtonClick( Sender );
}
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::EarliestDateEditMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    EarliestDateButtonClick( Sender );
}
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::SubmitDateEditMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    SubmitDateButtonClick( Sender );
}
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::ReplyDateEditMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    ReplyDateButtonClick( Sender );
}
//---------------------------------------------------------------------------

void __fastcall TClaimListForm::PatientEditKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{
    PatientSelect( (Int32u_t)Key );
}
//---------------------------------------------------------------------------



void __fastcall TClaimListForm::ManuallyReconcile1Click(TObject *Sender)
{
    mbDlgInfo( "Manually reconcile clicked" );
}
//---------------------------------------------------------------------------

