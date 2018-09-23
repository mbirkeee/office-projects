//---------------------------------------------------------------------------
// File:    pmcBatchImportForm.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 26, 2001
//---------------------------------------------------------------------------
// Description:
//
// Functions for managing the claim list form
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
#include "pmcMainForm.h"
#include "pmcGlobals.h"
#include "pmcDocumentEditForm.h"
#include "pmcDocumentListForm.h"
#include "pmcUtils.h"
#include "pmcDateSelectForm.h"
#include "pmcTables.h"
#include "pmcWordCreateForm.h"
#include "pmcBatchImportForm.h"
#include "pmcDocumentUtils.h"
#include "pmcDocument.h"

#pragma package(smart_init)
#pragma link "cdiroutl"
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
// Default constructor
//---------------------------------------------------------------------------
__fastcall TBatchImportForm::TBatchImportForm(TComponent* Owner)
    : TForm(Owner)
{
}

//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------
__fastcall TBatchImportForm::TBatchImportForm
(
    TComponent              *Owner,
    pmcBatchImportInfoo_p    batchInfo_p
)
: TForm(Owner)
{
    MbDateTime          dateTime;
    Char_p              buf_p;

    mbLog( "Batch Import form opened\n" );
    mbMalloc( buf_p, 256 );
    Initialized = FALSE;

    TotalCount = 0;
    ImportedCount = 0;
    FailedCount = 0;

    UpdateCounts( );

    FileListIn_q = qInitialize( &FileListInHead );
    FileListDone_q = qInitialize( &FileListDoneHead );

    BatchInfo_p = batchInfo_p;
    BatchInfo_p->returnCode = MB_BUTTON_CANCEL;

    pmcPickListBuild( DocumentTypeComboBox, &pmcDocumentBatchTypes[0], PMC_DOCUMENT_TYPE_ANY );
    DocumentTypeComboBoxIndex = -1;

    mbMalloc( ImportDir_p, 512 );
    if( batchInfo_p->mode == PMC_IMPORT_MODE_DOCS )
    {
        strcpy( ImportDir_p, pmcCfg[CFG_DOC_IMPORT_FROM_DIR].str_p );
    }
    else if( batchInfo_p->mode == PMC_IMPORT_MODE_SCANS )
    {
        strcpy( ImportDir_p, pmcCfg[CFG_DOC_IMPORT_SCANNER_DIR].str_p );
    }
    else
    {
        strcpy( ImportDir_p, pmcCfg[CFG_DOC_IMPORT_FROM_DIR].str_p );
    }

    mbMalloc( ImportFailedDir_p, 256 );
    strcpy( ImportFailedDir_p, pmcCfg[CFG_DOC_IMPORT_FAILED_DIR].str_p );

    ImportEdit->Text = ImportDir_p;
    FailedEdit->Text = ImportFailedDir_p;

    // Initialize controls
    ProviderRadioGroup->ItemIndex = 1;
    DateRadioGroup->ItemIndex = 1;
    PhnRadioGroup->ItemIndex = 1;
    PromptRadioGroup->ItemIndex = 0;

    FailPhnCheckBox->Checked = TRUE;
    FailDateCheckBox->Checked = TRUE;
    FailProviderCheckBox->Checked = TRUE;
    MoveFailedCheckBox->Checked = FALSE;

    FailedEdit->Enabled = FALSE;
    FailedDirSelectButton->Enabled = FALSE;

    Date = mbToday( );
    dateTime.SetDate( Date );
    DateEdit->Text = dateTime.MDY_DateString( );

    // Initialize the provider information
    BatchInfo_p->providerId = pmcProviderListBuild( ProviderComboBox, BatchInfo_p->providerId, FALSE, TRUE );
    ProviderIndex = ProviderComboBox->ItemIndex;

    // Get the initial list of files
    UpdateFileListIn( );
    Initialized = TRUE;

    mbFree( buf_p );
}

//---------------------------------------------------------------------------
// TODO: Must consider deprecation this ugly code
//---------------------------------------------------------------------------
Int32s_t pmcDocProcess
(
    mbFileListStruct_p      file_p,
    Int32u_p                documentId_p,
    Int32u_t                dialogMode,
    Int32u_t                providerMode,
    Int32u_t                phnMode,
    Int32u_t                dateMode,
    Int32u_t                moveFailedFlag,
    Int32u_t                providerIdIn,
    Int32u_t                patientIdIn,
    Int32u_t                dateIn,
    Int32u_t                failMask,
    Int32u_p                autoTerminateFlag_p,
    Int32u_p                formDisplayedFlag_p,
    Char_p                  failedDir_p,
    Char_p                  description_p,
    Int32u_t                importFlag
)
{
    Int32s_t                returnCode = FALSE;
    TDocumentEditForm      *form_p;
    pmcDocumentEditInfo_t   editInfo;
    bool                    dialogFlag = FALSE;
    bool                    updateFlag = FALSE;
    bool                    failedFlag = FALSE;
    bool                    phnFail = FALSE;
    Char_t                  phnDocument[32];
    Char_t                  phnFileName[32];
    bool                    phnDocumentFlag = FALSE;
    bool                    phnNameFlag = FALSE;
    bool                    added = FALSE;
    Int32u_t                existingDocumentId = 0;
    Int32u_t                providerCount = 0;
    pmcProviderList_p       provider_p;
    Int32u_t                formDisplayedFlag = FALSE;
    Char_p                  buf_p;
    Char_p                  importFileName_p;
    Int32u_t                count;
//    Int32u_t                recordLockId = 0;
    Int32u_t                documentId = 0;
    Int32u_t                fileSize;
    Int32u_t                status = 0;

    mbMalloc( buf_p,            1024 );
    mbMalloc( importFileName_p, 256 );

    memset( &editInfo, 0, sizeof(pmcDocumentEditInfo_t) );

    if( description_p )
    {
        mbMallocStr( editInfo.description_p, description_p );
    }
    editInfo.lockFailed = FALSE;

    if( dialogMode == PMC_IMPORT_DIALOG_ALWAYS ) dialogFlag = TRUE;

    // Read the passed in document Id
    if( documentId_p ) documentId = *documentId_p;

    // 20070203: Add this check because its not detecting existing documents
    if( documentId == 0 )
    {
        file_p->crc = 0;
        documentId = pmcDocExistingRecordCheck( file_p, &phnFileName[0], &editInfo, &status );

        if( status != PMC_DOCUMENT_STATUS_UNKNOWN ) documentId = 0;
    }

    if( documentId == 0 )
    {
        Int32u_t    fileSize = 0;

        // This is a new document
        editInfo.newFlag = TRUE;

        mbLog( "Starting CRC check for '%s'", file_p->fullName_p );

        // We need to compute the CRC and get the size before we check if the
        // document is already in the database
        if( ( file_p->crc = mbCrcFile( file_p->fullName_p, &fileSize, NIL, NIL, NIL, NIL, NIL ) ) == 0 )
        {
            mbDlgExclaim( "Error reading file %s\n", file_p->fullName_p );
            goto exit;
        }
        file_p->size64 = (Int64u_t)fileSize;

        mbLog( "CRC computation complete" );
        // 20021124: There may already an entry for this file.
        existingDocumentId = pmcDocExistingRecordCheck( file_p, &phnFileName[0], &editInfo, &status );

        if( existingDocumentId )
        {
            // This document already exits in the database.  By setting the
            // id to 0 a new record will be created
            editInfo.failReasonMask |= PMC_IMPORT_FAIL_DUPLICATE_NAME;
            existingDocumentId = 0;
        }

        if( existingDocumentId == 0 )
        {
            // This document does not have an existing record... determine
            // date, type, and PHN from file name

            // First lets try to determine the file type
            file_p->type = pmcFileTypeGet( file_p->name_p, file_p->type );

            if( phnMode != PMC_IMPORT_PHN_NONE )
            {
                // Attempt to get the phn and date from the file name
                pmcFileNameExtractSaskPhn( file_p->name_p, &phnFileName[0], &editInfo.date );
            }

            if( dateIn == 0 )
            {
                // 20021201: Attempt to get the date from the contents
                pmcDocSearchDate( file_p, &editInfo.date );
            }


            mbLog("Searching document contents for type.... does this take a long time?" );
            // 20021201: Attempt to get the document template from the contents
            pmcDocSearchTemplate( file_p, &editInfo.description_p );

            mbLog("Finished searching document contents for template");
        }

        editInfo.status = PMC_DOCUMENT_STATUS_FILED;

        if( editInfo.description_p == NIL )
        {
            mbMalloc( editInfo.description_p, 512 );
            *editInfo.description_p = 0;
        }

        // Deal with the phn first
        if( phnMode == PMC_IMPORT_PHN_DOCUMENT || phnMode == PMC_IMPORT_PHN_BOTH )
        {
            // Attempt to extract PHN from the file contents
            if( pmcFileExtractSaskPhn( file_p, &phnDocument[0] ) ) phnDocumentFlag = TRUE;
        }

        if( phnFileName[0] != 0 ) phnNameFlag = TRUE;

        if( phnMode == PMC_IMPORT_PHN_BOTH )
        {
            if( !phnDocumentFlag )
            {
                editInfo.failReasonMask |= PMC_IMPORT_FAIL_PHN_DOCUMENT;
                phnFail = TRUE;
            }
            if( !phnNameFlag     )
            {
                editInfo.failReasonMask |= PMC_IMPORT_FAIL_PHN_FILENAME;
                phnFail = TRUE;
            }

            if( !phnFail )
            {
                if( strcmp( phnDocument, phnFileName ) != 0 )
                {
                    // The two phns are different
                    mbLog( "PHNs differ doc '%s' name '%s'\n", phnDocument, phnFileName );
                    editInfo.failReasonMask |= PMC_IMPORT_FAIL_PHN_DIFFER;
                    phnFail = TRUE;
                }
            }
        }

        if( phnMode == PMC_IMPORT_PHN_DOCUMENT )
        {
            if( !phnDocumentFlag )
            {
                editInfo.failReasonMask |= PMC_IMPORT_FAIL_PHN_DOCUMENT;
                phnFail = TRUE;
            }
        }
        else if( phnMode == PMC_IMPORT_PHN_FILENAME )
        {
            if( !phnNameFlag )
            {
                editInfo.failReasonMask |= PMC_IMPORT_FAIL_PHN_FILENAME;
                phnFail = TRUE;
            }
            else
            {
                strcpy( phnDocument, phnFileName );
            }
        }
        else if( phnMode == PMC_IMPORT_PHN_NONE )
        {
            phnFail = TRUE;
        }

        if( !phnFail )
        {
            // PHN Matched... determine patient if necessary
            if( editInfo.patientId == 0 )
            {
                sprintf( buf_p, "select %s from %s where %s=%s",
                    PMC_SQL_FIELD_ID,
                    PMC_SQL_TABLE_PATIENTS,
                    PMC_SQL_PATIENTS_FIELD_PHN, phnDocument );

                 editInfo.patientId = pmcSqlSelectInt( buf_p, &count );
            }
            else
            {
                // Aready have a patient id
                count = 1;
            }

            if( count != 1 )
            {
                editInfo.patientId = 0;
                editInfo.failReasonMask |= PMC_IMPORT_FAIL_PHN_PATIENT;
                phnFail = TRUE;
                failedFlag = TRUE; // Fail on this condition even if PHN fail not selected
            }

            if( ( editInfo.patientId != patientIdIn ) && ( patientIdIn != 0 ) )
            {
                editInfo.failReasonMask |= PMC_IMPORT_FAIL_PHN_PATIENT_IN;
                phnFail = TRUE;
            }
        }
        else
        {
            // Phn compare failed... use the passed in patient id
            if( editInfo.patientId == 0 ) editInfo.patientId = patientIdIn;
        }

        if( ( phnFail == TRUE ) && ( failMask & PMC_IMPORT_FAIL_BIT_PHN ) )
        {
            failedFlag = TRUE;
        }

        if( dateMode == PMC_IMPORT_DATE_DETERMINE )
        {
            if( editInfo.date == 0 )
            {
                editInfo.failReasonMask |= PMC_IMPORT_FAIL_DATE_FILENAME;
                if( failMask & PMC_IMPORT_FAIL_BIT_DATE ) failedFlag = TRUE;
            }
        }
        else if( dateMode == PMC_IMPORT_DATE_SELECTED )
        {
            editInfo.date = dateIn;
        }
        else
        {
            editInfo.date = 0;
        }

        if( providerMode == PMC_IMPORT_PROVIDER_DETERMINE )
        {
            if( editInfo.providerId == 0 )
            {
                // Search the file for the various provider search strings
                mbLockAcquire( pmcProvider_q->lock );
                providerCount = 0;
                qWalk( provider_p, pmcProvider_q, pmcProviderList_p )
                {
                    if( provider_p->docSearchString_p )
                    {
                        if( strlen( provider_p->docSearchString_p ) > 0 )
                        {
                            if( pmcFileStringSearch( file_p->fullName_p, provider_p->docSearchString_p, PMC_CHECK_CASE, TRUE, NIL ) )
                            {
                                if( editInfo.providerId == 0 ) editInfo.providerId = provider_p->id;
                                providerCount++;
                            }
                        }
                    }
                }
                mbLockRelease( pmcProvider_q->lock );
            }
            else
            {
                // We have got a provider ID from the existing record
                providerCount = 1;
            }

            if( providerCount == 0 )
            {
                editInfo.failReasonMask |= PMC_IMPORT_FAIL_PROVIDER_NONE;

                if( failMask & PMC_IMPORT_FAIL_BIT_PROVIDER ) failedFlag = TRUE;
            }
            if( providerCount > 1 )
            {
                editInfo.failReasonMask |= PMC_IMPORT_FAIL_PROVIDER_MULTI;
                if( failMask & PMC_IMPORT_FAIL_BIT_PROVIDER ) failedFlag = TRUE;
            }
        }
        else if( providerMode == PMC_IMPORT_PROVIDER_SELECTED )
        {
            editInfo.providerId = providerIdIn;
        }
        else
        {
            editInfo.providerId = 0;
        }

        // Check to see if the document is already in the database
        fileSize = (Int32u_t)file_p->size64;
        sprintf( buf_p, "select %s from %s where %s=%lu and %s=%lu and %s=\"%s\" and %s=%ld",
            PMC_SQL_FIELD_ID,
            PMC_SQL_TABLE_DOCUMENTS,
            PMC_SQL_FIELD_CRC,                      file_p->crc,
            PMC_SQL_FIELD_SIZE,                     fileSize,
            PMC_SQL_DOCUMENTS_FIELD_ORIG_NAME,      file_p->name_p,
            PMC_SQL_FIELD_NOT_DELETED,              PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

        Int32u_t tempId = pmcSqlSelectInt( buf_p, &count );

        if( count > 0 )
        {
            mbLog( "Document '%s' already in database (id: %lu)\n", file_p->name_p, tempId );
            editInfo.failReasonMask |= PMC_IMPORT_FAIL_DUPLICATE_DOC;
            failedFlag = TRUE;
        }
        // Pop up dialog if failed
        if( dialogMode == PMC_IMPORT_DIALOG_IF_REQUIRED && failedFlag == TRUE ) dialogFlag = TRUE;


        if( failedFlag == FALSE ) updateFlag = TRUE;
    }
    else
    {
        // Function was called with a document id
        MbDateTime      dateTime;
        MbSQL           sql;

#if 0
        if( !pmcSqlRecordLock( PMC_SQL_TABLE_DOCUMENTS, documentId, TRUE  ) )
        {
            if( pmcSqlRecordDeleted( PMC_SQL_TABLE_DOCUMENTS, documentId ) )
            {
                mbDlgInfo( "This document record is not in the database.\n"
                               "It may have been deleted by another user.\n" );
                goto exit;
            }
            else
            {
                mbDlgInfo( "This document record is locked for editing by another user.\n" );
                editInfo.lockFailed = TRUE;
            }
        }
        else
        {
            recordLockId = documentId;
        }
#else
        if( pmcSqlRecordLock( PMC_SQL_TABLE_DOCUMENTS, documentId, TRUE  ) )
        {
            mbDlgError( "ERROR: Document was not locked: %lu", documentId );
        }
#endif

        // Document is not new... determine items from existing record
        editInfo.newFlag = FALSE;
        editInfo.id = documentId;

        //                       0  1  2  3  4  5  6  7  8  9 10
        sprintf( buf_p, "select %s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s from %s where %s=%ld",
            PMC_SQL_DOCUMENTS_FIELD_ORIG_NAME,      // 0
            PMC_SQL_FIELD_NAME,                     // 1
            PMC_SQL_FIELD_PATIENT_ID,               // 2
            PMC_SQL_FIELD_PROVIDER_ID,              // 3
            PMC_SQL_FIELD_DESC,                     // 4
            PMC_SQL_FIELD_DATE,                     // 5
            PMC_SQL_FIELD_TYPE,                     // 6
            PMC_SQL_FIELD_CREATED,                  // 7
            PMC_SQL_FIELD_MODIFIED,                 // 8
            PMC_SQL_FIELD_CRC,                      // 9
            PMC_SQL_FIELD_SIZE,                     // 10
            PMC_SQL_DOCUMENTS_FIELD_STATUS,

            PMC_SQL_TABLE_DOCUMENTS,
            PMC_SQL_FIELD_ID, documentId );

        count = 0;

        sql.Query( buf_p );
        while( sql.RowGet( ) )
        {
            // Original name
            mbFree( file_p->name_p );
            mbMallocStr( file_p->name_p, sql.String( 0 ) );

            // Status
            editInfo.status = sql.Int32u( 11 );

            // Name

            *buf_p = 0;
            if( editInfo.status == PMC_DOCUMENT_STATUS_FILED )
            {
                mbFree( file_p->fullName_p );
                if( strlen( pmcCfg[CFG_DOC_IMPORT_TO_DIR_NEW].str_p ) )
                {
                    sprintf( buf_p, "%s\\%s\\", pmcCfg[CFG_DOC_IMPORT_TO_DIR_NEW].str_p, pmcDocumentDirFromName( sql.String( 1 ) ) );
                }
                strcat( buf_p, sql.String( 1 ) );
                mbMallocStr( file_p->fullName_p, buf_p );
            }
            else if ( editInfo.status ==  PMC_DOCUMENT_STATUS_ACTIVE ||
                      editInfo.status ==  PMC_DOCUMENT_STATUS_PENDING )
            {
                mbFree( file_p->fullName_p );
                if( strlen( pmcCfg[CFG_WORD_CREATE_DIR].str_p ) )
                {
                    sprintf( buf_p, "%s\\", pmcCfg[CFG_WORD_CREATE_DIR].str_p );
                }
                strcat( buf_p, sql.String( 0 ) );
                mbMallocStr( file_p->fullName_p, buf_p );
            }

            // Must also make a copy of the import name here
            strcpy( importFileName_p, sql.String( 1 ) );

            editInfo.patientId = sql.Int32u( 2 );
            editInfo.providerId = sql.Int32u( 3 );

            // Description
            mbFree( editInfo.description_p );
            mbMallocStr( editInfo.description_p, sql.String( 4 ) );

            editInfo.date = sql.Int32u( 5 );
            file_p->type = sql.Int32u( 6 );

            dateTime.SetDateTime64( sql.DateTimeInt64u( 7 ) );
            editInfo.createdDate = dateTime.Date();
            editInfo.createdTime = dateTime.Time();

            dateTime.SetDateTime64( sql.DateTimeInt64u( 8 ) );
            editInfo.modifiedDate = dateTime.Date();
            editInfo.modifiedTime = dateTime.Time();

            file_p->crc    = sql.Int32u( 9 );
            file_p->size64 = (Int64u_t)sql.Int32u( 10 );

            count++;
        }

        // Sanity check
        if( count != 1 ) mbDlgDebug(( "Error count: %d id: %d\n", count, documentId ));

        // dialogFlag = TRUE;

        if( importFlag  )
        {
            failedFlag = FALSE;
            updateFlag = TRUE;
        }
    }

    if( dialogFlag )
    {
        failedFlag = TRUE;
        updateFlag = FALSE;

        editInfo.file_p = file_p;
        editInfo.batchMode = ( autoTerminateFlag_p ) ? TRUE : FALSE;

        form_p = new TDocumentEditForm( NIL, &editInfo );
        form_p->ShowModal( );
        delete form_p;

        // Indicate to calling code that window popped
        formDisplayedFlag = TRUE;

        if( editInfo.terminateBatch == TRUE && autoTerminateFlag_p )
        {
            *autoTerminateFlag_p = TRUE;
        }

        if( editInfo.returnCode == MB_BUTTON_OK )
        {
            // User pressed import button
            failedFlag = FALSE;
            updateFlag = TRUE;
        }
        else if( editInfo.returnCode == MB_BUTTON_CANCEL )
        {
            updateFlag = FALSE;
            failedFlag = FALSE;
        }
        else
        {
            updateFlag = FALSE;
            failedFlag = TRUE;
        }
    }

    if( updateFlag && editInfo.newFlag == TRUE )
    {
        // We are going to import the document.  First check to see if the
        // document is already in the database

        fileSize = (Int32u_t)file_p->size64;
         sprintf( buf_p, "select %s from %s where %s=%lu and %s=%lu and %s=\"%s\" and  %s=%ld",
            PMC_SQL_FIELD_ID,
            PMC_SQL_TABLE_DOCUMENTS,
            PMC_SQL_FIELD_CRC,                      file_p->crc,
            PMC_SQL_FIELD_SIZE,                     fileSize,
            PMC_SQL_DOCUMENTS_FIELD_ORIG_NAME,      file_p->name_p,
            PMC_SQL_FIELD_NOT_DELETED,              PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

        pmcSqlSelectInt( buf_p, &count );

        // There is a little hole here... another client could potentially add
        // this document to the database.  However do not worry about that
        // for now
        if( count > 0 )
        {
            if( mbDlgOkCancel( "This document is already in the database.  Continue import?\n"
                               "(Original name %s)", file_p->name_p ) == MB_BUTTON_CANCEL )
            {
                goto exit;
            }
        }

        if( existingDocumentId == 0 )
        {
            // We are going to proceed with the import.  Must create a new record
            if( ( documentId = pmcSqlRecordCreate( PMC_SQL_TABLE_DOCUMENTS, NIL ) ) == 0 )
            {
                mbDlgExclaim( "Error creating document record in database." );
                goto exit;
            }
            mbLog( "Created new document record (id: %lu)\n", documentId );
        }
        else
        {
            // There is already a database record
            documentId = existingDocumentId;
        }

        if( documentId_p ) *documentId_p = documentId;

        added = TRUE;

        // Next we must successfully copy the file to the actual database directory
        sprintf( importFileName_p, "%08ld-%08ld.pmc", mbToday( ), documentId );

        sprintf( buf_p, "%s\\%s\\%s", pmcCfg[CFG_DOC_IMPORT_TO_DIR_NEW].str_p,
            pmcDocumentDirFromName( importFileName_p ),
            importFileName_p );

        if( mbFileCopy( file_p->fullName_p, buf_p ) != MB_RET_OK )
        {
            // Failed to copy file... must delete record
            // pmcSqlRecordDelete( PMC_SQL_TABLE_DOCUMENTS, documentId );
            mbDlgError( "Failed to copy document %s to database", file_p->fullName_p );
            goto exit;
        }
        else
        {
            // Sucessfully copied document to the import directory.
            // Get rid of original file
            unlink( file_p->fullName_p );

            // Update the path to the document (so that is can still be viewed)
            mbFree( file_p->fullName_p );
            mbMalloc( file_p->fullName_p, strlen( buf_p ) + 1 );
            strcpy( file_p->fullName_p, buf_p );
        }
        returnCode = TRUE;
    }

    if( updateFlag && editInfo.newFlag == FALSE && importFlag )
    {
        editInfo.status = PMC_DOCUMENT_STATUS_FILED;

        // Next we must successfully copy the file to the actual database directory
        sprintf( importFileName_p, "%08ld-%08ld.pmc", mbToday( ), documentId );

        sprintf( buf_p, "%s\\%s\\%s", pmcCfg[CFG_DOC_IMPORT_TO_DIR_NEW].str_p,
            pmcDocumentDirFromName( importFileName_p ),
            importFileName_p );


        // mbFileCopyCall( srcFileName_p, destFileName_p, NIL, NIL, NIL, NIL, NIL )
        if( mbFileCopy( file_p->fullName_p, buf_p ) != MB_RET_OK )
        {
            // Failed to copy file... must delete record
            // pmcSqlRecordDelete( PMC_SQL_TABLE_DOCUMENTS, documentId );
            mbDlgError( "Failed to copy document %s to database", file_p->fullName_p );
            goto exit;
        }
        else
        {
            mbLog("Computing the CRC for %s (have %lu)", buf_p, file_p->crc);
            if( ( file_p->crc = mbCrcFile( buf_p , &fileSize, NIL, NIL, NIL, NIL, NIL ) ) == 0 )
            {
                mbDlgExclaim( "Error reading file %s\n", file_p->fullName_p );
                goto exit;
            }

            mbLog("Got CRC %lu", file_p->crc);
            file_p->size64 = (Int64u_t)fileSize;

            mbLog("Finished the import");
            // Sucessfully copied document to the import directory.
            // Get rid of original file
            unlink( file_p->fullName_p );
        }
    }

    if( updateFlag )
    {
        fileSize = (Int32u_t)file_p->size64;
        // At this point we are going to update the existing record
        //                             orig name  name       crc    size
        sprintf( buf_p, "update %s set %s=\"%s\", %s=\"%s\", %s=%lu,%s=%lu,%s=%ld,%s=%ld,%s=%ld,%s=%ld,%s=\"%s\",%s=%ld where %s=%ld",
            PMC_SQL_TABLE_DOCUMENTS,
            PMC_SQL_DOCUMENTS_FIELD_ORIG_NAME,      file_p->name_p,
            PMC_SQL_FIELD_NAME,                     importFileName_p,
            PMC_SQL_FIELD_CRC,                      file_p->crc,
            PMC_SQL_FIELD_SIZE,                     fileSize,
            PMC_SQL_FIELD_PATIENT_ID,               editInfo.patientId,
            PMC_SQL_FIELD_PROVIDER_ID,              editInfo.providerId,
            PMC_SQL_FIELD_DATE,                     editInfo.date,
            PMC_SQL_FIELD_TYPE,                     file_p->type,
            PMC_SQL_FIELD_DESC,                     editInfo.description_p,
            PMC_SQL_DOCUMENTS_FIELD_STATUS,         editInfo.status,
            PMC_SQL_FIELD_ID,                       documentId );

        if( !pmcSqlExec( buf_p ) )
        {
            mbDlgDebug(( "Failed to update document %ld\n", documentId ));
        }
        else
        {
            mbLog( "Updated document '%s' (id: %lu)\n", file_p->name_p, documentId );
        }

        returnCode = TRUE;

        if( editInfo.newFlag )
        {
            pmcSqlRecordUndelete(  PMC_SQL_TABLE_DOCUMENTS, documentId );
        }
    }
    else
    {
        mbLog( "Not updating document\n" );
    }

    /* 2017_11_18:  Add logging in attempt to determine why failed */
    /* documents are not getting moved to the failed directory */

#if 0
    mbLog("Test if document %s should be moved to failed directory\n", file_p->name_p);
    if( failedFlag == True )
    {
        mbLog( "failedFlag: TRUE\n" );
    }
    else
    {
        mbLog( "failedFlag: FALSE\n" );
    }

    if( moveFailedFlag == True )
    {
        mbLog( "moveFailedFlag: TRUE\n" );
    }
    else
    {
        mbLog( "moveFailedFlag: FALSE\n" );
    }

    if( failedDir_p )
    {
        mbLog( "failedDir_p: TRUE\n" );
    }
    else
    {
        mbLog( "failedDir_p: FALSE\n" );
    }

    if( editInfo.newFlag )
    {
        mbLog( "editInfo.newFlag: TRUE\n" );
    }
    else
    {
        mbLog( "editInfo.newFlag: FALSE\n" );
    }

    if( importFlag == TRUE )
    {
        mbLog( "importFlag: TRUE\n" );
    }
    else
    {
        mbLog( "importFlag: FALSE\n" );
    }

#endif
    /* end of added logging to determine problem */

#if 0
    /* 2017_11_18:  I think there is a bug in here.  The "importFlag" must
       be TRUE to allow the document to be moved to the failed directory.
       However I am not totally sure what the consequences of changing this
       test to TRUE is */
    if(    failedFlag == TRUE
        && moveFailedFlag == TRUE
        && failedDir_p
        && editInfo.newFlag
        && importFlag == FALSE )
#endif

    if(    failedFlag == TRUE
        && moveFailedFlag == TRUE
        && failedDir_p
        && editInfo.newFlag
        && importFlag == TRUE )
    {
        sprintf( buf_p, "%s\\%s", failedDir_p, file_p->name_p );
        if( mbFileCopy( file_p->fullName_p, buf_p ) == MB_RET_OK )
        {
            unlink( file_p->fullName_p );
            // Update the full name so the file can still be found from this list
            mbFree( file_p->fullName_p );
            mbMallocStr( file_p->fullName_p, buf_p );
        }
        else
        {
            mbDlgError( "Error moving %s to failed directory\n", file_p->fullName_p );
        }
    }

exit:

//    if( recordLockId ) pmcSqlRecordUnlock( PMC_SQL_TABLE_DOCUMENTS, recordLockId );

    // Indicate if form was displayed
    if( formDisplayedFlag_p ) *formDisplayedFlag_p = formDisplayedFlag;

    // Update count if document added (and not in batch mode)
    if( !autoTerminateFlag_p && added ) MainForm->DocumentCountUpdate( PMC_COUNTER_UPDATE );

    mbFree( buf_p );
    mbFree( importFileName_p );
    mbFree( editInfo.description_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Function: pmcDocSearchTemplate
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p  TemplateStrings[]
=
{
     "Bree Letter Consult"
    ,"Bree Echo Doppler Report"
    ,"Requisition Onsite Echo"
    ,"Requisition SDH Electrodiagnostics"
    ,"Bree Letter Generic"
    ,"Bree Letter No Show"
    ,"Bree Letter Referral Routine"
    ,"Bree Letter Referral Urgent"
    ,"Bree Letter to Doctor"
    ,"Bree Letter to Patient"
    ,"Bree Patient Info Request"
    ,"Bree App Letter Returned"
    ,"Bree Referral Urgent"
    ,"Bree Referral Routine"
    ,"Dr. Terry Bree Dictations"
    ,"Terry Bree Dictation"
    ,"Spadina Medical Consultation Letter"
    ,"Bree Dictations"
    ,"bree letterhead"
    ,"ECHO DOPPLER REPORT"
    ,"Invalid"
};

//---------------------------------------------------------------------------

Int32s_t pmcDocSearchTemplate
(
    mbFileListStruct_p      file_p,
    Char_p                 *result_pp
)
{
    Int32s_t    returnCode = MB_RET_ERR;
    Int32u_t    i;
    Int32u_t    matchCount = 0;
    Char_p      temp_p = NIL;

    if( file_p->type != PMC_DOCUMENT_TYPE_WORD )
    {
        mbLog("Skip template check for file type %d", file_p->type);
        goto exit;
    }

    for( i = 0 ; ; i++ )
    {
        if( strcmp( TemplateStrings[i], "Invalid" ) == 0 ) break;

        mbLog("Searching document for '%s' doc type %d", TemplateStrings[i], file_p->type);

        if( pmcFileStringSearch( file_p->fullName_p, TemplateStrings[i], PMC_CHECK_CASE, TRUE, NIL ) )
        {
            //mbLog( "got match '%s'\n", TemplateStrings[i] );
            matchCount++;
            mbMallocStr( temp_p, TemplateStrings[i] );
            break;
        }
    }

    if( matchCount != 1 ) goto exit;

    if( strcmp( temp_p, "bree letterhead" ) == 0 ) sprintf( temp_p, "Bree Letterhead" );

    if( result_pp ) *result_pp = temp_p;
    returnCode = MB_RET_OK;
exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// Function: pmcDocSearchDate
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p  TmpMonthStrings[]
=
{
     "Jan"
    ,"Feb"
    ,"Mar"
    ,"Apr"
    ,"May"
    ,"Jun"
    ,"Jul"
    ,"Aug"
    ,"Sep"
    ,"Oct"
    ,"Nov"
    ,"Dec"
    ,"Invalid"
};

//---------------------------------------------------------------------------

Char_p  TmpDateStrings[]
=
{
     " 2000"
    ," 2001"
    ," 2002"
    ," 01"
    ,"Invalid"
};

//---------------------------------------------------------------------------

Int32s_t pmcDocSearchDate
(
    mbFileListStruct_p  file_p,
    Int32u_p            date_p
)
{
    Int32s_t    returnCode = MB_RET_ERR;
    Int32u_t    i;
    Int32u_t    offset;
    FILE       *fp = NIL;
    Int8u_t     buf[64];
    Int32u_t    firstPosition = 0xFFFFFFFF;
    Int8u_t     c;
    int         pos;
    Int32u_t    year = 0;
    Int32u_t    month = 0;
    Int32u_t    day = 0;
    Int32u_t    date = 0;

    for( i = 0 ; ; i++ )
    {
        if( strcmp( TmpDateStrings[i], "Invalid" ) == 0 ) break;
        if( pmcFileStringSearch( file_p->fullName_p, TmpDateStrings[i], PMC_CHECK_CASE, FALSE, &offset ) )
        {
            if( offset < firstPosition )
            {
                firstPosition = offset;
            }
        }
    }

    if( firstPosition == 0xFFFFFFFF )
    {
        mbLog( "Did not find Year string in '%s'\n", file_p->name_p );
        goto exit;
    }

    firstPosition -= 15;
    fp = fopen( file_p->fullName_p, "rb" );
    if( fp == NIL ) goto exit;

    // Read some bytes from the file where the date is suspected to be
    fseek( fp, firstPosition, SEEK_SET);
    fread( (Void_p)buf, 1, 32, fp );
    buf[24] = 0;

    // Clean the string
    {
        Int8u_p  temp1_p = buf;
        Int8u_p  temp2_p = buf;

        for( i = 0 ; i < 24 ; i++ )
        {
            c = *temp1_p++;

            if(    ( c >= '0' && c <= '9' )
                || ( c >= 'a' && c <= 'z' )
                || ( c >= 'A' && c <= 'Z' )
                || ( c == ' ' )
                || ( c == ',' ) )
            {
                *temp2_p++ = c;
            }
        }
        *temp2_p = 0;
    }

    for( i = 0 ; ; i++ )
    {
        if( strcmp( TmpDateStrings[i], "Invalid" ) == 0 )
        {
            break;
        }
        pos = mbStrPos( (Char_p)buf, TmpDateStrings[i] );
        if( pos > 0 )
        {
            buf[pos] = 0;
            if( i == 0 ) year = 2000;
            if( i == 1 || i == 3 ) year = 2001;
            if( i == 2 ) year = 2002;
            break;
        }
    }

    if( year == 0 ) goto exit;

    for( i = 0 ; ; i++ )
    {
        if( strcmp( TmpMonthStrings[i], "Invalid" ) == 0 )
        {
            break;
        }
        pos = mbStrPos( (Char_p)buf, TmpMonthStrings[i] );
        if( pos >= 0 )
        {
            month = i + 1;
            break;
        }
    }

    if( month == 0 ) goto exit;

    buf[pos] = 0;

    mbStrDigitsOnly( (Char_p)&buf[pos+1] );

    day = atol( (Char_p)&buf[pos+1] );

    if( day < 1 || day > 31 ) goto exit;

    date = year * 10000 + month * 100 + day;

    if( date_p ) *date_p = date;

    returnCode = MB_RET_OK;
exit:

    if( fp ) fclose( fp );
    return returnCode;
}

//---------------------------------------------------------------------------
// Function: pmcDocExistingRecordCheck
//---------------------------------------------------------------------------
// This function imports the "new style" documents that have no PHN in the
// filename but already have a pre-existing record
//---------------------------------------------------------------------------

Int32u_t pmcDocExistingRecordCheck
(
    mbFileListStruct_p          file_p,
    Char_p                      phn_p,
    pmcDocumentEditInfo_p       editInfo_p,
    Int32u_p                    status_p
)
{
    Char_p              buf_p;
    Char_p              where_p;
    Int32u_t            count;
    Int32u_t            id = 0;
    Int32u_t            date = 0;
    Int32u_t            patientId = 0;
    Int32u_t            providerId = 0;
    Int32u_t            type = 0;
    Int32u_t            status;
    PmcSqlPatient_p     pat_p;
    MbSQL               sql;

    mbMalloc( buf_p, 1024 );
    mbMalloc( where_p, 512 );
    mbCalloc( pat_p, sizeof( PmcSqlPatient_t ) );

    if( file_p == NIL || phn_p == NIL || editInfo_p == NIL ) goto exit;

    // First lets see if this document is in the database
    sprintf( buf_p, "select %s,%s,%s,%s,%s,%s,%s from %s ",
        PMC_SQL_FIELD_ID,                           // 0
        PMC_SQL_DOCUMENTS_FIELD_STATUS,             // 1
        PMC_SQL_FIELD_DATE,                         // 2
        PMC_SQL_FIELD_PATIENT_ID,                   // 3
        PMC_SQL_FIELD_PROVIDER_ID,                  // 4
        PMC_SQL_FIELD_DESC,                         // 5
        PMC_SQL_FIELD_TYPE,                         // 6

        PMC_SQL_TABLE_DOCUMENTS );

    if( file_p->crc )
    {
        sprintf( where_p, " where %s=\"%s\" and %s=%lu",
            PMC_SQL_DOCUMENTS_FIELD_ORIG_NAME,      file_p->name_p,
            PMC_SQL_FIELD_CRC,                      file_p->crc );
    }
    else
    {
            sprintf( where_p, " where %s=\"%s\"",
            PMC_SQL_DOCUMENTS_FIELD_ORIG_NAME,      file_p->name_p );
    }

    strcat( buf_p, where_p );

    count = 0;

    sql.Query( buf_p );

    while( sql.RowGet( ) )
    {
        id            = sql.Int32u( 0 );
        status        = sql.Int32u( 1 );
        date          = sql.Int32u( 2 );
        patientId     = sql.Int32u( 3 );
        providerId    = sql.Int32u( 4 );

        // Description
        mbFree( editInfo_p->description_p );
        mbMallocStr( editInfo_p->description_p, sql.String( 5 ) );

        type          = sql.Int32u( 6 );

        count++;
    }

    if( id == 0 ) goto exit;
    if( count == 0 ) goto exit;
    if( count > 1 )
    {
        //mbDlgDebug(( "Unexpected count: %ld\n", count ));
        id = 0;
        goto exit;
    }

    // Get the patient details
    pmcSqlPatientDetailsGet( patientId, pat_p );

    if( phn_p ) strcpy( phn_p, pat_p->phn );
    editInfo_p->date = date;
    editInfo_p->patientId = patientId;
    editInfo_p->providerId = providerId;
    file_p->type = type;
    if( status_p ) *status_p = status;

exit:

    if( id )
    {
        mbLog( "Found existing record for document '%s' (id: %ld)\n", file_p->name_p, id );
    }
    else
    {
        if( file_p->name_p )
        {
            mbLog( "Did not find record for document '%s' (id: %ld)\n", file_p->name_p, id );
        }
        else
        {
            mbLog( "Did not find record for document id: %ld\n", id );
        }
    }

    mbFree( buf_p );
    mbFree( pat_p );
    mbFree( where_p );
    return id;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TBatchImportForm::CancelButtonClick(TObject *Sender)
{
    mbLog( "Batch import canceled\n" );
    BatchInfo_p->returnCode = MB_BUTTON_CANCEL;
    Close( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TBatchImportForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
    // Free any remaining entries in the file lists
    mbFileListFree( FileListIn_q );
    mbFileListFree( FileListDone_q );

    ListViewIn->Items->BeginUpdate( );
    ListViewSucceeded->Items->BeginUpdate( );
    ListViewFailed->Items->BeginUpdate( );

    ListViewIn->Items->Clear( );
    ListViewSucceeded->Items->Clear( );
    ListViewFailed->Items->Clear( );

    ListViewIn->Items->EndUpdate( );
    ListViewSucceeded->Items->EndUpdate( );
    ListViewFailed->Items->EndUpdate( );

    // Store the updated directories
    if( BatchInfo_p->mode == PMC_IMPORT_MODE_DOCS )
    {
        if( pmcCfg[CFG_DOC_IMPORT_FROM_DIR].str_p  ) mbFree( pmcCfg[CFG_DOC_IMPORT_FROM_DIR].str_p );
        mbMalloc( pmcCfg[CFG_DOC_IMPORT_FROM_DIR].str_p, strlen( ImportDir_p ) + 1 );
        strcpy( pmcCfg[CFG_DOC_IMPORT_FROM_DIR].str_p, ImportDir_p );
    }        

    if( pmcCfg[CFG_DOC_IMPORT_FAILED_DIR].str_p  ) mbFree( pmcCfg[CFG_DOC_IMPORT_FAILED_DIR].str_p );
    mbMalloc( pmcCfg[CFG_DOC_IMPORT_FAILED_DIR].str_p, strlen( ImportFailedDir_p ) + 1 );
    strcpy( pmcCfg[CFG_DOC_IMPORT_FAILED_DIR].str_p, ImportFailedDir_p );

    if( ImportDir_p ) mbFree( ImportDir_p );
    if( ImportFailedDir_p ) mbFree( ImportFailedDir_p );

    Action = caFree;
}

//---------------------------------------------------------------------------
// Function: StartButtonClick
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TBatchImportForm::StartButtonClick(TObject *Sender)
{
    mbFileListStruct_p      file_p;
    TThermometer           *thermometer_p = NIL;
    Char_p                  buf_p;
    TListItem              *itemIn_p;
    TListItem              *itemOut_p;
    TPoint                  point;
    Int32u_t                terminate = FALSE;
    Int32u_t                failMask;
    Int32u_t                formDisplayed;
    TCursor                 origCursor;

    mbMalloc( buf_p, 256 );

    CancelButton->SetFocus();

    // Scroll input window back to top if required
    point = ListViewIn->ViewOrigin;
    ListViewIn->Scroll( 0, -1 * point.y  );

    origCursor = Screen->Cursor;
    Screen->Cursor = crHourGlass;

    for( ; ; )
    {
        // Check if any more documents to process
        if( ListViewIn->Items->Count == 0 ) break;

        // Get first item from list
        itemIn_p = ListViewIn->Items->Item[0];

        file_p = (mbFileListStruct_p)itemIn_p->Data;

        // Set the file type to the type selected in this form;
        file_p->type = pmcDocumentTypes[DocumentTypeComboBox->ItemIndex].code;

        failMask = 0 ;
        formDisplayed = FALSE;

        if( FailPhnCheckBox->Checked      ) failMask |= PMC_IMPORT_FAIL_BIT_PHN;
        if( FailDateCheckBox->Checked     ) failMask |= PMC_IMPORT_FAIL_BIT_DATE;
        if( FailProviderCheckBox->Checked ) failMask |= PMC_IMPORT_FAIL_BIT_PROVIDER;

        if( pmcDocProcess( file_p,
                           NIL,                           // Document ID (NIL = NEW)
                           PromptRadioGroup->ItemIndex,
                           ProviderRadioGroup->ItemIndex,
                           PhnRadioGroup->ItemIndex,
                           DateRadioGroup->ItemIndex,
                           MoveFailedCheckBox->Checked,
                           BatchInfo_p->providerId,
                           0,                          // patient id
                           Date,
                           failMask,
                          &terminate,
                          &formDisplayed,
                           FailedEdit->Text.c_str(),
                           NIL, TRUE ) == TRUE )
        {
            // Import was successful
            itemOut_p = ListViewSucceeded->Items->Add( );
            ImportedCount++;
        }
        else
        {
            // Import was not successful
             itemOut_p = ListViewFailed->Items->Add( );
             FailedCount++;
        }

        UpdateCounts( );

        if( formDisplayed )
        {
            // Only redraw the form if the document edit form popped up over top
            Invalidate( );
            Update( );
        }

        itemOut_p->Caption = itemIn_p->Caption;
        itemOut_p->Data = itemIn_p->Data;

        // Remove the item from the input list
        ListViewIn->Items->Delete( 0 );

        // Redraw the lists
        ListViewIn->Update( );
        ListViewSucceeded->Update( );
        ListViewFailed->Update( );

        if( terminate == TRUE )
        {
            mbDlgInfo( "Batch document import terminated." );
            break;
        }

        StartButton->Enabled = TRUE;
        StartButton->Visible = TRUE;
    }

    MainForm->DocumentCountUpdate( PMC_COUNTER_UPDATE );

    // Restore the cursor
    Screen->Cursor = origCursor;

    if( thermometer_p ) delete thermometer_p;

    // Don't allow user to change document type once started
    DocumentTypeComboBox->Enabled = FALSE;

    // Change buttons to force user to quit (if no more items)
    if( ListViewIn->Items->Count == 0 )
    {
        StartButton->Enabled = FALSE;
        CancelButton->Caption = "OK";
    }
    else
    {
        StartButton->Caption = "Resume Import";
    }

    CancelButton->SetFocus();

    mbFree( buf_p );
}

//---------------------------------------------------------------------------
// Function: ListViewInDblClick
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TBatchImportForm::ListViewInDblClick(TObject *Sender)
{
    TListItem              *item_p;
    mbFileListStruct_p      file_p;

    item_p =  ListViewIn->Selected;

    if( item_p )
    {
        file_p = (mbFileListStruct_p)item_p->Data;

        pmcDocumentView( file_p, TRUE );
    }
}

//---------------------------------------------------------------------------
// Function: UpdateFileListIn
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TBatchImportForm::UpdateFileListIn( void )
{
    TCursor                 origCursor;
    mbFileListStruct_p      file_p;
    TListItem              *item_p;
    Int32u_t                type;
    Int32u_t                size;
    Int32u_t                i;

    origCursor = Screen->Cursor;
    Screen->Cursor = crHourGlass;

    // Free any entries in the existing lists
    mbFileListFree( FileListIn_q );
    mbFileListFree( FileListDone_q );

    ListViewIn->Items->BeginUpdate( );
    ListViewIn->Items->Clear( );
    ListViewSucceeded->Items->BeginUpdate( );
    ListViewSucceeded->Items->Clear( );
    ListViewFailed->Items->BeginUpdate( );
    ListViewFailed->Items->Clear( );

    // For now assume that the type matches the index.  This should be
    // beefed up later
    type = pmcFileIndexToType( DocumentTypeComboBox->ItemIndex );

    TotalCount = 0;
    ImportedCount = 0;
    FailedCount = 0;

    if( type >= PMC_DOCUMENT_TYPE_INVALID || type < 0 )
    {
        mbDlgDebug(( "Invalid document type" ));
        goto exit;
    }

    mbFileListGet( ImportDir_p, pmcDocumentTypeFilterStrings[type], FileListIn_q, FALSE );

    // MAB:20021208: Get rid of the merge file (if its found)
    size = FileListIn_q->size;
    for( i = 0 ; i < size ; i++ )
    {
        file_p = (mbFileListStruct_p)qRemoveFirst( FileListIn_q );

        if( mbStrPos( file_p->name_p, PMC_WORD_CREATE_MERGE_FILE ) == 0 )
        {
            mbFileListFreeElement( file_p );
        }
        else
        {
            qInsertLast( FileListIn_q, file_p );
        }
    }

    qWalk( file_p, FileListIn_q, mbFileListStruct_p )
    {
        file_p->type = type;

        item_p = ListViewIn->Items->Add( );
        item_p->Caption = file_p->name_p;

        // Set pointer to file entry in queue
        item_p->Data = (Void_p)file_p;
    }

    ListViewIn->Items->EndUpdate( );
    ListViewSucceeded->Items->EndUpdate( );
    ListViewFailed->Items->EndUpdate( );

    TotalCount = FileListIn_q->size;

exit:
    Screen->Cursor = origCursor;

    if( Initialized ) StartButton->SetFocus();

    UpdateCounts( );

    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TBatchImportForm::DocumentTypeComboBoxChange( TObject *Sender )
{
    // Check to see if index is valid
    if( DocumentTypeComboBox->ItemIndex >= 0 )
    {
        // Index is valid... has it changed?
        if( DocumentTypeComboBoxIndex != DocumentTypeComboBox->ItemIndex )
        {
            DocumentTypeComboBoxIndex = DocumentTypeComboBox->ItemIndex;
            UpdateFileListIn(  );
        }
    }
    else
    {
        // Index is invalid... restore previous value
        DocumentTypeComboBox->ItemIndex = DocumentTypeComboBoxIndex;
    }

    // Restore text
    DocumentTypeComboBox->Text = pmcDocumentBatchTypes[DocumentTypeComboBox->ItemIndex].description_p;
    DocumentTypeComboBox->Invalidate( );
    if( Initialized ) StartButton->SetFocus();

    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TBatchImportForm::DateSelectButtonClick(TObject *Sender)
{
    TDateSelectForm    *dateSelectForm_p;
    pmcDateSelectInfo_t dateSelectInfo;

    dateSelectInfo.mode = PMC_DATE_SELECT_PARMS;
    dateSelectInfo.string_p = NIL;
    dateSelectInfo.dateIn = Date;
    dateSelectInfo.caption_p = "Select Document Date";

    dateSelectForm_p = new TDateSelectForm( NULL,&dateSelectInfo );

    dateSelectForm_p->ShowModal( );
    delete dateSelectForm_p;

    if( dateSelectInfo.returnCode == MB_BUTTON_OK )
    {
        Date = dateSelectInfo.dateOut;
        MbDateTime dateTime = MbDateTime( Date, 0 );
        DateEdit->Text = dateTime.MDY_DateString( );
        DateRadioGroup->ItemIndex = 1;
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TBatchImportForm::ImportDirSelectButtonClick(
      TObject *Sender)
{
#if 0
    if( strcmp( ImportDir_p, pmcCfg[CFG_DOC_IMPORT_TO_DIR].str_p ) == 0 )
    {
    }
    else
    {
        UpdateDirectory( ImportDir_p );
    }
#endif
    UpdateDirectory( ImportDir_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TBatchImportForm::FailedDirSelectButtonClick(
      TObject *Sender)
{
    UpdateDirectory( ImportFailedDir_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TBatchImportForm::ImportEditMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    UpdateDirectory( ImportDir_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TBatchImportForm::FailedEditMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    UpdateDirectory( ImportFailedDir_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TBatchImportForm::UpdateDirectory( Char_p dir_p )
{
    TCursor                 origCursor;
    Void_p                  handle_p = NIL;
    Char_p                  newDir_p;

    mbMalloc( newDir_p, MAX_PATH );

    origCursor = Screen->Cursor;
    Screen->Cursor = crHourGlass;

    handle_p = mbPathHandleGet( dir_p );

    if( mbDlgBrowseFolderNew( newDir_p, ( dir_p == ImportDir_p )
                              ? "Choose Import Documents Folder"
                              : "Choose Failed Documents Folder",
                              &handle_p ) == MB_BUTTON_OK )
    {
        strcpy( dir_p, newDir_p );

        ImportEdit->Text = ImportDir_p;
        FailedEdit->Text = ImportFailedDir_p;

        if( dir_p == ImportDir_p ) UpdateFileListIn(  );
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

void __fastcall TBatchImportForm::ProviderComboBoxChange(TObject *Sender)
{
    Int32u_t    newProviderId;
    Char_p      buf_p;

    if( !Active ) return;

    mbMalloc( buf_p, 64 );

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
        ProviderRadioGroup->ItemIndex = 1;
        BatchInfo_p->providerId = newProviderId;
    }
    ProviderComboBox->Text = pmcProviderDescGet( BatchInfo_p->providerId, buf_p );

    mbFree( buf_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TBatchImportForm::DateEditMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    DateSelectButtonClick( Sender );
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TBatchImportForm::FormPaint(TObject *Sender)
{
    nbDlgDebug(( "Called\n" ));
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TBatchImportForm::UpdateCounts( void )
{
    Char_t  buf[32];

    // Sanity Check
    if( ( ImportedCount + FailedCount ) > TotalCount )
    {
        mbDlgDebug(( "Cound Error" ));
    }
    else
    {
        RemainingCount = TotalCount - ImportedCount - FailedCount;
    }
    sprintf( buf, "%ld", TotalCount );
    TotalCountLabel->Caption = buf;

    sprintf( buf, "%ld", RemainingCount );
    RemainingCountLabel->Caption = buf;

    sprintf( buf, "%ld", ImportedCount );
    ImportedCountLabel->Caption = buf;

    sprintf( buf, "%ld", FailedCount );
    FailedCountLabel->Caption = buf;

    TotalCountLabel->Invalidate();
    TotalCountLabel->Update();
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TBatchImportForm::MoveFailedCheckBoxClick(TObject *Sender)
{
    if( MoveFailedCheckBox->Checked )
    {
        FailedEdit->Enabled = TRUE;
        FailedDirSelectButton->Enabled = TRUE;
    }
    else
    {
        FailedEdit->Enabled = FALSE;
        FailedDirSelectButton->Enabled = FALSE;
    }
}


//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TBatchImportForm::ListViewSucceededDblClick
(
      TObject *Sender
)
{
    TListItem              *item_p;
    mbFileListStruct_p      file_p;

    item_p =  ListViewSucceeded->Selected;

    if( item_p )
    {
        file_p = (mbFileListStruct_p)item_p->Data;
        pmcDocumentView( file_p, TRUE );
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TBatchImportForm::ListViewFailedDblClick(TObject *Sender)
{
    TListItem              *item_p;
    mbFileListStruct_p      file_p;

    item_p =  ListViewFailed->Selected;

    if( item_p )
    {
        file_p = (mbFileListStruct_p)item_p->Data;

        pmcDocumentView( file_p, TRUE );
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t    pmcDocImportSingle( Int32u_t patientId )
{
    mbFileListStruct_p      file_p;
    Int32s_t                returnCode = FALSE;
    Int32u_t                newDocumentId = 0;

    if( pmcCfg[CFG_DATABASE_LOCAL].value == FALSE )
    {
        mbDlgInfo( "Cannot import documents when connected to a remote database.\n" );
        goto exit;
    }

    mbCalloc( file_p, sizeof(mbFileListStruct_t) );
    mbMalloc( file_p->name_p,     128 );
    mbMalloc( file_p->fullName_p, 256 );

    MainForm->OpenDialog->Title = "Select document to import..";
    MainForm->OpenDialog->InitialDir = ( pmcCfg[ CFG_DOC_IMPORT_FROM_DIR ].str_p ) ? pmcCfg[ CFG_DOC_IMPORT_FROM_DIR ].str_p : "" ;
    MainForm->OpenDialog->Filter = "All Files (*.*)|*.*";
    MainForm->OpenDialog->FileName = "";

    if( MainForm->OpenDialog->Execute() == TRUE )
    {
        strcpy( file_p->fullName_p, MainForm->OpenDialog->FileName.c_str() );
        pmcFilePathAndNameGet( file_p->fullName_p, NIL, file_p->name_p );
        file_p->type = PMC_DOCUMENT_TYPE_ANY;
        file_p->type = pmcFileTypeGet( file_p->name_p, PMC_DOCUMENT_TYPE_ANY );

        returnCode = pmcDocProcess
        (
            file_p,                         // File details
           &newDocumentId,                  // document id - (0 = NEW)
            PMC_IMPORT_DIALOG_ALWAYS,       // Show dialog or not
            PMC_IMPORT_PROVIDER_DETERMINE,
            PMC_IMPORT_PHN_BOTH,
            PMC_IMPORT_DATE_DETERMINE,
            FALSE,                          // TRUE = moved failed to fail dir
            0, patientId,                   // providerId, patientId
            mbToday( ), 0,                  // date, failMask
            NIL,                            // result flag - terminate auto import
            NIL,                            // result flag - form displayed
            NIL,                            // Failed directory
            NIL,                            // Description
            FALSE
        );

        if( returnCode == TRUE )
        {
            mbDlgInfo( "Document imported into database." );
        }
        else
        {
            mbDlgInfo( "Document not imported into database." );
        }
    }

    mbFileListFreeElement( file_p );
exit:
    return newDocumentId;
}

//---------------------------------------------------------------------------
// Function:  pmcDocInDatabase
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32s_t    pmcDocInDatabase( void )
{
    mbFileListStruct_p      file_p;
    Int32s_t                returnCode = FALSE;
    Int32u_t                count = 0;
    Int32u_t                fileSize;
    Char_p                  buf_p;

    mbCalloc( file_p, sizeof(mbFileListStruct_t) );
    mbMalloc( file_p->name_p,     128 );
    mbMalloc( file_p->fullName_p, 256 );
    mbMalloc( buf_p, 512 );

    MainForm->OpenDialog->Title = "Select document to check...";
    MainForm->OpenDialog->InitialDir = ( pmcCfg[ CFG_DOC_IMPORT_FROM_DIR ].str_p ) ? pmcCfg[ CFG_DOC_IMPORT_FROM_DIR ].str_p : "" ;
    MainForm->OpenDialog->Filter = "All Files (*.*)|*.*";
    MainForm->OpenDialog->FileName = "";

    if( MainForm->OpenDialog->Execute() == TRUE )
    {
        strcpy( file_p->fullName_p, MainForm->OpenDialog->FileName.c_str() );
        pmcFilePathAndNameGet( file_p->fullName_p, NIL, file_p->name_p );

        if( ( file_p->crc = mbCrcFile( file_p->fullName_p, &fileSize, NIL, NIL, NIL, NIL, NIL ) ) == 0 )
        {
            mbDlgExclaim( "Error reading file %s\n", file_p->fullName_p );
            goto exit;
        }
        file_p->size64 = (Int64u_t)fileSize;

        // Check to see if the document is already in the database
        sprintf( buf_p, "select %s from %s where %s=%lu and %s=%lu and %s>%lu and %s=\"%s\"",
            PMC_SQL_FIELD_ID,
            PMC_SQL_TABLE_DOCUMENTS,
            PMC_SQL_FIELD_CRC,                  file_p->crc,
            PMC_SQL_FIELD_SIZE,                 fileSize,
            PMC_SQL_DOCUMENTS_FIELD_STATUS,     PMC_DOCUMENT_STATUS_UNKNOWN,
            PMC_SQL_DOCUMENTS_FIELD_ORIG_NAME,  file_p->name_p );

        pmcSqlSelectInt( buf_p, &count );

        if( count == 0 )
        {
            mbDlgInfo( "%s\n\nis not in the database.\n", file_p->name_p );
        }
        else if( count == 1 )
        {
            mbDlgInfo( "%s\n\nis in the database.\n", file_p->name_p );
        }
        else
        {
            // Sanity Check
            mbDlgDebug(( "Error: document count: %d\n", count ));
        }
        returnCode = TRUE;
    }

exit:

    mbFileListFreeElement( file_p );
    mbFree( buf_p );

    return returnCode;
}



