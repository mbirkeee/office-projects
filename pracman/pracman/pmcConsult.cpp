//---------------------------------------------------------------------------
// (c) 2008 Michael A. Bree
//---------------------------------------------------------------------------
// Consult utilities
//---------------------------------------------------------------------------

// Platform include files
#include <stdio.h>
#pragma hdrstop

// Library include files
#include "mbUtils.h"

// Project include files
#include "pmcGlobals.h"
#include "pmcUtils.h"
#include "pmcPatient.h"
#include "pmcTables.h"
#include "pmcConsult.h"
#include "PmcDocument.h"

#pragma package(smart_init)

//---------------------------------------------------------------------------

Int32u_t pmcConsultNew( Int32u_t patientId, Int32u_t type )
{
    // There is a bit a race condition here. We need to lock the patient
    // record while we are doing this.

    PmcDocument consult = PmcDocument();

    consult.typeSet( type );
    consult.patientIdSet( patientId );

    // Normally, we only want to save object when the OK button is pushed,
    // but in the case of consult documents, we only want one active per
    // patient at a time.  Thus create it right away.
    consult.save( );
    return consult.id( );
}

//---------------------------------------------------------------------------
// Get the id of the most recent consult letter belonging to this patient
//---------------------------------------------------------------------------
Int32u_t pmcConsultFindLast( Int32u_t patientId )
{
    MbString    cmd;
    MbSQL       sql;
    Int32u_t    id = 0;

    if( patientId == 0 ) return 0;

    mbSprintf( &cmd, "select %s from %s where "
                     "%s=%lu and (%s=%lu or %s=%lu) and %s=%lu order by %s desc"

                   ,PMC_SQL_FIELD_ID

                   ,PMC_SQL_TABLE_DOCUMENTS

                   ,PMC_SQL_FIELD_PATIENT_ID        ,patientId
                   ,PMC_SQL_FIELD_TYPE              ,PMC_DOCUMENT_TYPE_CONSLT
                   ,PMC_SQL_FIELD_TYPE              ,PMC_DOCUMENT_TYPE_FOLLOWUP
                   ,PMC_SQL_FIELD_NOT_DELETED       ,PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE
                   ,PMC_SQL_FIELD_ID );

    sql.Query( cmd.get() );

    while( sql.RowGet() )
    {
        id = sql.Int32u( 0 );
        break;
    }
    return id;
}

//---------------------------------------------------------------------------
// Get the id of the most recent filed consult letter belonging to this
// patient.  Letter can be .DOC or .PDF, and is identified by finding the
// string "Letter Consult" in the original file name.
//
// TODO: Will probably have to tweak this when the PDF consults are working
//---------------------------------------------------------------------------
Int32u_t pmcConsultFindLastFiled( Int32u_t patientId )
{
    MbString    cmd;
    MbSQL       sql;
    Int32u_t    id = 0;
    Int32u_t    gotStatus;
    Int32u_t    gotType;

    if( patientId == 0 ) return 0;

    // Get all documents belonging to this patient
    mbSprintf( &cmd, "select %s,%s,%s,%s from %s where "
                     "%s=%lu and %s=%lu order by %s desc"

                   ,PMC_SQL_FIELD_ID
                   ,PMC_SQL_DOCUMENTS_FIELD_STATUS
                   ,PMC_SQL_FIELD_TYPE
                   ,PMC_SQL_DOCUMENTS_FIELD_ORIG_NAME

                   ,PMC_SQL_TABLE_DOCUMENTS
                   ,PMC_SQL_FIELD_PATIENT_ID        ,patientId
                   ,PMC_SQL_FIELD_NOT_DELETED       ,PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE
                   ,PMC_SQL_FIELD_ID );

    sql.Query( cmd.get() );

    while( sql.RowGet() )
    {
        gotStatus = sql.Int32u( 1 );
        gotType   = sql.Int32u( 2 );

        // Only consider document if it is filed
        if( gotStatus != PMC_DOCUMENT_STATUS_FILED ) continue;

        // Because the results are sorted, the first consult letter we find
        // is the most recent.

        if( gotType == PMC_DOCUMENT_TYPE_WORD || gotType == PMC_DOCUMENT_TYPE_PDF_FIXED )
        {
            if(    mbStrPosIgnoreCase( sql.String( 3 ), PMC_CONSULT_LETTER_FILENAME_TAG_WORD ) > 0
                || mbStrPosIgnoreCase( sql.String( 3 ), PMC_FOLLOWUP_LETTER_FILENAME_TAG_PDF  ) > 0
                || mbStrPosIgnoreCase( sql.String( 3 ), PMC_CONSLT_LETTER_FILENAME_TAG_PDF  ) > 0 )
            {
                id = sql.Int32u( 0 );
                break;
            }
        }
    }

    return id;
}
