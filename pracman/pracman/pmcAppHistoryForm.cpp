//---------------------------------------------------------------------------
// File:    pmcAppHistoryForm.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    July 13, 2002
//---------------------------------------------------------------------------
// Description:
//
// Functions for tracking appointment history
//---------------------------------------------------------------------------

#include <stdio.h>
#include <vcl.h>
#include <time.h>
#pragma hdrstop

#include "mbUtils.h"
#include "pmcMainForm.h"
#include "pmcGlobals.h"
#include "pmcUtils.h"
#include "pmcTables.h"
#include "pmcPatientEditForm.h"

#define PMC_INIT_GLOBALS
#include "pmcAppHistoryForm.h"
#undef PMC_INIT_GLOBALS

#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
// Function:    TAppHistoryForm
//---------------------------------------------------------------------------
// Description: Default constructor
//---------------------------------------------------------------------------

__fastcall TAppHistoryForm::TAppHistoryForm(TComponent* Owner)
    : TForm(Owner)
{
}

//---------------------------------------------------------------------------
// Function:    TAppHistoryForm
//---------------------------------------------------------------------------
// Description: Constructor
//---------------------------------------------------------------------------

__fastcall TAppHistoryForm::TAppHistoryForm(TComponent* Owner, Int32u_t appId )
    : TForm(Owner)
{
    List_q = qInitialize( &ListHead );
    mbLockInit( ListLock );

    AppId = appId;
    PatientId = 0;

    UpdateInfo( appId );
    ListGet( appId );
}

//---------------------------------------------------------------------------
// Function:    ListGet
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------
void __fastcall TAppHistoryForm::ListGet( Int32u_t appId )
{
    Char_p          buf_p;
    Boolean_t       added;
    Boolean_t       gotLock = FALSE;
    TListItem      *item_p;
    HistList_p      entry_p;
    HistList_p      entry2_p;
    MbDateTime      dateTime;
    MbSQL           sql;

    if( mbMalloc( buf_p, 2048 ) == NIL ) goto exit;

    // Free any entries in the list
    ListFree( );

    mbLockAcquire( ListLock );
    gotLock = TRUE;

    //                       0  1  2  3  4  5  6  7  8  9
    sprintf( buf_p, "select %s,%s,%s,%s,%s,%s,%s,%s,%s,%s from %s where %s=%ld",
        PMC_SQL_FIELD_ID,                       // 0
        PMC_SQL_FIELD_DATE,                     // 1
        PMC_SQL_FIELD_TIME,                     // 2
        PMC_SQL_APP_HIST_FIELD_ACTION,          // 3
        PMC_SQL_APP_HIST_FIELD_INT_1,           // 4
        PMC_SQL_APP_HIST_FIELD_INT_2,           // 5
        PMC_SQL_APP_HIST_FIELD_INT_3,           // 6
        PMC_SQL_APP_HIST_FIELD_INT_4,           // 7
        PMC_SQL_APP_HIST_FIELD_INT_5,           // 8
        PMC_SQL_APP_HIST_FIELD_STRING_1,        // 9

        PMC_SQL_TABLE_APP_HIST,
        PMC_SQL_APP_HIST_FIELD_APP_ID, appId );

    if( sql.Query( buf_p ) == FALSE ) goto exit;

    while( sql.RowGet() )
    {
        mbCalloc( entry_p, sizeof( HistList_t ) );

        entry_p->id     = sql.Int32u( 0 );
        entry_p->date   = sql.Int32u( 1 );
        entry_p->time   = sql.Int32u( 2 );
        entry_p->action = sql.Int32u( 3 );
        entry_p->int_1  = sql.Int32u( 4 );
        entry_p->int_2  = sql.Int32u( 5 );
        entry_p->int_3  = sql.Int32u( 6 );
        entry_p->int_4  = sql.Int32u( 7 );
        entry_p->int_5  = sql.Int32u( 8 );
        mbMallocStr( entry_p->string_1_p, sql.String( 9 ) );

        // Add to list sorted by ID (i.e., chronologically)
        added = FALSE;

        qWalk( entry2_p, List_q, HistList_p )
        {
            if( entry_p->id < entry2_p->id )
            {
                qInsertBefore( List_q, entry2_p, entry_p );
                added = TRUE;
                break;
            }
        }
        if( !added ) qInsertLast( List_q, entry_p );
    }

    // Now we must add the list entries to the form

    ListView->Items->BeginUpdate( );
    ListView->Items->Clear( );
    ListView->Items->EndUpdate( );

    qWalk( entry_p, List_q, HistList_p )
    {
        item_p = ListView->Items->Add( );

        // Set the date field
        if( entry_p->date )
        {
            dateTime.SetDate( entry_p->date );
            item_p->Caption = dateTime.MDY_DateString( );
        }
        else
        {
            item_p->Caption = "Not recorded";
        }

        // Set the time field
        if( entry_p->time )
        {
            dateTime.SetTime( entry_p->time );
            item_p->SubItems->Add( dateTime.HM_TimeString( ) );
        }
        else
        {
            item_p->SubItems->Add( "" );
        }

        // Add the action field
        item_p->SubItems->Add( pmcAppHistStrings[entry_p->action] );

        // Add the details field
        if( FormatDetails( entry_p ) == TRUE )
        {
            item_p->SubItems->Add( entry_p->details_p );
        }
    }

exit:
    if( gotLock )
    {
        mbLockRelease( ListLock );
    }
    mbFree( buf_p );
}

//---------------------------------------------------------------------------
// Function:    FormatDetails
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------
Int32s_t __fastcall TAppHistoryForm::FormatDetails( HistList_p entry_p )
{
    Int32s_t    returnCode = FALSE;
    Char_p      buf_p;
    Char_p      buf2_p;
    Ints_t      len;
    MbDateTime  dateTime;

    mbMalloc( buf_p, 2048 );
    mbMalloc( buf2_p, 512 );

    // Check pointer
    if( entry_p == NIL ) goto exit;

    // Free previously allocated details string
    if( entry_p->details_p ) mbFree( entry_p->details_p );

    *buf_p = 0;

    switch( entry_p->action )
    {
        case PMC_APP_ACTION_DELETE:
        case PMC_APP_ACTION_CUT:
        case PMC_APP_ACTION_CANCEL:
        case PMC_APP_ACTION_CONFIRM_LETTER_CANCEL:
        case PMC_APP_ACTION_CONFIRM_PHONE_CANCEL:
        case PMC_APP_ACTION_COMPLETE_SET:
        case PMC_APP_ACTION_COMPLETE_CLEAR:
            break;

        case PMC_APP_ACTION_CREATE:
        case PMC_APP_ACTION_CONFIRM_LETTER:
        case PMC_APP_ACTION_CONFIRM_PHONE:
        case PMC_APP_ACTION_PASTE:
        case PMC_APP_ACTION_MOVE:
            // int_1 - date
            // int_2 - time
            // int_3 - provider id
            // int_4 - duration  (CREATE only)
            dateTime.SetTime( entry_p->int_2 );
            strcat( buf_p, dateTime.HM_TimeString( ) );
            strcat( buf_p, " " );

            dateTime.SetDate( entry_p->int_1 );
            strcat( buf_p, dateTime.MDY_DateString( ) );
            strcat( buf_p, " " );

            if( entry_p->action == PMC_APP_ACTION_CREATE )
            {
                sprintf( buf2_p, "(%ld Min.)", entry_p->int_4 );
                strcat( buf_p, buf2_p );
                strcat( buf_p, " " );
            }

            if( pmcProviderDescGet( entry_p->int_3, buf2_p ) != NIL )
            {
                strcat( buf_p, buf2_p );
                strcat( buf_p, " " );
            }
            break;

        case PMC_APP_ACTION_REF_DR_ID:
            if( entry_p->int_1 != 0 )
            {
                PmcSqlDoctor_p   dr_p;
                mbMalloc( dr_p, sizeof( PmcSqlDoctor_t ) );
                if( pmcSqlDoctorDetailsGet( entry_p->int_1, dr_p ) == TRUE )
                {
                    pmcFormatName( dr_p->title, dr_p->firstName, NIL, dr_p->lastName,
                                   buf_p, PMC_FORMAT_NAME_STYLE_FMLT );
                }
                else
                {
                    sprintf( buf_p, "Error reading doctor details" );
                }
                mbFree( dr_p );
            }
            else
            {
                sprintf( buf_p, "None" );
            }
            break;

        case PMC_APP_ACTION_PATIENT_ID:
            if( entry_p->int_1 != 0 )
            {
                PmcSqlPatient_p   pat_p;
                mbMalloc( pat_p, sizeof( PmcSqlPatient_t ) );
                if( pmcSqlPatientDetailsGet( entry_p->int_1, pat_p ) == TRUE )
                {
                    pmcFormatName( pat_p->title, pat_p->firstName, NIL, pat_p->lastName,
                                   buf_p, PMC_FORMAT_NAME_STYLE_FMLT );
                }
                else
                {
                    sprintf( buf_p, "Error reading patient details" );
                }
                mbFree( pat_p );
            }
            else
            {
                sprintf( buf_p, "None" );
            }
            break;

        case PMC_APP_ACTION_TYPE:
            if( entry_p->int_1 > 0 )
            {
                sprintf( buf_p, pmcAppTypeStrings[entry_p->int_1] );
            }
            else
            {
                sprintf( buf_p, "None" );
            }
            break;

        case PMC_APP_ACTION_DURATION:
            sprintf( buf_p, "%ld Min.", entry_p->int_1 );
            break;

        case PMC_APP_ACTION_COMMENT:
//        case PMC_APP_ACTION_COMMENT_2:
            if( entry_p->string_1_p )
            {
                if( strlen( entry_p->string_1_p ) > 0 )
                {
                    strcpy( buf_p, entry_p->string_1_p );
                }
            }
            break;

        default:
            sprintf( buf_p, "default case reached" );
            break;
    }

    // Copy the format result into the entry
    if( ( len = strlen( buf_p ) ) > 0 )
    {
        mbMalloc( entry_p->details_p, len + 1 );
        strcpy( entry_p->details_p, buf_p );
        returnCode = TRUE;
    }

exit:
    mbFree( buf_p );
    mbFree( buf2_p );
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:    ListFree
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------
void __fastcall TAppHistoryForm::ListFree(  )
{
    HistList_p  entry_p;

    ListView->Items->BeginUpdate( );
    ListView->Items->Clear( );
    ListView->Items->EndUpdate( );

    mbLockAcquire( ListLock );
    while( !qEmpty( List_q ) )
    {
        entry_p = (HistList_p)qRemoveFirst( List_q );

        if( entry_p->string_1_p ) mbFree( entry_p->string_1_p );
        if( entry_p->details_p )  mbFree( entry_p->details_p );
        mbFree( entry_p );
    }
    mbLockRelease( ListLock );
}


//---------------------------------------------------------------------------
// Function:    CloseButtonClick
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------
void __fastcall TAppHistoryForm::CloseButtonClick(TObject *Sender)
{
    Close( );
}
//---------------------------------------------------------------------------
// Function:    FormClose
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------
void __fastcall TAppHistoryForm::FormClose
(
    TObject         *Sender,
    TCloseAction    &Action
)
{
    // Free any entries in the list
    ListFree( );
    Action = caFree;
}

//---------------------------------------------------------------------------
// Function:    UpdateInfo
//---------------------------------------------------------------------------
// Description: Update the info fields at the top of the form.
//---------------------------------------------------------------------------

void __fastcall TAppHistoryForm::UpdateInfo( Int32u_t id )
{
    Char_p            buf1_p;
    PmcSqlApp_p       app_p;
    PmcSqlDoctor_p    doc_p;
    PmcSqlProvider_p  provider_p;
    MbDateTime        dateTime;

    mbMalloc( buf1_p, 512 );
    mbMalloc( app_p, sizeof( PmcSqlApp_t) );
    mbMalloc( doc_p, sizeof( PmcSqlDoctor_t ) );
    mbMalloc( provider_p, sizeof( PmcSqlProvider_t ) );

    // Clear the fields first
    AppDateLabel->Caption = "";
    AppTimeLabel->Caption = "";
    AppProviderLabel->Caption = "";
    AppTypeLabel->Caption = "";
    AppDurationLabel->Caption = "";
    AppIdLabel->Caption = "";
    AppLetterConfLabel->Caption = "";
    AppPhoneConfLabel->Caption = "";
    AppReferringLabel->Caption = "";
    AppCommentLabel->Caption = "";

    sprintf( buf1_p, "%d", id );
    AppIdLabel->Caption = buf1_p;

    if( pmcSqlAppDetailsGet( id, app_p ) != TRUE ) goto exit;

    dateTime.SetDateTime( app_p->date, app_p->startTime );
    AppTimeLabel->Caption = dateTime.HM_TimeString( );
    AppDateLabel->Caption = dateTime.MDY_DateString( );
    AppTypeLabel->Caption = pmcAppTypeStrings[app_p->type];

    sprintf( buf1_p, "%d Min.", app_p->duration );
    AppDurationLabel->Caption = buf1_p;
    AppCommentLabel->Caption = app_p->commentIn;

    PatientId = app_p->patientId;

    if( app_p->confLetterDate )
    {
//        dateTime_p->SetDate( app_p->confLetterDate );
//        dateTime_p->HM_TimeString( buf1_p );
//        strcat( buf1_p, " " );

//        dateTime_p->SetTime( app_p->confLetterTime );
//        dateTime_p->MDY_DateString( buf2_p );
//        strcat( buf1_p, buf2_p );
//        strcat( buf1_p, " " );

//        if( pmcProviderDescGet( app_p->confLetterId , buf2_p ) != NIL )
//        {
//            strcat( buf1_p, buf2_p );
//        }
        AppLetterConfLabel->Caption = "Letter";

        if(    app_p->confLetterDate == app_p->date
            && app_p->confLetterTime == app_p->startTime
            && app_p->confLetterId   == app_p->providerId )
        {
            AppLetterConfLabel->Color = (TColor)pmcColor[ PMC_COLOR_CONFIRMED ];
        }
        else
        {
            AppLetterConfLabel->Color = (TColor)pmcColor[ PMC_COLOR_CONFLICT ];
        }
    }

    if( app_p->confPhoneDate )
    {
//        dateTime_p->SetDate( app_p->confPhoneDate );
//        dateTime_p->HM_TimeString( buf1_p );
//        strcat( buf1_p, " " );

//        dateTime_p->SetTime( app_p->confPhoneTime );
//        dateTime_p->MDY_DateString( buf2_p );
//        strcat( buf1_p, buf2_p );
//        strcat( buf1_p, " " );

//        if( pmcProviderDescGet( app_p->confPhoneId , buf2_p ) != NIL )
//        {
//            strcat( buf1_p, buf2_p );
//        }
        AppPhoneConfLabel->Caption = "Phone";

        if(    app_p->confPhoneDate == app_p->date
            && app_p->confPhoneTime == app_p->startTime
            && app_p->confPhoneId   == app_p->providerId )
        {
            AppPhoneConfLabel->Color = (TColor)pmcColor[ PMC_COLOR_CONFIRMED ];
        }
        else
        {
            AppPhoneConfLabel->Color = (TColor)pmcColor[ PMC_COLOR_CONFLICT ];
        }
    }

    if( pmcSqlDoctorDetailsGet( app_p->referringDrId, doc_p ) )
    {
        pmcFormatName( doc_p->title, doc_p->firstName, NIL, doc_p->lastName,
        buf1_p, PMC_FORMAT_NAME_STYLE_TFML );
        AppReferringLabel->Caption = buf1_p;
    }

    // Get and format the provider description
    if( pmcSqlProviderDetailsGet( app_p->providerId, provider_p ) )
    {
        AppProviderLabel->Caption = provider_p->description;
    }

    UpdatePatient( PatientId );

exit:

    mbFree( buf1_p );
    mbFree( app_p );
    mbFree( doc_p );
    mbFree( provider_p );
    return;
}

//---------------------------------------------------------------------------
// Function:    UpdatePatient
//---------------------------------------------------------------------------
// Description: Update the info fields at the top of the form.
//---------------------------------------------------------------------------

void __fastcall TAppHistoryForm::UpdatePatient( Int32u_t id )
{
    Char_p              buf1_p;
    PmcSqlPatient_p     pat_p;

    mbMalloc( pat_p, sizeof( PmcSqlPatient_t) );
    mbMalloc( buf1_p, 1024 );

    PatNameLabel->Caption = "";
    PatPhnLabel->Caption = "";
    PatPhoneLabel->Caption = "";
    PatIdLabel->Caption = "";

    // Get and format the patient details
    if( pmcSqlPatientDetailsGet( id , pat_p ) )
    {
        pmcFormatName( pat_p->title, pat_p->firstName, NIL, pat_p->lastName,
                       buf1_p, PMC_FORMAT_NAME_STYLE_FMLT );

        PatNameLabel->Caption = buf1_p;
        PatPhoneLabel->Caption = pat_p->formattedPhoneDay;
        pmcFormatPhnDisplay( pat_p->phn, pat_p->phnProv, buf1_p );
        PatPhnLabel->Caption = buf1_p;

        sprintf( buf1_p, "%ld", id );
        PatIdLabel->Caption = buf1_p;
    }
    mbFree( pat_p );
    mbFree( buf1_p );
}

//---------------------------------------------------------------------------
// Function:    pmcAppHistory
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t        pmcAppHistory
(
    Int32u_t    appId,
    Int32u_t    action,
    Int32u_t    int_1,
    Int32u_t    int_2,
    Int32u_t    int_3,
    Int32u_t    int_4,
    Char_p      string_1_p
)
{
    Int32u_t    today;
    Int32u_t    now;

    today = mbToday( );
    now   = mbTime( );

    return pmcAppHistoryTime( appId, today, now, action, int_1, int_2, int_3, int_4, string_1_p );
}

//---------------------------------------------------------------------------
// Function:    pmcAppHistoryTime
//---------------------------------------------------------------------------
// In order to speed up processing, insert records directly into
// the table instead of using the 'create record' utility function.
// We can do this because we do not need the record id.  Also, do no checking
// on the validity of the records (can implement a maint check later on).
//---------------------------------------------------------------------------

Int32s_t        pmcAppHistoryTime
(
    Int32u_t    appId,
    Int32u_t    date,
    Int32u_t    time,
    Int32u_t    action,
    Int32u_t    int_1,
    Int32u_t    int_2,
    Int32u_t    int_3,
    Int32u_t    int_4,
    Char_p      string_1_p
)
{
    Int32s_t    result = FALSE;
    Char_p      buf1_p;
    Char_p      buf2_p;
    MbSQL       sql;

    pmcSuspendPollInc( );

    mbMalloc( buf1_p, 4096 );
    mbMalloc( buf2_p, 4096 );

    mbStrClean( string_1_p, buf2_p, TRUE );

    sprintf( buf1_p, "insert into %s (%s,%s,%s,%s,%s,%s,%s,%s,%s) values "
                     "(%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,\"%s\")",

        PMC_SQL_TABLE_APP_HIST,
        PMC_SQL_APP_HIST_FIELD_APP_ID,
        PMC_SQL_FIELD_DATE,
        PMC_SQL_FIELD_TIME,
        PMC_SQL_APP_HIST_FIELD_ACTION,
        PMC_SQL_APP_HIST_FIELD_INT_1,
        PMC_SQL_APP_HIST_FIELD_INT_2,
        PMC_SQL_APP_HIST_FIELD_INT_3,
        PMC_SQL_APP_HIST_FIELD_INT_4,
        PMC_SQL_APP_HIST_FIELD_STRING_1,

        appId, date, time, action, int_1, int_2, int_3, int_4, buf2_p );

    sprintf( buf2_p, "lock tables %s write", PMC_SQL_TABLE_APP_HIST );

    sql.Update( buf2_p );
    sql.Update( buf1_p );
    sql.Update( "unlock tables" );

    pmcSuspendPollDec( );

    mbFree( buf1_p );
    mbFree( buf2_p );

    return result;
}

//---------------------------------------------------------------------------

Int32s_t    pmcAppHistoryForm( Int32u_t appId )
{
    TAppHistoryForm     *form_p;

    form_p = new TAppHistoryForm( NULL, appId );
    form_p->ShowModal();
    delete form_p;

    return TRUE;
}
//---------------------------------------------------------------------------

void __fastcall TAppHistoryForm::ConfButtonClick(TObject *Sender)
{
    pmcViewAppointmentConfirmation( AppId );
}

//---------------------------------------------------------------------------

#if PMC_APP_HISTORY_CREATE

void pmcAppHistoryCreate( void )
{
    Char_p                      buf_p;
    Char_p                      buf2_p;
    bool                        tableLocked = FALSE;
    Int32u_t                    count, i;
    Int32u_p                    appId_p = NIL;
    Int32u_p                    id_p;
    PmcSqlApp_p                 app_p;
    TDatabaseThermometer       *thermometer_p = NIL;
    MbDateTime                  dateTime;

    mbMalloc( buf_p, 1024 );
    mbMalloc( buf2_p, 512 );
    mbMalloc( app_p, sizeof( PmcSqlApp_t) );

    if( buf_p == NIL ) goto exit;

    // Lock the appointments table for the duration of this operation
    sprintf( buf_p, "lock tables %s write", PMC_SQL_TABLE_APPS );
    pmcSqlExec( buf_p );
    tableLocked = TRUE;
    pmcSuspendPoll = TRUE;

    // Get total number of app records
    sprintf( buf_p, "select %s from %s\n", PMC_SQL_CMD_COUNT, PMC_SQL_TABLE_APPS );
    count = pmcSqlSelectInt( buf_p, NIL );

    // Allocate space for array
    mbMalloc( appId_p, count * sizeof( Int32u_t ) );

    // Get the actual ids
    sprintf( buf_p, "select %s from %s where %s != 0",
        PMC_SQL_FIELD_ID,
        PMC_SQL_TABLE_APPS,
        PMC_SQL_FIELD_ID );

    id_p = appId_p;
    i = 0;

    PMC_GENERAL_QUERY_SETUP( buf_p );
    while( PMC_GENERAL_QUERY_CONTINUE( ) )
    {
        if( i == count )
        {
            break;
        }
        *id_p++ = pmcDataSet_p->Fields->Fields[0]->AsInteger;
        i++;
        PMC_GENERAL_QUERY_NEXT( );
    }
    PMC_GENERAL_QUERY_CLEANUP( );

    // Sanity check
    if( i != count )
    {
        mbDlgDebug(( "Error reading appointment ids" ));
        goto exit;
    }

    sprintf( buf_p, "Processing %ld appointment records...", count );
    thermometer_p = new TDatabaseThermometer( NULL, buf_p );
    thermometer_p->WindowState = wsNormal;
    thermometer_p->Gauge->MinValue = (Int32u_t)0;
    thermometer_p->Gauge->MaxValue = (Int32u_t)count;

    for( i = 0, id_p = appId_p ; i < count ; i++, id_p++ )
    {
        thermometer_p->Gauge->Progress++;

        if( pmcSqlAppDetailsGet( *id_p, app_p ) != TRUE )
        {
            mbDlgDebug(( "error reading app %d\n", id_p ));
        }

        dateTime_p->SetDate( app_p->createDate );
        dateTime_p->SetTime( app_p->createTime );

        // Record the creation of this appointment in the appointment history
        pmcAppHistoryTime( *id_p, app_p->createDate, app_p->createTime, PMC_APP_ACTION_CREATE,
            app_p->date, app_p->startTime, app_p->providerId, app_p->duration, NIL );

        // Set the patient ID
        if( app_p->patientId != 0 )
        {
            pmcAppHistoryTime( *id_p, app_p->createDate, app_p->createTime, PMC_APP_ACTION_PATIENT_ID,
                app_p->patientId, 0, 0, 0, NIL );
        }

        if( app_p->referringDrId != 0 )
        {
            pmcAppHistoryTime( *id_p, app_p->createDate, app_p->createTime, PMC_APP_ACTION_REF_DR_ID,
                app_p->referringDrId, 0, 0, 0, NIL );
        }

        if( app_p->type != 0 )
        {
            pmcAppHistoryTime( *id_p, app_p->createDate, app_p->createTime, PMC_APP_ACTION_TYPE,
                app_p->type, 0, 0, 0, NIL );
        }

        if( strlen( app_p->commentIn ) > 0 )
        {
            pmcAppHistoryTime( *id_p, app_p->createDate, app_p->createTime, PMC_APP_ACTION_COMMENT,
                    0, 0, 0, 0, app_p->commentIn );
        }

#if 0
        if( strlen( app_p->commentOut ) > 0 )
        {
            pmcAppHistoryTime( *id_p, app_p->createDate, app_p->createTime, PMC_APP_ACTION_COMMENT_2,
                    0, 0, 0, 0, app_p->commentOut );
        }
#endif

        if( app_p->confLetterDate )
        {
            if( app_p->confLetterTime == 0 || app_p->confLetterId == 0 )
            {
                mbDlgDebug(( "Letter conf error app id %d\n", *id_p ));
            }
            else
            {
                // Indicate "unknown" create times for confirmation
                pmcAppHistoryTime( *id_p, 0, 0, PMC_APP_ACTION_CONFIRM_LETTER,
                    app_p->confLetterDate, app_p->confLetterTime, app_p->confLetterId, 0, NIL );
           }
        }

        if( app_p->confPhoneDate )
        {
            if( app_p->confPhoneTime == 0 || app_p->confPhoneId == 0 )
            {
                mbDlgDebug(( "Phone conf error app id %d\n", *id_p ));
            }
            else
            {
                // Indicate "unknown" create times for confirmation
                 pmcAppHistoryTime( *id_p, 0, 0, PMC_APP_ACTION_CONFIRM_PHONE,
                    app_p->confPhoneDate, app_p->confPhoneTime, app_p->confPhoneId, 0, NIL );
            }
        }

        if( app_p->deleted )
        {
            if( app_p->completed == PMC_APP_COMPLETED_STATE_CANCELLED )
            {
               pmcAppHistoryTime( *id_p, app_p->createDate, app_p->createTime, PMC_APP_ACTION_CANCEL,
                    0, 0, 0, 0, NIL );
            }
            else
            {
               pmcAppHistoryTime( *id_p, app_p->createDate, app_p->createTime, PMC_APP_ACTION_DELETE,
                    0, 0, 0, 0, NIL );
            }
        }
        else
        {
            if( app_p->completed == PMC_APP_COMPLETED_STATE_COMPLETED )
            {
                pmcAppHistoryTime( *id_p, app_p->date, app_p->startTime, PMC_APP_ACTION_COMPLETE_SET,
                    0, 0, 0, 0, NIL );
            }
        }
    }

exit:

    if( thermometer_p ) delete thermometer_p;

    if( tableLocked )
    {
        sprintf( buf_p, "unlock tables" );
        pmcSqlExec( buf_p );
        pmcSuspendPoll = FALSE;
    }
    mbFree( buf_p );
    mbFree( buf2_p );
    mbFree( app_p );
    if( appId_p ) mbFree( appId_p );

    return;
}

#endif // PMC_APP_HISTORY_CREATE

void __fastcall TAppHistoryForm::PatientEditButtonClick(TObject *Sender)
{
    PatientEditView( PMC_EDIT_MODE_EDIT );
}
//---------------------------------------------------------------------------

void __fastcall TAppHistoryForm::PatientViewButtonClick(TObject *Sender)
{
    PatientEditView( PMC_EDIT_MODE_VIEW );
}
//---------------------------------------------------------------------------

void __fastcall TAppHistoryForm::PatientEditView( Int32u_t mode )
{
    pmcPatEditInfo_t        patEditInfo;
    TPatientEditForm       *patEditForm;

    if( PatientId )
    {
        patEditInfo.patientId = PatientId;
        patEditInfo.mode = mode;
        sprintf( patEditInfo.caption, "Patient Details" );

        patEditForm = new TPatientEditForm( NULL, &patEditInfo );
        patEditForm->ShowModal();
        delete patEditForm;

        if( patEditInfo.returnCode == MB_BUTTON_OK )
        {
            UpdatePatient( PatientId );
        }
        else
        {
            // User must have clicked cancel button
        }
    }
    return;
}
