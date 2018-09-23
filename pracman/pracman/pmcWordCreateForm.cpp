//---------------------------------------------------------------------------
// File:    pmcWordCreateForm.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    August 26, 2001
//---------------------------------------------------------------------------
// Description:
//
// This form allows the user to select a patient, an appointment, and
// a doctor.  The resulting information in placed into a Word "merge" file
// which can be used to automatically generate a Word document.
//---------------------------------------------------------------------------

// Platform include files
#include <stdio.h>
#include <stdlib.h>
#include <vcl.h>
#include <dos.h>
#include <dir.h>
#include <process.h>
#pragma hdrstop

// Library include files
#include "mbUtils.h"

// Project include files
#include "pmcGlobals.h"
#include "pmcUtils.h"
#include "pmcTables.h"
#include "pmcWordCreateForm.h"
#include "pmcPatientListForm.h"
#include "pmcPatientEditForm.h"
#include "pmcDoctorListForm.h"
#include "pmcDoctorEditForm.h"
#include "pmcDateSelectForm.h"
#include "pmcDocumentListForm.h"
#include "pmcDocument.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------
__fastcall TWordCreateForm::TWordCreateForm(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------
__fastcall TWordCreateForm::TWordCreateForm
(
    TComponent                  *Owner,
    pmcWordCreateFormInfo_p     formInfo_p
)
    : TForm(Owner)
{
    mbFileListStruct_p      file_p;
    Char_p                  buf_p;

    mbMalloc( CreateDir_p, 256 );
    mbMalloc( buf_p, 256 );

    NullString[0] = 0;

    strcpy( CreateDir_p, pmcCfg[CFG_WORD_CREATE_DIR].str_p );
    CreateDirectoryEdit->Text = CreateDir_p;

    DescriptionEdit->Text = "";

    FormInfo_p = formInfo_p;
    FormInfo_p->doctorId = 0;

    // Today's date by default
    Date = mbToday( );
    UpdateDate( Date );

    // Initialize queue of files
    File_q = qInitialize( &FileListHead );

    // Get list of files in the word template director
    mbFileListGet( pmcCfg[CFG_WORD_TEMPLATE_DIR].str_p,
                   pmcDocumentTypeFilterStrings[PMC_DOCUMENT_TYPE_WORD],
                   File_q, FALSE );

    TemplateComboBox->Text = "";
    TemplateComboBox->Items->Clear();

    qWalk( file_p, File_q, mbFileListStruct_p )
    {
        TemplateComboBox->Items->Add( file_p->name_p );
    }

    // Initialize the provider information
    FormInfo_p->providerId = pmcProviderListBuild( ProviderComboBox, FormInfo_p->providerId, FALSE, TRUE );
//    ProviderComboBox->ItemIndex =  pmcProviderIndexGet( ProviderComboBox, FormInfo_p->providerId );

    UpdatePatient( FormInfo_p->patientId );

    SelectedAppointId = 0;
    UpdateAppointment( SelectedAppointId );

    if( FormInfo_p->doctorId == 0 && FormInfo_p->patientId != 0 )
    {
        // If called with a patient, and the doctor id is 0, then fill
        // in the doctor with the patients default doctor
        sprintf( buf_p, "select %s from %s where %s=%ld",
            PMC_SQL_PATIENTS_FIELD_REFERRING_DR_ID,
            PMC_SQL_TABLE_PATIENTS,
            PMC_SQL_FIELD_ID,
            FormInfo_p->patientId );

        FormInfo_p->doctorId = pmcSqlSelectInt( buf_p, NIL );
    }
    UpdateDoctor( FormInfo_p->doctorId );

    mbFree( buf_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------
void __fastcall TWordCreateForm::CancelButtonClick(TObject *Sender)
{
    FormInfo_p->returnCode = MB_BUTTON_CANCEL;
    Close( );
}
//---------------------------------------------------------------------------
// Function: OkButtonClick
//---------------------------------------------------------------------------
// Create the specified document.  Also create an entry in the list of
// created documents that will be referenced later when the document is
// imported.
//
//---------------------------------------------------------------------------
void __fastcall TWordCreateForm::OkButtonClick(TObject *Sender)
{
    Boolean_t   close = FALSE;                                                                          
    Char_p      buf1_p;
    Char_p      buf2_p;
    Char_p      cmd_p;
    Char_p      fileFull_p;
    Char_p      fileName_p;
    Int32u_t    id;

#define PMC_WORD_MACRO "pmc_macro1"

    // Store the updated create directory.  This will change the default
    // create directory for the remainder of this session.
    if( pmcCfg[CFG_WORD_CREATE_DIR].str_p  ) mbFree( pmcCfg[CFG_WORD_CREATE_DIR].str_p );
    mbMalloc( pmcCfg[CFG_WORD_CREATE_DIR].str_p, strlen( CreateDir_p ) + 1 );

    strcpy( pmcCfg[CFG_WORD_CREATE_DIR].str_p, CreateDir_p );

    FormInfo_p->returnCode = MB_BUTTON_OK;

    mbMalloc( buf1_p,       1024 );
    mbMalloc( buf2_p,       1024 );
    mbMalloc( cmd_p,        1024 );
    mbMalloc( fileFull_p,   1024 );
    mbMalloc( fileName_p,   128 );

    // Ensure a template is selected
    if( strlen( TemplateComboBox->Text.c_str() ) == 0 )
    {
        mbDlgInfo( "No document template selected.\n" );
        goto exit;
    }

    // Create new target file name
    *buf1_p = 0;
    if( strlen( PatLastName ) )
    {
        sprintf( buf2_p, "%s-", PatLastName );
        strcat( buf1_p, buf2_p );
    }
    else
    {
        // Have no patient last name, use Dr last name if available
        if( strlen( DrLastName ) )
        {
            sprintf( buf2_p, "%s-", DrLastName );
            strcat( buf1_p, buf2_p );
        }
    }

    // Add template name
    strcat( buf1_p,  TemplateComboBox->Text.c_str() );

    {
        // Get rid of trailing .doc from template name
        Int32s_t len = strlen( buf1_p );
        Int32s_t pos = mbStrPosIgnoreCase( buf1_p, ".doc" );
        if( pos )
        {
            if( len - pos == 4 ) *( buf1_p + pos ) = 0;
        }
    }

    // Create new database record and get ID
    if( ( id = pmcSqlRecordCreate( PMC_SQL_TABLE_DOCUMENTS, NIL ) ) == 0 )
    {
        mbDlgExclaim( "Error creating database record." );
        goto exit;
    }

    // Create final version of target name by adding ID
    sprintf( fileName_p, "%s-%lu.doc", buf1_p, id );

    // Create qualified target file name
    sprintf( buf2_p, "%s\\%s", pmcCfg[CFG_WORD_CREATE_DIR].str_p, fileName_p );
    mbStrClean( buf2_p, fileFull_p, FALSE );

    // Create qualified source file name
    sprintf( buf1_p, "%s\\%s", pmcCfg[CFG_WORD_TEMPLATE_DIR].str_p, TemplateComboBox->Text.c_str() );

    // Now attempt to create the new file
    if( mbFileCopy( buf1_p, fileFull_p ) != MB_RET_OK )
    {
        mbDlgExclaim( "Could not create new Word document.\n" );
        goto exit;
    }

    // Add the created document to the list of created documents.  This list
    // will be used to delete created documents that were not modified in
    // any way.
    {
        Int32u_t            fileSize;
        mbFileListStruct_p  document_p;

        mbCalloc( document_p, sizeof( mbFileListStruct_t ) );
        mbMallocStr( document_p->name_p, fileFull_p );
        document_p->crc = mbCrcFile( document_p->name_p, &fileSize, NIL, NIL, NIL, NIL, NIL );
        document_p->size64 = (Int64u_t)fileSize;
        qInsertLast( pmcCreatedDocument_q, document_p );
    }

    // If no description entered, set it to the document template
    mbStrClean( DescriptionEdit->Text.c_str(), buf2_p, TRUE );
    if( strlen( buf2_p ) == 0 )
    {
        int len;
        mbStrClean( TemplateComboBox->Text.c_str(), buf2_p, TRUE );
        // Get rid if the .doc at the end
        len = strlen( buf2_p );
        if( len > 4 ) *( buf2_p + len - 4 ) = 0;
    }

    // Update the  newly created database record
    sprintf( cmd_p,  "update %s set %s=\"%s\","     // document name
                                   "%s=\"%s\","     // description
                                   "%s=%ld,"        // date
                                   "%s=%ld,"        // patient id
                                   "%s=%ld,"        // doctor id
                                   "%s=%ld,"        // provider id
                                   "%s=%ld,"        // not deleted
                                   "%s=%ld,"        // type - WORD
                                   "%s=%ld "        //
                                   "where %s=%ld",
        PMC_SQL_TABLE_DOCUMENTS,
        PMC_SQL_DOCUMENTS_FIELD_ORIG_NAME,      fileName_p,
        PMC_SQL_FIELD_DESC,                     buf2_p,
        PMC_SQL_FIELD_DATE,                     Date,
        PMC_SQL_FIELD_PATIENT_ID,               FormInfo_p->patientId,
        PMC_SQL_DOCUMENTS_FIELD_DOCTOR_ID,      FormInfo_p->doctorId,
        PMC_SQL_FIELD_PROVIDER_ID,              FormInfo_p->providerId,
        PMC_SQL_FIELD_NOT_DELETED,              PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE,
        PMC_SQL_FIELD_TYPE,                     PMC_DOCUMENT_TYPE_WORD,
//        PMC_SQL_DOCUMENTS_FIELD_STATUS,         PMC_DOCUMENT_STATUS_NEW,
        PMC_SQL_DOCUMENTS_FIELD_STATUS,         PMC_DOCUMENT_STATUS_ACTIVE,
        PMC_SQL_FIELD_ID,                       id );

    // Update the database
    pmcSqlExec( cmd_p );

    // Log the new document
    mbLog( "Created new word document '%s' (id: %lu)\n", fileName_p, id );

    // Must delete any existing merge file
    sprintf( buf1_p, "%s\\%s", pmcCfg[CFG_WORD_CREATE_DIR].str_p, PMC_WORD_CREATE_MERGE_FILE );
    unlink( buf1_p );

    // Create a merge file for the new document
    CreateMergeFile( buf1_p );

    // Change to the working directory
    chdir( pmcCfg[CFG_WORD_CREATE_DIR].str_p );

    // Add macro switch to word command
    sprintf( buf1_p, "/m%s", PMC_WORD_MACRO );

    // Put quotes around new file name
    sprintf( buf2_p, "\"%s\"", fileFull_p );

    // Execute word
    spawnle( P_NOWAITO,
             pmcCfg[CFG_WORD].str_p,
             pmcCfg[CFG_WORD].str_p,
             buf1_p,
             buf2_p,
             NULL, NULL );

    close = TRUE;

    chdir( pmcHomeDir );

exit:
    mbFree( buf1_p );
    mbFree( buf2_p );
    mbFree( fileFull_p );
    mbFree( fileName_p );
    mbFree( cmd_p );

    if( close ) Close( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------
void __fastcall TWordCreateForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
    mbFileListFree( File_q );

    mbFree( CreateDir_p );

    Action = caFree;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------
void __fastcall TWordCreateForm::PatientListButtonClick(TObject *Sender)
{
    PatientSelect( 0 );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
// Create the "merge file" for document creation.
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::CreateMergeFile
(
    Char_p  mergeFileName_p
)
{
    MbDateTime  dateTime;
    Char_p      buf_p, buf1_p;
    FILE       *fp = NIL;

    Char_p  fields[] =
    {
         "DATE"             // 0
        ,"PAT_LAST"         // 1
        ,"PAT_FIRST"        // 2
        ,"PAT_PHN"          // 3
        ,"PAT_DOB"          // 4
        ,"PAT_TITLE"        // 5
        ,"PAT_ADD1"         // 6
        ,"PAT_ADD2"         // 7
        ,"PAT_ADD3"         // 8
        ,"PAT_PHONE"        // 9
        ,"PAT_HESHE"        // 10
        ,"PAT_HISHER"       // 11
        ,"APP_DATE"         // 12
        ,"APP_TIME"         // 13
        ,"APP_PROVIDER"     // 14
        ,"DR_LAST"          // 15
        ,"DR_FIRST"         // 16
        ,"DR_ADD1"          // 17
        ,"DR_ADD2"          // 18
        ,"DR_ADD3"          // 19
        ,"DR_FAX"           // 20
        ,"DR_PHONE"         // 21
        ,PMC_INVALID_STRING
    };
    Ints_t  i, len;

    mbMalloc( buf_p, 2048 );
    mbMalloc( buf1_p, 128 );

    fp = fopen( mergeFileName_p, "w" );
    if( fp == NIL ) goto exit;

    // First, output all of the fields into the merge file
    *buf_p = 0;
    for( i = 0 ; ; i++ )
    {
        if( strcmp( fields[i], PMC_INVALID_STRING ) == 0 ) break;

        sprintf( buf1_p, "\"%s\",", fields[i] );
        strcat( buf_p, buf1_p );
    }

    // Get rid of the last comma
    len = strlen( buf_p );
    *( buf_p + len - 1 ) = 0;
    fprintf( fp, "%s\n", buf_p );

    // Output the date
    *buf_p = 0;
    dateTime.SetDate( Date );
    AddField( buf_p, dateTime.MDY_DateString( ), TRUE ); // 0

    // Add the patient details
    AddField( buf_p, PatLastName,       TRUE );     // 1
    AddField( buf_p, PatFirstName,      TRUE );     // 2
    AddField( buf_p, PhnFormat,         TRUE );     // 3

    // Add the patient date of birth
    if( DateOfBirth )                               // 4
    {
        dateTime.SetDate( DateOfBirth );
        AddField( buf_p, dateTime.MDY_DateString( ), TRUE );
    }
    else
    {
        AddField( buf_p, "", TRUE );
    }

    AddField( buf_p, PatTitle,          TRUE );     // 5
    AddField( buf_p, PatAddress_p[0],   TRUE );     // 6
    AddField( buf_p, PatAddress_p[1],   TRUE );     // 7
    AddField( buf_p, PatAddress_p[2],   TRUE );     // 8
    AddField( buf_p, PatPhone,          TRUE );     // 9

    // He/She field
    sprintf( buf1_p, "%s", ( PatGender == PMC_SQL_GENDER_MALE ) ? "he" : "she" );
    AddField( buf_p, buf1_p,            TRUE );     // 10

    // His/Her field
    sprintf( buf1_p, "%s", ( PatGender == PMC_SQL_GENDER_MALE ) ? "his" : "her" );
    AddField( buf_p, buf1_p,            TRUE );     // 11

    // App date field
    sprintf( buf1_p, "__________" );
    if( AppDate )                                   // 12
    {
        dateTime.SetDate( AppDate );
        AddField( buf_p, dateTime.MDY_DateString( ), TRUE );
    }
    else
    {
        AddField( buf_p, buf1_p,        TRUE );
    }

    // App time field
    sprintf( buf1_p, "__________" );
    if( AppTime )                                   // 13
    {
        dateTime.SetTime( AppTime );
        AddField( buf_p, dateTime.HM_TimeString( ), TRUE );
    }
    else
    {
        AddField( buf_p, buf1_p,            TRUE );
    }

    if( strlen( AppProvider ) == 0 )
    {
        sprintf( buf1_p, "__________" );
    }
    else
    {
        strcpy( buf1_p, AppProvider );
    }
    AddField( buf_p, buf1_p,            TRUE );     // 14

    AddField( buf_p, DrLastName,        TRUE );     // 15
    AddField( buf_p, DrFirstName,       TRUE );     // 16
    AddField( buf_p, DrAddress_p[0],    TRUE );     // 17
    AddField( buf_p, DrAddress_p[1],    TRUE );     // 18
    AddField( buf_p, DrAddress_p[2],    TRUE );     // 19
    AddField( buf_p, DrFax,             TRUE );     // 20
    AddField( buf_p, DrPhone,           FALSE );    // 21

    fprintf( fp, "%s\n", buf_p );

exit:

    if( fp ) fclose( fp );

    mbFree( buf_p );
    mbFree( buf1_p );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::AddField
(
    Char_p      buf_p,
    Char_p      field_p,
    Int32u_t    commaFlag
)
{
    Char_p      buf1_p, buf2_p;

    mbMalloc( buf1_p, 256 );
    mbMalloc( buf2_p, 256 );

    mbStrClean( field_p, buf2_p, TRUE );
    sprintf( buf1_p, "\"%s\"", buf2_p );
    strcat( buf_p, buf1_p );
    if( commaFlag ) strcat( buf_p, "," );

    mbFree( buf1_p );
    mbFree( buf2_p );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::UpdateAppointment
(
    Int32u_t    appointId
)
{
    Char_p      buf1_p;
    Char_p      buf2_p;
    Char_p      buf3_p;
    Ints_t      i;
    MbDateTime  dateTime;
    MbSQL       sql;

    mbMalloc( buf1_p,      1024 );
    mbMalloc( buf2_p,      256 );
    mbMalloc( buf3_p,      256 );

    *buf1_p  = 0;
    *buf2_p  = 0;
    *buf3_p  = 0;

    AppDate = 0;
    AppTime = 0;
    AppProvider[0] = 0;

    if( appointId == 0 ) goto exit;

    // Format SQL command
    sprintf( buf1_p, "select %s.%s,%s.%s,%s.%s from %s,%s where "
                     "%s.%s=%s.%s and %s.%s=%ld",

        PMC_SQL_TABLE_APPS,         PMC_SQL_FIELD_DATE,                     // 0
        PMC_SQL_TABLE_APPS,         PMC_SQL_APPS_FIELD_START_TIME,          // 1
        PMC_SQL_TABLE_PROVIDERS,    PMC_SQL_FIELD_DESC,                     // 2

        PMC_SQL_TABLE_APPS,
        PMC_SQL_TABLE_PROVIDERS,

        PMC_SQL_TABLE_PROVIDERS,    PMC_SQL_FIELD_ID,
        PMC_SQL_TABLE_APPS,         PMC_SQL_FIELD_PROVIDER_ID,

        PMC_SQL_TABLE_APPS,         PMC_SQL_FIELD_ID, appointId );

    i=0;

    sql.Query( buf1_p );

    while( sql.RowGet( ) )
    {
        AppDate = sql.Int32u( 0 );
        AppTime = sql.Int32u( 1 );
        strcpy( buf3_p, sql.String( 2 ) );
        i++;
    }

    // Sanity Check
    if( i != 1 )
    {
       mbDlgDebug(( "Error locating appointment id %ld in database (records: %d)", appointId, i ));
    }

    dateTime.SetDateTime( AppDate, AppTime );
    sprintf( buf1_p, dateTime.MDY_DateString( ) );
    sprintf( buf2_p, dateTime.HM_TimeString( ) );
    strcpy( AppProvider, buf3_p );

exit:

    AppDateLabel->Caption = buf1_p;
    AppTimeLabel->Caption = buf2_p;
    AppProviderLabel->Caption = buf3_p;

    mbFree( buf1_p );
    mbFree( buf2_p );
    mbFree( buf3_p );

    return;
}

//---------------------------------------------------------------------------
// TODO: It looks like this whole ugly function cound get replaced with a
// patient object!!!!!!
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::UpdatePatient
(
    Int32u_t    patientId
)
{
    Char_p      buf_p;
    Char_p      buf2_p;
    Char_p      buf3_p;
    Ints_t      i;
    MbDateTime  dateTime;
    Boolean_t   localFlag;
    Int32u_t    deceasedDate = 0;
    MbSQL       sql;

    mbMalloc( buf_p,      1024 );
    mbMalloc( buf2_p,      256 );
    mbMalloc( buf3_p,      256 );

    *buf_p   = 0;
    *buf2_p  = 0;
    *buf3_p  = 0;

    // First, zero out all the fields
    PatFirstName[0]  = 0;
    PatLastName[0]   = 0;
    PatTitle[0]      = 0;
    Phn[0]           = 0;
    PhnProv[0]       = 0;
    PhnFormat[0]     = 0;
    DateOfBirth      = 0;
    PatCity[0]       = 0;
    PatProvince[0]   = 0;
    PatPostalCode[0] = 0;
    PatAddress1[0]   = 0;
    PatAddress2[0]   = 0;
    PatAddress3[0]   = 0;
    PatPhone[0]      = 0;

    PatAddress_p[0] = NullString;
    PatAddress_p[1] = NullString;
    PatAddress_p[2] = NullString;

    PatGender = 0;

    PatNameLabel->Caption = "";
    PatAddr1Label->Caption = "";
    PatAddr2Label->Caption = "";
    PatAddr3Label->Caption = "";
    PatPhoneLabel->Caption = "";
    PhnLabel->Caption = "";
    BirthDateLabel->Caption = "";
    PatientEdit->Text = "";

    if( patientId == 0 ) goto exit;

    // Format SQL command    0  1  2  3  4  5  6  7  8  9 10 11 12
    sprintf( buf_p, "select %s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s from %s where %s=%ld",
        PMC_SQL_FIELD_LAST_NAME,                // 0
        PMC_SQL_FIELD_FIRST_NAME,               // 1
        PMC_SQL_FIELD_CITY,                     // 3
        PMC_SQL_FIELD_PROVINCE,                 // 2
        PMC_SQL_FIELD_POSTAL_CODE,              // 4
        PMC_SQL_FIELD_ADDRESS1,                 // 5
        PMC_SQL_FIELD_ADDRESS2,                 // 6
        PMC_SQL_PATIENTS_FIELD_HOME_PHONE,      // 7
        PMC_SQL_PATIENTS_FIELD_DATE_OF_BIRTH,   // 8
        PMC_SQL_PATIENTS_FIELD_PHN,             // 9
        PMC_SQL_PATIENTS_FIELD_PHN_PROVINCE,    // 10
        PMC_SQL_FIELD_TITLE,                    // 11
        PMC_SQL_PATIENTS_FIELD_GENDER,          // 12
        PMC_SQL_PATIENTS_FIELD_DATE_OF_DEATH,   // 13

        PMC_SQL_TABLE_PATIENTS,
        PMC_SQL_FIELD_ID, patientId );

    i=0;

    sql.Query( buf_p );
    while( sql.RowGet( ) )
    {
        strcpy( PatLastName, sql.String( 0 ) );
        strcpy( PatFirstName, sql.String( 1 ) );
        strcpy( PatCity, sql.String( 2 ) );
        strcpy( PatProvince, sql.String( 3 ) );
        strcpy( PatPostalCode, sql.String( 4 ) );
        strcpy( PatAddress1, sql.String( 5 ) );
        strcpy( PatAddress2, sql.String( 6 ) );
        strcpy( PatPhone, sql.String( 7 ) );
        DateOfBirth = sql.Int32u( 8 );
        strcpy( Phn, sql.String( 9 ) );
        strcpy( PhnProv, sql.String( 10 ) );
        strcpy( PatTitle, sql.String( 11 ) );
        PatGender =  sql.Int32u( 12 );
        deceasedDate = sql.Int32u( 13 );
        i++;
    }

    // Sanity Check
    if( i != 1 )
    {
       mbDlgDebug(( "Error locating patient id %ld in database (records: %d)", patientId, i ));
    }

    *buf_p = 0;
    if( strlen( PatTitle     ) ) { sprintf( buf2_p, "%s ", PatTitle );     strcat( buf_p, buf2_p ); }
    if( strlen( PatFirstName ) ) { sprintf( buf2_p, "%s ", PatFirstName ); strcat( buf_p, buf2_p ); }
    if( strlen( PatLastName  ) ) { sprintf( buf2_p, "%s",  PatLastName );  strcat( buf_p, buf2_p ); }
    PatientEdit->Text = buf_p;
    PatNameLabel->Caption = buf_p;

    *buf_p = 0;
    if( strlen( PatCity )       ) { sprintf( buf2_p, "%s, ", PatCity );       strcat( buf_p, buf2_p ); }
    if( strlen( PatProvince )   ) { sprintf( buf2_p, "%s  ", PatProvince );   strcat( buf_p, buf2_p ); }
    if( strlen( PatPostalCode ) ) { sprintf( buf2_p, "%s",   PatPostalCode ); strcat( buf_p, buf2_p ); }

    strcpy( PatAddress3, buf_p );

    i = 0;
    if( strlen( PatAddress1 ) ) PatAddress_p[i++] = PatAddress1;
    if( strlen( PatAddress2 ) ) PatAddress_p[i++] = PatAddress2;
    if( strlen( PatAddress3 ) ) PatAddress_p[i++] = PatAddress3;

    PatAddr1Label->Caption = PatAddress_p[0];
    PatAddr2Label->Caption = PatAddress_p[1];
    PatAddr3Label->Caption = PatAddress_p[2];

    pmcFormatPhnDisplay( Phn, PhnProv, PhnFormat );
    PhnLabel->Caption = PhnFormat;

    if( DateOfBirth )
    {
        dateTime.SetDate( DateOfBirth );
        BirthDateLabel->Caption = dateTime.MDY_DateString( );
    }
    else
    {
        BirthDateLabel->Caption = "";
    }

    pmcPhoneFormat( PatPhone, buf2_p, buf3_p, &localFlag );
    if( strlen( PatPhone ) )
    {
        if( localFlag )
        {
            sprintf( PatPhone, "%s", buf3_p );
        }
        else
        {
            sprintf( PatPhone, "(%s) %s", buf2_p, buf3_p );
        }
    }
    PatPhoneLabel->Caption = PatPhone;

    if( deceasedDate > 0 )
    {
        mbDlgExclaim( "This patient has been marked as deceased." );
    }

exit:
    mbFree( buf_p );
    mbFree( buf2_p );
    mbFree( buf3_p );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::UpdateDoctor
(
    Int32u_t    doctorId
)
{
    Char_p          buf_p;
    Char_p          buf2_p;
    Char_p          buf3_p;
    Ints_t          i;
    Boolean_t       localFlag;
    MbSQL           sql;

    mbMalloc( buf_p, 2048 );
    mbMalloc( buf2_p, 256 );
    mbMalloc( buf3_p, 256 );

    DrAddress_p[0] = NullString;
    DrAddress_p[1] = NullString;
    DrAddress_p[2] = NullString;

    DrFirstName[0]  = 0;
    DrLastName[0]   = 0;
    DrCity[0]       = 0;
    DrProvince[0]   = 0;
    DrPostalCode[0] = 0;
    DrAddress1[0]   = 0;
    DrAddress2[0]   = 0;
    DrAddress3[0]   = 0;
    DrFax[0]        = 0;
    DrPhone[0]      = 0;

    DrNameLabel->Caption = "";
    DrAddr1Label->Caption = "";
    DrAddr2Label->Caption = "";
    DrAddr3Label->Caption = "";
    DrPhoneLabel->Caption = "";
    DrFaxLabel->Caption = "";
    DoctorEdit->Text = "";

    if( doctorId == 0 ) goto exit;

    // Format SQL command    0  1  2  3  4  5  6  7  8
    sprintf( buf_p, "select %s,%s,%s,%s,%s,%s,%s,%s,%s from %s where %s=%ld",
        PMC_SQL_FIELD_LAST_NAME,                // 0
        PMC_SQL_FIELD_FIRST_NAME,               // 1
        PMC_SQL_FIELD_CITY,                     // 3
        PMC_SQL_FIELD_PROVINCE,                 // 2
        PMC_SQL_FIELD_POSTAL_CODE,              // 4
        PMC_SQL_FIELD_ADDRESS1,                 // 5
        PMC_SQL_FIELD_ADDRESS2,                 // 6
        PMC_SQL_DOCTORS_FIELD_WORK_FAX,         // 7
        PMC_SQL_DOCTORS_FIELD_WORK_PHONE,       // 8

        PMC_SQL_TABLE_DOCTORS,
        PMC_SQL_FIELD_ID, doctorId );

    i=0;

    sql.Query( buf_p );
    while( sql.RowGet( ) )
    {
        strcpy( DrLastName,     sql.String( 0 ) );
        strcpy( DrFirstName,    sql.String( 1 ) );
        strcpy( DrCity,         sql.String( 2 ) );
        strcpy( DrProvince,     sql.String( 3 ) );
        strcpy( DrPostalCode,   sql.String( 4 ) );
        strcpy( DrAddress1,     sql.String( 5 ) );
        strcpy( DrAddress2,     sql.String( 6 ) );
        strcpy( DrFax,          sql.String( 7 ) );
        strcpy( DrPhone,        sql.String( 8 ) );
        i++;
    }

    // Sanity Check
    if( i != 1 )
    {
        mbDlgDebug(( "Error locating doctor id %ld in database (records: %d)", doctorId, i ));
    }

    *buf_p = 0;
    sprintf( buf_p, "Dr. " );
    if( strlen( DrFirstName ) ) { sprintf( buf2_p, "%s ", DrFirstName ); strcat( buf_p, buf2_p ); }
    if( strlen( DrLastName  ) ) { sprintf( buf2_p, "%s",  DrLastName );  strcat( buf_p, buf2_p ); }
    DoctorEdit->Text = buf_p;
    DrNameLabel->Caption = buf_p;

    *buf_p = 0;
    if( strlen( DrCity )       ) { sprintf( buf2_p, "%s, ", DrCity );       strcat( buf_p, buf2_p ); }
    if( strlen( DrProvince )   ) { sprintf( buf2_p, "%s  ", DrProvince );   strcat( buf_p, buf2_p ); }
    if( strlen( DrPostalCode ) ) { sprintf( buf2_p, "%s",   DrPostalCode ); strcat( buf_p, buf2_p ); }

    strcpy( DrAddress3, buf_p );

    i = 0;
    if( strlen( DrAddress1 ) ) DrAddress_p[i++] = DrAddress1;
    if( strlen( DrAddress2 ) ) DrAddress_p[i++] = DrAddress2;
    if( strlen( DrAddress3 ) ) DrAddress_p[i++] = DrAddress3;

    DrAddr1Label->Caption = DrAddress_p[0];
    DrAddr2Label->Caption = DrAddress_p[1];
    DrAddr3Label->Caption = DrAddress_p[2];

    pmcPhoneFormat( DrFax, buf2_p, buf3_p, &localFlag );

    if( strlen( DrFax ) )
    {
        if( localFlag )
        {
            sprintf( DrFax, "%s", buf3_p );
        }
        else
        {
            sprintf( DrFax, "(%s) %s", buf2_p, buf3_p );
        }
    }
    DrFaxLabel->Caption = DrFax;

    if( strlen( DrPhone ) )
    {
        pmcPhoneFormat( DrPhone, buf2_p, buf3_p, &localFlag );
        if( localFlag )
        {
            sprintf( DrPhone, "%s", buf3_p );
        }
        else
        {
            sprintf( DrPhone, "(%s) %s", buf2_p, buf3_p );
        }
    }
    DrPhoneLabel->Caption = DrPhone;

exit:
    mbFree( buf2_p );
    mbFree( buf3_p );
    mbFree( buf_p );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
// This function can be called externally to launch this form
//---------------------------------------------------------------------------

Int32s_t pmcWordCreate( Int32u_t patientId, Int32u_t providerId )
{
    pmcWordCreateFormInfo_t     info;
    TWordCreateForm            *form_p;

    if( pmcCfg[CFG_WORD].init == FALSE )
    {
        mbDlgInfo( "Location of Word not specified in %s initialization file.\n", PMC_NAME );
    }
    else
    {
        info.patientId = patientId;
        info.providerId = providerId;
        form_p = new TWordCreateForm( NULL, &info );
        form_p->ShowModal( );
        delete form_p;
    }
    return TRUE;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::CreateDirectoryButtonClick( TObject *Sender )
{
   UpdateDirectory( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::UpdateDirectory( void  )
{
    Char_p                  newDir_p;
    Void_p                  handle_p;
    TCursor                 origCursor;

    mbMalloc( newDir_p, MAX_PATH );

    origCursor = Screen->Cursor;
    Screen->Cursor = crHourGlass;

    handle_p = mbPathHandleGet( CreateDir_p );

    if( mbDlgBrowseFolderNew( newDir_p, "Choose Create Document Folder",
                              &handle_p ) == MB_BUTTON_OK )
    {
        strcpy( CreateDir_p, newDir_p );
        CreateDirectoryEdit->Text = CreateDir_p;
    }

    mbPathHandleFree( handle_p );
    Screen->Cursor = origCursor;
    mbFree( newDir_p );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::CreateDirectoryEditMouseDown
(
      TObject *Sender, TMouseButton Button, TShiftState Shift, int X,
      int Y)
{
    UpdateDirectory( );
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::DateSelectButtonClick(TObject *Sender)
{
    TDateSelectForm    *dateSelectForm_p;
    pmcDateSelectInfo_t dateSelectInfo;

    dateSelectInfo.mode      = PMC_DATE_SELECT_PARMS;
    dateSelectInfo.string_p  = NIL;
    dateSelectInfo.dateIn    = Date;
    dateSelectInfo.caption_p = NIL;

    dateSelectForm_p = new TDateSelectForm( NULL, &dateSelectInfo );
    dateSelectForm_p->ShowModal( );
    delete dateSelectForm_p;

    if( dateSelectInfo.returnCode == MB_BUTTON_OK )
    {
        Date = dateSelectInfo.dateOut;
        UpdateDate( Date );
    }
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::DoctorListButtonClick(TObject *Sender)
{
    TDoctorListForm    *docListForm_p;
    pmcDocListInfo_t    docListInfo;

    // Put up the doctor list form
    docListInfo.mode = PMC_LIST_MODE_SELECT;
    docListInfo.doctorId = FormInfo_p->doctorId;
    docListForm_p = new TDoctorListForm( this, &docListInfo );
    docListForm_p->ShowModal( );
    delete docListForm_p;

    if( docListInfo.returnCode == MB_BUTTON_OK )
    {
        FormInfo_p->doctorId = docListInfo.doctorId;
        UpdateDoctor( FormInfo_p->doctorId );
    }
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::DoctorEditMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    DoctorListButtonClick( Sender );
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::DateEditMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    DateSelectButtonClick( Sender );
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::DateEditChange(TObject *Sender)
{
    UpdateDate( Date );
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::UpdateDate( Int32u_t date )
{
    MbDateTime dateTime = MbDateTime( date, 0 );
    DateEdit->Text = dateTime.MDY_DateString( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
void __fastcall TWordCreateForm::PatientEditMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    PatientSelect( 0 );
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::PatientClearButtonClick(TObject *Sender)
{
    FormInfo_p->patientId = 0;
    UpdatePatient( FormInfo_p->patientId );
    SelectedAppointId = 0;
    UpdateAppointment( SelectedAppointId );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::DoctorClearButtonClick(TObject *Sender)
{
    FormInfo_p->doctorId = 0;
    UpdateDoctor( FormInfo_p->doctorId );
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::PatientEditButtonClick(TObject *Sender)
{
    pmcPatEditInfo_t        patEditInfo;
    TPatientEditForm       *patEditForm;

    if( FormInfo_p->patientId )
    {
        patEditInfo.patientId = FormInfo_p->patientId;
        patEditInfo.mode = PMC_EDIT_MODE_EDIT;
        sprintf( patEditInfo.caption, "Patient Details" );

        patEditForm = new TPatientEditForm( NULL, &patEditInfo );
        patEditForm->ShowModal();
        delete patEditForm;

        if( patEditInfo.returnCode == MB_BUTTON_OK )
        {
            FormInfo_p->patientId = patEditInfo.patientId;
            UpdatePatient( FormInfo_p->patientId );
        }
        else
        {
            // User must have clicked cancel button
        }
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::DoctorEditButtonClick(TObject *Sender)
{
    pmcDocEditInfo_t       docEditInfo;
    TDoctorEditForm       *docEditForm;

    if( FormInfo_p->doctorId )
    {
        docEditInfo.id = FormInfo_p->doctorId;
        docEditInfo.mode = PMC_EDIT_MODE_EDIT;
        sprintf( docEditInfo.caption, "Doctor Details" );

        docEditForm = new TDoctorEditForm( NULL, &docEditInfo );
        docEditForm->ShowModal();
        delete docEditForm;

        if( docEditInfo.returnCode == MB_BUTTON_OK )
        {
            FormInfo_p->doctorId = docEditInfo.id;
            UpdateDoctor( FormInfo_p->doctorId );
        }
    }
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::PatientEditKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{
    PatientSelect( (Int32u_t)Key );
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::PatientSelect( Int32u_t key )
{
    TPatientListForm   *patListForm_p;
    PmcPatListInfo_t    patListInfo;

    patListInfo.patientId = FormInfo_p->patientId;
    patListInfo.providerId = 0;
    patListInfo.mode = PMC_LIST_MODE_SELECT;
    patListInfo.character = key;

    patListForm_p = new TPatientListForm( this, &patListInfo );
    patListForm_p->ShowModal( );
    delete patListForm_p;

    if( patListInfo.returnCode == MB_BUTTON_OK )
    {
        if( patListInfo.patientId != FormInfo_p->patientId )
        {
            FormInfo_p->patientId = patListInfo.patientId;
            UpdatePatient( FormInfo_p->patientId );
            SelectedAppointId = 0;
            UpdateAppointment( SelectedAppointId );
        }
    }
    else
    {
        // User pressed cancel button - do nothing
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::AppSelectButtonClick(TObject *Sender)
{
    Int32u_t    newAppointId;

    if( FormInfo_p->patientId == 0 )
    {
        mbDlgInfo( "A patient must be specified before an appointment can be selected.\n" );
    }
    else
    {
        newAppointId = pmcViewAppointments( FormInfo_p->patientId, TRUE, TRUE, 0, 0, 0, 0, PMC_LIST_MODE_SELECT );
        if( newAppointId != 0 ) SelectedAppointId = newAppointId;
        UpdateAppointment( SelectedAppointId );
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::AppClearButtonClick(TObject *Sender)
{
    SelectedAppointId = 0;
    UpdateAppointment( SelectedAppointId );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TWordCreateForm::TemplateComboBoxChange(TObject *Sender)
{
    pmcProviderList_p       provider_p;
    Boolean_t               found = FALSE;
    Char_p                  buf_p;

    mbMalloc( buf_p, 256 );

    // Search for the provider last name in the document template.  In the
    // future, the list of templates should probably be in the database
    // with the provider properly specified.

    mbLockAcquire( pmcProvider_q->lock );

    qWalk( provider_p, pmcProvider_q, pmcProviderList_p )
    {
        if( provider_p->lastNameLen == 0 ) continue;

        if( mbStrPos( TemplateComboBox->Text.c_str() , provider_p->lastName_p ) >= 0 )
        {
            FormInfo_p->providerId = provider_p->id;
            found = TRUE;
            break;
        }
    }
    mbLockRelease( pmcProvider_q->lock );

    if( found ) ProviderComboBox->ItemIndex = pmcProviderIndexGet( ProviderComboBox, FormInfo_p->providerId );

    // Set description to document template
    {
        int len;
        mbStrClean( TemplateComboBox->Text.c_str(), buf_p, TRUE );
        // Get rid if the .doc at the end
        len = strlen( buf_p );
        if( len > 4 ) *( buf_p + len - 4 ) = 0;

        DescriptionEdit->Text = buf_p;
    }

    mbFree( buf_p );

}

