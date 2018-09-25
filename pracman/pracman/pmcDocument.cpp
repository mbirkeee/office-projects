//---------------------------------------------------------------------------
// (c) 2008 Michael A. Bree
//---------------------------------------------------------------------------
// Document Object
//---------------------------------------------------------------------------

// Platform include files
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <dir.h>
#include <process.h>
#include <vcl.h>
#pragma hdrstop

// Library include files
#include "mbUtils.h"

// Project include files
#include "pmcGlobals.h"
#include "pmcUtils.h"
#include "pmcUtilsSql.h"
#include "pmcPatient.h"
#include "pmcPatientRecord.h"
#include "pmcPatientEditForm.h"
#include "pmcBatchImportForm.h"
#include "pmcTables.h"
#include "pmcDocumentUtils.h"
#include "pmcDocument.h"
#include "pmcDocumentEditForm.h"
#include "pmcConsult.h"

#pragma package(smart_init)

//---------------------------------------------------------------------------
// Default Constructor
//---------------------------------------------------------------------------
__fastcall PmcDocument::PmcDocument( void )
{
    this->init( );
}

//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------
__fastcall PmcDocument::PmcDocument( Int32u_t id )
{
    this->init( );
    this->idSet( id );
}

//---------------------------------------------------------------------------
// Constructor helper function
//---------------------------------------------------------------------------
Int32s_t __fastcall PmcDocument::init( void )
{
    mbObjectCountInc();

    this->myId = 0;
    this->lockedFlag = FALSE;
    this->alreadyLockedFlag = 0;

    for( int i = 0 ; i < PMC_DOCUMENT_STRING_COUNT ; i++ )
    {
        this->stringIn_p[i] = new MbString( );
        this->string_p[i] = new MbString( );
    }

    for( int i = 0 ; i < PMC_DOCUMENT_INT_COUNT ; i++ )
    {
        this->valIntIn[i] = 0;
        this->valInt[i] = 0;
    }
    return MB_RET_OK;
}

#if 0
//---------------------------------------------------------------------------
// Create a new document record in the database
//---------------------------------------------------------------------------

Int32s_t __fastcall PmcDocument::create( Int32u_t type )
{
    Int32s_t    result = MB_RET_ERR;

    if( this->myId != 0 )
    {
        mbDlgError( "Error: document create failed: Document object already has id %lu", this->myId );
        goto exit;
    }

    // Create new database record and get ID
    if( ( id = pmcSqlRecordCreate( PMC_SQL_TABLE_DOCUMENTS, NIL ) ) == 0 )
    {
        mbDlgError( "Error creating database record." );
        goto exit;
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

}
#endif

//---------------------------------------------------------------------------
// Set the document id; and get associated details.
//---------------------------------------------------------------------------
Int32s_t __fastcall PmcDocument::idSet( Int32u_t id )
{
    MbString        cmd;
    MbSQL           sql;
    Ints_t          i;
    Int32s_t        result = MB_RET_ERR;
    MbDateTime      dateTime;

    this->myId = id;

    // Format the command
    mbSprintf( &cmd, "select * from %s where %s=%ld",
            PMC_SQL_TABLE_DOCUMENTS,
            PMC_SQL_FIELD_ID, id );

    // Issue the SQL command to get the document details
    sql.Query( cmd.get() );

    if( sql.RowCount() != 1 )
    {
        mbDlgError( "Error retrieving document details from the database" );
        goto exit;
    }

    while( sql.RowGet() )
    {
        // Read the strings from the database
        for( i = 0 ; i < PMC_DOCUMENT_STRING_COUNT ; i++ )
        {
            this->stringIn_p[i]->set( sql.NmString( this->dbStr( i ) )  );
        }

        // Read the ints from the database
        for( i = 0 ; i < PMC_DOCUMENT_INT_COUNT ; i++ )
        {
            this->valIntIn[i] = sql.NmInt32u( this->dbInt( i ) );
        }

        // Get the lock value
        // 20090412:  Actually, should not set the lockedFlag here.  The lockedFlag
        // only indicates if *this* object has the lock, *not* if the document
        // is locked by another object
        // this->lockedFlag = sql.NmInt32u( PMC_SQL_FIELD_LOCK ) == 0 ? FALSE : TRUE;
        this->alreadyLockedFlag = sql.NmInt32u( PMC_SQL_FIELD_LOCK ) == 0 ? FALSE : TRUE;

        // Get created time
        dateTime.SetDateTime64( sql.NmDateTimeInt64u( PMC_SQL_FIELD_CREATED ) );
        this->createdDateInt = dateTime.Date();
        this->createdTimeInt = dateTime.Time();

        // Get modified time
        dateTime.SetDateTime64( sql.NmDateTimeInt64u( PMC_SQL_FIELD_MODIFIED ) );
        this->modifiedDateInt = dateTime.Date();
        this->modifiedTimeInt = dateTime.Time();
    }

    // Now copy the string In values to the string values
    for( i = 0 ; i < PMC_DOCUMENT_STRING_COUNT ; i++ )
    {
        this->string_p[i]->set( this->stringIn_p[i]->get( ) );
    }

    for( i = 0 ; i < PMC_DOCUMENT_INT_COUNT ; i++ )
    {
        this->valInt[i] = this->valIntIn[i];
    }

    result = MB_RET_OK;
exit:
    return result;
}

//---------------------------------------------------------------------------
// Save the document object to the database
//---------------------------------------------------------------------------

Int32s_t __fastcall PmcDocument::save( void )
{
    Ints_t  i;

    // If the document ID is 0, then this must be a new record
    if( this->myId == 0 )
    {
        this->myId = pmcSqlRecordCreate( PMC_SQL_TABLE_DOCUMENTS, NIL );
        if( this->myId == 0 )
        {
            mbDlgDebug(( "Error creating document record" ));
            goto exit;
        }
        mbLog( "Created new document record with id %ld\n", this->myId );

        // Lock the new record
        pmcSqlRecordLock( PMC_SQL_TABLE_DOCUMENTS, this->myId, FALSE );

        // Undelete makes it appear in the document list
        pmcSqlRecordUndelete( PMC_SQL_TABLE_DOCUMENTS, this->myId );

        this->lockedFlag = TRUE;
    }

    mbLog( "Update document %lu start...\n", this->id() );

    // Loop through the strings, saving them to the database
    for( i = 0 ; i < PMC_DOCUMENT_STRING_COUNT ; i++ )
    {
        if( strcmp( this->stringIn_p[i]->get(), this->string_p[i]->get() ) != 0 )
        {
            // First, make the change in the database
            pmcSqlExecString( PMC_SQL_TABLE_DOCUMENTS,
                              this->dbStr(i),
                              this->string_p[i]->get(),
                              this->myId);

            // Next record the change internally
            this->stringIn_p[i]->set( this->string_p[i]->get() );
        }
    }

    // Loop through the ints, saving them to the database
    for( i = 0 ; i < PMC_DOCUMENT_INT_COUNT ; i++ )
    {
        if( this->valInt[i] != this->valIntIn[i] )
        {
            // First, make the change in the database
            pmcSqlExecInt( PMC_SQL_TABLE_DOCUMENTS,
                           this->dbInt(i),
                           this->valInt[i],
                           this->myId );

            // Next record the change internally
            this->valIntIn[i] = this->valInt[i];
        }
    }

    mbLog( "Update document %lu complete\n", this->id() );

exit:
    return TRUE;

}

//---------------------------------------------------------------------------
// Detect any changes in the document details
//---------------------------------------------------------------------------
Int32s_t __fastcall PmcDocument::changed( void )
{
    Ints_t      i;
    Int32s_t    change = FALSE;

    for( i = 0 ; i < PMC_DOCUMENT_STRING_COUNT ; i++ )
    {
        if( strcmp( this->stringIn_p[i]->get(), this->string_p[i]->get() ) != 0 )
        {
            change = TRUE;
            mbLog( "Document string %d changed from '%s' to '%s'", i, this->stringIn_p[i]->get(), this->string_p[i]->get() );
            goto exit;
        }
    }

    for( i = 0 ; i < PMC_DOCUMENT_INT_COUNT ; i++ )
    {
        if( this->valInt[i] != this->valIntIn[i] )
        {
            change = TRUE;
            mbLog( "Document int %d changed from %u to %u", i, this->valIntIn[i], this->valInt[i] );
            goto exit;
        }
    }

exit:
    return change;
}

//---------------------------------------------------------------------------
// Edit document
//---------------------------------------------------------------------------
Int32s_t __fastcall PmcDocument::edit( void )
{
    Char_t  fileName[MAX_PATH];

    // If the document is a consult, don't bother checking status
    if( this->type( ) == PMC_DOCUMENT_TYPE_CONSLT ||
        this->type( ) == PMC_DOCUMENT_TYPE_FOLLOWUP )
    {
        pmcPatEditInfo_t        patEditInfo;
        TPatientEditForm       *patEditForm;

        patEditInfo.patientId = this->patientId();
        patEditInfo.mode = PMC_EDIT_MODE_EDIT;
        sprintf( patEditInfo.caption, "Edit Patient Details" );

        patEditForm = new TPatientEditForm( NULL, &patEditInfo );
        patEditForm->ShowModal();
        delete patEditForm;
    }
    else if( this->type( ) == PMC_DOCUMENT_TYPE_WORD )
    {
        if( this->lock( FALSE ) != MB_RET_OK )
        {
            mbDlgInfo( "Document locked for editing by another user, opening in view-only mode." );
            this->view( );
        }

        if( this->status( ) != PMC_DOCUMENT_STATUS_ACTIVE )
        {
            if( this->status( ) != PMC_DOCUMENT_STATUS_ACTIVE )
            {
                if( mbDlgYesNo( "This document is not active.  To edit this document, it must be made active.\n"
                                "Do you want to make it active?" ) == MB_BUTTON_NO )
                {
                    mbDlgInfo( "Opening document in view-only mode." );
                    this->view( );
                }
            }
        }

        sprintf( fileName, "\"%s\\%s\"", pmcCfg[CFG_WORD_CREATE_DIR].str_p, this->nameOrig() );

        // Execute word
        spawnle( P_NOWAITO,
             pmcCfg[CFG_WORD].str_p,
             pmcCfg[CFG_WORD].str_p,
             "",
             fileName,
             NULL, NULL );
    }
    else
    {
        mbDlgInfo( "Don't know how to edit documents of type %d", this->type() );
    }
    return MB_RET_OK;
}

//---------------------------------------------------------------------------
// Import CONSULT type document
//
// Called by import
//---------------------------------------------------------------------------
Int32s_t __fastcall PmcDocument::importConsult( void )
{
    Int32s_t                result = MB_RET_ERR;
    Int32u_t                crc;
    Int32u_t                size;
    PmcPatient              patient = PmcPatient( this->patientId( ) );
    PmcPatientHistory       history = PmcPatientHistory( this->patientId( ) );
    MbString                source;
    MbString                target;
    MbString                target2;
    PmcSqlProvider_t        provider;
    Char_t                  buf[256];
    pmcDocumentEditInfo_p   editInfo_p;
    TDocumentEditForm      *form_p;
    mbFileListStruct_p      file_p = NIL;
    Char_p                  template_p = NIL;
    Char_p                  tag_p;

    mbCalloc( file_p, sizeof(mbFileListStruct_t) );
    mbMalloc( file_p->name_p,     256 );
    mbMalloc( file_p->fullName_p, 256 );

    mbCalloc( editInfo_p, sizeof(pmcDocumentEditInfo_t) );

    if( this->type() == PMC_DOCUMENT_TYPE_FOLLOWUP )
    {
        template_p = PMC_PDF_TEMPLATE_PAT_FOLLOWUP;
        tag_p = PMC_FOLLOWUP_LETTER_FILENAME_TAG_PDF;
    }
    else
    {
        template_p = PMC_PDF_TEMPLATE_PAT_CONSLT;
        tag_p = PMC_CONSLT_LETTER_FILENAME_TAG_PDF;
    }

    // The following function actually produces the PDF file from the database contents.
    if( pmcPatDocumentPDF( &patient, history.subQueue(), NIL, template_p, this, FALSE, &source ) != MB_RET_OK )
    {
        mbDlgError( "Error generating PDF document ID: %lu", this->id());
        goto exit;
    }

    // Format a new "PDF" filename for this document, such that it
    // becomes the "original" file name for the document, for viewing,
    // extracting, etc.

    target.set( pmcCfg[CFG_TEMP_DIR].str_p );
    target.append( "\\" );

    if( pmcSqlProviderDetailsGet( patient.providerId(), &provider ) == TRUE )
    {
        target.append( provider.lastName );
    }
    else
    {
        target.append( "UNKNOWN" );
    }

    // Construct a filename for this document (up till now, it had none)
    sprintf( buf, "%s", patient.lastName() );
    mbStrAlphaNumericOnly( buf );
    target.append( "-" );
    target.append( buf );
    // The document ID alone should be enough to make this a unique filename
    sprintf( buf, "-%s-%lu", tag_p, this->id() );
    target.append( buf );
    target.append( ".pdf" );

    if( mbFileCopy( source.get(), target.get() ) != MB_RET_OK )
    {
        mbDlgError( "Error copying file '%s' for import.", target.get() );
        goto exit;
    }
    unlink( source.get() );

    editInfo_p->createdDate = this->createdDateInt;
    editInfo_p->createdTime = this->createdTimeInt;

    editInfo_p->modifiedDate = this->modifiedDateInt;
    editInfo_p->modifiedTime = this->modifiedTimeInt;

    editInfo_p->patientId = this->patientId();
    editInfo_p->providerId = this->providerId();
    editInfo_p->id = this->id();
    
    editInfo_p->batchMode = FALSE;
    editInfo_p->file_p = file_p;

    // Put the document description into the edit record
    mbMallocStr( editInfo_p->description_p, this->description() );

    // Must set the file type
    file_p->type = PMC_DOCUMENT_TYPE_PDF_FIXED;

    mbMallocStr( file_p->fullName_p, target.get() );

    pmcFilePathAndNameGet( file_p->fullName_p, NIL, file_p->name_p );

    // mbDlgInfo("file name: %s full: %s", file_p->name_p, file_p->fullName_p );

    form_p = new TDocumentEditForm( NIL, editInfo_p );
    form_p->ShowModal( );
    delete form_p;

    if( editInfo_p->returnCode != MB_BUTTON_OK )
    {
        // Not going to import, must delete temp file
        unlink( target.get() );
        goto exit;
    }

    this->descriptionSet( editInfo_p->description_p );

    source.set( target.get() );

    mbSprintf( &target, "%08ld-%08ld.pmc", mbToday( ), this->id());
    mbSprintf( &target2, "%s\\%s\\%s",  pmcCfg[CFG_DOC_IMPORT_TO_DIR_NEW].str_p,
                                       pmcDocumentDirFromName( target.get() ),
                                       target.get() );

    // mbDlgInfo( "target file name: %s", target2.get() );

    if( mbFileCopy( source.get(), target2.get() ) != MB_RET_OK )
    {
        mbDlgError( "Failed to copy document %s to database", source.get() );
        goto exit;
    }

    // Update the path to the document (so that is can still be viewed)
    this->nameSet( target.get() );
    this->nameOrigSet( file_p->name_p );

    crc = mbCrcFile( source.get(), &size, NIL, NIL, NIL, NIL, NIL );
    this->typeSet( PMC_DOCUMENT_TYPE_PDF_FIXED );
    this->crcSet( crc );
    this->sizeSet( size );
    this->statusSet( PMC_DOCUMENT_STATUS_FILED );

    // Sucessfully copied document to the import directory.  Get rid of original file
    unlink( source.get() );

    this->save( );

    result = MB_RET_OK;

exit:

    mbFree( editInfo_p->description_p );
    mbFree( editInfo_p );
    mbFileListFreeElement( file_p );
    return result;
}

//---------------------------------------------------------------------------
// Import wrapper.  Call new import function for consult documents,
// legacy code for WORD documents.
//---------------------------------------------------------------------------

Int32s_t __fastcall PmcDocument::import( void )
{
    Int32s_t result = MB_RET_ERR;

    if(    this->type( ) == PMC_DOCUMENT_TYPE_CONSLT
        || this->type( ) == PMC_DOCUMENT_TYPE_FOLLOWUP )
    {
        result = this->importConsult( );

        mbLog( "Imported consult document ID: %lu desc: '%s'\n",
            this->id(), this->description() );
    }
    else
    {
        result = this->importLegacy( );
    }
    return result;
}

//---------------------------------------------------------------------------
// Import the document (ugly... currently relies on legacy code)
//
// This method only works for existing WORD and CONSULT type documents.
//---------------------------------------------------------------------------
Int32s_t __fastcall PmcDocument::importLegacy( void )
{
    Int32s_t                result = MB_RET_ERR;
    mbFileListStruct_p      file_p = NIL;
    Int32u_t                docId;

    mbLog( "Want to import legacy document ID: %lu\n", this->id() );

    // Need to allocate a file list struct for the legacy code
    mbCalloc( file_p, sizeof(mbFileListStruct_t) );
    mbMalloc( file_p->name_p,     256 );
    mbMalloc( file_p->fullName_p, 256 );

    if( this->type( ) == PMC_DOCUMENT_TYPE_WORD )
    {
        file_p->type = PMC_DOCUMENT_TYPE_WORD;
        strcpy( file_p->fullName_p, this->nameOrig( ) );
    }
    else
    {
        mbDlgInfo( "Don't know how to import this document" );
        goto exit;
    }

    pmcFilePathAndNameGet( file_p->fullName_p, NIL, file_p->name_p );

    // Need to pass in a pointer so that ID can be returned for new documents
    docId = this->id( );

    // OK, here goes.... (man this legacy code is ugly... and its in a form too!)
    result = pmcDocProcess
    (
        file_p,                         // File details
        &docId,                         // document id - (0 = NEW)
        PMC_IMPORT_DIALOG_ALWAYS,       // Show dialog or not
        PMC_IMPORT_PROVIDER_SELECTED,
        PMC_IMPORT_PHN_NONE,
        PMC_IMPORT_DATE_SELECTED,
        TRUE,                           // TRUE = moved failed to fail dir
        this->providerId( ),
        this->patientId( ),
        this->date( ), 0,               // date, failMask
        NIL,                            // result flag - terminate auto import
        NIL,                            // result flag - form displayed
        NIL,                            // Failed directory
        NIL,                            // description
        TRUE
    );

exit:

    mbFileListFreeElement( file_p );

    return result;
}

//---------------------------------------------------------------------------
// View the document (a little ugly, currently relies on legacy view code)
//---------------------------------------------------------------------------
Int32s_t __fastcall PmcDocument::view( void )
{
    if(    this->type( ) == PMC_DOCUMENT_TYPE_CONSLT
        || this->type( ) == PMC_DOCUMENT_TYPE_FOLLOWUP )
    {
        Char_t  watermark[256];
        Char_p  template_p = PMC_PDF_TEMPLATE_PAT_CONSLT;

        // Get patient objects
        PmcPatient          patient = PmcPatient( this->patientId( ) );
        PmcPatientHistory   history = PmcPatientHistory( this->patientId( ) );

        watermark[0] = 0;
        if( this->status() == PMC_DOCUMENT_STATUS_PENDING )
        {
            sprintf( watermark, "Pending" );
        }
        else if( this->status() == PMC_DOCUMENT_STATUS_ACTIVE )
        {
            sprintf( watermark, "Active" );
        }

        if( this->type( ) == PMC_DOCUMENT_TYPE_FOLLOWUP ) template_p = PMC_PDF_TEMPLATE_PAT_FOLLOWUP;

        pmcPatDocumentPDF( &patient, history.subQueue(), watermark, template_p, this, TRUE, NIL );
    }
    else
    {
        mbFileListStruct_t          file;

        // Must pass the docment details into the viewer function via mbFileListStruct_t struct
        mbMalloc( file.fullName_p, 1024 );
        mbMalloc( file.name_p, 1024 );

        if( this->status() == PMC_DOCUMENT_STATUS_ACTIVE || this->status() == PMC_DOCUMENT_STATUS_PENDING )
        {
            sprintf( file.fullName_p, "%s\\%s", pmcCfg[CFG_WORD_CREATE_DIR].str_p, this->nameOrig() );
            sprintf( file.name_p, "%s", this->nameOrig() );
            file.type = this->type();
        }
        else
        {
            sprintf( file.fullName_p, "%s\\%s\\%s", pmcCfg[CFG_DOC_IMPORT_TO_DIR_NEW].str_p,
                 pmcDocumentDirFromName( this->name() ),  this->name() );
            sprintf( file.name_p, "%s", this->name() );
            file.type = this->type();
        }

        pmcDocumentView( &file, TRUE );
        mbFree( file.fullName_p );
        mbFree( file.name_p );
    }
    return MB_RET_OK;
}

//---------------------------------------------------------------------------
// Lock the record
//---------------------------------------------------------------------------

Int32s_t __fastcall PmcDocument::unlock( void )
{
    if( this->alreadyLockedFlag == TRUE )
    {
        if( mbDlgYesNo( "Document locked for editing by another user.\n\n"
                        "To unlock this document, click Yes.\n"
                        "To leave this document locked, click No.\n\n"
                        "WARNING:\n\nUnlocking a locked document can cause data corruption.\n"
                        "Unlock only if you're sure it is safe to do so." ) == MB_BUTTON_YES )
        {
            pmcSqlRecordUnlockForce( PMC_SQL_TABLE_DOCUMENTS, this->id( ) );
            return MB_RET_OK;
        }
    }
    else
    {
        mbDlgInfo( "Document is not locked." );
    }
    return MB_RET_ERROR;
}

//---------------------------------------------------------------------------
// Lock the record
//---------------------------------------------------------------------------
Int32s_t __fastcall PmcDocument::lock( Boolean_t forceFlag )
{
    Int32s_t    result = MB_RET_ERR;

    if( this->alreadyLockedFlag == TRUE )
    {
        mbDlgInfo( "Unable to lock document %lu (already locked)", this->id() );
        return MB_RET_ERROR;
    }

    if( this->lockedFlag == TRUE ) return MB_RET_OK;

    if( pmcSqlRecordLock( PMC_SQL_TABLE_DOCUMENTS, this->id(), TRUE ) == FALSE )
    {
        if( forceFlag == TRUE )
        {
            if( mbDlgYesNo( "Document locked for editing by another user.\n\n"
                            "To unlock this document, click Yes.\n"
                            "To leave this document locked, click No.\n\n"
                            "WARNING:\n\nUnlocking a locked document can cause data corruption.\n"
                            "Unlock only if you're sure it is safe to do so." ) == MB_BUTTON_YES )
            {
                pmcSqlRecordUnlockForce( PMC_SQL_TABLE_DOCUMENTS, this->id( ) );
                if( pmcSqlRecordLock( PMC_SQL_TABLE_DOCUMENTS, this->id( ), TRUE ) == TRUE )
                {
                    result = MB_RET_OK;
                }
                else
                {
                    mbDlgExclaim( "Document record unlock failed." );
                }
            }
        }
    }
    else
    {
        result = MB_RET_OK;
    }

    if( result == MB_RET_OK )
    {
        mbLog( "Locked document record id: %lu\n", this->id( ) );
        this->lockedFlag = TRUE;
    }
    return result;
}

//---------------------------------------------------------------------------
// Extract the document
//
// - Does the document need to be locked?  I don't think so.
//---------------------------------------------------------------------------
Int32s_t __fastcall PmcDocument::extract( void  )
{
    MbString        source;
    MbString        target;
    Boolean_t       gotFileName = FALSE;
    Int32s_t        result = MB_RET_ERR;
    TSaveDialog    *saveDialog_p = new TSaveDialog( NIL );

    if( this->type() == PMC_DOCUMENT_TYPE_CONSLT )
    {
        mbDlgInfo( "Can't extract documents of type 'Consult'." );
        goto exit;
    }

    if( this->type() == PMC_DOCUMENT_TYPE_FOLLOWUP )
    {
        mbDlgInfo( "Can't extract documents of type 'Followup'." );
        goto exit;
    }

    if( this->status() == PMC_DOCUMENT_STATUS_FILED )
    {
        mbSprintf( &source, "%s\\%s\\%s", pmcCfg[CFG_DOC_IMPORT_TO_DIR_NEW].str_p,
                pmcDocumentDirFromName( this->name( ) ), this->name( ) );

        gotFileName = TRUE;
    }
    else if( this->status() == PMC_DOCUMENT_STATUS_ACTIVE )
    {
        mbSprintf( &source, "%s\\%s", pmcCfg[CFG_WORD_CREATE_DIR].str_p, this->nameOrig( ) );
        gotFileName = TRUE;
    }

    if( gotFileName )
    {
        saveDialog_p->Title = "Save extracted file...";
        saveDialog_p->InitialDir = ( pmcCfg[CFG_DOC_EXTRACT_TO_DIR].str_p ) ? pmcCfg[CFG_DOC_EXTRACT_TO_DIR].str_p : "" ;
        saveDialog_p->Filter = "All Files (*.*)|*.*";
        saveDialog_p->FileName = this->nameOrig( );

        if( saveDialog_p->Execute() == TRUE )
        {
            mbSprintf( &target, "%s", saveDialog_p->FileName );

            if( mbFileCopy( source.get(), target.get() ) != MB_RET_OK )
            {
                mbDlgError( "File copy error while extracting file %s from database", this->nameOrig( ) );
            }
            else
            {
                mbLog( "extracted document (id: %ld) from database\n", this->id( ) );
                mbDlgInfo( "Document successfully extracted to:\n\n%s", target.get( ) );
                result = MB_RET_OK;
            }
        }
    }
exit:
    delete saveDialog_p;

    return result;
}

//---------------------------------------------------------------------------
// Delete the document
//---------------------------------------------------------------------------
Int32s_t __fastcall PmcDocument::del( void  )
{
    Int32s_t    result = MB_RET_ERR;
    Int32s_t    button;
    MbString    source;
    MbString    target;

    if( this->lock( FALSE ) != MB_RET_OK )
    {
        mbDlgInfo( "Document is locked for editing by another user.  Cannot delete at this time." );
        goto exit;
    }

    if( strlen( this->nameOrig() ) > 0 )
    {
        button = mbDlgYesNo( "Are you sure you want to delete the following file:\n\n%s?", this->nameOrig() );
    }
    else
    {
        button = mbDlgYesNo( "Are you sure you want to delete this document?" );
    }

    if( button == MB_BUTTON_NO ) goto exit;

    if( this->status() == PMC_DOCUMENT_STATUS_ACTIVE || this->status() == PMC_DOCUMENT_STATUS_PENDING )
    {
        if( this->nameOrigExists() )
        {
            // In the case of deleting active documents, move the original
            // file to the "delete" directory; otherwise, no files are touched
            mbSprintf( &source, "%s\\%s", pmcCfg[CFG_WORD_CREATE_DIR].str_p, this->nameOrig( ) );
            mbSprintf( &target, "%s\\%s", pmcCfg[CFG_DOC_DELETE_DIR].str_p, this->nameOrig( ) );
            mbLog( "deleting active file id: %ld, moving source: %s to target: %s\n", this->id(), source.get(), target.get() );

            if( mbFileCopy( source.get(), target.get() ) != MB_RET_OK )
            {
                mbDlgError( "File copy error while deleting file %s, delete terminated.", this->nameOrig( ) );
                goto exit;
            }
            else
            {
                unlink( source.get() );
                mbLog( "moved document (id: %ld) to %s\n", this->id(), source.get() );
            }
        }
    }

    mbLog( "deleted document id: %ld, status: %ld, type: %ld\n", this->id(), this->status(), this->type() );
    pmcSqlRecordDelete( PMC_SQL_TABLE_DOCUMENTS, this->id() );

    result = MB_RET_OK;

exit:
    return result;
}

//---------------------------------------------------------------------------
// Check if original name is set
//---------------------------------------------------------------------------
Boolean_t __fastcall PmcDocument::nameOrigExists( void  )
{
    if( this->nameOrig() == NIL ) return FALSE;

    if( strlen( this->nameOrig() ) == 0 ) return FALSE;

    return TRUE;
}

//---------------------------------------------------------------------------
// Set the description string
//---------------------------------------------------------------------------
Int32s_t __fastcall PmcDocument::descriptionSet( Char_p str_p )
{
    this->string_p[PMC_DOCUMENT_STRING_DESCRIPTION]->set( str_p );
    this->string_p[PMC_DOCUMENT_STRING_DESCRIPTION]->clean( );
    return TRUE;
}

//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------
__fastcall PmcDocument::~PmcDocument( void )
{
    Ints_t i;

    for( i = 0 ; i < PMC_DOCUMENT_STRING_COUNT ; i++ )
    {
        delete this->stringIn_p[i];
        delete this->string_p[i];
    }

    if( this->lockedFlag == TRUE )
    {
        mbLog( "Unlocking document object that is going out of scope\n" );
        pmcSqlRecordUnlock( PMC_SQL_TABLE_DOCUMENTS, this->id() );
    }

    mbObjectCountDec( );
}


