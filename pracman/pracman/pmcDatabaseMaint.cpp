//---------------------------------------------------------------------------
// File:    pmcDatabaseMaint.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 17, 2001
//---------------------------------------------------------------------------
// Description:
//
// Functions for maintaining the Practice Manager Database.
//---------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <dir.h>
#include <vcl.h>
#pragma hdrstop

#include "mbUtils.h"

#include "pmcMsp.h"
#include "pmcTables.h"
#include "pmcColors.h"
#include "pmcGlobals.h"
#include "pmcClaimEditform.h"
#include "pmcUtils.h"
#include "pmcDateSelectForm.h"
#include "pmcDatabaseMaint.h"
#include "pmcMainForm.h"
#include "pmcPatientEditForm.h"
#include "pmcReportForm.h"
#include "pmcMspReturns.h"
#include "pmcAppHistoryForm.h"
#include "pmcDocumentEditForm.h"
#include "pmcBatchImportForm.h"

#if PMC_SCRAMBLE_PATIENT_NAMES
#define INIT_GLOBALS
#include "pmcNames.h"
#undef  INIT_GLOBALS
#endif

#define PMC_MSP_EXCEL_DR_LIST_FIELDS 4

//---------------------------------------------------------------------------
// Function: pmcDatabaseCheckTimeslots()
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void pmcDatabaseCheckTimeslots( void )
{
    mbDlgDebug(( "called" ));
    return;
}
//---------------------------------------------------------------------------
// Function: pmcDatabaseEchosAssignPatients()
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void pmcDatabaseEchosAssignPatients( void )
{
    Char_p          buf_p = NIL;
    qHead_t         echoQueue;
    qHead_p         echo_q;
    PmcEchoCheck_p  echo_p;
    TThermometer   *thermometer_p = NIL;
    Boolean_t       cancelled = FALSE;
    Boolean_t       namedFlag = FALSE;
    MbSQL           sql;
    Char_t          firstName[128];
    Char_t          lastName[128];
    Int32s_t        i;
    Int32u_t        id;


    echo_q = qInitialize( &echoQueue );

    mbMalloc( buf_p, 512 );

    if( mbDlgYesNo( "Assign patients to echos?" ) == MB_BUTTON_NO ) goto exit;

    sprintf( buf_p, "select %s,%s,%s,%s from %s where %s>0 and %s=0 and %s=%lu",
        PMC_SQL_FIELD_ID,                       // 0
        PMC_SQL_ECHOS_FIELD_NAME,               // 1
        PMC_SQL_FIELD_DATE,                     // 2
        PMC_SQL_FIELD_PROVIDER_ID,              // 3

        PMC_SQL_TABLE_ECHOS,

        PMC_SQL_FIELD_ID,
        PMC_SQL_FIELD_PATIENT_ID,
        PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );


    sql.Query( buf_p );

    sprintf( buf_p, "Checking %lu echo records...", sql.RowCount( ) );
    thermometer_p = new TThermometer( buf_p, 0, sql.RowCount(), TRUE );

    while( sql.RowGet( ) )
    {
        mbCalloc( echo_p, sizeof( PmcEchoCheck_t ) );

        qInsertFirst( echo_q, echo_p );

        // Get the patient ID
        echo_p->id           = sql.Int32u( 0 );
        echo_p->date         = sql.Int32u( 2 );
        echo_p->providerId   = sql.Int32u( 3 );
        mbMallocStr( echo_p->name_p, sql.String( 1 ) );
    }

    qWalk( echo_p, echo_q, PmcEchoCheck_p )
    {
        if( thermometer_p->Increment( ) == FALSE || cancelled == TRUE )
        {
            cancelled = FALSE;
            if( mbDlgYesNo( "Cancel assign patients to echos?" ) == MB_BUTTON_YES )
            {
                cancelled = TRUE;
                break;
            }
        }

        namedFlag = FALSE;
        mbLog( "Echo: %lu name '%s'\n", echo_p->id, echo_p->name_p );

        memset( firstName, 0 , 128 );
        memset( lastName, 0, 128 );

        for( i = 0 ; i < 128 ; i++ )
        {
            if( *( echo_p->name_p + i ) == ' ' ) break;

            firstName[i] = *( echo_p->name_p + i );
        }
        strncpy( lastName, ( echo_p->name_p + i + 1), 128 );
        mbStrClean( firstName, NIL, TRUE );
        mbStrClean( lastName, NIL, TRUE );

        mbLog( "Got name '%s' first '%s' last '%s'\n", echo_p->name_p, firstName, lastName );

        if( strlen( firstName ) && strlen( lastName ) )
        {
            sprintf( buf_p, "select id from patients where first_name = \"%s\" and last_name = \"%s\"",
                firstName, lastName );

            if( sql.Query( buf_p ) )
            {
                if( sql.RowCount() == 1 )
                {
                    sql.RowGet();
                    id = sql.Int32u( 0 );
                    if( id !=0 )
                    {
                        sprintf( buf_p, "update echos set patient_id = %u where id = %lu",
                            id, echo_p->id );
                        sql.Update( buf_p );
                        namedFlag = TRUE;
                    }
                }
            }
        }

        if( namedFlag == FALSE )
        {
            if( pmcEchoPatSelect( echo_p->name_p, echo_p->date, 4, &id, TRUE  ) == MB_RETURN_OK )
            {
                if( id != 0 )
                {
                    mbLog( "Set echo id %lu patient id to %lu\n", echo_p->id, id );
                    sprintf( buf_p, "update echos set patient_id = %u where id = %lu", id, echo_p->id );
                    sql.Update( buf_p );
                }  
                cancelled = FALSE;
            }
            else
            {
                cancelled = TRUE;
            }
        }
    }

exit:

    if( thermometer_p ) delete thermometer_p;

    while( !qEmpty( echo_q ) )
    {
        echo_p = (PmcEchoCheck_p)qRemoveFirst( echo_q );
        mbFree( echo_p->name_p );
        mbFree( echo_p );
    }
    mbFree( buf_p );
    return;
}

//---------------------------------------------------------------------------
// Function: pmcCheckPatientRecords()
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void pmcCheckPatientsUnreferenced( void )
{
    Char_p          buf_p = NIL;
    qHead_t         pat_qhead;
    qHead_p         pat_q;
    patCheck_p      pat_p;
    TThermometer   *thermometer_p = NIL;
    MbSQL           sql;
    Int32u_t        total = 0;
    Int32u_t        unreferenced = 0;
    Int32u_t        checked = 0;

    pat_q = qInitialize( &pat_qhead );

    mbMalloc( buf_p, 512 );

    if( mbDlgYesNo( "Check database for unreferenced patients?" ) == MB_BUTTON_CANCEL ) goto exit;

    sprintf( buf_p, "select %s from %s where %s>0 and %s=%lu and created < 20030404",
        PMC_SQL_FIELD_ID,                       // 0
        PMC_SQL_TABLE_PATIENTS,
        PMC_SQL_FIELD_ID,
        PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

    sql.Query( buf_p );

    total =  sql.RowCount( );

    thermometer_p = new TThermometer( "", 0, sql.RowCount(), TRUE );

    while( sql.RowGet( ) )
    {
        mbCalloc( pat_p, sizeof( patCheck_t ) );

        qInsertFirst( pat_q, pat_p );

        // Get the patient ID
        pat_p->id           = sql.Int32u( 0 );
    }

    qWalk( pat_p, pat_q, patCheck_p )
    {
        sprintf( buf_p, "Remaining: %u  Deleted: %u", total - checked, unreferenced );
        thermometer_p->Caption = buf_p;

        checked++;

        if( thermometer_p->Increment( ) == FALSE )
        {
            if( mbDlgYesNo( "Cancel check for unreferences patients?" ) == MB_BUTTON_YES )
            {
                break;
            }
        }

        // Check for appointments
        sprintf( buf_p, "select %s from %s where %s=%lu",
             PMC_SQL_FIELD_ID,
             PMC_SQL_TABLE_APPS,
             PMC_SQL_FIELD_PATIENT_ID, pat_p->id );

        sql.Query( buf_p );
        if( sql.RowCount( ) > 0 )
        {
            mbLog( "patient %lu has appointments\n", pat_p->id );
            continue;
        }
        else
        {
 //           mbLog( "patient %lu has NO appointments\n", pat_p->id );
        }

        // Check for claims
        sprintf( buf_p, "select %s from %s where %s=%lu",
             PMC_SQL_FIELD_ID,
             PMC_SQL_TABLE_CLAIMS,
             PMC_SQL_FIELD_PATIENT_ID, pat_p->id );

        sql.Query( buf_p );
        if( sql.RowCount( ) > 0 )
        {
            mbLog( "patient %lu has claims\n", pat_p->id );
            continue;
        }
        else
        {
//            mbLog( "patient %lu has NO claims\n", pat_p->id );
        }

        // Check for documents
        sprintf( buf_p, "select %s from %s where %s=%lu",
             PMC_SQL_FIELD_ID,
             PMC_SQL_TABLE_DOCUMENTS,
             PMC_SQL_FIELD_PATIENT_ID, pat_p->id );

        sql.Query( buf_p );
        if( sql.RowCount( ) > 0 )
        {
            mbLog( "patient %lu has documents\n", pat_p->id );
            continue;
        }
        else
        {
//            mbLog( "patient %lu has NO documents\n", pat_p->id );
        }

        // Check for echos
        sprintf( buf_p, "select %s from %s where %s=%lu",
             PMC_SQL_FIELD_ID,
             PMC_SQL_TABLE_ECHOS,
             PMC_SQL_FIELD_PATIENT_ID, pat_p->id );

        sql.Query( buf_p );
        if( sql.RowCount( ) > 0 )
        {
            mbLog( "patient %lu has echos\n", pat_p->id );
            continue;
        }
        else
        {
//            mbLog( "patient %lu has NO echos\n", pat_p->id );
        }
        unreferenced++;
        sprintf( buf_p, "update %s set %s=%d where %s=%lu",
            PMC_SQL_TABLE_PATIENTS,
            PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_FALSE_VALUE,
            PMC_SQL_FIELD_ID, pat_p->id );

        mbLog( "Patient %lu is not referenced in the database, cmd: '%s'\n", pat_p->id, buf_p );
        sql.Update( buf_p );
    }

exit:

    if( thermometer_p ) delete thermometer_p;

    mbDlgInfo( "Deleted %lu unreferenced patients.",  unreferenced );

    while( !qEmpty( pat_q ) )
    {
        pat_p = (patCheck_p)qRemoveFirst( pat_q );
        mbFree( pat_p );
    }
    mbFree( buf_p );
    return;
}

//---------------------------------------------------------------------------
// Function: pmcCheckPatientRecords()
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void pmcCheckPatientRecords( void )
{
    Char_p          buf_p = NIL;
    Char_p          buf2_p = NIL;
    Char_p          cmd_p = NIL;
    qHead_t         pat_qhead;
    qHead_p         pat_q;
    patCheck_p      pat_p;
    Int32u_t        i;
    Int32u_t        matchCount = 0;
    Int32u_t        rowCount;
    FILE           *fp = NIL;
    Char_p          fileName_p = NIL;
    TThermometer   *thermometer_p = NIL;
    Boolean_t       cancelled = FALSE;
    Boolean_t       ran = FALSE;
    MbSQL           sql;

    mbMalloc( buf_p, 2048 );
    mbMalloc( buf2_p, 256 );
    mbMalloc( cmd_p, 256 );
    mbMalloc( fileName_p, 256 );

    pat_q = qInitialize( &pat_qhead );

    mbLog( "pmcCheckPatientRecords() called \n" );

    if( mbDlgOkCancel( "Check all patient records?" ) == MB_BUTTON_CANCEL ) goto exit;

    ran = TRUE;

    sprintf( buf_p, "select %s,%s,%s,%s,%s,%s,%s,%s,%s from %s where %s>0 and %s=%ld",
        PMC_SQL_FIELD_ID,                       // 0
        PMC_SQL_FIELD_FIRST_NAME,               // 1
        PMC_SQL_FIELD_LAST_NAME,                // 2
        PMC_SQL_FIELD_TITLE,                    // 3
        PMC_SQL_PATIENTS_FIELD_GENDER,          // 4
        PMC_SQL_PATIENTS_FIELD_REFERRING_DR_ID, // 5
        PMC_SQL_FIELD_PROVIDER_ID,              // 6
        PMC_SQL_PATIENTS_FIELD_PHN,             // 7
        PMC_SQL_PATIENTS_FIELD_PHN_PROV,        // 8

        PMC_SQL_TABLE_PATIENTS,
        PMC_SQL_FIELD_ID,
        PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );


    sql.Query( buf_p );

    sprintf( buf_p, "Processing %ld patient records...", sql.RowCount() );
    thermometer_p = new TThermometer( buf_p, 0, sql.RowCount(), TRUE );

    while( sql.RowGet( ) )
    {
        mbCalloc( pat_p, sizeof( patCheck_t ) );

        qInsertFirst( pat_q, pat_p );

        // Get the patient ID
        pat_p->id           = sql.Int32u( 0 );
        pat_p->refDrId      = sql.Int32u( 5 );
        pat_p->providerId   = sql.Int32u( 6 );

        // Get the first name
        mbMallocStr( pat_p->firstName_p, sql.String( 1 ) );

        // Get the last name
        mbMallocStr( pat_p->lastName_p, sql.String( 2 ));

        // Get title
        mbMallocStr( pat_p->title_p, sql.String( 3 ) );

        pat_p->gender = sql.Int32u( 4 );
    }

    sprintf( fileName_p, "%s_patient_report.txt", pmcMakeFileName( pmcCfg[CFG_REPORT_DIR].str_p, buf_p ) );
    fp = fopen( fileName_p, "w" );

    if( fp == NIL )
    {
        mbDlgDebug(( "Failed to open report file '%s'", fileName_p ));
        goto exit;
    }



    for( i = 0 ; ; i++ )
    {
        if( thermometer_p->Increment( ) == FALSE )
        {
            if( mbDlgYesNo( "Cancel check of patient records?" ) == MB_BUTTON_YES )
            {
                cancelled = TRUE;
                break;
            }
        }

        if( qEmpty( pat_q ) ) break;
        pat_p = (patCheck_p)qRemoveFirst( pat_q );

        // Check to see if default provider can be set (this will help sanity
        // checking on claims entry).  If no provider, guess based on appointments
        if( pat_p->providerId == 0 )
        {
            Int32u_t    appProviderId;
            // Query the appointents table
            sprintf( cmd_p, "select %s from %s where %s=%ld and %s=1",
                PMC_SQL_FIELD_PROVIDER_ID,
                PMC_SQL_TABLE_APPS,
                PMC_SQL_FIELD_PATIENT_ID, pat_p->id,
                PMC_SQL_FIELD_NOT_DELETED );

            sql.Query( cmd_p );
            rowCount = 0;
            while( sql.RowGet( ) )
            {
                appProviderId = sql.Int32u( 0 );
                rowCount++;
            }

            if( rowCount )
            {
                // Check that provider exists
                sprintf( cmd_p, "select %s from %s where %s=%ld and %s=%ld",
                    PMC_SQL_FIELD_ID,
                    PMC_SQL_TABLE_PROVIDERS,
                    PMC_SQL_FIELD_ID, appProviderId,
                    PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

                pmcSqlSelectInt( cmd_p, &matchCount );
                if( matchCount == 1 )
                {
                    mbLog( "Setting patient id %ld provider to %ld\n", pat_p->id, appProviderId );
                    pmcSqlExecInt( PMC_SQL_TABLE_PATIENTS, PMC_SQL_FIELD_PROVIDER_ID, appProviderId, pat_p->id );
                }
                else
                {
                    mbDlgExclaim( "Could not find provider %ld", appProviderId );
                }
            }
        }

        sprintf( buf_p, "" );

        // Check date of birth
        if( pat_p->dob )
        {
            Int32u_t    today = mbToday( );
            if( pat_p->dob > today )
            {
                sprintf( buf2_p, "- Invalid date of birth\n" );
                strcat( buf_p, buf2_p );
            }
            else if( ( today - pat_p->dob ) < 10000 )
            {
                sprintf( buf2_p, "- Patient less than 1 year of age\n" );
                strcat( buf_p, buf2_p );
            }
        }

        if( pat_p->refDrId )
        {
            // Check that the referring Dr exists
            sprintf( cmd_p, "select %s from %s where %s=%ld and %s=%ld",
                PMC_SQL_FIELD_ID,
                PMC_SQL_TABLE_DOCTORS,
                PMC_SQL_FIELD_ID, pat_p->refDrId,
                PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

            pmcSqlSelectInt( cmd_p, &matchCount );
            if( matchCount == 0 )
            {
                sprintf( buf2_p, " - Could not specified referring doctor in database" );
                strcat( buf_p, buf2_p );
            }
            else if( matchCount > 1 )
            {
                sprintf( buf2_p, " - Error locating referring doctor in database (match count = %ld)\n", matchCount );
                strcat( buf_p, buf2_p );
             }
        }

        if( strlen( pat_p->title_p ) == 0 )
        {
            sprintf( buf2_p, "- No title\n" );
            strcat( buf_p, buf2_p );
        }
        if( strlen( pat_p->firstName_p ) == 0 )
        {
            sprintf( buf2_p, "- No first name\n" );
            strcat( buf_p, buf2_p );
        }

        if( strlen( pat_p->lastName_p ) == 0 )
        {
            sprintf( buf2_p, "- No last name\n" );
            strcat( buf_p, buf2_p );
        }

        // Now check genders against title
        if( strlen( pat_p->title_p ) != 0 )
        {
            if( pat_p->gender == PMC_SQL_GENDER_MALE )
            {
                if( mbStrPos( pat_p->title_p, "Mrs"  ) >= 0 ||
                    mbStrPos( pat_p->title_p, "Ms"   ) >= 0 ||
                    mbStrPos( pat_p->title_p, "Miss" ) >= 0  )
                {
                    sprintf( buf2_p, "- Suspect gender setting\n" );
                    strcat( buf_p, buf2_p );
                }
            }
            else if( pat_p->gender == PMC_SQL_GENDER_FEMALE )
            {
                if( mbStrPos( pat_p->title_p, "Mr."  ) >= 0)
                {
                    sprintf( buf2_p, "- Suspect gender setting\n" );
                    strcat( buf_p, buf2_p );
                }
            }
            else
            {
                sprintf( buf2_p, "- Invalid gender setting\n" );
                strcat( buf_p, buf2_p );
            }
        }

        if( strlen( buf_p ) )
        {
            if( mbDlgOkCancel( "Patient record for '%s %s %s' contains following problems:\n\n%s\n\nEdit record?",
                                pat_p->title_p, pat_p->firstName_p, pat_p->lastName_p, buf_p ) == MB_BUTTON_OK )
            {
                // Edit patient record
                pmcPatEditInfo_t        patEditInfo;
                TPatientEditForm       *patEditForm;

                patEditInfo.patientId = pat_p->id;
                patEditInfo.mode = PMC_EDIT_MODE_EDIT;
                sprintf( patEditInfo.caption, "Edit Patient Details - %s, %s",
                    pat_p->lastName_p, pat_p->firstName_p );

                patEditForm = new TPatientEditForm( NULL, &patEditInfo );
                patEditForm->ShowModal();
                delete patEditForm;
            }
        }

        mbFree( pat_p->firstName_p );
        mbFree( pat_p->lastName_p );
        mbFree( pat_p->title_p );

        mbFree( pat_p );
    }

exit:

    while( !qEmpty( pat_q ) )
    {
        pat_p = (patCheck_p)qRemoveFirst( pat_q );

        mbFree( pat_p->firstName_p );
        mbFree( pat_p->lastName_p );
        mbFree( pat_p->title_p );
        mbFree( pat_p );
    }


    if( fp ) fclose( fp );

    if( thermometer_p ) delete thermometer_p;

    mbFree( buf_p );
    mbFree( buf2_p );
    mbFree( cmd_p );
    mbFree( fileName_p );

    if( ran )
    {
        mbDlgInfo( "Patient records check %s.\n", cancelled ? "cancelled" : "complete" );
    }

    return;
}

//---------------------------------------------------------------------------
// Function: pmcDatabaseCheckDocuments()
//---------------------------------------------------------------------------
// Description:
//
// This is a temporary function to extract all documents for a provider.
//---------------------------------------------------------------------------

#define PMC_DOC_UPDATE  0

void pmcDatabaseCheckDocuments( void )
{
    Char_p                      buf_p;
    Char_p                      buf2_p;
    Char_p                      buf3_p;
    pmcDocumentMaintStruct_p    document_p;
    qHead_t                     documentQueue;
    qHead_p                     document_q;
    qHead_t                     fileQueue;
    qHead_p                     file_q;
    qHead_t                     fileDoneQueue;
    qHead_p                     fileDone_q;
    AnsiString                  tempStr = "";
    TThermometer               *thermometer_p = NIL;
    Boolean_t                   cancelled = FALSE;
    Boolean_t                   ran = FALSE;
    PmcSqlPatient_p   pat_p;
    mbFileListStruct_p          file_p;
#if PMC_DOC_UPDATE
    TCursor                     cursorOrig;
    Boolean_t                   match;
#endif

    mbMalloc( buf_p,  1024 );
    mbMalloc( buf2_p, 1024 );
    mbMalloc( buf3_p, 1024 );
    mbMalloc( pat_p, sizeof( PmcSqlPatient_t ) );

    document_q = qInitialize( &documentQueue );
    file_q = qInitialize( &fileQueue );
    fileDone_q = qInitialize( &fileDoneQueue );

    if( mbDlgOkCancel( "Check all document records?" ) == MB_BUTTON_CANCEL )
    {
        goto exit;
    }

    if( mbDlgInfo( "Must update to new document directory structure" ) == MB_BUTTON_CANCEL )
    {
        goto exit;
    }
#if 0

    ran = TRUE;

    cursorOrig = Screen->Cursor;
    Screen->Cursor = crHourGlass;

    // Read list of files from storage directory
    mbFileListGet( pmcCfg[CFG_DOC_IMPORT_TO_DIR].str_p, "*.*", file_q, FALSE );

    // Read list of documents from the database.
    sprintf( buf_p, "select %s,%s,%s,%s,%s,%s,%s"
#if PMC_DOC_UPDATE
                    ",%s"
#endif
                    " from %s where %s=%ld",
        PMC_SQL_FIELD_NAME,
        PMC_SQL_DOCUMENTS_FIELD_ORIG_NAME,
        PMC_SQL_FIELD_PATIENT_ID,
        PMC_SQL_FIELD_ID,
        PMC_SQL_FIELD_TYPE,
        PMC_SQL_FIELD_CRC,
        PMC_SQL_FIELD_SIZE,
#if PMC_DOC_UPDATE
        PMC_SQL_FIELD_DESC,
#endif
        PMC_SQL_TABLE_DOCUMENTS,
        PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

    mbDlgDebug(( "obsolete section" ));
#if 0
    PMC_GENERAL_QUERY_SETUP( buf_p );
    while( !pmcDataSet_p->Eof )
    {
        mbCalloc( document_p, sizeof( pmcDocumentMaintStruct_t ) );

        tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_FIELD_NAME ] );
        mbMallocStr( document_p->name_p, tempStr.c_str( ) );

        tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_DOCUMENTS_FIELD_ORIG_NAME ] );
        mbMallocStr( document_p->origName_p, tempStr.c_str( ) );

        document_p->id = pmcDataSet_p->FieldValues[ PMC_SQL_FIELD_ID ];
        document_p->patientId = pmcDataSet_p->FieldValues[ PMC_SQL_FIELD_PATIENT_ID ];

        document_p->type = pmcDataSet_p->FieldValues[ PMC_SQL_FIELD_TYPE ];
        document_p->crc  = pmcDataSet_p->FieldValues[ PMC_SQL_FIELD_CRC ];
        document_p->size = pmcDataSet_p->FieldValues[ PMC_SQL_FIELD_SIZE ];

#if PMC_DOC_UPDATE
        tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_FIELD_DESC ] );
        mbMallocStr( document_p->desc_p, tempStr.c_str( ) );
#endif

        qInsertLast( document_q, document_p );

        pmcDataSet_p->Next( );
    }
    PMC_GENERAL_QUERY_CLEANUP( );
#endif

    Screen->Cursor = cursorOrig;

    // Next process the documents
    sprintf( buf_p, "Checking %ld document records...", document_q->size );
    thermometer_p = new TThermometer( buf_p, 0, document_q->size, TRUE );

    for( ; ; )
    {
        if( qEmpty( document_q ) ) break;

        if( thermometer_p->Increment( ) == FALSE )
        {
            if( mbDlgYesNo( "Cancel check of document records?" ) == MB_BUTTON_YES )
            {
                cancelled = TRUE;
                goto exit;
            }
        }

        document_p = (pmcDocumentMaintStruct_p)qRemoveFirst( document_q );

        match = FALSE;
 
        qWalk( file_p, file_q, mbFileListStruct_p )
        {
            if( strcmp( document_p->name_p, file_p->name_p ) == 0 )
            {
                match = TRUE;
                break;
            }
        }

#if PMC_DOC_UPDATE
        Char_p      result_p;
        Int32u_t    date;

        // Attempt to update the date in the documents
        if( match )
        {
            if( pmcDocSearchDate( file_p, &date ) )
            {
                sprintf( buf_p, "update %s set %s=%lu where %s=%ld",
                    PMC_SQL_TABLE_DOCUMENTS,
                    PMC_SQL_FIELD_DATE, date,
                    PMC_SQL_FIELD_ID, document_p->id );

                mbLog( buf_p );
                pmcSqlExec( buf_p );
            }
        }

        // This section of code is going to attempt to set the document
        // description based on the template used to create the document
        if( match && strlen( document_p->desc_p ) == 0 )
        {
            if( pmcDocSearchTemplate( file_p, &result_p ) )
            {
                sprintf( buf_p, "update %s set %s=\"%s\" where %s=%lu",
                    PMC_SQL_TABLE_DOCUMENTS,
                    PMC_SQL_FIELD_DESC,         result_p,
                    PMC_SQL_FIELD_ID,           document_p->id );

                mbLog( buf_p );
                pmcSqlExec( buf_p );

                // Lets make a new original filename for this document
                *buf2_p = 0;
                *buf3_p = 0;

                if( document_p->patientId )
                {
                    pmcSqlPatientNameGet( document_p->patientId, NIL, buf3_p, NIL );
                }
                if( strlen( buf3_p ) )
                {
                    sprintf( buf2_p, "%s-", buf3_p );
                }
                strcat( buf2_p, result_p );

                if( document_p->type > PMC_DOCUMENT_TYPE_ANY && document_p->type  < PMC_DOCUMENT_TYPE_INVALID )
                {
                    sprintf( buf3_p, "-%ld%s", document_p->id, pmcDocumentTypeExtStrings[ document_p->type ] );
                    strcat( buf2_p, buf3_p );

                    mbStrClean( buf2_p, buf3_p );

                    sprintf( buf_p, "update %s set %s=\"%s\" where %s=%lu",
                        PMC_SQL_TABLE_DOCUMENTS,
                        PMC_SQL_DOCUMENTS_FIELD_ORIG_NAME, buf3_p,
                        PMC_SQL_FIELD_ID,           document_p->id );

                    mbLog( buf_p );
                    pmcSqlExec( buf_p );
                }
                mbFree( result_p );
                result_p = NIL;
            }
            else
            {
                mbLog( "Could not find template for '%s'\n", file_p->name_p );
            }
        }
        else
        {
            // mbLog( "Found a document with existing desc '%s'\n", document_p->desc_p );
        }
#endif

        if( match )
        {
            // There was a bug in the document update stuff that was setting CRCs
            // and sizes to 0.
            if( document_p->crc == 0 )
            {
                mbLog( "Found a document with CRC = 0\n" );
                document_p->crc = mbCrcFile( file_p->fullName_p, &document_p->size, NIL, NIL, NIL, NIL, NIL );

                sprintf( buf_p, "update %s set %s=%lu,%s=%lu where %s=%lu",
                    PMC_SQL_TABLE_DOCUMENTS,
                    PMC_SQL_FIELD_CRC,      document_p->crc,
                    PMC_SQL_FIELD_SIZE,     document_p->size,
                    PMC_SQL_FIELD_ID,       document_p->id );

                mbLog( buf_p );
                pmcSqlExec( buf_p );
            }

            // Done with this file entry
            qRemoveEntry( file_q, file_p );
            qInsertLast( fileDone_q, file_p );
        }
        else
        {
            if( mbDlgYesNo( "Cannot locate file '%s' id: %ld\n"
                            "Continue with check?", document_p->name_p, document_p->id ) == MB_BUTTON_NO )
            {
                cancelled = TRUE;
                goto exit;
            }
        }

        // check that patient exists
        if( document_p->patientId )
        {
            if( pmcSqlPatientDetailsGet( document_p->patientId, pat_p ) == FALSE )
            {
               mbDlgInfo( "Could not find patient id %ld referenced by document id %ld.\n",
                               document_p->patientId, document_p->id );
            }
        }

        mbFree( document_p->name_p );
        mbFree( document_p->origName_p );
        mbFree( document_p->desc_p );
        mbFree( document_p );
    }

#endif
    
exit:

    if( thermometer_p ) delete thermometer_p;

    if( !cancelled )
    {
        while( !qEmpty( file_q ) )
        {
            file_p = (mbFileListStruct_p)qRemoveFirst( file_q );
            qInsertLast( fileDone_q, file_p );
            if( mbDlgYesNo( "File %s has no database record.\n"
                            "Continue with check?", file_p->name_p ) == MB_BUTTON_NO )
            {
                cancelled = TRUE;
                break;
            }
        }
    }

    mbFileListFree( file_q );
    mbFileListFree( fileDone_q );

    while( !qEmpty( document_q ) )
    {
        document_p = (pmcDocumentMaintStruct_p)qRemoveFirst( document_q );
        mbFree( document_p->name_p );
        mbFree( document_p->origName_p );
        mbFree( document_p->desc_p );
        mbFree( document_p );
    }

    if( ran )
    {
        mbDlgInfo( "Document check %s.\n", cancelled ? "cancelled" : "complete" );
    }

    mbFree( pat_p );
    mbFree( buf2_p );
    mbFree( buf3_p );
    mbFree( buf_p );
}

//---------------------------------------------------------------------------
// This function checks the files in the "documents_new" directory againts
// the active and new records in the database
//---------------------------------------------------------------------------


void pmcDatabaseCheckActiveDocuments( void )
{
    pmcDocumentMaintStruct_p    document_p;
    qHead_t                     documentQueue;
    qHead_p                     document_q;
    qHead_t                     fileQueue;
    qHead_p                     file_q;
    qHead_t                     fileDoneQueue;
    qHead_p                     fileDone_q;
    AnsiString                  tempStr = "";
    TThermometer               *thermometer_p = NIL;
    Boolean_t                   cancelled = FALSE;
    Boolean_t                   ran = FALSE;
    Boolean_t                   found;
    mbFileListStruct_p          file_p;
    MbString                    buf;
    MbSQL                       sql;
    Int32u_t                    size;

    document_q = qInitialize( &documentQueue );
    file_q = qInitialize( &fileQueue );
    fileDone_q = qInitialize( &fileDoneQueue );

    MbCursor cursor = MbCursor( crHourGlass );

    if( mbDlgOkCancel( "Check active records? (Nothing is changed; only logged)" ) == MB_BUTTON_CANCEL )
    {
        goto exit;
    }

    // Read list of files from storage directory
    mbFileListGet( pmcCfg[CFG_WORD_CREATE_DIR].str_p, "*.*", file_q, FALSE );

    // Read list of documents from the database.
    mbSprintf( &buf, "select %s,%s,%s,%s,%s,%s,%s"
                    " from %s where %s=%ld and %s=%ld and %s != %ld",
        PMC_SQL_FIELD_NAME,
        PMC_SQL_DOCUMENTS_FIELD_ORIG_NAME,
        PMC_SQL_FIELD_PATIENT_ID,
        PMC_SQL_FIELD_ID,
        PMC_SQL_FIELD_TYPE,
        PMC_SQL_FIELD_CRC,
        PMC_SQL_FIELD_SIZE,

        PMC_SQL_TABLE_DOCUMENTS,
        PMC_SQL_FIELD_NOT_DELETED,      PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE,
        PMC_SQL_FIELD_TYPE,             PMC_DOCUMENT_TYPE_WORD,
        PMC_SQL_DOCUMENTS_FIELD_STATUS, PMC_DOCUMENT_STATUS_FILED );

    // Execute the command
    sql.Query( buf.get( ) );

    // Loop through the results; store only the IDs
    while( sql.RowGet() )
    {
        mbCalloc( document_p, sizeof( pmcDocumentMaintStruct_t ) );

        mbMallocStr( document_p->name_p, sql.String( 0 ) );
        mbMallocStr( document_p->origName_p, sql.String( 1 ) );

        document_p->patientId = sql.Int32u( 2 );
        document_p->id = sql.Int32u( 3 );

        qInsertLast( document_q, document_p );
    }

    size = document_q->size + file_q->size;

    mbSprintf( &buf, "Checking %ld document records...", document_q->size );
    thermometer_p = new TThermometer( buf.get(), 0, size, TRUE );

    // This loop looks for records that have no matching files
    qWalk( document_p, document_q, pmcDocumentMaintStruct_p )
    {
        if( thermometer_p->Increment( ) == FALSE )
        {
            if( mbDlgYesNo( "Cancel docuemnt check?" ) == MB_BUTTON_YES ) goto exit;
        }

        found = FALSE;
        qWalk( file_p, file_q, mbFileListStruct_p )
        {
            // mbLog( "Comparing '%s' to '%s'\n", document_p->origName_p, file_p->name_p );
            if( strcmp( document_p->origName_p, file_p->name_p ) == 0 )
            {
                found = TRUE;
                break;
            }
        }

        if( !found )
        {
            mbLog( "Did not find file for record %lu (%s)\n", document_p->id, document_p->origName_p );
        }
    }

    // This loop looks for files that have no matching records
    qWalk( file_p, file_q, mbFileListStruct_p )
    {
        if( thermometer_p->Increment( ) == FALSE )
        {
            if( mbDlgYesNo( "Cancel docuemnt check?" ) == MB_BUTTON_YES ) goto exit;
        }

        found = FALSE;
        qWalk( document_p, document_q, pmcDocumentMaintStruct_p )
        {
            // mbLog( "Comparing '%s' to '%s'\n", document_p->origName_p, file_p->name_p );
            if( strcmp( document_p->origName_p, file_p->name_p ) == 0 )
            {
                found = TRUE;
                break;
            }
        }

        if( !found )
        {
            mbLog( "Did not find record for file: %s\n", file_p->name_p );
        }
    }

    ran = TRUE;
    
exit:

    if( thermometer_p ) delete thermometer_p;

    mbFileListFree( file_q );
    mbFileListFree( fileDone_q );

    while( !qEmpty( document_q ) )
    {
        document_p = (pmcDocumentMaintStruct_p)qRemoveFirst( document_q );
        mbFree( document_p->name_p );
        mbFree( document_p->origName_p );
        mbFree( document_p->desc_p );
        mbFree( document_p );
    }

    if( ran )
    {
        mbDlgInfo( "Document check %s.\n", cancelled ? "cancelled" : "complete" );
    }
}

//---------------------------------------------------------------------------
// Function: pmcDatabaseCheckClaimRecords()
//---------------------------------------------------------------------------
// Description:
//
// This function checks the claim database.  We must force the fee codes
// into a known format since we must rely on these for serching the database.
// Also, we must ensure that no set of claims has the same claim number
// and fee code and service date.
//---------------------------------------------------------------------------

void pmcDatabaseIncrementClaimNumbers( void )
{
    Char_p                      buf_p = NIL;
    qHead_t                     claimQueue;
    qHead_p                     claim_q;

    TThermometer               *thermometer_p = NIL;

    //Int32u_t                    minClaimNumber = 0;       // 2013_12_27
    //Int32u_t                    maxClaimNumber = 20000;   // 2013_12_27

    //Int32u_t                    minClaimNumber = 20000;     // 2014_12_29
    //Int32u_t                    maxClaimNumber = 30000;     // 2014_12_29

    //Int32u_t                    minClaimNumber = 30000;     // 2016_02_15
    //Int32u_t                    maxClaimNumber = 40000;     // 2016_02_15

    // NOTE: be wary that 999999 is a special case and cannot be changed
    //Int32u_t                    minClaimNumber = 40000;     // 2016_03_26
    //Int32u_t                    maxClaimNumber = 50000;     // 2016_03_26

    // Int32u_t                    minClaimNumber = 50000;     // 2019_02_05
    // Int32u_t                    maxClaimNumber = 60000;     // 2019_02_05

    Int32u_t                    minClaimNumber = 60000;     // 2020_09_20
    Int32u_t                    maxClaimNumber = 70000;     // 2020_09_20

    claimCheckStruct_p          claim_p;

    MbString                    buf;
    MbString                    buf2;
    MbSQL                       sql;
    MbSQL                       sql2;
    Int32u_t                    newClaimNumber;

    claim_q = qInitialize( &claimQueue );

    mbMalloc( buf_p, 2048 );

    // Read claims from the database
    mbSprintf( &buf, "select %s,%s,%s,%s,%s,%s,%s,%s from %s where %s>=%d and %s<%d",
        PMC_SQL_FIELD_ID,
        PMC_SQL_FIELD_PROVIDER_ID,
        PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER,
        PMC_SQL_CLAIMS_FIELD_SERVICE_DATE,
        PMC_SQL_CLAIMS_FIELD_FEE_CODE,
        PMC_SQL_CLAIMS_FIELD_CLAIM_HEADER_ID,
        PMC_SQL_FIELD_NOT_DELETED,
        PMC_SQL_CLAIMS_FIELD_STATUS,

        PMC_SQL_TABLE_CLAIMS,
        PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER, minClaimNumber,
        PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER, maxClaimNumber
    );

    mbLog( "This is the command: %s", &buf );

    // mbDlgInfo( "Claim numbers from 0 to 20000 were incremented on Dec 27 2013");
    // goto exit;

    //mbDlgInfo( "Claim numbers from 40000 to 50000 were incremented on Mar. 26 2016");
    //mbDlgInfo( "Claim numbers from 50000 to 60000 were incremented on Feb. 5 2019");

    mbDlgInfo( "Claim numbers from 60000 to 70000 were incremented on Sept. 20, 2020");
    goto exit
    // The "if" statement prevents a compile WARNING
    // if( maxClaimNumber == 60000 ) goto exit;

    if( mbDlgOkCancel( "Increment claim numbers?" ) == MB_BUTTON_CANCEL ) goto exit;

    // Execute the command
    sql.Query( buf.get( ) );

    sprintf( buf_p, "Processing %ld claim records...", sql.RowCount() );
    thermometer_p = new TThermometer( buf_p, 0, sql.RowCount(), TRUE );

    // Loop through the results; store only the IDs
    while( sql.RowGet() )
    {
        if( thermometer_p->Increment( ) == FALSE )
        {
            if( mbDlgYesNo( "Cancel claim number increment?" ) == MB_BUTTON_YES ) goto exit;
        }

        mbCalloc( claim_p, sizeof( claimCheckStruct_t ) );

        claim_p->id = sql.Int32u( 0 );
        claim_p->providerId = sql.Int32u( 1 );
        claim_p->claimNumber = sql.Int32u( 2 );

        mbLog("ID: %d provider ID: %d claim number %d",
            claim_p->id, claim_p->providerId, claim_p->claimNumber );

        qInsertLast( claim_q, claim_p );

        newClaimNumber = claim_p->claimNumber + 1000000;

        mbSprintf( &buf2, "update claims set claim_number = %d where id=%d",
            newClaimNumber, claim_p->id );

        mbLog("This is the command: %s", buf2.get());
        sql2.Update( buf2.get() );
        mbLog("back from sql update");
    }
exit:

    mbLog("We are done");

    if( thermometer_p ) delete thermometer_p;

    while( !qEmpty( claim_q ) )
    {
        claim_p = (claimCheckStruct_p)qRemoveFirst( claim_q );
     //   mbFree( document_p->name_p );
     //   mbFree( document_p->origName_p );
     //   mbFree( document_p->desc_p );
        mbFree( claim_p );
    }
    mbFree( buf_p);
}

void pmcDatabaseCheckClaimRecords( void )
{
    Char_p                  buf_p = NIL;
    Char_p                  buf2_p = NIL;
    Char_p                  cmd_p = NIL;
    qHead_t                 claimQueueHead;
    qHead_t                 claimQueueHead2;
    qHead_t                 claimHeaderQueueHead;
    qHead_p                 claim_q;
    qHead_p                 claim2_q;
    qHead_p                 header_q;
    claimCheckStruct_p      claim1_p, claim2_p;
    claimHeaderStruct_p     header_p;
    TThermometer           *thermometer_p = NIL;
    AnsiString              tempStr = "";
    Int32u_t                size, i, j;
    Boolean_t               found;
    Boolean_t               cancelled = TRUE;
    Boolean_t               ran = FALSE;

    mbMalloc( cmd_p, 2048 );
    mbMalloc( buf2_p, 256 );
    mbMalloc( buf_p, 256 );

    claim_q = qInitialize( &claimQueueHead );
    claim2_q = qInitialize( &claimQueueHead2 );
    header_q = qInitialize( &claimHeaderQueueHead );

    if( mbDlgOkCancel( "Check all claim records?" ) == MB_BUTTON_CANCEL ) goto exit;

    ran = TRUE;

    mbLog( "Starting check of claims database\n" );

    // Read claims from the database
    sprintf( buf_p, "select %s,%s,%s,%s,%s,%s,%s,%s from %s where %s>0",
        PMC_SQL_FIELD_ID,
        PMC_SQL_FIELD_PROVIDER_ID,
        PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER,
        PMC_SQL_CLAIMS_FIELD_SERVICE_DATE,
        PMC_SQL_CLAIMS_FIELD_FEE_CODE,
        PMC_SQL_CLAIMS_FIELD_CLAIM_HEADER_ID,
        PMC_SQL_FIELD_NOT_DELETED,
        PMC_SQL_CLAIMS_FIELD_STATUS,

        PMC_SQL_TABLE_CLAIMS,
        PMC_SQL_FIELD_ID );

    mbDlgDebug(( "obsolete section" ));
#if 0
    PMC_GENERAL_QUERY_SETUP( buf_p );
    while( !pmcDataSet_p->Eof )
    {
        mbCalloc( claim1_p, sizeof( claimCheckStruct_t ) );

        qInsertLast( claim_q, claim1_p );

        claim1_p->id          = pmcDataSet_p->FieldValues[ PMC_SQL_FIELD_ID ];
        claim1_p->providerId  = pmcDataSet_p->FieldValues[ PMC_SQL_FIELD_PROVIDER_ID ];
        claim1_p->claimNumber = pmcDataSet_p->FieldValues[ PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER ];
        claim1_p->serviceDate = pmcDataSet_p->FieldValues[ PMC_SQL_CLAIMS_FIELD_SERVICE_DATE ];
        claim1_p->headerId    = pmcDataSet_p->FieldValues[ PMC_SQL_CLAIMS_FIELD_CLAIM_HEADER_ID ];
        claim1_p->notDeleted  = pmcDataSet_p->FieldValues[ PMC_SQL_FIELD_NOT_DELETED ];
        claim1_p->status      = pmcDataSet_p->FieldValues[ PMC_SQL_CLAIMS_FIELD_STATUS ];

        claim1_p->statusChecked = FALSE;

        tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_CLAIMS_FIELD_FEE_CODE ] );
        strncpy( claim1_p->feeCode, tempStr.c_str( ), 7 );

        pmcDataSet_p->Next( );
    }
    PMC_GENERAL_QUERY_CLEANUP( );
#endif

    // Move "deleted" claims to backup queue
    size = claim_q->size;
    for( i = 0 ; i < size ; i++ )
    {
        claim1_p = (claimCheckStruct_p)qRemoveFirst( claim_q );
        if( claim1_p->notDeleted )
        {
            qInsertLast( claim_q, claim1_p );
        }
        else
        {
            qInsertLast( claim2_q, claim1_p );
        }
    }

    // Read claim headers from the database
    //                      id  #  0  1  2  3  4  5  6  7  8
    sprintf( buf_p, "select %s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s from %s where %s>0 and %s=%ld",
        PMC_SQL_FIELD_ID,
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_NUMBER,
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_0,
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_1,
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_2,
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_3,
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_4,
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_5,
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_6,
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_7,
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_8,
        PMC_SQL_TABLE_CLAIM_HEADERS,
        PMC_SQL_FIELD_ID,
        PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

        mbDlgDebug(( "obsolete section" ));
#if 0
    PMC_GENERAL_QUERY_SETUP( buf_p );
    while( !pmcDataSet_p->Eof )
    {
        mbCalloc( header_p, sizeof( claimHeaderStruct_t ) );

        qInsertLast( header_q, header_p );

        header_p->id          = pmcDataSet_p->FieldValues[ PMC_SQL_FIELD_ID ];
        header_p->claimId[0]  = pmcDataSet_p->FieldValues[ PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_0 ];
        header_p->claimId[1]  = pmcDataSet_p->FieldValues[ PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_1 ];
        header_p->claimId[2]  = pmcDataSet_p->FieldValues[ PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_2 ];
        header_p->claimId[3]  = pmcDataSet_p->FieldValues[ PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_3 ];
        header_p->claimId[4]  = pmcDataSet_p->FieldValues[ PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_4 ];
        header_p->claimId[5]  = pmcDataSet_p->FieldValues[ PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_5 ];
        header_p->claimId[6]  = pmcDataSet_p->FieldValues[ PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_6 ];
        header_p->claimId[7]  = pmcDataSet_p->FieldValues[ PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_7 ];
        header_p->claimId[8]  = pmcDataSet_p->FieldValues[ PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_8 ];
        pmcDataSet_p->Next( );
    }
    PMC_GENERAL_QUERY_CLEANUP( );
#endif

    sprintf( buf_p, "Processing %ld claim records...", claim_q->size );
    thermometer_p = new TThermometer( buf_p, 0, ( claim_q->size * 6 ) + header_q->size, TRUE );

    // Loop to update all the fee codes to the official MSP version
    size = claim_q->size;
    for( i = 0 ; i < size ; i++ )
    {
        if( thermometer_p->Increment( ) == FALSE )
        {
            if( mbDlgYesNo( "Cancel check of claim records?" ) == MB_BUTTON_YES ) goto exit;
        }

        claim1_p = (claimCheckStruct_p)qRemoveFirst( claim_q );
        qInsertLast( claim_q, claim1_p );

        pmcFeeCodeFormatDatabase( claim1_p->feeCode, buf_p );

        if( strcmp( claim1_p->feeCode, buf_p ) != 0 )
        {
            // Must update the fee code in the database
            sprintf( cmd_p, "update %s set %s=\"%s\" where %s=%ld\n",
                PMC_SQL_TABLE_CLAIMS,
                PMC_SQL_CLAIMS_FIELD_FEE_CODE, buf_p,
                PMC_SQL_FIELD_ID, claim1_p->id );

            pmcSqlExec( cmd_p );
            mbLog( "%s -> %s (id: %ld)\n", claim1_p->feeCode, buf_p, claim1_p->id );
            strcpy( claim1_p->feeCode, buf_p );
        }
    }

    // Loop to check that all fee codes are valid
    size = claim_q->size;
    for( i = 0 ; i < size ; i++ )
    {
        if( thermometer_p->Increment( ) == FALSE )
        {
            if( mbDlgYesNo( "Cancel check of claim records?" ) == MB_BUTTON_YES ) goto exit;
        }

        claim1_p = (claimCheckStruct_p)qRemoveFirst( claim_q );
        qInsertLast( claim_q, claim1_p );

        if( pmcFeeCodeVerify( claim1_p->feeCode, 0, NIL, NIL, NIL, NIL, NIL, PMC_CLAIM_TYPE_ANY ) == FALSE )
        {
            mbDlgExclaim( "Fee code %s not verified.", claim1_p->feeCode );
        }
    }

    // Loop to check for duplicate fee codes.
    size = claim_q->size;
    for( i = 0 ; i < size ; i++ )
    {
        if( thermometer_p->Increment( ) == FALSE )
        {
            if( mbDlgYesNo( "Cancel check of claim records?" ) == MB_BUTTON_YES ) goto exit;
        }

        claim1_p = (claimCheckStruct_p)qRemoveFirst( claim_q );

        qWalk( claim2_p, claim_q, claimCheckStruct_p )
        {
            // Sanity check
            if( claim1_p->id == claim2_p->id )
            {
                mbDlgDebug(( "Should not be here\n" ));
            }
            if( claim1_p->headerId    == claim2_p->headerId    &&
                claim1_p->serviceDate == claim2_p->serviceDate )
            {
                if( strcmp( claim1_p->feeCode, claim2_p->feeCode ) == 0 )
                {
                    mbDlgExclaim( "Cannot have save fee code on same date of service on same claim.\n"
                                  "(claim id: %ld claim number: %ld)", claim1_p->headerId, claim1_p->claimNumber );
                }
            }
        }
        qInsertLast( claim_q, claim1_p );
    }

    // Ensure claim numbers match
    size = claim_q->size;
    for( i = 0 ; i < size ; i++ )
    {
        if( thermometer_p->Increment( ) == FALSE )
        {
            if( mbDlgYesNo( "Cancel check of claim records?" ) == MB_BUTTON_YES ) goto exit;
        }
        claim1_p = (claimCheckStruct_p)qRemoveFirst( claim_q );

        qWalk( claim2_p, claim_q, claimCheckStruct_p )
        {
            // Sanity check
            if( claim1_p->id == claim2_p->id )
            {
                mbDlgDebug(( "Should not be here\n" ));
            }

            if( claim1_p->headerId  == claim2_p->headerId )
            {
                if( claim1_p->claimNumber != claim2_p->claimNumber )
                {
                    mbDlgExclaim( "Claim number mis-match (claim id: %ld)", claim1_p->headerId );
                }
            }
        }
        qInsertLast( claim_q, claim1_p );
    }

    // Ensure no duplicate claim numbers
    size = claim_q->size;
    for( i = 0 ; i < size ; i++ )
    {
        if( thermometer_p->Increment( ) == FALSE )
        {
            if( mbDlgYesNo( "Cancel check of claim records?" ) == MB_BUTTON_YES ) goto exit;
        }
        claim1_p = (claimCheckStruct_p)qRemoveFirst( claim_q );

        qWalk( claim2_p, claim_q, claimCheckStruct_p )
        {
            // Sanity check
            if( claim1_p->id == claim2_p->id )
            {
                mbDlgDebug(( "Should not be here\n" ));
            }
            if( claim1_p->claimNumber != PMC_CLAIM_NUMBER_NOT_ASSIGNED )
            {
                if( claim1_p->claimNumber == claim2_p->claimNumber &&
                    claim1_p->providerId == claim2_p->providerId )
                {
                    if( claim1_p->headerId != claim2_p->headerId )
                    {
                        mbDlgExclaim( "Claim id mis-match (claim1 id: %ld claim2 id: %ld)",
                        claim1_p->id, claim2_p->id );
                    }
                }
            }
        }
        qInsertLast( claim_q, claim1_p );
    }


    // Look for submitted and non submitted claims
    size = claim_q->size;
    for( i = 0 ; i < size ; i++ )
    {
        if( thermometer_p->Increment( ) == FALSE )
        {
            if( mbDlgYesNo( "Cancel check of claim records?" ) == MB_BUTTON_YES ) goto exit;
        }
        claim1_p = (claimCheckStruct_p)qRemoveFirst( claim_q );

        if( claim1_p->status == PMC_CLAIM_STATUS_SUBMITTED && claim1_p->statusChecked != TRUE )
        {
            claim1_p->statusChecked == TRUE;

            qWalk( claim2_p, claim_q, claimCheckStruct_p )
            {
                // Sanity check
                if( claim1_p->claimNumber == claim2_p->claimNumber &&
                    claim1_p->providerId  == claim2_p->providerId  )
                {
                    claim2_p->statusChecked = TRUE;
                    if( claim2_p->status !=  PMC_CLAIM_STATUS_SUBMITTED )
                    {
                        mbDlgExclaim( "Claim %d (provider %d) has both SUBMITTED and NON-SUBMITTED statuses.",
                            claim1_p->claimNumber, claim1_p->providerId );
                        break;
                    }
                }
            }
        }
        qInsertLast( claim_q, claim1_p );
    }


    // Check all header - claim cross references.  To do this we need to get the
    // deleted claims from the backup queue

    size = claim2_q->size;
    for( i = 0 ; i < size ; i++ )
    {
        claim1_p = (claimCheckStruct_p)qRemoveFirst( claim2_q );
        qInsertLast( claim_q, claim1_p );
    }

    size = header_q->size;
    for( i = 0 ; i < size ; i++ )
    {
        if( thermometer_p->Increment( ) == FALSE )
        {
            if( mbDlgYesNo( "Cancel check of claim records?" ) == MB_BUTTON_YES ) goto exit;
        }

        header_p = (claimHeaderStruct_p)qRemoveFirst( header_q );
        qInsertLast( header_q, header_p );

        for( j = 0 ; j < PMC_CLAIM_COUNT ; j++ )
        {
            if( header_p->claimId[j] != 0 )
            {
                // Look for specified claim record
                found = FALSE;

                qWalk( claim1_p, claim_q, claimCheckStruct_p )
                {
                    if( claim1_p->id == header_p->claimId[j] )
                    {
                        found = TRUE;
                        break;
                    }
                }

                if( found )
                {
                    qRemoveEntry( claim_q, claim1_p );
                    qInsertLast( claim2_q, claim1_p );

                    if( claim1_p->headerId != header_p->id )
                    {
                        mbDlgExclaim( "Claim id: %ld referenced by header id: %ld has wrong header id: %ld",
                            header_p->claimId[j], header_p->id, claim1_p->headerId );
                    }
                }
                else
                {
                    mbDlgExclaim( "Claim id: %ld referenced by header id: %ld not found",
                        header_p->claimId[j], header_p->id );
                }
            }
        }
    }

    // Check for unreferenced claims
    if( claim_q->size )
    {
        i = 0;

        qWalk( claim1_p, claim_q, claimCheckStruct_p )
        {
            if( claim1_p->notDeleted != FALSE ) i++;
        }

        if( i > 0 )
        {
            mbDlgExclaim( "%ld unreferenced claims; %ld not deleted\n", claim_q->size, i );
        }
    }

    cancelled = FALSE;

exit:

    if( thermometer_p ) delete thermometer_p;

    if( ran )
    {
        mbDlgExclaim( "Claim records check %s.", cancelled ? "cancelled" : "completed" );
    }

    for( ; ; )
    {
        if( qEmpty( claim_q ) ) break;
        claim1_p = (claimCheckStruct_p)qRemoveFirst( claim_q );
        mbFree( claim1_p );
    }

    for( ; ; )
    {
        if( qEmpty( claim2_q ) ) break;
        claim1_p = (claimCheckStruct_p)qRemoveFirst( claim2_q );
        mbFree( claim1_p );
    }

    for( ; ; )
    {
        if( qEmpty( header_q ) ) break;
        header_p = (claimHeaderStruct_p)qRemoveFirst( header_q );
        mbFree( header_p );
    }

    if( buf_p )         mbFree( buf_p );
    if( buf2_p )        mbFree( buf2_p );
    if( cmd_p )         mbFree( cmd_p );

    return;
}

//---------------------------------------------------------------------------
// Function: pmcImportMspDrListExcel()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void pmcImportMspDrListExcel( void )
{
    Char_p      currentDir_p;
    Char_p      buf_p;
    Char_p      reportFileName_p;
    Char_p      fileName_p;
    Char_p      firstName_p;
    Char_p      lastName_p;
    Char_p      city_p;
    Char_p      specialty_p;
    Char_t      prov[4];
    FILE       *fp = NIL;
    FILE       *rp = NIL;
    qHead_t     docQueueHead;
    qHead_p             doc_q;
    docMaintStruct_p    doc_p;
    Int32u_t            lineCount = 0;
    Int32u_t    tabCount = 0;
    Int32u_t    doctorNumber;
    Int32u_t    doctorId;
    Char_p      temp_p;
    Ints_t      i, len;
    bool        found;
    bool        tablesLocked = FALSE;
    AnsiString  tempStr = "";
    Char_p      field_p[PMC_MSP_EXCEL_DR_LIST_FIELDS];
    TThermometer *thermometer_p = NIL;

    mbMalloc( currentDir_p, 256 );
    mbMalloc( buf_p, 512 );
    mbMalloc( reportFileName_p, 256 );
    mbMalloc( fileName_p, 256 );

    mbMalloc( firstName_p, 64 );
    mbMalloc( lastName_p, 64 );
    mbMalloc( specialty_p, 128 );
    mbMalloc( city_p, 64 );

    doc_q = qInitialize( &docQueueHead );

    getcwd( currentDir_p, 256 );

    MainForm->OpenDialog->Title = "Select Excel MSP doctor list (in TAB delimited format)...";
    MainForm->OpenDialog->InitialDir = currentDir_p;
    MainForm->OpenDialog->Filter = "Text files (*.txt)|*.TXT";
    MainForm->OpenDialog->FilterIndex = 1;
    MainForm->OpenDialog->FileName = "";

    // Check for cancel button
    if( MainForm->OpenDialog->Execute() != TRUE ) goto exit;

    strcpy( fileName_p, MainForm->OpenDialog->FileName.c_str() );
    fp = fopen( fileName_p, "r" );
    if( fp == NIL )
    {
        mbDlgExclaim( "Could not open file '%s'.", fileName_p );
        goto exit;
    }

    // Open report file
    pmcMakeFileName( pmcCfg[CFG_REPORT_DIR].str_p, reportFileName_p );
    strcat( reportFileName_p, "_msp_import.txt" );
    if( ( rp = fopen( reportFileName_p, "w" ) ) == NIL )
    {
        mbDlgExclaim( "Could not open report file '%s'.", reportFileName_p );
        goto exit;
    }

    MainForm->RefreshTable( PMC_TABLE_BIT_DOCTORS );

    // Now read the lines from the file
    lineCount = 0;
    while( fgets( buf_p, 512, fp ) != 0 )
    {
        // Null terminate buffer (in case some kind of binary data was read)
        *(buf_p + 511) = 0;
        len = strlen( buf_p );

        // Check that this line looks valid
        for( temp_p = buf_p, tabCount = 0, i = 0 ; i < len ; i++, temp_p++ )
        {
            if( *temp_p == '\t' ) tabCount++;
        }

        if( tabCount != ( PMC_MSP_EXCEL_DR_LIST_FIELDS - 1 ) )
        {
            if( mbDlgOkCancel( "Read invalid line (tabCount: %ld). Continue reading file?", tabCount ) == MB_BUTTON_CANCEL )
            {
                mbDlgExclaim( "Terminating read of invalid file." );
                goto exit;
            }
        }
        lineCount++;
    }

    rewind( fp );

    sprintf( buf_p, "Processing %ld records...", lineCount );
    thermometer_p = new TThermometer( buf_p, 0, lineCount, FALSE );

    // Lock all of the tables we are going to use
    // Suspend polling before lock.  Must ensure polling is stopped or deadlock can ensue

    pmcSuspendPollInc( );

    sprintf( buf_p, "lock tables %s write",
        PMC_SQL_TABLE_DOCTORS );
    pmcSqlExec( buf_p );
    tablesLocked = TRUE;

    // Want to mark all records as on msp list, then individually remove
    // those records not on the list.  Want to do this because it is
    // possible that a doctor with a number moved away (off the list)
    // then back (on the list)
    sprintf( buf_p, "update %s set %s=%d where %s>0",
        PMC_SQL_TABLE_DOCTORS,
        PMC_SQL_DOCTORS_FIELD_ON_MSP_LIST, PMC_SQL_TRUE_VALUE,
        PMC_SQL_FIELD_ID );

    pmcSqlExec( buf_p );

    // Read all the doctor records
    sprintf( buf_p, "select %s,%s,%s,%s,%s from %s where %s>0 and %s=%ld",
        PMC_SQL_FIELD_ID,
        PMC_SQL_FIELD_LAST_NAME,
        PMC_SQL_DOCTORS_FIELD_DOCTOR_NUMBER,
        PMC_SQL_DOCTORS_FIELD_SPECIALTY,
        PMC_SQL_FIELD_CITY,
        PMC_SQL_TABLE_DOCTORS,
        PMC_SQL_FIELD_ID,
        PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

        mbDlgDebug(( "obsolete section" ));
#if 0
    PMC_GENERAL_QUERY_SETUP( buf_p );

    while( !pmcDataSet_p->Eof )
    {
        mbCalloc( doc_p, sizeof( docMaintStruct_t ) );

        // Get the appointment Id
        doc_p->id = pmcDataSet_p->FieldValues[ PMC_SQL_FIELD_ID ];

        // Referring Dr id
        doc_p->doctorNumber = pmcDataSet_p->FieldValues[ PMC_SQL_DOCTORS_FIELD_DOCTOR_NUMBER ];

        // Get the last name
        tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_FIELD_LAST_NAME ] );
        mbMalloc( doc_p->lastName_p, strlen( tempStr.c_str() ) + 1 );
        strcpy( doc_p->lastName_p, tempStr.c_str( ) );

        // Get the city
        tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_FIELD_CITY ] );
        mbMalloc( doc_p->city_p, strlen( tempStr.c_str() ) + 1 );
        strcpy( doc_p->city_p, tempStr.c_str( ) );

        // Get the specialty
        tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_DOCTORS_FIELD_SPECIALTY ] );
        mbMalloc( doc_p->specialty_p, strlen( tempStr.c_str() ) + 1 );
        strcpy( doc_p->specialty_p, tempStr.c_str( ) );

        if( doc_p->doctorNumber )
        {
            // Look for duplicate numbers
            for( doc2_p = (docMaintStruct_p)qInit( doc_q ) ;
                                            qDone( doc_q ) ;
                 doc2_p = (docMaintStruct_p)qNext( doc_q ) )
            {
                if( doc_p->doctorNumber == doc2_p->doctorNumber )
                {
                    mbDlgExclaim( "Duplicate doctor number %ld detected.", doc_p->doctorNumber );
                }
            }
        }
        else
        {
            fprintf( rp, "Read doctor number of 0 (doctor id: %ld)\n", doc_p->id );
        }
        qInsertFirst( doc_q, doc_p );

        pmcDataSet_p->Next( );
    }
    PMC_GENERAL_QUERY_CLEANUP( );
#endif

    fprintf( rp, "Read %ld doctor records from database\n\n", doc_q->size );

    lineCount = 0;

    // Now read the lines from the file
    while( fgets( buf_p, 512, fp ) != 0 )
    {
        thermometer_p->Increment( );

        // Null terminate buffer (in case some kind of binary data was read)
        *(buf_p + 511) = 0;
        len = strlen( buf_p );

        // Check that this line looks valid
        for( temp_p = buf_p, tabCount = 0, i = 0 ; i < len ; i++, temp_p++ )
        {
            if( *temp_p == '\t' ) tabCount++;
        }

        if( tabCount != ( PMC_MSP_EXCEL_DR_LIST_FIELDS - 1 ) )
        {
            if( mbDlgOkCancel( "Read invalid line (tabCount: %ld). Continue reading file?", tabCount ) == MB_BUTTON_CANCEL )
            {
                mbDlgExclaim( "Terminating read of invalid file." );
                goto exit;
            }
        }

        // Get individual fields
        temp_p = buf_p;
        for( i = 0 ; i < PMC_MSP_EXCEL_DR_LIST_FIELDS ; i++ )
        {
            field_p[i] = temp_p;
            for( ; ; temp_p++ )
            {
                if( *temp_p == '\t' )
                {
                    *temp_p++ = 0;
                    break;
                }
                if( *temp_p == 0 ) break;
            }
        }

        doctorNumber = atol( field_p[0] );
        pmcMspCleanNames( field_p[1], firstName_p, lastName_p );
        mbStrClean( pmcStringStripDoubleQuotes( field_p[2] ), city_p, TRUE );
        sprintf( prov, "SK" );
        pmcMspFixCity( city_p, &prov[0] );
        mbStrClean( pmcStringStripDoubleQuotes( field_p[3] ), specialty_p, TRUE );

        // Look for this doctor number in the database
        found = FALSE;

        qWalk( doc_p, doc_q, docMaintStruct_p )
        {
            if( doc_p->doctorNumber == doctorNumber )
            {
                found = TRUE;
                break;
            }
        }

        if( found )
        {
            // Update the specialty
            if( strlen( specialty_p ) )
            {
                if( strcmp( specialty_p, doc_p->specialty_p ) != 0 )
                {
                    sprintf( buf_p, "update %s set %s=\"%s\" where %s=%ld",
                        PMC_SQL_TABLE_DOCTORS,
                        PMC_SQL_DOCTORS_FIELD_SPECIALTY, specialty_p,
                        PMC_SQL_FIELD_ID, doc_p->id );

                    pmcSqlExec( buf_p );
                }
            }

            // Check the last name - just print to report if they are different.
            // I think that human intervention in the database will be
            // required if the names differ.
            if( mbStrPosIgnoreCase( lastName_p, doc_p->lastName_p ) < 0 )
            {
                fprintf( rp, "Dr #%04ld: Last name (%s) does not match database (%s, id: %ld)\n",
                    doc_p->doctorNumber, lastName_p, doc_p->lastName_p, doc_p->id );
            }

            if( strcmp( city_p, doc_p->city_p ) != 0 )
            {
                fprintf( rp, "Dr #%04ld: City (%s) does not match database (%s, id: %ld)\n",
                    doc_p->doctorNumber, city_p, doc_p->city_p, doc_p->id );

                // This doctor must have moved.  Invalidate any address and
                // phone mumber fields (it is too bad they are not in the
                // list that we get from MSP).
                //                             city     add1     add2    post    prov     phone   fax
                sprintf( buf_p, "update %s set %s=\"%s\",%s=\"\",%s=\"\",%s=\"\",%s=\"\",%s=\"\",%s=\"\" where %s=%d",
                    PMC_SQL_TABLE_DOCTORS,
                    PMC_SQL_FIELD_CITY, city_p,
                    PMC_SQL_FIELD_ADDRESS1,
                    PMC_SQL_FIELD_ADDRESS2,
                    PMC_SQL_FIELD_PROVINCE,
                    PMC_SQL_FIELD_POSTAL_CODE,
                    PMC_SQL_DOCTORS_FIELD_WORK_PHONE,
                    PMC_SQL_DOCTORS_FIELD_WORK_FAX,
                    PMC_SQL_FIELD_ID, doc_p->id );

                pmcSqlExec( buf_p );
            }

            qRemoveEntry( doc_q, doc_p );
            if( doc_p->lastName_p ) mbFree( doc_p->lastName_p );
            if( doc_p->specialty_p ) mbFree( doc_p->specialty_p );
            if( doc_p->city_p ) mbFree( doc_p->city_p );
            mbFree( doc_p );
        }
        else
        {
            doctorId = pmcSqlRecordCreate( PMC_SQL_TABLE_DOCTORS, NIL );
            if( doctorId == 0 )
            {
                fprintf( rp, "Error creating doctor record" );
            }
            else
            {   //                              Dr #   last      first    city      prov     specialty
                sprintf( buf_p, "update %s set %s=%ld,%s=\"%s\",%s=\"%s\",%s=\"%s\",%s=\"%s\",%s=\"%s\",%s=%d where %s=%ld",
                    PMC_SQL_TABLE_DOCTORS,
                    PMC_SQL_DOCTORS_FIELD_DOCTOR_NUMBER,    doctorNumber,
                    PMC_SQL_FIELD_LAST_NAME,                lastName_p,
                    PMC_SQL_FIELD_FIRST_NAME,               firstName_p,
                    PMC_SQL_FIELD_CITY,                     city_p,
                    PMC_SQL_FIELD_PROVINCE,                 prov,
                    PMC_SQL_DOCTORS_FIELD_SPECIALTY,        specialty_p,
                    PMC_SQL_DOCTORS_FIELD_ON_MSP_LIST,      PMC_SQL_TRUE_VALUE,
                    PMC_SQL_FIELD_ID,                       doctorId );

                pmcSqlExec( buf_p );

                fprintf( rp, "Dr #%04ld: Created new entry - %s %s %s (id: %ld)\n",
                    doctorNumber, firstName_p, lastName_p, city_p, doctorId );

                pmcSqlRecordUndelete( PMC_SQL_TABLE_DOCTORS, doctorId );
            }
        }
        lineCount++;
    }

    fprintf( rp, "Read %ld lines from file '%s'\n", lineCount, fileName_p );

    fprintf( rp, "\n\nThe following doctors do not appear in the MSP file:\n\n" );
    for( ; ; )
    {
        if( qEmpty( doc_q ) ) break;
        doc_p = (docMaintStruct_p)qRemoveFirst( doc_q );
        fprintf( rp, "Dr #%04ld: (%s, id: %ld)\n", doc_p->doctorNumber, doc_p->lastName_p, doc_p->id );

        sprintf( buf_p, "update %s set %s=%d where %s=%ld",
            PMC_SQL_TABLE_DOCTORS,
            PMC_SQL_DOCTORS_FIELD_ON_MSP_LIST, PMC_SQL_FALSE_VALUE,
            PMC_SQL_FIELD_ID, doc_p->id );

        pmcSqlExec( buf_p );

        if( doc_p->lastName_p ) mbFree( doc_p->lastName_p );
        if( doc_p->specialty_p ) mbFree( doc_p->specialty_p );
        if( doc_p->city_p ) mbFree( doc_p->city_p );
        mbFree( doc_p );
    }

    mbDlgInfo( "Doctor list has successfully been imported into the database." );

exit:

    if( thermometer_p ) delete thermometer_p;

    if( tablesLocked )
    {
        sprintf( buf_p, "unlock tables" );
        pmcSqlExec( buf_p );
        pmcSuspendPollDec( );
    }

    // Clean out the queue
    for( ; ; )
    {
        if( qEmpty( doc_q ) ) break;
        doc_p = (docMaintStruct_p) qRemoveFirst( doc_q );
        if( doc_p->lastName_p ) mbFree( doc_p->lastName_p );
        if( doc_p->specialty_p ) mbFree( doc_p->specialty_p );
        if( doc_p->city_p ) mbFree( doc_p->city_p );
        mbFree( doc_p );
    }

    if( fp ) fclose( fp );

    if( rp )
    {
        pmcReportFormInfo_t     reportFormInfo;
        TReportForm            *reportForm_p;

        fclose( rp );

        sprintf( buf_p, "Result of MSP doctor file import" );
        reportFormInfo.fileName_p = reportFileName_p;
        reportFormInfo.caption_p = buf_p;

        reportForm_p = new TReportForm( NULL, &reportFormInfo );
        reportForm_p->ShowModal();
        delete reportForm_p;
    }

    mbFree( currentDir_p );
    mbFree( buf_p );
    mbFree( reportFileName_p );
    mbFree( fileName_p );

    mbFree( firstName_p );
    mbFree( lastName_p );
    mbFree( specialty_p );
    mbFree( city_p );

    // Update the doctor list
    MainForm->RefreshTable( PMC_TABLE_BIT_DOCTORS );

    return;
}

void pmcMspFixCity( Char_p in_p, Char_p prov_p )
{
    Char_p  buf_p;
    Ints_t  len;

    len = strlen( in_p );
    mbMalloc( buf_p, len + 1 );

    strcpy( buf_p, in_p );
    mbStrToUpper( buf_p );

    if( mbStrPos( buf_p, "CROSSE" ) >= 5 )         { sprintf( in_p, "Ile a la Crosse" ); goto done; }
    if( mbStrPos( buf_p, "FOND DU" ) == 0 )        { sprintf( in_p, "Fond du Lac" ); goto done; }
    if( mbStrPos( buf_p, "ILE A LA" ) == 0 )       { sprintf( in_p, "Ile a la Crosse" ); goto done; }
    if( mbStrPos( buf_p, "N BATTLE" ) >= 0 )       { sprintf( in_p, "North Battleford" ); goto done; }
    if( mbStrPos( buf_p, "PR ALBERT" ) >= 0 )      { sprintf( in_p, "Prince Albert" ); goto done; }
    if( mbStrPos( buf_p, "FORT QU" ) >= 0 )        { sprintf( in_p, "Fort Qu'Appelle" ); goto done; }
    if( mbStrPos( buf_p, "CENTRAL BUT" ) >= 0 )    { sprintf( in_p, "Central Butte" ); goto done; }
    if( mbStrPos( buf_p, "MACKLIN" ) >= 0 )        { sprintf( in_p, "Macklin" ); goto done; }
    if( mbStrPos( buf_p, "PORCUPINE P" ) >= 0 )    { sprintf( in_p, "Porcupine Plain" ); goto done; }
    if( mbStrPos( buf_p, "SHERWOOD P" ) >= 0 )     { sprintf( in_p, "Sherwood Park" ); goto done; }
    if( mbStrPos( buf_p, "SW CURR" ) >= 0 )        { sprintf( in_p, "Swift Current" ); goto done; }
    if( mbStrPos( buf_p, "FLIN FLON" ) >= 0 )      { sprintf( in_p, "Flin Flon" ); sprintf( prov_p, "MB" ); goto done; }
    if( mbStrPos( buf_p, "CALGARY" ) >= 0 )        { sprintf( in_p, "Calgary" ); sprintf( prov_p, "AB" ); goto done; }
    if( mbStrPos( buf_p, "EDMONTON" ) >= 0 )       { sprintf( in_p, "Edmonton" ); sprintf( prov_p, "AB" );goto done; }
    if( mbStrPos( buf_p, "VICTORIA" ) >= 0 )       { sprintf( in_p, "Victoria" ); sprintf( prov_p, "BC" );goto done; }
    if( mbStrPos( buf_p, "PARADISE HIL" ) >= 0 )   { sprintf( in_p, "Paradise Hill" ); goto done; }


done:
    mbFree( buf_p );
    return;
}

//---------------------------------------------------------------------------
// Function: pmcMspCleanNames()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void pmcMspCleanNames
(
    Char_p  in_p,
    Char_p  firstName_p,
    Char_p  lastName_p
)
{
    Char_p  buf_p;
    Char_p  temp1_p;
    Ints_t  i, len;
    bool    found;

    sprintf( firstName_p, "" );
    sprintf( lastName_p, "" );
    
    len = strlen( in_p );
    mbMalloc( buf_p, len + 1 );

    strcpy( buf_p, in_p );
    pmcStringStripDoubleQuotes( buf_p );

    len = strlen( buf_p );
    temp1_p = buf_p;
    for( i = 0, found = FALSE ; i < len ; i++ )
    {
        if( *temp1_p == ',' )
        {
            found = TRUE;
            break;
        }
        temp1_p++;
    }

    if( found )
    {
        *temp1_p = 0;
        mbStrClean( buf_p, lastName_p, TRUE );
        mbStrClean( temp1_p+1, firstName_p, TRUE );
    }

    mbFree( buf_p );
    return;
}

//---------------------------------------------------------------------------
// Function: pmcCheckAppointmentRecords()
//---------------------------------------------------------------------------
// Description:
//
// This function checks that all the patient and referring doctor IDs
// referenced by the appointment records actually exist.
//---------------------------------------------------------------------------

void pmcCheckAppointmentRecords( void )
{
    Char_p                      buf_p = NIL;
    Char_p                      buf2_p = NIL;
    MbDateTime                  dateTime;
    Int32u_t                    i;
    Int32u_t                    future;
    Int32u_t                    today;
    Boolean_t                   tableLocked = FALSE;
    TThermometer               *thermometer_p = NIL;
    PmcSqlPatient_p             pat_p = NIL;
    PmcSqlDoctor_p              dr_p = NIL;
    PmcSqlProvider_p            provider_p = NIL;
    Int32u_p                    appId_p = NIL;
    Int32u_p                    id_p;
    PmcSqlApp_p                 app_p = NIL;
    Int32u_t                    count;
    Boolean_t                   cancelled = FALSE;
    Boolean_t                   ran = FALSE;

    mbMalloc( buf_p, 2048 );
    mbMalloc( buf2_p, 256 );
    mbMalloc( pat_p, sizeof( PmcSqlPatient_t ) );
    mbMalloc( app_p, sizeof( PmcSqlApp_t) );
    mbMalloc( dr_p,  sizeof( PmcSqlDoctor_t) );
    mbMalloc( provider_p, sizeof( PmcSqlProvider_t) );

    // Flag appointment more than 5 years into the future
    today = mbToday( );
    future = today + 50000;

    dateTime.SetDate( today );

    if( mbDlgOkCancel( "Check all appointment records?" ) == MB_BUTTON_CANCEL ) goto exit;

    ran = TRUE;
    pmcSuspendPollInc( );

    // Lock the appointments table for the duration of this operation
    sprintf( buf_p, "lock tables %s write,%s write,%s write",
        PMC_SQL_TABLE_APPS, PMC_SQL_TABLE_PATIENTS, PMC_SQL_TABLE_DOCTORS );
    pmcSqlExec( buf_p );
    tableLocked = TRUE;

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

    // Sanity check
    if( i != count )
    {
        mbDlgDebug(( "Error reading appointment IDs" ));
        goto exit;
    }

    sprintf( buf_p, "Processing %s appointment records...", mbStrInt32u( count, buf2_p ) );
    thermometer_p = new TThermometer( buf_p, 0, count, TRUE );

    for( i = 0, id_p = appId_p ; i < count ; i++, id_p++ )
    {
        if( thermometer_p->Increment( ) == FALSE )
        {
            if( mbDlgYesNo( "Cancel check of appointment records?" ) == MB_BUTTON_YES )
            {
                cancelled = TRUE;
                break;
            }
        }

        if( pmcSqlAppDetailsGet( *id_p, app_p ) != TRUE )
        {
            mbDlgDebug(( "error reading app %d\n", id_p ));
            continue;
        }

        // Do not bother checking deleted appointments
        if( app_p->deleted == TRUE && app_p->completed != PMC_APP_COMPLETED_STATE_CANCELLED ) continue;

        if( app_p->referringDrId )
        {
            if( pmcSqlDoctorDetailsGet( app_p->referringDrId, dr_p ) == FALSE )
            {
                mbDlgInfo( "Could not find referring Dr (id %ld) referenced by appointment id %ld.\n",
                               app_p->referringDrId, *id_p );
            }
        }

        if( app_p->patientId )
        {
            if( pmcSqlPatientDetailsGet( app_p->patientId, pat_p ) == FALSE )
            {
                mbDlgInfo( "Could not find patient (id %ld) referenced by appointment id %ld.\n",
                               app_p->patientId, *id_p );
            }
            else
            {
                if(    pat_p->deceasedDate > 0
                    && pat_p->deceasedDate <= app_p->date
                    && app_p->date >= today )
                {
                    dateTime.SetDate( app_p->date );

                    if( pmcSqlProviderDetailsGet( app_p->providerId, provider_p ) != TRUE )
                    {
                        sprintf( provider_p->description, "UNKNOWN" );
                    }

                    if( mbDlgYesNo( "Appointment %ld on %s with %s\nrefers to a patient marked as deceased (%s %s).\n"
                                       "Delete appointment?",
                                        *id_p,
                                        dateTime.MDY_DateString( ),
                                        provider_p->description,
                                        pat_p->firstName, pat_p->lastName ) == MB_BUTTON_YES )
                    {
                        pmcSqlRecordDelete( PMC_SQL_TABLE_APPS, *id_p );
                        pmcAppHistory( *id_p, PMC_APP_ACTION_DELETE, 0 ,0 ,0 ,0, NIL );
                    }
                }
            }
        }

        if( app_p->date < 20000000 || app_p->date > future )
        {
            dateTime.SetDate( app_p->date );

            if( pmcSqlProviderDetailsGet( app_p->providerId, provider_p ) != TRUE )
            {
                sprintf( provider_p->description, "UNKNOWN" );
            }
            mbDlgInfo( "Appointment %ld with %s has suspicious date %s\n",
                *id_p, provider_p->description, dateTime.MDY_DateString( ) );
        }

        // Check the day of the week for the appointment
        if( app_p->date > today )
        {
            Int32u_t   dayOfWeek = mbDayOfWeek( app_p->date );
            Boolean_t  displayed = FALSE;

            if(    dayOfWeek == MB_DAY_SATURDAY
                || dayOfWeek == MB_DAY_SUNDAY )
            {
                dateTime.SetDate( app_p->date );
                if( app_p->patientId )
                {
                    if( pmcSqlPatientDetailsGet( app_p->patientId, pat_p ) )
                    {
                        mbDlgInfo( "Appointment for %s %s is on a %s (%s)\n",
                            pat_p->firstName, pat_p->lastName,
                            mbDayStringsArrayLong[dayOfWeek],
                            dateTime.MDY_DateString( ) );
                        displayed = TRUE;
                    }
                }
                if( !displayed )
                {
                    mbDlgInfo( "Appointment %ld is on a %s (%s)\n",
                        *id_p, mbDayStringsArrayLong[dayOfWeek],
                        dateTime.MDY_DateString( ) );
                }
            }
        }

        if( app_p->confLetterDate || app_p->confLetterTime || app_p->confLetterId )
        {
            if( app_p->confLetterDate == 0 || app_p->confLetterTime == 0 || app_p->confLetterId == 0 )
            {
                mbDlgInfo( "Appointment %ld letter confirmation error\n", *id_p );
            }
        }

        if( app_p->confPhoneDate || app_p->confPhoneTime || app_p->confPhoneId )
        {
            if( app_p->confPhoneDate == 0 || app_p->confPhoneTime == 0 || app_p->confPhoneId == 0 )
            {
                mbDlgInfo( "Appointment %ld phone confirmation error\n", *id_p );
            }
        }
   }

exit:

    if( tableLocked )
    {
        sprintf( buf_p, "unlock tables" );
        pmcSqlExec( buf_p );
    }

    if( thermometer_p ) delete thermometer_p;

    if( ran )
    {
        pmcSuspendPollDec();
        mbDlgInfo( "Appointment records check %s.", cancelled ? "cancelled" : "completed" );
    }

    mbFree( appId_p );
    mbFree( buf_p );
    mbFree( buf2_p );
    mbFree( pat_p );
    mbFree( app_p );
    mbFree( dr_p );
    mbFree( provider_p );

    return;
}



#if PMC_SCRAMBLE_PATIENT_NAMES
//---------------------------------------------------------------------------
// Function: pmcScrambleNames()
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void pmcScrambleNames( void )
{
    Char_p          buf_p = NIL;
    Char_p          buf2_p = NIL;
    Char_p          cmd_p = NIL;
    qHead_t         pat_qhead;
    qHead_p         pat_q;
    AnsiString      tempStr = "";
    patCheck_p      pat_p;
    Int32u_t        i;
    FILE           *fp = NIL;
    Char_p          fileName_p = NIL;
 //   TDatabaseThermometer
 //                   *thermometer_p = NIL;

    MbSQL           sql;
    TThermometer   *thermometer_p = NIL;
    Int32u_t        maleNameCount = 0;
    Int32u_t        femaleNameCount = 0;
    Int32u_t        firstIndex, lastIndex;

    if( pmcCfg[CFG_SCRAMBLE_NAMES_FLAG].value == FALSE ) return;

    mbMalloc( buf_p, 2048 );
    mbMalloc( buf2_p, 256 );
    mbMalloc( cmd_p, 256 );
    mbMalloc( fileName_p, 256 );

    if( mbDlgOkCancel( "Scramble all patient names in database?" ) == MB_BUTTON_CANCEL ) goto exit;

    // Count male names in list
    for( i = 0 ; ; i++ )
    {
        if( mbStrPos( lgMaleNames[i], LG_END_OF_LIST ) >= 0 ) break;
        maleNameCount++;
    }

    // Count female names in list
    for( i = 0 ; ; i++ )
    {
        if( mbStrPos( lgFemaleNames[i], LG_END_OF_LIST ) >= 0 ) break;
        femaleNameCount++;
    }
    mbLog( "pmcScrambleNames() called; male names: %ld female names: %ld\n", maleNameCount, femaleNameCount );

    pat_q = qInitialize( &pat_qhead );

    sprintf( buf_p, "select %s,%s from %s where %s>0 and %s=%ld",
        PMC_SQL_FIELD_ID,
        PMC_SQL_PATIENTS_FIELD_GENDER,

        PMC_SQL_TABLE_PATIENTS,
        PMC_SQL_FIELD_ID,
        PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );


    sql.Query( buf_p );

    while( sql.RowGet( ) )
    {
        mbCalloc( pat_p, sizeof( patCheck_t) );

        qInsertFirst( pat_q, pat_p );

        // Get the patient ID
        pat_p->id       = sql.Int32u( 0 );
        pat_p->gender   = sql.Int32u( 1 );
    }

    sprintf( buf_p, "Processing %ld patient records...", pat_q->size );

    thermometer_p = new TThermometer( buf_p, 0, sql.RowCount(), TRUE );

    for( i = 0 ; ; i++ )
    {
        if( qEmpty( pat_q ) ) break;
        pat_p = (patCheck_p)qRemoveFirst( pat_q );

        thermometer_p->Increment();

        // Use male names as last names
        lastIndex = rand();
        lastIndex = (Int32u_t)( (float)lastIndex * float(maleNameCount) / (float)RAND_MAX );

        if( pat_p->gender == PMC_SQL_GENDER_MALE )
        {
            firstIndex = rand();
            firstIndex = (Int32u_t)( (float)firstIndex * float(maleNameCount) / (float)RAND_MAX );
            sprintf( buf2_p, "%s", lgMaleNames[firstIndex] );
        }
        else
        {
            firstIndex = rand();
            firstIndex = (Int32u_t)( (float)firstIndex * float(femaleNameCount) / (float)RAND_MAX );
            sprintf( buf2_p, "%s", lgFemaleNames[firstIndex] );
        }

        //mbLog( "%s %s -> %s %s\n", pat_p->firstName_p, pat_p->lastName_p, buf2_p, lgMaleNames[lastIndex] );

        sprintf( buf_p, "update %s set %s=\"%s\", %s=\"%s\" where %s=%ld",
            PMC_SQL_TABLE_PATIENTS,
            PMC_SQL_FIELD_FIRST_NAME, buf2_p,
            PMC_SQL_FIELD_LAST_NAME, lgMaleNames[lastIndex],
            PMC_SQL_FIELD_ID, pat_p->id );

//#if 0
        pmcSqlExec( buf_p );
//#endif

        if( pat_p->firstName_p )    mbFree( pat_p->firstName_p );
        if( pat_p->lastName_p )     mbFree( pat_p->lastName_p );
        if( pat_p->title_p )        mbFree( pat_p->title_p );

        mbFree( pat_p );
    }

    mbDlgInfo( "Patient name scramble complete." );
exit:

    if( fp ) fclose( fp );

    if( thermometer_p ) delete thermometer_p;

    if( buf_p )         mbFree( buf_p );
    if( buf2_p )        mbFree( buf2_p );
    if( cmd_p )         mbFree( cmd_p );
    if( fileName_p )    mbFree( fileName_p );

    mbLog( "pmcScrambleNames() complete \n" );

    return;
}
#endif // PMC_SCRAMBLE_PATIENT_NAMES

//---------------------------------------------------------------------------
// Function: pmcDatabaseCleanFields()
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void pmcDatabaseCleanFields( void )
{
    Char_p      buf_p = NIL;
    qHead_t     pat_qhead;
    qHead_p     pat_q;
    AnsiString  tempStr = "";
    patTemp_p   pat_p;
    bool        update = FALSE;
    bool        modified = FALSE;
    FILE       *fp = NIL;
    Char_p      fileName_p = NIL;
    Char_p      orig_p = NIL;
    TThermometer   *thermometer_p = NIL;

    mbMalloc( orig_p, 128 );
    mbMalloc( buf_p, 512 );
    mbMalloc( fileName_p, 256 );

    pat_q = qInitialize( &pat_qhead );

    if( mbDlgOkCancel( "Clean database fields? (postal codes, telephone numbers)" ) == MB_BUTTON_CANCEL ) goto exit;

    sprintf( buf_p, "select %s,%s,%s,%s from %s where %s>0",
        PMC_SQL_FIELD_ID,
        PMC_SQL_FIELD_POSTAL_CODE,
        PMC_SQL_PATIENTS_FIELD_HOME_PHONE,
        PMC_SQL_PATIENTS_FIELD_WORK_PHONE,
        PMC_SQL_TABLE_PATIENTS,
        PMC_SQL_FIELD_ID );

        mbDlgDebug(( "obsolete section" ));
#if 0
    PMC_GENERAL_QUERY_SETUP( buf_p );
    while( !pmcDataSet_p->Eof )
    {
            mbMalloc( pat_p, sizeof( patTemp_t) );
            memset( pat_p, 0, sizeof( patTemp_p ) );
            pat_p->code = 0;
            qInsertFirst( pat_q, pat_p );

            // Get the patient ID
            pat_p->id = pmcDataSet_p->FieldValues[ PMC_SQL_FIELD_ID ];

            // Get the patient postal code
            tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_FIELD_POSTAL_CODE ] );
            mbMalloc( pat_p->postalCode_p, strlen( tempStr.c_str() ) + 10 );
            strcpy( pat_p->postalCode_p, tempStr.c_str( ) );

            // Get home phone number
            tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_PATIENTS_FIELD_HOME_PHONE ] );
            mbMalloc( pat_p->homePhone_p, strlen( tempStr.c_str() ) + 10 );
            strcpy( pat_p->homePhone_p, tempStr.c_str( ) );

            // Get work phone number
            tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_PATIENTS_FIELD_WORK_PHONE ] );
            mbMalloc( pat_p->workPhone_p, strlen( tempStr.c_str() ) + 10 );
            strcpy( pat_p->workPhone_p, tempStr.c_str( ) );

            pmcDataSet_p->Next( );
    }
    PMC_GENERAL_QUERY_CLEANUP( );
#endif

    sprintf( buf_p, "select %s,%s,%s,%s from %s where %s>0",
        PMC_SQL_FIELD_ID,
        PMC_SQL_FIELD_POSTAL_CODE,
        PMC_SQL_DOCTORS_FIELD_WORK_PHONE,
        PMC_SQL_DOCTORS_FIELD_WORK_FAX,
        PMC_SQL_TABLE_DOCTORS,
        PMC_SQL_FIELD_ID );

        mbDlgDebug(( "obsolete section" ));
#if 0
    PMC_GENERAL_QUERY_SETUP( buf_p );
    while( !pmcDataSet_p->Eof )
    {
            mbMalloc( pat_p, sizeof( patTemp_t) );
            memset( pat_p, 0, sizeof( patTemp_p ) );
            qInsertFirst( pat_q, pat_p );
            pat_p->code = 1;

            // Get the patient ID
            pat_p->id = pmcDataSet_p->FieldValues[ PMC_SQL_FIELD_ID ];

            // Get the postal code
            tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_FIELD_POSTAL_CODE ] );
            mbMalloc( pat_p->postalCode_p, strlen( tempStr.c_str() ) + 10 );
            strcpy( pat_p->postalCode_p, tempStr.c_str( ) );

            // Get home phone number
            tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_DOCTORS_FIELD_WORK_FAX ] );
            mbMalloc( pat_p->homePhone_p, strlen( tempStr.c_str() ) + 10 );
            strcpy( pat_p->homePhone_p, tempStr.c_str( ) );

            // Get work phone number
            tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_DOCTORS_FIELD_WORK_PHONE ] );
            mbMalloc( pat_p->workPhone_p, strlen( tempStr.c_str() ) + 10 );
            strcpy( pat_p->workPhone_p, tempStr.c_str( ) );

            pmcDataSet_p->Next( );
    }
    PMC_GENERAL_QUERY_CLEANUP( );
#endif

    sprintf( fileName_p, "%s_clean_report.txt", pmcMakeFileName( pmcCfg[CFG_REPORT_DIR].str_p, buf_p ) );
    fp = fopen( fileName_p, "w" );

    if( fp == NIL )
    {
        mbDlgDebug(( "Failed to open report file '%s'", fileName_p ));
        goto exit;
    }

    sprintf( buf_p, "Processing %ld records...", pat_q->size );
    thermometer_p = new TThermometer( buf_p, 0, pat_q->size, FALSE );

    for( ; ; )
    {
        update = FALSE;
        if( qEmpty( pat_q ) ) break;
        pat_p = (patTemp_p)qRemoveFirst( pat_q );

        thermometer_p->Increment( );

        strcpy( orig_p, pat_p->postalCode_p );

        if( pmcPostalCodeFix( pat_p->postalCode_p, &modified ) != TRUE )
        {
            update = TRUE;
            sprintf( pat_p->postalCode_p, "" );
        }
        if( modified == TRUE )
        {
             update = TRUE;
        }

        if( update )
        {
            if( fp )
            {
                fprintf( fp, "Record: %05ld: '%s' -> '%s' (code: %ld)\n",
                    pat_p->id, orig_p, pat_p->postalCode_p, pat_p->code );
            }

            if( pat_p->code == 0 )
            {
                pmcSqlExecString( PMC_SQL_TABLE_PATIENTS, PMC_SQL_FIELD_POSTAL_CODE, pat_p->postalCode_p, pat_p->id );
            }
            else
            {
                pmcSqlExecString( PMC_SQL_TABLE_DOCTORS, PMC_SQL_FIELD_POSTAL_CODE, pat_p->postalCode_p, pat_p->id );
            }
        }

        // Home phone (fax for doctors)
        strcpy( orig_p, pat_p->homePhone_p );
        update = FALSE;
        if( pmcPhoneFix( pat_p->homePhone_p, &modified ) != TRUE )
        {
            sprintf( pat_p->homePhone_p, "" );
            update = TRUE;
        }
        else
        {
            if( modified == TRUE )
            {
                update = TRUE;
            }
        }

        if( update )
        {
            if( fp )
            {
                fprintf( fp, "Record: %05ld: Phone '%s' -> '%s' (code: %ld)\n",
                    pat_p->id, orig_p, pat_p->homePhone_p, pat_p->code );
            }
            if( pat_p->code == 0 )
            {
                pmcSqlExecString( PMC_SQL_TABLE_PATIENTS, PMC_SQL_PATIENTS_FIELD_HOME_PHONE, pat_p->homePhone_p, pat_p->id );
            }
            else
            {
                pmcSqlExecString( PMC_SQL_TABLE_DOCTORS, PMC_SQL_DOCTORS_FIELD_WORK_FAX, pat_p->homePhone_p, pat_p->id );
            }
        }

        // Work phone
        strcpy( orig_p, pat_p->workPhone_p );
        update = FALSE;
        if( pmcPhoneFix( pat_p->workPhone_p, &modified ) != TRUE )
        {
            sprintf( pat_p->workPhone_p, "" );
            update = TRUE;
        }
        else
        {
            if( modified == TRUE )
            {
                update = TRUE;
            }
        }

        if( update )
        {
            if( fp )
            {
                fprintf( fp, "Record: %05ld: Phone '%s' -> '%s' (code: %ld)\n",
                    pat_p->id, orig_p, pat_p->workPhone_p, pat_p->code );
            }
            if( pat_p->code == 0 )
            {
                pmcSqlExecString( PMC_SQL_TABLE_PATIENTS, PMC_SQL_PATIENTS_FIELD_WORK_PHONE, pat_p->workPhone_p, pat_p->id );
            }
            else
            {
                pmcSqlExecString( PMC_SQL_TABLE_DOCTORS, PMC_SQL_DOCTORS_FIELD_WORK_PHONE, pat_p->workPhone_p, pat_p->id );
            }
        }

        if( pat_p->postalCode_p )   mbFree( pat_p->postalCode_p );
        if( pat_p->workPhone_p )    mbFree( pat_p->workPhone_p );
        if( pat_p->homePhone_p )    mbFree( pat_p->homePhone_p );

        mbFree( pat_p );
    }

exit:

    if( fp ) fclose( fp );

    if( thermometer_p ) delete thermometer_p;

    if( buf_p )         mbFree( buf_p );
    if( fileName_p )    mbFree( fileName_p );
    if( orig_p )        mbFree( orig_p );

    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void pmcImportMspFiles( void )
{
    mbDlgInfo( "Feature compiled out.\n" );
#if 0
    qHead_t                 fileListQueue;
    qHead_p                 fileList_q;
    pmcFileListStruct_p     file_p;
    Int32u_t                type;
    Int32u_t                crc;
    Int32u_t                size;
    Char_p                  buf_p;
    bool                    add;

    mbMalloc( buf_p, 256 );
    TDatabaseThermometer   *thermometer_p = NIL;

    fileList_q = qInitialize( &fileListQueue );

    pmcFileListGet( pmcCfg[CFG_MSP_ARCHIVE_DIR].str_p, "*.*", fileList_q );

    sprintf( buf_p, "Processing %ld MSP files...", fileList_q->size );
    thermometer_p = new TDatabaseThermometer( NULL, buf_p );
    thermometer_p->WindowState = wsNormal;
    thermometer_p->Gauge->MinValue = (Int32u_t)0;
    thermometer_p->Gauge->MaxValue = (Int32u_t)fileList_q->size;

    while( !qEmpty( fileList_q ) )
    {
        thermometer_p->Gauge->Progress++;

        file_p = (pmcFileListStruct_p)qRemoveFirst( fileList_q );

        mbLog( "Read file '%s' '%s' crc: %ld\n", file_p->name_p, file_p->fullName_p, file_p->crc );

        type = PMC_MSP_FILE_TYPE_INVALID;

        if( mbStrPos( file_p->name_p, "RETURNS"  ) > 0 ) type = PMC_MSP_FILE_TYPE_RETURNS;

        if( mbStrPos( file_p->name_p, "CLAIMSIN" ) > 0 ) type = PMC_MSP_FILE_TYPE_CLAIMSIN;

        if( mbStrPos( file_p->name_p, "NOTICE"   ) > 0 ) type = PMC_MSP_FILE_TYPE_NOTICE;

        if( mbStrPos( file_p->name_p, "VALIDRPT" ) > 0 ) type = PMC_MSP_FILE_TYPE_VALIDRPT;

        if( type == PMC_MSP_FILE_TYPE_INVALID )
        {
            mbDlgInfo( "Invalid MSP file name '%s'\n", file_p->name_p );
        }
        else
        {
            add = FALSE;
            crc = pmcFileCrc( file_p->fullName_p, &size );
            if( type == PMC_MSP_FILE_TYPE_NOTICE )
            {
                add = TRUE;
            }
            else
            {
                if( ( pmcMspFileDatabaseCheck( crc, size, type ) > 0 ) )
                {
                    mbDlgExclaim( "The file %s is already in the database\n", file_p->name_p );
                }
                else
                {
                    add = TRUE;
                }
            }

            if( add )
            {
                pmcMspFileDatabaseAdd( file_p->fullName_p, type );
            }
        }
        if( file_p->name_p ) mbFree( file_p->name_p );
        if( file_p->fullName_p ) mbFree( file_p->fullName_p );
        mbFree( file_p );
    }

    delete thermometer_p;
    mbFree( buf_p );

    mbDlgInfo( "MSP file import complete" );
#endif
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

#if 0

void __fastcall pmcConvertDrNumbersClick(TObject *Sender)
{
    Char_t  buf[512];
    Char_t  result[32];

    patTemp_p   pat_p;
    docTemp_p   doc_p;
    appTemp_p   app_p;

    qHead_t     pat_q;
    qHead_t     app_q;
    qHead_t     doc_q;

    Int16u_t    temp;
    Int32u_t    old_nums;
    TDataSet   *dataSet_p;
    TDatabaseThermometer   *thermometer_p;
    FILE       *fp = NIL;

    qInitialize( &pat_q );
    qInitialize( &app_q );
    qInitialize( &doc_q );

    mbDlgDebug(( "About to convert all Dr numbers to Dr ids in database!!!" ));

    sprintf( buf, "select health_num from patients where first_name='Michael' and last_name='Bree'" );

    result[0] = 0;

    pmcSqlSelectString( buf, "health_num", &result[0], 32 );

    if( mbStrPos( result, "629324401" ) == -1 )
    {
        mbDlgDebug(( "Database already converted... cannot proceed" ));
        goto exit;
    }

    mbDlgDebug(( "Proceeding with conversion" ));

    fp = fopen( "dr_conv.txt", "w" );
    if( fp == NIL )
    {
        mbDlgDebug(( "Failed to open result file" ));
        goto exit;
    }

    PMC_ACQUIRE_LOCK( pmcPatListLock );
    PMC_ACQUIRE_LOCK( pmcDocListLock );
    PMC_ACQUIRE_LOCK( pmcGeneralQueryLock );

    // First, read all patient records
    dataSet_p = MainDatabaseModule->GeneralQueryDataSource->DataSet;
    sprintf( buf, "select id,%s,%s from patients where id>0",
        PMC_SQL_PATIENTS_FIELD_REFERRING_DR_ID, PMC_SQL_PATIENTS_FIELD_FAMILY_DR_ID );

    MainDatabaseModule->GeneralQuery->SQL->Clear( );
    MainDatabaseModule->GeneralQuery->SQL->Add( buf );
    MainDatabaseModule->GeneralQuery->Close( );
    MainDatabaseModule->GeneralQuery->Open( );

    dataSet_p->DisableControls( );
    try
    {
        dataSet_p->First( );
        while( !dataSet_p->Eof )
        {
            mbMalloc( pat_p, sizeof( patTemp_t) );
            qInsertFirst( &pat_q, pat_p );

            pat_p->id = dataSet_p->FieldValues[ PMC_SQL_FIELD_ID ];
            pat_p->refDr = dataSet_p->FieldValues[ PMC_SQL_PATIENTS_FIELD_REFERRING_DR_ID ];
            pat_p->famDr = dataSet_p->FieldValues[ PMC_SQL_PATIENTS_FIELD_FAMILY_DR_ID ];

            dataSet_p->Next( );
        }
    }
    __finally
    {
        dataSet_p->EnableControls( );
    }

    mbDlgDebug(( "Read %ld patient records", pat_q.size ));

    // Next, read all doctor records
    dataSet_p = MainDatabaseModule->GeneralQueryDataSource->DataSet;
    sprintf( buf, "select id,%s from doctors where id>0",
        PMC_SQL_DOCTORS_FIELD_DOCTOR_NUMBER );

    MainDatabaseModule->GeneralQuery->SQL->Clear( );
    MainDatabaseModule->GeneralQuery->SQL->Add( buf );
    MainDatabaseModule->GeneralQuery->Close( );
    MainDatabaseModule->GeneralQuery->Open( );

    dataSet_p->DisableControls( );
    try
    {

        dataSet_p->First( );
        while( !dataSet_p->Eof )
        {
            mbMalloc( doc_p, sizeof( docTemp_t) );
            qInsertFirst( &doc_q, doc_p );

            doc_p->id = dataSet_p->FieldValues[ PMC_SQL_FIELD_ID ];
            doc_p->num = dataSet_p->FieldValues[ PMC_SQL_DOCTORS_FIELD_DOCTOR_NUMBER ];

            dataSet_p->Next( );
        }
    }
    __finally
    {
        dataSet_p->EnableControls( );
    }

    mbDlgDebug(( "Read %ld doctor records", doc_q.size ));

    // Next, read appointment  records
    dataSet_p = MainDatabaseModule->GeneralQueryDataSource->DataSet;
    sprintf( buf, "select id,%s,%s from appointments where id>0",
        PMC_SQL_APPS_FIELD_REFERRING_DR_ID, PMC_SQL_APPS_FIELD_FAMILY_DR_ID );

    MainDatabaseModule->GeneralQuery->SQL->Clear( );
    MainDatabaseModule->GeneralQuery->SQL->Add( buf );
    MainDatabaseModule->GeneralQuery->Close( );
    MainDatabaseModule->GeneralQuery->Open( );

    dataSet_p->DisableControls( );
    try
    {
        dataSet_p->First( );
        while( !dataSet_p->Eof )
        {
            mbMalloc( app_p, sizeof( appTemp_t) );
            qInsertFirst( &app_q, app_p );

            app_p->id = dataSet_p->FieldValues[ PMC_SQL_FIELD_ID ];
            app_p->refDr = dataSet_p->FieldValues[ PMC_SQL_APPS_FIELD_REFERRING_DR_ID ];
            app_p->famDr = dataSet_p->FieldValues[ PMC_SQL_APPS_FIELD_FAMILY_DR_ID ];

            dataSet_p->Next( );
        }
    }
    __finally
    {
        dataSet_p->EnableControls( );
    }

    PMC_RELEASE_LOCK( pmcGeneralQueryLock );

    mbDlgDebug(( "Read %ld appointment records", app_q.size ));

    PMC_RELEASE_LOCK( pmcPatListLock );
    PMC_RELEASE_LOCK( pmcDocListLock );

    sprintf( buf, "Converting Dr. numbers to Dr. IDs... " );
    thermometer_p = new TDatabaseThermometer( NULL, buf );
    thermometer_p->WindowState = wsNormal;
    thermometer_p->Gauge->MinValue = (Int32u_t)0;
    thermometer_p->Gauge->MaxValue = (Int32u_t)(pat_q.size + app_q.size);


    // Loop through the patient records converting dr number to dr id
    for( pat_p = (patTemp_p)qFirst( &pat_q );
                             qDone( &pat_q );
         pat_p = (patTemp_p) qNext( &pat_q ) )
    {
        thermometer_p->Gauge->Progress++;

        old_nums = 0;
        if( pat_p->refDr )
        {
            temp = (Int16u_t)pat_p->refDr;

            old_nums = (Int32u_t)(temp *10000);

            pat_p->refDr = pmcDrNumToId( pat_p->refDr, &doc_q );

            pmcSqlExecInt( PMC_SQL_TABLE_PATIENTS, PMC_SQL_PATIENTS_FIELD_REFERRING_DR_ID, pat_p->refDr, pat_p->id );

            if( pat_p->refDr == 0 )
            {
                fprintf( fp, "failed to find dr: %ld (patient id: %ld)\n", temp, pat_p->id );
            }

        }
        if( pat_p->famDr )
        {
            temp = (Int16u_t)pat_p->famDr;
            old_nums += temp;
            pat_p->famDr = pmcDrNumToId( pat_p->famDr, &doc_q );
            pmcSqlExecInt( PMC_SQL_TABLE_PATIENTS, PMC_SQL_PATIENTS_FIELD_FAMILY_DR_ID, pat_p->famDr, pat_p->id );
            if( pat_p->famDr == 0 )
            {
                fprintf( fp, "failed to find dr: %ld (patient id: %ld)\n", temp, pat_p->id );
            }
        }
        if( old_nums )
        {
            pmcSqlExecInt( PMC_SQL_TABLE_PATIENTS, "future_int_1", old_nums, pat_p->id );
        }
    }

    mbDlgDebug(( "Finished processing patient records" ));

    // Loop through the appointment records converting dr number to dr id
    for( app_p = (appTemp_p)qFirst( &app_q );
                             qDone( &app_q );
         app_p = (appTemp_p) qNext( &app_q ) )
    {
        thermometer_p->Gauge->Progress++;

        old_nums = 0;

        if( app_p->refDr )
        {
            temp = (Int16u_t)app_p->refDr;
            old_nums = (Int32u_t)(temp *10000);
            app_p->refDr = pmcDrNumToId( app_p->refDr, &doc_q );
            pmcSqlExecInt( PMC_SQL_TABLE_APPS, PMC_SQL_APPS_FIELD_REFERRING_DR_ID, app_p->refDr, app_p->id );
            if( app_p->refDr == 0 )
            {
                fprintf( fp, "failed to find dr: %d (appoint id: %ld)\n", temp, app_p->id );
            }
        }
        if( app_p->famDr )
        {
            temp = (Int16u_t)app_p->famDr;
            old_nums += temp;
            app_p->famDr = pmcDrNumToId( app_p->famDr, &doc_q );
            pmcSqlExecInt( PMC_SQL_TABLE_APPS, PMC_SQL_APPS_FIELD_FAMILY_DR_ID, app_p->famDr, app_p->id );
            if( app_p->famDr == 0 )
            {
                fprintf( fp, "failed to find dr: %d (appoint id: %ld)\n", temp, app_p->id );
            }
        }
        if( old_nums )
        {
            pmcSqlExecInt( PMC_SQL_TABLE_APPS, "future_int_2", old_nums, app_p->id );
        }
    }

    delete thermometer_p;

    mbDlgInfo( "Conversion Complete" );

exit:
    if( fp ) fclose( fp );

    for( ; ; )
    {
        if( qEmpty( &pat_q ) ) break;
        pat_p = (patTemp_p)qRemoveFirst( &pat_q );
        mbFree( pat_p );
    }

    for( ; ; )
    {
        if( qEmpty( &doc_q ) ) break;
        doc_p = (docTemp_p)qRemoveFirst( &doc_q );
        mbFree( doc_p );
    }

    for( ; ; )
    {
        if( qEmpty( &app_q ) ) break;
        app_p = (appTemp_p)qRemoveFirst( &app_q );
        mbFree( app_p );
    }
    return;
}

#endif

#if 0
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t  pmcDrNumToId( Int32u_t  drNum, qHead_p  q_p )
{
    docTemp_p   doc_p;
    bool        match;
    Int32u_t    returnValue = 0;

    for( doc_p = (docTemp_p)qInit( q_p ) ;
                            qDone( q_p ) ;
         doc_p = (docTemp_p)qNext( q_p ) )
    {
        if( doc_p->doctorNumber == drNum )
        {
            match = TRUE;
            break;
        }
    }

    if( match )
    {
        returnValue = doc_p->id;
    }
    else
    {
//        mbDlgDebug(( "Could not find dr number %ld\n", drNum ));
    }
    return returnValue;
}

#endif

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void pmcSmaDoctorListImport( void )
{
    FILE       *fp = NIL;
    FILE       *report_p = NIL;
    Char_p      buf1_p;
    Char_p      buf2_p;
    Char_p      buf3_p;
    Char_p      temp1_p;

    Int32u_t    lineCount = 0;
    Int32u_t    doctorId, intCount;
    Ints_t      tabCount, l, i;
    Int32u_t    doctorNumber, doctorNumberZeroCount;
    Int32u_t    recordCount;
    Char_p      field_p[16];
    TThermometer   *thermometer_p = NIL;

    mbMalloc( buf1_p, 512 );
    mbMalloc( buf2_p, 512 );
    mbMalloc( buf3_p, 512 );

    doctorNumberZeroCount = 0;
    recordCount = 0;

    // First lets get a file name
    MainForm->OpenDialog->Title = "Select SMA physcian list file";
    MainForm->OpenDialog->Filter = "Text files (*.txt)|*.TXT";
    if( !MainForm->OpenDialog->Execute( ) )
    {
        goto exit;
    }

    nbDlgDebug(( "Got file name '%s'", MainForm->OpenDialog->FileName.c_str() ));

    if( ( fp = fopen( MainForm->OpenDialog->FileName.c_str(), "r" ) ) == NIL )
    {
        mbDlgError( "Could not open SMA file '%s'",  MainForm->OpenDialog->FileName.c_str() );
        goto exit;
    }

    if( ( report_p = fopen( "sma_import_report.txt", "w" ) ) == NIL )
    {
        mbDlgError( "Could not open SMA import report file 'sma_import_report.txt'" );
        goto exit;
    }

    // This first loop just reads the file to determine how many lines there are
    while( fgets( buf1_p, 512, fp ) != 0 )
    {
        // Null terminate buffer (in case some kind of binary data was read)
        *(buf1_p + 511) = 0;
        l = strlen( buf1_p );

        // Check that this line looks valid
        for( temp1_p = buf1_p, tabCount = 0, i = 0 ; i < l ; i++, temp1_p++ )
        {
            if( *temp1_p == '\t' ) tabCount++;
        }

        nbDlgDebug(( "Got tab count: %ld", tabCount ));

        if( tabCount != ( PMC_SMA_DR_LIST_FIELDS - 1 ) )
        {
            if( mbDlgOkCancel( "Read invalid line (tabCount: %ld). Continue reading file?", tabCount ) == MB_BUTTON_CANCEL )
            {
                mbDlgExclaim( "Terminating read of invalid file." );
                goto exit;
            }
            mbDlgDebug(( "Line: '%s'", buf1_p ));
        }

        // Skip first line in the file
        if( lineCount++ == 0 ) continue;

        recordCount++;
    }

    rewind( fp );

    sprintf( buf1_p, "Processing %ld doctor records...", recordCount );
    thermometer_p = new TThermometer( buf1_p, 0, recordCount, FALSE );

    recordCount = 0;
    lineCount = 0;

    while( fgets( buf1_p, 512, fp ) != 0 )
    {
        // Null terminate buffer (in case some kind of binary data was read)
        *(buf1_p + 511) = 0;
        l = strlen( buf1_p );

        // Check that this line looks valid
        for( temp1_p = buf1_p, tabCount = 0, i = 0 ; i < l ; i++, temp1_p++ )
        {
            if( *temp1_p == '\t' ) tabCount++;
        }

        nbDlgDebug(( "Got tab count: %ld", tabCount ));

        if( tabCount != ( PMC_SMA_DR_LIST_FIELDS - 1 ) )
        {
            if( mbDlgOkCancel( "Read invalid line. Continue reading file?" ) == MB_BUTTON_CANCEL )
            {
                mbDlgExclaim( "Terminating read of invalid file." );
                goto exit;
            }
        }

        // Skip first line in the file
        if( lineCount++ == 0 ) continue;

        recordCount++;
//        thermometer_p->Gauge->Progress++;
        thermometer_p->Increment( );

        nbDlgDebug(( "Read line '%s'", buf1_p ));

        temp1_p = buf1_p;
        for( i = 0 ; i < PMC_SMA_DR_LIST_FIELDS ; i++ )
        {
            field_p[i] = temp1_p;
            for( ; ; temp1_p++ )
            {
                if( *temp1_p == '\t' )
                {
                    *temp1_p++ = 0;
                    break;
                }
                if( *temp1_p == 0 ) break;
            }
        }

        sprintf( buf3_p, "fields:\n\n" );

        for( i = 0 ; i < PMC_SMA_DR_LIST_FIELDS ; i++ )
        {
            mbStrClean( field_p[i], buf2_p, TRUE );
            strcpy( field_p[i], buf2_p );
            sprintf( buf2_p, "field_p[%d] = '%s'\n", i, field_p[i] );
            strcat( buf3_p, buf2_p );
        }
        nbDlgDebug(( buf3_p ));

        if( strlen( field_p[PMC_SMA_FIELD_DOCTOR_NUMBER] ) > 0 )
        {
            doctorNumber = atoi( field_p[PMC_SMA_FIELD_DOCTOR_NUMBER] );
            if( doctorNumber == 0 )
            {
                fprintf( report_p, "Ignoring SMA record with doctor number: 0 (%s %s)\n",
                    field_p[PMC_SMA_FIELD_FIRST_NAME], field_p[PMC_SMA_FIELD_LAST_NAME] );
                doctorNumberZeroCount++;
                continue;
            }

            sprintf( buf2_p, "select %s from %s where %s=%ld and %s=%ld",
                PMC_SQL_FIELD_ID,
                PMC_SQL_TABLE_DOCTORS,
                PMC_SQL_DOCTORS_FIELD_DOCTOR_NUMBER, doctorNumber,
                PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

            //doctorId = pmcSqlSelectInt( buf2_p, PMC_SQL_DOCTORS_FIELD_ID, &intCount );
            doctorId = pmcSqlSelectInt( buf2_p, &intCount );
            if( intCount == 1 )
            {
                // Record exits
                pmcSmaDoctorRecordUpdate( doctorId, &field_p[0], FALSE, report_p );
            }
            else if( intCount == 0 )
            {
                // Record does not exist, create it
                pmcSmaDoctorRecordUpdate( 0, &field_p[0], TRUE, report_p );
            }
            else
            {
                mbDlgDebug(( "Error: found %ld records with doctor number '%s'\n",
                    intCount, field_p[PMC_SMA_FIELD_DOCTOR_NUMBER] ));
            }
        }
    }

exit:

    if( thermometer_p ) delete thermometer_p;

    mbDlgInfo( "Processed %ld doctor records.", recordCount );

    if( fp ) fclose( fp );
    if( report_p )fclose( report_p );
    if( buf1_p ) mbFree( buf1_p );
    if( buf2_p ) mbFree( buf2_p );
    if( buf3_p ) mbFree( buf3_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void pmcSmaDoctorRecordUpdate
(
    Int32u_t    doctorId,
    Char_p     *field_pp,
    bool        createFlag,
    FILE       *fp
)
{
    mbDlgExclaim( "Feature compiled out." );

#if 0
    Char_p      buf1_p;
    Char_p      buf2_p;
    Char_p      buf3_p;
    Char_p      buf4_p;
    Ints_t      i;
    Int32u_t    rowCount = 0;
    AnsiString  tempStr = "";
    Int32u_t    doctorNumber;
    bool        updateFlag;
    bool        queryFlag = FALSE;
    bool        recordLocked = FALSE;
    Char_p      lastName_p = NIL;
    Char_p      firstName_p = NIL;
    Char_p      middleName_p = NIL;
    Char_p      fax_p = NIL;
    Char_p      phone_p = NIL;
    Char_p      country_p = NIL;
    Char_p      postalCode_p = NIL;
    Char_p      province_p = NIL;
    Char_p      city_p = NIL;
    Char_p      address1_p = NIL;
    Char_p      address2_p = NIL;
    Ints_t      len1, len2, len3, len4;
    Ints_t      group1, group2;

    
    mbMalloc( buf1_p, 512 );
    mbMalloc( buf2_p, 512 );
    mbMalloc( buf3_p, 512 );
    mbMalloc( buf4_p, 512 );

    sprintf( buf1_p, "Fields\n\n" );
    for( i = 0 ; i < PMC_SMA_DR_LIST_FIELDS ; i++ )
    {
        sprintf( buf2_p, "field_p[%d] = '%s'\n", i, field_pp[i] );
        strcat( buf1_p, buf2_p );
    }

    nbDlgDebug(( buf1_p ));

    if( createFlag == TRUE )
    {
        // Create a new record
        doctorId = pmcSqlRecordCreate( PMC_SQL_TABLE_DOCTORS );
        if( doctorId == 0 )
        {
            mbDlgDebug(( "Error creating doctor record" ));
            goto exit;
        }
        fprintf( fp, "Record: %05ld -> Create\n", doctorId );
    }

    // Lock the record
    if( pmcSqlRecordLock( PMC_SQL_TABLE_DOCTORS, doctorId, ( createFlag == TRUE ) ? FALSE : TRUE ) )
    {
        recordLocked = TRUE;
        if( createFlag == TRUE )
        {
            doctorNumber = atoi( field_pp[PMC_SMA_FIELD_DOCTOR_NUMBER] );

            // An entry with this doctor number should not exist... the
            // table has already been queried for an entry with this
            // doctor number

            pmcSqlExecInt( PMC_SQL_TABLE_DOCTORS,
                           PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE,
                           doctorId );

            pmcSqlExecInt( PMC_SQL_TABLE_DOCTORS,
                           PMC_SQL_DOCTORS_FIELD_DOCTOR_NUMBER, doctorNumber,
                           doctorId );
            fprintf( fp, "Record: %05ld: %s -> %ld\n", doctorId, PMC_SQL_DOCTORS_FIELD_DOCTOR_NUMBER, doctorNumber );

            pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                              PMC_SQL_DOCTORS_FIELD_COUNTRY, "Canada",
                              doctorId );

            sprintf( buf1_p, "Record created automatically by Practice Manager SMA data import" );
            pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                              PMC_SQL_DOCTORS_FIELD_COMMENT, buf1_p,
                              doctorId );
        }

        // Now process remainder of fields in the record... presumably the
        // data in the import file is more up to date, so it will take priority
        sprintf( buf1_p, "select %s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s from %s where %s=%ld",
            PMC_SQL_DOCTORS_FIELD_DOCTOR_NUMBER,
            PMC_SQL_DOCTORS_FIELD_LAST_NAME,
            PMC_SQL_DOCTORS_FIELD_FIRST_NAME,
            PMC_SQL_DOCTORS_FIELD_MIDDLE_NAME,
            PMC_SQL_DOCTORS_FIELD_ADDRESS1,
            PMC_SQL_DOCTORS_FIELD_ADDRESS2,
            PMC_SQL_DOCTORS_FIELD_CITY,
            PMC_SQL_DOCTORS_FIELD_PROVINCE,
            PMC_SQL_DOCTORS_FIELD_POSTAL_CODE,
            PMC_SQL_DOCTORS_FIELD_WORK_PHONE,
            PMC_SQL_DOCTORS_FIELD_WORK_FAX,
            PMC_SQL_DOCTORS_FIELD_COUNTRY,
            PMC_SQL_TABLE_DOCTORS,
            PMC_SQL_FIELD_ID, doctorId );

        PMC_GENERAL_QUERY_SETUP( buf1_p );
        while( !pmcDataSet_p->Eof )
        {
            doctorNumber = pmcDataSet_p->FieldValues[ PMC_SQL_DOCTORS_FIELD_DOCTOR_NUMBER ];

            // Last name
            tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_DOCTORS_FIELD_LAST_NAME ] );
            mbMalloc( lastName_p, strlen( tempStr.c_str( ) ) + 1 );
            strcpy( lastName_p, tempStr.c_str() );

            // First name
            tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_DOCTORS_FIELD_FIRST_NAME ] );
            mbMalloc( firstName_p, strlen( tempStr.c_str( ) ) + 1 );
            strcpy( firstName_p, tempStr.c_str() );

            // First name
            tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_DOCTORS_FIELD_MIDDLE_NAME ] );
            mbMalloc( middleName_p, strlen( tempStr.c_str( ) ) + 1 );
            strcpy( middleName_p, tempStr.c_str() );

            tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_DOCTORS_FIELD_ADDRESS1 ] );
            mbMalloc( address1_p, strlen( tempStr.c_str( ) ) + 1 );
            strcpy( address1_p, tempStr.c_str() );

            tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_DOCTORS_FIELD_ADDRESS2 ] );
            mbMalloc( address2_p, strlen( tempStr.c_str( ) ) + 1 );
            strcpy( address2_p, tempStr.c_str() );

            tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_DOCTORS_FIELD_CITY ] );
            mbMalloc( city_p, strlen( tempStr.c_str( ) ) + 1 );
            strcpy( city_p, tempStr.c_str() );

            tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_DOCTORS_FIELD_POSTAL_CODE ] );
            mbMalloc( postalCode_p, strlen( tempStr.c_str( ) ) + 1 );
            strcpy( postalCode_p, tempStr.c_str() );

            tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_DOCTORS_FIELD_PROVINCE ] );
            mbMalloc( province_p, strlen( tempStr.c_str( ) ) + 1 );
            strcpy( province_p, tempStr.c_str() );

            tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_DOCTORS_FIELD_COUNTRY ] );
            mbMalloc( country_p, strlen( tempStr.c_str( ) ) + 1 );
            strcpy( country_p, tempStr.c_str() );

            tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_DOCTORS_FIELD_WORK_PHONE ] );
            mbMalloc( phone_p, strlen( tempStr.c_str( ) ) + 1 );
            strcpy( phone_p, tempStr.c_str() );

            tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_DOCTORS_FIELD_WORK_FAX ] );
            mbMalloc( fax_p, strlen( tempStr.c_str( ) ) + 1 );
            strcpy( fax_p, tempStr.c_str() );

            rowCount++;
            pmcDataSet_p->Next( );
        }
        PMC_GENERAL_QUERY_CLEANUP( );

        // Sanity check
        if( rowCount != 1 ) PMC_ERROR_BOX(( "Row count: %ld\n", rowCount ));

        // SMA records combine middle and first names
        pmcSmaCleanFirstName( field_pp[PMC_SMA_FIELD_FIRST_NAME], buf1_p, buf2_p );

        queryFlag = FALSE;
        if( strlen( firstName_p) > 0 && strcmp( firstName_p, buf1_p ) != 0 ) queryFlag = TRUE;
        if( strlen( lastName_p)  > 0 && strcmp( lastName_p,  field_pp[PMC_SMA_FIELD_LAST_NAME] ) != 0 ) queryFlag = TRUE;

        if( queryFlag )
        {
            sprintf( buf3_p, "Names differ for doctor number %ld\n\n", doctorNumber );
            sprintf( buf4_p, "In database:\t%s %s\n", firstName_p, lastName_p );
            strcat( buf3_p, buf4_p );
            sprintf( buf4_p, "In SMA file:\t%s %s\n", buf1_p, field_pp[PMC_SMA_FIELD_LAST_NAME] );
            strcat( buf3_p, buf4_p );
            sprintf( buf4_p, "\nProceed with update of this doctor's record?" );
            strcat( buf3_p, buf4_p );

            if( mbDlgOkCancel( buf3_p ) == MB_BUTTON_CANCEL )
            {
                fprintf( fp,  "Record %05ld: Skipping update\n", doctorId );
                mbDlgExclaim( "Skipping update of record for %s, %s", firstName_p, lastName_p )
                goto exit;
            }
        }

        if( strlen( country_p ) == 0 )
        {
            // Just force all records to Canada
            pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                              PMC_SQL_DOCTORS_FIELD_COUNTRY, "Canada",
                              doctorId );
        }

        // Update first name
        if( strlen( buf1_p ) > 0 && strcmp( firstName_p, buf1_p ) != 0 )
       {
            fprintf( fp, "Record: %05ld %s: '%s' -> '%s'\n", doctorId,
                PMC_SQL_DOCTORS_FIELD_FIRST_NAME, firstName_p, buf1_p );

            pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                              PMC_SQL_DOCTORS_FIELD_FIRST_NAME, buf1_p,
                              doctorId );
        }

        // Update middle name
        if( strlen( buf2_p ) > 0 && strcmp( middleName_p, buf2_p ) != 0 )
        {
           fprintf( fp, "Record: %05ld %s: '%s' -> '%s'\n", doctorId,
                PMC_SQL_DOCTORS_FIELD_MIDDLE_NAME, middleName_p, buf2_p );

           pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                             PMC_SQL_DOCTORS_FIELD_MIDDLE_NAME, buf2_p,
                             doctorId );
        }

        // Update last name
        if( strlen( field_pp[PMC_SMA_FIELD_LAST_NAME] ) > 0 && strcmp( lastName_p, field_pp[PMC_SMA_FIELD_LAST_NAME] ) != 0 )
        {
           fprintf( fp, "Record: %05ld %s: '%s' -> '%s'\n", doctorId,
                PMC_SQL_DOCTORS_FIELD_LAST_NAME, lastName_p, field_pp[PMC_SMA_FIELD_LAST_NAME] );

           pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                             PMC_SQL_DOCTORS_FIELD_LAST_NAME, field_pp[PMC_SMA_FIELD_LAST_NAME],
                             doctorId );
        }

        // Update fax number
        if( strlen( field_pp[PMC_SMA_FIELD_FAX] ) > 0 && strcmp( fax_p, field_pp[PMC_SMA_FIELD_FAX] ) != 0 )
        {
            strcpy( buf1_p, field_pp[PMC_SMA_FIELD_FAX] );
            if( pmcPhoneFix( buf1_p, NIL ) == FALSE )
            {
                fprintf( fp, "Record: %05ld %s: INVALID fax number '%s'\n", doctorId,
                    PMC_SQL_DOCTORS_FIELD_WORK_FAX, buf1_p );
                    mbDlgDebug(( "Detected invalid fax number '%s'.", buf1_p ));
            }

            fprintf( fp, "Record: %05ld %s: '%s' -> '%s'\n", doctorId,
                PMC_SQL_DOCTORS_FIELD_WORK_FAX, fax_p, buf1_p );

            pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                              PMC_SQL_DOCTORS_FIELD_WORK_FAX, buf1_p,
                              doctorId );
        }

        // Update phone number
        if( strlen( field_pp[PMC_SMA_FIELD_PHONE] ) > 0 && strcmp( phone_p, field_pp[PMC_SMA_FIELD_PHONE] ) != 0 )
        {
            strcpy( buf1_p, field_pp[PMC_SMA_FIELD_PHONE] );
            if( pmcPhoneFix( buf1_p, NIL ) == FALSE )
            {
               fprintf( fp, "Record: %05ld %s: INVALID phone number '%s'\n", doctorId,
                    PMC_SQL_DOCTORS_FIELD_WORK_FAX, buf1_p );
               mbDlgDebug(( "Detected invalid phone number '%s'.", buf1_p ));
            }

            fprintf( fp, "Record: %05ld %s: '%s' -> '%s'\n", doctorId,
                PMC_SQL_DOCTORS_FIELD_WORK_PHONE, phone_p, buf1_p );

            pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                              PMC_SQL_DOCTORS_FIELD_WORK_PHONE, buf1_p,
                              doctorId );
        }

        // Update postal code
        if( strlen( field_pp[PMC_SMA_FIELD_POSTAL_CODE] ) > 0 && strcmp( postalCode_p, field_pp[PMC_SMA_FIELD_POSTAL_CODE] ) != 0 )
        {
            strcpy( buf1_p, field_pp[PMC_SMA_FIELD_POSTAL_CODE] );
            if( pmcPostalCodeFix( buf1_p, NIL ) == FALSE )
            {
               fprintf( fp, "Record: %05ld %s: INVALID postal code '%s'\n", doctorId,
                    PMC_SQL_DOCTORS_FIELD_POSTAL_CODE, buf1_p );
               mbDlgDebug(( "Detected invalid postal code '%s'.", buf1_p ));
            }

            fprintf( fp, "Record: %05ld %s: '%s' -> '%s'\n", doctorId,
                PMC_SQL_DOCTORS_FIELD_POSTAL_CODE, postalCode_p, buf1_p );

            pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                              PMC_SQL_DOCTORS_FIELD_POSTAL_CODE, buf1_p,
                              doctorId );
        }

        // Update province
        if( strlen( field_pp[PMC_SMA_FIELD_PROVINCE] ) > 0 && strcmp( province_p, field_pp[PMC_SMA_FIELD_PROVINCE] ) != 0 )
        {
            strcpy( buf1_p, field_pp[PMC_SMA_FIELD_PROVINCE] );
            if( pmcProvinceFix( buf1_p, NIL ) == FALSE )
            {
               fprintf( fp, "Record: %05ld %s: INVALID province '%s'\n", doctorId,
                    PMC_SQL_DOCTORS_FIELD_PROVINCE, buf1_p );
               mbDlgDebug(( "Detected invalid province '%s'.", buf1_p ));
            }

            fprintf( fp, "Record: %05ld %s: '%s' -> '%s'\n", doctorId,
                PMC_SQL_DOCTORS_FIELD_PROVINCE, province_p, buf1_p );

            pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                              PMC_SQL_DOCTORS_FIELD_PROVINCE, buf1_p,
                              doctorId );
        }

        // Update city
        if( strlen( field_pp[PMC_SMA_FIELD_CITY] ) > 0 && strcmp( city_p, field_pp[PMC_SMA_FIELD_CITY] ) != 0 )
        {
            strcpy( buf1_p, field_pp[PMC_SMA_FIELD_CITY] );

            fprintf( fp, "Record: %05ld %s: '%s' -> '%s'\n", doctorId,
                PMC_SQL_DOCTORS_FIELD_CITY, city_p, buf1_p );

            pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                              PMC_SQL_DOCTORS_FIELD_CITY, buf1_p,
                              doctorId );
        }

        sprintf( buf1_p, "address:\n\n%d: %s\n%d: %s\n%d: %s\n%d: %s\n",
            strlen( field_pp[PMC_SMA_FIELD_ADDRESS1] ), field_pp[PMC_SMA_FIELD_ADDRESS1],
            strlen( field_pp[PMC_SMA_FIELD_ADDRESS2] ), field_pp[PMC_SMA_FIELD_ADDRESS2],
            strlen( field_pp[PMC_SMA_FIELD_ADDRESS3] ), field_pp[PMC_SMA_FIELD_ADDRESS3],
            strlen( field_pp[PMC_SMA_FIELD_ADDRESS4] ), field_pp[PMC_SMA_FIELD_ADDRESS4] );

        nbDlgDebug(( buf1_p ));

        sprintf( buf1_p, "" );
        sprintf( buf2_p, "" );
        // Update address
        if( strlen( field_pp[PMC_SMA_FIELD_ADDRESS3] ) == 0 &&
            strlen( field_pp[PMC_SMA_FIELD_ADDRESS4] ) == 0  )
        {
            if( strlen( field_pp[PMC_SMA_FIELD_ADDRESS1] ) ) strcpy( buf1_p, field_pp[PMC_SMA_FIELD_ADDRESS1] );
            if( strlen( field_pp[PMC_SMA_FIELD_ADDRESS2] ) ) strcpy( buf2_p, field_pp[PMC_SMA_FIELD_ADDRESS2] );
        }
        else if( strlen( field_pp[PMC_SMA_FIELD_ADDRESS3] ) != 0 &&
                 strlen( field_pp[PMC_SMA_FIELD_ADDRESS4] ) != 0 )
        {
            sprintf( buf1_p, "%s %s", field_pp[PMC_SMA_FIELD_ADDRESS1], field_pp[PMC_SMA_FIELD_ADDRESS2] );
            sprintf( buf2_p, "%s %s", field_pp[PMC_SMA_FIELD_ADDRESS3], field_pp[PMC_SMA_FIELD_ADDRESS4] );
        }
        else if(  strlen( field_pp[PMC_SMA_FIELD_ADDRESS3] ) != 0 &&
                  strlen( field_pp[PMC_SMA_FIELD_ADDRESS4] ) == 0 )
        {
            len1 = sprintf( buf1_p, "%s %s", field_pp[PMC_SMA_FIELD_ADDRESS1], field_pp[PMC_SMA_FIELD_ADDRESS2] );
            len2 = sprintf( buf2_p, "%s", field_pp[PMC_SMA_FIELD_ADDRESS3] );
            len3 = sprintf( buf3_p, "%s", field_pp[PMC_SMA_FIELD_ADDRESS1] );
            len4 = sprintf( buf4_p, "%s %s", field_pp[PMC_SMA_FIELD_ADDRESS2], field_pp[PMC_SMA_FIELD_ADDRESS3] );

            group1 = ( len1 > len2 ) ? len1 : len2;
            group2 = ( len3 > len4 ) ? len3 : len4;

            if( group1 > group2 )
            {
                // Use combination with shortest overall length
                strcpy( buf1_p, buf3_p );
                strcpy( buf2_p, buf4_p );
            }
        }
        else
        {
            mbDlgDebug(( "Got funny address" ));
        }
        updateFlag = FALSE;
        if( strlen( buf1_p ) > 0 && strcmp( address1_p, buf1_p ) != 0 ) updateFlag = TRUE;
        if( strlen( buf2_p ) > 0 && strcmp( address2_p, buf2_p ) != 0 ) updateFlag = TRUE;

        if( updateFlag )
        {
            fprintf( fp, "Record: %05ld %s: '%s' -> '%s'\n", doctorId,
                PMC_SQL_DOCTORS_FIELD_ADDRESS1, address1_p, buf1_p );

            pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                              PMC_SQL_DOCTORS_FIELD_ADDRESS1, buf1_p,
                              doctorId );

            fprintf( fp, "Record: %05ld %s: '%s' -> '%s'\n", doctorId,
                PMC_SQL_DOCTORS_FIELD_ADDRESS1, address2_p, buf2_p );

            pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                              PMC_SQL_DOCTORS_FIELD_ADDRESS2, buf2_p,
                              doctorId );
        }
    }
    else
    {
        mbDlgDebug(( "Failed to lock record" ));
    }

exit:

    if( recordLocked ) pmcSqlRecordUnlock( PMC_SQL_TABLE_DOCTORS, doctorId );

    if( lastName_p )    mbFree( lastName_p );
    if( firstName_p )   mbFree( firstName_p );
    if( middleName_p )  mbFree( middleName_p );
    if( fax_p )         mbFree( fax_p );
    if( phone_p )       mbFree( phone_p );
    if( postalCode_p )  mbFree( postalCode_p );
    if( country_p )     mbFree( country_p );
    if( province_p )    mbFree( province_p );
    if( city_p )        mbFree( city_p );
    if( address1_p )    mbFree( address1_p );
    if( address2_p )    mbFree( address2_p );

    if( buf1_p ) mbFree( buf1_p );
    if( buf2_p ) mbFree( buf2_p );
    if( buf3_p ) mbFree( buf3_p );
    if( buf4_p ) mbFree( buf4_p );
#endif
}

#if 0
//---------------------------------------------------------------------------
// Function: pmcStolenPatientRecords()
//---------------------------------------------------------------------------
// Description:
//
// This function will examine all patient records for a particular
// provider and assign them to an alternate provider if that patient has
// appointments or claims with the alternate provider.
//---------------------------------------------------------------------------

void pmcStolenPatientLetters( void )
{
    Char_p          buf1_p = NIL;
    qHead_t         doc_qhead;
    qHead_p         doc_q;
    qHead_t         doc2_qhead;
    qHead_p         doc2_q;
    pmcDocumentMaintStruct_p    doc_p;
    pmcDocumentMaintStruct_p    doc2_p;
    PmcSqlPatient_p   pat_p;

    Int32u_t                count = 0;
    TDatabaseThermometer   *thermometer_p = NIL;
    bool                    duplicate;
    Int32u_t                duplicateCount = 0;

    mbMalloc( buf1_p, 2048 );

    doc_q  = qInitialize( &doc_qhead );
    doc2_q = qInitialize( &doc2_qhead );

    mbCalloc( pat_p, sizeof(PmcSqlPatient_t) );

    // First, get all the patient ids from the database
    sprintf( buf1_p, "select %s from %s where %s=1 and not_deleted=1 and created < 20010922",
        PMC_SQL_DOCUMENTS_FIELD_PATIENT_ID,
        PMC_SQL_TABLE_DOCUMENTS,
        PMC_SQL_DOCUMENTS_FIELD_PROVIDER_ID );
    //sprintf( buf1_p, "select %s from %s where %s=1 and not_deleted=1 and created < 20010922",
    //    PMC_SQL_DOCUMENTS_FIELD_PATIENT_ID,
    //    PMC_SQL_TABLE_CLAIMS,
    //    PMC_SQL_DOCUMENTS_FIELD_PROVIDER_ID );

    PMC_GENERAL_QUERY_SETUP( buf1_p );
    while( !pmcDataSet_p->Eof )
    {
        mbCalloc( doc_p, sizeof( pmcDocumentMaintStruct_t ) );
        qInsertFirst( doc_q, doc_p );

        // Get the patient ID
        doc_p->patientId = pmcDataSet_p->FieldValues[ PMC_SQL_DOCUMENTS_FIELD_PATIENT_ID ];

        pmcDataSet_p->Next( );
    }
    PMC_GENERAL_QUERY_CLEANUP( );

    sprintf( buf1_p, "Processing %ld records...", doc_q->size );
    thermometer_p = new TDatabaseThermometer( NULL, buf1_p );
    thermometer_p->WindowState = wsNormal;
    thermometer_p->Gauge->MinValue = (Int32u_t)0;
    thermometer_p->Gauge->MaxValue = (Int32u_t)doc_q->size;

    mbLog( "found %ld records\n", doc_q->size );
    for( ; ; )
    {
        if( qEmpty( doc_q ) ) break;
        doc_p = (pmcDocumentMaintStruct_p)qRemoveFirst( doc_q );

        duplicate = FALSE;
        for( doc2_p = (pmcDocumentMaintStruct_p)qInit( doc2_q );
                                                qDone( doc2_q );
             doc2_p = (pmcDocumentMaintStruct_p)qNext( doc2_q ) )
        {
            if( doc_p->patientId == doc2_p->patientId )
            {
                duplicate = TRUE;
                break;
            }
        }

        if( duplicate )
        {
            mbLog( "Duplicate Patient ID: %ld\n", doc_p->patientId );
            thermometer_p->Gauge->Progress++;
            duplicateCount++;
            mbFree( doc_p );
        }
        else
        {
            qInsertLast( doc2_q, doc_p );
            mbLog( "New Patient ID: %ld\n", doc_p->patientId );
        }

    }

    mbLog( "Found %ld duplicates\n", duplicateCount );
    for( ; ; )
    {
        if( qEmpty( doc2_q ) ) break;
        doc_p = (pmcDocumentMaintStruct_p)qRemoveFirst( doc2_q );
        thermometer_p->Gauge->Progress++;

        //Int32s_t    pmcSqlPatientDetailsGet
        //(
        //    Int32u_t        id,
        //    Char_p          firstName_p,
        //    Char_p          lastName_p,
        //    Char_p          title_p,
        //    Char_p          phn_p,
        //    Char_p          phnProv_p,
        //    Int32u_p        dob_p,
        //    Int32u_p        gender_p
        //)

        pmcSqlPatientDetailsGet( doc_p->patientId, pat_p );

        mbLog( "%s, %s, %s, %s\n", pat_p->phn, pat_p->title, pat_p->lastName, pat_p->firstName  );
        count++;

        mbFree( doc_p );
    }

    mbDlgInfo( "Extracted %ld patient records.\n", count );

    delete thermometer_p;

    mbFree( buf1_p );
    mbFree( pat_p );
    return;
}
#endif

#if 0
//---------------------------------------------------------------------------
// Function: pmcReassignPatients()
//---------------------------------------------------------------------------
// Description:
//
// This function will examine all patient records for a particular
// provider and assign them to an alternate provider if that patient has
// appointments or claims with the alternate provider.
//---------------------------------------------------------------------------

void pmcReassignPatients( void )
{
    Char_p          buf_p = NIL;
    Char_p          buf2_p = NIL;
    Char_p          cmd_p = NIL;
    qHead_t         pat_qhead;
    qHead_p         pat_q;
    AnsiString      tempStr = "";
    patCheck_p      pat_p;
    Int32u_t        i;
    Int32u_t        updateCount = 0;
    Int32u_t        rowCount;
    FILE           *fp = NIL;
    Char_p          fileName_p = NIL;
    TDatabaseThermometer
                    *thermometer_p = NIL;
    Int32u_t        oldProvider;
    Int32u_t        newProvider;
    bool            updateFlag = FALSE;

    oldProvider = 2;
    newProvider = 1;

    mbMalloc( buf_p, 2048 );
    mbMalloc( buf2_p, 256 );
    mbMalloc( cmd_p, 256 );
    mbMalloc( fileName_p, 256 );

    pat_q = qInitialize( &pat_qhead );

    if( mbDlgOkCancel( "Proceed with patient reassignment?" ) == MB_BUTTON_CANCEL ) goto exit;

    // First, get all the patient from the original provider
    sprintf( buf_p, "select %s from %s where %s=%ld",
        PMC_SQL_PATIENTS_FIELD_ID,
        PMC_SQL_TABLE_PATIENTS,
        PMC_SQL_PATIENTS_FIELD_PROVIDER_ID, oldProvider );

    PMC_GENERAL_QUERY_SETUP( buf_p );
    while( !pmcDataSet_p->Eof )
    {
        mbCalloc( pat_p, sizeof( patCheck_t ) );
        qInsertFirst( pat_q, pat_p );

        // Get the patient ID
        pat_p->id = pmcDataSet_p->FieldValues[ PMC_SQL_FIELD_ID ];
        pmcDataSet_p->Next( );
    }
    PMC_GENERAL_QUERY_CLEANUP( );

    sprintf( buf_p, "Processing %ld patient records...", pat_q->size );
    thermometer_p = new TDatabaseThermometer( NULL, buf_p );
    thermometer_p->WindowState = wsNormal;
    thermometer_p->Gauge->MinValue = (Int32u_t)0;
    thermometer_p->Gauge->MaxValue = (Int32u_t)pat_q->size;

    for( i = 0 ; ; i++ )
    {
        if( qEmpty( pat_q ) ) break;
        pat_p = (patCheck_p)qRemoveFirst( pat_q );

        thermometer_p->Gauge->Progress++;

        updateFlag = FALSE;

        // First check appointments
        sprintf( cmd_p, "select %s from %s where %s=%ld and %s=%ld",
                PMC_SQL_APPS_FIELD_ID,
                PMC_SQL_TABLE_APPS,
                PMC_SQL_APPS_FIELD_PATIENT_ID, pat_p->id,
                PMC_SQL_APPS_FIELD_PROVIDER_ID, newProvider );

        PMC_GENERAL_QUERY_SETUP( cmd_p );
        rowCount = 0;
        while( !pmcDataSet_p->Eof )
        {
            rowCount++;
            pmcDataSet_p->Next( );
        }
        PMC_GENERAL_QUERY_CLEANUP( );

        if( rowCount )
        {
            updateFlag = TRUE;
            goto update;
        }

        // Next check claims
        sprintf( cmd_p, "select %s from %s where %s=%ld and %s=%ld",
                PMC_SQL_CLAIMS_FIELD_ID,
                PMC_SQL_TABLE_CLAIMS,
                PMC_SQL_CLAIMS_FIELD_PATIENT_ID, pat_p->id,
                PMC_SQL_CLAIMS_FIELD_PROVIDER_ID, newProvider );

        PMC_GENERAL_QUERY_SETUP( cmd_p );
        rowCount = 0;
        while( !pmcDataSet_p->Eof )
        {
            rowCount++;
            pmcDataSet_p->Next( );
        }
        PMC_GENERAL_QUERY_CLEANUP( );

        if( rowCount )
        {
            updateFlag = TRUE;
            goto update;
        }

        // Next check documents
        sprintf( cmd_p, "select %s from %s where %s=%ld and %s=%ld",
                PMC_SQL_FIELD_ID,
                PMC_SQL_TABLE_DOCUMENTS,
                PMC_SQL_DOCUMENTS_FIELD_PATIENT_ID, pat_p->id,
                PMC_SQL_DOCUMENTS_FIELD_PROVIDER_ID, newProvider );

        PMC_GENERAL_QUERY_SETUP( cmd_p );
        rowCount = 0;
        while( !pmcDataSet_p->Eof )
        {
            rowCount++;
            pmcDataSet_p->Next( );
        }
        PMC_GENERAL_QUERY_CLEANUP( );

        if( rowCount )
        {
            updateFlag = TRUE;
            goto update;
        }

update:
        if( updateFlag )
        {
            updateCount++;
            sprintf( cmd_p, "update %s set %s=%ld where %s=%ld",
                PMC_SQL_TABLE_PATIENTS,
                PMC_SQL_PATIENTS_FIELD_PROVIDER_ID, newProvider,
                PMC_SQL_FIELD_ID, pat_p->id );
            pmcSqlExec( cmd_p );
        }

        if( pat_p->firstName_p )    mbFree( pat_p->firstName_p );
        if( pat_p->lastName_p )     mbFree( pat_p->lastName_p );
        if( pat_p->title_p )        mbFree( pat_p->title_p );

        mbFree( pat_p );
    }

exit:

    if( fp ) fclose( fp );

    if( thermometer_p ) delete thermometer_p;

    if( buf_p )         mbFree( buf_p );
    if( buf2_p )        mbFree( buf2_p );
    if( cmd_p )         mbFree( cmd_p );
    if( fileName_p )    mbFree( fileName_p );

    mbDlgInfo( "Reassigned %ld patient records.\n", updateCount );

    return;
}
#endif

#if 0

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void pmcSmaCleanFirstName
(
    Char_p  nameIn_p,
    Char_p  firstNameOut_p,
    Char_p  middleNameOut_p
)
{
    Char_p  buf_p = NIL;
    Char_p  free_p = NIL;
    Ints_t  l, i;
    Char_p  temp_p;
    bool    foundSpace = FALSE;

    l = strlen( nameIn_p );
    mbMalloc( buf_p, l+1 );
    free_p = buf_p;

    if( firstNameOut_p )  sprintf( firstNameOut_p, "" );
    if( middleNameOut_p ) sprintf( middleNameOut_p, "" );

    // Clean string, and copy it into buf_p

    mbStrClean( nameIn_p, buf_p, TRUE );
    l = strlen( buf_p );
    for( temp_p = buf_p, i = 0 ; i < l ; i++, temp_p++ )
    {
        if( *temp_p == 0x20 )
        {
            *temp_p = 0;
            temp_p++;
            foundSpace = TRUE;
            break;
        }
    }

    // Write the first name
    if( firstNameOut_p )
    {
        mbStrClean( buf_p, firstNameOut_p, TRUE );
    }

    if( foundSpace )
    {
        if( middleNameOut_p )
        {
            mbStrClean( temp_p, middleNameOut_p, TRUE );
        }
    }

    nbDlgDebug(( "In '%s' first '%s' middle '%s'", nameIn_p, firstNameOut_p, middleNameOut_p ));

//exit:
    if( free_p != buf_p )
    {
        mbDlgDebug(( "What the??" ));
    }

    if( buf_p ) mbFree( buf_p );
}

#endif

#if 0
//---------------------------------------------------------------------------
// Function: pmcDatabaseCheckClaimRecords()
//---------------------------------------------------------------------------
// Description:
//
// This is a temporary function to extract all documents for a provider.
//---------------------------------------------------------------------------

#define EXTRACT_PROVIDER_ID 2
void pmcExtractDocuments( void )
{
    Char_p                      buf_p;
    Char_p                      buf2_p;
    pmcDocumentMaintStruct_p    document_p, document2_p;
    qHead_t                     documentQueue;
    qHead_p                     document_q;
    AnsiString                  tempStr = "";
    TDatabaseThermometer       *thermometer_p = NIL;
    bool                        duplicate;
    Int32u_t                    deleteFlag = FALSE;

    mbMalloc( buf_p, 512 );
    mbMalloc( buf2_p, 512 );

    document_q = qInitialize( &documentQueue );

    // Read list of documents from the database
    sprintf( buf_p, "select %s,%s,%s from %s where %s=%d",
        PMC_SQL_DOCUMENTS_FIELD_NAME,
        PMC_SQL_DOCUMENTS_FIELD_ORIG_NAME,
        PMC_SQL_FIELD_ID,
        PMC_SQL_TABLE_DOCUMENTS,
        PMC_SQL_DOCUMENTS_FIELD_PROVIDER_ID, EXTRACT_PROVIDER_ID );

    PMC_GENERAL_QUERY_SETUP( buf_p );
    while( !pmcDataSet_p->Eof )
    {
        mbCalloc( document_p, sizeof( pmcDocumentMaintStruct_t ) );


        tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_DOCUMENTS_FIELD_NAME ] );
        strncpy( document_p->name, tempStr.c_str( ), 127 );

        tempStr = VarToStr( pmcDataSet_p->FieldValues[ PMC_SQL_DOCUMENTS_FIELD_ORIG_NAME ] );
        strncpy( document_p->origName, tempStr.c_str( ), 127 );

        document_p->id = pmcDataSet_p->FieldValues[ PMC_SQL_FIELD_ID ];

        // Do a quick check for duplicate names
        for( ; ; )
        {
            duplicate = FALSE;
            for( document2_p = (pmcDocumentMaintStruct_p)qInit( document_q ) ;
                                                         qDone( document_q ) ;
                document2_p = (pmcDocumentMaintStruct_p)qNext( document_q ) )
            {
                strcpy( buf_p,  document_p->origName );
                strcpy( buf2_p, document2_p->origName );

                pmcUpperCase( buf_p );
                pmcUpperCase( buf2_p );

                if( strcmp( buf_p, buf2_p ) == 0 )
                {
                    mbDlgExclaim( "Duplicate document name '%s' detected\n", document_p->origName );
                    duplicate = TRUE;
                    strcat( document_p->origName, ".1" );
                }
            }
            if( duplicate == FALSE ) break;
        }

        qInsertLast( document_q, document_p );
        pmcDataSet_p->Next( );
    }
    PMC_GENERAL_QUERY_CLEANUP( );

    // Next process the documents
    sprintf( buf_p, "Extracting %ld documents for provider %d...", document_q->size, EXTRACT_PROVIDER_ID );
    thermometer_p = new TDatabaseThermometer( NULL, buf_p );
    thermometer_p->WindowState = wsNormal;
    thermometer_p->Gauge->MinValue  = (Int32u_t)0;
    thermometer_p->Gauge->MaxValue  = (Int32u_t)document_q->size;

    deleteFlag = mbDlgOkCancel( "Delete extracted documents from database?\n" );

    for( ; ; )
    {
        if( qEmpty( document_q ) ) break;
        thermometer_p->Gauge->Progress++;

        document_p = (pmcDocumentMaintStruct_p)qRemoveFirst( document_q );

        sprintf( buf_p, "%s\\%s", pmcCfg[CFG_DOC_IMPORT_TO_DIR].str_p, document_p->name );
        sprintf( buf2_p, "%s\\%s", pmcCfg[CFG_DOC_EXTRACT_TO_DIR].str_p, document_p->origName );

        if( pmcFileCopy( buf_p, buf2_p ) )
        {
            mbLog( "Extract #%d %s -> %s\n", thermometer_p->Gauge->Progress, buf_p, buf2_p );
            if( deleteFlag == PMC_OK_BUTTON )
            {
                mbLog( "Deleting document %s id %d\n", buf_p, document_p->id );
                unlink( buf_p );
                pmcSqlRecordDelete( PMC_SQL_TABLE_DOCUMENTS, document_p->id );
            }
        }
        else
        {
            mbDlgExclaim( "Error extracting document %s to %s\n", buf_p, buf2_p );
        }

        mbFree( document_p );
    }

    mbDlgInfo( "Document extraction complete.\n" );
    delete thermometer_p;
    mbFree( buf2_p );
    mbFree( buf_p );
}

#endif


