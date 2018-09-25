//---------------------------------------------------------------------------
// File:    pmcAppLettersForm.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 11, 2001
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#include <stdio.h>
#include <vcl.h>
#pragma hdrstop

#include "mbUtils.h"

#include "pmcGlobals.h"
#include "pmcDateSelectForm.h"
#include "pmcTables.h"
#include "pmcUtils.h"
#include "pmcMainForm.h"
#include "pmcAppLettersForm.h"
#include "pmcAppHistoryForm.h"

#pragma package(smart_init)
#pragma link "CGAUGES"
#pragma resource "*.dfm"

//---------------------------------------------------------------------------

__fastcall TAppLettersForm::TAppLettersForm(TComponent* Owner)
    : TForm(Owner)
{
    mbDlgDebug(( "Error: default constructor called" ));
}

//---------------------------------------------------------------------------

__fastcall TAppLettersForm::TAppLettersForm
(
    TComponent*     Owner,
    Int32u_t        selectedDate,
    Int32u_t        providerId
)
: TForm(Owner)
{
    MbDateTime      dateTime;

    StartDate = selectedDate;
    EndDate = StartDate;

    // Initiaize the strings in the edit boxes
    dateTime.SetDate( StartDate );
    StartDateEdit->Text = dateTime.MDY_DateString( );

    dateTime.SetDate( EndDate );
    EndDateEdit->Text = dateTime.MDY_DateString( );

    EndDateEdit->Enabled = FALSE;
    StartDateEdit->Enabled = FALSE;

    PrintReportCheckBox->Checked = TRUE;
    PrintReportCheckBox->Enabled = TRUE;

    PrintLabelsCheckBox->Enabled = FALSE;
    PrintLabelsCheckBox->Checked = FALSE;

    MarkAsConfirmedCheckBox->Enabled = TRUE;
    MarkAsConfirmedCheckBox->Checked = TRUE;

    SkipLetterPrintCheckBox->Enabled = TRUE;
    SkipLetterPrintCheckBox->Checked = FALSE;

    ReportAddressesCheckBox->Enabled = TRUE;
    ReportAddressesCheckBox->Checked = FALSE;

    ProviderId = pmcProviderListBuild( ProviderCombo, providerId, FALSE, TRUE );

    // Initialize queue of appointments
    App_q = qInitialize( &AppQueueHead );

    // Create a default name for the address merge file
    sprintf( AddressMergeFileName, "%s\\address_merge.txt", pmcCfg[CFG_MERGE_DIR].str_p );
    AddressMergeEdit->Text = AddressMergeFileName;
    AddressMergeFileDialog->InitialDir = pmcCfg[CFG_MERGE_DIR].str_p;
    AddressMergeFileDialog->FileName = "address_merge.txt";

    SucceededCount = 0;
    FailedCount = 0;

    // Get list of appointments
    AppointmentListGet( );
}
//---------------------------------------------------------------------------

__fastcall TAppLettersForm::~TAppLettersForm( void )
{
    // Clean out the list of appointments
    AppointmentListFree( );

    pmcProviderListFree( ProviderCombo );
}

//---------------------------------------------------------------------------

void __fastcall TAppLettersForm::StartDateButtonClick(TObject *Sender)
{
    TDateSelectForm        *dateSelectForm_p;
    MbDateTime              dateTime;
    pmcDateSelectInfo_t     dateSelectInfo;

    dateSelectInfo.mode = PMC_DATE_SELECT_PARMS;
    dateSelectInfo.string_p = NIL;
    dateSelectInfo.dateIn = StartDate;
    dateSelectInfo.caption_p = "Select Start Date";

    dateSelectForm_p = new TDateSelectForm
    (
        NULL,
        &dateSelectInfo
    );
    dateSelectForm_p->ShowModal( );
    delete dateSelectForm_p;

    if( dateSelectInfo.returnCode == MB_BUTTON_OK )
    {
        StartDate = dateSelectInfo.dateOut;
        dateTime.SetDate( StartDate );
        StartDateEdit->Text = dateTime.MDY_DateString( );
        AppointmentListGet( );
    }
    StartButton->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TAppLettersForm::EndDateButtonClick(TObject *Sender)
{
    TDateSelectForm    *dateSelectForm_p;
    pmcDateSelectInfo_t dateSelectInfo;

    dateSelectInfo.mode = PMC_DATE_SELECT_PARMS;
    dateSelectInfo.string_p = NIL;
    dateSelectInfo.dateIn = EndDate;
    dateSelectInfo.caption_p = "Select End Date";

    dateSelectForm_p = new TDateSelectForm
    (
        NULL,
        &dateSelectInfo
    );

    dateSelectForm_p->ShowModal( );
    delete dateSelectForm_p;

    if( dateSelectInfo.returnCode == MB_BUTTON_OK )
    {
        EndDate = dateSelectInfo.dateOut;
        MbDateTime dateTime = MbDateTime( EndDate, 0 );

        EndDateEdit->Text = dateTime.MDY_DateString( );
        AppointmentListGet( );
    }
    StartButton->SetFocus();

}
//---------------------------------------------------------------------------

void __fastcall TAppLettersForm::AppointmentListGet( void )
{
    pmcPatAppLetter_p   app_p;
    Char_p              cmd_p = NIL;
    bool                confirmed;
    MbSQL               sql;

    //First, delete any appointments in the list
    AppointmentListFree( );

    if( mbMalloc( cmd_p, 512 ) == NIL ) goto exit;

    if( EndDate < StartDate ) goto exit;

    // Format SQL command
    sprintf( cmd_p, "select %s,%s,%s,%s,%s,%s,%s,%s from %s where %s>=%ld and %s<=%ld and %s=%ld and %s !=0 and %s=%ld",
        PMC_SQL_FIELD_ID,
        PMC_SQL_FIELD_PATIENT_ID,
        PMC_SQL_APPS_FIELD_START_TIME,
        PMC_SQL_FIELD_DATE,
        PMC_SQL_FIELD_PROVIDER_ID,
        PMC_SQL_APPS_FIELD_CONF_LETTER_DATE,
        PMC_SQL_APPS_FIELD_CONF_LETTER_TIME,
        PMC_SQL_APPS_FIELD_CONF_LETTER_ID,
        PMC_SQL_TABLE_APPS,
        PMC_SQL_FIELD_DATE, StartDate,
        PMC_SQL_FIELD_DATE, EndDate,
        PMC_SQL_FIELD_PROVIDER_ID, ProviderId,
        PMC_SQL_FIELD_PATIENT_ID,
        PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

    if( sql.Query( cmd_p ) == FALSE ) goto exit;

    while( sql.RowGet( ) )
    {
        mbMalloc( app_p, sizeof( pmcPatAppLetter_t ) );
        memset( app_p, 0, sizeof( pmcPatAppLetter_t ) );

        app_p->id               = sql.NmInt32u( PMC_SQL_FIELD_ID                    );
        app_p->day              = sql.NmInt32u( PMC_SQL_FIELD_DATE                  );
        app_p->startTime        = sql.NmInt32u( PMC_SQL_APPS_FIELD_START_TIME       );
        app_p->patientId        = sql.NmInt32u( PMC_SQL_FIELD_PATIENT_ID            );
        app_p->providerId       = sql.NmInt32u( PMC_SQL_FIELD_PROVIDER_ID           );
        app_p->confLetterDay    = sql.NmInt32u( PMC_SQL_APPS_FIELD_CONF_LETTER_DATE );
        app_p->confLetterTime   = sql.NmInt32u( PMC_SQL_APPS_FIELD_CONF_LETTER_TIME );
        app_p->confLetterId     = sql.NmInt32u( PMC_SQL_APPS_FIELD_CONF_LETTER_ID   );

        // Compute the appointment time as a 64 bit integer for sorting
        app_p->dateTimeInt64 = (unsigned __int64)app_p->day;
        app_p->dateTimeInt64 *= 1000000;
        app_p->dateTimeInt64 += (unsigned __int64)app_p->startTime;

        confirmed = FALSE;
        if( PrintRadio->ItemIndex == 0 )
        {
            // We want unconfirmed appointments only
            if( app_p->confLetterTime == app_p->startTime &&
                app_p->confLetterDay  == app_p->day &&
                app_p->confLetterId   == app_p->providerId )
            {
                confirmed = TRUE;
            }
        }

        if( confirmed )
        {
            // Don't bother with this appointment that is already confirmed
            mbFree( app_p );
        }
        else
        {
            // Insert into queue.  Sort later
            qInsertFirst( App_q, app_p );
        }
     }
     nbDlgDebug(( "Read %ld appointment records", App_q->size ));

exit:
    sprintf( cmd_p, "%ld appointment%s match%s the criteria", App_q->size,
        ( App_q->size == 1 ) ? "" : "s",
        ( App_q->size == 1 ) ? "es" : "" );

    AppointmentCountLabel->Caption = cmd_p;
    mbFree( cmd_p );
    return;
}

//---------------------------------------------------------------------------

void __fastcall TAppLettersForm::FormClose(TObject *Sender, TCloseAction &Action)
{
    Action = caFree;
}
//---------------------------------------------------------------------------

void __fastcall TAppLettersForm::CancelButtonClick(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------

void __fastcall TAppLettersForm::StartButtonClick(TObject *Sender)
{
    pmcPatAppLetter_p   app_p, app2_p;
    qHead_t             app2;
    qHead_p             app2_q;
    bool                added;
    bool                close = TRUE;
    Char_p              b;
    Char_p              add1_p;
    Char_p              add2_p;
    Char_p              add3_p;
    Char_p              add4_p;

    Ints_t              l;
    FILE               *fp = NIL;

    mbMalloc( b, 256 );
    mbMalloc( add1_p, 1024 );
    mbMalloc( add2_p, 1024 );
    mbMalloc( add3_p, 1024 );
    mbMalloc( add4_p, 1024 );

    if( App_q->size == 0 )
    {
        mbDlgExclaim( "No appointments letters to print." );
        close = FALSE;
        goto exit;
    }

    // Before starting the letters, ensure that the address merge file
    // is successfully opened. Do not want to generate the letters and
    // then fail to produce the address merge file
    fp = fopen( AddressMergeEdit->Text.c_str(), "w" );
    if( fp == NIL )
    {
        mbDlgError( "Could not open address merge file '%s'", AddressMergeEdit->Text.c_str( ) );
        close = FALSE;
        goto exit;
    }

    fprintf( fp, "\"ADDRESS1\",\"ADDRESS2\",\"ADDRESS3\",\"ADDRESS4\"\n" );

    Thermometer->MinValue = (Int32u_t)0;
    Thermometer->MaxValue = (Int32u_t)App_q->size;

    // Sort the appointments by date
    app2_q = qInitialize( &app2 );

    // First move all into temp queue
    for( ; ; )
    {
        if( qEmpty( App_q ) ) break;
        app_p = (pmcPatAppLetter_p)qRemoveFirst( App_q );
        qInsertFirst( app2_q, app_p );
    }

    // Move back into global queue in sorted order
    for( ; ; )
    {
        if( qEmpty( app2_q ) ) break;

        app_p = (pmcPatAppLetter_p)qRemoveFirst( app2_q );

        // Insert into list... sort by date and time
        added = FALSE;

        qWalk( app2_p, App_q, pmcPatAppLetter_p )
        {
            if( app_p->dateTimeInt64 < app2_p->dateTimeInt64 )
            {
                qInsertBefore( App_q, app2_p, app_p );

                added = TRUE;
                break;
            }
        }
        if( added == FALSE )
        {
            qInsertLast( App_q, app_p );
        }
    }

    MarkAsConfirmedCheckBox->Checked;

    // Done sorting list.  Now generate letters
    qWalk( app_p, App_q, pmcPatAppLetter_p )
    {
        pmcPatAppLetterMake( app_p, MarkAsConfirmedCheckBox->Checked, SkipLetterPrintCheckBox->Checked );

        if( app_p->result == PMC_APP_LETTER_RESULT_SUCCESS )
        {
            SucceededCount++;
        }
        else
        {
            FailedCount++;
        }

        Thermometer->Progress++;
    }

    // Next, write out address merge file for all sucessfully produced letters
    // Do not give user option to skip this as they might really wish they did
    // it after skipping it

    qWalk( app_p, App_q, pmcPatAppLetter_p )
    {
        if( app_p->result == PMC_APP_LETTER_RESULT_SUCCESS )
        {
            pmcMakeMailingAddress( app_p->subStr[ PMC_SUB_STR_TITLE ],
                                   app_p->subStr[ PMC_SUB_STR_FIRST_NAME ],
                                   app_p->subStr[ PMC_SUB_STR_LAST_NAME ],
                                   app_p->subStr[ PMC_SUB_STR_ADDRESS1 ],
                                   app_p->subStr[ PMC_SUB_STR_ADDRESS2 ],
                                   app_p->subStr[ PMC_SUB_STR_CITY ],
                                   app_p->subStr[ PMC_SUB_STR_PROVINCE ],
                                   app_p->subStr[ PMC_SUB_STR_POSTAL_CODE ],
                                   add1_p, add2_p, add3_p, add4_p, TRUE );

            fprintf( fp, "\"%s\",\"%s\",\"%s\",\"%s\"\n", add1_p, add2_p, add3_p, add4_p );
        }
    }

    // Finally, generate a report on the success of the operation.

    if( PrintReportCheckBox->Checked == TRUE )
    {
        PrintReport( );
    }

    // Generate a closing dialog
    l  = sprintf( b,   "%ld appointment%s processed.\n",  App_q->size, ( App_q->size == 1 ) ? "" : "s" );
    l += sprintf( b+l, "%ld appointment letter%s %s printed.\n", SucceededCount,
         ( SucceededCount == 1 ) ? "" : "s",
         ( SkipLetterPrintCheckBox->Checked ) ? "could be" : "were" );

    l += sprintf( b+l, "%ld appointment letter%s %s not be printed.", FailedCount,
        ( FailedCount == 1 ) ? "" : "s",
        ( SkipLetterPrintCheckBox->Checked ) ? "would" : "could" );
        
    mbDlgInfo( b );

exit:

    if( fp )
    {
        fclose( fp );

        sprintf( add1_p, "%s", pmcMakeFileName( pmcCfg[CFG_MERGE_DIR].str_p, add2_p ) );
        sprintf( add2_p, "%s.txt", add1_p );

        // Archive the address merge file
        if( mbFileCopy( AddressMergeEdit->Text.c_str(), add2_p ) != MB_RET_OK )
        {
            mbDlgExclaim( "Error: failed to archive address merge file %s.\nContact system administrator.", add2_p );
        }
    }

    mbFree( b );
    mbFree( add1_p );
    mbFree( add2_p );
    mbFree( add3_p );
    mbFree( add4_p );

    if( close )  Close();
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TAppLettersForm::AppointmentListFree( void )
{
    pmcPatAppLetter_p   app_p;

    for( ; ; )
    {
        if( qEmpty( App_q ) ) break;

        app_p = (pmcPatAppLetter_p)qRemoveFirst( App_q );

        pmcPatAppLetterFree( app_p );
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void    pmcPatAppLetterMake
(
    pmcPatAppLetter_p   app_p,
    bool                markAsConfirmed,
    bool                skipPrint
)
{
    Char_p          buf1_p;
    Char_p          buf_p;
    Char_p          fileName_p;
    Char_p          spoolFileName_p;
    Char_p          flagFileName_p;
    FILE           *in_p = NIL;
    FILE           *out_p = NIL;
    Ints_t          i, p, j;
    Boolean_t       foundAddress2;
    Boolean_t       match;
    Int32u_t        deceased;
    PmcSqlDoctor_p  doctor_p;
    MbSQL           sql;
    MbDateTime      dateTime;

    mbMalloc( buf_p, 512 );
    mbMalloc( buf1_p, 512 );
    mbMalloc( fileName_p, 128 );
    mbMalloc( spoolFileName_p, 128 );
    mbMalloc( flagFileName_p, 128 );
    mbMalloc( doctor_p, sizeof(PmcSqlDoctor_t) );

    dateTime.SetDateTime( app_p->day, app_p->startTime );

    app_p->result = PMC_APP_LETTER_RESULT_FAILURE;

    mbMallocStr( app_p->subStr[ PMC_SUB_STR_APP_TIME ], dateTime.HM_TimeString( ) );
    mbMallocStr( app_p->subStr[ PMC_SUB_STR_APP_DATE ], dateTime.DMY_DateStringLong( ) );

    dateTime.SetDate( mbToday( ) );

    mbMallocStr( app_p->subStr[ PMC_SUB_STR_PRINT_DATE ], dateTime.DMY_DateStringLong( ) );

    pmcProviderDescGet( app_p->providerId, buf_p );
    mbMalloc( app_p->subStr[ PMC_SUB_STR_PROVIDER_DESC ], strlen( buf_p ) + 1 );
    strcpy( app_p->subStr[ PMC_SUB_STR_PROVIDER_DESC ], buf_p );

    if( app_p->patientId )
    {
        // OK, lets get the patient information

        //                      0   1  2  3  4  5  6  7  8
        sprintf( buf_p, "select %s,%s,%s,%s,%s,%s,%s,%s,%s from %s where %s=%ld",
            PMC_SQL_FIELD_FIRST_NAME,                   // 0
            PMC_SQL_FIELD_LAST_NAME,                    // 1
            PMC_SQL_FIELD_TITLE,                        // 2
            PMC_SQL_FIELD_ADDRESS1,                     // 3
            PMC_SQL_FIELD_ADDRESS2,                     // 4
            PMC_SQL_FIELD_CITY,                         // 5
            PMC_SQL_FIELD_PROVINCE,                     // 6
            PMC_SQL_FIELD_POSTAL_CODE,                  // 7
            PMC_SQL_PATIENTS_FIELD_DATE_OF_DEATH,       // 8
            PMC_SQL_TABLE_PATIENTS,
            PMC_SQL_FIELD_ID,
            app_p->patientId );

        app_p->result = PMC_APP_LETTER_RESULT_DB_FAILED;
        if( sql.Query( buf_p ) == FALSE ) goto exit;
        if( sql.RowCount( ) != 1 )
        {
            app_p->result = PMC_APP_LETTER_RESULT_NO_PATIENT;
            goto exit;
        }
        if( sql.RowGet( ) == FALSE ) goto exit;

        mbMallocStr( app_p->subStr[PMC_SUB_STR_FIRST_NAME], sql.NmString( PMC_SQL_FIELD_FIRST_NAME  ) );
        mbMallocStr( app_p->subStr[PMC_SUB_STR_LAST_NAME],  sql.NmString( PMC_SQL_FIELD_LAST_NAME   ) );
        mbMallocStr( app_p->subStr[PMC_SUB_STR_TITLE],      sql.NmString( PMC_SQL_FIELD_TITLE       ) );
        mbMallocStr( app_p->subStr[PMC_SUB_STR_ADDRESS1],   sql.NmString( PMC_SQL_FIELD_ADDRESS1    ) );
        mbMallocStr( app_p->subStr[PMC_SUB_STR_ADDRESS2],   sql.NmString( PMC_SQL_FIELD_ADDRESS2    ) );
        mbMallocStr( app_p->subStr[PMC_SUB_STR_CITY],       sql.NmString( PMC_SQL_FIELD_CITY        ) );
        mbMallocStr( app_p->subStr[PMC_SUB_STR_PROVINCE],   sql.NmString( PMC_SQL_FIELD_PROVINCE    ) );
        mbMallocStr( app_p->subStr[PMC_SUB_STR_POSTAL_CODE],sql.NmString( PMC_SQL_FIELD_POSTAL_CODE ) );
        deceased = sql.NmInt32u( PMC_SQL_PATIENTS_FIELD_DATE_OF_DEATH );
    }
    else
    {
        app_p->result = PMC_APP_LETTER_RESULT_NO_PATIENT;
        goto exit;
    }

    // 20041228: Move the opening of the template file to before the checks.
    // Add a loop that checks to see what fields are in fact required.

    // Construct name of the template file
    sprintf( fileName_p, "%s\\provider%ld_pat_app.html", pmcCfg[CFG_TEMPLATE_DIR].str_p, app_p->providerId );

    in_p = fopen( fileName_p, "r" );
    if( in_p == NIL )
    {
        app_p->result = PMC_APP_LETTER_RESULT_NO_TEMPLATE;
        goto exit;
    }

    while( fgets( buf_p, 512, in_p ) != 0 )
    {
        for( i = 0 ; i < PMC_SUB_STR_COUNT ; i++ )
        {
            p = mbStrPos( buf_p, pmcSubStrings[i] );
            if( p >= 0 )
            {
                // Found a match
                app_p->subStrRequired[i] = TRUE;
            }
        }
    }
    rewind( in_p );

    if( app_p->referringId > 0 && app_p->subStrRequired[ PMC_SUB_STR_REF_DR ] > 0 )
    {
        if( pmcSqlDoctorDetailsGet( app_p->referringId, doctor_p ) )
        {
            mbMallocStr( app_p->subStr[ PMC_SUB_STR_REF_DR ], doctor_p->lastName );
        }
        else
        {
            app_p->subStr[ PMC_SUB_STR_REF_DR ] = NIL;
        }
    }

    // This is a special case... not required even if in template
    app_p->subStrRequired[ PMC_SUB_STR_ADDRESS2 ] = FALSE;

    if( deceased > 0 )                                    { app_p->result = PMC_APP_LETTER_RESULT_DECEASED;         goto exit; }

    // 20041228: Obsoleted by new loop below
#if 0
    if( app_p->subStr[ PMC_SUB_STR_FIRST_NAME ]  == NIL ) { app_p->result = PMC_APP_LETTER_RESULT_NO_FIRST_NAME;    goto exit; }
    if( app_p->subStr[ PMC_SUB_STR_LAST_NAME ]   == NIL ) { app_p->result = PMC_APP_LETTER_RESULT_NO_LAST_NAME;     goto exit; }
    if( app_p->subStr[ PMC_SUB_STR_TITLE ]       == NIL ) { app_p->result = PMC_APP_LETTER_RESULT_NO_TITLE;         goto exit; }
    if( app_p->subStr[ PMC_SUB_STR_ADDRESS1 ]    == NIL ) { app_p->result = PMC_APP_LETTER_RESULT_NO_ADDRESS;       goto exit; }
    if( app_p->subStr[ PMC_SUB_STR_CITY ]        == NIL ) { app_p->result = PMC_APP_LETTER_RESULT_NO_CITY;          goto exit; }
    if( app_p->subStr[ PMC_SUB_STR_PROVINCE ]    == NIL ) { app_p->result = PMC_APP_LETTER_RESULT_NO_PROVINCE;      goto exit; }
    if( app_p->subStr[ PMC_SUB_STR_POSTAL_CODE ] == NIL ) { app_p->result = PMC_APP_LETTER_RESULT_NO_POSTAL_CODE;   goto exit; }

    // Check to see if we have the required information
    if( strlen( app_p->subStr[ PMC_SUB_STR_FIRST_NAME ] )   == 0 ) { app_p->result = PMC_APP_LETTER_RESULT_NO_FIRST_NAME;   goto exit; }
    if( strlen( app_p->subStr[ PMC_SUB_STR_LAST_NAME ] )    == 0 ) { app_p->result = PMC_APP_LETTER_RESULT_NO_LAST_NAME;    goto exit; }
    if( strlen( app_p->subStr[ PMC_SUB_STR_TITLE ] )        == 0 ) { app_p->result = PMC_APP_LETTER_RESULT_NO_TITLE;        goto exit; }
    if( strlen( app_p->subStr[ PMC_SUB_STR_ADDRESS1 ] )     == 0 ) { app_p->result = PMC_APP_LETTER_RESULT_NO_ADDRESS;      goto exit; }
    if( strlen( app_p->subStr[ PMC_SUB_STR_CITY ] )         == 0 ) { app_p->result = PMC_APP_LETTER_RESULT_NO_CITY;         goto exit; }
    if( strlen( app_p->subStr[ PMC_SUB_STR_PROVINCE ] )     == 0 ) { app_p->result = PMC_APP_LETTER_RESULT_NO_PROVINCE;     goto exit; }
    if( strlen( app_p->subStr[ PMC_SUB_STR_POSTAL_CODE ])   == 0 ) { app_p->result = PMC_APP_LETTER_RESULT_NO_POSTAL_CODE;  goto exit; }
#endif

    // Check for required information
    for( i = 0 ; i < PMC_SUB_STR_COUNT ; i++ )
    {
        if( app_p->subStrRequired[i] )
        {
            app_p->result = pmcSubStringErrCode[i];
            if( app_p->subStr[i] == NIL ) goto exit;
            if( strlen( app_p->subStr[i] ) == 0 ) goto exit;
        }
    }
    app_p->result = PMC_APP_LETTER_RESULT_FAILURE;
    // 20041228: End of changes

    if( skipPrint )
    {
        // At this point, skip printing.  We have got the required information
        // to generate a report
        fclose( in_p );
        in_p = NIL;
        app_p->result = PMC_APP_LETTER_RESULT_SUCCESS;
        goto exit;
    }

    sprintf( fileName_p, "%s.html", pmcMakeFileName( NIL, buf1_p ) );

    sprintf( spoolFileName_p, "%s\\spool\\%s", pmcCfg[CFG_PMCDESPL_DIR].str_p, fileName_p );
    sprintf( flagFileName_p,  "%s\\flags\\%s", pmcCfg[CFG_PMCDESPL_DIR].str_p, fileName_p );

    out_p = fopen( spoolFileName_p, "w" );
    if( out_p == NIL )
    {
        mbDlgDebug(( "Error opening temporary appointment letter file '%s'", spoolFileName_p ));
        app_p->result = PMC_APP_LETTER_RESULT_OUTPUT_FILE_FAILED;
        goto exit;
    }

    while( fgets( buf_p, 512, in_p ) != 0 )
    {
        foundAddress2 = FALSE;

        // Loop until no matches are found
        for( ; ; )
        {
            match = FALSE;
            for( i = 0 ; i < PMC_SUB_STR_COUNT ; i++ )
            {
                p = mbStrPos( buf_p, pmcSubStrings[i] );
                if( p >= 0 )
                {
                    // Found a match
                    match = TRUE;
                    // Look for an "address2" match which is a special case
                    if( mbStrPos( buf_p, pmcSubStrings[PMC_SUB_STR_ADDRESS2] ) >= 0 )
                    {
                        foundAddress2 = TRUE;
                    }

                    // Copy all characters up to match into second buffer
                    for( j = 0 ; j < p ; j++ )
                    {
                        *(buf1_p+j) = *(buf_p+j);
                    }
                   *(buf1_p+j) = 0;

                    // Now add the substitution string
                    if( app_p->subStr[i] )
                    {
                        strcat( buf1_p, app_p->subStr[i] );
                    }

                    // Add the remainder of the original line
                    strcat( buf1_p, buf_p+p+strlen( pmcSubStrings[i] )  );

                    // Copy modified string back into original buffer
                    strcpy( buf_p, buf1_p );
                }
            }
            // Only stop looking when no matches are found
            if( match == FALSE ) break;
        }
        // Write updated line to output file.  Suppress output if the substitution
        // string __ADDRESS2__ appeared in the line, and the line is less than
        // four bytes long

        if( !foundAddress2 ||
            ( foundAddress2 && strlen( buf_p ) > 5 ) )
        {
            fprintf( out_p, buf_p );
        }
    }

    if( out_p ) fclose( out_p );

    // Write flag file to trigger despooler
    out_p = fopen( flagFileName_p, "w" );
    if( out_p == NIL )
    {
        mbDlgDebug(( "Could not open flag file '%s'", flagFileName_p ));
    }
    else
    {
        fprintf( out_p, "%s", flagFileName_p );
        fclose( out_p );
    }

    if( markAsConfirmed )
    {
        sprintf( buf_p, "update %s set %s=%ld,%s=%ld,%s=%ld where %s=%ld",
            PMC_SQL_TABLE_APPS,
            PMC_SQL_APPS_FIELD_CONF_LETTER_TIME, app_p->startTime,
            PMC_SQL_APPS_FIELD_CONF_LETTER_DATE, app_p->day,
            PMC_SQL_APPS_FIELD_CONF_LETTER_ID,   app_p->providerId,
            PMC_SQL_FIELD_ID, app_p->id );
        pmcSqlExec( buf_p );

        pmcAppHistory( app_p->id, PMC_APP_ACTION_CONFIRM_LETTER, app_p->day, app_p->startTime, app_p->providerId, 0 , NIL );
    }

    app_p->result = PMC_APP_LETTER_RESULT_SUCCESS;

exit:

    if( in_p ) fclose( in_p );

    mbFree( buf_p );
    mbFree( buf1_p );
    mbFree( fileName_p );
    mbFree( spoolFileName_p );
    mbFree( flagFileName_p );
    mbFree( doctor_p );

    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void pmcPatAppLetterFree
(
   pmcPatAppLetter_p   app_p
)
{
    Ints_t  i;

    for( i = 0 ; i < PMC_SUB_STR_COUNT ; i++ )
    {
        if( app_p->subStr[i] ) mbFree( app_p->subStr[i] );
    }
    mbFree( app_p );

    return;
}

void __fastcall TAppLettersForm::Button1Click(TObject *Sender)
{
    AddressMergeFileDialog->Filter = "Text files (*.txt)|*.TXT";
    AddressMergeFileDialog->FileName = AddressMergeEdit->Text;
    AddressMergeFileDialog->Execute();

    StartButton->SetFocus();
}

//---------------------------------------------------------------------------

void __fastcall TAppLettersForm::ProviderComboChange( TObject *Sender )
{
    ProviderId = pmcProviderIdGet( ProviderCombo->Text.c_str() );
    AppointmentListGet( );
}
//---------------------------------------------------------------------------

void __fastcall TAppLettersForm::PrintRadioClick( TObject *Sender )
{
    // Must rebuild the list
    AppointmentListGet( );
}
//---------------------------------------------------------------------------

#define PMC_APP_REPORT_SPACES "                                    "

void __fastcall TAppLettersForm::PrintReport( void )
{
    FILE               *fp;
    pmcPatAppLetter_p   app_p;
    MbDateTime          dateTime;
    Char_p              buf1_p;
    Char_p              buf2_p;
    Char_p              buf3_p;
    Char_p              spoolFileName_p;
    Char_p              flagFileName_p;
    Char_p              fileName_p;
    Int32u_t            i;

    mbMalloc( buf1_p, 128 );
    mbMalloc( buf2_p, 128 );
    mbMalloc( buf3_p, 128 );
    mbMalloc( spoolFileName_p, 128 );
    mbMalloc( flagFileName_p, 128 );
    mbMalloc( fileName_p, 128 );

    sprintf( fileName_p, "%s.html", pmcMakeFileName( NIL, buf1_p ) );

    sprintf( spoolFileName_p, "%s\\spool\\%s", pmcCfg[CFG_PMCDESPL_DIR].str_p, fileName_p );
    sprintf( flagFileName_p,  "%s\\flags\\%s", pmcCfg[CFG_PMCDESPL_DIR].str_p, fileName_p );

    nbDlgDebug(( "spool '%s'\nflags: '%s'\n", spoolFileName_p, flagFileName_p ));

    fp = fopen( spoolFileName_p, "w" );
    if( fp == NIL )
    {
        mbDlgDebug(( "Error opening appointment letter report file '%s'", spoolFileName_p ));
        goto exit;
    }

    fprintf( fp, "<HTML><HEAD><Title>Appointment Letter Report</TITLE></HEAD><BODY>\n" );
    fprintf( fp, "<H2>Appointment Letter Report</H2><HR>\n" );
    fprintf( fp, "<PRE WIDTH = 80>\n" );

    fprintf( fp, "Provider:                    %s\n", pmcProviderDescGet( ProviderId, buf1_p ) );
    dateTime.SetDate( StartDate );
    fprintf( fp, "Start Date:                  %s\n", dateTime.MDY_DateString( ) );
    dateTime.SetDate( EndDate );
    fprintf( fp, "End Date:                    %s\n", dateTime.MDY_DateString( ) );
    fprintf( fp, "Print (Unconfirmed/All):     %s\n", ( PrintRadio->ItemIndex == 0 ) ? "Unconfirmed" : "All" );
    fprintf( fp, "Mark as Confirmed (Yes/No):  %s\n", ( MarkAsConfirmedCheckBox->Checked ) ? "Yes" : "No" );
    fprintf( fp, "Skip Letter Print (Yes/No):  %s\n", ( SkipLetterPrintCheckBox->Checked ) ? "Yes" : "No" );
    fprintf( fp, "Address Merge File          '%s'\n", AddressMergeEdit->Text );

    dateTime.SetDateTime( mbToday( ), mbTime( ) );
    fprintf( fp, "Report Generated:            %s %s\n", dateTime.MDY_DateString( ), dateTime.HM_TimeString( ) );

    fprintf( fp, "\n\n" );
    fprintf( fp, "The following appointment letters %s printed:\n\n",
            (  SkipLetterPrintCheckBox->Checked ) ? "would not be" : "were not" );

    i = 1;

    qWalk( app_p, App_q, pmcPatAppLetter_p )
    {
        if( app_p->result != PMC_APP_LETTER_RESULT_SUCCESS )
        {
            dateTime.SetDateTime( app_p->day, app_p->startTime );

            sprintf( buf1_p, "___ %-15.15s %9.9s   Reason: %s  (%s %s)",
                     dateTime.MDY_DateString( ), dateTime.HM_TimeString( ),
                     pmcPatAppLetterResultStrings[ app_p->result ],
                     app_p->subStr[PMC_SUB_STR_FIRST_NAME], app_p->subStr[PMC_SUB_STR_LAST_NAME] );

            fprintf( fp, "%3d %-75.75s\n", i++, buf1_p );
        }
    }

    fprintf( fp, "\n\n" );
    fprintf( fp, "The following appointment letters %s printed:\n\n",
            (  SkipLetterPrintCheckBox->Checked ) ? "would be" : "were" );

    i = 1;

    qWalk( app_p, App_q, pmcPatAppLetter_p )
    {
        if( app_p->result == PMC_APP_LETTER_RESULT_SUCCESS )
        {
            dateTime.SetDateTime( app_p->day, app_p->startTime );

            sprintf( buf1_p, "___ %-15.15s %9.9s   %s %s %s",
                     dateTime.MDY_DateString( ), dateTime.HM_TimeString( ),
                     app_p->subStr[ PMC_SUB_STR_TITLE ],
                     app_p->subStr[PMC_SUB_STR_FIRST_NAME], app_p->subStr[PMC_SUB_STR_LAST_NAME] );

            fprintf( fp, "%3d %-75.75s\n", i++, buf1_p );

            if( ReportAddressesCheckBox->Checked == TRUE )
            {
                // Print addresses in report file as well
                if( strlen( app_p->subStr[ PMC_SUB_STR_ADDRESS1 ] ) ) fprintf( fp, "%s%s\n", PMC_APP_REPORT_SPACES, app_p->subStr[ PMC_SUB_STR_ADDRESS1 ] );
                if( strlen( app_p->subStr[ PMC_SUB_STR_ADDRESS2 ] ) ) fprintf( fp, "%s%s\n", PMC_APP_REPORT_SPACES, app_p->subStr[ PMC_SUB_STR_ADDRESS2 ] );
                sprintf( buf2_p, "" );
                if( strlen( app_p->subStr[ PMC_SUB_STR_CITY ]  ) )
                {
                    sprintf( buf3_p, "%s, ", app_p->subStr[ PMC_SUB_STR_CITY ] );
                    strcat( buf2_p, buf3_p );
                }
                if( strlen( app_p->subStr[ PMC_SUB_STR_PROVINCE ]  ) )
                {
                    sprintf( buf3_p, "%s, ", app_p->subStr[ PMC_SUB_STR_PROVINCE ] );
                    strcat( buf2_p, buf3_p );
                }
                if( strlen( app_p->subStr[ PMC_SUB_STR_POSTAL_CODE ]  ) )
                {
                    sprintf( buf3_p, "%s", app_p->subStr[ PMC_SUB_STR_POSTAL_CODE ] );
                    strcat( buf2_p, buf3_p );
                }
                fprintf( fp, "%s%s\n\n", PMC_APP_REPORT_SPACES, buf2_p );
            }

        }
    }

    fprintf( fp, "</PRE><HR>\n\n" );
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
    }

exit:

    mbFree( buf1_p );
    mbFree( buf2_p );
    mbFree( buf3_p );
    mbFree( spoolFileName_p );
    mbFree( flagFileName_p );
    mbFree( fileName_p );
}

//---------------------------------------------------------------------------

void __fastcall TAppLettersForm::AddressMergeFileDialogCanClose(TObject *Sender,
          bool &CanClose)
{
   nbDlgDebug(( "called, canClose: %ld", CanClose ));
   AddressMergeEdit->Text = AddressMergeFileDialog->FileName;
}

//---------------------------------------------------------------------------

void __fastcall TAppLettersForm::SkipLetterPrintCheckBoxClick(TObject *Sender)
{
    if( SkipLetterPrintCheckBox->Checked == TRUE )
    {
        MarkAsConfirmedCheckBox->Checked = FALSE;
        PrintReportCheckBox->Checked = TRUE;
    }
}

//---------------------------------------------------------------------------

void __fastcall TAppLettersForm::MarkAsConfirmedCheckBoxClick(
      TObject *Sender)
{
    // If setting mark as confirmed TRUE, disable skip print
    if( MarkAsConfirmedCheckBox->Checked == TRUE )
    {
        SkipLetterPrintCheckBox->Checked = FALSE;
    }
}
//---------------------------------------------------------------------------

