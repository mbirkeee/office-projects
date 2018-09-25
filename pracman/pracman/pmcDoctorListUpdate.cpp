//---------------------------------------------------------------------------
// Function: pmcDoctorListUpdate.cpp
//---------------------------------------------------------------------------
// Date: Feb. 15, 2001
//---------------------------------------------------------------------------
// Description:
//
// This file contains functions for updating the internal list of patients.
// The internal list is maintained to speed searching for patient records.
//---------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <vcl.h>
#pragma hdrstop

#include "mbUtils.h"

#include "pmcGlobals.h"
#include "pmcUtils.h"
#include "pmcMainForm.h"

#pragma package(smart_init)

//---------------------------------------------------------------------------
// Function: TMainForm::UpdateDoctorList
//---------------------------------------------------------------------------
// Description:
//
// Read modified records from SQL database and insert into (or delete from)
// internal list.
//---------------------------------------------------------------------------

void __fastcall TMainForm::UpdateDoctorList
(
    pmcTableStatus_p        tableStatus_p,
    Boolean_t               forceFlag
)
{
    TThermometer           *thermometer_p;
    Int32u_t                i;
    Int32u_t                records = 0;
    pmcDocRecordStruct_p    docRecord_p;
    Boolean_t               initialRead;
    Boolean_t               localFlag;
    Char_t                  area[8];
    pmcLinkageStruct_p      linkage_p;
    Char_p                  buf_p = NIL;
    Char_p                  phone_p = NIL;
    Char_p                  temp_p;
    MbSQL                   sql;

    if( UpdateDoctorListDoInit == FALSE ) return;

    if( !forceFlag )
    {
        if( tableStatus_p->newModifyTime == tableStatus_p->curModifyTime &&
            tableStatus_p->newDataSize == tableStatus_p->curDataSize )
        {
            return;
        }
    }

    mbMalloc( buf_p, 512 );
    mbMalloc( phone_p, 128 );

    nbDlgDebug(( "Must update doctor records: new: %Ld cur:%Ld last read: %Ld\n",
        tableStatus_p->newModifyTime,
        tableStatus_p->curModifyTime,
        tableStatus_p->lastReadTime  ));

    UpdateDoctorListDone =  FALSE;

    if( tableStatus_p->lastReadTime == 0i64 )
    {
        // Read all non deleted patient records
        nbDlgDebug(( "This is initial read of the table" ));
        initialRead = TRUE;

        sprintf( buf_p, "select %s,%s,%s,%s,%s,%s,%s from %s where %s=1 and %s>0 and %s>0",
            PMC_SQL_FIELD_FIRST_NAME,               // 0
            PMC_SQL_FIELD_LAST_NAME,                // 1
            PMC_SQL_FIELD_ID,                       // 2
            PMC_SQL_DOCTORS_FIELD_DOCTOR_NUMBER,    // 3
            PMC_SQL_DOCTORS_FIELD_WORK_PHONE,       // 4
            PMC_SQL_DOCTORS_FIELD_WORK_FAX,         // 5
            PMC_SQL_FIELD_NOT_DELETED,              // 6

            PMC_SQL_TABLE_DOCTORS,
            PMC_SQL_FIELD_NOT_DELETED,
            PMC_SQL_FIELD_MODIFIED,
            PMC_SQL_FIELD_ID );
    }
    else
    {
        MbDateTime dateTime = MbDateTime( tableStatus_p->lastReadTime );
        // Read all records (incl deleted) with modify time > lastReadTime
        initialRead = FALSE;
        dateTime.BackOneMinute();
        tableStatus_p->lastReadTime = dateTime.Int64();
        sprintf( phone_p, "%Lu", tableStatus_p->lastReadTime );

        sprintf( buf_p, "select %s,%s,%s,%s,%s,%s,%s from %s where %s>%s and %s > 0",
            PMC_SQL_FIELD_FIRST_NAME,               // 0
            PMC_SQL_FIELD_LAST_NAME,                // 1
            PMC_SQL_FIELD_ID,                       // 2
            PMC_SQL_DOCTORS_FIELD_DOCTOR_NUMBER,    // 3
            PMC_SQL_DOCTORS_FIELD_WORK_PHONE,       // 4
            PMC_SQL_DOCTORS_FIELD_WORK_FAX,         // 5
            PMC_SQL_FIELD_NOT_DELETED,              // 6

            PMC_SQL_TABLE_DOCTORS,
            PMC_SQL_FIELD_MODIFIED,  phone_p,
            PMC_SQL_FIELD_ID );
    }

    nbDlgDebug(( "SQL: '%s'", buf_p ));

    mbLockAcquire( pmcDocListLock );

    tableStatus_p->curModifyTime = tableStatus_p->newModifyTime;
    tableStatus_p->lastReadTime  = tableStatus_p->newModifyTime;
    tableStatus_p->curDataSize   = tableStatus_p->newDataSize;

    // Get record count
    if( initialRead == TRUE )
    {
        Char_t  tmp[32];

        // We will be reading the entire table, get count for thermometer
        records = pmcSqlSelectInt( PMC_SQL_CMD_DOCTORS_COUNT, NIL );
        sprintf( phone_p, "Processing %s doctor records...", mbStrInt32u( records, tmp ) );
        thermometer_p = new TThermometer( phone_p, 0, records, FALSE );
    }

    i = 0;

    if( sql.Query( buf_p ) == FALSE ) goto exit;

    while( sql.RowGet( ) )
    {
        if( initialRead ) thermometer_p->Set( i );

        // Allocate space for the patient record
        mbMalloc( docRecord_p, sizeof( pmcDocRecordStruct_t ) );
        memset( docRecord_p, 0,  sizeof( pmcDocRecordStruct_t ) );

        docRecord_p->magicNumber = PMC_DOC_RECORD_MAGIC_NUMBER;

        // Set pointers in linkage structs to start of this record
        docRecord_p->nameLinkage.record_p   = (Void_p)docRecord_p;
        docRecord_p->phoneLinkage.record_p  = (Void_p)docRecord_p;
        docRecord_p->numLinkage.record_p    = (Void_p)docRecord_p;
        docRecord_p->deleteLinkage.record_p = (Void_p)docRecord_p;

        // Get "deleted" indicator
        docRecord_p->notDeleted = sql.Int32u( 6 );

        // Get doctor Id
        docRecord_p->id = sql.Int32u( 2 );

        // Get first name
        temp_p = sql.String( 0 );
        docRecord_p->firstNameLen = strlen( temp_p ) + 1;
        mbMalloc( docRecord_p->firstName_p, docRecord_p->firstNameLen );
        strcpy( docRecord_p->firstName_p, temp_p );

        // Get doctor number
        docRecord_p->numInt32 = sql.Int32u( 3 );
        sprintf( buf_p, "%ld", docRecord_p->numInt32 );
        docRecord_p->numLen =  strlen( buf_p ) + 1;
        mbMalloc( docRecord_p->num_p, docRecord_p->numLen );
        strcpy( docRecord_p->num_p, buf_p );

        // Get last name
        temp_p = sql.String( 1 );
        docRecord_p->lastNameLen =  strlen( temp_p ) + 1;
        mbMalloc( docRecord_p->lastName_p, docRecord_p->lastNameLen );
        strcpy( docRecord_p->lastName_p, temp_p );

        // Last name (search version)
        docRecord_p->lastNameSearchLen = docRecord_p->lastNameLen;
        mbMalloc( docRecord_p->lastNameSearch_p, docRecord_p->lastNameSearchLen );
        strcpy( docRecord_p->lastNameSearch_p, temp_p );
        mbStrToLower( docRecord_p->lastNameSearch_p );

        // Get fax number
        strcpy( buf_p, sql.String( 5 ) );
        pmcPhoneFormat( buf_p, area, phone_p, &localFlag );
        docRecord_p->displayFaxAreaCode = ( localFlag == TRUE ) ? FALSE : TRUE;
        strncpy( docRecord_p->faxAreaCode, area, PMC_AREA_CODE_LEN );

        docRecord_p->workFaxLen = strlen( phone_p ) + 1;
        mbMalloc( docRecord_p->workFax_p, docRecord_p->workFaxLen );
        strcpy( docRecord_p->workFax_p, phone_p );

        // Get work phone
        strcpy( buf_p, sql.String( 4 ) );
        pmcPhoneFix( buf_p, NIL );
        docRecord_p->workPhoneSearchLen = strlen( buf_p ) + 4;
        mbMalloc( docRecord_p->workPhoneSearch_p, docRecord_p->workPhoneSearchLen );
        memset( docRecord_p->workPhoneSearch_p, 0,  docRecord_p->workPhoneSearchLen );
        strcpy( docRecord_p->workPhoneSearch_p, buf_p );

        pmcPhoneFormat( docRecord_p->workPhoneSearch_p, area, phone_p, &localFlag );

        nbDlgDebug(( "phone: '%s' area: '%s' phone: '%s' local: %d", docRecord_p->workPhoneSearch_p, area, phone_p, localFlag ));

        docRecord_p->displayWorkAreaCode = ( localFlag == TRUE ) ? FALSE : TRUE;
        strncpy( docRecord_p->workAreaCode, area, PMC_AREA_CODE_LEN );

        docRecord_p->workPhoneLen = strlen( phone_p ) + 1;
        mbMalloc( docRecord_p->workPhone_p, docRecord_p->workPhoneLen );
        strcpy( docRecord_p->workPhone_p, phone_p );

        // If the area code equals the local area code, set to 0
        // so local numbers appear first in the sorted list
        if( localFlag )
        {
            if( *(docRecord_p->workPhoneSearch_p     ) >= '0' &&  *(docRecord_p->workPhoneSearch_p ) <= '9' &&
                *(docRecord_p->workPhoneSearch_p + 1 ) >= '0' &&  *(docRecord_p->workPhoneSearch_p  + 1 ) <= '9' &&
                *(docRecord_p->workPhoneSearch_p + 2 ) >= '0' &&  *(docRecord_p->workPhoneSearch_p  + 2 ) <= '9' )
            {
                *(docRecord_p->workPhoneSearch_p     ) = '0';
                *(docRecord_p->workPhoneSearch_p + 1 ) = '0';
                *(docRecord_p->workPhoneSearch_p + 2 ) = '0';
            }
            else
            {
                mbDlgDebug(( "Should not be here" ));
            }
        }
        docRecord_p->workPhoneInt64 = pmcAtoI64( docRecord_p->workPhoneSearch_p );

        // Delete all modified records if not initial read
        if( initialRead == FALSE )
        {
            UpdateDoctorListRecordDelete( docRecord_p );
        }

        if( docRecord_p->notDeleted == TRUE )
        {
            // Put record into sorted list
            UpdateDoctorListRecordAdd( docRecord_p );

            // Restore area code after sorted list placement
            if( mbStrPos( docRecord_p->workPhoneSearch_p, "000" ) == 0 )
            {
                *(docRecord_p->workPhoneSearch_p     ) = *(pmcCfg[CFG_AREA_CODE].str_p + 0 );
                *(docRecord_p->workPhoneSearch_p + 1 ) = *(pmcCfg[CFG_AREA_CODE].str_p + 1 );
                *(docRecord_p->workPhoneSearch_p + 2 ) = *(pmcCfg[CFG_AREA_CODE].str_p + 2 );
            }
        }
        else
        {
            DoctorRecordFree( docRecord_p );
        }

        i++;
    }

    if( initialRead ) thermometer_p->Set( records );

    nbDlgDebug(( "UpdateDoctorRecords processed %ld records", i ));

    // Sanity checks
    if( pmcDocName_q->size != pmcDocNum_q->size )
    {
        mbDlgDebug(( "Error: pmcDocName_q->size != pmcDocNum_q->size (%ld != !ld)",
             pmcDocName_q->size, pmcDocNum_q->size ));
    }

    // Finally, assign each record an "offset".  This can be used for fast
    // indexing in the doctor list form.
    i = 0;
 
    qWalk( linkage_p, pmcDocName_q, pmcLinkageStruct_p )
    {
        // Get start of record
        docRecord_p = (pmcDocRecordStruct_p)linkage_p->record_p;
        docRecord_p->offset = i++;
    }

exit:
    mbLockRelease( pmcDocListLock );

    if( initialRead )
    {
        delete thermometer_p;
    }
    UpdateDoctorListDone = TRUE;

    mbStrInt32u( pmcDocName_q->size, buf_p );
    DoctorCountLabel->Caption = buf_p;

    WeekViewGrid->Invalidate( );
    DayViewGrid->Invalidate( );
    MonthViewGrid->Invalidate( );
    ProviderViewGrid->Invalidate( );

    if( buf_p ) mbFree( buf_p );
    if( phone_p ) mbFree( phone_p );
}

//---------------------------------------------------------------------------
// Function: TMainForm::UpdateDoctorListDeleteRecord
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::UpdateDoctorListRecordDelete
(
    pmcDocRecordStruct_p    docRecord_p
)
{
    pmcDocRecordStruct_p    docRecord2_p = NIL;
    pmcLinkageStruct_p      linkage_p;
    Int32u_t                foundCount = 0;

    nbDlgDebug(( "Must delete modified doctor id %ld name '%s'",
        docRecord_p->id,
        docRecord_p->lastName_p ));

    qWalk( linkage_p, pmcDocNum_q, pmcLinkageStruct_p )
    {
        // Get start of record
        docRecord2_p = (pmcDocRecordStruct_p)linkage_p->record_p;
        if( docRecord_p->id == docRecord2_p->id )
        {
            qRemoveEntry( pmcDocNum_q, &docRecord2_p->numLinkage );
            foundCount++;
            break;
        }
    }

    qWalk( linkage_p, pmcDocPhone_q, pmcLinkageStruct_p )
    {
        // Get start of record
        docRecord2_p = (pmcDocRecordStruct_p)linkage_p->record_p;
        if( docRecord_p->id == docRecord2_p->id )
        {
            qRemoveEntry( pmcDocPhone_q, &docRecord2_p->phoneLinkage );
            foundCount++;
            break;
        }
    }

    qWalk( linkage_p, pmcDocDelete_q, pmcLinkageStruct_p )
    {
        // Get start of record
        docRecord2_p = (pmcDocRecordStruct_p)linkage_p->record_p;
        if( docRecord_p->id == docRecord2_p->id )
        {
            qRemoveEntry( pmcDocDelete_q, &docRecord2_p->deleteLinkage );
            foundCount++;
            break;
        }
    }

    qWalk( linkage_p, pmcDocName_q,  pmcLinkageStruct_p )
    {
        // Get start of record
        docRecord2_p = (pmcDocRecordStruct_p)linkage_p->record_p;
        if( docRecord_p->id == docRecord2_p->id )
        {
            qRemoveEntry( pmcDocName_q, &docRecord2_p->nameLinkage );
            foundCount++;
            break;
        }
    }

    if( foundCount != 0 )
    {
        if( foundCount != 4 )
        {
            mbDlgDebug(( "Error deleting doctor record id %ld (foundCount: %d)",
                docRecord_p->id, foundCount ));
        }
        else
        {
            DoctorRecordFree( docRecord2_p );
        }
    }
    else
    {
        // Not found.... thats ok.
    }
    return;
}

//---------------------------------------------------------------------------
// Function: TMainForm::DoctorRecordFree
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------
void __fastcall TMainForm::DoctorRecordFree
(
    pmcDocRecordStruct_p    docRecord_p
)
{
    if( docRecord_p )
    {
         // Must free this record (must have been a delete record)
         mbFree( docRecord_p->firstName_p );
         mbFree( docRecord_p->lastName_p );
         mbFree( docRecord_p->lastNameSearch_p );
         mbFree( docRecord_p->num_p );
         mbFree( docRecord_p->workPhoneSearch_p );
         mbFree( docRecord_p->workFax_p );
         mbFree( docRecord_p->workPhone_p );
         mbFree( docRecord_p );
    }
}
//---------------------------------------------------------------------------
// Function: TMainForm::UpdateDoctorListRecordAdd
//---------------------------------------------------------------------------
// Description:
//
// This function adds a patient record into the linked list of records.
// The linked lists are sorted by last name, phn, and home phone number.
//---------------------------------------------------------------------------

void __fastcall TMainForm::UpdateDoctorListRecordAdd
(
    pmcDocRecordStruct_p    docRecord_p
)

{
    Boolean_t               added;
    pmcDocRecordStruct_p    docRecord2_p;
    pmcLinkageStruct_p      entry_p;
    Int32u_t                num1;
    Int32u_t                num2;
    Ints_t                  result;
    Int64u_t                phone1;
    Int64u_t                phone2;
    Ints_t                  diff;
    Ints_t                  j;
    Ints_t                  forward;
    Ints_t                  firstPos;
    Ints_t                  lastPos;
    Ints_t                  pos;
    Ints_t                  totalSize;
    Ints_t                  shift;

    forward = TRUE;
    added = FALSE;
    diff = pmcDocName_q->size;
    entry_p = qFirst( pmcDocName_q, pmcLinkageStruct_p );

    totalSize = diff;
    pos = 0;
    firstPos = 0;
    lastPos = totalSize - 1;

    diff = lastPos - firstPos;

    for( j = 0  ; j < 32 ; j++ )
    {
        if( totalSize == 0 ) break;

        PMC_COMPUTE_SHIFT( shift, diff );

        if( forward )
        {
            pos += shift;
            for( ; shift > 0 ; shift-- ) entry_p = qNext( entry_p, pmcLinkageStruct_p );
        }
        else
        {
            pos -= shift;
            for( ; shift > 0 ; shift-- ) entry_p = qPrev( entry_p, pmcLinkageStruct_p );
        }

        docRecord2_p = (pmcDocRecordStruct_p)entry_p->record_p;

        result = strcmp( docRecord_p->lastNameSearch_p, docRecord2_p->lastNameSearch_p );

        if( result < 0 )
        {
            forward = FALSE;
            if( diff <= 1 )
            {
                qInsertBefore(  pmcDocName_q,
                               &docRecord2_p->nameLinkage,
                               &docRecord_p->nameLinkage );
                added = TRUE;
                break;
            }
            else
            {
                diff = pos - firstPos;
                lastPos = pos;
            }
        }
        else if( result > 0 )
        {
            forward = TRUE;
            if( diff <= 1 )
            {
                qInsertAfter(  pmcDocName_q,
                              &docRecord2_p->nameLinkage,
                              &docRecord_p->nameLinkage );

                added = TRUE;
                break;
            }
            else
            {
                diff = lastPos - pos;
                firstPos = pos;
            }
        }
        else
        {
            // Rewind until last names do not match or start of queue
            mbStrClean( docRecord_p->firstName_p, pmcBuf1_p, TRUE );
            mbStrToUpper( pmcBuf1_p );
            for( ; ; )
            {
                if( pos == 0 ) break;
                pos--;
                entry_p = qPrev( entry_p, pmcLinkageStruct_p );
                docRecord2_p = (pmcDocRecordStruct_p)entry_p->record_p;
                if( strcmp( docRecord_p->lastNameSearch_p, docRecord2_p->lastNameSearch_p ) != 0 )
                {
                    entry_p = qNext( entry_p, pmcLinkageStruct_p );
                    docRecord2_p = (pmcDocRecordStruct_p)entry_p->record_p;
                    pos++;
                    break;
                }
            }

            for( ; ; )
            {
                mbStrClean( docRecord2_p->firstName_p, pmcBuf2_p, TRUE );
                mbStrToUpper( pmcBuf2_p );

                result = strcmp( pmcBuf1_p, pmcBuf2_p );
                if( result < 0 )
                {
                    qInsertBefore(  pmcDocName_q,
                                   &docRecord2_p->nameLinkage,
                                   &docRecord_p->nameLinkage );
                    added = TRUE;
                    break;
                }

                if( pos == totalSize - 1 ) break;
                pos++;
                entry_p = qNext( entry_p, pmcLinkageStruct_p );
                docRecord2_p = (pmcDocRecordStruct_p)entry_p->record_p;

                if( strcmp( docRecord_p->lastNameSearch_p, docRecord2_p->lastNameSearch_p ) != 0 )
                {
                    pos--;
                    entry_p = qPrev( entry_p, pmcLinkageStruct_p );
                    docRecord2_p = (pmcDocRecordStruct_p)entry_p->record_p;
                    break;
                }
            }

            if( !added )
            {
                qInsertAfter(  pmcDocName_q,
                               &docRecord2_p->nameLinkage,
                               &docRecord_p->nameLinkage );
                added = TRUE;
            }
            break;
        }
        if( diff == 0 ) break;
    }

    if( added == FALSE )
    {
        qInsertLast( pmcDocName_q, &docRecord_p->nameLinkage );
    }

    // Add Sorted doc number list
    added = FALSE;
    diff = pmcDocNum_q->size;
    entry_p = qFirst( pmcDocNum_q, pmcLinkageStruct_p );
    totalSize = diff;
    pos = 0;
    firstPos = 0;
    lastPos = totalSize - 1;
    forward = TRUE;

    diff = lastPos - firstPos;

    for( j = 0  ; j < 32 ; j++ )
    {
        if( totalSize == 0 ) break;

        PMC_COMPUTE_SHIFT( shift, diff );

        if( forward )
        {
            pos += shift;
            for( ; shift > 0 ; shift-- ) entry_p = qNext( entry_p, pmcLinkageStruct_p );
        }
        else
        {
            pos -= shift;
            for( ; shift > 0 ; shift-- ) entry_p = qPrev( entry_p, pmcLinkageStruct_p );
        }

        docRecord2_p = (pmcDocRecordStruct_p)entry_p->record_p;

        num1 = docRecord_p->numInt32;
        num2 = docRecord2_p->numInt32;

        if( num1 == 0 ) num1 = PMC_INVALID_DOCTOR_NUMBER;
        if( num2 == 0 ) num2 = PMC_INVALID_DOCTOR_NUMBER;

        // Check for duplicate PHNs while we are at it
        if( ( num1 == num2 ) && ( num1 != PMC_INVALID_DOCTOR_NUMBER ) )
        {
            // Clean this check up later on
            mbDlgExclaim( "Duplicate Doctor Number detected: '%d'", docRecord_p->numInt32 );
        }

        if( num1 < num2 )
        {
            forward = FALSE;
            if( diff <= 1 )
            {
                qInsertBefore(  pmcDocNum_q,
                               &docRecord2_p->numLinkage,
                               &docRecord_p->numLinkage );
                added = TRUE;
                break;
            }
            else
            {
                diff = pos - firstPos;
                lastPos = pos;
            }
        }
        else
        {
            forward = TRUE;
            if( diff <= 1 )
            {
                qInsertAfter(  pmcDocNum_q,
                              &docRecord2_p->numLinkage,
                              &docRecord_p->numLinkage );

                added = TRUE;
                break;
            }
            else
            {
                diff = lastPos - pos;
                firstPos = pos;
            }
        }
        if( diff == 0 ) break;
    }

    if( added == FALSE )
    {
        qInsertLast( pmcDocNum_q, &docRecord_p->numLinkage );
    }

    // Add Sorted Phone list
    added = FALSE;
    diff = pmcDocPhone_q->size;
    entry_p = qFirst( pmcDocPhone_q, pmcLinkageStruct_p );
    totalSize = diff;
    pos = 0;
    firstPos = 0;
    lastPos = totalSize - 1;
    forward = TRUE;

    diff = lastPos - firstPos;

    for( j = 0  ; j < 32 ; j++ )
    {
        if( totalSize == 0 ) break;

        PMC_COMPUTE_SHIFT( shift, diff );

        if( forward )
        {
            pos += shift;
            for( ; shift > 0 ; shift-- ) entry_p = qNext( entry_p, pmcLinkageStruct_p );
        }
        else
        {
            pos -= shift;
            for( ; shift > 0 ; shift-- ) entry_p = qPrev( entry_p, pmcLinkageStruct_p );
        }

        docRecord2_p = (pmcDocRecordStruct_p)entry_p->record_p;

        // Convert health numbers to ints for compares
        phone1 = docRecord_p->workPhoneInt64;
        phone2 = docRecord2_p->workPhoneInt64;

        if( phone1 == 0 ) phone1 = PMC_INVALID_PHONE;
        if( phone2 == 0 ) phone2 = PMC_INVALID_PHONE;

        if( phone1 < phone2 )
        {
            forward = FALSE;
            if( diff <= 1 )
            {
                qInsertBefore(  pmcDocPhone_q,
                               &docRecord2_p->phoneLinkage,
                               &docRecord_p->phoneLinkage );
                added = TRUE;
                break;
            }
            else
            {
                diff = pos - firstPos;
                lastPos = pos;
            }
        }
        else
        {
            forward = TRUE;
            if( diff <= 1 )
            {
                qInsertAfter(  pmcDocPhone_q,
                              &docRecord2_p->phoneLinkage,
                              &docRecord_p->phoneLinkage );

                added = TRUE;
                break;
            }
            else
            {
                diff = lastPos - pos;
                firstPos = pos;
            }
        }
        if( diff == 0 ) break;
    }
    if( added == FALSE )
    {
        qInsertLast( pmcDocPhone_q, &docRecord_p->phoneLinkage );
    }
    
    qInsertFirst( pmcDocDelete_q, &docRecord_p->deleteLinkage );
}
