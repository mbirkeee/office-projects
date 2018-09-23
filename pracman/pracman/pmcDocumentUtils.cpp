//---------------------------------------------------------------------------
// Document utility functions
//---------------------------------------------------------------------------
// (c) 2008 Michael A. Bree
//---------------------------------------------------------------------------

// Platform includes
#include <stdio.h>
#include <vcl.h>
#include <time.h>
#include <dos.h>
#include <dir.h>
#include <process.h>
#include <Filectrl.hpp>
#pragma hdrstop

// Library include files
#include "mbUtils.h"

// Program include files
#include "pmcUtils.h"
#include "pmcInitialize.h"
#include "pmcGlobals.h"
#include "pmcTables.h"
#include "pmcDocumentUtils.h"
#include "pmcDocument.h"

// TODO: Shouldn't this be in an include somewhere?
extern  pmcTableStatusUpdate( Char_p name_p );

Int64u_t    gDocumentsLastReadTime;

//---------------------------------------------------------------------------
// Return Document Status String
//---------------------------------------------------------------------------

Char_p pmcDocumentStatusString( Int32u_t status )
{
    if( status == PMC_DOCUMENT_STATUS_FILED )    return "Filed";
    if( status == PMC_DOCUMENT_STATUS_PENDING )  return "Pending";
    if( status == PMC_DOCUMENT_STATUS_ACTIVE )   return "Active";
    if( status == PMC_DOCUMENT_STATUS_NEW  )     return "New";
    return "Unknown";
}

//---------------------------------------------------------------------------
// Return Document Status Image Index
//---------------------------------------------------------------------------

Int32s_t pmcDocumentStatusImageIndex( Int32u_t status )
{
    if( status == PMC_DOCUMENT_STATUS_FILED )    return PMC_COLORSQUARE_INDEX_GREEN;
    if( status == PMC_DOCUMENT_STATUS_PENDING )  return PMC_COLORSQUARE_INDEX_BLUE;
    if( status == PMC_DOCUMENT_STATUS_ACTIVE )   return PMC_COLORSQUARE_INDEX_YELLOW;
    return PMC_COLORSQUARE_INDEX_RED;
}


#if 0
//---------------------------------------------------------------------------
// View the specified document
//---------------------------------------------------------------------------
Int32s_t pmcDocumentIdView( Int32u_t id )
{
    return TRUE;
}
#endif

//---------------------------------------------------------------------------
// View the specified document
//
// If the temp flag is set, the file is copied into the temp dir before
// viewing.
//---------------------------------------------------------------------------
Int32s_t pmcDocumentView
(
    mbFileListStruct_p      file_p,
    Int32u_t                tempFlag
)
{
    Int32s_t        returnCode = FALSE;
    bool            noViewer = TRUE;
    MbCursor        cursor = MbCursor( crHourGlass );
    MbString        name;
    MbString        buf;

    if( !FileExists( file_p->fullName_p ) )
    {
        Ints_t  len = 0;

        if( file_p->name_p )
        {
            len = strlen( file_p->name_p );
        }

        if( len )
        {
            mbDlgExclaim( "Cannot access document: %s\n", file_p->fullName_p );
        }
        else
        {
             mbDlgExclaim( "Cannot access selected document.\n" );
        }

        noViewer = FALSE;
        goto exit;
    }

    if( tempFlag == TRUE )
    {
        pmcMakeFileName( pmcCfg[CFG_TEMP_DIR].str_p, name.get() );
        if( file_p->type > PMC_DOCUMENT_TYPE_ANY && file_p->type < PMC_DOCUMENT_TYPE_INVALID )
        {
            name.append( pmcDocumentTypeExtStrings[ file_p->type ] );
        }

        // Copying file into temp dir
        if( mbFileCopy( file_p->fullName_p, name.get() ) != MB_RET_OK ) goto exit;
    }
    else
    {
        strcpy( name.get(), file_p->fullName_p );
    }

    switch( file_p->type )
    {
        case PMC_DOCUMENT_TYPE_WORD:
            mbSprintf( &buf, "\"%s\"", name.get() );
            if( pmcCfg[CFG_VIEWER_WORD].str_p )
            {
                noViewer = FALSE;
                spawnle( P_NOWAITO,
                    pmcCfg[CFG_VIEWER_WORD].str_p,
                    pmcCfg[CFG_VIEWER_WORD].str_p,
                    buf.get(),
                    NULL, NULL );
            }
            break;

        case PMC_DOCUMENT_TYPE_TXT:
            mbSprintf( &buf, "\"%s\"", name.get() );
            if( pmcCfg[CFG_VIEWER_TXT].str_p )
            {
                noViewer = FALSE;
                spawnle( P_NOWAITO,
                    pmcCfg[CFG_VIEWER_TXT].str_p,
                    pmcCfg[CFG_VIEWER_TXT].str_p,
                    buf.get(),
                    NULL, NULL );
            }
            break;

        case PMC_DOCUMENT_TYPE_PDF:
        case PMC_DOCUMENT_TYPE_PDF_FIXED:
            mbSprintf( &buf, "\"%s\"", name.get() );
            if( pmcCfg[CFG_VIEWER_PDF].str_p )
            {
                noViewer = FALSE;
                spawnle( P_NOWAITO,
                    pmcCfg[CFG_VIEWER_PDF].str_p,
                    pmcCfg[CFG_VIEWER_PDF].str_p,
                    buf.get(),
                    NULL, NULL );
            }
            break;

        case PMC_DOCUMENT_TYPE_JPG:
            mbSprintf( &buf, "\"%s\"", name.get() );
            if( pmcCfg[CFG_VIEWER_JPG].str_p )
            {
                noViewer = FALSE;
                spawnle( P_NOWAITO,
                    pmcCfg[CFG_VIEWER_JPG].str_p,
                    pmcCfg[CFG_VIEWER_JPG].str_p,
                    buf.get(),
                    NULL, NULL );
            }
            break;

        case PMC_DOCUMENT_TYPE_PPT:
            mbSprintf( &buf, "\"%s\"", name.get() );
            if( pmcCfg[CFG_VIEWER_JPG].str_p )
            {
                noViewer = FALSE;
                spawnle( P_NOWAITO,
                    pmcCfg[CFG_VIEWER_PPT].str_p,
                    pmcCfg[CFG_VIEWER_PPT].str_p,
                    buf.get(),
                    NULL, NULL );
            }
            break;

        default:
            noViewer = TRUE;
            break;
    }

exit:

    if( noViewer )
    {
        mbDlgExclaim( "No viewer configured for documents of type '%s'  (*%s)\n",
            pmcDocumentTypeDescStrings[ file_p->type ],
            pmcDocumentTypeExtStrings[  file_p->type ] );
    }
    return returnCode;
}

//---------------------------------------------------------------------------
// Get array of pointers to document items.  CALLER MUST FREE!!!
//---------------------------------------------------------------------------
pmcDocumentItem_p   *pmcDocumentListGet( Int32u_p count_p )
{
    pmcDocumentItem_p  *doc_pp;
    pmcDocumentItem_p   doc_p;
    Int32u_t            index = 0;

    mbMalloc( doc_pp, sizeof( pmcDocumentItem_p ) * pmcDocument_q->size  );

    qWalk( doc_p, pmcDocument_q, pmcDocumentItem_p )
    {
        doc_pp[index++] = doc_p;
    }

    *count_p = pmcDocument_q->size;
    return doc_pp;
}

//---------------------------------------------------------------------------
// Updates the document list if the table has been modified since the last
// update time.
//---------------------------------------------------------------------------
Int32s_t    pmcDocumentListUpdate( void )
{
    pmcDocumentItem_p           doc_p, doc2_p;
    MbSQL                       sql;
    Int32s_t                    returnCode = FALSE;
    Char_t                      buf[256];
    MbDateTime                  dateTime;
    Int64u_t                    modifyTime;
    Boolean_t                   found;
    MbString                    cmd;

    pmcTableStatusUpdate( PMC_SQL_TABLE_DOCUMENTS );
    modifyTime = pmcPollTableModifyTime[PMC_TABLE_INDEX_DOCUMENTS];

    if( modifyTime > gDocumentsLastReadTime )
    {
//        mbDlgInfo( "table changed\n" );
    }
    else
    {
        return FALSE;
    }

    dateTime.SetDateTime64( gDocumentsLastReadTime );
    gDocumentsLastReadTime = modifyTime;
    dateTime.BackOneMinute();

    sprintf( buf, "%Lu", dateTime.Int64() );

    MbCursor cursor = MbCursor( crHourGlass );

    //                           0     1     2     3     4     5     6     7     8     9
    mbSprintf( &cmd, "select %s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,"
                            "%s.%s,%s.%s,%s.%s,%s.%s,%s.%s "
                     "from %s,%s where %s.%s>0 and %s.%s=%s.%s and %s.%s>%s order by %s.%s",

        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_DESC,                     // 0
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_NAME,                     // 1
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_DOCUMENTS_FIELD_ORIG_NAME,      // 2
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_SIZE,                     // 3
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_TYPE,                     // 4
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_PATIENT_ID,               // 5
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_PROVIDER_ID,              // 6
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_ID,                       // 7
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_DATE,                     // 8
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_DOCUMENTS_FIELD_STATUS,         // 9
        PMC_SQL_TABLE_PATIENTS,  PMC_SQL_FIELD_FIRST_NAME,               // 10
        PMC_SQL_TABLE_PATIENTS,  PMC_SQL_FIELD_LAST_NAME,                // 11
        PMC_SQL_TABLE_PATIENTS,  PMC_SQL_FIELD_TITLE,                    // 12
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_CREATED,                  // 13
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_NOT_DELETED,              // 14

        PMC_SQL_TABLE_DOCUMENTS,
        PMC_SQL_TABLE_PATIENTS,

        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_ID,
        PMC_SQL_TABLE_PATIENTS,  PMC_SQL_FIELD_ID,                  PMC_SQL_TABLE_DOCUMENTS,  PMC_SQL_FIELD_PATIENT_ID,

        PMC_SQL_TABLE_DOCUMENTS ,PMC_SQL_FIELD_MODIFIED     ,       buf,
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_ID

        );

    if( sql.Query( cmd.get() ) == FALSE ) goto exit;

    // mbDlgInfo( "found %d changed records\n", sql.RowCount( ) );

    while( sql.RowGet() )
    {
        // Comment
        mbCalloc( doc_p, sizeof( pmcDocumentItem_t ) );
        mbMallocStr( doc_p->description_p, sql.String( 0 ) );

        // File name
        mbMallocStr( doc_p->name_p, sql.String( 1 ) );

        // Original file name
        mbMallocStr( doc_p->origName_p, sql.String( 2 ) );

        doc_p->size         = sql.Int32u( 3 );
        doc_p->type         = sql.Int32u( 4 );
        doc_p->patientId    = sql.Int32u( 5 );
        doc_p->providerId   = sql.Int32u( 6 );
        doc_p->id           = sql.Int32u( 7 );
        doc_p->date         = sql.Int32u( 8 );
        doc_p->status       = sql.Int32u( 9 );
        doc_p->notDeleted   = sql.Int32u( 14 );

        if( doc_p->patientId )
        {
            sprintf( buf, "%s, %s (%s)",  sql.String( 11 ), sql.String( 10 ), sql.String( 12 ) );
            mbMallocStr( doc_p->patient_p, buf );
        }

        dateTime.SetDateTime64( sql.DateTimeInt64u( 13 ) );
        doc_p->createdDate = dateTime.Date();

        found = FALSE;
        qWalk( doc2_p, pmcDocument_q, pmcDocumentItem_p )
        {
            if( doc2_p->id == doc_p->id )
            {
                found = TRUE;
                break;
            }
        }

        if( found )
        {
            if( doc_p->notDeleted == PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE )
            {
                // Document was updated.... but we cannt change the entry
                // underfoot, as there could be multiple document list windows
                // open pointing to this entry.  Must therefore update
                // the existing record

                mbRemallocStr( doc2_p->name_p,        doc_p->name_p );
                mbRemallocStr( doc2_p->origName_p,    doc_p->origName_p );
                mbRemallocStr( doc2_p->description_p, doc_p->description_p );
                mbRemallocStr( doc2_p->patient_p,     doc_p->patient_p );

                doc2_p->providerId  = doc_p->providerId;
                doc2_p->patientId   = doc_p->patientId;
                doc2_p->type        = doc_p->type;
                doc2_p->size        = doc_p->size;
                doc2_p->id          = doc_p->id;
                doc2_p->date        = doc_p->date;
                doc2_p->status      = doc_p->status;
                doc2_p->createdDate = doc_p->createdDate;
                doc2_p->notDeleted  = doc_p->notDeleted;
            }
            else
            {
                // document was deleted
                qRemoveEntry( pmcDocument_q, doc2_p );
                qInsertLast( pmcDocumentDeleted_q, doc2_p );
            }
            pmcDocumentListFreeItem( doc_p );
        }
        else
        {
            if( doc_p->notDeleted == PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE )
            {
                qInsertFirst( pmcDocument_q, doc_p );
            }
            else
            {
                pmcDocumentListFreeItem( doc_p );
            }
        }
    }

    returnCode = TRUE;
exit:
   return returnCode;
}

//---------------------------------------------------------------------------

void pmcDocumentListFreeItem( pmcDocumentItem_p doc_p )
{
    if( doc_p )
    {
        mbFree( doc_p->description_p );
        mbFree( doc_p->name_p );
        mbFree( doc_p->origName_p );
        mbFree( doc_p->patient_p );
        mbFree( doc_p );
    }
}

//---------------------------------------------------------------------------

Int32s_t    pmcDocumentListInit( void )
{
    pmcDocumentItem_p           doc_p;
    MbSQL                       sql;
    Int32s_t                    returnCode = FALSE;
    MbDateTime                  dateTime;
    MbCursor                    cursor = MbCursor( crHourGlass );
    MbString                    cmd;

    pmcTableStatusUpdate( PMC_SQL_TABLE_DOCUMENTS );
    gDocumentsLastReadTime = pmcPollTableModifyTime[PMC_TABLE_INDEX_DOCUMENTS];

    //                           0     1     2     3     4     5     6     7     8     9
    mbSprintf( &cmd, "select %s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,"
                            "%s.%s,%s.%s,%s.%s,%s.%s,%s.%s "
                     "from %s,%s where %s.%s=%ld and %s.%s>0 and %s.%s=%s.%s order by %s.%s",

        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_DESC,                     // 0
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_NAME,                     // 1
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_DOCUMENTS_FIELD_ORIG_NAME,      // 2
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_SIZE,                     // 3
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_TYPE,                     // 4
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_PATIENT_ID,               // 5
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_PROVIDER_ID,              // 6
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_ID,                       // 7
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_DATE,                     // 8
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_DOCUMENTS_FIELD_STATUS,         // 9

        PMC_SQL_TABLE_PATIENTS,  PMC_SQL_FIELD_FIRST_NAME,               // 10
        PMC_SQL_TABLE_PATIENTS,  PMC_SQL_FIELD_LAST_NAME,                // 11
        PMC_SQL_TABLE_PATIENTS,  PMC_SQL_FIELD_TITLE,                    // 12
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_CREATED,                  // 13
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_NOT_DELETED,              // 14

        PMC_SQL_TABLE_DOCUMENTS,
        PMC_SQL_TABLE_PATIENTS,

        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_NOT_DELETED,         PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE,
        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_ID,
        PMC_SQL_TABLE_PATIENTS,  PMC_SQL_FIELD_ID,                  PMC_SQL_TABLE_DOCUMENTS,  PMC_SQL_FIELD_PATIENT_ID,

        PMC_SQL_TABLE_DOCUMENTS, PMC_SQL_FIELD_ID );

    if( sql.Query( cmd.get() ) == FALSE ) goto exit;

    while( sql.RowGet() )
    {
        // Comment
        mbCalloc( doc_p, sizeof( pmcDocumentItem_t ) );

        mbMallocStr( doc_p->description_p, sql.String( 0 ) );

        // File name
        mbMallocStr( doc_p->name_p, sql.String( 1 ) );

        // Original file name
        mbMallocStr( doc_p->origName_p, sql.String( 2 ) );

        doc_p->size         = sql.Int32u( 3 );
        doc_p->type         = sql.Int32u( 4 );
        doc_p->patientId    = sql.Int32u( 5 );
        doc_p->providerId   = sql.Int32u( 6 );
        doc_p->id           = sql.Int32u( 7 );
        doc_p->date         = sql.Int32u( 8 );
        doc_p->status       = sql.Int32u( 9 );
        doc_p->notDeleted   = sql.Int32u( 14 );

        if( doc_p->patientId )
        {
            mbSprintf( &cmd, "%s, %s (%s)",  sql.String( 11 ), sql.String( 10 ), sql.String( 12 ) );
            mbMallocStr( doc_p->patient_p, cmd.get() );
        }

        dateTime.SetDateTime64( sql.DateTimeInt64u( 13 ) );
        doc_p->createdDate = dateTime.Date();
        qInsertFirst( pmcDocument_q, doc_p );
    }

    returnCode = TRUE;
exit:

    if( returnCode == FALSE )
    {
        mbDlgDebug(( "Failed to read document list" ));
    }
    return returnCode;
}


