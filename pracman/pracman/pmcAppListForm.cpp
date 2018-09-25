//---------------------------------------------------------------------------
// File:    pmcAppListForm.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    December 28, 2001
//---------------------------------------------------------------------------
// Description:
//
// Displays a list of appointments for a particular patient.  This function
// is not smart enough to realize if another application modifies the
// appointment list in the interim.
//---------------------------------------------------------------------------

// Platfor includes
#include <stdio.h>
#include <stdlib.h>
#include <vcl.h>
#pragma hdrstop

// Library includes
#include "mbUtils.h"

// Program includes
#include "pmcUtils.h"
#include "pmcInitialize.h"
#include "pmcGlobals.h"
#include "pmcPatientListForm.h"
#include "pmcPatientEditForm.h"
#include "pmcPatientRecord.h"
#include "pmcMainForm.h"
#include "pmcAppList.h"
#include "pmcAppListForm.h"
#include "pmcAppHistoryForm.h"
#include "pmcSeiko240.h"
#include "pmcColors.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
__fastcall TAppListForm::TAppListForm(TComponent* Owner)
    : TForm(Owner)
{
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

__fastcall TAppListForm::TAppListForm
(
    TComponent         *Owner,
    pmcAppListInfo_p    formInfo_p
)
: TForm(Owner)
{
    mbLockInit( AppListLock );

    Active = FALSE;
    DoubleClickFlag = FALSE;

    // Prevent database access until we are ready
    SkipAppListGet = TRUE;
    AppArray_pp = NIL;

    Today = mbToday( );

    PastAppsCount = 0;
    FutureAppsCount = 0;
    CancelledAppsCount = 0;
    DeletedAppsCount = 0;

    FutureAppsCountLabel->Caption = "";
    PastAppsCountLabel->Caption = "";

    // Set up for first sort
    SortCode = PMC_APP_SORT_DATE_ASCENDING;
    PreviousSortCode = PMC_APP_SORT_NONE;

    FormInfo_p = formInfo_p;
    FormInfo_p->gotGoto = FALSE;

    FutureAppsCheckBox->Checked = FormInfo_p->showFuture;
    PastAppsCheckBox->Checked   = FormInfo_p->showPast;
    CancelledAppsCheckBox->Checked = FALSE;

    if( FormInfo_p->allowGoto )
    {
        DoubleClickLabel->Caption = "Double click appointment to go to that appointment on the Main form.";
    }
    PastAutoEnable = TRUE;

    if( FormInfo_p->allowPatientSelect == FALSE )
    {
        PatientSelectButton->Enabled = FALSE;
    }

    if( FormInfo_p->mode == PMC_LIST_MODE_LIST )
    {
        OkButton->Visible = FALSE;
        CancelButton->Caption = "Close";
    }

    UpdatePatient( );

    // Initialize the linked list of appointments.
    App_q = qInitialize( &AppQueueHead );

    SkipAppListGet = FALSE;
    AppListGet( );
    Active = TRUE;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TAppListForm::UpdatePatient( void )
{
    PmcSqlPatient_p   pat_p;
    Char_p                      buf_p;

    mbCalloc( buf_p, 256 );

    // Allocate and zero patient info structure
    mbCalloc( pat_p, sizeof( PmcSqlPatient_t ) );

    if( FormInfo_p->patientId == 0 ) goto exit;

    // Get details about this patient
    if( pmcSqlPatientDetailsGet( FormInfo_p->patientId, pat_p ) == TRUE )
    {
        pmcFormatPhnDisplay( pat_p->phn, pat_p->phnProv, buf_p );

        strcat( pat_p->lastName, ", " );
        strcat( pat_p->lastName, pat_p->firstName );
    }

exit:

    PhnLabel->Caption = buf_p;
    PatientNameEdit->Text = pat_p->lastName;
    PhoneLabel->Caption = pat_p->formattedPhoneDay;

    if( pat_p->deceasedDate > 0 )
    {
        mbDlgExclaim( "This patient has been marked as deceased.\n" );
    }

    mbFree( pat_p );
    mbFree( buf_p );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TAppListForm::UpdateSelectedAppointment( void )
{
    Ints_t      i;
    MbDateTime  dateTime;

    Char_p      buf1_p;
    Char_p      buf2_p;
    Char_p      buf3_p;

    mbCalloc( buf1_p, 128 );
    mbCalloc( buf2_p, 128 );
    mbCalloc( buf3_p, 128 );

    mbLockAcquire( AppListLock );

    // NOTE: This does not look correct to me!!!!

    for( i = 0 ; i < AppArraySize ; i++ )
    {
        if( AppArray_pp[i]->id == FormInfo_p->appointId )
        {
            dateTime.SetDateTime( AppArray_pp[i]->date, AppArray_pp[i]->time );
            sprintf( buf1_p, dateTime.MDY_DateString( ) );
            sprintf( buf2_p, dateTime.HM_TimeString( ) );
            strcpy( buf3_p, AppArray_pp[i]->providerDesc_p );
        }
    }

    mbLockRelease( AppListLock );

    AppDateLabel->Caption = buf1_p;
    AppTimeLabel->Caption = buf2_p;
    AppProviderLabel->Caption = buf3_p;

    mbFree( buf1_p );
    mbFree( buf2_p );
    mbFree( buf3_p );

    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListGet( void )
{
    Char_p              buf_p;
    Char_p              buf2_p;
    Char_p              buf3_p;
    pmcAppListStruct_p  app_p;
    pmcAppListStruct_p  app2_p;
    MbDateTime          dateTime;
    MbSQL               sql;
    TCursor             cursorOrig;
    qHead_t             tempQueue;
    qHead_p             temp_q;
    Int32u_t            size, i;
    Boolean_t           found;

    temp_q          = qInitialize( &tempQueue );

    cursorOrig = Screen->Cursor;
    Screen->Cursor = crHourGlass;

    mbMalloc( buf_p, 20000 );
    mbMalloc( buf3_p, 20000 );
    mbMalloc( buf2_p, 256 );

    AppListInvalid = TRUE;
    AppListFree( );

    // Skip SQL read if no patient
    if( FormInfo_p->patientId == 0 ) goto exit;

    mbLockAcquire( AppListLock );

    // Format SQL command
    //                          0     1     2     3     4     5     6     7     8     9
    sprintf( buf_p, "select %s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,"
                           "%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s "
                           "from %s,%s,%s where "
                           "%s.%s=%ld and %s.%s=%s.%s and %s.%s=%s.%s",

        PMC_SQL_TABLE_APPS,     PMC_SQL_FIELD_DATE,                     // 0
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_DURATION,            // 1
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_START_TIME,          // 2
        PMC_SQL_TABLE_APPS,     PMC_SQL_FIELD_ID,                       // 3
        PMC_SQL_TABLE_PROVIDERS,PMC_SQL_FIELD_DESC,                     // 4
        PMC_SQL_TABLE_DOCTORS,  PMC_SQL_FIELD_LAST_NAME,                // 5
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_COMPLETED,           // 6
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_COMMENT_IN,          // 7

        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_CONF_PHONE_DATE,     // 8
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_CONF_PHONE_TIME,     // 9
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_CONF_PHONE_ID,       // 10
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_CONF_LETTER_DATE,    // 11
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_CONF_LETTER_TIME,    // 12
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_CONF_LETTER_ID,      // 13
        PMC_SQL_TABLE_APPS,     PMC_SQL_FIELD_PROVIDER_ID,              // 14
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_TYPE,                // 15
        PMC_SQL_TABLE_APPS,     PMC_SQL_FIELD_NOT_DELETED,              // 16

        PMC_SQL_TABLE_APPS,
        PMC_SQL_TABLE_PROVIDERS,
        PMC_SQL_TABLE_DOCTORS,

        PMC_SQL_TABLE_APPS,     PMC_SQL_FIELD_PATIENT_ID,   FormInfo_p->patientId,

        PMC_SQL_TABLE_PROVIDERS,PMC_SQL_FIELD_ID,
        PMC_SQL_TABLE_APPS,     PMC_SQL_FIELD_PROVIDER_ID,

        PMC_SQL_TABLE_DOCTORS,  PMC_SQL_FIELD_ID,
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_REFERRING_DR_ID );

    sql.Query( buf_p );
    while( sql.RowGet() )
    {
        mbCalloc( app_p, sizeof( pmcAppListStruct_t ) );
        app_p->date             = sql.Int32u( 0 );
        app_p->duration         = sql.Int32u( 1 );
        app_p->time             = sql.Int32u( 2 );
        app_p->id               = sql.Int32u( 3 );
        app_p->completedState   = sql.Int32u( 6 );

        mbMallocStr( app_p->providerDesc_p,     sql.String( 4 ) );
        mbMallocStr( app_p->referringDesc_p,    sql.String( 5 ) );
        mbMallocStr( app_p->comment_p,          sql.String( 7 ) );

        app_p->confirmedPhoneDate  = sql.Int32u(  8 );
        app_p->confirmedPhoneTime  = sql.Int32u(  9 );
        app_p->confirmedPhoneId    = sql.Int32u( 10 );
        app_p->confirmedLetterDate = sql.Int32u( 11 );
        app_p->confirmedLetterTime = sql.Int32u( 12 );
        app_p->confirmedLetterId   = sql.Int32u( 13 );
        app_p->providerId          = sql.Int32u( 14 );

        app_p->type                = sql.Int32u( 15 );
        app_p->notDeleted          = sql.Int32u( 16 );

        dateTime.SetTime( app_p->time );

        app_p->startTimeMin = dateTime.Hour( ) * 60 + dateTime.Min( );
        app_p->endTimeMin = app_p->startTimeMin + app_p->duration;

        app_p->conflict = FALSE;

        nbDlgDebug(( "Found apppointment date %d time %d duration %d provider %s startTime %d end time %d timeslots %s\n",
            app_p->date, app_p->time, app_p->duration, app_p->providerDesc_p,
            app_p->startTimeMin, app_p->endTimeMin, app_p->timeslots_p ));

#if 0
        if( app_p->notDeleted == FALSE && app_p->completedState == PMC_APP_COMPLETED_STATE_NONE )
        {
            // Free "deleted appointments that are not "cancelled"
            mbFree( app_p->providerDesc_p );
            mbFree( app_p->referringDesc_p );
            mbFree( app_p->comment_p );
            mbFree( app_p->timeslots_p );
            mbFree( app_p );
        }
        else
#endif
        {
            qInsertFirst( App_q, app_p );
        }
    }

    // Find all unique provider/date combinations
    size = App_q->size;
    for( i = 0 ; i < size ; i++ )
    {
        app_p = (pmcAppListStruct_p)qRemoveFirst( App_q );
        found = FALSE;

        qWalk( app2_p, temp_q, pmcAppListStruct_p )
        {
            if(     app_p->providerId == app2_p->providerId
                &&  app_p->date       == app2_p->date )
            {
                found = TRUE;
                break;
            }
        }
        if( found == TRUE )
        {
            //mbLog( "Found duplicate provider/date %lu/%lu\n", app_p->providerId, app_p->date );
            qInsertLast( App_q, app_p );
        }
        else
        {
            //mbLog( "Found unique provider/date %lu/%lu\n", app_p->providerId, app_p->date );
            qInsertLast( temp_q, app_p );
        }
    }

    sprintf( buf_p, "select %s,%s,%s from %s where ",
              PMC_SQL_TIMESLOTS_FIELD_AVAILABLE,
              PMC_SQL_FIELD_PROVIDER_ID,
              PMC_SQL_FIELD_DATE,
              PMC_SQL_TABLE_TIMESLOTS );

    size = 0;
    sprintf( buf3_p, "(" );
    qWalk( app_p, temp_q, pmcAppListStruct_p )
    {
        if( size++ > 0 ) strcat( buf3_p, " or " );

        sprintf( buf2_p, "( %s=%lu and %s=%lu )",
            PMC_SQL_FIELD_PROVIDER_ID, app_p->providerId,
            PMC_SQL_FIELD_DATE, app_p->date );

        strcat( buf3_p, buf2_p );
    }

    strcat( buf3_p , ")" );

    // Must restore original queue
    while( !qEmpty( temp_q ) )
    {
        app2_p = (pmcAppListStruct_p)qRemoveFirst( temp_q );
        qInsertLast( App_q, app2_p );
    }

    strcat( buf_p, buf3_p );
    if( size > 0 )
    {
        sql.Query( buf_p );
        while( sql.RowGet( ) )
        {
            qWalk( app_p, App_q, pmcAppListStruct_p )
            {
                if(    app_p->providerId ==  sql.Int32u( 1 )
                    && app_p->date       ==  sql.Int32u( 2 ) )
                {
                    strncpy( app_p->timeslots, sql.String( 0 ), PMC_TIMESLOTS_PER_DAY );

                    // Required because of bug in database: many available time strings are short
                    while( strlen( app_p->timeslots ) <  PMC_TIMESLOTS_PER_DAY )
                    {
                        strcat( app_p->timeslots, "0" );
                    }
                }
            }
        }
    }

    mbLockRelease( AppListLock );

exit:

    // Look for conflicts
    AppListConflictsPatient(  );
    AppListConflictsTimeslots( );
    AppListConflictsProvider( ( size > 0 ) ? buf3_p : NIL );

    AppListSort( );

    Screen->Cursor = cursorOrig;

    mbFree( buf_p );
    mbFree( buf2_p );
    mbFree( buf3_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListSort( void )
{
    Int32u_t            i;
    pmcAppListStruct_p  app_p, app2_p;
    bool                include;
    bool                added;
    qHead_t             app2Queue;
    qHead_p             app2_q;
    Char_t              buf[32];

    // Do nothing if not yet ready
    if( SkipAppListGet == TRUE ) return;

    mbLockAcquire( AppListLock );

    if( SortCode != PreviousSortCode )
    {
        PreviousSortCode = SortCode;
        app2_q = qInitialize( &app2Queue );

        // First, move all entries into a backup queue
        for( ; ; )
        {
            if( qEmpty( App_q ) ) break;
            app_p = (pmcAppListStruct_p)qRemoveFirst( App_q );
            qInsertFirst( app2_q, app_p );
        }

        // Move all entries back into main queue in desired order
        for( ; ; )
        {
            if( qEmpty( app2_q ) ) break;

            // Get one entry from backup list
            app_p = (pmcAppListStruct_p)qRemoveFirst( app2_q );

            added = FALSE;

            // Loop through entries in the main queue
            qWalk( app2_p, App_q, pmcAppListStruct_p )
            {
                if( SortCode == PMC_APP_SORT_DATE_ASCENDING )
                {
                    if( ( added = AppListSortDateAscending( app_p, app2_p ) ) == TRUE ) break;
                }
                else if( SortCode == PMC_APP_SORT_DATE_DESCENDING )
                {
                    if( ( added = AppListSortDateDescending( app_p, app2_p ) ) == TRUE ) break;
                }
            }

            // Put entry at end of main list if not added elsewhere
            if( added == FALSE ) qInsertLast( App_q, app_p );
        }
    } // End of Sort function

    // Free array that points to linked list entries
    if( AppArray_pp ) mbFree( AppArray_pp )

    nbDlgDebug(( "Allocating %ld bytes (list size: %ld)",  sizeof(pmcClaimStruct_p) * Claim_q->size, Claim_q->size ));

    mbMalloc( AppArray_pp, sizeof(pmcAppListStruct_p) * App_q->size );

    // Loop through appointment list to count appointments to
    // see if we should auto enable past appointments.  Auto
    // enable past appointments if we have not autoenabled
    // before and there are new future appointments
    i = 0;
    PastAppsCount = 0;
    FutureAppsCount = 0;
    CancelledAppsCount = 0;
    DeletedAppsCount = 0;

    qWalk( app_p, App_q, pmcAppListStruct_p )
    {
        if( app_p->notDeleted == 0 )
        {
            if( app_p->completedState == PMC_APP_COMPLETED_STATE_CANCELLED )
            {
                CancelledAppsCount++;
            }
            else
            {
                DeletedAppsCount++;
            }
        }
        else if( app_p->date >= Today )
        {
            FutureAppsCount++;
        }
        else
        {
            PastAppsCount++;
        }
    }
    if( PastAutoEnable && FormInfo_p->patientId != 0 )
    {
        PastAutoEnable = FALSE;
        if( FutureAppsCount == 0 )
        {
            // Auto enable past appointments
            Active = FALSE;
            PastAppsCheckBox->Checked = TRUE;
            Active = TRUE;
        }
    }

    // Loop through the appointment list adding to the array of pointers
    i = 0;
    qWalk( app_p, App_q, pmcAppListStruct_p )
    {
        include = FALSE;
        if( app_p->notDeleted == 0 )
        {
            if( app_p->completedState == PMC_APP_COMPLETED_STATE_CANCELLED )
            {
                if( CancelledAppsCheckBox->Checked == TRUE ) include = TRUE;
            }
            else
            {
                if( DeletedAppsCheckBox->Checked == TRUE ) include = TRUE;
            }
        }
        else if( app_p->date >= Today )
        {
            if( FutureAppsCheckBox->Checked == TRUE ) include = TRUE;
         }
        else
        {
            if( PastAppsCheckBox->Checked == TRUE ) include = TRUE;
        }
        if( include ) AppArray_pp[i++] = app_p;
    }

    sprintf( buf, "%d", PastAppsCount );
    PastAppsCountLabel->Caption = buf;

    sprintf( buf, "%d", FutureAppsCount );
    FutureAppsCountLabel->Caption = buf;

    sprintf( buf, "%d", CancelledAppsCount );
    CancelledAppsCountLabel->Caption = buf;

    sprintf( buf, "%d", DeletedAppsCount );
    DeletedAppsCountLabel->Caption = buf;

    AppArraySize = i;

    AppListGrid->RowCount  = (( i + 1 ) == 1 ) ? 2 : i + 1;
    AppListGrid->Col = PMC_APP_LIST_COL_CONFIRMED_LETTER;

    mbLockRelease( AppListLock );

    FormInfo_p->appointId = ( i > 0 ) ? AppArray_pp[0]->id : 0;

    UpdateSelectedAppointment( );

    AppListGrid->Invalidate( );
}

//---------------------------------------------------------------------------
// Function:  TAppListForm::AppListSortDateAscending
//---------------------------------------------------------------------------
// Description:
//
// Sort appointments by date (and time)
//---------------------------------------------------------------------------

bool __fastcall TAppListForm::AppListSortDateAscending
(
    pmcAppListStruct_p  app1_p,
    pmcAppListStruct_p  app2_p
)
{
    bool                added = FALSE;

    if( app1_p->date > app2_p->date )
    {
        qInsertBefore(  App_q, app2_p, app1_p );
        added = TRUE;
        goto exit;
    }

    if( app1_p->date < app2_p->date ) goto exit;

    // At this point it is known that the dates are the same.  Next sort by time
    if( app1_p->time > app2_p->time )
    {
        qInsertBefore(  App_q, app2_p, app1_p );
        added = TRUE;
        goto exit;
    }

exit:
    return added;
}

//---------------------------------------------------------------------------
// Function:  TAppListForm::AppListSortDateDescending
//---------------------------------------------------------------------------
// Description:
//
// Sort appointments by date (and time)
//---------------------------------------------------------------------------

bool __fastcall TAppListForm::AppListSortDateDescending
(
    pmcAppListStruct_p  app1_p,
    pmcAppListStruct_p  app2_p
)
{
    bool                added = FALSE;

    if( app1_p->date < app2_p->date )
    {
        qInsertBefore(  App_q, app2_p, app1_p );
        added = TRUE;
        goto exit;
    }

    if( app1_p->date > app2_p->date ) goto exit;

    // At this point it is known that the dates are the same.  Next sort by time
    if( app1_p->time < app2_p->time )
    {
        qInsertBefore(  App_q, app2_p, app1_p );
        added = TRUE;
        goto exit;
    }

exit:
    return added;
}

//---------------------------------------------------------------------------
// Function:  TAppListForm::AppListConflictsProvider
//---------------------------------------------------------------------------
// Description:
//
// Look for conflicts with other patients
//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListConflictsProvider( Char_p provDates_p )
{
    Char_p              buf_p;
    pmcAppListStruct_p  app_p;
    Int32u_t            startTimeMin;
    Int32u_t            endTimeMin;
    MbDateTime          dateTime;
    MbSQL               sql;

    if( App_q->size == 0 ) return;

    if( provDates_p == NIL ) return;

    mbLockAcquire( AppListLock );

    mbMalloc( buf_p, 4024 );

    // Format SQL command    0  1  2  3  4  5
    sprintf( buf_p, "select %s,%s,%s,%s,%s,%s from %s where %s and %s=%lu"
            ,PMC_SQL_FIELD_ID                   // 0
            ,PMC_SQL_FIELD_DATE                 // 1
            ,PMC_SQL_FIELD_PATIENT_ID           // 2
            ,PMC_SQL_APPS_FIELD_START_TIME      // 3
            ,PMC_SQL_APPS_FIELD_DURATION        // 4
            ,PMC_SQL_FIELD_PROVIDER_ID

            ,PMC_SQL_TABLE_APPS

            ,provDates_p
            ,PMC_SQL_FIELD_NOT_DELETED   , PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

    sql.Query( buf_p );
    while( sql.RowGet( ) )
    {
        qWalk( app_p, App_q, pmcAppListStruct_p )
        {
            if( app_p->providerId != sql.Int32u( 5 ) ) continue;

            if( app_p->date != sql.Int32u( 1 ) ) continue;

            // Found myself... don't compare
            if( app_p->id == sql.Int32u( 0 ) )  continue;

            dateTime.SetTime( sql.Int32u( 3 ) );
            startTimeMin = dateTime.Hour( ) * 60 + dateTime.Min( );
            endTimeMin = startTimeMin + sql.Int32u( 4 );

            //mbLog( "found another app on the same provider/day: startTime: %u endTime: %u\n", startTimeMin, endTimeMin );

            if( endTimeMin >  app_p->startTimeMin &&
                endTimeMin <= app_p->endTimeMin )
            {
                app_p->conflict = TRUE;
            }
            else if( startTimeMin >= app_p->startTimeMin &&
                     startTimeMin <  app_p->endTimeMin )
            {
                app_p->conflict = TRUE;
            }
            else if( app_p->endTimeMin >  startTimeMin &&
                     app_p->endTimeMin <= endTimeMin )
            {
                app_p->conflict = TRUE;
            }
            else if( app_p->startTimeMin >= startTimeMin &&
                     app_p->startTimeMin <  endTimeMin )
            {
                app_p->conflict = TRUE;
            }
        }
    }

    mbLockRelease( AppListLock );
    mbFree( buf_p );
    return;
}

//---------------------------------------------------------------------------
// Function:  TAppListForm::AppListConflictsPatient
//---------------------------------------------------------------------------
// Description:
//
// Look for conflicts among the patient's apppointments
//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListConflictsPatient( void )
{
    Int32u_t            i;
    Int32u_t            size;
    Int32u_t            today;
    pmcAppListStruct_p  app_p, app2_p;

    // Do nothing if not yet ready
    if( SkipAppListGet == TRUE ) return;

    today = mbToday( );

    mbLockAcquire( AppListLock );

    size = App_q->size;

    for( i = 0 ; i < size ; i++ )
    {
        app_p = (pmcAppListStruct_p)qRemoveFirst( App_q );

        // Ingore "deleted" apps and past apps when computing conflicts
        if( app_p->notDeleted == PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE && app_p->date >= today )
        {
            // Loop through entries in the main queue
            qWalk( app2_p, App_q, pmcAppListStruct_p )
            {
                if( app2_p->notDeleted == PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE )
                {
                    if( app_p->date == app2_p->date )
                    {
                        app_p->appCount++;

                        //appointments are on the same date... do they overlap?
                        if( app2_p->endTimeMin >  app_p->startTimeMin &&
                            app2_p->endTimeMin <= app_p->endTimeMin )
                        {
                            app_p->conflict = TRUE;
                        }
                        else if( app2_p->startTimeMin >= app_p->startTimeMin &&
                                 app2_p->startTimeMin <  app_p->endTimeMin )
                        {
                            app_p->conflict = TRUE;
                        }
                        else if( app_p->endTimeMin >  app2_p->startTimeMin &&
                                 app_p->endTimeMin <= app2_p->endTimeMin )
                        {
                            app_p->conflict = TRUE;
                        }
                        else if( app_p->startTimeMin >= app2_p->startTimeMin &&
                                 app_p->startTimeMin <  app2_p->endTimeMin )
                        {
                            app_p->conflict = TRUE;
                        }
                    }
                    if( app_p->conflict == TRUE )
                    {
                        if( app_p->providerId != app2_p->providerId )
                        {
                            app_p->conflict = FALSE;
                            app_p->conflictCount++;
                        }
                    }
                }
            }
        }
        qInsertLast( App_q, app_p );
    }
    mbLockRelease( AppListLock );
    return;
}

//---------------------------------------------------------------------------
// Function:  TAppListForm::AppListConflictsProvider
//---------------------------------------------------------------------------
// Description:
//
// Look for conflicts among the patient's apppointments
//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListConflictsTimeslots( void )
{
    Int32u_t            i;
    pmcAppListStruct_p  app_p;
    Int32u_t            slotIndex;
    Int32u_t            startTime;
    MbDateTime          dateTime;
    Int32u_t            today;

    // Do nothing if not yet ready
    if( SkipAppListGet == TRUE ) return;

    today = mbToday( );
    dateTime.SetDate( today );

    mbLockAcquire( AppListLock );

     // Loop through entries checking the available time
    qWalk( app_p, App_q, pmcAppListStruct_p )
    {
        // MAB:20020708: Ignore "deleted" apps when computing conflicts
        // MAB:20020720: Also only compute conflicts for future appointments
        if( app_p->notDeleted == PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE && app_p->date > today)
        {
            // Update the available times
            for( i = 1, slotIndex = 1 ; i < PMC_TIMESLOTS_PER_DAY ; i++, slotIndex++ )
            {
                if( app_p->timeslots[slotIndex] == PMC_TIMESLOT_AVAILABLE )
                {
                    // This timeslot is available
                }
                else
                {
                    dateTime.SetTime( pmcTimeSlotInts[i] );

                    startTime = dateTime.Hour( ) * 60 + dateTime.Min( );

                    nbDlgDebug(( "slot %d is NOT available (start time %d end time %d)\n",
                        pmcTimeSlotInts[i], startTime, endTime ));

                    if( startTime >= app_p->startTimeMin && startTime < app_p->endTimeMin )
                    {
                        app_p->conflict = TRUE;
                    }
                }
            }
        }
    }
    mbLockRelease( AppListLock );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFree( void )
{
    pmcAppListStruct_p  app_p;

    mbLockAcquire( AppListLock );

    while( !qEmpty( App_q ) )
    {
        app_p = (pmcAppListStruct_p)qRemoveFirst( App_q );

        mbFree( app_p->providerDesc_p );
        mbFree( app_p->referringDesc_p );
        mbFree( app_p->comment_p );
        mbFree( app_p );
    }

    if( AppArray_pp )
    {
        mbFree( AppArray_pp );
        AppArray_pp = NIL;
        AppArraySize = 0;
    }

    mbLockRelease( AppListLock );

    return;
}
//---------------------------------------------------------------------------

void __fastcall TAppListForm::CancelButtonClick(TObject *Sender)
{
    FormInfo_p->returnCode = MB_BUTTON_CANCEL;
    Close( );
}
//---------------------------------------------------------------------------

void __fastcall TAppListForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
    AppListFree( );
    // MAB FormInfo_p->appointId = AppointId;
    Action = caFree;
}

//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListGridMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    Ints_t      row = 0, col = 0;
    TRect       rect;

    if( DoubleClickFlag == TRUE )
    {
        DoubleClickFlag = FALSE;
        return;
    }

    AppListGrid->MouseToCell( X, Y, col, row );
    rect = AppListGrid->CellRect( col, row );

    MouseInRow = ( row >= 0 && row <= (Ints_t)AppArraySize )   ? row : -1;
    MouseInCol = ( col >= 0 && col <= PMC_APP_LIST_COL_COUNT ) ? col : -1;

    if( row == 0 )
    {
        if(    col == PMC_APP_LIST_COL_DATE
            || col == PMC_APP_LIST_COL_YEAR
            || col == PMC_APP_LIST_COL_DAY
            || col == PMC_APP_LIST_COL_TIME )
        {
            pmcButtonDown( AppListGrid->Canvas, &rect, pmcAppListStrings[col] );
        }
    }
}

//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListGridMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    pmcButtonUp( );

    if( MouseInRow == 0 )
    {
        // Figure out if we should resort the list
        if(    MouseInCol == PMC_APP_LIST_COL_DATE
            || MouseInCol == PMC_APP_LIST_COL_YEAR
            || MouseInCol == PMC_APP_LIST_COL_DAY
            || MouseInCol == PMC_APP_LIST_COL_TIME )
        {
            // Sort by name
            SortCode = ( SortCode == PMC_APP_SORT_DATE_DESCENDING ) ? PMC_APP_SORT_DATE_ASCENDING :  PMC_APP_SORT_DATE_DESCENDING;
            AppListSort( );
        }
    }
    else
    {
        if( MouseInRow > 0 && MouseInRow <= AppArraySize )
        {
            FormInfo_p->appointId = AppArray_pp[MouseInRow - 1]->id;
            UpdateSelectedAppointment( );
        }
    }
    AppListGrid->Invalidate( );
}

//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListGridDrawCell(TObject *Sender,
      int ACol, int ARow, TRect &Rect, TGridDrawState State)
{
    Char_p              buf_p;
    bool                rightJustify;
    AnsiString          str = "";
    pmcAppListStruct_p  app_p;
    Ints_t              cellWidth;
    Ints_t              textWidth;
    Ints_t              offset = 2;
    MbDateTime          dateTime;
    TColor              color;

    if( SkipAppListGet ) return;

    mbLockAcquire( AppListLock );

    mbMalloc( buf_p, 128 );

    rightJustify = FALSE;

    if( ARow == 0 )
    {
        // This is the title row
        str = pmcAppListStrings[ACol];
        AppListGrid->Canvas->TextRect( Rect, Rect.Left + 2, Rect.Top, str );
    }
    else if( ARow <= (Ints_t)AppArraySize )
    {
        str = "";

        // Claim array size could be 0 if there are no entries to display
        if( AppArraySize > 0 )
        {
            // Look for selected Row to determined SelectedClaimId
            if( ARow == AppListGrid->Row )
            {
                app_p = AppArray_pp[ARow - 1];
            }

            app_p = AppArray_pp[ARow-1];

            dateTime.SetDateTime( app_p->date, app_p->time );

            color = (TColor)pmcColor[ PMC_COLOR_APP ];
            if( app_p->notDeleted == 0 )
            {
                // 20021020: Show both cancelled and deleted apps as this color
                // if( app_p->completedState == PMC_APP_COMPLETED_STATE_CANCELLED )
                {
                    color = (TColor)pmcColor[ PMC_COLOR_REDUCED ];
                }
            }
            else if( app_p->conflict )
            {
                color = (TColor)pmcColor[ PMC_COLOR_CONFLICT ];
            }
            else if( app_p->completedState == PMC_APP_COMPLETED_STATE_COMPLETED )
            {
                color = (TColor)pmcColor[ PMC_COLOR_COMPLETED ];
            }
            else if( app_p->date < Today )
            {
                color = (TColor)pmcColor[ PMC_COLOR_NOSHOW ];
            }

            if( app_p->appCount )
            {
                PMC_COLOR_DARKEN( (Int32u_t)color );
            }
            if( app_p->conflictCount )
            {
                PMC_COLOR_DARKEN( (Int32u_t)color );
            }
#if 0
            // Determine the color for this cell
            if( app_p->completedState == PMC_APP_COMPLETED_STATE_CANCELLED )
            {
                color = (TColor)pmcColor[ PMC_COLOR_REDUCED ];
            }
            else if( app_p->date >= Today )
            {

                if( ACol != PMC_APP_LIST_COL_CONFIRMED_PHONE &&
                    ACol != PMC_APP_LIST_COL_CONFIRMED_LETTER )
                {
                    if( app_p->conflict )  color = (TColor)pmcColor[ PMC_COLOR_CONFLICT ];
                }
            }
            else
            {
                if( app_p->completedState == PMC_APP_COMPLETED_STATE_COMPLETED )
                {
                    color = (TColor)pmcColor[ PMC_COLOR_COMPLETED ];
                }
                else
                {
                    color = (TColor)pmcColor[ PMC_COLOR_NOSHOW ];
                }
            }
#endif

            // Determine the string for this call
            sprintf( buf_p, "" );
            str = buf_p;
            if( ACol == PMC_APP_LIST_COL_DATE  )
            {
                str = dateTime.MD_DateString( );
            }
            if( ACol == PMC_APP_LIST_COL_YEAR  )
            {
                str = dateTime.Year_DateString( );
            }
            if( ACol == PMC_APP_LIST_COL_DAY  )
            {
                str = dateTime.Day_DateString( );
            }
            if( ACol == PMC_APP_LIST_COL_TIME )
            {
                str = dateTime.HM_TimeString( );
                rightJustify = TRUE;
            }
            if( ACol == PMC_APP_LIST_COL_DURATION )
            {
                sprintf( buf_p, "%d", app_p->duration );
                str = buf_p;
                rightJustify = TRUE;
            }
            if( ACol == PMC_APP_LIST_COL_PROVIDER )
            {
                str = app_p->providerDesc_p;
            }
            if( ACol == PMC_APP_LIST_COL_TYPE )
            {
                if( app_p->type < PMC_APP_TYPE_INVALID )
                {
                    str = pmcAppTypeStrings[app_p->type];
                    if( app_p->type == PMC_APP_TYPE_URGENT )
                    {
                        color = (TColor)pmcColor[ PMC_COLOR_CONFLICT ];
                    }
                }
            }
            if( ACol == PMC_APP_LIST_COL_REFERRING )
            {
                str = app_p->referringDesc_p;
            }
            if( ACol == PMC_APP_LIST_COL_CONFIRMED_PHONE )
            {
                // Check if any confimation done
                if( app_p->confirmedPhoneDate || app_p->confirmedPhoneTime || app_p->confirmedPhoneId )
                {
                    // There is confirmation...
                    color = (TColor)pmcColor[ PMC_COLOR_CONFLICT ];
                    str = "P";
                    if( app_p->confirmedPhoneDate == app_p->date &&
                        app_p->confirmedPhoneTime == app_p->time &&
                        app_p->confirmedPhoneId   == app_p->providerId )
                    {
                        color = (TColor)pmcColor[ PMC_COLOR_CONFIRMED ];
                    }
                }
            }
            if( ACol == PMC_APP_LIST_COL_CONFIRMED_LETTER )
            {
                // Check if any confimation done
                if( app_p->confirmedLetterDate || app_p->confirmedLetterTime || app_p->confirmedLetterId )
                {
                   // There is confirmation...
                   str = "L";
                   color = (TColor)pmcColor[ PMC_COLOR_CONFLICT ];
                   if( app_p->confirmedLetterDate == app_p->date &&
                       app_p->confirmedLetterTime == app_p->time &&
                       app_p->confirmedLetterId   == app_p->providerId )
                   {
                       color = (TColor)pmcColor[ PMC_COLOR_CONFIRMED ];
                   }
                }
            }
            if( ACol == PMC_APP_LIST_COL_COMMENT )
            {
                str = app_p->comment_p;
            }
        }

        // Color the rectangle
        AppListGrid->Canvas->Brush->Color =  color;
        AppListGrid->Canvas->FillRect( Rect );

        // Output the string... right justify if requested
        if( rightJustify )
        {
            cellWidth = Rect.Right - Rect.Left;
            textWidth = AppListGrid->Canvas->TextWidth( str );
            offset = cellWidth - textWidth - 3;
            if( offset < 3 ) offset = 3;
        }
        AppListGrid->Canvas->TextRect( Rect, Rect.Left + offset, Rect.Top, str);
    }
    else
    {
        nbDlgDebug(( "Got invalid Row: %ld, AppArraySize: %ld", ARow, AppArraySize ));
    }

    mbFree( buf_p );
    mbLockRelease( AppListLock );
}

//---------------------------------------------------------------------------

void __fastcall TAppListForm::PastAppsCheckBoxClick(TObject *Sender)
{
    AppListGrid->Col = PMC_APP_LIST_COL_CONFIRMED_LETTER;
    if( Active )
    {
        AppListGrid->SetFocus( );
        AppListSort( );
    }
}
//---------------------------------------------------------------------------
void __fastcall TAppListForm::CancelledAppsCheckBoxClick(TObject *Sender)
{
    AppListGrid->Col = PMC_APP_LIST_COL_CONFIRMED_LETTER;
    if( Active )
    {
        AppListGrid->SetFocus( );
        AppListSort( );
    }
}
//---------------------------------------------------------------------------

void __fastcall TAppListForm::FutureAppsCheckBoxClick(TObject *Sender)
{
    AppListGrid->Col = PMC_APP_LIST_COL_CONFIRMED_LETTER;
    if( Active )
    {
        AppListGrid->SetFocus( );
        AppListSort( );
    }
}

void __fastcall TAppListForm::DeletedAppsCheckBoxClick(TObject *Sender)
{
    AppListGrid->Col = PMC_APP_LIST_COL_CONFIRMED_LETTER;
    if( Active )
    {
        AppListGrid->SetFocus( );
        AppListSort( );
    }
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

void __fastcall TAppListForm::PatientEditButtonClick(TObject *Sender)
{
    PatientEditView( PMC_EDIT_MODE_EDIT );

    AppListGrid->Col = PMC_APP_LIST_COL_CONFIRMED_LETTER;
    if( Active ) AppListGrid->SetFocus( );
}
//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupPopup(TObject *Sender)
{
    pmcAppListStruct_p  app_p;
    Int32u_t            today;

    today = mbToday( );

    pmcPopupItemEnableAll( AppListFormPopup, FALSE );
    FormInfo_p->appointId = 0;

    if( MouseInRow <= AppArraySize && MouseInRow > 0 )
    {
        AppListGrid->Row = MouseInRow;
        app_p = AppArray_pp[MouseInRow - 1];

        if( app_p->id != 0 )
        {
            FormInfo_p->appointId = app_p->id;
            UpdateSelectedAppointment( );

            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_PATIENT,                  TRUE );
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_PATIENT_VIEW,             TRUE );
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_PATIENT_EDIT,             TRUE );
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_PATIENT_PRINT_RECORD,     TRUE );
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_DR,                       TRUE );
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_DR_VIEW,                  TRUE );
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_DR_EDIT,                  TRUE );
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_DR_PRINT_RECORD,          TRUE );

            // Allow the confirmation time to be checked no matter what type of appointment this is
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_CONFIRM,                  TRUE );
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_CONFIRM_CHECK,            TRUE );

            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_HISTORY,                  TRUE );

            if( pmcCfg[CFG_SLP_NAME].str_p )
            {
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_DR_PRINT_ADD_LBL,         TRUE );
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_PATIENT_PRINT_ADD_LBL,    TRUE );
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_PATIENT_PRINT_REQ_LBL,    TRUE );
            }
        }

//        if( app_p->id != 0 && app_p->completedState != PMC_APP_COMPLETED_STATE_CANCELLED )
        if( app_p->id != 0 && app_p->notDeleted != 0 )
        {
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_GOTO,                     TRUE );
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_PATIENT_PRINT_APP_LETTER, TRUE );

            if( app_p->date >= today && app_p->completedState != PMC_APP_COMPLETED_STATE_COMPLETED )
            {
            // Do not allow past appointments to be cut or deleted
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_DR_CHANGE,                TRUE );
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_DR_CLEAR,                 TRUE );

            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_CUT,                      TRUE );
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_DELETE,                   TRUE );
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_CANCEL,                   TRUE );
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_TYPE,                     TRUE );
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_TYPE_NEW,                 TRUE );
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_TYPE_URGENT,              TRUE );
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_TYPE_REVIEW,              TRUE );
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_TYPE_CLEAR,               TRUE );
            }

            if( app_p->date >= today )
            {
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_CONFIRM_PHONE,            TRUE );
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_CONFIRM_LETTER,           TRUE );
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_CONFIRM_PHONE_CANCEL,     TRUE );
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_CONFIRM_LETTER_CANCEL,    TRUE );
            }

            if( app_p->date <= today )
            {
            pmcPopupItemEnable( AppListFormPopup, PMC_APP_LIST_FORM_POPUP_COMPLETE,                 TRUE );
            }
        }
    }
    else
    {
        nbDlgDebug(( "Mouse in row: %d AppArraySize: %d", MouseInRow, AppArraySize ));
    }
}

//---------------------------------------------------------------------------
// Function:   AppListFormPopupCutClick
//---------------------------------------------------------------------------
// Description:
//
// The appointment cut function on the main form is kind of ugly so just
// implement a separate appointment cut function here for now.
//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupCutClick(TObject *Sender)
{
    pmcAppointCutBuf_p  appoint_p = NIL;
    Char_t              buf[32];

    nbDlgDebug(( "Cut appointment called id: %d", AppointId ));

    if( FormInfo_p->appointId == 0 ) goto exit;

    mbLockAcquire( pmcAppointCutBuf_q->lock );
    mbMalloc( appoint_p, sizeof(pmcAppointCutBuf_t) );

    if( pmcSqlRecordDelete( PMC_SQL_TABLE_APPS, FormInfo_p->appointId ) == TRUE )
    {
        pmcAppHistory( FormInfo_p->appointId, PMC_APP_ACTION_CUT, 0, 0, 0, 0, NIL );

        appoint_p->appointId = FormInfo_p->appointId;
        sprintf( appoint_p->desc, "%s", PatientNameEdit->Text.c_str( ) );

        PreviousSortCode = PMC_APP_SORT_NONE;
        AppListGet( );
        AppListGrid->Invalidate( );

        qInsertFirst( pmcAppointCutBuf_q, appoint_p );

        mbLog( "Cut appointment id %ld (%s)", FormInfo_p->appointId, appoint_p->desc );

        sprintf( buf, "%ld", pmcAppointCutBuf_q->size );
        MainForm->CutBufferSizeLabel->Caption = buf;
        MainForm->AppointCutBufLabel->Caption = appoint_p->desc;
    }
    else
    {
        mbDlgDebug(( "Error cutting appointment %d", FormInfo_p->appointId ));
        // Error deleting appointment
        mbFree( appoint_p );
    }
    mbLockRelease( pmcAppointCutBuf_q->lock );

exit:
    return;
}

//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupCancelClick(TObject *Sender)
{
    AppListFormPopupDeleteCancel( TRUE );
}

//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupDeleteClick(TObject *Sender)
{
    AppListFormPopupDeleteCancel( FALSE );
}

//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupDeleteCancel
(
    Int32u_t                    cancelFlag
)
{
    PmcSqlPatient_p     pat_p;
    Char_p              buf_p;
    Char_p              buf1_p;
    Char_p              buf2_p;
    MbDateTime          dateTime;
    Ints_t              i;
    Int32u_t            action;
    Boolean_t           found = FALSE;
    Int32u_t            appId;

    mbCalloc( buf_p, 256 );
    mbCalloc( buf1_p, 256 );
    mbCalloc( buf2_p, 256 );
    mbCalloc( pat_p, sizeof( PmcSqlPatient_t ) );

    pmcSqlPatientDetailsGet( FormInfo_p->patientId, pat_p );

    mbLockAcquire( AppListLock );

    for( i = 0 ; i < AppArraySize ; i++ )
    {
        if( AppArray_pp[i]->id == FormInfo_p->appointId )
        {
            dateTime.SetDateTime( AppArray_pp[i]->date, AppArray_pp[i]->time );
            sprintf( buf1_p, dateTime.MDY_DateString( ) );
            sprintf( buf2_p, dateTime.HM_TimeString( ) );
            found = TRUE;
        }
    }
    mbLockRelease( AppListLock );

    // Sanity Check
    if( !found )
    {
        mbDlgDebug(( "Did not find app %ld in list", FormInfo_p->appointId ));
        goto exit;
    }

    sprintf( buf_p, "%s %s appointment on %s?",
        ( cancelFlag == TRUE ) ? "Cancel" : "Delete",
        buf2_p, buf1_p );

    action = ( cancelFlag == TRUE ) ? PMC_APP_ACTION_CANCEL : PMC_APP_ACTION_DELETE;

    if( mbDlgOkCancel( buf_p ) == MB_BUTTON_OK )
    {
        if( pmcSqlRecordDelete( PMC_SQL_TABLE_APPS, FormInfo_p->appointId ) == TRUE )
        {
            if( cancelFlag )
            {
                // Mark the appointment as cancelled
                pmcSqlExecInt(  PMC_SQL_TABLE_APPS, PMC_SQL_APPS_FIELD_COMPLETED,
                    PMC_APP_COMPLETED_STATE_CANCELLED, FormInfo_p->appointId );
            }
            appId = FormInfo_p->appointId;
            pmcAppHistory( appId, action, 0, 0, 0, 0, NIL );

            PreviousSortCode = PMC_APP_SORT_NONE;
            AppListGet( );
            AppListGrid->Invalidate( );

            mbLog( "%s appointment %d (%s %s)",
                ( cancelFlag == TRUE ) ? "Cancelled" : "Deleted",
                appId, pat_p->firstName, pat_p->lastName );

            MainForm->AppointmentCountUpdate( PMC_COUNTER_UPDATE );
        }
        else
        {
            mbDlgDebug(( "Error deleteing appointment %d", FormInfo_p->appointId ));
        }
    }

exit:

    mbFree( pat_p );
    mbFree( buf_p );
    mbFree( buf1_p );
    mbFree( buf2_p );
}

//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListGridDblClick(TObject *Sender)
{
    Int32s_t        i;
    bool            closeFlag = FALSE;
    bool            cancelledFlag = FALSE;

    DoubleClickFlag = TRUE;

    if( MouseInRow == -1 || MouseInRow == 0 )
    {
        return;
    }
    else
    {
        if( FormInfo_p->allowGoto )
        {
            mbLockAcquire( AppListLock );
            for( i = 0 ; i < AppArraySize ; i++ )
            {
                if( AppArray_pp[i]->id == FormInfo_p->appointId )
                {
                    //if( AppArray_pp[i]->completedState == PMC_APP_COMPLETED_STATE_CANCELLED )
                    if( AppArray_pp[i]->notDeleted == 0 )
                    {
                        cancelledFlag = TRUE;
                    }
                    else
                    {
                        FormInfo_p->providerId = AppArray_pp[i]->providerId;
                        FormInfo_p->date = AppArray_pp[i]->date;
                        FormInfo_p->gotGoto = TRUE;
                        closeFlag = TRUE;
                    }
                    break;
                }
            }
            mbLockRelease( AppListLock );

            if( cancelledFlag ) mbDlgInfo( "Cannot go to a cancelled or deleted appointment." );
        }
        if( closeFlag ) Close( );
    }
}

//---------------------------------------------------------------------------

void __fastcall TAppListForm::PatientNameEditMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if( FormInfo_p->allowPatientSelect )
    {
        PatientSelectButtonClick( Sender );
    }
}
//---------------------------------------------------------------------------

void __fastcall TAppListForm::PatientSelectButtonClick(TObject *Sender)
{
    TPatientListForm   *patListForm_p;
    PmcPatListInfo_t    patListInfo;

    patListInfo.patientId = 0;
    patListInfo.providerId = 0;
    patListInfo.mode = PMC_LIST_MODE_SELECT;
    patListInfo.character = 0;
    patListInfo.allowGoto = FALSE;

    patListForm_p = new TPatientListForm( this, &patListInfo );
    patListForm_p->ShowModal( );
    delete patListForm_p;

    AppListGrid->SetFocus( );

    if( patListInfo.returnCode == MB_BUTTON_OK )
    {
        if( patListInfo.patientId != FormInfo_p->patientId )
        {
            FormInfo_p->patientId = patListInfo.patientId;
            UpdatePatient( );

            if( FutureAppsCheckBox->Checked == FALSE &&
                PastAppsCheckBox->Checked == FALSE )
            {
                // No appointments are selected for display.  Force future appointments
                Active = FALSE;
                FutureAppsCheckBox->Checked = TRUE;
                Active = TRUE;
            }
            FormInfo_p->appointId = 0;
            PreviousSortCode = PMC_APP_SORT_NONE;
            AppListGet( );
            UpdateSelectedAppointment( );
        }
    }
    else
    {
        // User pressed cancel button - do nothing
    }
}
//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupGotoClick(TObject *Sender)
{
    Int32s_t    i;
    bool        match = FALSE;

    mbLockAcquire( AppListLock );
    for( i = 0 ; i < AppArraySize ; i++ )
    {
        if( AppArray_pp[i]->id == FormInfo_p->appointId )
        {
            FormInfo_p->providerId = AppArray_pp[i]->providerId;
            FormInfo_p->date = AppArray_pp[i]->date;
            FormInfo_p->gotGoto = TRUE;
            match = TRUE;
            break;
        }
    }
    mbLockRelease( AppListLock );

    if( match ) Close( );

}
//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupCompleteClick
(
    TObject *Sender
)
{
    Int32u_t    completedFlag;
    Char_p      cmd_p;
    bool        update = FALSE;
    Int32u_t    action;

    mbMalloc( cmd_p, 256 );

    if( FormInfo_p->appointId == 0 ) goto exit;

    sprintf( cmd_p, "select %s from %s where %s=%ld",
             PMC_SQL_APPS_FIELD_COMPLETED,
             PMC_SQL_TABLE_APPS,
             PMC_SQL_FIELD_ID,
             FormInfo_p->appointId );

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
        pmcSqlExecInt(  PMC_SQL_TABLE_APPS, PMC_SQL_APPS_FIELD_COMPLETED, completedFlag, FormInfo_p->appointId );
        mbLog( "Set appointment %d completed state to %d\n", FormInfo_p->appointId, completedFlag );
        pmcAppHistory( FormInfo_p->appointId, action, 0, 0, 0, 0, NIL );
    }

    PreviousSortCode = PMC_APP_SORT_NONE;
    AppListGet( );
    UpdateSelectedAppointment( );

exit:

    mbFree( cmd_p );
    return;
}

//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupConfirmCheckClick(
      TObject *Sender)
{
    pmcViewAppointmentConfirmation( FormInfo_p->appointId );
}
//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupConfirmPhoneClick
(
    TObject *Sender
)
{
    Ints_t  i;

    mbLockAcquire( AppListLock );
    for( i = 0 ; i < AppArraySize ; i++ )
    {
        if( AppArray_pp[i]->id == FormInfo_p->appointId )
        {
            FormInfo_p->providerId = AppArray_pp[i]->providerId;
            break;
        }
    }
    mbLockRelease( AppListLock );

    pmcAppointmentConfirmPhone( FormInfo_p->appointId, FormInfo_p->providerId, TRUE );

    PreviousSortCode = PMC_APP_SORT_NONE;
    AppListGet( );
    UpdateSelectedAppointment( );
}
//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupConfirmLetterClick(
      TObject *Sender)
{
    pmcAppointmentConfirmLetter( FormInfo_p->appointId, TRUE );

    PreviousSortCode = PMC_APP_SORT_NONE;
    AppListGet( );
    UpdateSelectedAppointment( );
}
//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupConfirmLetterCancelClick(
      TObject *Sender)
{
    pmcAppointmentConfirmLetter( FormInfo_p->appointId, FALSE );

    PreviousSortCode = PMC_APP_SORT_NONE;
    AppListGet( );
    UpdateSelectedAppointment( );
}
//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupConfirmPhoneCancelClick(
      TObject *Sender)
{
    pmcAppointmentConfirmPhone( FormInfo_p->appointId, 0, FALSE );

    PreviousSortCode = PMC_APP_SORT_NONE;
    AppListGet( );
    UpdateSelectedAppointment( );
}
//---------------------------------------------------------------------------


void __fastcall TAppListForm::AppListFormPopupAppTypeSet( Int32u_t appointId, Int32u_t appType )
{
    if( appointId == 0 ) return;

    pmcSqlExecInt(  PMC_SQL_TABLE_APPS, PMC_SQL_APPS_FIELD_TYPE, appType, appointId );
    pmcAppHistory( appointId, PMC_APP_ACTION_TYPE, appType, 0, 0, 0, NIL );

    PreviousSortCode = PMC_APP_SORT_NONE;
    AppListGet( );
    UpdateSelectedAppointment( );
    return;

}
//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupTypeNewClick(TObject *Sender)
{
    AppListFormPopupAppTypeSet( FormInfo_p->appointId, PMC_APP_TYPE_NEW );
}
//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupTypeUrgentClick(
      TObject *Sender)
{
    AppListFormPopupAppTypeSet( FormInfo_p->appointId, PMC_APP_TYPE_URGENT );
}
//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupTypeReviewClick(
      TObject *Sender)
{
    AppListFormPopupAppTypeSet( FormInfo_p->appointId, PMC_APP_TYPE_REVIEW );
}
//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupTypeClearClick(
      TObject *Sender)
{
    AppListFormPopupAppTypeSet( FormInfo_p->appointId, PMC_APP_TYPE_NONE );
}
//---------------------------------------------------------------------------

void __fastcall TAppListForm::PatientEditView( Int32u_t mode )
{
    pmcPatEditInfo_t        patEditInfo;
    TPatientEditForm       *patEditForm;

    if( FormInfo_p->patientId )
    {
        patEditInfo.patientId = FormInfo_p->patientId;
        patEditInfo.mode = mode;
        sprintf( patEditInfo.caption, "Patient Details" );

        patEditForm = new TPatientEditForm( NULL, &patEditInfo );
        patEditForm->ShowModal();
        delete patEditForm;

        if( patEditInfo.returnCode == MB_BUTTON_OK )
        {
            UpdatePatient( );
        }
        else
        {
            // User must have clicked cancel button
        }
    }
}

void __fastcall TAppListForm::PatientViewButtonClick(TObject *Sender)
{
    PatientEditView( PMC_EDIT_MODE_VIEW );

    AppListGrid->Col = PMC_APP_LIST_COL_CONFIRMED_LETTER;
    if( Active ) AppListGrid->SetFocus( );
}

//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupPatientEditClick(
      TObject *Sender)
{
    PatientEditView( PMC_EDIT_MODE_EDIT );

}
//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupPatientViewClick
(
      TObject *Sender
)
{
    PatientEditView( PMC_EDIT_MODE_VIEW );
}

//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupPatientPrintRecordClick
(
    TObject *Sender
)
{
    if( FormInfo_p->patientId == 0 )
    {
        mbDlgExclaim( "Appointment has no patient." );
    }
    else
    {
        pmcPatRecordHTML( FormInfo_p->patientId );
    }
}

//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupPatientPrintAddLblClick(
      TObject *Sender)
{
    if( FormInfo_p->patientId == 0 )
    {
        mbDlgExclaim( "Appointment has no patient." );
    }
    else
    {
        pmcLabelPrintPatAddress( FormInfo_p->patientId);
    }
}

//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupPatientPrintReqLblClick
(
    TObject *Sender
)
{
    Ints_t  i;
    bool    match = FALSE;

    mbLockAcquire( AppListLock );
    for( i = 0 ; i < AppArraySize ; i++ )
    {
        if( AppArray_pp[i]->id == FormInfo_p->appointId )
        {
            FormInfo_p->providerId = AppArray_pp[i]->providerId;
            match = TRUE;
            break;
        }
    }
    mbLockRelease( AppListLock );

    if( match )
    {
        pmcLabelPrintPatReq( FormInfo_p->appointId, FormInfo_p->providerId, 0 );
    }
    else
    {
        mbDlgDebug(( "Could not find appointment\n" ));
    }
}

//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupPatientPrintAppLetterClick
(
    TObject *Sender
)
{
    pmcAppointmentConfirmLetter( FormInfo_p->appointId, TRUE );

    PreviousSortCode = PMC_APP_SORT_NONE;
    AppListGet( );
    UpdateSelectedAppointment( );
}

//---------------------------------------------------------------------------

void __fastcall TAppListForm::OkButtonClick(TObject *Sender)
{
    FormInfo_p->returnCode = MB_BUTTON_OK;
    Close( );
}

//---------------------------------------------------------------------------



void __fastcall TAppListForm::AppLegendDrawGridDrawCell
(
    TObject        *Sender,
    int             ACol,
    int             ARow,
    TRect          &Rect,
    TGridDrawState  State
)
{
    Int32s_t    color;
    AnsiString  str = "";

    color = pmcAppLegendColor[ARow];
    str   = pmcAppLegendString_p[ARow];

    AppLegendDrawGrid->Canvas->Brush->Color = (TColor)color;
    AppLegendDrawGrid->Canvas->FillRect( Rect );
    AppLegendDrawGrid->Canvas->TextRect( Rect, Rect.Left + 2, Rect.Top, str );

    AppLegendDrawGrid->Row = 6;
    AppLegendDrawGrid->TopRow = 0;

    if( Active )
    {
        AppListGrid->SetFocus( );
    }
}

//---------------------------------------------------------------------------

void __fastcall TAppListForm::AppListFormPopupHistoryClick(TObject *Sender)
{
   pmcAppHistoryForm( FormInfo_p->appointId );
}
//---------------------------------------------------------------------------



