//---------------------------------------------------------------------------
// File:    pmcUpdatePatientList.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    Feb. 21, 2001
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
#include "pmcMainForm.h"
#include "pmcReportForm.h"

#pragma package(smart_init)

//---------------------------------------------------------------------------
// Function: TMainForm::UpdatePatientList
//---------------------------------------------------------------------------
// Description:
//
// Read modified records from SQL database and insert into (or delete from)
// internal list.
//---------------------------------------------------------------------------

void __fastcall TMainForm::UpdatePatientList
(
    pmcTableStatus_p        tableStatus_p,
    Boolean_t               forceFlag
)
{
    TThermometer           *thermometer_p;
    Char_p                  temp_p;
    Int32u_t                i;
    Int32u_t                records = 0;
    Int32u_t                invalidPhnCount = 0;
    pmcPatRecordStruct_p    patRecord_p;
    Boolean_t               initialRead;
    Boolean_t               localFlag;
    Boolean_t               checkPhn;
    FILE                   *fp = NIL;
    pmcLinkageStruct_p      linkage_p;
    Char_p                  buf_p;
    Char_p                  buf2_p;
    Char_p                  buf3_p;
    Char_t                  area[8];
    Char_t                  phone[64];
    Char_p                  fileName_p = NIL;
    MbSQL                   sql;

    if( UpdatePatientListDoInit == FALSE ) return;

    if( !forceFlag )
    {
        if( tableStatus_p->newModifyTime == tableStatus_p->curModifyTime &&
            tableStatus_p->newDataSize   == tableStatus_p->curDataSize )
        {
            return;
        }
    }

    mbMalloc( buf_p, 512 );
    mbMalloc( buf2_p, 256 );
    mbMalloc( buf3_p, 256 );

//    pmcLog( "Must update patient records: new: %Ld cur:%Ld last read: %Ld\n",
//        tableStatus_p->newModifyTime,
//        tableStatus_p->curModifyTime,
//        tableStatus_p->lastReadTime  );

    UpdatePatientListDone =  FALSE;

    if( tableStatus_p->lastReadTime == 0i64 )
    {
        // Read all non deleted patient records
        nbDlgDebug(( "This is initial read of the table" ));
        initialRead = TRUE;

        sprintf( buf_p, "select %s,%s,%s,%s,%s,%s,%s,%s,%s from %s where %s=1 and %s>0 and %s>0 order by last_name",
            PMC_SQL_FIELD_FIRST_NAME,           // 0
            PMC_SQL_FIELD_LAST_NAME,            // 1
            PMC_SQL_FIELD_ID,                   // 2
            PMC_SQL_PATIENTS_FIELD_PHN,         // 3
            PMC_SQL_PATIENTS_FIELD_PHN_PROV,    // 4
            PMC_SQL_PATIENTS_FIELD_HOME_PHONE,  // 5
            PMC_SQL_FIELD_TITLE,                // 6
            PMC_SQL_PATIENTS_FIELD_GENDER,      // 7
            PMC_SQL_FIELD_NOT_DELETED,          // 8

            PMC_SQL_TABLE_PATIENTS,
            PMC_SQL_FIELD_NOT_DELETED,
            PMC_SQL_FIELD_MODIFIED,
            PMC_SQL_FIELD_ID );
    }
    else
    {
        MbDateTime tempTime = MbDateTime( tableStatus_p->lastReadTime );
        // Read all records (incl deleted) with modify time > lastReadTime
        initialRead = FALSE;
        tempTime.BackOneMinute();
        tableStatus_p->lastReadTime = tempTime.Int64();

        sprintf( buf2_p, "%Lu",  tableStatus_p->lastReadTime );

        //                       0  1  2  3  4  5  6  7  8
        sprintf( buf_p, "select %s,%s,%s,%s,%s,%s,%s,%s,%s from %s where %s>%s and %s>0 order by last_name"
           ,PMC_SQL_FIELD_FIRST_NAME           // 0
            ,PMC_SQL_FIELD_LAST_NAME            // 1
            ,PMC_SQL_FIELD_ID                   // 2
            ,PMC_SQL_PATIENTS_FIELD_PHN         // 3
            ,PMC_SQL_PATIENTS_FIELD_PHN_PROV    // 4
            ,PMC_SQL_PATIENTS_FIELD_HOME_PHONE  // 5
            ,PMC_SQL_FIELD_TITLE                // 6
            ,PMC_SQL_PATIENTS_FIELD_GENDER      // 7
            ,PMC_SQL_FIELD_NOT_DELETED          // 8

            ,PMC_SQL_TABLE_PATIENTS
            ,PMC_SQL_FIELD_MODIFIED     ,buf2_p
            ,PMC_SQL_FIELD_ID );

        pmcCfg[CFG_SKIP_SORT].value = FALSE;
    }

    mbLockAcquire( pmcPatListLock );

    // Get a patient record count
    if( initialRead == TRUE )
    {
        // We will be reading the entire table, get count for thermometer
        records = pmcSqlSelectInt( PMC_SQL_CMD_PATIENTS_COUNT, NIL );
    }

    if( initialRead )
    {
        Char_t  tmp[32];

        sprintf( buf2_p, "Processing %s patient records...", mbStrInt32u( records, tmp ) );
        thermometer_p = new TThermometer( buf2_p, 0, records, FALSE );
    }

    i = 0;

    if( sql.Query( buf_p ) == FALSE ) goto exit;

    while( sql.RowGet( ) )
    {
        if( initialRead ) thermometer_p->Set( i );

        // Allocate space for the patient record
        mbCalloc( patRecord_p, sizeof( pmcPatRecordStruct_t ) );

        // Set pointers in linkage structs to start of this record
        patRecord_p->nameLinkage.record_p   = (Void_p)patRecord_p;
        patRecord_p->phoneLinkage.record_p  = (Void_p)patRecord_p;
        patRecord_p->phnLinkage.record_p    = (Void_p)patRecord_p;
        patRecord_p->deleteLinkage.record_p = (Void_p)patRecord_p;

        // Get "deleted" indicator
        patRecord_p->notDeleted = sql.Int32u( 8 ); //strtoul( row[8], NIL, 10 );

        // Get patient Id
        patRecord_p->id = sql.Int32u( 2 ); //strtoul( row[2], NIL, 10 );

        // Get first name
        temp_p = sql.String( 0 );
        patRecord_p->firstNameLen = strlen( temp_p ) + 1;
        mbMalloc( patRecord_p->firstName_p, patRecord_p->firstNameLen );
        strcpy( patRecord_p->firstName_p, temp_p );

        // Get last name
        temp_p = sql.String( 1 );
        patRecord_p->lastNameLen =  strlen( temp_p ) + 1;
        mbMalloc( patRecord_p->lastName_p, patRecord_p->lastNameLen );
        strcpy( patRecord_p->lastName_p, temp_p );

         // Last name (search version)
        patRecord_p->lastNameSearchLen = patRecord_p->lastNameLen;
        mbMalloc( patRecord_p->lastNameSearch_p, patRecord_p->lastNameSearchLen );
        strcpy( patRecord_p->lastNameSearch_p, temp_p );
        mbStrToLower( patRecord_p->lastNameSearch_p );

        // Get title
        strncpy( patRecord_p->title, sql.String( 6 ), PMC_TITLE_LEN );

        // Get PHN (Personal Health Number)
        sprintf( buf_p, sql.String( 3 ) );

        *buf2_p = 0;
        sprintf( buf2_p, sql.String( 4 ) );

        // Set default health number province to SK
        if( strlen( buf2_p ) == 0 ) sprintf( buf2_p, PMC_PHN_DEFAULT_PROVINCE );

        checkPhn = ( mbStrPos( buf2_p, PMC_PHN_DEFAULT_PROVINCE ) == 0 ) ? TRUE : FALSE;

        pmcFormatPhnDisplay( buf_p, buf2_p, buf3_p );
        patRecord_p->phnLen = strlen( buf3_p ) + 1;
        mbMalloc( patRecord_p->phn_p, patRecord_p->phnLen );
        strcpy( patRecord_p->phn_p, buf3_p );

        pmcFormatPhnSearch( buf_p, buf3_p );
        patRecord_p->phnSearchLen = strlen( buf3_p ) + 1;
        mbMalloc( patRecord_p->phnSearch_p, patRecord_p->phnSearchLen );
        strcpy( patRecord_p->phnSearch_p, buf3_p );

        if( checkPhn && initialRead )
        {
            // Only check on initial read Sask PHNs
            if( strlen( patRecord_p->phnSearch_p ) )
            {
                if( pmcPhnVerifyString( patRecord_p->phnSearch_p ) == FALSE )
                {
                    if( fp == NIL && fileName_p == NIL )
                    {
                        // Open report file if not open already
                        mbMalloc( fileName_p, 256 );
                        if( fileName_p )
                        {
                            pmcMakeFileName( pmcCfg[CFG_REPORT_DIR].str_p, fileName_p );
                            strcat( fileName_p, "_invalid_phn.txt" );
                            if( ( fp = fopen( fileName_p, "w" ) ) == NIL )
                            {
                                mbDlgExclaim( "Could not open report file '%s'.", fileName_p );
                            }
                        }
                    }
                    if( fp )
                    {
                        fprintf( fp, "Invalid PHN: '%s' (%s, %s)\n",
                            patRecord_p->phnSearch_p, patRecord_p->lastName_p, patRecord_p->firstName_p );
                        invalidPhnCount++;
                    }
                }
            }
        }

        mbStrDigitsOnly( buf_p );
        patRecord_p->phnInt64 = pmcAtoI64( buf_p );

        // Get home phone in integer format
        strcpy( buf_p, sql.String( 5 ) );
        pmcPhoneFix( buf_p, NIL );

        // buf contains a string of digits suitable for string searching
        patRecord_p->homePhoneSearchLen = strlen( buf_p ) + 4;
        mbMalloc( patRecord_p->homePhoneSearch_p, patRecord_p->homePhoneSearchLen );
        memset( patRecord_p->homePhoneSearch_p, 0, patRecord_p->homePhoneSearchLen );
        strcpy( patRecord_p->homePhoneSearch_p, buf_p );

        pmcPhoneFormat( patRecord_p->homePhoneSearch_p, area, phone, &localFlag );
        patRecord_p->displayAreaCode = ( localFlag == TRUE ) ? FALSE : TRUE;
        strncpy( patRecord_p->areaCode, area, PMC_AREA_CODE_LEN );

        patRecord_p->homePhoneLen = strlen( phone ) + 1;
        mbMalloc( patRecord_p->homePhone_p, patRecord_p->homePhoneLen );
        strcpy( patRecord_p->homePhone_p, phone );

        // If the area code equals the local area code, set to 0
        // so local numbers appear first in the sorted list
        if( localFlag )
        {
            // Sanity check
            if( *(patRecord_p->homePhoneSearch_p     ) >= '0' &&  *(patRecord_p->homePhoneSearch_p ) <= '9' &&
                *(patRecord_p->homePhoneSearch_p + 1 ) >= '0' &&  *(patRecord_p->homePhoneSearch_p  + 1 ) <= '9' &&
                *(patRecord_p->homePhoneSearch_p + 2 ) >= '0' &&  *(patRecord_p->homePhoneSearch_p  + 2 ) <= '9' )
            {
                *(patRecord_p->homePhoneSearch_p     ) = '0';
                *(patRecord_p->homePhoneSearch_p + 1 ) = '0';
                *(patRecord_p->homePhoneSearch_p + 2 ) = '0';
            }
            else
            {
                mbDlgDebug(( "Should not be here" ));
            }
        }
        patRecord_p->homePhoneInt64 = pmcAtoI64( patRecord_p->homePhoneSearch_p );

        // Delete all modified records if not initial read
        if( initialRead == FALSE )
        {
            UpdatePatientListRecordDelete( patRecord_p );
        }

        if( patRecord_p->notDeleted == TRUE )
        {
            // Put patient record into sorted list
            UpdatePatientListRecordAdd( patRecord_p );

            // Restore area code after sorted list placement
            if( mbStrPos( patRecord_p->homePhoneSearch_p, "000" ) == 0 )
            {
                *(patRecord_p->homePhoneSearch_p     ) = *( pmcCfg[CFG_AREA_CODE].str_p     );
                *(patRecord_p->homePhoneSearch_p + 1 ) = *( pmcCfg[CFG_AREA_CODE].str_p + 1 );
                *(patRecord_p->homePhoneSearch_p + 2 ) = *( pmcCfg[CFG_AREA_CODE].str_p + 2 );
            }
        }
        else
        {
            PatientRecordFree( patRecord_p );
        }
        i++;
    }

    if( initialRead )
    {
        thermometer_p->Set( records );
    }

    // Sanity checks
    if( pmcPatName_q->size != pmcPatPhn_q->size )
    {
        mbDlgDebug(( "Error: pmcPatName_q->size != pmcPatPhn_q->size (%lu != %lu)",
             pmcPatName_q->size, pmcPatPhn_q->size ));
    }
    if( pmcPatName_q->size != pmcPatPhone_q->size )
    {
        mbDlgDebug(( "Error: pmcPatName_q->size != pmcPatPhone_q->size (%lu != %lu)",
             pmcPatName_q->size, pmcPatPhone_q->size ));
    }

    if( pmcPatName_q->size != pmcPatDelete_q->size )
    {
        mbDlgDebug(( "Error: pmcPatName_q->size != pmcPatDelete_q->size (%lu != %lu)",
             pmcPatName_q->size, pmcPatDelete_q->size ));
    }

    // Finally, assign each record an "offset".  This can be used for fast
    // indexing in the patient list form.
    i = 0;

    qWalk( linkage_p, pmcPatName_q, pmcLinkageStruct_p )
    {
        // Get start of record
        patRecord_p = (pmcPatRecordStruct_p)linkage_p->record_p;
        patRecord_p->offset = i++;
    }

exit:
    mbLockRelease( pmcPatListLock );

    if( initialRead )
    {
        delete thermometer_p;
        // The program started out with the day view grid resource used.
        // Free it here (one time only )
        PMC_DEC_USE( DayViewInfo.notReady       );
        PMC_DEC_USE( WeekViewInfo.notReady      );
        PMC_DEC_USE( MonthViewInfo.notReady     );
        PMC_DEC_USE( ProviderViewInfo.notReady  );
    }

    UpdatePatientListDone = TRUE;

    MainForm->PatientCountUpdate( PMC_COUNTER_UPDATE );
    if( initialRead )
    {
        MainForm->AppointmentCountUpdate( PMC_COUNTER_UPDATE );
        MainForm->DocumentCountUpdate( PMC_COUNTER_UPDATE );
    }

    tableStatus_p->curModifyTime = tableStatus_p->newModifyTime;
    tableStatus_p->lastReadTime  = tableStatus_p->newModifyTime;
    tableStatus_p->curDataSize   = tableStatus_p->newDataSize;

    if( initialRead && pmcCfg[CFG_INIT_MODIFY_FLAG].value )
    {
        // Check to see if all the modify times in the tables should be set.
        // This is required for demos with "natasha".  That computer's clock
        // does not work, and it is possible to have many records with
        // modification times in the future. These records end up getting
        // read every time we do a list update.  Setting all the modify times
        // to the current table time removes this problem.

        if( mbDlgOkCancel( "Initialize database modification times?\n" ) == MB_BUTTON_OK )
        {
            sprintf( buf_p, "update patients      set modified=%Ld where id>0", tableStatus_p->curModifyTime - 200i64 );  pmcSqlExec( buf_p );
            sprintf( buf_p, "update doctors       set modified=%Ld where id>0", tableStatus_p->curModifyTime - 200i64 );  pmcSqlExec( buf_p );
            sprintf( buf_p, "update providers     set modified=%Ld where id>0", tableStatus_p->curModifyTime - 200i64 );  pmcSqlExec( buf_p );
            sprintf( buf_p, "update appointments  set modified=%Ld where id>0", tableStatus_p->curModifyTime - 200i64 );  pmcSqlExec( buf_p );
            mbDlgInfo( "Database modification time update complete (set to %Ld)\n", tableStatus_p->curModifyTime - 200i64 );
        }
    }

    //pmcLog( "Table info after update: new: %Ld cur:%Ld last read: %Ld\n",
    //    tableStatus_p->newModifyTime,
    //    tableStatus_p->curModifyTime,
    //    tableStatus_p->lastReadTime  );

    // Indicate results of PHN check
    if( fp ) fclose( fp );
    if( invalidPhnCount && initialRead )
    {
#if 0
        pmcReportFormInfo_t     reportFormInfo;
        TReportForm            *reportForm_p;

        sprintf( buf_p, "%ld invalid PHN%s detected", invalidPhnCount, ( invalidPhnCount == 1 ) ? "" : "s" );
        reportFormInfo.fileName_p = fileName_p;
        reportFormInfo.caption_p = buf_p;

        reportForm_p = new TReportForm( this, &reportFormInfo );
        reportForm_p->ShowModal();
        delete reportForm_p;
#endif
    }

    WeekViewGrid->Invalidate( );
    DayViewGrid->Invalidate( );
    MonthViewGrid->Invalidate( );
    ProviderViewGrid->Invalidate( );

    if( fileName_p ) mbFree( fileName_p );

    mbFree( buf_p  );
    mbFree( buf2_p );
    mbFree( buf3_p );
}

//---------------------------------------------------------------------------
// Function: TMainForm::UpdatePatientListDeleteRecord
//---------------------------------------------------------------------------
// Description:
//
// This function adds a patient record into the linked list of records.
// The linked lists are sorted by last name, phn, and home phone number.
//---------------------------------------------------------------------------

void __fastcall TMainForm::UpdatePatientListRecordDelete
(
    pmcPatRecordStruct_p    patRecord_p
)
{
    pmcPatRecordStruct_p    patRecord2_p = NIL;
    pmcLinkageStruct_p      linkage_p;
    Int32u_t                foundCount = 0;

    nbDlgDebug(( "Must delete modified patient id %ld name '%s'", patRecord_p->id, patRecord_p->lastName_p ));

    qWalk( linkage_p, pmcPatDelete_q, pmcLinkageStruct_p )
    {
        // Get start of record
        patRecord2_p = (pmcPatRecordStruct_p)linkage_p->record_p;
        if( patRecord_p->id == patRecord2_p->id )
        {
            qRemoveEntry( pmcPatDelete_q, &patRecord2_p->deleteLinkage );
            nbDlgDebug(( "Delete patient id %ld from phone list", patRecord2_p->id ));
            foundCount++;
            break;
        }
    }

    qWalk( linkage_p, pmcPatPhone_q, pmcLinkageStruct_p )
    {
        // Get start of record
        patRecord2_p = (pmcPatRecordStruct_p)linkage_p->record_p;
        if( patRecord_p->id == patRecord2_p->id )
        {
            qRemoveEntry( pmcPatPhone_q, &patRecord2_p->phoneLinkage );
            nbDlgDebug(( "Delete patient id %ld from phone list", patRecord2_p->id ));
            foundCount++;
            break;
        }
    }

    qWalk( linkage_p, pmcPatPhn_q, pmcLinkageStruct_p )
    {
        // Get start of record
        patRecord2_p = (pmcPatRecordStruct_p)linkage_p->record_p;
        if( patRecord_p->id == patRecord2_p->id )
        {
            qRemoveEntry( pmcPatPhn_q, &patRecord2_p->phnLinkage );
            nbDlgDebug(( "Delete patient id %ld from PHN list", patRecord_p->id ));
            foundCount++;
            break;
        }
    }

    qWalk( linkage_p, pmcPatName_q, pmcLinkageStruct_p )
    {
        // Get start of record
        patRecord2_p = (pmcPatRecordStruct_p)linkage_p->record_p;
        if( patRecord_p->id == patRecord2_p->id )
        {
            qRemoveEntry( pmcPatName_q, &patRecord2_p->nameLinkage );
            nbDlgDebug(( "Delete patient id %ld from Name list", patRecord_p->id ));
            foundCount++;
            break;
        }
    }

    if( foundCount != 0 )
    {
        if( foundCount != 4 )
        {
            mbDlgDebug(( "Error deleting patient record id %ld (foundCount: %d)",
                patRecord_p->id, foundCount ));
        }
        else
        {
            PatientRecordFree( patRecord2_p );
        }
    }
    else
    {
        // Not found.... thats ok.
    }
    return;
}

//---------------------------------------------------------------------------
// Function: TMainForm::PatientRecordFree
//---------------------------------------------------------------------------
// Description:
//

//---------------------------------------------------------------------------
void __fastcall TMainForm::PatientRecordFree
(
    pmcPatRecordStruct_p    patRecord_p
)
{
    if( patRecord_p )
    {
        mbFree( patRecord_p->firstName_p );
        mbFree( patRecord_p->lastName_p );
        mbFree( patRecord_p->lastNameSearch_p );
        mbFree( patRecord_p->phnSearch_p );
        mbFree( patRecord_p->phn_p );
        mbFree( patRecord_p->homePhoneSearch_p );
        mbFree( patRecord_p->homePhone_p );
        mbFree( patRecord_p );
    }
}
//---------------------------------------------------------------------------
// Function: TMainForm::UpdatePatientListRecordAdd
//---------------------------------------------------------------------------
// Description:
//
// This function adds a patient record into the linked list of records.
// The linked lists are sorted by last name, phn, and home phone number.
//---------------------------------------------------------------------------

#define PMC_SEARCH_DEBUG (0)

void __fastcall TMainForm::UpdatePatientListRecordAdd
(
    pmcPatRecordStruct_p    patRecord_p
)

{

    if( pmcCfg[CFG_SKIP_SORT].value == TRUE )
    {

    qInsertLast( pmcPatName_q, &patRecord_p->nameLinkage );
    qInsertLast( pmcPatPhone_q, &patRecord_p->phoneLinkage );
    qInsertLast( pmcPatPhn_q, &patRecord_p->phnLinkage );
    qInsertFirst( pmcPatDelete_q, &patRecord_p->deleteLinkage );

    }
    else
    {

    bool                    added;
    Ints_t                  result;
    pmcPatRecordStruct_p    patRecord2_p;
    pmcLinkageStruct_p      entry_p;
    Int64u_t                phn1;
    Int64u_t                phn2;
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
    diff = pmcPatName_q->size;
    entry_p = qFirst( pmcPatName_q, pmcLinkageStruct_p );
    totalSize = diff;
    pos = 0;
    firstPos = 0;
    lastPos = totalSize - 1;

    diff = lastPos - firstPos;

    for( j = 0 ; j < 32 ; j++ )
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

        patRecord2_p = (pmcPatRecordStruct_p)entry_p->record_p;
        result = strcmp( patRecord_p->lastNameSearch_p, patRecord2_p->lastNameSearch_p );

#if PMC_SEARCH_DEBUG
        {
           mbLog( "pos %5d diff %5d shift: %5d (%s %s) result: %d\n",
                pos,
                diff, shift,
                patRecord_p->lastNameSearch_p, patRecord2_p->lastNameSearch_p,
                result );
        }
#endif

        if( result < 0 )
        {
            forward = FALSE;
            if( diff <= 1 )
            {
#if PMC_SEARCH_DEBUG

                mbLog( "add %s before %s pos: %d\n",
                        patRecord_p->lastNameSearch_p,
                        patRecord2_p->lastNameSearch_p,
                        pos );

#endif
                qInsertBefore(  pmcPatName_q,
                               &patRecord2_p->nameLinkage,
                               &patRecord_p->nameLinkage );
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
#if PMC_SEARCH_DEBUG
                mbLog( "add %s after %s pos: %d\n",
                          patRecord_p->lastNameSearch_p,
                          patRecord2_p->lastNameSearch_p,
                          pos );

#endif
                qInsertAfter(  pmcPatName_q,
                              &patRecord2_p->nameLinkage,
                              &patRecord_p->nameLinkage );

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
            mbStrClean( patRecord_p->firstName_p, pmcBuf1_p, TRUE );
            mbStrToUpper( pmcBuf1_p );
            for( ; ; )
            {
#if PMC_SEARCH_DEBUG
                mbLog( "Looping A" );
#endif
                if( pos == 0 ) break;
                pos--;
                entry_p = qPrev( entry_p, pmcLinkageStruct_p );
                patRecord2_p = (pmcPatRecordStruct_p)entry_p->record_p;
                if( strcmp( patRecord_p->lastNameSearch_p, patRecord2_p->lastNameSearch_p ) != 0 )
                {
                    entry_p = qNext( entry_p, pmcLinkageStruct_p );
                    patRecord2_p = (pmcPatRecordStruct_p)entry_p->record_p;
                    pos++;
                    break;
                }
            }

            for( ; ; )
            {
#if PMC_SEARCH_DEBUG
                mbLog( "Looping B" );
#endif
                mbStrClean( patRecord2_p->firstName_p, pmcBuf2_p, TRUE );
                mbStrToUpper( pmcBuf2_p );

                result = strcmp( pmcBuf1_p, pmcBuf2_p );
                if( result < 0 )
                {
                    qInsertBefore(  pmcPatName_q,
                                   &patRecord2_p->nameLinkage,
                                   &patRecord_p->nameLinkage );
                    added = TRUE;
                    break;
                }

                if( pos == ( totalSize - 1  ) ) break;
                pos++;
                entry_p = qNext( entry_p, pmcLinkageStruct_p );
                patRecord2_p = (pmcPatRecordStruct_p)entry_p->record_p;

                if( strcmp( patRecord_p->lastNameSearch_p, patRecord2_p->lastNameSearch_p ) != 0 )
                {
//                    pos--;
                    entry_p = qPrev( entry_p, pmcLinkageStruct_p );
                    patRecord2_p = (pmcPatRecordStruct_p)entry_p->record_p;
                    break;
                }
            }

            if( !added )
            {
                qInsertAfter(  pmcPatName_q,
                               &patRecord2_p->nameLinkage,
                               &patRecord_p->nameLinkage );
                added = TRUE;
            }
            break;
        }
        if( diff == 0 ) break;
    }

    if( added == FALSE )
    {
#if PMC_SEARCH_DEBUG
        mbLog( "Add %s to end of queue\n", patRecord_p->lastNameSearch_p );
#endif
        qInsertLast( pmcPatName_q, &patRecord_p->nameLinkage );
    }

    // Add Sorted PHN list
    added = FALSE;
    diff = pmcPatPhn_q->size;
    entry_p = qFirst( pmcPatPhn_q, pmcLinkageStruct_p );
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

        patRecord2_p = (pmcPatRecordStruct_p)entry_p->record_p;

        // Convert health numbers to ints for compares
        phn1 = patRecord_p->phnInt64;
        phn2 = patRecord2_p->phnInt64;

        if( phn1 == 0 ) phn1 = PMC_INVALID_PHN;
        if( phn2 == 0 ) phn2 = PMC_INVALID_PHN;

        // Check for duplicate PHNs while we are at it
        if( ( phn1 == phn2 ) && ( phn1 != PMC_INVALID_PHN ) )
        {
            // Clean this check up later on
            //mbDlgExclaim( "Duplicate PHN detected: '%s'", patRecord_p->phnSearch_p );
            mbLog(  "Duplicate PHN detected: '%s'", patRecord_p->phnSearch_p );
        }

        if( phn1 < phn2 )
        {
            forward = FALSE;
            if( diff <= 1 )
            {
                qInsertBefore(  pmcPatPhn_q,
                               &patRecord2_p->phnLinkage,
                               &patRecord_p->phnLinkage );
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
                qInsertAfter(  pmcPatPhn_q,
                              &patRecord2_p->phnLinkage,
                              &patRecord_p->phnLinkage );

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
        qInsertLast( pmcPatPhn_q, &patRecord_p->phnLinkage );
    }

    // Add Sorted Phone list
    added = FALSE;
    diff = pmcPatPhone_q->size;
    entry_p = qFirst( pmcPatPhone_q, pmcLinkageStruct_p );
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

        patRecord2_p = (pmcPatRecordStruct_p)entry_p->record_p;

        // Convert health numbers to ints for compares
        phone1 = patRecord_p->homePhoneInt64;
        phone2 = patRecord2_p->homePhoneInt64;

        if( phone1 == 0 ) phone1 = PMC_INVALID_PHONE;
        if( phone2 == 0 ) phone2 = PMC_INVALID_PHONE;

        if( phone1 < phone2 )
        {
            forward = FALSE;
            if( diff <= 1 )
            {
                qInsertBefore(  pmcPatPhone_q,
                               &patRecord2_p->phoneLinkage,
                               &patRecord_p->phoneLinkage );
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
                qInsertAfter(  pmcPatPhone_q,
                              &patRecord2_p->phoneLinkage,
                              &patRecord_p->phoneLinkage );

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
        qInsertLast( pmcPatPhone_q, &patRecord_p->phoneLinkage );
    }

    qInsertFirst( pmcPatDelete_q, &patRecord_p->deleteLinkage );

    }
}

